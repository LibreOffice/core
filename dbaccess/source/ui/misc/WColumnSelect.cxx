/*************************************************************************
 *
 *  $RCSfile: WColumnSelect.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:07:10 $
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

#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#include "WColumnSelect.hxx"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
//  using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace dbaui;

// -----------------------------------------------------------------------
String OWizColumnSelect::GetTitle() const { return String(ModuleRes(STR_WIZ_COLUMN_SELECT_TITEL)); }
// -----------------------------------------------------------------------
OWizardPage::OWizardPage( Window* pParent, WinBits nStyle)
    : TabPage(pParent,nStyle)
    ,m_pParent(static_cast<OCopyTableWizard*>(pParent))
    ,m_bFirstTime(sal_True)
{
}
// -----------------------------------------------------------------------------
OWizardPage::OWizardPage( Window* pParent, const ResId& rResId )
    : TabPage(pParent,rResId)
    ,m_pParent(static_cast<OCopyTableWizard*>(pParent))
    ,m_bFirstTime(sal_True)
{
}
//========================================================================
// OWizColumnSelect
DBG_NAME(OWizColumnSelect);
//========================================================================
OWizColumnSelect::OWizColumnSelect( Window* pParent)
               :OWizardPage( pParent, ModuleRes( TAB_WIZ_COLUMN_SELECT )),
               m_flColumns( this, ModuleRes( FL_COLUMN_SELECT ) ),
               m_lbOrgColumnNames( this, ModuleRes( LB_ORG_COLUMN_NAMES ) ),
               m_ibColumn_RH( this, ModuleRes( IB_COLUMN_RH ) ),
               m_ibColumn_LH( this, ModuleRes( IB_COLUMN_LH ) ),
               m_ibColumns_RH( this, ModuleRes( IB_COLUMNS_RH ) ),
               m_ibColumns_LH( this, ModuleRes( IB_COLUMNS_LH ) ),
               m_lbNewColumnNames( this, ModuleRes( LB_NEW_COLUMN_NAMES ) )
{
    DBG_CTOR(OWizColumnSelect,NULL);
    m_ibColumn_RH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_ibColumn_LH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_ibColumns_RH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_ibColumns_LH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));

    m_lbOrgColumnNames.EnableMultiSelection(sal_True);
    m_lbNewColumnNames.EnableMultiSelection(sal_True);

    m_lbOrgColumnNames.SetDoubleClickHdl(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
    m_lbNewColumnNames.SetDoubleClickHdl(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
    FreeResource();
}
// -----------------------------------------------------------------------
OWizColumnSelect::~OWizColumnSelect()
{
    DBG_DTOR(OWizColumnSelect,NULL);
    while ( m_lbNewColumnNames.GetEntryCount() )
    {
        void* pData = m_lbNewColumnNames.GetEntryData(0);
        if ( pData )
            delete static_cast<OFieldDescription*>(pData);

        m_lbNewColumnNames.RemoveEntry(0);
    }
    m_lbNewColumnNames.Clear();
}

// -----------------------------------------------------------------------
void OWizColumnSelect::Reset()
{
    // urspr"unglichen zustand wiederherstellen
    DBG_CHKTHIS(OWizColumnSelect,NULL);

    clearListBox(m_lbOrgColumnNames);
    clearListBox(m_lbNewColumnNames);

    // insert the source columns in the left listbox
    const ODatabaseExport::TColumnVector* pSrcColumns = m_pParent->getSrcVector();
    ODatabaseExport::TColumnVector::const_iterator aIter = pSrcColumns->begin();

    for(;aIter != pSrcColumns->end();++aIter)
    {
        sal_uInt16 nPos = m_lbOrgColumnNames.InsertEntry((*aIter)->first);
        m_lbOrgColumnNames.SetEntryData(nPos,(*aIter)->second);
    }

    // m_pParent->clearDestColumns();

    if(m_lbOrgColumnNames.GetEntryCount())
        m_lbOrgColumnNames.SelectEntryPos(0);

    m_bFirstTime = sal_False;
}
// -----------------------------------------------------------------------
void OWizColumnSelect::ActivatePage( )
{
    DBG_CHKTHIS(OWizColumnSelect,NULL);
    // if there are no dest columns reset the left side with the origibnal columns
    if(m_pParent->getDestColumns()->size() == 0)
        Reset();

    clearListBox(m_lbNewColumnNames);

    const ODatabaseExport::TColumnVector* pDestColumns = m_pParent->getDestVector();

    ODatabaseExport::TColumnVector::const_iterator aIter = pDestColumns->begin();
    for(;aIter != pDestColumns->end();++aIter)
    {
        USHORT nPos = m_lbNewColumnNames.InsertEntry((*aIter)->first);
        m_lbNewColumnNames.SetEntryData(nPos,new OFieldDescription(*((*aIter)->second)));
        m_lbOrgColumnNames.RemoveEntry((*aIter)->first);
    }
    m_pParent->GetOKButton().Enable(m_lbNewColumnNames.GetEntryCount() != 0);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,m_lbNewColumnNames.GetEntryCount() && m_pParent->getCreateStyle() != OCopyTableWizard::WIZARD_APPEND_DATA);
    m_ibColumns_RH.GrabFocus();
}
// -----------------------------------------------------------------------
sal_Bool OWizColumnSelect::LeavePage()
{
    DBG_CHKTHIS(OWizColumnSelect,NULL);

    //  m_pParent->getColumns()->clear();
    m_pParent->clearDestColumns();

    for(sal_uInt16 i=0 ; i< m_lbNewColumnNames.GetEntryCount();++i)
    {
        OFieldDescription* pField = static_cast<OFieldDescription*>(m_lbNewColumnNames.GetEntryData(i));
        OSL_ENSURE(pField,"The field information can not be null!");
        m_pParent->insertColumn(i,pField);
    }

    clearListBox(m_lbNewColumnNames);


    if(m_pParent->WasButtonPressed() == OCopyTableWizard::WIZARD_NEXT || m_pParent->WasButtonPressed() == OCopyTableWizard::WIZARD_FINISH)
        return m_pParent->getDestColumns()->size();
    else
        return sal_True;
}
// -----------------------------------------------------------------------
IMPL_LINK( OWizColumnSelect, ButtonClickHdl, Button *, pButton )
{
    MultiListBox *pLeft = NULL;
    MultiListBox *pRight = NULL;
    sal_Bool bAll = sal_False;

    if(pButton == &m_ibColumn_RH)
    {
        pLeft  = &m_lbOrgColumnNames;
        pRight = &m_lbNewColumnNames;
    }
    else if(pButton == &m_ibColumn_LH)
    {
        pLeft  = &m_lbNewColumnNames;
        pRight = &m_lbOrgColumnNames;
    }
    else if(pButton == &m_ibColumns_RH)
    {
        pLeft  = &m_lbOrgColumnNames;
        pRight = &m_lbNewColumnNames;
        bAll   = sal_True;
    }
    else if(pButton == &m_ibColumns_LH)
    {
        pLeft  = &m_lbNewColumnNames;
        pRight = &m_lbOrgColumnNames;
        bAll   = sal_True;
    }
    // else ????

    Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
    ::rtl::OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::TStringMixEqualFunctor aCase(xMetaData->storesMixedCaseQuotedIdentifiers());
    ::std::vector< ::rtl::OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    String aColumnName;
    if(!bAll)
    {
        for(sal_uInt16 i=0; i < pLeft->GetSelectEntryCount(); ++i)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->GetSelectEntry(i),sExtraChars,nMaxNameLen,aCase);

        for(sal_uInt16 j=pLeft->GetSelectEntryCount(); j ; --j)
            pLeft->RemoveEntry(pLeft->GetSelectEntry(j-1));
    }
    else
    {
        sal_uInt16 nEntries = pLeft->GetEntryCount();
        for(sal_uInt16 i=0; i < nEntries; ++i)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->GetEntry(i),sExtraChars,nMaxNameLen,aCase);
        for(sal_uInt16 j=pLeft->GetEntryCount(); j ; --j)
            pLeft->RemoveEntry(j-1);
    }

    enableButtons();

    if(m_lbOrgColumnNames.GetEntryCount())
        m_lbOrgColumnNames.SelectEntryPos(0);

    return 0;
}
// -----------------------------------------------------------------------
IMPL_LINK( OWizColumnSelect, ListDoubleClickHdl, MultiListBox *, pListBox )
{
    MultiListBox *pLeft,*pRight;
    if(pListBox == &m_lbOrgColumnNames)
    {
        pLeft  = &m_lbOrgColumnNames;
        pRight = &m_lbNewColumnNames;
    }
    else
    {
        pRight = &m_lbOrgColumnNames;
        pLeft  = &m_lbNewColumnNames;
    }

    //////////////////////////////////////////////////////////////////////
    // Wenn Datenbank PrimaryKeys verarbeiten kann, PrimaryKey anlegen
    Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
    ::rtl::OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::TStringMixEqualFunctor aCase(xMetaData->storesMixedCaseQuotedIdentifiers());
    ::std::vector< ::rtl::OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    String aColumnName;
    for(sal_uInt16 i=0; i < pLeft->GetSelectEntryCount(); ++i)
        moveColumn(pRight,pLeft,aRightColumns,pLeft->GetSelectEntry(i),sExtraChars,nMaxNameLen,aCase);
    for(sal_uInt16 j=pLeft->GetSelectEntryCount(); j ; --j)
        pLeft->RemoveEntry(pLeft->GetSelectEntry(j-1));

    enableButtons();
    return 0;
}
// -----------------------------------------------------------------------------
void OWizColumnSelect::clearListBox(MultiListBox& _rListBox)
{
    while(_rListBox.GetEntryCount())
        _rListBox.RemoveEntry(0);
    _rListBox.Clear();
}
// -----------------------------------------------------------------------------
void OWizColumnSelect::fillColumns(ListBox* pRight,::std::vector< ::rtl::OUString> &_rRightColumns)
{
    sal_uInt16 nCount = pRight->GetEntryCount();
    _rRightColumns.reserve(nCount);
    for(sal_uInt16 i=0; i < nCount;++i)
        _rRightColumns.push_back(pRight->GetEntry(i));
}
// -----------------------------------------------------------------------------
void OWizColumnSelect::createNewColumn( ListBox* _pListbox,
                                        OFieldDescription* _pSrcField,
                                        ::std::vector< ::rtl::OUString>& _rRightColumns,
                                        const ::rtl::OUString&  _sColumnName,
                                        const ::rtl::OUString&  _sExtraChars,
                                        sal_Int32               _nMaxNameLen,
                                        const ::comphelper::TStringMixEqualFunctor& _aCase,
                                        USHORT nPos)
{
    ::rtl::OUString sConvertedName = m_pParent->convertColumnName(TMultiListBoxEntryFindFunctor(&_rRightColumns,_aCase),
                                                                _sColumnName,
                                                                _sExtraChars,
                                                                _nMaxNameLen);
    OFieldDescription* pNewField = new OFieldDescription(*_pSrcField);
    pNewField->SetName(sConvertedName);
    sal_Bool bNotConvert = sal_True;
    pNewField->SetType(m_pParent->convertType(_pSrcField->getTypeInfo(),bNotConvert));
    if ( !m_pParent->supportsPrimaryKey() )
        pNewField->SetPrimaryKey(sal_False);

    _pListbox->SetEntryData(_pListbox->InsertEntry(sConvertedName),pNewField);
    _rRightColumns.push_back(sConvertedName);

    if ( !bNotConvert )
        m_pParent->showColumnTypeNotSupported(sConvertedName);
}
// -----------------------------------------------------------------------------
void OWizColumnSelect::moveColumn(  ListBox* _pRight,
                                    ListBox* _pLeft,
                                    ::std::vector< ::rtl::OUString>& _rRightColumns,
                                    const ::rtl::OUString&  _sColumnName,
                                    const ::rtl::OUString&  _sExtraChars,
                                    sal_Int32               _nMaxNameLen,
                                    const ::comphelper::TStringMixEqualFunctor& _aCase)
{
    if(_pRight == &m_lbNewColumnNames)
    {
        // we copy the column into the new format for the dest
        OFieldDescription* pSrcField = static_cast<OFieldDescription*>(_pLeft->GetEntryData(_pLeft->GetEntryPos(String(_sColumnName))));
        createNewColumn(_pRight,pSrcField,_rRightColumns,_sColumnName,_sExtraChars,_nMaxNameLen,_aCase);
    }
    else
    {
        // find the new column in the dest name mapping to obtain the old column
        OCopyTableWizard::TNameMapping::iterator aIter = ::std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
                                                                ::std::compose1(
                                                                    ::std::bind2nd(_aCase, _sColumnName),
                                                                    ::std::select2nd<OCopyTableWizard::TNameMapping::value_type>())
                                                                    );

        DBG_ASSERT(aIter != m_pParent->m_mNameMapping.end(),"Column must to be defined");
        const ODatabaseExport::TColumns* pSrcColumns = m_pParent->getSourceColumns();
        ODatabaseExport::TColumns::const_iterator aSrcIter = pSrcColumns->find((*aIter).first);
        if ( aSrcIter != pSrcColumns->end() )
        {
            // we need also the old position of this column to insert it back on that position again
            const ODatabaseExport::TColumnVector* pSrcVector = m_pParent->getSrcVector();
            ODatabaseExport::TColumnVector::const_iterator aPos = ::std::find(pSrcVector->begin(),pSrcVector->end(),aSrcIter);
            OSL_ENSURE( aPos != pSrcVector->end(),"Invalid position for the iterator here!");
            USHORT nPos = (aPos - pSrcVector->begin()) - adjustColumnPosition(_pLeft, _sColumnName, (aPos - pSrcVector->begin()), _aCase);

            _pRight->SetEntryData( _pRight->InsertEntry( (*aIter).first, nPos),aSrcIter->second );
            _rRightColumns.push_back((*aIter).first);
        }
    }
}
// -----------------------------------------------------------------------------
// Simply returning fields back to their original position is
// not enough. We need to take into acccount what fields have
// been removed earlier and adjust accordingly. Based on the
// algorithm employed in moveColumn().
USHORT OWizColumnSelect::adjustColumnPosition( ListBox* _pLeft,
                                               const ::rtl::OUString&   _sColumnName,
                                               USHORT nCurrentPos,
                                               const ::comphelper::TStringMixEqualFunctor& _aCase)
{
    USHORT nAdjustedPos = 0;

    // if returning all entries to their original position,
    // then there is no need to adjust the positions.
    if (m_ibColumns_LH.HasFocus())
        return nAdjustedPos;

    sal_uInt16 nCount = _pLeft->GetEntryCount();
    ::rtl::OUString sColumnString;
    for(sal_uInt16 i=0; i < nCount; ++i)
    {
        sColumnString = _pLeft->GetEntry(i);
        if(_sColumnName != sColumnString)
        {
            // find the new column in the dest name mapping to obtain the old column
            OCopyTableWizard::TNameMapping::iterator aIter = ::std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
                                                                    ::std::compose1(
                                                                    ::std::bind2nd(_aCase, sColumnString),
                                                                    ::std::select2nd<OCopyTableWizard::TNameMapping::value_type>())
                                                                    );

            DBG_ASSERT(aIter != m_pParent->m_mNameMapping.end(),"Column must be defined");
            const ODatabaseExport::TColumns* pSrcColumns = m_pParent->getSourceColumns();
            ODatabaseExport::TColumns::const_iterator aSrcIter = pSrcColumns->find((*aIter).first);
            if ( aSrcIter != pSrcColumns->end() )
            {
                // we need also the old position of this column to insert it back on that position again
                const ODatabaseExport::TColumnVector* pSrcVector = m_pParent->getSrcVector();
                ODatabaseExport::TColumnVector::const_iterator aPos = ::std::find(pSrcVector->begin(),pSrcVector->end(),aSrcIter);
                USHORT nPos = aPos - pSrcVector->begin();
                if( nPos < nCurrentPos)
                {
                    nAdjustedPos++;
                }
            }
        }
    }

    return nAdjustedPos;
}
// -----------------------------------------------------------------------------
void OWizColumnSelect::enableButtons()
{
    sal_Bool bEntries = m_lbNewColumnNames.GetEntryCount() != 0;
    if(!bEntries)
        m_pParent->m_mNameMapping.clear();

    m_pParent->GetOKButton().Enable(bEntries);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,bEntries && m_pParent->getCreateStyle() != OCopyTableWizard::WIZARD_APPEND_DATA);
}
// -----------------------------------------------------------------------------

