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
#include "WizardPages.hrc"
#include <vcl/scrbar.hxx>
#include <com/sun/star/sdbc/DataType.hpp>

using namespace ::dbaui;
//========================================================================
// OWizColumnSelect
DBG_NAME(OWizNameMatching)
//========================================================================
OWizNameMatching::OWizNameMatching( Window* pParent)
        :OWizardPage( pParent, ModuleRes( TAB_WIZ_NAME_MATCHING     ) )
        ,m_FT_TABLE_LEFT(       this, ModuleRes( FT_TABLE_LEFT          ) )
        ,m_FT_TABLE_RIGHT(      this, ModuleRes( FT_TABLE_RIGHT         ) )
        ,m_CTRL_LEFT(           this, ModuleRes( CTRL_LEFT              ) )
        ,m_CTRL_RIGHT(          this, ModuleRes( CTRL_RIGHT             ) )
        ,m_ibColumn_up(         this, ModuleRes( IB_COLUMN_UP           ) )
        ,m_ibColumn_down(       this, ModuleRes( IB_COLUMN_DOWN         ) )
        ,m_ibColumn_up_right(   this, ModuleRes( IB_COLUMN_UP_RIGHT     ) )
        ,m_ibColumn_down_right( this, ModuleRes( IB_COLUMN_DOWN_RIGHT   ) )
        ,m_pbAll(               this, ModuleRes( PB_ALL                 ) )
        ,m_pbNone(              this, ModuleRes( PB_NONE                    ) )

{
    DBG_CTOR(OWizNameMatching,NULL);

    m_ibColumn_up.SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));
    m_ibColumn_down.SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));

    m_ibColumn_up_right.SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));
    m_ibColumn_down_right.SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));

    m_pbAll.SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));
    m_pbNone.SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));

    m_CTRL_LEFT.SetSelectHdl(LINK(this,OWizNameMatching,TableListClickHdl));
    m_CTRL_RIGHT.SetSelectHdl(LINK(this,OWizNameMatching,TableListRightSelectHdl));
    m_CTRL_RIGHT.EnableCheckButton( NULL );

    m_CTRL_LEFT.SetStyle( m_CTRL_LEFT.GetStyle() | WB_FORCE_MAKEVISIBLE );
    m_CTRL_RIGHT.SetStyle( m_CTRL_RIGHT.GetStyle() | WB_FORCE_MAKEVISIBLE );

    m_sSourceText   = m_FT_TABLE_LEFT.GetText();
    m_sSourceText.AppendAscii("\n");
    m_sDestText     = m_FT_TABLE_RIGHT.GetText();
    m_sDestText.AppendAscii("\n");

    FreeResource();
}
// -----------------------------------------------------------------------
OWizNameMatching::~OWizNameMatching()
{
    DBG_DTOR(OWizNameMatching,NULL);
}

// -----------------------------------------------------------------------
void OWizNameMatching::Reset()
{
    // urspr"unglichen zustand wiederherstellen
    DBG_CHKTHIS(OWizNameMatching,NULL);
    // the left tree contains bitmaps so i need to resize the right one
    if(m_bFirstTime)
    {
        m_CTRL_RIGHT.SetReadOnly(); // sets autoinc to readonly
        m_CTRL_RIGHT.SetEntryHeight(m_CTRL_LEFT.GetEntryHeight());
        m_CTRL_RIGHT.SetIndent(m_CTRL_LEFT.GetIndent());
        m_CTRL_RIGHT.SetSpaceBetweenEntries(m_CTRL_LEFT.GetSpaceBetweenEntries());

        m_bFirstTime = sal_False;
    }

}
// -----------------------------------------------------------------------
void OWizNameMatching::ActivatePage( )
{
    DBG_CHKTHIS(OWizNameMatching,NULL);

    // set source table name
    String aName = m_sSourceText;
    aName += String(m_pParent->m_sSourceName);

    m_FT_TABLE_LEFT.SetText(aName);

    // set dest table name
    aName = m_sDestText;
    aName += String(m_pParent->m_sName);
    m_FT_TABLE_RIGHT.SetText(aName);


    m_CTRL_LEFT.FillListBox(*m_pParent->getSrcVector());
    m_CTRL_RIGHT.FillListBox(*m_pParent->getDestVector());

    m_ibColumn_up.Enable( m_CTRL_LEFT.GetEntryCount() > 1 );
    m_ibColumn_down.Enable( m_CTRL_LEFT.GetEntryCount() > 1 );

    m_ibColumn_up_right.Enable( m_CTRL_RIGHT.GetEntryCount() > 1 );
    m_ibColumn_down_right.Enable( m_CTRL_RIGHT.GetEntryCount() > 1 );


    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_False);
    m_CTRL_LEFT.GrabFocus();
}
// -----------------------------------------------------------------------
sal_Bool OWizNameMatching::LeavePage()
{
    DBG_CHKTHIS(OWizNameMatching,NULL);

    const ODatabaseExport::TColumnVector* pSrcColumns = m_pParent->getSrcVector();

    m_pParent->m_vColumnPos.clear();
    m_pParent->m_vColumnTypes.clear();
    m_pParent->m_vColumnPos.resize( pSrcColumns->size(), ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
    m_pParent->m_vColumnTypes.resize( pSrcColumns->size(), COLUMN_POSITION_NOT_FOUND );


    sal_Int32 nParamPos = 0;
    SvLBoxEntry* pLeftEntry = m_CTRL_LEFT.GetModel()->First();
    SvLBoxEntry* pRightEntry = m_CTRL_RIGHT.GetModel()->First();
    while(pLeftEntry && pRightEntry)
    {
        OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeftEntry->GetUserData());
        OSL_ENSURE(pSrcField,"OWizNameMatching: OColumn can not be null!");

        ODatabaseExport::TColumnVector::const_iterator aSrcIter = pSrcColumns->begin();
        ODatabaseExport::TColumnVector::const_iterator aSrcEnd  = pSrcColumns->end();
        for(;aSrcIter != aSrcEnd && (*aSrcIter)->second != pSrcField;++aSrcIter)
            ;
        const sal_Int32 nPos = ::std::distance(pSrcColumns->begin(),aSrcIter);

        if(m_CTRL_LEFT.GetCheckButtonState(pLeftEntry) == SV_BUTTON_CHECKED)
        {
            OFieldDescription* pDestField = static_cast<OFieldDescription*>(pRightEntry->GetUserData());
            OSL_ENSURE(pDestField,"OWizNameMatching: OColumn can not be null!");
            const ODatabaseExport::TColumnVector* pDestColumns          = m_pParent->getDestVector();
            ODatabaseExport::TColumnVector::const_iterator aDestIter    = pDestColumns->begin();
            ODatabaseExport::TColumnVector::const_iterator aDestEnd = pDestColumns->end();

            for(;aDestIter != aDestEnd && (*aDestIter)->second != pDestField;++aDestIter)
                ;

            OSL_ENSURE((nPos) < static_cast<sal_Int32>(m_pParent->m_vColumnPos.size()),"m_pParent->m_vColumnPos: Illegal index for vector");
            m_pParent->m_vColumnPos[nPos].first = ++nParamPos;
            m_pParent->m_vColumnPos[nPos].second = ::std::distance(pDestColumns->begin(),aDestIter) + 1;
            sal_Bool bNotConvert = sal_True;
            TOTypeInfoSP pTypeInfo = m_pParent->convertType((*aDestIter)->second->getSpecialTypeInfo(),bNotConvert);
            sal_Int32 nType = ::com::sun::star::sdbc::DataType::VARCHAR;
            if ( pTypeInfo.get() )
                nType = pTypeInfo->nType;
            m_pParent->m_vColumnTypes[nPos] = nType;
        }
        else
        {
            m_pParent->m_vColumnPos[nPos].first = COLUMN_POSITION_NOT_FOUND;
            m_pParent->m_vColumnPos[nPos].second = COLUMN_POSITION_NOT_FOUND;
        }

        pLeftEntry = m_CTRL_LEFT.GetModel()->Next(pLeftEntry);
        pRightEntry = m_CTRL_RIGHT.GetModel()->Next(pRightEntry);
    }

    return sal_True;
}
// -----------------------------------------------------------------------
String OWizNameMatching::GetTitle() const { return String(ModuleRes(STR_WIZ_NAME_MATCHING_TITEL)); }
// -----------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, ButtonClickHdl, Button *, pButton )
{
    SvLBoxEntry* pEntry = m_CTRL_LEFT.FirstSelected();
    if ( pEntry )
    {
        sal_Int32 nPos      = m_CTRL_LEFT.GetModel()->GetAbsPos(pEntry);
        if(pButton == &m_ibColumn_up && nPos)
            --nPos;
        else if(pButton == &m_ibColumn_down)
            nPos += 2;

        m_CTRL_LEFT.ModelIsMoving(pEntry,NULL,nPos);
        m_CTRL_LEFT.GetModel()->Move(pEntry,NULL,nPos);
        m_CTRL_LEFT.ModelHasMoved(pEntry);

        long nThumbPos      = m_CTRL_LEFT.GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_CTRL_LEFT.GetVScroll()->GetVisibleSize();

        if(pButton == &m_ibColumn_down && (nThumbPos+nVisibleSize+1) < nPos)
        {
            m_CTRL_LEFT.GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
        }

        TableListClickHdl(&m_CTRL_LEFT);
    }


    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, RightButtonClickHdl, Button *, pButton )
{
    SvLBoxEntry* pEntry = m_CTRL_RIGHT.FirstSelected();
    if ( pEntry )
    {
        sal_Int32 nPos      = m_CTRL_RIGHT.GetModel()->GetAbsPos(pEntry);
        if(pButton == &m_ibColumn_up_right && nPos)
            --nPos;
        else if(pButton == &m_ibColumn_down_right)
            nPos += 2;

        m_CTRL_RIGHT.ModelIsMoving(pEntry,NULL,nPos);
        m_CTRL_RIGHT.GetModel()->Move(pEntry,NULL,nPos);
        m_CTRL_RIGHT.ModelHasMoved(pEntry);
        long nThumbPos      = m_CTRL_RIGHT.GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_CTRL_RIGHT.GetVScroll()->GetVisibleSize();

        if(pButton == &m_ibColumn_down_right && (nThumbPos+nVisibleSize+1) < nPos)
            m_CTRL_RIGHT.GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
        TableListRightSelectHdl(&m_CTRL_RIGHT);
    }
    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, TableListClickHdl, void*, /*NOTINTERESTEDIN*/ )
{
    SvLBoxEntry* pEntry = m_CTRL_LEFT.FirstSelected();
    if(pEntry)
    {
        sal_uLong nPos          = m_CTRL_LEFT.GetModel()->GetAbsPos(pEntry);
        SvLBoxEntry* pOldEntry = m_CTRL_RIGHT.FirstSelected();
        if(pOldEntry && nPos != m_CTRL_RIGHT.GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_CTRL_RIGHT.Select(pOldEntry,sal_False);
            pOldEntry = m_CTRL_RIGHT.GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uLong nNewPos = m_CTRL_LEFT.GetModel()->GetAbsPos(m_CTRL_LEFT.GetFirstEntryInView());
                if ( nNewPos - nPos == 1 )
                    --nNewPos;
                m_CTRL_RIGHT.MakeVisible(m_CTRL_RIGHT.GetEntry(nNewPos),sal_True);
                m_CTRL_RIGHT.Select(pOldEntry,sal_True);
            }
        }
        else if(!pOldEntry)
        {
            pOldEntry = m_CTRL_RIGHT.GetEntry(nPos);
            if(pOldEntry)
            {
                m_CTRL_RIGHT.Select(pOldEntry,sal_True);
            }
        }
    }

    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, TableListRightSelectHdl, void*, /*NOTINTERESTEDIN*/ )
{
    SvLBoxEntry* pEntry = m_CTRL_RIGHT.FirstSelected();
    if(pEntry)
    {
        sal_uLong nPos          = m_CTRL_RIGHT.GetModel()->GetAbsPos(pEntry);
        SvLBoxEntry* pOldEntry = m_CTRL_LEFT.FirstSelected();
        if(pOldEntry && nPos != m_CTRL_LEFT.GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_CTRL_LEFT.Select(pOldEntry,sal_False);
            pOldEntry = m_CTRL_LEFT.GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uLong nNewPos = m_CTRL_RIGHT.GetModel()->GetAbsPos(m_CTRL_RIGHT.GetFirstEntryInView());
                if ( nNewPos - nPos == 1 )
                    nNewPos--;
                m_CTRL_LEFT.MakeVisible(m_CTRL_LEFT.GetEntry(nNewPos),sal_True);
                m_CTRL_LEFT.Select(pOldEntry,sal_True);
            }
        }
        else if(!pOldEntry)
        {
            pOldEntry = m_CTRL_LEFT.GetEntry(nPos);
            if(pOldEntry)
            {
                m_CTRL_LEFT.Select(pOldEntry,sal_True);
            }
        }
    }

    return 0;
}
// -----------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, AllNoneClickHdl, Button *, pButton )
{
    sal_Bool bAll = pButton == &m_pbAll;
    SvLBoxEntry* pEntry = m_CTRL_LEFT.First();
    while(pEntry)
    {
        m_CTRL_LEFT.SetCheckButtonState( pEntry, bAll ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED);
        pEntry = m_CTRL_LEFT.Next(pEntry);
    }

    return 0;
}
// -----------------------------------------------------------------------
//========================================================================
// class OColumnString
//========================================================================
class OColumnString : public SvLBoxString
{
    sal_Bool m_bReadOnly;
public:
    OColumnString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr,sal_Bool _RO)
        :SvLBoxString(pEntry,nFlags,rStr)
        ,m_bReadOnly(_RO)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
};
//------------------------------------------------------------------------
void OColumnString::Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /*nFlags*/, SvLBoxEntry* /*pEntry*/ )
{
    if(m_bReadOnly)
    {
        const StyleSettings& rStyleSettings = rDev.GetSettings().GetStyleSettings();
        rDev.SetTextColor( rStyleSettings.GetDisableColor() );
        rDev.SetTextFillColor( rStyleSettings.GetFieldColor() );
    }
    rDev.DrawText( rPos, GetText() );
}
//========================================================================
OColumnTreeBox::OColumnTreeBox( Window* pParent, const ResId& rResId )
    : OMarkableTreeListBox(pParent,NULL,rResId)
{
    SetDragDropMode( 0 );
    EnableInplaceEditing( sal_False );
    SetStyle(GetStyle() | WB_BORDER | WB_HASBUTTONS | WB_HSCROLL);
    SetSelectionMode( SINGLE_SELECTION );
}
//------------------------------------------------------------------------
void OColumnTreeBox::InitEntry(SvLBoxEntry* pEntry, const String& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind)
{
    DBTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pString = new OColumnString(pEntry, 0, rStr,sal_False);
    if (pString)
        pEntry->ReplaceItem( pString, pEntry->ItemCount() - 1 );
}
//------------------------------------------------------------------------
sal_Bool OColumnTreeBox::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
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
//------------------------------------------------------------------------
void OColumnTreeBox::FillListBox( const ODatabaseExport::TColumnVector& _rList)
{
    Clear();
    ODatabaseExport::TColumnVector::const_iterator aIter = _rList.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = _rList.end();
    for(;aIter != aEnd;++aIter)
    {
        SvLBoxEntry* pEntry = InsertEntry((*aIter)->first,0,sal_False,LIST_APPEND,(*aIter)->second);
        SvButtonState eState = !(m_bReadOnly && (*aIter)->second->IsAutoIncrement()) ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SetCheckButtonState( pEntry, eState );
    }
}
// -----------------------------------------------------------------------------





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
