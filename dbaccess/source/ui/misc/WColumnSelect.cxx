/*************************************************************************
 *
 *  $RCSfile: WColumnSelect.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:07:41 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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
               m_gpColumns( this, ModuleRes( GB_COLUMN_SELECT ) ),
               m_lbOrgColumnNames( this, ModuleRes( LB_ORG_COLUMN_NAMES ) ),
               m_pbColumn_RH( this, ModuleRes( PB_COLUMN_RH ) ),
               m_pbColumn_LH( this, ModuleRes( PB_COLUMN_LH ) ),
               m_pbColumns_RH( this, ModuleRes( PB_COLUMNS_RH ) ),
               m_pbColumns_LH( this, ModuleRes( PB_COLUMNS_LH ) ),
               m_lbNewColumnNames( this, ModuleRes( LB_NEW_COLUMN_NAMES ) )
{
    DBG_CTOR(OWizColumnSelect,NULL);
    m_pbColumn_RH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pbColumn_LH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pbColumns_RH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pbColumns_LH.SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));

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
}

// -----------------------------------------------------------------------
void OWizColumnSelect::Reset()
{
    // urspr"unglichen zustand wiederherstellen
    DBG_CHKTHIS(OWizColumnSelect,NULL);

    while(m_lbOrgColumnNames.GetEntryCount())
        m_lbOrgColumnNames.RemoveEntry(0);
    m_lbOrgColumnNames.Clear();

    while(m_lbNewColumnNames.GetEntryCount())
        m_lbNewColumnNames.RemoveEntry(0);
    m_lbNewColumnNames.Clear();

    // insert the source columns in the left listbox
    const ODatabaseExport::TColumnVector* pSrcColumns = m_pParent->getSrcVector();
    ODatabaseExport::TColumnVector::const_iterator aIter = pSrcColumns->begin();

    for(;aIter != pSrcColumns->end();++aIter)
    {
        sal_uInt16 nPos = m_lbOrgColumnNames.InsertEntry((*aIter)->first);
        m_lbOrgColumnNames.SetEntryData(nPos,(*aIter)->second);
    }

    m_pParent->clearDestColumns();

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

    while(m_lbNewColumnNames.GetEntryCount())
        m_lbNewColumnNames.RemoveEntry(0);
    m_lbNewColumnNames.Clear();

    //////////////////////////////////////////////////////////////////////
    // Wenn Datenbank PrimaryKeys verarbeiten kann, PrimaryKey anlegen
    Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
    sal_Bool bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();

    const ODatabaseExport::TColumnVector* pDestColumns = m_pParent->getDestVector();

    ODatabaseExport::TColumnVector::const_iterator aIter = pDestColumns->begin();
    for(;aIter != pDestColumns->end();++aIter)
    {
        USHORT nPos = m_lbNewColumnNames.InsertEntry((*aIter)->first);
        m_lbNewColumnNames.SetEntryData(nPos,new OFieldDescription(*((*aIter)->second)));
        m_lbOrgColumnNames.RemoveEntry((*aIter)->first);
    }
    m_pParent->GetOKButton().Enable(m_lbNewColumnNames.GetEntryCount() != 0);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,m_lbNewColumnNames.GetEntryCount() && m_pParent->m_eCreateStyle != OCopyTableWizard::WIZARD_APPEND_DATA);
    m_pbColumns_RH.GrabFocus();
}
// -----------------------------------------------------------------------
sal_Bool OWizColumnSelect::LeavePage()
{
    DBG_CHKTHIS(OWizColumnSelect,NULL);

    //  m_pParent->getColumns()->clear();

    for(sal_uInt16 i=0 ; i< m_lbNewColumnNames.GetEntryCount();++i)
    {
        OFieldDescription* pField = static_cast<OFieldDescription*>(m_lbNewColumnNames.GetEntryData(i));
        OSL_ENSURE(pField,"The field information can not be null!");
        m_pParent->insertColumn(i,pField);
    }

    if(m_pParent->WasButtonPressed() == OCopyTableWizard::WIZARD_NEXT || m_pParent->WasButtonPressed() == OCopyTableWizard::WIZARD_FINISH)
        return m_pParent->getDestColumns()->size();
    else
        return sal_True;
}
// -----------------------------------------------------------------------
IMPL_LINK( OWizColumnSelect, ButtonClickHdl, Button *, pButton )
{
    MultiListBox *pLeft,*pRight;
    sal_Bool bAll = sal_False;
    if(pButton == &m_pbColumn_RH)
    {
        pLeft  = &m_lbOrgColumnNames;
        pRight = &m_lbNewColumnNames;
    }
    else if(pButton == &m_pbColumn_LH)
    {
        pLeft  = &m_lbNewColumnNames;
        pRight = &m_lbOrgColumnNames;
    }
    else if(pButton == &m_pbColumns_RH)
    {
        pLeft  = &m_lbOrgColumnNames;
        pRight = &m_lbNewColumnNames;
        bAll   = sal_True;
    }
    else if(pButton == &m_pbColumns_LH)
    {
        pLeft  = &m_lbNewColumnNames;
        pRight = &m_lbOrgColumnNames;
        bAll   = sal_True;
    }

    //////////////////////////////////////////////////////////////////////
    // Wenn Datenbank PrimaryKeys verarbeiten kann, PrimaryKey anlegen
    Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());

    sal_Bool bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();
    sal_Int32 nLen = xMetaData->getMaxColumnNameLength();

    //  DlgFieldMatch   dlgMissingFields(GetpApp()->GetDefDialogParent());
    //  ListBox*        pInfoBox = dlgMissingFields.GetInfoBox();

    String aColumnName,aOldColName;
    if(!bAll)
    {
        for(sal_uInt16 i=0; i < pLeft->GetSelectEntryCount(); ++i)
        {
            aColumnName = pLeft->GetSelectEntry(i);
            aOldColName = aColumnName;
            if(pRight == &m_lbNewColumnNames)
            {
                OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeft->GetEntryData(pLeft->GetEntryPos(aColumnName)));
//              if(!bPKeyAllowed)
//                  pColumn->Put(SfxBoolItem(SBA_DEF_FLTPRIMARY, sal_False));
                if(nLen && nLen < aColumnName.Len())
                {
                    String aNewName( aColumnName.Copy( 0, (xub_StrLen)nLen ));
                    String aBaseName( aColumnName.Copy( 0, (xub_StrLen)nLen-2 ));
                    sal_uInt16 i=1;
                    while( pLeft->GetEntryPos(aNewName) == CONTAINER_ENTRY_NOTFOUND && (i<100) )
                    {
                        aNewName = aBaseName;
                        aNewName += String::CreateFromInt32(i);
                        i++;
                    }
                    m_pParent->m_mNameMapping[aOldColName] = aNewName;
                    aColumnName = aNewName;
                }
                else
                    m_pParent->m_mNameMapping[aOldColName] = aColumnName;

                // now create a column
                OFieldDescription* pNewField = new OFieldDescription(*pSrcField);
                pNewField->SetName(aColumnName);
                pRight->SetEntryData(pRight->InsertEntry(aColumnName),pNewField);
            }
            else
            {
                OCopyTableWizard::TNameMapping::const_iterator aIter =  m_pParent->m_mNameMapping.begin();
                for(;aIter != m_pParent->m_mNameMapping.end();++aIter)
                {
                    if(aIter->second == ::rtl::OUString(aColumnName))
                        break;
                }
                DBG_ASSERT(aIter != m_pParent->m_mNameMapping.end(),"Column must to be defined");
                const ODatabaseExport::TColumns* pSrcColumns = m_pParent->getSourceColumns();
                ODatabaseExport::TColumns::const_iterator aSrcIter = pSrcColumns->find((*aIter).first);
                if(aSrcIter != pSrcColumns->end())
                    pRight->SetEntryData(pRight->InsertEntry((*aIter).first),aSrcIter->second);
            }

        }
        for(sal_uInt16 j=pLeft->GetSelectEntryCount(); j ; --j)
            pLeft->RemoveEntry(pLeft->GetSelectEntry(j-1));
    }
    else
    {
        for(sal_uInt16 i=pLeft->GetEntryCount(); i ; --i)
        {
            aColumnName = pLeft->GetEntry(i-1);
            aOldColName = aColumnName;

            if(pRight == &m_lbNewColumnNames)
            {
                OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeft->GetEntryData(i-1));
//              if(!bPKeyAllowed)
//                  pColumn->Put(SfxBoolItem(SBA_DEF_FLTPRIMARY, sal_False));
                if(nLen && nLen < aColumnName.Len())
                {
                    String aNewName( aColumnName.Copy( 0, (xub_StrLen)nLen ));
                    String aBaseName( aColumnName.Copy( 0, (xub_StrLen)nLen-2 ));
                    sal_uInt16 i=1;
                    while( pLeft->GetEntryPos(aNewName) == CONTAINER_ENTRY_NOTFOUND && (i<100) )
                    {
                        aNewName = aBaseName;
                        aNewName += String(i);
                        i++;
                    }
                    m_pParent->m_mNameMapping[aOldColName] = aNewName;
                    aColumnName = aNewName;
                }
                else
                    m_pParent->m_mNameMapping[aOldColName] = aColumnName;

                // now create a column
                OFieldDescription* pNewField = new OFieldDescription(*pSrcField);
                pNewField->SetName(aColumnName);
                pRight->SetEntryData(pRight->InsertEntry(aColumnName,0),pNewField);
                pLeft->RemoveEntry(pLeft->GetEntry(i-1));
            }
            else
            {
                OCopyTableWizard::TNameMapping::const_iterator aIter =  m_pParent->m_mNameMapping.begin();
                for(;aIter != m_pParent->m_mNameMapping.end();++aIter)
                {
                    if(aIter->second == ::rtl::OUString(aColumnName))
                        break;
                }
                DBG_ASSERT(aIter != m_pParent->m_mNameMapping.end(),"Column must to be defined");
                const ODatabaseExport::TColumns* pSrcColumns = m_pParent->getSourceColumns();
                ODatabaseExport::TColumns::const_iterator aSrcIter = pSrcColumns->find((*aIter).first);

                if(aSrcIter != pSrcColumns->end())
                {
                    pRight->SetEntryData(pRight->InsertEntry((*aIter).first,0),aSrcIter->second);
                    pLeft->RemoveEntry(pLeft->GetEntry(i-1));
                }
            }
        }
    }

    //  if(pInfoBox->GetEntryCount())
        //  dlgMissingFields.Execute();

    m_pParent->GetOKButton().Enable(m_lbNewColumnNames.GetEntryCount() != 0);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,m_lbNewColumnNames.GetEntryCount() && m_pParent->m_eCreateStyle != OCopyTableWizard::WIZARD_APPEND_DATA);

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
    sal_Bool bPKeyAllowed   = xMetaData->supportsCoreSQLGrammar();
    sal_Int32 nLen          = xMetaData->getMaxColumnNameLength();

    String aColumnName;
    for(sal_uInt16 i=0; i < pLeft->GetSelectEntryCount(); ++i)
    {
        aColumnName = pLeft->GetSelectEntry(i);

        // here I move columns from the left side to the right side
        if(pRight == &m_lbNewColumnNames)
        {
            OFieldDescription* pSrcField = static_cast<OFieldDescription*>(pLeft->GetEntryData(pLeft->GetEntryPos(aColumnName)));
//          if(!bPKeyAllowed)
//              pColumn->Put(SfxBoolItem(SBA_DEF_FLTPRIMARY, sal_False));
            if(nLen && nLen < aColumnName.Len())
            {
                String aNewName( aColumnName.Copy( 0, (xub_StrLen)nLen ));
                String aBaseName( aColumnName.Copy( 0, (xub_StrLen)nLen-2 ));
                sal_uInt16 i=1;
                while( pLeft->GetEntryPos(aColumnName) == CONTAINER_ENTRY_NOTFOUND && (i<100) )
                {
                    aNewName = aBaseName;
                    aNewName += String(i);
                    i++;
                }
                m_pParent->m_mNameMapping[aColumnName] = aNewName;
                aColumnName = aNewName;
            }
            else
                m_pParent->m_mNameMapping[aColumnName] = aColumnName;

            // now create a column
            OFieldDescription* pNewField = new OFieldDescription(*pSrcField);
            pNewField->SetName(aColumnName);

            pRight->SetEntryData(pRight->InsertEntry(aColumnName),pNewField);
        }
        else
        {
            OCopyTableWizard::TNameMapping::const_iterator aIter =  m_pParent->m_mNameMapping.begin();
            for(;aIter != m_pParent->m_mNameMapping.end();++aIter)
            {
                if(aIter->second == ::rtl::OUString(aColumnName))
                    break;
            }
            DBG_ASSERT(aIter != m_pParent->m_mNameMapping.end(),"Column must to be defined");
            const ODatabaseExport::TColumns* pSrcColumns = m_pParent->getSourceColumns();
            ODatabaseExport::TColumns::const_iterator aSrcIter = pSrcColumns->find((*aIter).first);

            if(aSrcIter != pSrcColumns->end())
                pRight->SetEntryData(pRight->InsertEntry((*aIter).first),aSrcIter->second);
        }

    }
    for(sal_uInt16 j=pLeft->GetSelectEntryCount(); j ; --j)
        pLeft->RemoveEntry(pLeft->GetSelectEntry(j-1));

    m_pParent->GetOKButton().Enable(m_lbNewColumnNames.GetEntryCount() != 0);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,m_lbNewColumnNames.GetEntryCount() && m_pParent->m_eCreateStyle != OCopyTableWizard::WIZARD_APPEND_DATA);
    return 0;
}
// -----------------------------------------------------------------------------



