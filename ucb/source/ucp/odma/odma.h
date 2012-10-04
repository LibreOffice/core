/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* odma.h - Definitions, prototypes, etc. for Open Document Managment API
    (ODMA) version 2.0.
               OPEN DOCUMENT MANAGEMENT API LICENSE 1.0

                 ODMA 2.0 SPECIFICATIONS AND SOFTWARE
                 ------------------------------------

               Copyright © 1994-1998 AIIM International

LICENSE:

    Redistribution and use in source and binary forms, with or
    without modifications, are permitted provided that the
    following conditions are met:

      * Redistributions of source code must retain the above
        copyright notice, this list of conditions and the
        following disclaimer.

      * Redistributions in binary form must reproduce the
        above copyright notice, this list of conditions and
        the following disclaimer in the documentation and/or
        other materials provided with the distribution.

      * Neither the name of AIIM International nor the names
        of its contributors may be used to endorse or promote
        products derived from this software without specific
        prior written permission.

DISCLAIMER:

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    COPYRIGHT (C) 1994, 1995
    AIIM International
    All Right Reserved
*/

#ifndef ODMA_H
#define ODMA_H

/* Type definitions */
typedef LPVOID ODMHANDLE;
typedef LPSTR (*ODMSAVEASCALLBACK)(DWORD dwEnvData, LPSTR lpszFormat, LPVOID pInstanceData);
typedef int ODMSTATUS;

/* Constants */
#define ODM_API_VERSION            200        /* Version of the API */

#ifdef WIN32
    #define ODM_DOCID_MAX        255        /* Win32 */

#elif defined( _WINDOWS_ ) || defined( _MSDOS )
    #define ODM_DOCID_MAX        80        /* Windows 3.x */

#else
    #define ODM_DOCID_MAX        255        /* Other platforms */
#endif

#define ODM_DMSID_MAX            9        /* Max length of a DMS ID including the
                               terminating NULL character.  */

#define ODM_APPID_MAX            16        /* Max length of a application ID including
                               the terminating NULL character.  */

// ODMA 2.0
#define ODM_QUERYID_MAX            255        // Max length of a query ID including
                                                    // the terminating NULL character.

#define ODM_FORMAT_MAX            81 // Max length of a format including
                                                    // the terminating NULL character.

// Maximum length of a filename returned by ODMA including the terminating
//    NULL character.  Platform dependent.

#ifdef WIN32
    #define ODM_FILENAME_MAX    255        /* Win32 */

#elif defined( _WINDOWS_ ) || defined( _MSDOS )
    #define ODM_FILENAME_MAX    128        /* Windows 3.x */

#elif defined( unix ) || defined( _UNIX )
    #define ODM_FILENAME_MAX    1024    /* Unix */

#else
    #define ODM_FILENAME_MAX    255        /* Other platforms */
#endif


/* Common format type names */
#define ODM_FORMAT_TEXT    "Text"
#define ODM_FORMAT_RTF    "Rich text format"
#define ODM_FORMAT_DCA    "DCA RFT"        /* IBM DCA Rich Format Text */
#define ODM_FORMAT_TIFF    "Tiff"
#define ODM_FORMAT_GIF    "Gif"                /* Compuserve Graphics Interchange Format */
#define ODM_FORMAT_BMP    "Windows bitmap"
#define ODM_FORMAT_PCX    "PCX"
#define ODM_FORMAT_CGM    "CGM"                /* Computer Graphics Metafile */
#define ODM_FORMAT_EXE    "Executable file"
#define ODM_FORMAT_PCL    "PCL"                /* HP Printer Control Language */
#define ODM_FORMAT_PS    "PostScript"


/* Error returns */
#define ODM_SUCCESS            0        // Success!
#define ODM_E_FAIL            1        /* Unspecified failure */
#define ODM_E_CANCEL        2        /* Action was cancelled at user's request */
#define ODM_E_NODMS            3        /* DMS not registered */
#define ODM_E_CANTINIT        4        /* DMS failed to initalize */
#define ODM_E_VERSION        5        /* DMS doesn't support the requested
                                               version of ODMA */
#define ODM_E_APPSELECT        6        /* User has indicated that he wants to use
                                       the application's file selection
                                       capabilities rather than those of the
                                       DMS. */
#define ODM_E_USERINT        7        /* Requested action cannot be performed
                                       without user interaction, but silent
                                       mode was specified. */
#define ODM_E_HANDLE        8        /* The DMHANDLE argument was invalid. */
#define ODM_E_ACCESS        9        /* User does not have requested access
                                       rights to specified document. */
#define ODM_E_INUSE            10        /* Document is currently in use and cannot
                                       be accessed in specified mode. */
#define ODM_E_DOCID            11        /* Invalid document ID */
#define ODM_E_OPENMODE        12        /* The specified action is incompatible
                                       with the mode in which the document was
                                       opened. */
#define ODM_E_NOOPEN        13        /* The specified document is not open. */
#define ODM_E_ITEM            14        /* Invalid item specifier. */
#define ODM_E_OTHERAPP        15        /* Selected document was for another app. */
#define ODM_E_NOMOREDATA    16        /* No more data is available */
#define ODM_E_PARTIALSUCCESS 17        /* */
// Additional Error code from ODMA 2.0
#define ODM_E_REQARG        18        /* */
#define ODM_E_NOSUPPORT        19        /* */
#define ODM_E_TRUNCATED        20        /* */
#define ODM_E_INVARG  21
#define ODM_E_OFFLINE  22        /* */


// ODMOpenDoc modes
#define ODM_MODIFYMODE        1        /* Open document in a modifiable mode. */
#define ODM_VIEWMODE        2        /* Open document in non-modifiable mode. */
// ODMA 2.0
#define ODM_REFCOPY            3


// Actions for ODMActivate
#define ODM_NONE              0        /* No specific action is requested.  */
#define ODM_DELETE            1        /* Delete the specified document.  */
#define ODM_SHOWATTRIBUTES    2        /* Display the specified document's profile
                                          or attributes. */
#define ODM_EDITATTRIBUTES    3        /* Edit the specified document's profile or
                                          attributes. */
#define ODM_VIEWDOC           4        /* Display the specified document in a
                                          viewer window. */
#define ODM_OPENDOC           5        /* Open the specified document in its
                                          native application.  */
// ODMA 2.0
#define ODM_NEWDOC            6
#define ODM_CHECKOUT          7
#define ODM_CANCELCHECKOUT    8
#define ODM_CHECKIN           9
#define ODM_SHOWHISTORY      10


// Item selectors for ODMGetDocInfo and ODMSetDocInfo
#define ODM_AUTHOR            1         /* Author of the document. */
#define ODM_NAME            2           /* Descriptive name of the document.  */
#define ODM_TYPE            3           /* Type of the document.  */
#define ODM_TITLETEXT        4          /* Suggested text to display in the
                                           document window's title bar. */
#define ODM_DMS_DEFINED        5        /* DMS defined data. */
#define ODM_CONTENTFORMAT    6          /* String describing document's format */
// ODMA 2.0
#define ODM_ALTERNATE_RENDERINGS    7
#define ODM_CHECKEDOUTBY    8
#define ODM_CHECKOUTCOMMENT    9
#define ODM_CHECKOUTDATE    10
#define ODM_CREATEDBY        11
#define ODM_CREATEDDATE        12
#define ODM_DOCID_LATEST    13
#define ODM_DOCID_RELEASED    14
#define ODM_DOCVERSION        15
#define ODM_DOCVERSION_LATEST    16
#define ODM_DOCVERSION_RELEASED    17
#define ODM_LOCATION        18
#define ODM_KEYWORDS        19
#define ODM_LASTCHECKINBY    20
#define ODM_LASTCHECKINDATE    21
#define ODM_MODIFYDATE        22
#define ODM_MODIFYDATE_LATEST    23
#define ODM_MODIFYDATE_RELEASED    24
#define ODM_OWNER            25
#define ODM_SUBJECT            26
#define ODM_TITLETEXT_RO    27
#define ODM_URL                28


// Item selectors for ODMQueryCapability ODMA 2.0
#define ODM_QC_ACTIVATE                1
#define ODM_QC_CLOSEDOC                2
#define ODM_QC_CLOSEDOCEX            3
#define ODM_QC_GETALTERNATECONTENT    4
#define ODM_QC_GETDMSINFO            5
#define ODM_QC_GETDOCINFO            6
#define ODM_QC_GETDOCRELATION        7
#define ODM_QC_GETLEADMONIKER        8
#define ODM_QC_NEWDOC                9
#define ODM_QC_OPENDOC                10
#define ODM_QC_QUERYCLOSE            11
#define ODM_QC_QUERYEXECUTE            12
#define ODM_QC_QUERYGETRESULTS        13
#define ODM_QC_SAVEAS                14
#define ODM_QC_SAVEASEX                15
#define ODM_QC_SAVEDOC                16
#define ODM_QC_SAVEDOCEX            17
#define ODM_QC_SELECTDOC            18
#define ODM_QC_SELECTDOCEX            19
#define ODM_QC_SETALTERNATECONTENT    20
#define ODM_QC_SETDOCEVENT            21
#define ODM_QC_SETDOCRELATION        22
#define ODM_QC_SETDOCINFO            23


// Misc. modes, flags
#define ODM_SILENT            16        /* Don't interact with the user while
                                           fulfilling this request. */
//ODMA 2.0
#define ODM_VERSION_SAME       1
#define ODM_VERSION_MAJOR      2
#define ODM_VERSION_MINOR      4
#define ODM_VERSION_CHANGED    8
#define ODM_ALT_DELETE        32

//ODMA 2.0 DMS Info Flags
#define ODM_EXT_QUERY        1
#define ODM_EXT_WORKFLOW     2

// Flags for Query Interface
#define ODM_ALL              1       // All DMS's should be searched
#define ODM_SPECIFIC        2        // Only specific DMS's should be searched


// Function prototypes
#ifdef __cplusplus
extern "C" {
#endif

ODMSTATUS WINAPI ODMRegisterApp(ODMHANDLE FAR *pOdmHandle, WORD version,
    LPSTR lpszAppId, DWORD dwEnvData, LPVOID pReserved);

void WINAPI ODMUnRegisterApp(ODMHANDLE odmHandle);

ODMSTATUS WINAPI ODMSelectDoc(ODMHANDLE odmHandle, LPSTR lpszDocId,
    LPDWORD pdwFlags);

ODMSTATUS WINAPI ODMOpenDoc(ODMHANDLE odmHandle, DWORD flags,
    LPSTR lpszDocId, LPSTR lpszDocLocation);

ODMSTATUS WINAPI ODMSaveDoc(ODMHANDLE odmHandle, LPSTR lpszDocId,
    LPSTR lpszNewDocId);

ODMSTATUS WINAPI ODMCloseDoc(ODMHANDLE odmHandle, LPSTR lpszDocId,
    DWORD activeTime, DWORD pagesPrinted, LPVOID sessionData, WORD dataLen);

ODMSTATUS WINAPI ODMNewDoc(ODMHANDLE odmHandle, LPSTR lpszDocId,
    DWORD dwFlags, LPSTR lpszFormat, LPSTR lpszDocLocation);

ODMSTATUS WINAPI ODMSaveAs(ODMHANDLE odmHandle, LPSTR lpszDocId,
    LPSTR lpszNewDocId, LPSTR lpszFormat, ODMSAVEASCALLBACK pcbCallBack,
    LPVOID pInstanceData);

ODMSTATUS WINAPI ODMActivate(ODMHANDLE odmHandle, WORD action,
    LPSTR lpszDocId);

ODMSTATUS WINAPI ODMGetDocInfo(ODMHANDLE odmHandle, LPSTR lpszDocId,
    WORD item, LPSTR lpszData, WORD dataLen);

ODMSTATUS WINAPI ODMSetDocInfo(ODMHANDLE odmHandle, LPSTR lpszDocId,
    WORD item, LPSTR lpszData);

ODMSTATUS WINAPI ODMGetDMSInfo(ODMHANDLE odmHandle, LPSTR lpszDmsId,
    LPWORD pwVerNo, LPDWORD pdwExtensions);

/* Query Enhancements */
WORD WINAPI ODMGetDMSCount();

WORD WINAPI ODMGetDMSList( LPSTR buffer, WORD buffer_size );

ODMSTATUS WINAPI ODMGetDMS( LPCSTR lpszAppId, LPSTR lpszDMSId );

ODMSTATUS WINAPI ODMSetDMS( LPCSTR lpszAppId, LPCSTR lpszDMSId );

ODMSTATUS WINAPI ODMQueryExecute(ODMHANDLE odmHandle, LPCSTR lpszQuery,
                                 DWORD flags, LPCSTR lpszDMSList, LPSTR queryId );

ODMSTATUS WINAPI ODMQueryGetResults(ODMHANDLE odmHandle, LPCSTR queryId,
                                    LPSTR lpszDocId, LPSTR lpszDocName, WORD docNameLen,
                                    WORD *docCount );

ODMSTATUS WINAPI ODMQueryClose(ODMHANDLE odmHandle, LPCSTR queryId );

/* ODMA 2.0 Enhancements */
ODMSTATUS WINAPI ODMCloseDocEx(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPDWORD pdwFlags, DWORD activeTime, DWORD pagesPrinted,
                            LPVOID sessionData, WORD dataLen);

ODMSTATUS WINAPI ODMSaveAsEx(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPSTR lpszNewDocId, LPSTR lpszFormat, ODMSAVEASCALLBACK pcbCallBack,
                            LPVOID pInstanceData, LPDWORD pdwFlags);

ODMSTATUS WINAPI ODMSaveDocEx(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPSTR lpszNewDocId,    LPDWORD pdwFlags);

ODMSTATUS WINAPI ODMSelectDocEx(ODMHANDLE odmHandle, LPSTR lpszDocIds,
                            LPWORD pwDocIdsLen, LPWORD pwDocCount, LPDWORD pdwFlags,
                            LPSTR lpszFormatFilter);

ODMSTATUS WINAPI ODMQueryCapability(ODMHANDLE odmHandle, LPCSTR lpszDmsId,
                            DWORD function, DWORD item, DWORD flags);

ODMSTATUS WINAPI ODMSetDocEvent(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            DWORD flags, DWORD event, LPVOID lpData, DWORD dwDataLen,
                            LPSTR lpszComment);

ODMSTATUS WINAPI ODMGetAlternateContent(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPDWORD pdwFlags, LPSTR lpszFormat, LPSTR lpszDocLocation);

ODMSTATUS WINAPI ODMSetAlternateContent(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPDWORD pdwFlags, LPSTR lpszFormat, LPSTR lpszDocLocation);

ODMSTATUS WINAPI ODMGetDocRelation(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPDWORD pdwFlags, LPSTR lpszLinkedId, LPSTR lpszFormat,
                            LPSTR lpszPreviousId);

ODMSTATUS WINAPI ODMSetDocRelation(ODMHANDLE odmHandle, LPSTR lpszDocId,
                            LPDWORD pdwFlags, LPSTR lpszLinkedId, LPSTR lpszFormat,
                            LPSTR lpszPreviousId);

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
