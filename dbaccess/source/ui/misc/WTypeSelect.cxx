/*************************************************************************
 *
 *  $RCSfile: WTypeSelect.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:08:27 $
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
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SVPARSER_HXX
#include <svtools/svparser.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef DBAUI_FIELDCONTROLS_HXX
#include "FieldControls.hxx"
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
OWizTypeSelectControl::OWizTypeSelectControl(Window* pParent, OTableDesignHelpBar* pHelpBar)
            : OFieldDescControl(pParent,pHelpBar)
{
}
// -----------------------------------------------------------------------------
OWizTypeSelectControl::OWizTypeSelectControl(Window* pParent, const ResId& rResId,OTableDesignHelpBar* pHelpBar)
    : OFieldDescControl(pParent,rResId,pHelpBar)
{
}
// -----------------------------------------------------------------------------
OWizTypeSelectControl::~OWizTypeSelectControl()
{
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
    OSL_ENSURE(nPos != LISTBOX_ENTRY_NOTFOUND,"Columnname could not be found in the listbox");
    pActFieldDescr = static_cast<OFieldDescription*>(aListBox.GetEntryData(nPos));
    if ( !pActFieldDescr )
        return;

    ::rtl::OUString sName = pActFieldDescr->GetName();
    ::rtl::OUString sNewName;
    const OPropColumnEditCtrl* pColumnName = getColumnCtrl();
    if ( pColumnName )
        sNewName = pColumnName->GetText();

    switch(nColId)
    {
        case FIELD_PRPOERTY_COLUMNNAME:
            {
                OCopyTableWizard* pWiz = static_cast<OCopyTableWizard*>(GetParent()->GetParent());
                // first we have to check if this name already exists
                sal_Bool bDoubleName = sal_False;
                sal_Bool bCase = sal_True;
                if ( getMetaData().is() && !getMetaData()->storesMixedCaseQuotedIdentifiers() )
                {
                    bCase = sal_False;
                    USHORT nCount = aListBox.GetEntryCount();
                    for (USHORT i=0 ; !bDoubleName && i < nCount ; ++i)
                    {
                        ::rtl::OUString sEntry(aListBox.GetEntry(i));
                        bDoubleName = sNewName.equalsIgnoreAsciiCase(sEntry);
                    }
                    if ( !bDoubleName && pWiz->isAutoincrementEnabled() )
                        bDoubleName = sNewName.equalsIgnoreAsciiCase(pWiz->getPrimaryKeyName());

                }
                else
                    bDoubleName =  ((aListBox.GetEntryPos(String(sNewName)) != LISTBOX_ENTRY_NOTFOUND)
                                    || ( pWiz->isAutoincrementEnabled()
                                        &&  pWiz->getPrimaryKeyName() == sNewName) );

                if ( bDoubleName )
                {
                    String strMessage = String(ModuleRes(STR_TABLEDESIGN_DUPLICATE_NAME));
                    strMessage.SearchAndReplaceAscii("$column$", sNewName);
                    String sTitle(ModuleRes(STR_STAT_WARNING));
                    OSQLMessageBox aMsg(this,sTitle,strMessage,WB_OK | WB_DEF_OK,OSQLMessageBox::Error);
                    aMsg.Execute();
                    pActFieldDescr->SetName(sName);
                    DisplayData(pActFieldDescr);
                    static_cast<OWizTypeSelect*>(GetParent())->setDuplicateName(sal_True);
                    return;
                }

                ::rtl::OUString sOldName = pActFieldDescr->GetName();
                pActFieldDescr->SetName(sNewName);
                static_cast<OWizTypeSelect*>(GetParent())->setDuplicateName(sal_False);

                // now we change the name
                OCopyTableWizard::TNameMapping::iterator aIter = pWiz->m_mNameMapping.begin();
                OCopyTableWizard::TNameMapping::iterator aEnd  = pWiz->m_mNameMapping.end();

                ::comphelper::UStringMixEqual aCase(bCase);
                for(;aIter != aEnd;++aIter)
                {
                    if ( aCase(aIter->second,sName) )
                    {
                        aIter->second = sNewName;
                        break;
                    }
                }

                aListBox.RemoveEntry(nPos);
                aListBox.InsertEntry(pActFieldDescr->GetName(),nPos);
                aListBox.SetEntryData(nPos,pActFieldDescr);

                pWiz->replaceColumn(nPos,pActFieldDescr,sOldName);
            }
            break;
    }
    SaveData(pActFieldDescr);
}
// -----------------------------------------------------------------------
void OWizTypeSelectControl::SetModified(sal_Bool bModified) {}
// -----------------------------------------------------------------------------
::com::sun::star::lang::Locale  OWizTypeSelectControl::GetLocale() const
{
    return static_cast<OWizTypeSelect*>(GetParent())->m_pParent->GetLocale();
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter > OWizTypeSelectControl::GetFormatter() const
{
    return static_cast<OWizTypeSelect*>(GetParent())->m_pParent->GetFormatter();
}
// -----------------------------------------------------------------------------
TOTypeInfoSP    OWizTypeSelectControl::getTypeInfo(sal_Int32 _nPos)
{
    return static_cast<OWizTypeSelect*>(GetParent())->m_pParent->getDestTypeInfo(_nPos);
}
// -----------------------------------------------------------------------------
const OTypeInfoMap* OWizTypeSelectControl::getTypeInfo() const
{
    return static_cast<OWizTypeSelect*>(GetParent())->m_pParent->getDestTypeInfo();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> OWizTypeSelectControl::getMetaData()
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->m_xConnection->getMetaData();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> OWizTypeSelectControl::getConnection()
{
    return ((OWizTypeSelect*)GetParent())->m_pParent->m_xConnection;
}
// -----------------------------------------------------------------------------
sal_Bool OWizTypeSelectControl::isAutoIncrementValueEnabled() const
{
    return ((OWizTypeSelect*)GetParent())->m_bAutoIncrementEnabled;
}
// -----------------------------------------------------------------------------
::rtl::OUString OWizTypeSelectControl::getAutoIncrementValue() const
{
    return ((OWizTypeSelect*)GetParent())->m_sAutoIncrementValue;
}
// -----------------------------------------------------------------------------

//========================================================================
DBG_NAME(OWizTypeSelect);
#define IMG_PRIMARY_KEY 1
//========================================================================
OWizTypeSelect::OWizTypeSelect( Window* pParent,SvStream*   _pStream)
               :OWizardPage( pParent, ModuleRes( TAB_WIZ_TYPE_SELECT ))
               ,m_lbColumnNames( this, ModuleRes( LB_NEW_COLUMN_NAMES ) )
               ,m_flColumns( this, ModuleRes( FL_COLUMN_NAME ) )
               ,m_aTypeControl( this, ModuleRes( CONTROL_CONTAINER ))
               ,m_flAutoType( this, ModuleRes( FL_AUTO_TYPE ) )
               ,m_ftAuto( this, ModuleRes( FT_AUTO ) )
               ,m_etAuto( this, ModuleRes( ET_AUTO ) )
               ,m_pbAuto( this, ModuleRes( PB_AUTO ) )
               ,m_pParserStream(_pStream)
               ,m_bAutoIncrementEnabled(sal_False)
               ,m_nDisplayRow(0)
               ,m_bDuplicateName(sal_False)
{
    DBG_CTOR(OWizTypeSelect,NULL);
    m_lbColumnNames.SetSelectHdl(LINK(this,OWizTypeSelect,ColumnSelectHdl));

    ModuleRes aModuleRes(isHiContrast(&m_lbColumnNames) ? IMG_JOINS_H : IMG_JOINS);
    ImageList aImageList(aModuleRes);
    m_imgPKey = aImageList.GetImage(IMG_PRIMARY_KEY);

    m_aTypeControl.Show();
    m_aTypeControl.Init();
    m_etAuto.SetText(String::CreateFromAscii("10"));
    m_etAuto.SetDecimalDigits(0);
    m_pbAuto.SetClickHdl(LINK(this,OWizTypeSelect,ButtonClickHdl));

    m_lbColumnNames.EnableMultiSelection(sal_True);

    try
    {
        // Datenbank kann keine PrimKeys verarbeiten oder keine Zeilenselektion
        Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
        m_lbColumnNames.SetPKey(xMetaData->supportsCoreSQLGrammar());

        ::dbaui::fillAutoIncrementValue(m_pParent->m_xConnection,m_bAutoIncrementEnabled,m_sAutoIncrementValue);
    }
    catch(const Exception&)
    {
        OSL_ENSURE(0,"supportsCoreSQLGrammar OR fillAutoIncrementValue failed!");
    }

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
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    return String(ModuleRes(STR_WIZ_TYPE_SELECT_TITEL));
}
// -----------------------------------------------------------------------
void OWizTypeSelect::Resize()
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
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
    sal_Bool bOldFirstTime = m_bFirstTime;
    Reset();
    m_bFirstTime = bOldFirstTime;

    m_lbColumnNames.SelectEntryPos(static_cast<USHORT>(m_nDisplayRow));
    m_nDisplayRow = 0;
    m_lbColumnNames.GetSelectHdl().Call(&m_lbColumnNames);
}
// -----------------------------------------------------------------------
sal_Bool OWizTypeSelect::LeavePage()
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    String aColumnName( m_lbColumnNames.GetSelectEntry() );

    sal_Bool bDuplicateName = sal_False;
    OFieldDescription* pField = static_cast<OFieldDescription*>(m_lbColumnNames.GetEntryData(m_lbColumnNames.GetEntryPos(aColumnName)));
    if ( pField )
    {
        m_aTypeControl.SaveData(pField);
        bDuplicateName = m_bDuplicateName;
    }
    return !bDuplicateName;
}
//------------------------------------------------------------------------------
void OWizTypeSelect::EnableAuto(sal_Bool bEnable)
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    m_ftAuto.Show(bEnable);
    m_etAuto.Show(bEnable);
    m_pbAuto.Show(bEnable);
    m_flAutoType.Show(bEnable);
}
//------------------------------------------------------------------------------
IMPL_LINK( OWizTypeSelect, ButtonClickHdl, Button *, pButton )
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    sal_Int32 nBreakPos;
    m_pParent->CheckColumns(nBreakPos);
    fillColumnList(m_etAuto.GetText().ToInt32());

    ActivatePage();

    return 0;
}
//------------------------------------------------------------------------
sal_Bool OWizTypeSelectList::IsPrimaryKeyAllowed() const
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    sal_uInt16 nCount = GetSelectEntryCount();
    sal_uInt16 j;

    for( j = 0; m_bPKey && j < nCount; ++j )
    {
        OFieldDescription* pField = static_cast<OFieldDescription*>(GetEntryData(GetSelectEntryPos(j)));
        if(!pField || pField->getTypeInfo()->nSearchType == ColumnSearch::NONE)
            break;
    }
    return j == nCount;
}
// -----------------------------------------------------------------------------
void OWizTypeSelectList::setPrimaryKey(OFieldDescription* _pFieldDescr,sal_uInt16 _nPos,sal_Bool _bSet)
{
    String sColumnName = GetEntry(_nPos);
    RemoveEntry(_nPos);
    _pFieldDescr->SetPrimaryKey(_bSet);
    if( _bSet )
        InsertEntry(sColumnName,((OWizTypeSelect*)GetParent())->m_imgPKey,_nPos);
    else if( _pFieldDescr->getTypeInfo()->bNullable )
    {
        _pFieldDescr->SetControlDefault(Any());
        InsertEntry(sColumnName,_nPos);
    }
    SetEntryData(_nPos,_pFieldDescr);
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
                    String sColumnName;
                    sal_uInt16 nCount = GetEntryCount();
                    for(sal_uInt16 j = 0 ; j < nCount ; ++j)
                    {
                        OFieldDescription* pFieldDescr = static_cast<OFieldDescription*>(GetEntryData(j));
                        if( pFieldDescr )
                        {
                            if(pFieldDescr->IsPrimaryKey() && !IsEntryPosSelected(j))
                                setPrimaryKey(pFieldDescr,j);
                            else if(IsEntryPosSelected(j))
                            {
                                setPrimaryKey(pFieldDescr,j,!pFieldDescr->IsPrimaryKey());
                                SelectEntryPos(j);
                            }
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
void OWizTypeSelect::fillColumnList(sal_uInt32 nRows)
{
    DBG_CHKTHIS(OWizTypeSelect,NULL);
    if(m_pParserStream)
    {
        sal_uInt32 nTell = m_pParserStream->Tell(); // verändert vielleicht die Position des Streams

        SvParser *pReader = createReader(nRows);
        if(pReader)
        {
            pReader->AddRef();
            pReader->CallParser();
            pReader->ReleaseRef();
        }
        m_pParserStream->Seek(nTell);
    }
}
// -----------------------------------------------------------------------------


