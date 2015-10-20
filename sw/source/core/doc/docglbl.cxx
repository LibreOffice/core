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
#include <IDocumentRedlineAccess.hxx>
#include <DocumentSettingManager.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <globdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <section.hxx>
#include <doctxm.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>
#include <memory>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

using namespace ::com::sun::star;

enum SwSplitDocType
{
    SPLITDOC_TO_GLOBALDOC,
    SPLITDOC_TO_HTML
};

bool SwDoc::GenerateGlobalDoc( const OUString& rPath,
                                   const SwTextFormatColl* pSplitColl )
{
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, false, pSplitColl );
}

bool SwDoc::GenerateGlobalDoc( const OUString& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_GLOBALDOC, rPath, true, 0, nOutlineLevel );
}

bool SwDoc::GenerateHTMLDoc( const OUString& rPath, int nOutlineLevel )
{
    return SplitDoc( SPLITDOC_TO_HTML, rPath, true, 0, nOutlineLevel );
}

bool SwDoc::GenerateHTMLDoc( const OUString& rPath,
                                 const SwTextFormatColl* pSplitColl )
{
    return SplitDoc( SPLITDOC_TO_HTML, rPath, false, pSplitColl );
}

// two helpers for outline mode
SwNodePtr GetStartNode( SwOutlineNodes* pOutlNds, int nOutlineLevel, sal_uInt16* nOutl )
{
    SwNodePtr pNd;

    for( ; *nOutl < pOutlNds->size(); ++(*nOutl) )
        if( ( pNd = (*pOutlNds)[ *nOutl ])->GetTextNode()->GetAttrOutlineLevel() == nOutlineLevel && !pNd->FindTableNode() )
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

        const int nLevel = pNd->GetTextNode()->GetAttrOutlineLevel();

        if( ( 0 < nLevel && nLevel <= nOutlineLevel ) &&
            !pNd->FindTableNode() )
        {
            return pNd;
        }
    }
    return 0;
}

// two helpers for collection mode
SwNodePtr GetStartNode( const SwOutlineNodes* pOutlNds, const SwTextFormatColl* pSplitColl, sal_uInt16* nOutl )
{
    SwNodePtr pNd;
    for( ; *nOutl < pOutlNds->size(); ++(*nOutl) )
        if( ( pNd = (*pOutlNds)[ *nOutl ])->GetTextNode()->
                    GetTextColl() == pSplitColl &&
            !pNd->FindTableNode() )
        {
            return pNd;
        }
    return 0;
}

SwNodePtr GetEndNode( const SwOutlineNodes* pOutlNds, const SwTextFormatColl* pSplitColl, sal_uInt16* nOutl )
{
    SwNodePtr pNd;

    for( ++(*nOutl); *nOutl < pOutlNds->size(); ++(*nOutl) )
    {
        pNd = (*pOutlNds)[ *nOutl ];
        SwTextFormatColl* pTColl = pNd->GetTextNode()->GetTextColl();

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

bool SwDoc::SplitDoc( sal_uInt16 eDocType, const OUString& rPath, bool bOutline, const SwTextFormatColl* pSplitColl, int nOutlineLevel )
{
    // Iterate over all the template's Nodes, creating an own
    // document for every single one and replace linked sections (GlobalDoc) for links (HTML).
    // Finally, we save this document as a GlobalDoc/HTMLDoc.
    if( !mpDocShell || !mpDocShell->GetMedium() ||
        ( SPLITDOC_TO_GLOBALDOC == eDocType && GetDocumentSettingManager().get(DocumentSettingId::GLOBAL_DOCUMENT) ) )
        return false;

    sal_uInt16 nOutl = 0;
    SwOutlineNodes* pOutlNds = const_cast<SwOutlineNodes*>(&GetNodes().GetOutLineNds());
    std::unique_ptr<SwOutlineNodes> xTmpOutlNds;
    SwNodePtr pStartNd;

    if ( !bOutline) {
        if( pSplitColl )
        {
            // If it isn't a OutlineNumbering, then use an own array and collect the Nodes.
            if( pSplitColl->GetAttrOutlineLevel() == 0 )
            {
                xTmpOutlNds.reset(new SwOutlineNodes);
                pOutlNds = xTmpOutlNds.get();
                SwIterator<SwTextNode,SwFormatColl> aIter( *pSplitColl );
                for( SwTextNode* pTNd = aIter.First(); pTNd; pTNd = aIter.Next() )
                    if( pTNd->GetNodes().IsDocNodes() )
                        pOutlNds->insert( pTNd );

                if( pOutlNds->empty() )
                    return false;
            }
        }
        else
        {
            // Look for the 1st level OutlineTemplate
            const SwTextFormatColls& rFormatColls =*GetTextFormatColls();
            for( SwTextFormatColls::size_type n = rFormatColls.size(); n; )
                if ( rFormatColls[ --n ]->GetAttrOutlineLevel() == 1 )
                {
                    pSplitColl = rFormatColls[ n ];
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
    getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)(getIDocumentRedlineAccess().GetRedlineMode() & ~nsRedlineMode_t::REDLINE_ON));

    OUString sExt = pFilter->GetSuffixes().getToken(0, ',');
    if( sExt.isEmpty() )
    {
        sExt = ".sxw";
    }
    else
    {
        if( '.' != sExt[ 0 ] )
        {
            sExt = "." + sExt;
        }
    }

    INetURLObject aEntry(rPath);
    OUString sLeading(aEntry.GetBase());
    aEntry.removeSegment();
    OUString sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
    utl::TempFile aTemp(sLeading, true, &sExt, &sPath);
    aTemp.EnableKillingFile();

    DateTime aTmplDate( DateTime::SYSTEM );
    {
        tools::Time a2Min( 0 ); a2Min.SetMin( 2 );
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
            OUString sFileName;
            if( pStartNd->GetIndex() + 1 < aEndIdx.GetIndex() )
            {
                SfxObjectShellLock xDocSh( new SwDocShell( SfxObjectCreateMode::INTERNAL ));
                if( xDocSh->DoInitNew() )
                {
                    SwDoc* pDoc = static_cast<SwDocShell*>(&xDocSh)->GetDoc();

                    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                        static_cast<SwDocShell*>(&xDocSh)->GetModel(),
                        uno::UNO_QUERY_THROW);
                    uno::Reference<document::XDocumentProperties> xDocProps(
                        xDPS->getDocumentProperties());
                    OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");
                    // the GlobalDoc is the template
                    xDocProps->setTemplateName(OUString());
                    ::util::DateTime uDT = aTmplDate.GetUNODateTime();
                    xDocProps->setTemplateDate(uDT);
                    xDocProps->setTemplateURL(rPath);
                    // Set the new doc's title to the text of the "split para".
                    // If the current doc has a title, insert it at the begin.
                    OUString sTitle( xDocProps->getTitle() );
                    if (!sTitle.isEmpty())
                        sTitle += ": ";
                    sTitle += pStartNd->GetTextNode()->GetExpandText();
                    xDocProps->setTitle( sTitle );

                    // Replace template
                    pDoc->ReplaceStyles( *this );

                    // Take over chapter numbering
                    if( mpOutlineRule )
                        pDoc->SetOutlineNumRule( *mpOutlineRule );

                    SwNodeRange aRg( *pStartNd, 0, aEndIdx.GetNode() );
                    SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
                    GetNodes()._Copy( aRg, aTmpIdx, false );

                    // Delete the initial TextNode
                    SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
                    if( aIdx.GetIndex() + 1 !=
                        pDoc->GetNodes().GetEndOfContent().GetIndex() )
                        pDoc->GetNodes().Delete( aIdx );

                    // All Flys in the section
                    GetDocumentContentOperationsManager().CopyFlyInFlyImpl( aRg, 0, aIdx );

                    // And what's with all the Bookmarks?
                    // ?????

                    utl::TempFile aTempFile2(sLeading, true, &sExt, &sPath);
                    sFileName = aTempFile2.GetURL();
                    SfxMedium* pTmpMed = new SfxMedium( sFileName,
                                                STREAM_STD_READWRITE );
                    pTmpMed->SetFilter( pFilter );

                    // We need to have a Layout for the HTMLFilter, so that
                    // TextFrames/Controls/OLE objects can be exported correctly as graphics.
                    if( SPLITDOC_TO_HTML == eDocType &&
                        !pDoc->GetSpzFrameFormats()->empty() )
                    {
                            SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
                    }
                    xDocSh->DoSaveAs( *pTmpMed );
                    xDocSh->DoSaveCompleted( pTmpMed );

                    // do not insert a FileLinkSection in case of error
                    if( xDocSh->GetError() )
                        sFileName.clear();
                }
                xDocSh->DoClose();
            }

            // We can now insert the section
            if( !sFileName.isEmpty() )
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
                            CorrAbs( aSIdx, aEIdx, *aTmp.GetPoint(), true);

                            // If FlyFrames are still around, delete these too
                            for( SwFrameFormats::size_type n = 0; n < GetSpzFrameFormats()->size(); ++n )
                            {
                                SwFrameFormat* pFly = (*GetSpzFrameFormats())[n];
                                const SwFormatAnchor* pAnchor = &pFly->GetAnchor();
                                SwPosition const*const pAPos =
                                    pAnchor->GetContentAnchor();
                                if (pAPos &&
                                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                                    aSIdx <= pAPos->nNode &&
                                    pAPos->nNode < aEIdx )
                                {
                                    getIDocumentLayoutAccess().DelLayoutFormat( pFly );
                                    --n;
                                }
                            }

                            GetNodes().Delete( aSIdx, nNodeDiff );
                        }

                        // set the link in the StartNode
                        SwFormatINetFormat aINet( sFileName , OUString() );
                        SwTextNode* pTNd = pStartNd->GetTextNode();
                        pTNd->InsertItem(aINet, 0, pTNd->GetText().getLength());

                        // If the link cannot be found anymore,
                        // it has to be a bug!
                        if( !pOutlNds->Seek_Entry( pStartNd, &nOutl ))
                            pStartNd = 0;
                        ++nOutl;
                    }
                    break;

                default:
                    {
                        const OUString sNm( INetURLObject( sFileName ).GetName() );
                        SwSectionData aSectData( FILE_LINK_SECTION,
                                        GetUniqueSectionName( &sNm ));
                        SwSectionFormat* pFormat = MakeSectionFormat( 0 );
                        aSectData.SetLinkFileName(sFileName);
                        aSectData.SetProtectFlag(true);

                        --aEndIdx;  // in the InsertSection the end is inclusive
                        while( aEndIdx.GetNode().IsStartNode() )
                            --aEndIdx;

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
                                DelSectionFormat( pSectNd->GetSection().GetFormat() );
                                if( bMvIdx )
                                    --aEndIdx;
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
                                *pFormat, aSectData, 0, &aEndIdx, false);
                        }
                        else
                        {
                            pSectNd = GetNodes().InsertTextSection(aEndIdx,
                                *pFormat, aSectData, 0, &aStartIdx, false);
                        }
                        // <- #i26762#

                        pSectNd->GetSection().CreateLink( CREATE_CONNECT );
                    }
                    break;
                }
            }
        }
    } while( pStartNd );

    xTmpOutlNds.reset();

    switch( eDocType )
    {
    case SPLITDOC_TO_HTML:
        if( GetDocumentSettingManager().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        {
            // save all remaining sections
            while( !GetSections().empty() )
                DelSectionFormat( GetSections().front() );

            SfxFilterContainer* pFCntnr = mpDocShell->GetFactory().GetFilterContainer();
            pFilter = pFCntnr->GetFilter4EA( pFilter->GetTypeName(), SfxFilterFlags::EXPORT );
        }
        break;

    default:
        // save the Globaldoc
        GetDocumentSettingManager().set(DocumentSettingId::GLOBAL_DOCUMENT, true);
        GetDocumentSettingManager().set(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS, false);
    }

    // The medium isn't locked after reopening the document.
    SfxRequest aReq( SID_SAVEASDOC, SfxCallMode::SYNCHRON, GetAttrPool() );
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, rPath ) );
    aReq.AppendItem( SfxBoolItem( SID_SAVETO, true ) );
    if(pFilter)
        aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );
    const SfxBoolItem *pRet = static_cast<const SfxBoolItem*>(mpDocShell->ExecuteSlot( aReq ));

    return pRet && pRet->GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
