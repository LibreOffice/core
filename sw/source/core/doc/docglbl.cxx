/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <swundo.hxx>       // for the UndoIds
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
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, false, pSplitColl, 0 );
}

sal_Bool SwDoc::GenerateGlobalDoc( const String& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, true, 0, nOutlineLevel );
}

sal_Bool SwDoc::GenerateHTMLDoc( const String& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_HTML, rPath, true, 0, nOutlineLevel );
}

sal_Bool SwDoc::GenerateHTMLDoc( const String& rPath,
                                 const SwTxtFmtColl* pSplitColl )
{
    return SplitDoc( SPLITDOC_TO_HTML, rPath, false, pSplitColl, 0 );
}

// two helpers for outline mode
SwNodePtr GetStartNode( SwOutlineNodes* pOutlNds, int nOutlineLevel, sal_uInt16* nOutl )
{
    SwNodePtr pNd;

    for( ; *nOutl < pOutlNds->size(); ++(*nOutl) )
        if( ( pNd = (*pOutlNds)[ *nOutl ])->GetTxtNode()->GetAttrOutlineLevel() == nOutlineLevel && !pNd->FindTableNode() )
        {
            return pNd;
        }

    return 0;
}

SwNodePtr GetEndNode( SwOutlineNodes* pOutlNds, int nOutlineLevel, sal_uInt16* nOutl )
{
    SwNodePtr pNd;

    for( ++(*nOutl); (*nOutl) < pOutlNds->size(); ++(*nOutl) )
    {
        pNd = (*pOutlNds)[ *nOutl ];

        const int nLevel = pNd->GetTxtNode()->GetAttrOutlineLevel();

        if( ( 0 < nLevel && nLevel <= nOutlineLevel ) &&
            !pNd->FindTableNode() )
        {
            return pNd;
        }
    }
    return 0;
}

// two helpers for collection mode
SwNodePtr GetStartNode( const SwOutlineNodes* pOutlNds, const SwTxtFmtColl* pSplitColl, sal_uInt16* nOutl )
{
    SwNodePtr pNd;
    for( ; *nOutl < pOutlNds->size(); ++(*nOutl) )
        if( ( pNd = (*pOutlNds)[ *nOutl ])->GetTxtNode()->
                    GetTxtColl() == pSplitColl &&
            !pNd->FindTableNode() )
        {
            return pNd;
        }
    return 0;
}

SwNodePtr GetEndNode( const SwOutlineNodes* pOutlNds, const SwTxtFmtColl* pSplitColl, sal_uInt16* nOutl )
{
    SwNodePtr pNd;

    for( ++(*nOutl); *nOutl < pOutlNds->size(); ++(*nOutl) )
    {
        pNd = (*pOutlNds)[ *nOutl ];
        SwTxtFmtColl* pTColl = pNd->GetTxtNode()->GetTxtColl();

        if( ( pTColl == pSplitColl ||
              (   pSplitColl->GetAttrOutlineLevel() > 0 &&
                  pTColl->GetAttrOutlineLevel() > 0   &&
                  pTColl->GetAttrOutlineLevel() <
                  pSplitColl->GetAttrOutlineLevel() )) &&
            !pNd->FindTableNode() )
        {
            return pNd;
        }
    }
    return 0;
}

bool SwDoc::SplitDoc( sal_uInt16 eDocType, const String& rPath, bool bOutline, const SwTxtFmtColl* pSplitColl, int nOutlineLevel )
{
    // Iterate over all the template's Nodes, creating an own
    // document for every single one and replace linked sections (GlobalDoc) for links (HTML).
    // Finally, we save this document as a GlobalDoc/HTMLDoc.
    if( !mpDocShell || !mpDocShell->GetMedium() ||
        ( SPLITDOC_TO_GLOBALDOC == eDocType && get(IDocumentSettingAccess::GLOBAL_DOCUMENT) ) )
        return false;

    sal_uInt16 nOutl = 0;
    SwOutlineNodes* pOutlNds = (SwOutlineNodes*)&GetNodes().GetOutLineNds();
    SwNodePtr pStartNd;

    if ( !bOutline) {
    if( pSplitColl )
    {
        // If it isn't a OutlineNumbering, then use an own array and collect the Nodes.
        if( pSplitColl->GetAttrOutlineLevel() == 0 )//<-end,zhaojianwei, 0814
        {
            pOutlNds = new SwOutlineNodes;
            SwIterator<SwTxtNode,SwFmtColl> aIter( *pSplitColl );
            for( SwTxtNode* pTNd = aIter.First(); pTNd; pTNd = aIter.Next() )
                if( pTNd->GetNodes().IsDocNodes() )
                    pOutlNds->insert( pTNd );

            if( pOutlNds->empty() )
            {
                delete pOutlNds;
                return false;
            }
        }
    }
    else
    {
        // Look for the 1st level OutlineTemplate
        const SwTxtFmtColls& rFmtColls =*GetTxtFmtColls();
        for( sal_uInt16 n = rFmtColls.size(); n; )
            if ( rFmtColls[ --n ]->GetAttrOutlineLevel() == 1 )//<-end,zhaojianwei
            {
                pSplitColl = rFmtColls[ n ];
                break;
            }

        if( !pSplitColl )
            return false;
    }
    }

    const SfxFilter* pFilter;
    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        pFilter = SwIoSystem::GetFilterOfFormat(OUString("HTML"));
        break;

    default:
        pFilter = SwIoSystem::GetFilterOfFormat(OUString(FILTER_XML));
        eDocType = SPLITDOC_TO_GLOBALDOC;
        break;
    }

    if( !pFilter )
        return false;

    // Deactivate Undo/Redline in any case
    GetIDocumentUndoRedo().DoUndo(false);
    SetRedlineMode_intern( (RedlineMode_t)(GetRedlineMode() & ~nsRedlineMode_t::REDLINE_ON));

    OUString sExt = pFilter->GetSuffixes().getToken(0, ',');
    if( sExt.isEmpty() )
    {
        sExt = ".sxw";
    }
    else
    {
        if( '.' != sExt[ 0 ] )
        {
            sExt = OUString(".") + sExt;
        }
    }

    INetURLObject aEntry(rPath);
    String sLeading(aEntry.GetBase());
    aEntry.removeSegment();
    OUString sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
    utl::TempFile aTemp(sLeading,&sExt,&sPath );
    aTemp.EnableKillingFile();

    DateTime aTmplDate( DateTime::SYSTEM );
    {
        Time a2Min( 0 ); a2Min.SetMin( 2 );
        aTmplDate += a2Min;
    }


    // Skip all invalid ones
    while( nOutl < pOutlNds->size() &&
        (*pOutlNds)[ nOutl ]->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
        ++nOutl;

    do {
        if( bOutline )
            pStartNd = GetStartNode( pOutlNds, nOutlineLevel, &nOutl );
        else
            pStartNd = GetStartNode( pOutlNds, pSplitColl, &nOutl );

        if( pStartNd )
        {
            SwNodePtr pEndNd;
            if( bOutline )
                pEndNd = GetEndNode( pOutlNds, nOutlineLevel, &nOutl );
            else
                pEndNd = GetEndNode( pOutlNds, pSplitColl, &nOutl );
            SwNodeIndex aEndIdx( pEndNd ? *pEndNd
                                        : GetNodes().GetEndOfContent() );

            // Write out the Nodes completely
            String sFileName;
            if( pStartNd->GetIndex() + 1 < aEndIdx.GetIndex() )
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
                    OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");
                    // the GlobalDoc is the template
                    xDocProps->setTemplateName(aEmptyStr);
                    ::util::DateTime uDT(aTmplDate.GetNanoSec(),
                        aTmplDate.GetSec(), aTmplDate.GetMin(),
                        aTmplDate.GetHour(), aTmplDate.GetDay(),
                        aTmplDate.GetMonth(), aTmplDate.GetYear(),
                        false );
                    xDocProps->setTemplateDate(uDT);
                    xDocProps->setTemplateURL(rPath);
                    // Set the new doc's title to the text of the "split para".
                    // If the current doc has a title, insert it at the begin.
                    String sTitle( xDocProps->getTitle() );
                    if( sTitle.Len() )
                        sTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ));
                    sTitle += ((SwTxtNode*)pStartNd)->GetExpandTxt();
                    xDocProps->setTitle( sTitle );

                    // Replace template
                    pDoc->ReplaceStyles( *this );

                    // Take over chapter numbering
                    if( mpOutlineRule )
                        pDoc->SetOutlineNumRule( *mpOutlineRule );

                    SwNodeRange aRg( *pStartNd, 0, aEndIdx.GetNode() );
                    SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
                    GetNodes()._Copy( aRg, aTmpIdx, sal_False );

                    // Delete the initial TextNode
                    SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
                    if( aIdx.GetIndex() + 1 !=
                        pDoc->GetNodes().GetEndOfContent().GetIndex() )
                        pDoc->GetNodes().Delete( aIdx, 1 );

                    // All Flys in the section
                    CopyFlyInFlyImpl( aRg, 0, aIdx );


                    // And what's with all the Bookmarks?
                    // ?????

                    utl::TempFile aTempFile2(sLeading,&sExt,&sPath );
                    sFileName = aTempFile2.GetURL();
                    SfxMedium* pTmpMed = new SfxMedium( sFileName,
                                                STREAM_STD_READWRITE );
                    pTmpMed->SetFilter( pFilter );

                    // We need to have a Layout for the HTMLFilter, so that
                    // TextFrames/Controls/OLE objects can be exported correctly as graphics.
                    if( SPLITDOC_TO_HTML == eDocType &&
                        !pDoc->GetSpzFrmFmts()->empty() )
                    {
                            SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
                    }
                    xDocSh->DoSaveAs( *pTmpMed );
                    xDocSh->DoSaveCompleted( pTmpMed );

                    // do not insert a FileLinkSection in case of error
                    if( xDocSh->GetError() )
                        sFileName.Erase();
                }
                xDocSh->DoClose();
            }

            // We can now insert the section
            if( sFileName.Len() )
            {
                switch( eDocType )
                {
                case SPLITDOC_TO_HTML:
                    {
                        // Delete all nodes in the section and, in the "start node",
                        // set the Link to the saved document.
                        sal_uLong nNodeDiff = aEndIdx.GetIndex() -
                                            pStartNd->GetIndex() - 1;
                        if( nNodeDiff )
                        {
                            SwPaM aTmp( *pStartNd, aEndIdx.GetNode(), 1, -1 );
                            aTmp.GetPoint()->nContent.Assign( 0, 0 );
                            aTmp.GetMark()->nContent.Assign( 0, 0 );
                            SwNodeIndex aSIdx( aTmp.GetMark()->nNode );
                            SwNodeIndex aEIdx( aTmp.GetPoint()->nNode );

                            // Try to move past the end
                            if( !aTmp.Move( fnMoveForward, fnGoNode ) )
                            {
                                // well then, back to the beginning
                                aTmp.Exchange();
                                if( !aTmp.Move( fnMoveBackward, fnGoNode ))
                                {
                                    OSL_FAIL( "no more Nodes!" );
                                }
                            }
                            // Move Bookmarks and so forth
                            CorrAbs( aSIdx, aEIdx, *aTmp.GetPoint(), sal_True);

                            // If FlyFrames are still around, delete these too
                            for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->size(); ++n )
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

                        // set the link in the StartNode
                        SwFmtINetFmt aINet( sFileName , OUString() );
                        SwTxtNode* pTNd = (SwTxtNode*)pStartNd;
                        pTNd->InsertItem(aINet, 0, pTNd->GetTxt().getLength());

                        // If the link cannot be found anymore,
                        // it has to be a bug!
                        if( !pOutlNds->Seek_Entry( pStartNd, &nOutl ))
                            pStartNd = 0;
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

                        aEndIdx--;  // in the InsertSection the end is inclusive
                        while( aEndIdx.GetNode().IsStartNode() )
                            aEndIdx--;

                        // If any Section ends or starts in the new sectionrange,
                        // they must end or start before or after the range!
                        SwSectionNode* pSectNd = pStartNd->FindSectionNode();
                        while( pSectNd && pSectNd->EndOfSectionIndex()
                                <= aEndIdx.GetIndex() )
                        {
                            const SwNode* pSectEnd = pSectNd->EndOfSectionNode();
                            if( pSectNd->GetIndex() + 1 ==
                                    pStartNd->GetIndex() )
                            {
                                bool bMvIdx = aEndIdx == *pSectEnd;
                                DelSectionFmt( pSectNd->GetSection().GetFmt() );
                                if( bMvIdx )
                                    aEndIdx--;
                            }
                            else
                            {
                                SwNodeRange aRg( *pStartNd, *pSectEnd );
                                SwNodeIndex aIdx( *pSectEnd, 1 );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }
                            pSectNd = pStartNd->FindSectionNode();
                        }

                        pSectNd = aEndIdx.GetNode().FindSectionNode();
                        while( pSectNd && pSectNd->GetIndex() >
                                pStartNd->GetIndex() )
                        {
                            // #i15712# don't attempt to split sections if
                            // they are fully enclosed in [pSectNd,aEndIdx].
                            if( aEndIdx < pSectNd->EndOfSectionIndex() )
                            {
                                SwNodeRange aRg( *pSectNd, 1, aEndIdx, 1 );
                                SwNodeIndex aIdx( *pSectNd );
                                GetNodes()._MoveNodes( aRg, GetNodes(), aIdx );
                            }

                            pSectNd = pStartNd->FindSectionNode();
                        }

                        // -> #i26762#
                        // Ensure order of start and end of section is sane.
                        SwNodeIndex aStartIdx(*pStartNd);

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
    } while( pStartNd );

    if( pOutlNds != &GetNodes().GetOutLineNds() )
        delete pOutlNds;

    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        if( get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        {
            // save all remaining sections
            while( !GetSections().empty() )
                DelSectionFmt( GetSections().front() );

            SfxFilterContainer* pFCntnr = mpDocShell->GetFactory().GetFilterContainer();
            pFilter = pFCntnr->GetFilter4EA( pFilter->GetTypeName(), SFX_FILTER_EXPORT );
        }
        break;

    default:
        // save the Globaldoc
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
        set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, false);
    }

    // The medium isn't locked after reopening the document.
    SfxRequest aReq( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, GetAttrPool() );
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, rPath ) );
    aReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
    if(pFilter)
        aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );
    const SfxBoolItem *pRet = (const SfxBoolItem*)mpDocShell->ExecuteSlot( aReq );

    return pRet && pRet->GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
