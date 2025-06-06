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
#include <osl/diagnose.h>
#include <vcl/commandevent.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentState.hxx>
#include <editsh.hxx>
#include <frame.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <swundo.hxx>
#include <calc.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <extinput.hxx>
#include <scriptinfo.hxx>
#include <unocrsrhelper.hxx>
#include <section.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>
#include <unocrsr.hxx>
#include <calbck.hxx>

using namespace com::sun::star;

void SwEditShell::Insert( sal_Unicode c, bool bOnlyCurrCursor )
{
    StartAllAction();
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        const bool bSuccess = GetDoc()->getIDocumentContentOperations().InsertString(rPaM, OUString(c));
        OSL_ENSURE( bSuccess, "Doc->Insert() failed." );

        SaveTableBoxContent( rPaM.GetPoint() );
        if( bOnlyCurrCursor )
            break;

    }

    EndAllAction();
}

void SwEditShell::Insert2(const OUString &rStr, const bool bForceExpandHints )
{
    StartAllAction();
    {
        const SwInsertFlags nInsertFlags =
            bForceExpandHints
            ? (SwInsertFlags::FORCEHINTEXPAND | SwInsertFlags::EMPTYEXPAND)
            : SwInsertFlags::EMPTYEXPAND;

        for(SwPaM& rCurrentCursor : getShellCursor( true )->GetRingContainer())
        {
            //OPT: GetSystemCharSet
            GetDoc()->getIDocumentContentOperations().SetIME(false);
            const bool bSuccess =
                GetDoc()->getIDocumentContentOperations().InsertString(rCurrentCursor, rStr, nInsertFlags);
            OSL_ENSURE( bSuccess, "Doc->Insert() failed." );

            if (bSuccess)
            {
                GetDoc()->UpdateRsid( rCurrentCursor, rStr.getLength() );

                // Set paragraph rsid if beginning of paragraph
                SwTextNode *const pTextNode =
                    rCurrentCursor.GetPoint()->GetNode().GetTextNode();
                if( pTextNode && pTextNode->Len() == 1)
                    GetDoc()->UpdateParRsid( pTextNode );
            }

            SaveTableBoxContent( rCurrentCursor.GetPoint() );

        }
    }

    // calculate cursor bidi level
    SwCursor* pTmpCursor = GetCursor_();
    const bool bDoNotSetBidiLevel = ! pTmpCursor ||
                                ( dynamic_cast<SwUnoCursor*>(pTmpCursor) !=  nullptr );

    if ( ! bDoNotSetBidiLevel )
    {
        SwNode& rNode = pTmpCursor->GetPoint()->GetNode();
        if ( rNode.IsTextNode() )
        {
            sal_Int32 nPrevPos = pTmpCursor->GetPoint()->GetContentIndex();
            if ( nPrevPos )
                --nPrevPos;

            SwTextFrame const* pFrame;
            SwScriptInfo *const pSI = SwScriptInfo::GetScriptInfo(
                    static_cast<SwTextNode&>(rNode), &pFrame, true);

            sal_uInt8 nLevel = 0;
            if ( ! pSI )
            {
                // seems to be an empty paragraph.
                Point aPt;
                std::pair<Point, bool> const tmp(aPt, false);
                pFrame = static_cast<SwTextFrame*>(
                        static_cast<SwTextNode&>(rNode).getLayoutFrame(
                            GetLayout(), pTmpCursor->GetPoint(), &tmp));

                SwScriptInfo aScriptInfo;
                aScriptInfo.InitScriptInfo(static_cast<SwTextNode&>(rNode),
                        pFrame->GetMergedPara(), pFrame->IsRightToLeft());
                TextFrameIndex const iPrevPos(pFrame->MapModelToView(
                            &static_cast<SwTextNode&>(rNode), nPrevPos));
                nLevel = aScriptInfo.DirType( iPrevPos );
            }
            else
            {
                if (TextFrameIndex(COMPLETE_STRING) != pSI->GetInvalidityA())
                {
                    // mystery why this doesn't use the other overload?
                    pSI->InitScriptInfo(static_cast<SwTextNode&>(rNode), pFrame->GetMergedPara());
                }
                TextFrameIndex const iPrevPos(pFrame->MapModelToView(
                            &static_cast<SwTextNode&>(rNode), nPrevPos));
                nLevel = pSI->DirType(iPrevPos);
            }

            pTmpCursor->SetCursorBidiLevel( nLevel );
        }
    }

    SetInFrontOfLabel( false ); // #i27615#

    EndAllAction();
}

void SwEditShell::Overwrite(const OUString &rStr)
{
    StartAllAction();
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( !GetDoc()->getIDocumentContentOperations().Overwrite(rPaM, rStr ) )
        {
            OSL_FAIL( "Doc->getIDocumentContentOperations().Overwrite(Str) failed." );
        }
        SaveTableBoxContent( rPaM.GetPoint() );
    }
    EndAllAction();
}

void SwEditShell::SplitNode( bool bAutoFormat, bool bCheckTableStart )
{
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        // Here, a table cell becomes a normal text cell.
        GetDoc()->ClearBoxNumAttrs( rPaM.GetPoint()->GetNode() );
        GetDoc()->getIDocumentContentOperations().SplitNode( *rPaM.GetPoint(), bCheckTableStart );
    }

    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

    if( bAutoFormat )
        AutoFormatBySplitNode();

    ClearTableBoxContent();

    EndAllAction();
}

bool SwEditShell::AppendTextNode()
{
    bool bRet = false;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        GetDoc()->ClearBoxNumAttrs( rPaM.GetPoint()->GetNode() );
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( *rPaM.GetPoint()) || bRet;
    }

    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

    ClearTableBoxContent();

    EndAllAction();
    return bRet;
}

// the returned SwGrfNode pointer is used in GetGraphic() and GetGraphicSize()
SwGrfNode * SwEditShell::GetGrfNode_() const
{
    SwGrfNode *pGrfNode = nullptr;
    SwPaM* pCursor = GetCursor();
    if( !pCursor->HasMark() ||
        pCursor->GetPoint()->GetNode() == pCursor->GetMark()->GetNode() )
        pGrfNode = pCursor->GetPoint()->GetNode().GetGrfNode();

    return pGrfNode;
}

// returns a Graphic pointer if CurrentCursor->GetPoint() points to a SwGrfNode and
// GetMark is not set or points to the same Graphic
const Graphic* SwEditShell::GetGraphic( bool bWait ) const
{
    SwGrfNode* pGrfNode = GetGrfNode_();
    const Graphic* pGrf( nullptr );
    if ( pGrfNode )
    {
        pGrf = &(pGrfNode->GetGrf(bWait && GraphicType::Default == pGrfNode->GetGrf().GetType()));
    }
    return pGrf;
}

bool SwEditShell::IsLinkedGrfSwapOut() const
{
    SwGrfNode *pGrfNode = GetGrfNode_();
    return pGrfNode &&
           pGrfNode->IsLinkedFile() &&
           GraphicType::Default == pGrfNode->GetGrfObj().GetType();
}

const GraphicObject* SwEditShell::GetGraphicObj() const
{
    SwGrfNode* pGrfNode = GetGrfNode_();
    return pGrfNode ? &(pGrfNode->GetGrfObj()) : nullptr;
}

const GraphicAttr* SwEditShell::GetGraphicAttr( GraphicAttr& rGA ) const
{
    SwGrfNode* pGrfNode = GetGrfNode_();
    const SwFrame* pFrame = GetCurrFrame(false);
    return pGrfNode ? &(pGrfNode->GetGraphicAttr( rGA, pFrame )) : nullptr;
}

GraphicType SwEditShell::GetGraphicType() const
{
    SwGrfNode *pGrfNode = GetGrfNode_();
    return pGrfNode ? pGrfNode->GetGrfObj().GetType() : GraphicType::NONE;
}

// returns the size of a graphic in <rSz> if CurrentCursor->GetPoint() points to a SwGrfNode and
// GetMark is not set or points to the same graphic
bool SwEditShell::GetGrfSize(Size& rSz) const
{
    SwNoTextNode* pNoTextNd;
    SwPaM* pCurrentCursor = GetCursor();
    if( ( !pCurrentCursor->HasMark()
         || pCurrentCursor->GetPoint()->GetNode() == pCurrentCursor->GetMark()->GetNode() )
         && nullptr != ( pNoTextNd = pCurrentCursor->GetPointNode().GetNoTextNode() ) )
    {
        rSz = pNoTextNd->GetTwipSize();
        return true;
    }
    return false;

}

/// Read again if graphic is not OK and replace old one
void SwEditShell::ReRead( const OUString& rGrfName, const OUString& rFltName,
                    const Graphic* pGraphic )
{
    StartAllAction();
    mxDoc->getIDocumentContentOperations().ReRead( *GetCursor(), rGrfName, rFltName, pGraphic );
    EndAllAction();
}

/// Returns the name and the filter name of a graphic if the pointer is on a graphic.
/// If a String-pointer is != 0 then return corresponding name.
void SwEditShell::GetGrfNms( OUString* pGrfName, OUString* pFltName ) const
{
    OSL_ENSURE( pGrfName || pFltName, "No parameters" );
    SwGrfNode *pGrfNode = GetGrfNode_();
    if( pGrfNode && pGrfNode->IsLinkedFile() )
        pGrfNode->GetFileFilterNms( pGrfName, pFltName );
}

const tools::PolyPolygon *SwEditShell::GetGraphicPolygon() const
{
    SwNoTextNode *pNd = GetCursor()->GetPointNode().GetNoTextNode();
    return pNd->HasContour();
}

void SwEditShell::SetGraphicPolygon( const tools::PolyPolygon *pPoly )
{
    SwNoTextNode *pNd = GetCursor()->GetPointNode().GetNoTextNode();
    StartAllAction();
    pNd->SetContour( pPoly );
    SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pNd->getLayoutFrame(GetLayout())->GetUpper());
    const SwFormatSurround &rSur = pFly->GetFormat()->GetSurround();
    pFly->GetFormat()->CallSwClientNotify(sw::LegacyModifyHint(&rSur, &rSur));
    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

void SwEditShell::ClearAutomaticContour()
{
    SwNoTextNode *pNd = GetCursor()->GetPointNode().GetNoTextNode();
    assert(pNd && "is no NoTextNode!");
    if( pNd->HasAutomaticContour() )
    {
        StartAllAction();
        pNd->SetContour( nullptr );
        SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pNd->getLayoutFrame(GetLayout())->GetUpper());
        const SwFormatSurround &rSur = pFly->GetFormat()->GetSurround();
        pFly->GetFormat()->CallSwClientNotify(sw::LegacyModifyHint(&rSur, &rSur));
        GetDoc()->getIDocumentState().SetModified();
        EndAllAction();
    }
}

/** Get OLE object at pointer.
 *
 * Returns a pointer to a SvInPlaceObjectRef if CurrentCursor->GetPoint() points to a SwOLENode and
 * GetMark is not set or points to the same object reference. Gets this pointer from the Doc
 * if the object should be searched by name.
 */
svt::EmbeddedObjectRef& SwEditShell::GetOLEObject() const
{
    OSL_ENSURE(  CNT_OLE == GetCntType(), "GetOLEObj: no OLENode." );
    OSL_ENSURE( !GetCursor()->HasMark() ||
            (GetCursor()->HasMark() &&
                GetCursor()->GetPoint()->GetNode() == GetCursor()->GetMark()->GetNode()),
            "GetOLEObj: no OLENode." );

    SwOLENode *pOLENode = GetCursor()->GetPointNode().GetOLENode();
    OSL_ENSURE( pOLENode, "GetOLEObj: no OLENode." );
    SwOLEObj& rOObj = pOLENode->GetOLEObj();
    return rOObj.GetObject();
}

bool SwEditShell::HasOLEObj( const UIName& rName ) const
{
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwNode& rNd = aIdx.GetNode();
        if( rNd.IsOLENode() &&
            rName == static_cast<SwOLENode&>(rNd).GetChartTableName() &&
            static_cast<SwOLENode&>(rNd).getLayoutFrame( GetLayout() ) )
            return true;

        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    return false;
}

void SwEditShell::SetChartName( const UIName &rName )
{
    SwOLENode *pONd = GetCursor()->GetPointNode().GetOLENode();
    OSL_ENSURE( pONd, "ChartNode not found" );
    pONd->SetChartTableName( rName );
}

void SwEditShell::UpdateCharts( const UIName& rName )
{
    GetDoc()->UpdateCharts( rName );
}

/// change table name
void SwEditShell::SetTableName( SwFrameFormat& rTableFormat, const UIName &rNewName )
{
    GetDoc()->SetTableName( rTableFormat, rNewName );
}

/// request current word
OUString SwEditShell::GetCurWord() const
{
    const SwPaM& rPaM = *GetCursor();
    const SwTextNode* pNd = rPaM.GetPointNode().GetTextNode();
    if (!pNd)
    {
        return OUString();
    }
    SwTextFrame const*const pFrame(static_cast<SwTextFrame*>(pNd->getLayoutFrame(GetLayout())));
    if (pFrame)
    {
        return pFrame->GetCurWord(*rPaM.GetPoint());
    }
    return OUString();
}

void SwEditShell::UpdateDocStat( )
{
    StartAllAction();
    GetDoc()->getIDocumentStatistics().UpdateDocStat( false, true );
    EndAllAction();
}

const SwDocStat& SwEditShell::GetUpdatedDocStat()
{
    StartAllAction();
    const SwDocStat &rRet = GetDoc()->getIDocumentStatistics().GetUpdatedDocStat( false, true );
    EndAllAction();
    return rRet;
}

/// get the reference of a given name in the Doc
const SwFormatRefMark* SwEditShell::GetRefMark( const SwMarkName& rName ) const
{
    return GetDoc()->GetRefMark( rName );
}

/// get the names of all references in a Doc
sal_uInt16 SwEditShell::GetRefMarks( std::vector<OUString>* pStrings ) const
{
    return GetDoc()->GetRefMarks( pStrings );
}

OUString SwEditShell::GetDropText( const sal_Int32 nChars ) const
{
    /*
     * pb: made changes for #i74939#
     *
     * always return a string even though there is a selection
     */

    OUString aText;
    SwPaM* pCursor = GetCursor();
    if ( IsMultiSelection() )
    {
        // if a multi selection exists, search for the first line
        // -> it is the cursor with the lowest index
        SwNodeOffset nIndex = pCursor->GetMark()->GetNodeIndex();
        bool bPrev = true;
        SwPaM* pLast = pCursor;
        SwPaM* pTemp = pCursor;
        while ( bPrev )
        {
            SwPaM* pPrev2 = pTemp->GetPrev();
            bPrev = ( pPrev2 && pPrev2 != pLast );
            if ( bPrev )
            {
                pTemp = pPrev2;
                SwNodeOffset nTemp = pPrev2->GetMark()->GetNodeIndex();
                if ( nTemp < nIndex )
                {
                    nIndex = nTemp;
                    pCursor = pPrev2;
                }
            }
        }
    }

    SwTextNode const*const pTextNd = pCursor->GetMarkNode().GetTextNode();
    if( pTextNd )
    {
        SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(
            pTextNd->getLayoutFrame(GetLayout())));
        SAL_WARN_IF(!pTextFrame, "sw.core", "GetDropText cursor has no frame?");
        if (pTextFrame)
        {
            TextFrameIndex const nDropLen(pTextFrame->GetDropLen(TextFrameIndex(nChars)));
            aText = pTextFrame->GetText().copy(0, sal_Int32(nDropLen));
        }
    }

    return aText;
}

void SwEditShell::ReplaceDropText( const OUString &rStr, SwPaM* pPaM )
{
    SwPaM* pCursor = pPaM ? pPaM : GetCursor();
    if( !(pCursor->GetPoint()->GetNode() == pCursor->GetMark()->GetNode() &&
        pCursor->GetPointNode().GetTextNode()->IsTextNode()) )
        return;

    StartAllAction();

    const SwNode& rNd = pCursor->GetPoint()->GetNode();
    SwPaM aPam( rNd, rStr.getLength(), rNd, 0 );
    SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(
        rNd.GetTextNode()->getLayoutFrame(GetLayout())));
    if (pTextFrame)
    {
        *aPam.GetPoint() = pTextFrame->MapViewToModelPos(TextFrameIndex(0));
        *aPam.GetMark() = pTextFrame->MapViewToModelPos(TextFrameIndex(
            std::min(rStr.getLength(), pTextFrame->GetText().getLength())));
    }
    if( !GetDoc()->getIDocumentContentOperations().Overwrite( aPam, rStr ) )
    {
        OSL_FAIL( "Doc->getIDocumentContentOperations().Overwrite(Str) failed." );
    }

    EndAllAction();
}

OUString SwEditShell::Calculate()
{
    OUStringBuffer aFormel;                    // the final formula
    SwCalc    aCalc( *GetDoc() );
    const CharClass& rCC = GetAppCharClass();

    for(SwPaM& rCurrentPaM : GetCursor()->GetNext()->GetRingContainer())
    {
        SwTextNode* pTextNd = rCurrentPaM.GetPointNode().GetTextNode();
        if(pTextNd)
        {
            const SwPosition *pStart = rCurrentPaM.Start(), *pEnd = rCurrentPaM.End();
            const sal_Int32 nStt = pStart->GetContentIndex();
            OUString aStr = pTextNd->GetExpandText(GetLayout(),
                                nStt, pEnd->GetContentIndex() - nStt);

            aStr = rCC.lowercase( aStr );

            bool bValidFields = false;
            sal_Int32 nPos = 0;

            while( nPos < aStr.getLength() )
            {
                sal_Unicode ch = aStr[ nPos++ ];
                if( rCC.isLetter( aStr, nPos-1 ) || ch == '_' )
                {
                    sal_Int32 nTmpStt = nPos-1;
                    while(  nPos < aStr.getLength() &&
                            0 != ( ch = aStr[ nPos++ ]) &&
                           (rCC.isLetterNumeric( aStr, nPos - 1 ) ||
                               ch == '_'|| ch == '.' ))
                        ;

                    if( nPos < aStr.getLength() )
                        --nPos;

                    OUString sVar = aStr.copy( nTmpStt, nPos - nTmpStt );
                    if( !::FindOperator( sVar ) &&
                        (aCalc.GetVarTable().find(sVar) != aCalc.GetVarTable().end() ||
                         aCalc.VarLook( sVar )) )
                    {
                        if( !bValidFields )
                        {
                            GetDoc()->getIDocumentFieldsAccess().FieldsToCalc( aCalc,
                                                  pStart->GetNodeIndex(),
                                                  pStart->GetContentIndex() );
                            bValidFields = true;
                        }
                        aFormel.append("(" + aCalc.GetStrResult( aCalc.VarLook( sVar )->nValue ) + ")");
                    }
                    else
                        aFormel.append(sVar);
                }
                else
                    aFormel.append(ch);
            }
        }
    }

    return aCalc.GetStrResult( aCalc.Calculate(aFormel.makeStringAndClear()) );
}

sfx2::LinkManager& SwEditShell::GetLinkManager()
{
    return mxDoc->getIDocumentLinksAdministration().GetLinkManager();
}

void *SwEditShell::GetIMapInventor() const
{
    // The node on which the cursor points should be sufficient as a unique identifier
    return static_cast<void*>(&(GetCursor()->GetPointNode()));
}

// #i73788#
Graphic SwEditShell::GetIMapGraphic() const
{
    // returns always a graphic if the cursor is in a Fly
    CurrShell aCurr( const_cast<SwEditShell*>(this) );
    Graphic aRet;
    SwPaM* pCursor = GetCursor();
    if ( !pCursor->HasMark() )
    {
        SwNode& rNd =pCursor->GetPointNode();
        if( rNd.IsGrfNode() )
        {
            SwGrfNode & rGrfNode(static_cast<SwGrfNode&>(rNd));
            aRet = rGrfNode.GetGrf(GraphicType::Default == rGrfNode.GetGrf().GetType());
        }
        else if ( rNd.IsOLENode() )
        {
            if (const Graphic* pGraphic = static_cast<SwOLENode&>(rNd).GetGraphic())
                aRet = *pGraphic;
        }
        else
        {
            SwFlyFrame* pFlyFrame = rNd.GetContentNode()->getLayoutFrame( GetLayout() )->FindFlyFrame();
            if(pFlyFrame)
                aRet = pFlyFrame->GetFormat()->MakeGraphic();
        }
    }
    return aRet;
}

bool SwEditShell::InsertURL( const SwFormatINetFormat& rFormat, const OUString& rStr, bool bKeepSelection )
{
    // URL and hint text (directly or via selection) necessary
    if( rFormat.GetValue().isEmpty() || ( rStr.isEmpty() && !HasSelection() ) )
        return false;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::UI_INSERT_URLTXT, nullptr);
    bool bInsText = true;

    if( !rStr.isEmpty() )
    {
        SwPaM* pCursor = GetCursor();
        if( pCursor->HasMark() && *pCursor->GetPoint() != *pCursor->GetMark() )
        {
            // Selection existent, multi selection?
            bool bDelText = true;
            if( !pCursor->IsMultiSelection() )
            {
                // simple selection -> check the text
                const OUString sText(comphelper::string::stripEnd(GetSelText(), ' '));
                if( sText == rStr )
                    bDelText = bInsText = false;
            }
            else if( rFormat.GetValue() == rStr ) // Are Name and URL equal?
                bDelText = bInsText = false;

            if( bDelText )
                Delete(true);
        }
        else if( pCursor->IsMultiSelection() && rFormat.GetValue() == rStr )
            bInsText = false;

        if( bInsText )
        {
            Insert2( rStr );
            SetMark();
            ExtendSelection( false, rStr.getLength() );
        }
    }
    else
        bInsText = false;

    SetAttrItem( rFormat );
    if (bInsText && !IsCursorPtAtEnd())
        SwapPam();
    if(!bKeepSelection)
        ClearMark();
    if( bInsText )
        DontExpandFormat();
    GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::UI_INSERT_URLTXT, nullptr );
    EndAllAction();
    return true;
}

void SwEditShell::GetINetAttrs(SwGetINetAttrs& rArr, bool bIncludeInToxContent)
{
    rArr.clear();

    const SwCharFormats* pFormats = GetDoc()->GetCharFormats();
    for( auto n = pFormats->size(); 1 < n; )
    {
        SwIterator<SwTextINetFormat,SwCharFormat> aIter(*(*pFormats)[--n]);
        for( SwTextINetFormat* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
        {
            SwTextNode const*const pTextNd(pFnd->GetpTextNode());
            SwTextFrame const*const pFrame(pTextNd
                ? static_cast<SwTextFrame const*>(pTextNd->getLayoutFrame(GetLayout()))
                : nullptr);
            if (nullptr != pTextNd && nullptr != pFrame
                && pTextNd->GetNodes().IsDocNodes()
                // check it's not fully deleted
                && pFrame->MapModelToView(pTextNd, pFnd->GetStart())
                    != pFrame->MapModelToView(pTextNd, *pFnd->GetEnd()))
            {
                // tdf#52113, tdf#148312 Don't include table of contents hyperlinks in the
                // Navigator content tree Hyperlinks entries
                if (!bIncludeInToxContent)
                {
                    if(const SwSectionNode* pSectNd = pTextNd->FindSectionNode())
                    {
                        SectionType eType = pSectNd->GetSection().GetType();
                        if(SectionType::ToxContent == eType)
                            continue;
                    }
                }

                SwTextINetFormat& rAttr = *pFnd;
                OUString sText( pTextNd->GetExpandText(GetLayout(),
                        rAttr.GetStart(), *rAttr.GetEnd() - rAttr.GetStart()) );

                sText = sText.replaceAll("\x0a", "");
                sText = comphelper::string::strip(sText, ' ');

                if( !sText.isEmpty() )
                {
                    rArr.emplace_back(sText, rAttr);
                }
            }
        }
    }
}

/// If the cursor is in an INetAttribute then it will be deleted completely (incl. hint text, the
/// latter is needed for drag & drop)
void SwEditShell::DelINetAttrWithText()
{
    bool bRet = SelectTextAttr( RES_TXTATR_INETFMT, false );
    if( bRet )
        DeleteSel(*GetCursor(), true);
}

/// Set the DontExpand flag at the text character attributes
bool SwEditShell::DontExpandFormat()
{
    bool bRet = false;
    if( !IsTableMode() && GetDoc()->DontExpandFormat( *GetCursor()->GetPoint() ))
    {
        bRet = true;
        CallChgLnk();
    }
    return bRet;
}

SvNumberFormatter* SwEditShell::GetNumberFormatter()
{
    return GetDoc()->GetNumberFormatter();
}

bool SwEditShell::ConvertFieldsToText()
{
    StartAllAction();
    bool bRet = GetDoc()->ConvertFieldsToText(*GetLayout());
    EndAllAction();
    return bRet;
}

void SwEditShell::SetNumberingRestart()
{
    StartAllAction();
    Push();
    // iterate over all text contents - body, frames, header, footer, footnote text
    SwPaM* pCursor = GetCursor();
    for(int i = 0; i < 2; i++)
    {
        if(!i)
            MakeFindRange(SwDocPositions::Start, SwDocPositions::End, pCursor); // body content
        else
            MakeFindRange(SwDocPositions::OtherStart, SwDocPositions::OtherEnd, pCursor); // extra content
        SwPosition* pSttPos = pCursor->Start(), *pEndPos = pCursor->End();
        SwNodeOffset nCurrNd = pSttPos->GetNodeIndex();
        SwNodeOffset nEndNd = pEndPos->GetNodeIndex();
        if( nCurrNd <= nEndNd )
        {
            SwContentFrame* pContentFrame;
            bool bGoOn = true;
            // iterate over all paragraphs
            while( bGoOn )
            {
                SwNode* pNd = GetDoc()->GetNodes()[ nCurrNd ];
                switch( pNd->GetNodeType() )
                {
                case SwNodeType::Text:
                    pContentFrame = static_cast<SwTextNode*>(pNd)->getLayoutFrame( GetLayout() );
                    if( nullptr != pContentFrame )
                    {
                        // skip hidden frames - ignore protection!
                        if( !pContentFrame->IsHiddenNow() )
                        {
                            // if the node is numbered and the starting value of the numbering equals the
                            // start value of the numbering rule then set this value as hard starting value

                            // get the node num
                            // OD 2005-11-09
                            SwTextNode* pTextNd( pNd->GetTextNode() );
                            SwNumRule* pNumRule( pTextNd->GetNumRule() );

                            // sw_redlinehide: not sure what this should do, only called from mail-merge
                            bool bIsNodeNum =
                               ( pNumRule && pTextNd->GetNum() &&
                                 ( pTextNd->HasNumber() || pTextNd->HasBullet() ) &&
                                 pTextNd->IsCountedInList() &&
                                 !pTextNd->IsListRestart() );
                            if (bIsNodeNum)
                            {
                                int nListLevel = pTextNd->GetActualListLevel();

                                if (nListLevel < 0)
                                    nListLevel = 0;

                                if (nListLevel >= MAXLEVEL)
                                    nListLevel = MAXLEVEL - 1;

                                bIsNodeNum = pTextNd->GetNum()->GetNumber() ==
                                    pNumRule->Get( o3tl::narrowing<sal_uInt16>(nListLevel) ).GetStart();
                            }
                            if (bIsNodeNum)
                            {
                                // now set the start value as attribute
                                SwPosition aCurrentNode(*pNd);
                                GetDoc()->SetNumRuleStart( aCurrentNode );
                            }
                        }
                    }
                    break;
                case SwNodeType::Section:
                    // skip hidden sections  - ignore protection!
                    if(static_cast<SwSectionNode*>(pNd)->GetSection().IsHidden() )
                        nCurrNd = pNd->EndOfSectionIndex();
                    break;
                default: break;
                }

                bGoOn = nCurrNd < nEndNd;
                ++nCurrNd;
            }
        }
    }

    Pop(PopMode::DeleteCurrent);
    EndAllAction();
}

sal_Int32 SwEditShell::GetLineCount()
{
    sal_Int32 nRet = 0;
    CalcLayout();
    SwPaM* pPam = GetCursor();
    SwNodeIndex aStart( pPam->GetPoint()->GetNode() );
    SwContentNode* pCNd;
    SwContentFrame *pContentFrame = nullptr;

    aStart = SwNodeOffset(0);

    while (nullptr != (pCNd = SwNodes::GoNextSection(
                &aStart, true, false )) )
    {
        if( nullptr != ( pContentFrame = pCNd->getLayoutFrame( GetLayout() ) ) && pContentFrame->IsTextFrame() )
        {
            SwTextFrame *const pFrame(static_cast<SwTextFrame*>(pContentFrame));
            nRet = nRet + pFrame->GetLineCount(TextFrameIndex(COMPLETE_STRING));
            if (GetLayout()->HasMergedParas())
            {
                if (auto const*const pMerged = pFrame->GetMergedPara())
                {
                    aStart = *pMerged->pLastNode;
                }
            }
        }
    }
    return nRet;
}

tools::Long SwEditShell::CompareDoc( const SwDoc& rDoc )
{
    StartAllAction();
    tools::Long nRet = GetDoc()->CompareDoc( rDoc );
    EndAllAction();
    return nRet;
}

tools::Long SwEditShell::MergeDoc( const SwDoc& rDoc )
{
    StartAllAction();
    tools::Long nRet = GetDoc()->MergeDoc( rDoc );
    EndAllAction();
    return nRet;
}

const SwFootnoteInfo& SwEditShell::GetFootnoteInfo() const
{
    return GetDoc()->GetFootnoteInfo();
}

void SwEditShell::SetFootnoteInfo(const SwFootnoteInfo& rInfo)
{
    StartAllAction();
    CurrShell aCurr( this );
    GetDoc()->SetFootnoteInfo(rInfo);
    CallChgLnk();
    EndAllAction();
}

const SwEndNoteInfo& SwEditShell::GetEndNoteInfo() const
{
    return GetDoc()->GetEndNoteInfo();
}

void SwEditShell::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
{
    StartAllAction();
    CurrShell aCurr( this );
    GetDoc()->SetEndNoteInfo(rInfo);
    EndAllAction();
}

const SwLineNumberInfo& SwEditShell::GetLineNumberInfo() const
{
    return GetDoc()->GetLineNumberInfo();
}

void SwEditShell::SetLineNumberInfo(const SwLineNumberInfo& rInfo)
{
    StartAllAction();
    CurrShell aCurr( this );
    GetDoc()->SetLineNumberInfo(rInfo);
    AddPaintRect( GetLayout()->getFrameArea() );
    EndAllAction();
}

sal_uInt16 SwEditShell::GetLinkUpdMode() const
{
    return getIDocumentSettingAccess().getLinkUpdateMode( false );
}

void SwEditShell::SetLinkUpdMode( sal_uInt16 nMode )
{
    getIDocumentSettingAccess().setLinkUpdateMode( nMode );
}

// Interface for TextInputData - (for text input of japanese/chinese characters)
void SwEditShell::CreateExtTextInput(LanguageType eInputLanguage)
{
    SwExtTextInput* pRet = GetDoc()->CreateExtTextInput( *GetCursor() );
    pRet->SetLanguage(eInputLanguage);
    pRet->SetOverwriteCursor( SwCursorShell::IsOverwriteCursor() );
}

OUString SwEditShell::DeleteExtTextInput( bool bInsText )
{
    const SwPosition& rPos = *GetCursor()->GetPoint();
    SwExtTextInput* pDel = GetDoc()->GetExtTextInput( rPos.GetNode(),
                                      rPos.GetContentIndex() );
    if( !pDel )
    {
        //JP 25.10.2001: under UNIX the cursor is moved before the Input-
        //              Engine event comes in. So take any - normally there
        //              exist only one at the time. -- Task 92016
        pDel = GetDoc()->GetExtTextInput();
    }
    OUString sRet;
    if( pDel )
    {
        OUString sTmp;
        SwUnoCursorHelper::GetTextFromPam(*pDel, sTmp);
        sRet = sTmp;
        CurrShell aCurr( this );
        StartAllAction();
        pDel->SetInsText( bInsText );
        SetOverwriteCursor( pDel->IsOverwriteCursor() );
        const SwPosition aPos( *pDel->GetPoint() );
        GetDoc()->DeleteExtTextInput( pDel );

        // In this case, the "replace" function did not set the cursor
        // to the original position. Therefore we have to do this manually.
        if ( ! bInsText && IsOverwriteCursor() )
            *GetCursor()->GetPoint() = aPos;

        EndAllAction();
    }
    return sRet;
}

void SwEditShell::SetExtTextInputData( const CommandExtTextInputData& rData )
{
    SwPaM* pCurrentCursor = GetCursor();
    const SwPosition& rPos = *pCurrentCursor->GetPoint();
    SwExtTextInput* pInput = GetDoc()->GetExtTextInput( rPos.GetNode() );
    if( !pInput )
        return;

    StartAllAction();
    CurrShell aCurr( this );

    if( !rData.IsOnlyCursorChanged() )
        pInput->SetInputData( rData );
    // position cursor
    const SwPosition& rStt = *pInput->Start();
    const sal_Int32 nNewCursorPos = rStt.GetContentIndex() + rData.GetCursorPos();

    // ugly but works
    ShowCursor();
    const sal_Int32 nDiff = nNewCursorPos - rPos.GetContentIndex();
    if( nDiff != 0)
    {
        bool bLeft = nDiff < 0;
        sal_Int32 nMaxGuard = std::abs(nDiff);
        while (true)
        {
            auto nOldPos = pCurrentCursor->GetPoint()->GetContentIndex();
            if (bLeft)
                Left(1, SwCursorSkipMode::Chars);
            else
                Right(1, SwCursorSkipMode::Chars);
            auto nNewPos = pCurrentCursor->GetPoint()->GetContentIndex();

            // expected success
            if (nNewPos == nNewCursorPos)
                break;

            if (nNewPos == nOldPos)
            {
                // if there was no movement, we have failed for some reason
                SAL_WARN("sw.core", "IM cursor move failed");
                break;
            }

            if (--nMaxGuard == 0)
            {
                // if it takes more cursor moves than there are utf-16 chars to move past
                // something has probably gone wrong
                SAL_WARN("sw.core", "IM abandoning cursor positioning");
                break;
            }
        }
    }

    SetOverwriteCursor( rData.IsCursorOverwrite() );

    EndAllAction();

    if( !rData.IsCursorVisible() )  // must be called after the EndAction
        HideCursor();

}

void SwEditShell::TransliterateText( TransliterationFlags nType )
{
    utl::TransliterationWrapper aTrans( ::comphelper::getProcessComponentContext(), nType );
    StartAllAction();
    CurrShell aCurr( this );

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
        for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            if( rPaM.HasMark() )
                GetDoc()->getIDocumentContentOperations().TransliterateText( rPaM, aTrans );
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
    }
    else
        GetDoc()->getIDocumentContentOperations().TransliterateText( *pCursor, aTrans );

    EndAllAction();
}

void SwEditShell::CountWords( SwDocStat& rStat ) const
{
    for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( rPaM.HasMark() )
            SwDoc::CountWords( rPaM, rStat );

    }
}

void SwEditShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwCursorShell::StartAction();
    SwViewShell::ApplyViewOptions( rOpt );
    SwEditShell::EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
