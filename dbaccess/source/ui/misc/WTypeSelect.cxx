/*************************************************************************
 *
 *  $RCSfile: WTypeSelect.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:07:19 $
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
#ifndef DBAUI_WIZ_TYPESELECT_HXX
#include "WTypeSelect.hxx"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
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

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
//  using namespace ::com::sun::star::sdbcx;

//========================================================================
// OWizTypeSelectControl
//========================================================================
OWizTypeSelectControl::~OWizTypeSelectControl()
{
    delete pActFieldDescr;
}
// -----------------------------------------------------------------------
void OWizTypeSelectControl::ActivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpAutoIncrement:
            break;
        default:
            OFieldDescControl::ActivateAggregate( eType );
    }
}
// -----------------------------------------------------------------------
void OWizTypeSelectControl::DeactivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpAutoIncrement:
            break;
        default:
            OFieldDescControl::DeactivateAggregate( eType );
    }
}
// -----------------------------------------------------------------------
void OWizTypeSelectControl::CellModified(long nRow, sal_uInt16 nColId )
{
    DBG_ASSERT(nRow == -1,"nRow muss -1 sein!");

    MultiListBox &aListBox = ((OWizTypeSelect*)GetParent())->m_lbColumnNames;

    sal_uInt16 nPos = aListBox.GetEntryPos(String(pActFieldDescr->GetName()));
    pActFieldDescr = static_cast<OFieldDescription*>(aListBox.GetEntryData(nPos));

    ::rtl::OUString sName = pActFieldDescr->GetName();
    SaveData(pActFieldDescr);
    switch(nColId)
    {
        case FIELD_PRPOERTY_COLUMNNAME:
            {
                OCopyTableWizard::TNameMapping::iterator aIter = ((OWizTypeSelect*)GetParent())->m_pParent->m_mNameMapping.begin();
                for(;aIter != ((OWizTypeSelect*)GetParent())->m_pParent->m_mNameMapping.end();++aIter)
                {
                    if(aIter->second == sName)
                    {
                        aIter->second = pActFieldDescr->GetName();
                        break;
                    }
                }

                aListBox.RemoveEntry(nPos);
                aListBox.InsertEntry(pActFieldDescr->GetName(),nPos);
                aListBox.SetEntryData(nPos,pActFieldDescr);
            }
            break;
    }
}
// -----------------------------------------------------------------------
void OWizTypeSelectControl::SetModified(sal_Bool bModified) {}
// -----------------------------------------------------------------------------
::com::sun::star::lang::Locale  OWizTypeSelectControl::GetLocale() const
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->GetLocale();
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter > OWizTypeSelectControl::GetFormatter()
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->GetFormatter();
}
// -----------------------------------------------------------------------------
const OTypeInfo*    OWizTypeSelectControl::getTypeInfo(sal_Int32 _nPos)
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->getTypeInfo(_nPos);
}
// -----------------------------------------------------------------------------
const OTypeInfoMap* OWizTypeSelectControl::getTypeInfo() const
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->getTypeInfo();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> OWizTypeSelectControl::getMetaData()
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->m_xConnection->getMetaData();
}
//========================================================================
DBG_NAME(OWizTypeSelect);
#define IMG_PRIMARY_KEY 1
//========================================================================
OWizTypeSelect::OWizTypeSelect( Window* pParent)
               :OWizardPage( pParent, ModuleRes( TAB_WIZ_TYPE_SELECT ))
               ,m_lbColumnNames( this, ModuleRes( LB_NEW_COLUMN_NAMES ) )
               ,m_gpColumns( this, ModuleRes( GB_COLUMN_NAME ) )
               ,m_aTypeControl( this, ModuleRes( CONTROL_CONTAINER ))
               ,m_gpAutoType( this, ModuleRes( GB_AUTO_TYPE ) )
               ,m_ftAuto( this, ModuleRes( FT_AUTO ) )
               ,m_etAuto( this, ModuleRes( ET_AUTO ) )
               ,m_pbAuto( this, ModuleRes( PB_AUTO ) )
{
    DBG_CTOR(OWizTypeSelect,NULL);
    m_lbColumnNames.SetSelectHdl(LINK(this,OWizTypeSelect,ColumnSelectHdl));

    ImageList aImageList(ModuleRes(IMG_JOINS));
    m_imgPKey = aImageList.GetImage(IMG_PRIMARY_KEY);

    m_aTypeControl.Show();
    //  m_etAuto.SetInternational(Application::GetAppInternational());
    m_etAuto.SetText(String::CreateFromAscii("10"));
    m_etAuto.SetDecimalDigits(0);
    m_pbAuto.SetClickHdl(LINK(this,OWizTypeSelect,ButtonClickHdl));

    m_lbColumnNames.EnableMultiSelection(sal_True);

    // Datenbank kann keine PrimKeys verarbeiten oder keine Zeilenselektion
    Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
    m_lbColumnNames.SetPKey(xMetaData->supportsCoreSQLGrammar());


    FreeResource();
}
// -----------------------------------------------------------------------
OWizTypeSelect::~OWizTypeSelect()
{
    DBG_DTOR(OWizTypeSelect,NULL);
}
// -----------------------------------------------------------------------------
String OWizTypeSelect::GetTitle() const
{
    return String(ModuleRes(STR_WIZ_TYPE_SELECT_TITEL));
}
// -----------------------------------------------------------------------
void OWizTypeSelect::Resize()
{
}
// -----------------------------------------------------------------------
IMPL_LINK( OWizTypeSelect, ColumnSelectHdl, MultiListBox *, pListBox )
{
    String aColumnName( m_lbColumnNames.GetSelectEntry() );

    OFieldDescription* pField = static_cast<OFieldDescription*>(m_lbColumnNames.GetEntryData(m_lbColumnNames.GetEntryPos(aColumnName)));
    if(pField)
        m_aTypeControl.DisplayData(pField);
    return 0;
}

// -----------------------------------------------------------------------
void OWizTypeSelect::Reset()
{
    // urspr"unglichen zustand wiederherstellen
    DBG_CHKTHIS(OWizTypeSelect,NULL);

    while(m_lbColumnNames.GetEntryCount())
        m_lbColumnNames.RemoveEntry(0);
    m_lbColumnNames.Clear();

    const ODatabaseExport::TColumnVector* pDestColumns = m_pParent->getDestVector();
    ODatabaseExport::TColumnVector::const_iterator aIter = pDestColumns->begin();
    for(;aIter != pDestColumns->end();++aIter)
    {
        sal_uInt16 nPos;
        if((*aIter)->second->IsPrimaryKey())
            nPos = m_lbColumnNames.InsertEntry((*aIter)->first, m_imgPKey );
        else
            nPos = m_lbColumnNames.InsertEntry((*aIter)->first);
        m_lbColumnNames.SetEntryData(nPos,(*aIter)->second);
    }
    m_bFirstTime = sal_False;
}
// -----------------------------------------------------------------------
void OWizTypeSelect::ActivatePage( )
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);

    while(m_lbColumnNames.GetEntryCount())
        m_lbColumnNames.RemoveEntry(0);
    m_lbColumnNames.Clear();

    const ODatabaseExport::TColumnVector* pDestColumns = m_pParent->getDestVector();
    ODatabaseExport::TColumnVector::const_iterator aIter = pDestColumns->begin();
    for(;aIter != pDestColumns->end();++aIter)
    {
        sal_uInt16 nPos;
        if((*aIter)->second->IsPrimaryKey())
            nPos = m_lbColumnNames.InsertEntry((*aIter)->first, m_imgPKey );
        else
            nPos = m_lbColumnNames.InsertEntry((*aIter)->first);

        m_lbColumnNames.SetEntryData(nPos,(*aIter)->second);
    }

    m_lbColumnNames.SelectEntryPos(0);
    m_lbColumnNames.GetSelectHdl().Call(&m_lbColumnNames);
}
// -----------------------------------------------------------------------
sal_Bool OWizTypeSelect::LeavePage()
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    return sal_True;
}
//------------------------------------------------------------------------------
void OWizTypeSelect::EnableAuto(sal_Bool bEnable)
{
    if(bEnable)
    {
        m_ftAuto.Show();
        m_etAuto.Show();
        m_pbAuto.Show();
        m_gpAutoType.Show();
    }
    else
    {
        m_ftAuto.Hide();
        m_etAuto.Hide();
        m_pbAuto.Hide();
        m_gpAutoType.Hide();
    }
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizTypeSelect, ButtonClickHdl, Button *, pButton )
{
    m_pParent->CheckColumns();
    fillColumnList(m_etAuto.GetText().ToInt32());

//  OColumnList &aList = FillColumnList(m_pParent->m_xTableDef->GetColumns(),m_etAuto.GetText().ToInt32());
//
//  for(sal_uInt32 i=0;i<aList.Count();++i)
//  {
//      sal_uInt32 nPos = m_pColumnList.GetPos(aList.GetObject(i));
//      OColumn *pSrcColumn = aList.GetObject(i);
//      SFX_ITEMSET_GET( *pSrcColumn, pName, ONameItem, SBA_DEF_FLTNAME, sal_True );
//      OColumn *pDestColumn = m_pParent->m_lColumnList.Lookup(m_pParent->m_mNameMapping[pName->GetValue()],String());
//      if(pDestColumn)
//      {
//          OColumn*    pNewColumn = pDestColumn;
//          OColumn*    pColumn = aList.GetObject(i);
//
//          SFX_ITEMSET_GET(*pColumn, pType, ODataFieldTypeItem, SBA_DEF_FLTTYPE, sal_True);
//          SFX_ITEMSET_GET(*pColumn, pLength, SfxUInt32Item, SBA_DEF_FLTLENGTH, sal_True);
//          SFX_ITEMSET_GET(*pColumn, pScale, SfxUInt16Item, SBA_DEF_FLTSCALE, sal_True);
//          SFX_ITEMSET_GET(*pColumn, pFormat, SfxUInt32Item, SBA_DEF_FMTVALUE, sal_True );
//
//          SdbTypeInfo* pInfo = m_pParent->m_xTableDef->GetDatabase()->IsTypeAvailable(pType->GetType(), pLength->GetValue());
//
//          pNewColumn->Put(ODataFieldTypeItem(SBA_DEF_FLTTYPE,(OFieldType) pType->GetValue()));
//          pNewColumn->Put(SfxUInt32Item(SBA_DEF_FLTLENGTH,pLength->GetValue()));
//          pNewColumn->Put(SfxUInt16Item(SBA_DEF_FLTSCALE,pScale->GetValue()));
//          pNewColumn->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, pFormat->GetValue()));
//
//          if(!pInfo)
//              m_pParent->m_xTableDef->AdjustType(pColumn,pNewColumn);
//
//          SFX_ITEMSET_GET(*pNewColumn, pType2, ODataFieldTypeItem, SBA_DEF_FLTTYPE, sal_True);
//
//          switch(pType2->GetType())
//          {
//              case dbChar:
//              case dbText:
//                  if(pInfo && pInfo->nPrecision && pInfo->nPrecision < pLength->GetValue()) // this type is to small for my data
//                      m_pParent->m_xTableDef->ConvertTypeTo(dbMemo, pNewColumn);
//                  break;
//          }
//      }
//  }
    ActivatePage();

    return 0;
}
//------------------------------------------------------------------------
sal_Bool OWizTypeSelectList::IsPrimaryKeyAllowed() const
{
    for(sal_uInt16 j=0;m_bPKey && j<GetSelectEntryCount();++j)
    {
        OFieldDescription* pField = static_cast<OFieldDescription*>(GetEntryData(GetSelectEntryPos(j)));
        if(!pField || pField->IsPrimaryKey())
            break;
    }
    return j == GetSelectEntryCount();
}
//------------------------------------------------------------------------
long OWizTypeSelectList::PreNotify( NotifyEvent& rEvt )
{
    long nDone = 0;
    switch( rEvt.GetType() )
    {
        case EVENT_MOUSEBUTTONDOWN:
        {
            const MouseEvent* pMEvt = rEvt.GetMouseEvent();
            if(pMEvt->IsRight() && !pMEvt->GetModifier())
                nDone = 1;
        }
            break;
        case EVENT_COMMAND:
        {
            if(!IsPrimaryKeyAllowed())
                break;

            const CommandEvent* pComEvt = rEvt.GetCommandEvent();
            if(pComEvt->GetCommand() != COMMAND_CONTEXTMENU)
                break;
            // die Stelle, an der geklickt wurde
            Point ptWhere(0,0);
            if (pComEvt->IsMouseEvent())
                ptWhere = pComEvt->GetMousePosPixel();

            PopupMenu aContextMenu(ModuleRes(RID_SBA_RTF_PKEYPOPUP));
            switch( aContextMenu.Execute( this, ptWhere ) )
            {
                case SID_TABLEDESIGN_TABED_PRIMARYKEY:
                {
                    OFieldDescription* pField = static_cast<OFieldDescription*>(GetEntryData(GetSelectEntryPos()));
                    if(!pField)
                        break;

                    String aColumnName;
                    for(sal_uInt16 j=0;j<GetEntryCount();++j)
                    {
                        if(!IsEntryPosSelected(j))
                        {
                            OFieldDescription* pOld = static_cast<OFieldDescription*>(GetEntryData(j));
                            pOld->SetPrimaryKey(sal_False);
                            aColumnName = GetEntry(j);
                            RemoveEntry(j);
                            InsertEntry(aColumnName,j);
                            SetEntryData(j,pOld);
                        }
                        else
                        {
                            aColumnName = GetEntry(j);
                            OFieldDescription* pNewHold = static_cast<OFieldDescription*>(GetEntryData(j));
                            RemoveEntry(j);
                            pNewHold->SetPrimaryKey(!m_bPKey);
                            if(m_bPKey)
                                InsertEntry(aColumnName,j);
                            else
                            {
                                pNewHold->SetIsNullable(ColumnValue::NO_NULLS);
                                InsertEntry(aColumnName,((OWizTypeSelect*)GetParent())->m_imgPKey,j);
                            }
                            SetEntryData(j,pNewHold);
                            SelectEntryPos(j);
                        }
                    }
                    GetSelectHdl().Call(this);
                }
                break;
            }
            nDone = 1;
        }
        break;
    }
    return nDone ? nDone : MultiListBox::PreNotify(rEvt);
}
// -----------------------------------------------------------------------------

