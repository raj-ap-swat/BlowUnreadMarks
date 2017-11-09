#define NT

#define _CRT_SECURE_NO_WARNINGS

/* OS and C include files */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/* Notes API include files */
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <osenv.h>
#include <osfile.h>
#include <osmisc.h>

char    error_text[200];
Print_API_Error(STATUS api_error);

int main(int argc, char *argv[])
{

	STATUS		err = NOERROR;
	char		szLocalMailFile[256], szLocalMailFileTempCopy[256];
	HANDLE		hDB2;

	DWORD        Stats[2];      /* status return code */


	if (argc != 2)
	{
		printf("Usage: %s <Databse with path relative to data folder>", argv[0]);
		exit(0);
	}

	if (NotesInitExtended(argc, argv))
	{
		printf("\nError initializing Notes. Exiting the program.");
		exit(0);
	}

	strcpy(szLocalMailFile, argv[1]);
	sprintf(szLocalMailFileTempCopy, "%s.tmp", szLocalMailFile);

	printf("Processing:  %s.\n", szLocalMailFile);
	printf("Disabling unread marks..\n");
	if (err = NSFDbCompactExtended(szLocalMailFile, DBCOMPACT_DISABLE_UNREAD, &Stats[0]))
	{
		printf("Unable to disable unread marks table.\n");
		Print_API_Error(err);
			
	}
	else
	{
		printf("Enabling unread marks..\n");

		if (err = NSFDbCompactExtended(szLocalMailFile, DBCOMPACT_ENABLE_UNREAD, &Stats[0]))
		{
			printf("Unable to reenable unread marks table.\n");
			Print_API_Error(err);
		}


		if (err = NSFDbCreateAndCopy(szLocalMailFile, szLocalMailFileTempCopy, NOTE_CLASS_ALL, 0, DBCOPY_REPLICA, &hDB2))
		{
			printf(" Error Creating temp replica %s.", szLocalMailFileTempCopy);
			Print_API_Error(err);
			NotesTerm();
			return (0);
		}

		NSFDbClose(hDB2);

		if (err = NSFDbCompactExtended(szLocalMailFileTempCopy, DBCOMPACT_ENABLE_UNREAD, &Stats[0]))
		{		
			printf("Unable to reenable unread marks table.\n");
			Print_API_Error(err);
		}

		if (err = NSFDbDelete(szLocalMailFile))
		{
			printf("There was an error deleting old database after a new replica created.");
			Print_API_Error(err);
			NotesTerm();
			return (0);
		}

		if (err = NSFDbRename(szLocalMailFileTempCopy, szLocalMailFile))
		{
			Print_API_Error(err);
			NotesTerm();
			return (0);
		}
	}

	printf("\nDone.");
	NotesTerm();

	return 0;
}

Print_API_Error(STATUS api_error)
{
	STATUS   string_id = ERR(api_error);
	WORD     text_len;

	/* Get the text for this API error code from the resource string table. */
	text_len = OSLoadString(NULLHANDLE, string_id, error_text, sizeof(error_text));
	fprintf(stderr, " APIError:  %s\n", error_text);
}