/*************************************************************************
 *
 *  $RCSfile: WNameMatch.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:06:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#include "WNameMatch.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif

using namespace ::dbaui;
//========================================================================
// OWizColumnSelect
DBG_NAME(OWizNameMatching);
//========================================================================
OWizNameMatching::OWizNameMatching( Window* pParent)
        :OWizardPage( pParent, ModuleRes( TAB_WIZ_NAME_MATCHING     ) )
        ,m_FT_TABLE_LEFT(       this, ModuleRes( FT_TABLE_LEFT          ) )
        ,m_FT_TABLE_RIGHT(      this, ModuleRes( FT_TABLE_RIGHT         ) )
        ,m_CTRL_LEFT(           this, ModuleRes( CTRL_LEFT              ) )
        ,m_CTRL_RIGHT(          this, ModuleRes( CTRL_RIGHT             ) )
        ,m_pbColumn_up(         this, ModuleRes( PB_COLUMN_UP           ) )
        ,m_pbColumn_down(       this, ModuleRes( PB_COLUMN_DOWN         ) )
        ,m_pbColumn_up_right(   this, ModuleRes( PB_COLUMN_UP_RIGHT     ) )
        ,m_pbColumn_down_right( this, ModuleRes( PB_COLUMN_DOWN_RIGHT   ) )
        ,m_pbAll(               this, ModuleRes( PB_ALL                 ) )
        ,m_pbNone(              this, ModuleRes( PB_NONE                    ) )

{
    DBG_CTOR(OWizNameMatching,NULL);

    m_pbColumn_up.SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));
    m_pbColumn_down.SetClickHdl(LINK(this,OWizNameMatching,ButtonClickHdl));

    m_pbColumn_up_right.SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));
    m_pbColumn_down_right.SetClickHdl(LINK(this,OWizNameMatching,RightButtonClickHdl));

    m_pbAll.SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));
    m_pbNone.SetClickHdl(LINK(this,OWizNameMatching,AllNoneClickHdl));

    m_CTRL_LEFT.SetSelectHdl(LINK(this,OWizNameMatching,TableListClickHdl));
    m_CTRL_RIGHT.SetSelectHdl(LINK(this,OWizNameMatching,TableListRightSelectHdl));
    m_CTRL_RIGHT.EnableCheckButton( NULL );

    m_CTRL_LEFT.SetWindowBits( WB_FORCE_MAKEVISIBLE );
    m_CTRL_RIGHT.SetWindowBits( WB_FORCE_MAKEVISIBLE );

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
    m_bFirstTime = sal_False;
}
// -----------------------------------------------------------------------
void OWizNameMatching::ActivatePage( )
{
    DBG_CHKTHIS(OWizNameMatching,NULL);

    // set source table name
    String aName = m_FT_TABLE_LEFT.GetText().GetToken(0,':');
    aName.AppendAscii(": \n");

    aName += String(m_pParent->m_sName);
    m_FT_TABLE_LEFT.SetText(aName);

    // set dest table name
    aName = m_FT_TABLE_RIGHT.GetText().GetToken(0,':');
    aName.AppendAscii(": \n");

    //  aName += ;
    m_FT_TABLE_RIGHT.SetText(aName);

    m_CTRL_RIGHT.SetReadOnly(); // sets autoinc to readonly
    m_CTRL_LEFT.FillListBox(*m_pParent->getSrcVector());

    // the left tree contains bitmaps so i need to resize the right one
    m_CTRL_RIGHT.SetEntryHeight(m_CTRL_LEFT.GetEntryHeight());
    m_CTRL_RIGHT.SetIndent(m_CTRL_LEFT.GetIndent());
    m_CTRL_RIGHT.SetSpaceBetweenEntries(m_CTRL_LEFT.GetSpaceBetweenEntries());
    m_CTRL_RIGHT.FillListBox(*m_pParent->getDestVector());

    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_False);
    m_CTRL_LEFT.GrabFocus();
}
// -----------------------------------------------------------------------
sal_Bool OWizNameMatching::LeavePage()
{
    DBG_CHKTHIS(OWizNameMatching,NULL);

    const ODatabaseExport::TColumnVector* pSrcColumns       = m_pParent->getSrcVector();
    ODatabaseExport::TColumnVector::const_iterator aIter    = pSrcColumns->begin();
    for(;aIter != pSrcColumns->end();++aIter)
    {
        m_pParent->m_vColumnPos.push_back(CONTAINER_ENTRY_NOTFOUND);
        m_pParent->m_vColumnTypes.push_back(0);
    }

    const ODatabaseExport::TColumns* pDestColumns = m_pParent->getSourceColumns();

    SvLBoxEntry* pLeftEntry = m_CTRL_LEFT.GetModel()->First();
    SvLBoxEntry* pRightEntry = m_CTRL_RIGHT.GetModel()->First();
    while(pLeftEntry && pRightEntry)
    {
        OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeftEntry->GetUserData());
        DBG_ASSERT(pSrcField,"OWizNameMatching: OColumn can not be null!");

        OFieldDescription* pDestField = static_cast<OFieldDescription*>(pRightEntry->GetUserData());
        DBG_ASSERT(pDestField,"OWizNameMatching: OColumn can not be null!");

        sal_Int32 nPos = m_CTRL_LEFT.GetModel()->GetAbsPos(pLeftEntry);
        if(m_CTRL_LEFT.GetCheckButtonState(pLeftEntry) == SV_BUTTON_CHECKED)
        {
            const ODatabaseExport::TColumnVector* pDestColumns      = m_pParent->getDestVector();
            ODatabaseExport::TColumnVector::const_iterator aDestIter    = pDestColumns->begin();
            for(;aDestIter != pDestColumns->end();++aDestIter)
                if((*aDestIter)->second == pDestField)
                    break;

            m_pParent->m_vColumnPos[nPos]   = pSrcColumns->end() - aDestIter;
            m_pParent->m_vColumnTypes[nPos] = pDestField->GetType();
        }
        else
            m_pParent->m_vColumnPos[nPos] = CONTAINER_ENTRY_NOTFOUND;

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
    SvLBoxEntry* pEntry = NULL;
    if(pEntry = m_CTRL_LEFT.FirstSelected())
    {
        sal_uInt32 nPos     = m_CTRL_LEFT.GetModel()->GetAbsPos(pEntry);
        if(pButton == &m_pbColumn_up && nPos)
            --nPos;
        else if(pButton == &m_pbColumn_down)
            nPos += 2;

        m_CTRL_LEFT.ModelIsMoving(pEntry,NULL,nPos);
        m_CTRL_LEFT.GetModel()->Move(pEntry,NULL,nPos);
        m_CTRL_LEFT.ModelHasMoved(pEntry);

        long nThumbPos      = m_CTRL_LEFT.GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_CTRL_LEFT.GetVScroll()->GetVisibleSize();

        if(pButton == &m_pbColumn_down && (nThumbPos+nVisibleSize+1) < nPos)
        {
            m_CTRL_LEFT.GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
            //  m_CTRL_LEFT.MakeVisible(pEntry,sal_True);
        }

        TableListClickHdl(&m_CTRL_LEFT);
    }


    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, RightButtonClickHdl, Button *, pButton )
{
    SvLBoxEntry* pEntry = NULL;
    if(pEntry = m_CTRL_RIGHT.FirstSelected())
    {
        sal_uInt32 nPos     = m_CTRL_RIGHT.GetModel()->GetAbsPos(pEntry);
        if(pButton == &m_pbColumn_up_right && nPos)
            --nPos;
        else if(pButton == &m_pbColumn_down_right)
            nPos += 2;

        m_CTRL_RIGHT.ModelIsMoving(pEntry,NULL,nPos);
        m_CTRL_RIGHT.GetModel()->Move(pEntry,NULL,nPos);
        m_CTRL_RIGHT.ModelHasMoved(pEntry);
        long nThumbPos      = m_CTRL_RIGHT.GetVScroll()->GetThumbPos();
        long nVisibleSize   = m_CTRL_RIGHT.GetVScroll()->GetVisibleSize();

        if(pButton == &m_pbColumn_down_right && (nThumbPos+nVisibleSize+1) < nPos)
            m_CTRL_RIGHT.GetVScroll()->DoScrollAction(SCROLL_LINEDOWN);
        TableListRightSelectHdl(&m_CTRL_RIGHT);
    }
    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizNameMatching, TableListClickHdl, SvTreeListBox *, pListBox )
{
    SvLBoxEntry* pEntry = m_CTRL_LEFT.FirstSelected();
    if(pEntry)
    {
        sal_uInt32 nPos         = m_CTRL_LEFT.GetModel()->GetAbsPos(pEntry);
        SvLBoxEntry* pOldEntry = m_CTRL_RIGHT.FirstSelected();
        if(pOldEntry && nPos != m_CTRL_RIGHT.GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_CTRL_RIGHT.Select(pOldEntry,sal_False);
            sal_uInt32 nOldPos = m_CTRL_RIGHT.GetModel()->GetAbsPos(pOldEntry);
            pOldEntry = m_CTRL_RIGHT.GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uInt32 nNewPos = m_CTRL_LEFT.GetModel()->GetAbsPos(m_CTRL_LEFT.GetFirstEntryInView());
                if(nPos - nNewPos == -1)
                    nNewPos--;
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
IMPL_LINK( OWizNameMatching, TableListRightSelectHdl, SvTreeListBox *, pListBox )
{
    SvLBoxEntry* pEntry = m_CTRL_RIGHT.FirstSelected();
    if(pEntry)
    {
        sal_uInt32 nPos         = m_CTRL_RIGHT.GetModel()->GetAbsPos(pEntry);
        SvLBoxEntry* pOldEntry = m_CTRL_LEFT.FirstSelected();
        if(pOldEntry && nPos != m_CTRL_LEFT.GetModel()->GetAbsPos(pOldEntry))
        {
            if(pOldEntry)
                m_CTRL_LEFT.Select(pOldEntry,sal_False);
            sal_uInt32 nOldPos = m_CTRL_RIGHT.GetModel()->GetAbsPos(pOldEntry);
            pOldEntry = m_CTRL_LEFT.GetEntry(nPos);
            if(pOldEntry)
            {
                sal_uInt32 nNewPos = m_CTRL_RIGHT.GetModel()->GetAbsPos(m_CTRL_RIGHT.GetFirstEntryInView());
                if(nPos - nNewPos == -1)
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

    virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
    //virtual void InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData);
};



//------------------------------------------------------------------------
/*
void OColumnString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData)
{
    SvLBoxString::InitViewData(pView,pEntry,pViewData);
}
*/
//------------------------------------------------------------------------
void OColumnString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
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
OColumnTreeBox::OColumnTreeBox( Window* pParent, WinBits nWinStyle )
    : OMarkableTreeListBox(pParent,nWinStyle)
{
    SetDragDropMode( 0 );
    EnableInplaceEditing( sal_False );
    SetWindowBits(WB_BORDER | WB_HASBUTTONS | WB_HSCROLL);
    SetSelectionMode( SINGLE_SELECTION );
}
//------------------------------------------------------------------------
OColumnTreeBox::OColumnTreeBox( Window* pParent, const ResId& rResId )
    : OMarkableTreeListBox(pParent,rResId)
{
    SetDragDropMode( 0 );
    EnableInplaceEditing( sal_False );
    SetWindowBits(WB_BORDER | WB_HASBUTTONS | WB_HSCROLL);
    SetSelectionMode( SINGLE_SELECTION );
}
//------------------------------------------------------------------------
void OColumnTreeBox::InitEntry(SvLBoxEntry* pEntry, const String& rStr, const Image& rImg1, const Image& rImg2)
{
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2 );
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
            bSelect = SvTreeListBox::Select( pEntry,bSelect );
    }
    else
        bSelect = SvTreeListBox::Select( pEntry,bSelect );
    return bSelect;
}
//------------------------------------------------------------------------
void OColumnTreeBox::FillListBox( const ODatabaseExport::TColumnVector& _rList)
{
    Clear();
    ODatabaseExport::TColumnVector::const_iterator aIter = _rList.begin();
    for(;aIter != _rList.end();++aIter)
    {
        SvLBoxEntry* pEntry = InsertEntry((*aIter)->first,0,sal_False,LIST_APPEND,(*aIter)->second);
        SvButtonState eState = !(m_bReadOnly && (*aIter)->second->IsAutoIncrement()) ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SetCheckButtonState( pEntry, SV_BUTTON_CHECKED);
    }
}
// -----------------------------------------------------------------------------





