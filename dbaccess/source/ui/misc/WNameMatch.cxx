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

#include "WNameMatch.hxx"
#include <osl/diagnose.h>
#include "FieldDescriptions.hxx"
#include "WCopyTable.hxx"
#include "dbaccess_helpid.hrc"
#include "dbu_misc.hrc"
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include "svtools/treelistentry.hxx"
#include <com/sun/star/sdbc/DataType.hpp>

using namespace ::dbaui;
// OWizColumnSelect
OWizNameMatching::OWizNameMatching( vcl::Window* pParent)
        :OWizardPage( pParent, "NameMatching", "dbaccess/ui/namematchingpage.ui" )
        , m_aImgUp(ModuleRes(IMG_UP))
        , m_aImgDown(ModuleRes(IMG_DOWN))
{
    get(m_pTABLE_LEFT, "leftlabel");
    get(m_pTABLE_RIGHT, "rightlabel");
    get(m_pCTRL_LEFT, "left");
    get(m_pCTRL_RIGHT, "right");
    get(m_pColumn_up, "up");
    m_pColumn_up->SetModeImage(m_aImgUp);
    get(m_pColumn_down, "down");
    m_pColumn_down->SetModeImage(m_aImgDown);
    get(m_pColumn_up_right, "up_right");
    m_pColumn_up_right->SetModeImage(m_aImgUp);
    get(m_pColumn_down_right, "down_right");
    m_pColumn_down_right->SetModeImage(m_aImgDown);
    get(m_pAll, "all");
    get(m_pNone, "none");

    m_pColumn_up->SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));
    m_pColumn_down->SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));

    m_pColumn_up_right->SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));
    m_pColumn_down_right->SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));

    m_pAll->SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));
    m_pNone->SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));

    m_pCTRL_LEFT->SetSelectHdl(LINK(this,OWizNameMatching,TableListClickHdl));
    m_pCTRL_RIGHT->SetSelectHdl(LINK(this,OWizNameMatching,TableListRightSelectHdl));
    m_pCTRL_RIGHT->EnableCheckButton( nullptr );

    m_pCTRL_LEFT->SetStyle( m_pCTRL_LEFT->GetStyle() | WB_FORCE_MAKEVISIBLE );
    m_pCTRL_RIGHT->SetStyle( m_pCTRL_RIGHT->GetStyle() | WB_FORCE_MAKEVISIBLE );

    m_sSourceText = m_pTABLE_LEFT->GetText();
    m_sSourceText += "\n";
    m_sDestText   = m_pTABLE_RIGHT->GetText();
    m_sDestText   += "\n";
}

OWizNameMatching::~OWizNameMatching()
{
    disposeOnce();
}

void OWizNameMatching::dispose()
{
    m_pTABLE_LEFT.clear();
    m_pTABLE_RIGHT.clear();
    m_pCTRL_LEFT.clear();
    m_pCTRL_RIGHT.clear();
    m_pColumn_up.clear();
    m_pColumn_down.clear();
    m_pColumn_up_right.clear();
    m_pColumn_down_right.clear();
    m_pAll.clear();
    m_pNone.clear();
    OWizardPage::dispose();
}

void OWizNameMatching::Reset()
{
    // urspr"unglichen zustand wiederherstellen
    // the left tree contains bitmaps so i need to resize the right one
    if(m_bFirstTime)
    {
        m_pCTRL_RIGHT->SetReadOnly(); // sets autoinc to readonly
        m_pCTRL_RIGHT->SetEntryHeight(m_pCTRL_LEFT->GetEntryHeight());
        m_pCTRL_RIGHT->SetIndent(m_pCTRL_LEFT->GetIndent());
        m_pCTRL_RIGHT->SetSpaceBetweenEntries(m_pCTRL_LEFT->GetSpaceBetweenEntries());

        m_bFirstTime = false;
    }

}

void OWizNameMatching::ActivatePage( )
{

    // set source table name
    OUString aName = m_sSourceText;
    aName += m_pParent->m_sSourceName;

    m_pTABLE_LEFT->SetText(aName);

    // set dest table name
    aName = m_sDestText;
    aName += m_pParent->m_sName;
    m_pTABLE_RIGHT->SetText(aName);

    m_pCTRL_LEFT->FillListBox(m_pParent->getSrcVector());
    m_pCTRL_RIGHT->FillListBox(m_pParent->getDestVector());

    m_pColumn_up->Enable( m_pCTRL_LEFT->GetEntryCount() > 1 );
    m_pColumn_down->Enable( m_pCTRL_LEFT->GetEntryCount() > 1 );

    m_pColumn_up_right->Enable( m_pCTRL_RIGHT->GetEntryCount() > 1 );
    m_pColumn_down_right->Enable( m_pCTRL_RIGHT->GetEntryCount() > 1 );

    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,false);
    m_pCTRL_LEFT->GrabFocus();
}

bool OWizNameMatching::LeavePage()
{

    const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();

    m_pParent->m_vColumnPos.clear();
    m_pParent->m_vColumnTypes.clear();
    m_pParent->m_vColumnPos.resize( rSrcColumns.size(), ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
    m_pParent->m_vColumnTypes.resize( rSrcColumns.size(), COLUMN_POSITION_NOT_FOUND );

    sal_Int32 nParamPos = 0;
    SvTreeListEntry* pLeftEntry = m_pCTRL_LEFT->GetModel()->First();
    SvTreeListEntry* pRightEntry = m_pCTRL_RIGHT->GetModel()->First();
    while(pLeftEntry && pRightEntry)
    {
        OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeftEntry->GetUserData());
        OSL_ENSURE(pSrcField,"OWizNameMatching: OColumn can not be null!");

        ODatabaseExport::TColumnVector::const_iterator aSrcIter = rSrcColumns.begin();
        ODatabaseExport::TColumnVector::const_iterator aSrcEnd  = rSrcColumns.end();
        for(;aSrcIter != aSrcEnd && (*aSrcIter)->second != pSrcField;++aSrcIter)
            ;
        const sal_Int32 nPos = ::std::distance(rSrcColumns.begin(),aSrcIter);

        if(m_pCTRL_LEFT->GetCheckButtonState(pLeftEntry) == SV_BUTTON_CHECKED)
        {
            OFieldDescription* pDestField = static_cast<OFieldDescription*>(pRightEntry->GetUserData());
            OSL_ENSURE(pDestField,"OWizNameMatching: OColumn can not be null!");
            const ODatabaseExport::TColumnVector& rDestColumns          = m_pParent->getDestVector();
            ODatabaseExport::TColumnVector::const_iterator aDestIter    = rDestColumns.begin();
            ODatabaseExport::TColumnVector::const_iterator aDestEnd = rDestColumns.end();

            for(;aDestIter != aDestEnd && (*aDestIter)->second != pDestField;++aDestIter)
                ;

            OSL_ENSURE((nPos) < static_cast<sal_Int32>(m_pParent->m_vColumnPos.size()),"m_pParent->m_vColumnPos: Illegal index for vector");
            m_pParent->m_vColumnPos[nPos].first = ++nParamPos;
            m_pParent->m_vColumnPos[nPos].second = ::std::distance(rDestColumns.begin(),aDestIter) + 1;

            TOTypeInfoSP pTypeInfo;

            assert(aDestIter != aDestEnd);
            if (aDestIter != aDestEnd)
            {
                bool bNotConvert = true;
                pTypeInfo = m_pParent->convertType((*aDestIter)->second->getSpecialTypeInfo(), bNotConvert);
            }

            sal_Int32 nType = css::sdbc::DataType::VARCHAR;
            if ( pTypeInfo.get() )
                nType = pTypeInfo->nType;
            m_pParent->m_vColumnTypes[nPos] = nType;
        }
        else
        {
            m_pParent->m_vColumnPos[nPos].first = COLUMN_POSITION_NOT_FOUND;
            m_pParent->m_vColumnPos[nPos].second = COLUMN_POSITION_NOT_FOUND;
        }

        pLeftEntry = m_pCTRL_LEFT->GetModel()->Next(pLeftEntry);
        pRightEntry = m_pCTRL_RIGHT->GetModel()->Next(pRightEntry);
    }

    return true;
}

OUString OWizNameMatching::GetTitle() const { return ModuleRes(STR_WIZ_NAME_MATCHING_TITEL); }

IMPL_LINK_TYPED( OWizNameMatching, ButtonClickHdl, Button *, pButton, void )
{
    SvTreeListEntry* pEntry = m_pCTRL_LEFT->FirstSelected();
    if ( pEntry )
    {
        sal_Int32 nPos      = m_pCTRL_LEFT->GetModel()->GetAbsPos(pEntry);
        if(pButton == m_pColumn_up && nPos)
            --nPos;
        else if(pButton == m_pColumn_down)
            nPos += 2;

        m_pCTRL_LEFT->ModelIsMoving(pEntry,nullptr,nPos);
        m_pCTRL_LEFT->GetModel()->Move(pEntry,nullptr,nPos);
        m_pCTRL_LEFT->ModelHasMoved(pEntry);

        long nThumbPos      = m_pCTRL_LEFT->GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_pCTRL_LEFT->GetVScroll()->GetVisibleSize();

        if(pButton == m_pColumn_down && (nThumbPos+nVisibleSize+1) < nPos)
        {
            m_pCTRL_LEFT->GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
        }

        TableListClickHdl(m_pCTRL_LEFT);
    }
}

IMPL_LINK_TYPED( OWizNameMatching, RightButtonClickHdl, Button *, pButton, void )
{
    SvTreeListEntry* pEntry = m_pCTRL_RIGHT->FirstSelected();
    if ( pEntry )
    {
        sal_Int32 nPos      = m_pCTRL_RIGHT->GetModel()->GetAbsPos(pEntry);
        if(pButton == m_pColumn_up_right && nPos)
            --nPos;
        else if(pButton == m_pColumn_down_right)
            nPos += 2;

        m_pCTRL_RIGHT->ModelIsMoving(pEntry,nullptr,nPos);
        m_pCTRL_RIGHT->GetModel()->Move(pEntry,nullptr,nPos);
        m_pCTRL_RIGHT->ModelHasMoved(pEntry);
        long nThumbPos      = m_pCTRL_RIGHT->GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_pCTRL_RIGHT->GetVScroll()->GetVisibleSize();

        if(pButton == m_pColumn_down_right && (nThumbPos+nVisibleSize+1) < nPos)
            m_pCTRL_RIGHT->GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
        TableListRightSelectHdl(m_pCTRL_RIGHT);
    }
}

IMPL_LINK_NOARG_TYPED( OWizNameMatching, TableListClickHdl, SvTreeListBox*, void )
{
    SvTreeListEntry* pEntry = m_pCTRL_LEFT->FirstSelected();
    if(pEntry)
    {
        sal_uLong nPos          = m_pCTRL_LEFT->GetModel()->GetAbsPos(pEntry);
        SvTreeListEntry* pOldEntry = m_pCTRL_RIGHT->FirstSelected();
        if(pOldEntry && nPos != m_pCTRL_RIGHT->GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_pCTRL_RIGHT->Select(pOldEntry,false);
            pOldEntry = m_pCTRL_RIGHT->GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uLong nNewPos = m_pCTRL_LEFT->GetModel()->GetAbsPos(m_pCTRL_LEFT->GetFirstEntryInView());
                if ( nNewPos - nPos == 1 )
                    --nNewPos;
                m_pCTRL_RIGHT->MakeVisible(m_pCTRL_RIGHT->GetEntry(nNewPos), true);
                m_pCTRL_RIGHT->Select(pOldEntry);
            }
        }
        else if(!pOldEntry)
        {
            pOldEntry = m_pCTRL_RIGHT->GetEntry(nPos);
            if(pOldEntry)
            {
                m_pCTRL_RIGHT->Select(pOldEntry);
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED( OWizNameMatching, TableListRightSelectHdl, SvTreeListBox*, void )
{
    SvTreeListEntry* pEntry = m_pCTRL_RIGHT->FirstSelected();
    if(pEntry)
    {
        sal_uLong nPos          = m_pCTRL_RIGHT->GetModel()->GetAbsPos(pEntry);
        SvTreeListEntry* pOldEntry = m_pCTRL_LEFT->FirstSelected();
        if(pOldEntry && nPos != m_pCTRL_LEFT->GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_pCTRL_LEFT->Select(pOldEntry,false);
            pOldEntry = m_pCTRL_LEFT->GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uLong nNewPos = m_pCTRL_RIGHT->GetModel()->GetAbsPos(m_pCTRL_RIGHT->GetFirstEntryInView());
                if ( nNewPos - nPos == 1 )
                    nNewPos--;
                m_pCTRL_LEFT->MakeVisible(m_pCTRL_LEFT->GetEntry(nNewPos), true);
                m_pCTRL_LEFT->Select(pOldEntry);
            }
        }
        else if(!pOldEntry)
        {
            pOldEntry = m_pCTRL_LEFT->GetEntry(nPos);
            if(pOldEntry)
            {
                m_pCTRL_LEFT->Select(pOldEntry);
            }
        }
    }
}

IMPL_LINK_TYPED( OWizNameMatching, AllNoneClickHdl, Button *, pButton, void )
{
    bool bAll = pButton == m_pAll;
    SvTreeListEntry* pEntry = m_pCTRL_LEFT->First();
    while(pEntry)
    {
        m_pCTRL_LEFT->SetCheckButtonState( pEntry, bAll ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED);
        pEntry = m_pCTRL_LEFT->Next(pEntry);
    }
}

// class OColumnString
class OColumnString : public SvLBoxString
{
    bool m_bReadOnly;
public:
    OColumnString( SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rStr, bool _RO)
        :SvLBoxString(pEntry,nFlags,rStr)
        ,m_bReadOnly(_RO)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void OColumnString::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                          const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    rRenderContext.Push(PushFlags::TEXTCOLOR | PushFlags::TEXTFILLCOLOR);
    if(m_bReadOnly)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());
        rRenderContext.SetTextFillColor(rStyleSettings.GetFieldColor());
    }
    rRenderContext.DrawText(rPos, GetText());
    rRenderContext.Pop();
}

OColumnTreeBox::OColumnTreeBox( vcl::Window* pParent, WinBits nBits )
    : OMarkableTreeListBox(pParent, nBits)
    , m_bReadOnly(false)
{
    SetDragDropMode( DragDropMode::NONE );
    EnableInplaceEditing( false );
    SetStyle(GetStyle() | WB_BORDER | WB_HASBUTTONS | WB_HSCROLL | nBits);
    SetSelectionMode( SINGLE_SELECTION );
}

VCL_BUILDER_FACTORY(OColumnTreeBox)

void OColumnTreeBox::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind)
{
    DBTreeListBox::InitEntry(pEntry, rStr, rImg1, rImg2, eButtonKind);
    std::unique_ptr<SvLBoxString> pString(new OColumnString(pEntry, 0, rStr,false));
    pEntry->ReplaceItem(std::move(pString), pEntry->ItemCount() - 1);
}

bool OColumnTreeBox::Select( SvTreeListEntry* pEntry, bool bSelect )
{
    if(bSelect)
    {
        OFieldDescription* pColumn = static_cast<OFieldDescription*>(pEntry->GetUserData());
        if(!(pColumn->IsAutoIncrement() && m_bReadOnly))
            bSelect = DBTreeListBox::Select( pEntry,bSelect );
    }
    else
        bSelect = DBTreeListBox::Select( pEntry,bSelect );
    return bSelect;
}

void OColumnTreeBox::FillListBox( const ODatabaseExport::TColumnVector& _rList)
{
    Clear();
    ODatabaseExport::TColumnVector::const_iterator aIter = _rList.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = _rList.end();
    for(;aIter != aEnd;++aIter)
    {
        SvTreeListEntry* pEntry = InsertEntry((*aIter)->first, nullptr, false, TREELIST_APPEND, (*aIter)->second);
        SvButtonState eState = !(m_bReadOnly && (*aIter)->second->IsAutoIncrement()) ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SetCheckButtonState( pEntry, eState );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
