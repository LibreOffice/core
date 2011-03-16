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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <unotools/tempfile.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <fmtinfmt.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <globdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <section.hxx>
#include <doctxm.hxx>
#include <poolfmt.hxx>
#include <switerator.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

using namespace ::com::sun::star;

enum SwSplitDocType
{
    SPLITDOC_TO_GLOBALDOC,
    SPLITDOC_TO_HTML
};

sal_Bool SwDoc::GenerateGlobalDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl )
{
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, pSplitColl );
}

//#outline level,add by zhaojianwei
sal_Bool SwDoc::GenerateGlobalDoc( const String& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, nOutlineLevel );
}
sal_Bool SwDoc::GenerateHTMLDoc( const String& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_HTML, rPath, nOutlineLevel );
}
//<-end,zhaojianwei

sal_Bool SwDoc::GenerateHTMLDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl )
{
#ifdef JP_TEST
    if( !pSplitColl )
    {
        sal_uInt8 nLvl = 1;
        const SwTxtFmtColls& rFmtColls =*GetTxtFmtColls();
        for( sal_uInt16 n = rFmtColls.Count(); n; )
            //if( nLvl == rFmtColls[ --n ]->GetOutlineLevel() )//#outline level,zhaojianwei
            if( nLvl == rFmtColls[ --n ]->GetAttrOutlineLevel() -1 )//<-end,zhaojianwei 0814
            {
                pSplitColl = rFmtColls[ n ];
                break;
            }

        if( !pSplitColl )
            pSplitColl = GetTxtCollFromPool( RES_POOLCOLL_HEADLINE2 );
    }
#endif

    return SplitDoc( SPLITDOC_TO_HTML, rPath, pSplitColl );
}

sal_Bool SwDoc::SplitDoc( sal_uInt16 eDocType, const String& rPath,
                        const SwTxtFmtColl* pSplitColl )
{
    // ueber alle Node der Vorlage Iterieren und dafuer einzelne
    // Dokumente erzeugen und in diesem gegen
    // - gelinkte Bereiche (GlobalDoc)
    // - Links (HTML)
    // austauschen.
    // Am Ende wird dieses Doc als GlobalDoc/HTML-Doc gespreichert.
    if( !pDocShell || !pDocShell->GetMedium() ||
        ( SPLITDOC_TO_GLOBALDOC == eDocType && get(IDocumentSettingAccess::GLOBAL_DOCUMENT) ) )
        return sal_False;

    sal_uInt16 nOutl = 0;
    SwOutlineNodes* pOutlNds = (SwOutlineNodes*)&GetNodes().GetOutLineNds();
    SwNodePtr pSttNd;

    if( pSplitColl )
    {
        // wenn keine OutlineNumerierung ist, dann benutze eigenes Array
        // und sammel die Nodes zusammen.
        //if( NO_NUMBERING == pSplitColl->GetOutlineLevel() )//#outline level,zhaojianwei
        if( pSplitColl->GetAttrOutlineLevel() == 0 )//<-end,zhaojianwei, 0814
        {
            pOutlNds = new SwOutlineNodes( 8, 8 );
            SwIterator<SwTxtNode,SwFmtColl> aIter( *pSplitColl );
            for( SwTxtNode* pTNd = aIter.First(); pTNd; pTNd = aIter.Next() )
                if( pTNd->GetNodes().IsDocNodes() )
                    pOutlNds->Insert( pTNd );

            if( !pOutlNds->Count() )
            {
                delete pOutlNds;
                return sal_False;
            }
        }
    }
    else
    {
        // dann suche die Gliederungs - Vorlage, der 1. Ebene
        const SwTxtFmtColls& rFmtColls =*GetTxtFmtColls();
        for( sal_uInt16 n = rFmtColls.Count(); n; )
            //if( !rFmtColls[ --n ]->GetOutlineLevel() )//#outline level,zhaojianwei
            if ( rFmtColls[ --n ]->GetAttrOutlineLevel() == 1 )//<-end,zhaojianwei
            {
                pSplitColl = rFmtColls[ n ];
                break;
            }

        if( !pSplitColl )
            return sal_False;
    }

    const SfxFilter* pFilter;
    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        pFilter = SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "HTML" )));
        break;

    default:
//  case SPLITDOC_TO_GLOBALDOC:
        pFilter = SwIoSystem::GetFilterOfFormat(
                                    String::CreateFromAscii( FILTER_XML ));
        eDocType = SPLITDOC_TO_GLOBALDOC;
        break;
    }

    if( !pFilter )
        return sal_False;

    // Undo/Redline aufjedenfall abschalten
    GetIDocumentUndoRedo().DoUndo(false);
    SetRedlineMode_intern( (RedlineMode_t)(GetRedlineMode() & ~nsRedlineMode_t::REDLINE_ON));

    String sExt( pFilter->GetSuffixes().GetToken(0, ',') );
    if( !sExt.Len() )
        sExt.AssignAscii( "sxw" );
    if( '.' != sExt.GetChar( 0 ) )
        sExt.Insert( '.', 0 );

    INetURLObject aEntry(rPath);
    String sLeading(aEntry.GetBase());
    aEntry.removeSegment();
    String sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
    utl::TempFile aTemp(sLeading,&sExt,&sPath );
    aTemp.EnableKillingFile();

    DateTime aTmplDate;
    {
        Time a2Min( 0 ); a2Min.SetMin( 2 );
        aTmplDate += a2Min;
    }


    // alle Ungueltigen ueberspringen
    while( nOutl < pOutlNds->Count() &&
        pOutlNds->GetObject( nOutl )->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
        ++nOutl;

    do {
        pSttNd = 0;

        SwNodePtr pNd;
        for( ; nOutl < pOutlNds->Count(); ++nOutl )
            if( ( pNd = pOutlNds->GetObject( nOutl ))->GetTxtNode()->
                    GetTxtColl() == pSplitColl &&
                !pNd->FindTableNode() )
            {
                pSttNd = pNd;
                break;
            }

        if( pSttNd )
        {
            SwNodePtr pEndNd = 0;
            for( ++nOutl; nOutl < pOutlNds->Count(); ++nOutl )
            {
                pNd = pOutlNds->GetObject( nOutl );
                SwTxtFmtColl* pTColl = pNd->GetTxtNode()->GetTxtColl();

                //if( ( pTColl == pSplitColl ||     //#outline level,zhaojianwei
                //  (   NO_NUMBERING != pSplitColl->GetOutlineLevel() &&
                //      pTColl->GetOutlineLevel() <
                //      pSplitColl->GetOutlineLevel() )) &&
                //  !pNd->FindTableNode() )
                if( ( pTColl == pSplitColl ||
                    (   pSplitColl->GetAttrOutlineLevel() > 0 &&
                        pTColl->GetAttrOutlineLevel() > 0     &&
                        pTColl->GetAttrOutlineLevel() <
                        pSplitColl->GetAttrOutlineLevel() )) &&
                    !pNd->FindTableNode() )         //<-end,zhaojianwei
                {
                    pEndNd = pNd;

                    break;
                }
            }
            SwNodeIndex aEndIdx( pEndNd ? *pEndNd
                                        : GetNodes().GetEndOfContent() );

            // die Nodes komplett rausschreiben
            String sFileName;
            if( pSttNd->GetIndex() + 1 < aEndIdx.GetIndex() )
            {
                SfxObjectShellLock xDocSh( new SwDocShell( SFX_CREATE_MODE_INTERNAL ));
                if( xDocSh->DoInitNew( 0 ) )
                {
                    SwDoc* pDoc = ((SwDocShell*)(&xDocSh))->GetDoc();

                    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                        ((SwDocShell*)(&xDocSh))->GetModel(),
                        uno::UNO_QUERY_THROW);
                    uno::Reference<document::XDocumentProperties> xDocProps(
                        xDPS->getDocumentProperties());
                    DBG_ASSERT(xDocProps.is(), "Doc has no DocumentProperties");
                    // the GlobalDoc is the template
                    xDocProps->setTemplateName(aEmptyStr);
                    ::util::DateTime uDT(aTmplDate.Get100Sec(),
                        aTmplDate.GetSec(), aTmplDate.GetMin(),
                        aTmplDate.GetHour(), aTmplDate.GetDay(),
                        aTmplDate.GetMonth(), aTmplDate.GetYear());
                    xDocProps->setTemplateDate(uDT);
                    xDocProps->setTemplateURL(rPath);
                    //JP 14.06.99: Set the text of the "split para" as title
                    //              from the new doc. Is the current doc has
                    //              a title, insert it at begin.
                    String sTitle( xDocProps->getTitle() );
                    if( sTitle.Len() )
                        sTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ));
                    sTitle += ((SwTxtNode*)pSttNd)->GetExpandTxt();
                    xDocProps->setTitle( sTitle );

                    // Vorlagen ersetzen
                    pDoc->ReplaceStyles( *this );

                    // KapitelNumerierung uebernehmen
                    if( pOutlineRule )
                        pDoc->SetOutlineNumRule( *pOutlineRule );

                    SwNodeRange aRg( *pSttNd, 0, aEndIdx.GetNode() );
                    SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
                    GetNodes()._Copy( aRg, aTmpIdx, sal_False );

                    // den initialen TextNode loeschen
                    SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
                    if( aIdx.GetIndex() + 1 !=
                        pDoc->GetNodes().GetEndOfContent().GetIndex() )
                        pDoc->GetNodes().Delete( aIdx, 1 );

                    // alle Flys in dem Bereich
                    CopyFlyInFlyImpl( aRg, 0, aIdx );


                    // und noch alle Bookmarks
                    // ?????

                    utl::TempFile aTempFile2(sLeading,&sExt,&sPath );
                    sFileName = aTempFile2.GetURL();
                    SfxMedium* pTmpMed = new SfxMedium( sFileName,
                                                STREAM_STD_READWRITE, sal_True );
                    pTmpMed->SetFilter( pFilter );

                    // fuer den HTML-Filter mussen wir aber ein Layout
                    // haben, damit Textrahmen/Controls/OLE-Objecte korrekt
                    // als Grafik exportiert werden koennen.
                    if( SPLITDOC_TO_HTML == eDocType &&
                        pDoc->GetSpzFrmFmts()->Count() )
                    {
                        /* SfxViewFrame* pFrame = */
                            SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
                    }
                    xDocSh->DoSaveAs( *pTmpMed );
                    xDocSh->DoSaveCompleted( pTmpMed );

                    // beim Fehler wird keine FileLinkSection eingefuegt
                    if( xDocSh->GetError() )
                        sFileName.Erase();
                }
                xDocSh->DoClose();
            }

            // dann koennen ja die Bereiche eingefuegt werden
            if( sFileName.Len() )
            {
                switch( eDocType )
                {
                case SPLITDOC_TO_HTML:
                    {
                        // loesche alle Nodes im Bereich und setze im "Start-
                        // Node" den Link auf das gespeicherte Doc
                        sal_uLong nNodeDiff = aEndIdx.GetIndex() -
                                            pSttNd->GetIndex() - 1;
                        if( nNodeDiff )
                        {
                            SwPaM aTmp( *pSttNd, aEndIdx.GetNode(), 1, -1 );
                            aTmp.GetPoint()->nContent.Assign( 0, 0 );
                            aTmp.GetMark()->nContent.Assign( 0, 0 );
                            SwNodeIndex aSIdx( aTmp.GetMark()->nNode );
                            SwNodeIndex aEIdx( aTmp.GetPoint()->nNode );

                            // versuche hinters Ende zu verschieben
                            if( !aTmp.Move( fnMoveForward, fnGoNode ) )
                            {
                                // na gut, dann an den Anfang
                                aTmp.Exchange();
                                if( !aTmp.Move( fnMoveBackward, fnGoNode ))
                                {
                                    ASSERT( sal_False, "kein Node mehr vorhanden" );
                                }
                            }
                                // Bookmarks usw. verschieben
                            CorrAbs( aSIdx, aEIdx, *aTmp.GetPoint(), sal_True);

                            // stehen noch FlyFrames rum, loesche auch diese
                            for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
                            {
                                SwFrmFmt* pFly = (*GetSpzFrmFmts())[n];
                                const SwFmtAnchor* pAnchor = &pFly->GetAnchor();
                                SwPosition const*const pAPos =
                                    pAnchor->GetCntntAnchor();
                                if (pAPos &&
                                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                                    aSIdx <= pAPos->nNode &&
                                    pAPos->nNode < aEIdx )
                                {
                                    DelLayoutFmt( pFly );
                                    --n;
                                }
                            }

                            GetNodes().Delete( aSIdx, nNodeDiff );
                        }

                        // dann setze im StartNode noch den Link:
                        SwFmtINetFmt aINet( sFileName , aEmptyStr );
                        SwTxtNode* pTNd = (SwTxtNode*)pSttNd;
                        pTNd->InsertItem( aINet, 0, pTNd->GetTxt().Len() );

                        // wenn der nicht mehr gefunden wird, kann das nur
                        // ein Bug sein!
                        if( !pOutlNds->Seek_Entry( pSttNd, &nOutl ))
                            pSttNd = 0;
                        ++nOutl;
                    }
                    break;

                default:
                    {
                        String sNm( INetURLObject( sFileName ).GetName() );
                        SwSectionData aSectData( FILE_LINK_SECTION,
                                        GetUniqueSectionName( &sNm ));
                        SwSectionFmt* pFmt = MakeSectionFmt( 0 );
                        aSectData.SetLinkFileName(sFileName);
                        aSectData.SetProtectFlag(true);

                        aEndIdx--;  // im InsertSection ist Ende inclusive
                        while( aEndIdx.GetNode().IsStartNode() )
                            aEndIdx--;

                        // JP 06.07.99 - Bug 67361 - is any Section ends or
                        // starts in the new sectionrange, they must end or
                        // start before or behind the range!
                        SwSectionNode* pSectNd = pSttNd->FindSectionNode();
                        while( pSectNd && pSectNd->EndOfSectionIndex()
                                <= aEndIdx.GetIndex() )
                        {
                            const SwNode* pSectEnd = pSectNd->EndOfSectionNode();
                            if( pSectNd->GetIndex() + 1 ==
                                    pSttNd->GetIndex() )
                            {
                                sal_Bool bMvIdx = aEndIdx == *pSectEnd;
                                DelSectionFmt( pSectNd->GetSection().GetFmt() );
                                if( bMvIdx )
                                    aEndIdx--;
                            }
                            else
                            {
                                SwNodeRange aRg( *pSttNd, *pSectEnd );
                                SwNodeIndex aIdx( *pSectEnd, 1 );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }
                            pSectNd = pSttNd->FindSectionNode();
                        }

                        pSectNd = aEndIdx.GetNode().FindSectionNode();
                        while( pSectNd && pSectNd->GetIndex() >
                                pSttNd->GetIndex() )
                        {
                            // #i15712# don't attempt to split sections if
                            // they are fully enclosed in [pSectNd,aEndIdx].
                            if( aEndIdx < pSectNd->EndOfSectionIndex() )
                            {
                                SwNodeRange aRg( *pSectNd, 1, aEndIdx, 1 );
                                SwNodeIndex aIdx( *pSectNd );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }

                            pSectNd = pSttNd->FindSectionNode();
                        }

                        // -> #i26762#
                        // Ensure order of start and end of section is sane.
                        SwNodeIndex aStartIdx(*pSttNd);

                        if (aEndIdx >= aStartIdx)
                        {
                            pSectNd = GetNodes().InsertTextSection(aStartIdx,
                                *pFmt, aSectData, 0, &aEndIdx, false);
                        }
                        else
                        {
                            pSectNd = GetNodes().InsertTextSection(aEndIdx,
                                *pFmt, aSectData, 0, &aStartIdx, false);
                        }
                        // <- #i26762#

                        pSectNd->GetSection().CreateLink( CREATE_CONNECT );
                    }
                    break;
                }
            }
        }
    } while( pSttNd );

//  if( pOutlNds != (SwOutlineNodes*)&GetNodes().GetOutLineNds();
    if( pOutlNds != &GetNodes().GetOutLineNds() )
        delete pOutlNds;

    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        if( get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        {
            // dann alles verbliebenen Bereiche aufheben
            while( GetSections().Count() )
                DelSectionFmt( GetSections()[ 0 ] );

            SfxFilterContainer* pFCntnr = pDocShell->GetFactory().GetFilterContainer();
            pFilter = pFCntnr->GetFilter4EA( pFilter->GetTypeName(), SFX_FILTER_EXPORT );
        }
        break;

//  case SPLITDOC_TO_GLOBALDOC:
    default:
        // dann das Globaldoc speichern
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, false);
    }

    //              Medium istn't locked after reopen the document. Bug 91462
    SfxRequest aReq( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, GetAttrPool() );
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, rPath ) );
    aReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
    if(pFilter)
        aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );
    const SfxBoolItem *pRet = (const SfxBoolItem*)pDocShell->ExecuteSlot( aReq );

    return pRet && pRet->GetValue();
}

//#outline level,add by zhaojianwei
sal_Bool SwDoc::SplitDoc( sal_uInt16 eDocType, const String& rPath, int nOutlineLevel )
{
    if( !pDocShell || !pDocShell->GetMedium() ||
        ( SPLITDOC_TO_GLOBALDOC == eDocType && get(IDocumentSettingAccess::GLOBAL_DOCUMENT) ) )
        return sal_False;

    sal_uInt16 nOutl = 0;
    SwOutlineNodes* pOutlNds = (SwOutlineNodes*)&GetNodes().GetOutLineNds();
    SwNodePtr pSttNd;

    const SfxFilter* pFilter;
    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        pFilter = SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "HTML" )));
        break;

    default:
//  case SPLITDOC_TO_GLOBALDOC:
        pFilter = SwIoSystem::GetFilterOfFormat(
                                    String::CreateFromAscii( FILTER_XML ));
        eDocType = SPLITDOC_TO_GLOBALDOC;
        break;
    }

    if( !pFilter )
        return sal_False;

    // Undo/Redline aufjedenfall abschalten
    GetIDocumentUndoRedo().DoUndo(false);
    SetRedlineMode_intern( (RedlineMode_t)(GetRedlineMode() & ~nsRedlineMode_t::REDLINE_ON));

    String sExt( pFilter->GetSuffixes().GetToken(0, ',') );
    if( !sExt.Len() )
        sExt.AssignAscii( "sxw" );
    if( '.' != sExt.GetChar( 0 ) )
        sExt.Insert( '.', 0 );

    INetURLObject aEntry(rPath);
    String sLeading(aEntry.GetBase());
    aEntry.removeSegment();
    String sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
    utl::TempFile aTemp(sLeading,&sExt,&sPath );
    aTemp.EnableKillingFile();

    DateTime aTmplDate;
    {
        Time a2Min( 0 ); a2Min.SetMin( 2 );
        aTmplDate += a2Min;
    }


    // alle Ungueltigen ueberspringen
    while( nOutl < pOutlNds->Count() &&
        pOutlNds->GetObject( nOutl )->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
        ++nOutl;

    do {
        pSttNd = 0;

        SwNodePtr pNd;
        for( ; nOutl < pOutlNds->Count(); ++nOutl )
            if( ( pNd = pOutlNds->GetObject( nOutl ))->GetTxtNode()->GetAttrOutlineLevel() == nOutlineLevel &&
                !pNd->FindTableNode() )
            {
                pSttNd = pNd;
                break;
            }

        if( pSttNd )
        {
            SwNodePtr pEndNd = 0;
            for( ++nOutl; nOutl < pOutlNds->Count(); ++nOutl )
            {
                pNd = pOutlNds->GetObject( nOutl );

                const int nLevel = pNd->GetTxtNode()->GetAttrOutlineLevel();

                if( ( 0 < nLevel && nLevel <= nOutlineLevel ) &&
                    !pNd->FindTableNode() )
                {
                    pEndNd = pNd;

                    break;
                }
            }
            SwNodeIndex aEndIdx( pEndNd ? *pEndNd
                                        : GetNodes().GetEndOfContent() );

            String sFileName;
            if( pSttNd->GetIndex() + 1 < aEndIdx.GetIndex() )
            {
                SfxObjectShellLock xDocSh( new SwDocShell( SFX_CREATE_MODE_INTERNAL ));
                if( xDocSh->DoInitNew( 0 ) )
                {
                    SwDoc* pDoc = ((SwDocShell*)(&xDocSh))->GetDoc();

                    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                        ((SwDocShell*)(&xDocSh))->GetModel(),
                        uno::UNO_QUERY_THROW);
                    uno::Reference<document::XDocumentProperties> xDocProps(
                        xDPS->getDocumentProperties());
                    DBG_ASSERT(xDocProps.is(), "Doc has no DocumentProperties");
                    // the GlobalDoc is the template
                    xDocProps->setTemplateName(aEmptyStr);
                    ::util::DateTime uDT(aTmplDate.Get100Sec(),
                        aTmplDate.GetSec(), aTmplDate.GetMin(),
                        aTmplDate.GetHour(), aTmplDate.GetDay(),
                        aTmplDate.GetMonth(), aTmplDate.GetYear());
                    xDocProps->setTemplateDate(uDT);
                    xDocProps->setTemplateURL(rPath);
                    //JP 14.06.99: Set the text of the "split para" as title
                    //              from the new doc. Is the current doc has
                    //              a title, insert it at begin.
                    String sTitle( xDocProps->getTitle() );
                    if( sTitle.Len() )
                        sTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ));
                    sTitle += ((SwTxtNode*)pSttNd)->GetExpandTxt();
                    xDocProps->setTitle( sTitle );

                    // Vorlagen ersetzen
                    pDoc->ReplaceStyles( *this );

                    // KapitelNumerierung uebernehmen
                    if( pOutlineRule )
                        pDoc->SetOutlineNumRule( *pOutlineRule );

                    SwNodeRange aRg( *pSttNd, 0, aEndIdx.GetNode() );
                    SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
                    GetNodes()._Copy( aRg, aTmpIdx, sal_False );

                    // den initialen TextNode loeschen
                    SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
                    if( aIdx.GetIndex() + 1 !=
                        pDoc->GetNodes().GetEndOfContent().GetIndex() )
                        pDoc->GetNodes().Delete( aIdx, 1 );

                    // alle Flys in dem Bereich
                    CopyFlyInFlyImpl( aRg, 0, aIdx );


                    // und noch alle Bookmarks
                    // ?????

                    utl::TempFile aTempFile2(sLeading,&sExt,&sPath );
                    sFileName = aTempFile2.GetURL();
                    SfxMedium* pTmpMed = new SfxMedium( sFileName,
                                                STREAM_STD_READWRITE, sal_True );
                    pTmpMed->SetFilter( pFilter );

                    // fuer den HTML-Filter mussen wir aber ein Layout
                    // haben, damit Textrahmen/Controls/OLE-Objecte korrekt
                    // als Grafik exportiert werden koennen.
                    if( SPLITDOC_TO_HTML == eDocType &&
                        pDoc->GetSpzFrmFmts()->Count() )
                    {
                        /* SfxViewFrame* pFrame = */
                            SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
                    }
                    xDocSh->DoSaveAs( *pTmpMed );
                    xDocSh->DoSaveCompleted( pTmpMed );

                    // beim Fehler wird keine FileLinkSection eingefuegt
                    if( xDocSh->GetError() )
                        sFileName.Erase();
                }
                xDocSh->DoClose();
            }

            // dann koennen ja die Bereiche eingefuegt werden
            if( sFileName.Len() )
            {
                switch( eDocType )
                {
                case SPLITDOC_TO_HTML:
                    {
                        // loesche alle Nodes im Bereich und setze im "Start-
                        // Node" den Link auf das gespeicherte Doc
                        sal_uLong nNodeDiff = aEndIdx.GetIndex() -
                                            pSttNd->GetIndex() - 1;
                        if( nNodeDiff )
                        {
                            SwPaM aTmp( *pSttNd, aEndIdx.GetNode(), 1, -1 );
                            aTmp.GetPoint()->nContent.Assign( 0, 0 );
                            aTmp.GetMark()->nContent.Assign( 0, 0 );
                            SwNodeIndex aSIdx( aTmp.GetMark()->nNode );
                            SwNodeIndex aEIdx( aTmp.GetPoint()->nNode );

                            // versuche hinters Ende zu verschieben
                            if( !aTmp.Move( fnMoveForward, fnGoNode ) )
                            {
                                // na gut, dann an den Anfang
                                aTmp.Exchange();
                                if( !aTmp.Move( fnMoveBackward, fnGoNode ))
                                {
                                    ASSERT( sal_False, "kein Node mehr vorhanden" );
                                }
                            }
                                // Bookmarks usw. verschieben
                            CorrAbs( aSIdx, aEIdx, *aTmp.GetPoint(), sal_True);

                            // stehen noch FlyFrames rum, loesche auch diese
                            for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
                            {
                                SwFrmFmt* pFly = (*GetSpzFrmFmts())[n];
                                const SwFmtAnchor* pAnchor = &pFly->GetAnchor();
                                SwPosition const*const pAPos =
                                    pAnchor->GetCntntAnchor();
                                if (pAPos &&
                                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                                    aSIdx <= pAPos->nNode &&
                                    pAPos->nNode < aEIdx )
                                {
                                    DelLayoutFmt( pFly );
                                    --n;
                                }
                            }

                            GetNodes().Delete( aSIdx, nNodeDiff );
                        }

                        // dann setze im StartNode noch den Link:
                        SwFmtINetFmt aINet( sFileName , aEmptyStr );
                        SwTxtNode* pTNd = (SwTxtNode*)pSttNd;
                        pTNd->InsertItem( aINet, 0, pTNd->GetTxt().Len() );

                        // wenn der nicht mehr gefunden wird, kann das nur
                        // ein Bug sein!
                        if( !pOutlNds->Seek_Entry( pSttNd, &nOutl ))
                            pSttNd = 0;
                        ++nOutl;
                    }
                    break;

                default:
                    {
                        String sNm( INetURLObject( sFileName ).GetName() );
                        SwSectionData aSectData( FILE_LINK_SECTION,
                                        GetUniqueSectionName( &sNm ));
                        SwSectionFmt* pFmt = MakeSectionFmt( 0 );
                        aSectData.SetLinkFileName(sFileName);
                        aSectData.SetProtectFlag(true);

                        aEndIdx--;  // im InsertSection ist Ende inclusive
                        while( aEndIdx.GetNode().IsStartNode() )
                            aEndIdx--;

                        // JP 06.07.99 - Bug 67361 - is any Section ends or
                        // starts in the new sectionrange, they must end or
                        // start before or behind the range!
                        SwSectionNode* pSectNd = pSttNd->FindSectionNode();
                        while( pSectNd && pSectNd->EndOfSectionIndex()
                                <= aEndIdx.GetIndex() )
                        {
                            const SwNode* pSectEnd = pSectNd->EndOfSectionNode();
                            if( pSectNd->GetIndex() + 1 ==
                                    pSttNd->GetIndex() )
                            {
                                sal_Bool bMvIdx = aEndIdx == *pSectEnd;
                                DelSectionFmt( pSectNd->GetSection().GetFmt() );
                                if( bMvIdx )
                                    aEndIdx--;
                            }
                            else
                            {
                                SwNodeRange aRg( *pSttNd, *pSectEnd );
                                SwNodeIndex aIdx( *pSectEnd, 1 );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }
                            pSectNd = pSttNd->FindSectionNode();
                        }

                        pSectNd = aEndIdx.GetNode().FindSectionNode();
                        while( pSectNd && pSectNd->GetIndex() >
                                pSttNd->GetIndex() )
                        {
                            if( aEndIdx < pSectNd->EndOfSectionIndex() )
                            {
                                SwNodeRange aRg( *pSectNd, 1, aEndIdx, 1 );
                                SwNodeIndex aIdx( *pSectNd );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }

                            pSectNd = pSttNd->FindSectionNode();
                        }

                        SwNodeIndex aStartIdx(*pSttNd);

                        if (aEndIdx >= aStartIdx)
                        {
                            pSectNd = GetNodes().InsertTextSection(aStartIdx,
                                *pFmt, aSectData, 0, &aEndIdx, false);
                        }
                        else
                        {
                            pSectNd = GetNodes().InsertTextSection(aEndIdx,
                                *pFmt, aSectData, 0, &aStartIdx, false);
                        }

                        pSectNd->GetSection().CreateLink( CREATE_CONNECT );
                    }
                    break;
                }
            }
        }
    } while( pSttNd );

    if( pOutlNds != &GetNodes().GetOutLineNds() )
        delete pOutlNds;

    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        if( get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        {
            while( GetSections().Count() )
                DelSectionFmt( GetSections()[ 0 ] );

            SfxFilterContainer* pFCntnr = pDocShell->GetFactory().GetFilterContainer();
            pFilter = pFCntnr->GetFilter4EA( pFilter->GetTypeName(), SFX_FILTER_EXPORT );
        }
        break;

//  case SPLITDOC_TO_GLOBALDOC:
    default:
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, false);
    }

    SfxRequest aReq( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, GetAttrPool() );
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, rPath ) );
    aReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
    if(pFilter)
        aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );
    const SfxBoolItem *pRet = (const SfxBoolItem*)pDocShell->ExecuteSlot( aReq );

    return pRet && pRet->GetValue();
}//<-end,zhaojianwei

