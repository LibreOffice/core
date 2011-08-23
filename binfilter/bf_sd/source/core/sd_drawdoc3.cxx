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

#include <utility>
#include <algorithm>

#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif



#include <set>

#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "strmname.h"

#include "bf_sd/docshell.hxx"

namespace binfilter {

using namespace ::com::sun::star;

#define POOL_BUFFER_SIZE		(USHORT)32768
#define BASIC_BUFFER_SIZE		(USHORT)8192
#define DOCUMENT_BUFFER_SIZE	(USHORT)32768

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/


/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/


/*************************************************************************
|*
|* Fuegt ein Bookmark (Seite oder Objekt) ein
|*
\************************************************************************/


/*************************************************************************
|*
|* Fuegt ein Bookmark als Seite ein
|*
\************************************************************************/

/** Concrete incarnations get called by IterateBookmarkPages, for
    every page in the bookmark document/list
 */

/*************************************************************************
|*
|* Fuegt ein Bookmark als Objekt ein
|*
\************************************************************************/


/*************************************************************************
|*
|* Beendet das Einfuegen von Bookmarks
|*
\************************************************************************/

void SdDrawDocument::CloseBookmarkDoc()
{
    if (xBookmarkDocShRef.Is())
/*?*/ 	{
/*?*/ 		xBookmarkDocShRef->DoClose();
/*?*/ 	}

    xBookmarkDocShRef.Clear();
    aBookmarkFile = String();
}

/*************************************************************************
|*
|* Dokument laden (fuer gelinkte Objekte)
|*
\************************************************************************/


/*************************************************************************
|*
|* Dokument schliessen (fuer gelinkte Objekte)
|*
\************************************************************************/

void SdDrawDocument::DisposeLoadedModels()
{
    CloseBookmarkDoc();
}

/*************************************************************************
|*
|* Ist das Dokument read-only?
|*
\************************************************************************/



/*************************************************************************
|*
|* In anschliessendem AllocModel() wird eine DocShell erzeugt
|* (xAllocedDocShRef). Eine bereits bestehende DocShell wird ggf. geloescht
|*
\************************************************************************/

void SdDrawDocument::SetAllocDocSh(BOOL bAlloc)
{
    bAllocDocSh = bAlloc;

    if (xAllocedDocShRef.Is())
/*?*/ 	{
/*?*/ 		xAllocedDocShRef->DoClose();
/*?*/ 	}

    xAllocedDocShRef.Clear();
}

/*************************************************************************
|*
|* Liste der CustomShows zurueckgeben (ggf. zuerst erzeugen)
|*
\************************************************************************/

List* SdDrawDocument::GetCustomShowList(BOOL bCreate)
{
    if (!pCustomShowList && bCreate)
    {
        // Liste erzeugen
        pCustomShowList = new List();
    }

    return(pCustomShowList);
}

/*************************************************************************
|*
|* Document-Stream herausgeben (fuer load-on-demand Graphiken)
|*
\************************************************************************/

SvStream* SdDrawDocument::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
{
    SotStorage*	pStor = pDocSh ? pDocSh->GetMedium()->GetStorage() : NULL;
    SvStream*	pRet = NULL;

    if( pStor )
    {
        if( rStreamInfo.maUserData.Len() &&
            ( rStreamInfo.maUserData.GetToken( 0, ':' ) ==
              String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
        {
            const String aPicturePath( rStreamInfo.maUserData.GetToken( 1, ':' ) );

            // graphic from picture stream in picture storage in XML package
            if( aPicturePath.GetTokenCount( '/' ) == 2 )
            {
                const String aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );

                if( !xPictureStorage.Is() )
                {
                    const String aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );

                    if( pStor->IsContained( aPictureStorageName ) &&
                        pStor->IsStorage( aPictureStorageName )  )
                    {
                        // cast away const
                        ((SdDrawDocument*)this)->xPictureStorage = pStor->OpenUCBStorage( aPictureStorageName, STREAM_READ );
                    }
                }

                if( xPictureStorage.Is() &&
                    xPictureStorage->IsContained( aPictureStreamName ) &&
                    xPictureStorage->IsStream( aPictureStreamName ) )
                {
                    pRet = xPictureStorage->OpenSotStream( aPictureStreamName, STREAM_READ );

                    if( pRet )
                    {
                        pRet->SetVersion( xPictureStorage->GetVersion() );
                        pRet->SetKey( xPictureStorage->GetKey() );
                    }
                }
            }

            rStreamInfo.mbDeleteAfterUse = ( pRet != NULL );
        }
        else
        {
            // graphic from plain binary document stream
            if( !pDocStor )
            {
                if( pStor->IsStream( pStarDrawDoc ) )
                {
                    BOOL bOK = pStor->Rename(pStarDrawDoc, pStarDrawDoc3);
                    DBG_ASSERT(bOK, "Umbenennung des Streams gescheitert");
                }

                SotStorageStreamRef docStream = pStor->OpenSotStream( pStarDrawDoc3, STREAM_READ );
                docStream->SetVersion( pStor->GetVersion() );
                docStream->SetKey( pStor->GetKey() );

                // cast away const (should be regarded logical constness)
                ((SdDrawDocument*)this)->xDocStream = docStream;
                ((SdDrawDocument*)this)->pDocStor = pStor;
            }

            pRet = xDocStream;
            rStreamInfo.mbDeleteAfterUse = FALSE;
        }
    }

#if OSL_DEBUG_LEVEL > 1
    if( pRet )
    {
        // try to get some information from stream
        const ULONG nStartPos = pRet->Tell();
        const ULONG nEndPos = pRet->Seek( STREAM_SEEK_TO_END );
        const ULONG nStmLen = nEndPos - nStartPos;
        sal_uChar	aTestByte;

        // try to read one byte
        if( nStmLen )
            *pRet >> aTestByte;

        pRet->Seek( nStartPos );
    }
#endif

    return pRet;
}


/*************************************************************************
|*
|* Release doc stream, if no longer valid
|*
\************************************************************************/

void SdDrawDocument::HandsOff()
{
    xPictureStorage = SotStorageRef();
    pDocStor = NULL;
}

void SdDrawDocument::RemoveDuplicateMasterPages()
{
    USHORT nSdMasterPageCount = GetMasterSdPageCount( PK_STANDARD );
    for (sal_Int32 nMPage = nSdMasterPageCount - 1; nMPage >= 0; nMPage--)
    {
        SdPage* pMaster = (SdPage*) GetMasterSdPage( (USHORT) nMPage, PK_STANDARD );
        SdPage* pNotesMaster = (SdPage*) GetMasterSdPage( (USHORT) nMPage, PK_NOTES );

        DBG_ASSERT( pMaster->GetPageKind() == PK_STANDARD, "wrong page kind" );

        if ( pMaster->GetPageKind() == PK_STANDARD && GetMasterPageUserCount( pMaster ) == 0 && pNotesMaster )
        {
            const String aLayoutName( pMaster->GetLayoutName() );
            for (USHORT i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
            {
                SdPage* pMPg = (SdPage*) GetMasterSdPage( i, PK_STANDARD );
                if( pMPg != pMaster && pMPg->GetLayoutName() == aLayoutName )
                {
                    RemoveMasterPage( pNotesMaster->GetPageNum() );
                    RemoveMasterPage( pMaster->GetPageNum() );
                    break;
                }
            }
        }
    }
}

}
