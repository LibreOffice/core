/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif



#ifndef UCHAR
typedef unsigned char  UCHAR;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif



/* max filename length (bytes): don't change */

#define ETCP_MAX_FILENAME_LENGTH 255



/* VB file attributes */

#define ETCP_ATTR_NORMAL     0    /* normal file */
#define ETCP_ATTR_READONLY   1    /* read-only */
#define ETCP_ATTR_HIDDEN     2    /* hidden */
#define ETCP_ATTR_SYSTEM     4    /* system */
#define ETCP_ATTR_VOLUME     8    /* disk volume label */
#define ETCP_ATTR_DIRECTORY 16    /* subdirectory */
#define ETCP_ATTR_ARCHIVE   32    /* archive */



/* Archive type constants. */

#define ETCP_ARCHIVE_UNKNOWN      0    /* unknown */
#define ETCP_ARCHIVE_ZIP          1    /* .ZIP archive, PK format */
#define ETCP_ARCHIVE_ZIP_ETMV     2    /* .ZIP multi-volume archive, EllTech format */
#define ETCP_ARCHIVE_ZIP_PKMV     3    /* .ZIP multi-volume archive, PK format */
#define ETCP_ARCHIVE_ZIP_IMMV     4    /* .ZIP multi-volume archive, Inner Media format */
#define ETCP_ARCHIVE_LZH          5    /* .LZH archive (LHA/LHARC format) */
#define ETCP_ARCHIVE_ARJ          6    /* .ARJ archive */
#define ETCP_ARCHIVE_ZOO          7    /* .ZOO archive */
#define ETCP_ARCHIVE_ARC          8    /* .ARC archive */
#define ETCP_ARCHIVE_ZIP_EXE     13    /* .EXE archive, PKZIP self-extracting format */



/* If the user status routine decides to let us handle it after all... */

#define ETCP_YOU_DO_IT -10000



/* Status display constants. */

#define ETCP_OPERATION_ZIP      0      /* zipping */
#define ETCP_OPERATION_UNZIP    1      /* unzipping */
#define ETCP_OPERATION_DELETE   2      /* deleting */
#define ETCP_OPERATION_TEST     3      /* testing */



/* Status display message type constants. */

#define ETCP_MESSAGETYPE_ERROR    0    /* error */
#define ETCP_MESSAGETYPE_QUERY    1    /* query */
#define ETCP_MESSAGETYPE_STATUS   2    /* status or progress report */



/* Status Operation and MhZip .Action constants */

#define ETCP_ACTION_NONE    0
#define ETCP_ACTION_EXAMINE 1       /* directory services */
#define ETCP_ACTION_ADD     2       /* add files to an archive */
#define ETCP_ACTION_UPDATE  3       /* update an archive */
#define ETCP_ACTION_FRESHEN 4       /* freshen an archive */
#define ETCP_ACTION_DELETE  5       /* delete files from an archive */
#define ETCP_ACTION_EXTRACT 6       /* extract files from an archive */
#define ETCP_ACTION_TEST    7       /* test archive files without extracting */

/* Constants for query codes (MhZip_Prompt event) and CPrint callbacks */

#define ETCP_QUERY_INSERT_DISK 1    /* insert a specific disk # in a MV set */
#define ETCP_QUERY_OVERWRITE   2    /* overwrite confirmation */
#define ETCP_QUERY_INSERT_LAST 3    /* insert the last disk in a PK-compatible MV set */

/* MhZip .OverWrite property constants */

#define ETCP_OVERWRITE_PROMPT 0
#define ETCP_OVERWRITE_ALWAYS 1
#define ETCP_OVERWRITE_NEVER  2

/* MhZip .EntryTypes property constants */

#define ETCP_ENTRIES_FILES  0
#define ETCP_ENTRIES_DIRVOL 1
#define ETCP_ENTRIES_ALL    2

/* MhZip .ExtractFreshen property constants */

#define ETCP_EXFR_ALL         0
#define ETCP_EXFR_NEWEREXISTS 1
#define ETCP_EXFR_NEWER       2

/* MhZip .FileArchiveBit property constants */

#define ETCP_FAB_IGNORE         0
#define ETCP_FAB_ONLYIFSET      1
#define ETCP_FAB_ALLRESET       2
#define ETCP_FAB_ONLYIFSETRESET 3

/* MhZip .IncludeHidSys property constants */

#define ETCP_HIDSYS_EXCLUDE 0
#define ETCP_HIDSYS_HIDDEN  1
#define ETCP_HIDSYS_SYSTEM  2
#define ETCP_HIDSYS_BOTH    3

/* MhZip .StorePath property constants */

#define ETCP_PATHS_NONE             0
#define ETCP_PATH_RECURSED          1
#define ETCP_PATH_SPECIFIEDRECURSED 2

/* MhZip .ZipFileDate property constants */
#define ETCP_ZIPDATE_CURRENT  0
#define ETCP_ZIPDATE_ORIGINAL 1
#define ETCP_ZIPDATE_LATEST   2

/* constants for error codes */

#define ETCP_ERR_NOT_ENOUGH_MEMORY     108       /* not enough memory */
#define ETCP_ERR_INVALID_HANDLE        106       /* invalid (file or archive) handle */
#define ETCP_ERR_ACCESS_DENIED         105       /* access denied (file error of some sort) */
#define ETCP_ERR_PATH_NOT_FOUND        103       /* path not found (or unable to create subdirectory) */
#define ETCP_ERR_FILE_NOT_FOUND        102       /* file not found */
#define ETCP_ERR_NO_PROBLEM              0       /* (no error) */
#define ETCP_ERR_UNKNOWN_ARCHIVE_TYPE   -1       /* not a .ZIP archive */
#define ETCP_ERR_INVALID_ARCHIVE_DIR    -2       /* error in archive directory */
#define ETCP_ERR_UNKNOWN_METHOD         -3       /* unknown compression method */
#define ETCP_ERR_USER_ABORT             -4       /* the user wants to cancel */
#define ETCP_ERR_NO_FILES_FOUND         -7       /* no (more) matching files */
#define ETCP_ERR_INCOMPLETE             -8       /* unable to complete request */
#define ETCP_ERR_INVALID_SWITCH         -9       /* invalid switch option */
#define ETCP_ERR_DISK_FULL             -10       /* write error (disk full?) */
#define ETCP_ERR_READ_PAST_END         -11       /* read error (read past end?) */
#define ETCP_ERR_INVALID_OFFSET        -12       /* invalid starting offset for EtUnZipPtr */
#define ETCP_ERR_OVERFLOW              -18       /* overflow (integer > 32767 or string too long) */
#define ETCP_ERR_NOTHING_TO_DO         -19       /* no action specified [might want to respond by popping up help] */
#define ETCP_ERR_SWITCH_CONFLICT       -20       /* conflicting switches were specified */
#define ETCP_ERR_TOO_MANY_FILES        -21       /* too many files to process (more than 65535 or too many for multi-vol dir) */
#define ETCP_ERR_INVALID_DATE          -22       /* invalid date specified */
#define ETCP_ERR_BAD_FILE_MODE         -23       /* invalid file mode specified */
#define ETCP_ERR_BAD_PASSWORD          -24       /* incorrect password */
#define ETCP_ERR_BAD_CRC               -25       /* computed CRC doesn't match recorded CRC */
#define ETCP_ERR_NOT_WITH_MV           -26       /* function not supported for multi-volume or alien archives */
#define ETCP_ERR_CANT_CONTINUE         -27       /* can't continue (.ZIP was modified between Et4ZipFindNext calls) */
#define ETCP_ERR_NOT_REMOVABLE         -28       /* selected multi-volume format requires removable disks for target files */



/* constants for .ZIP file open mode */

#define ETCP_OPEN_ANY        0    /* open for r/w; create if need be */
#define ETCP_OPEN_READONLY   1    /* open for read (only) */
#define ETCP_OPEN_READWRITE  2    /* open for read/write */
#define ETCP_OPEN_CREATE     3    /* create; open for read/write */



/* Constants for zip method specification. These are not the same as the numbers used in the .ZIP
 * itself-- the problem being that the .ZIP methods are partially given by numbers, and partially
 * by assorted flags, which is a nuisance to handle. Internally, we use these single well-defined
 * constants instead, and convert back and forth when dealing with the .ZIP file headers directly. */

#define ETCP_METHOD_NONE          0
#define ETCP_METHOD_DEFLATE0      1
#define ETCP_METHOD_DEFLATE1      2
#define ETCP_METHOD_DEFLATE2      3
#define ETCP_METHOD_DEFLATE3      4
#define ETCP_METHOD_SCRUNCH       5
#define ETCP_METHOD_IMPLODE       6
#define ETCP_METHOD_HUFFMAN       7
#define ETCP_METHOD_MASH          8
#define ETCP_METHOD_SHRINK        9



typedef struct tagCENTRALDIRREC
{
    DWORD cdrSignature;      /* signature/I.D. */
    USHORT cdrCreationOS;    /* PK version used to create archive */
    USHORT cdrExtractVer;    /* PK version needed to extract file */
    USHORT cdrGenPurFlag;    /* misc flags */
    short cdrMethod;         /* compression method */
    USHORT cdrFTime;         /* file time, in DOS format */
    USHORT cdrFDate;         /* file date, in DOS format */
    long cdrCRC32;           /* CRC of file */
    long cdrCompSize;        /* compressed size of file */
    long cdrUncompSize;      /* uncompressed size of file */
    USHORT cdrFileNameLen;   /* length of file name */
    USHORT cdrExtraFieldLen; /* length of extra field */
    USHORT cdrCommentLen;    /* length of file comment */
    USHORT cdrDiskNumStart;  /* starting disk number [0..n] */
    USHORT cdrIntFileAttr;   /* internal file attribute (.ZIP internal attr) */
    DWORD cdrExtFileAttr;    /* external file attribute (includes DOS attr) */
    long cdrLocalHeader;     /* file pointer to local header structure */
} CENTRALDIRREC, FAR * lpCENTRALDIRREC;

typedef struct tagENDOFCENTRALDIRREC
{
    DWORD ecdSignature;      /* signature/I.D. */
    USHORT ecdDiskNum;       /* current disk number [0..n] */
    USHORT ecdDiskWCD;       /* disk # with start of central dir */
    USHORT ecdNumThisDisk;   /* # of entries in central dir on this disk */
    USHORT ecdTotalCD;       /* total # of entries in central dir */
    long ecdCDSize;          /* size of central dir (bytes) */
    long ecdCDOffset;        /* offset of central dir on 1st disk with same */
    USHORT ecdCommentLen;    /* strlen of zip file comment */
} ENDOFCENTRALDIRREC, FAR * lpENDOFCENTRALDIRREC;

typedef struct tagCOMPINFO
{
   UCHAR ciZipFileName[ETCP_MAX_FILENAME_LENGTH];     /* name of .ZIP file being processed */
   UCHAR ciFileName[ETCP_MAX_FILENAME_LENGTH];        /* name of the file being processed (added to .ZIP, deleted from .ZIP, or whatever) */
   short ciOperation;        /* operation: 0 zip, 1 unzip, 2 delete, 3 test */
   long ciTotalSize;         /* total number of bytes to process */
   long ciCurrentByte;       /* current byte being processed */
   long ciCompSize;          /* compressed size */
   short ciMethod;           /* compression method */
   short ciPass;             /* pass number [1..n] */
   long ciPassSize;          /* total bytes this pass */
   short ciMessageType;      /* message type (status, error, query) */
   short ciMessageCode;      /* message number (if error message) */
   short ciDiskNumber;       /* disk number (for multi-volume query) */
} COMPINFO;

typedef struct tagMORECOMPINFO
{
   USHORT ciFDate;           // file date, in DOS format
   USHORT ciFTime;           // file time, in DOS format
   // There may be additional data added on to the end of this structure in the future...
} MORECOMPINFO, FAR * lpMORECOMPINFO;



/* prototypes */

short FAR PASCAL Et4AddUnZipPathFilter (LPSTR sOldPath, LPSTR sNewPath);

void FAR PASCAL Et4Cancel ();

void FAR PASCAL Et4CompressDosDate (short wMonth, short wDay, short wYear,
   USHORT FAR * wDate);

void FAR PASCAL Et4CompressDosTime (short wHour, short wMinute, short wSecond,
   USHORT FAR * wTime);

void FAR PASCAL Et4CPrintHook (FARPROC CPrintRoutine);

short FAR PASCAL Et4CrLfToLf (LPSTR sInFile, LPSTR sOutFile);

short FAR PASCAL Et4DirExist (LPSTR sDirName);

short FAR PASCAL Et4DosGetFileTime (LPSTR sFileName, USHORT FAR * lpwDate,
   USHORT FAR * lpwTime);

short FAR PASCAL Et4DosSetFileTime (LPSTR sFileName, USHORT wDate,
   USHORT wTime);

void FAR PASCAL Et4ExpandDosDate (USHORT wDate, short FAR * wMonth,
   short FAR * wDay, short FAR * wYear);

void FAR PASCAL Et4ExpandDosTime (USHORT wTime, short FAR * wHour,
   short FAR * wMinute, short FAR * wSecond);

short FAR PASCAL Et4ExtendedCPrintInfo (short wcbBuffer, void * lpBuffer);

short FAR PASCAL Et4GetCPrint ();

short FAR PASCAL Et4GetDoEvents ();

short FAR PASCAL Et4GetParsePlace ();

short FAR PASCAL Et4GetParseType ();

short FAR PASCAL Et4IdentifyFile (LPSTR sZipFile, short FAR * wArchiveType);

short FAR PASCAL Et4LfToCrLf (LPSTR sInFile, LPSTR sOutFile, short fCtrlZ);

short FAR PASCAL Et4MakeSFX (LPSTR sZipFile, LPSTR sSfxFile, short iOverwrite,
   short iEncryptedOk, short iDestPathDialog, short iStartDialog,
   short iEndDialog, LPSTR sAutoExecFile);

short FAR PASCAL Et4QUnZip (LPSTR sZipFile, LPSTR sDestination,
   LPSTR sFileSpec, LPSTR sSwitches);

short FAR PASCAL Et4QZip (LPSTR sZipFile, LPSTR sFileSpec, LPSTR sSwitches);

void FAR PASCAL Et4SetCPrint (short StatusDisplay);

void FAR PASCAL Et4SetDoEvents (short DoEvents);

void FAR PASCAL Et4SetParseType (short wChoice);

short FAR PASCAL Et4UnZip (HFILE FAR * hZipFile, LPSTR sDestination,
   LPSTR asFileSpec, LPSTR asExcludeSpec, LPSTR sSwitches);

short FAR PASCAL Et4UnZipPtr (HFILE hZipFile, UCHAR FAR * dPtr,
   long dStartingOffset, long dcbBytesWanted, LPSTR sInternalName,
   LPSTR sSwitches);

short FAR PASCAL Et4Zip (HFILE FAR * hZipFile, LPSTR asFileSpec,
   LPSTR asExcludeSpec, LPSTR sSwitches);

void FAR PASCAL Et4ZipClose (HFILE FAR * hZipFile);

short FAR PASCAL Et4ZipComment (HFILE hZipFile, LPSTR sComment,
   UINT wMaxCommentLen);

short FAR PASCAL Et4ZipCount (HFILE hZipFile, UINT FAR * wFileCount);

short FAR PASCAL Et4ZipCreateMV (LPSTR sZipFile, USHORT wZipType,
   DWORD dcbFirstDisk, DWORD dcbNextDisk, HFILE FAR * hZipFile);

short FAR PASCAL Et4ZipFindFirst (HFILE hZipFile, CENTRALDIRREC FAR * cdr,
   LPSTR sFile, LPSTR sExtraField, USHORT MaxExtraFieldLen, LPSTR sComment,
   USHORT MaxCommentLen);

short FAR PASCAL Et4ZipFindNext (HFILE hZipFile, CENTRALDIRREC FAR * cdr,
   LPSTR sFile, LPSTR sExtraField, USHORT MaxExtraFieldLen, LPSTR sComment,
   USHORT MaxCommentLen);

short FAR PASCAL Et4ZipNewComment (HFILE hZipFile, LPSTR sComment);

short FAR PASCAL Et4ZipNewFileComment (HFILE FAR * hZipFile, LPSTR sFileName,
   LPSTR sFileComment);

short FAR PASCAL Et4ZipOpen (LPSTR sZipFile, USHORT wFileMode,
   HFILE FAR * hZipFile);

short FAR PASCAL Et4ZipPercent (long dcbUncompressedSize,
   long dcbCompressedSize);

short FAR PASCAL Et4ZipPtr (HFILE FAR * hZipFile, UCHAR FAR * dPtr,
   long dcbBytesLeft, LPSTR sInternalName, LPSTR sSwitches);



/* prototypes for low-level compression routines */

long WINAPI EtCompress (UCHAR FAR * SrcPtr, long SrcLen,
    UCHAR FAR * DestPtr, long DestLen, int CompSpeed);

long WINAPI EtCRC32A (UCHAR FAR * SrcPtr, long Bytes, long StartingCRC);

void WINAPI EtDecryptData (void FAR * DataPtr, long DataSize);

short WINAPI EtDeflate (short MaxCompares, short StaticFlag,
   UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize, BOOL EndOfInput,
   UCHAR FAR * Ptr1, UCHAR FAR * Ptr2, UCHAR FAR * Ptr3);

void WINAPI EtDeflateBufSize (long FAR * BufSize1, long FAR * BufSize2,
   long FAR * BufSize3);

void WINAPI EtDeflateInit (UCHAR FAR * BufPtr1, UCHAR FAR * BufPtr2,
   UCHAR FAR * BufPtr3);

void WINAPI EtEncryptData (void FAR * DataPtr, long DataSize);

short WINAPI EtHuffman (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtHuffmanBufSize ();

void WINAPI EtHuffmanInit (UCHAR FAR * BufPtr);

short WINAPI EtHuffmanScan (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtHuffmanUCBufSize ();

short WINAPI EtHuffmanUC (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

void WINAPI EtHuffmanUCInit (UCHAR FAR * BufPtr);

short WINAPI EtImplode (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtImplodeBufSize ();

void WINAPI EtImplodeInit (UCHAR FAR * BufPtr);

short WINAPI EtImplodeScan (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

short WINAPI EtImplodeUC (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtImplodeUCBufSize ();

void WINAPI EtImplodeUCParams (short ThreeTrees, short EightKWindow);

void WINAPI EtImplodeUCInit (UCHAR FAR * BufPtr);

short WINAPI EtInflateUC (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtInflateUCBufSize ();

void WINAPI EtInflateUCInit (UCHAR FAR * BufPtr);

void WINAPI EtInitHeader (void FAR * HeaderPtr, long FileCRC32);

void WINAPI EtInitKeys (UCHAR FAR * PassPtr, short PassSize);

short WINAPI EtMash (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtMashBufSize ();

void WINAPI EtMashInit ();

void WINAPI EtMashParams ();

short WINAPI EtMashUC (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtMashUCBufSize ();

void WINAPI EtMashUCInit (UCHAR FAR * BufPtr);

void WINAPI EtMashUCParams (short MaxBits, short RleFirst);

short WINAPI EtScrunch (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtScrunchBufSize ();

void WINAPI EtScrunchInit (UCHAR FAR * BufPtr);

short WINAPI EtScrunchUC (UCHAR FAR * InBufPtr, long InStart,
   long FAR * InSize, UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtScrunchUCBufSize ();

void WINAPI EtScrunchUCInit (UCHAR FAR * BufPtr);

short WINAPI EtShrink (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtShrinkBufSize ();

void WINAPI EtShrinkInit (UCHAR FAR * BufPtr);

short WINAPI EtShrinkUC (UCHAR FAR * InBufPtr, long InStart, long FAR * InSize,
   UCHAR FAR * OutBufPtr, long OutStart, long FAR * OutSize,
   UCHAR FAR * IntVarPtr, BOOL EndOfInput);

long WINAPI EtShrinkUCBufSize ();

void WINAPI EtShrinkUCInit (UCHAR FAR * BufPtr);

long WINAPI EtUnCompress (UCHAR FAR * StrPtr, long SrcLen,
    UCHAR FAR * DestPtr, long DestLen);



#ifdef __cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
