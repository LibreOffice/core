/*************************************************************************
 *
 *  $RCSfile: WCPage.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:06:42 $
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

#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
//========================================================================
// Klasse OCopyTable
//========================================================================
DBG_NAME(OCopyTable);
//------------------------------------------------------------------------
OCopyTable::OCopyTable( Window * pParent, EImportMode atWhat, sal_Bool bIsView, OCopyTableWizard::Wizard_Create_Style nLastAction )
    : OWizardPage( pParent, ModuleRes(TAB_WIZ_COPYTABLE) ),
    m_ftTableName(          this, ResId( FT_TABLENAME       ) ),
    m_edTableName(          this, ResId( ET_TABLENAME       ) ),
    m_aFL_Options(          this, ResId( FL_OPTIONS         ) ),
    m_aRB_DefData(          this, ResId( RB_DEFDATA         ) ),
    m_aRB_Def(              this, ResId( RB_DEF             ) ),
    m_aRB_View(             this, ResId( RB_VIEW            ) ),
    m_aRB_AppendData(       this, ResId( RB_APPENDDATA      ) ),
    m_aCB_PrimaryColumn(    this, ResId( CB_PRIMARY_COLUMN  ) ),
    m_aFT_KeyName(          this, ResId( FT_KEYNAME         ) ),
    m_edKeyName(            this, ResId( ET_KEYNAME         ) ),
    m_pPage2(NULL),
    m_pPage3(NULL),
    m_bIsViewAllowed(bIsView)
{
    DBG_CTOR(OCopyTable,NULL);

    m_edTableName.SetMaxTextLen( EDIT_NOLIMIT );

    if(m_pParent->m_xConnection.is())
    {
        // first we have to determine if we support views
        Reference< XDatabaseMetaData >  xMetaData(m_pParent->m_xConnection->getMetaData());
        Reference< XViewsSupplier > xViewSups(m_pParent->m_xConnection,UNO_QUERY);
        if(!(m_bIsViewAllowed = xViewSups.is()))
        {
            try
            {
                const static ::rtl::OUString sVIEW = ::rtl::OUString::createFromAscii("VIEW");
                Reference<XResultSet> xRs = xMetaData->getTableTypes();
                if ( xRs.is() )
                {
                    Reference<XRow> xRow(xRs,UNO_QUERY);
                    while ( xRs->next() )
                    {
                        ::rtl::OUString sValue = xRow->getString(1);
                        if ( !xRow->wasNull() && sValue.equalsIgnoreAsciiCase(sVIEW) )
                        {
                            m_bIsViewAllowed = sal_True;
                            break;
                        }
                    }
                }
            }
            catch(const SQLException& e)
            {
                ::dbaui::showError(SQLExceptionInfo(e),pParent->GetParent(),m_pParent->m_xFactory);
            }
        }

        if ( !m_bIsViewAllowed || bIsView ) // if it is a view disable the view checkbox #100644# OJ
            m_aRB_View.Disable();

        //////////////////////////////////////////////////////////////////////
        // do we support pkeys
        try
        {
            m_bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();
        }
        catch(const SQLException&)
        {
            m_bPKeyAllowed = sal_False;
        }

        m_aCB_PrimaryColumn.Enable(m_bPKeyAllowed);

        // reselect the last action before
        switch(nLastAction)
        {
            case OCopyTableWizard::WIZARD_DEF_DATA:
                m_aRB_DefData.Check(sal_True);
                break;
            case OCopyTableWizard::WIZARD_DEF:
                m_aRB_Def.Check(sal_True);
                break;
            case OCopyTableWizard::WIZARD_APPEND_DATA:
                m_aRB_AppendData.Check(sal_True);
                m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_False);
                break;
            case OCopyTableWizard::WIZARD_DEF_VIEW:
                if(m_bIsViewAllowed)
                {
                    m_aRB_View.Check(sal_True);
                    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_False);
                }
                else
                    m_aRB_DefData.Check(sal_True);
        }
        m_aRB_AppendData.SetClickHdl(   LINK( this, OCopyTable, AppendDataClickHdl  ) );

        m_aRB_DefData.SetClickHdl(      LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_aRB_Def.SetClickHdl(          LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_aRB_View.SetClickHdl(         LINK( this, OCopyTable, RadioChangeHdl      ) );

        m_aCB_PrimaryColumn.SetClickHdl( LINK( this, OCopyTable, KeyClickHdl ) );

        m_aFT_KeyName.Enable(sal_False);
        m_edKeyName.Enable(sal_False);

        ::rtl::OUString sKeyName(::rtl::OUString::createFromAscii("ID"));
        sKeyName = m_pParent->createUniqueName(sKeyName);
        m_edKeyName.SetText(sKeyName);

        sal_Int32 nMaxLen = m_pParent->getMaxColumnNameLength();
        m_edKeyName.SetMaxTextLen(nMaxLen ? (xub_StrLen)nMaxLen : EDIT_NOLIMIT);
    }

    FreeResource();

    if (MOVE != atWhat)
        SetText(String(ModuleRes(STR_COPYTABLE_TITLE_COPY)));
    else
    {
        SetText(String(ModuleRes(STR_COPYTABLE_TITLE_MOVE)));
        m_aRB_Def.Disable();
            // beim Verschieben ist der Punkt "nur Definition" nicht zulaessig
    }
}

//------------------------------------------------------------------------
OCopyTable::~OCopyTable()
{
    DBG_DTOR(OCopyTable,NULL);
}
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, AppendDataClickHdl, Button*, pButton )
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_True);
    m_aFT_KeyName.Enable(sal_False);
    m_aCB_PrimaryColumn.Enable(sal_False);
    m_edKeyName.Enable(sal_False);
    m_pParent->setCreateStyle(OCopyTableWizard::WIZARD_APPEND_DATA);

    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, RadioChangeHdl, Button*, pButton )
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,pButton != &m_aRB_View);
    sal_Bool bKey = m_bPKeyAllowed && pButton != &m_aRB_View;
    m_aFT_KeyName.Enable(bKey && m_aCB_PrimaryColumn.IsChecked());
    m_edKeyName.Enable(bKey && m_aCB_PrimaryColumn.IsChecked());
    m_aCB_PrimaryColumn.Enable(bKey);

    // set typ what to do
    if( IsOptionDefData() )
        m_pParent->setCreateStyle(OCopyTableWizard::WIZARD_DEF_DATA);
    else if( IsOptionDef() )
        m_pParent->setCreateStyle(OCopyTableWizard::WIZARD_DEF);
    else if( IsOptionView() )
        m_pParent->setCreateStyle(OCopyTableWizard::WIZARD_DEF_VIEW);

    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, KeyClickHdl, Button*, pButton )
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_edKeyName.Enable(m_aCB_PrimaryColumn.IsChecked());
    m_aFT_KeyName.Enable(m_aCB_PrimaryColumn.IsChecked());
    return 0;
}
//------------------------------------------------------------------------
sal_Bool OCopyTable::LeavePage()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->m_bCreatePrimaryColumn   = (m_bPKeyAllowed && m_aCB_PrimaryColumn.IsEnabled()) ? m_aCB_PrimaryColumn.IsChecked() : sal_False;
    m_pParent->m_aKeyName               = m_pParent->m_bCreatePrimaryColumn ? m_edKeyName.GetText() : String();

    // first check if the table already exists in the database
    if( m_pParent->getCreateStyle() != OCopyTableWizard::WIZARD_APPEND_DATA )
    {
        Reference<XTablesSupplier > xSup(m_pParent->m_xConnection,UNO_QUERY);
        Reference<XNameAccess> xTables;
        if ( xSup.is() )
            xTables = xSup->getTables();
        if ( xTables.is() && xTables->hasByName(m_edTableName.GetText()) )
        {
            String aInfoString( ModuleRes(STR_ERR_DUPL_TABLENAME) );
            aInfoString.SearchAndReplaceAscii("$name$",m_edTableName.GetText());
            InfoBox aNameInfoBox( this, aInfoString );
            aNameInfoBox.Execute();
            return sal_False;
        }
        else // we have to check the length of the table name
        {
            Reference<XDatabaseMetaData> xMeta = m_pParent->m_xConnection->getMetaData();
            ::rtl::OUString sCatalog;
            ::rtl::OUString sSchema;
            ::rtl::OUString sTable;
            ::dbtools::qualifiedNameComponents( xMeta,
                                                m_edTableName.GetText(),
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::eInDataManipulation);
            sal_Int32 nMaxLength = xMeta->getMaxTableNameLength();
            if ( nMaxLength && sTable.getLength() > nMaxLength )
            {
                ErrorBox(this, ModuleRes(ERROR_INVALID_TABLE_NAME_LENGTH)).Execute();
                return sal_False;
            }
        }

        // now we have to check if the name of the primary key already exists
        if (    m_pParent->m_bCreatePrimaryColumn
            &&  m_pParent->m_aKeyName != m_pParent->createUniqueName(m_pParent->m_aKeyName) )
        {
            String aInfoString( ModuleRes(STR_WIZ_PKEY_ALREADY_DEFINED) );
            aInfoString += String(' ');
            aInfoString += String(m_pParent->m_aKeyName);
            InfoBox aNameInfoBox( this, aInfoString );
            aNameInfoBox.Execute();
            return sal_False;
        }
    }

    if ( !m_edTableName.GetSavedValue().Equals(m_edTableName.GetText()) )
    { // table exists and name has changed
        if ( m_pParent->getCreateStyle() == OCopyTableWizard::WIZARD_APPEND_DATA )
        {
            if(!checkAppendData())
                return sal_False;
        }
        else if(m_eOldStyle == OCopyTableWizard::WIZARD_APPEND_DATA)
        {
            m_pParent->m_xDestObject = NULL;
            m_edTableName.SaveValue();
            return LeavePage();
        }
    }
    else
    { // table exist and is not new or doesn't exist and so on
        switch(m_pParent->getCreateStyle())
        {
            case OCopyTableWizard::WIZARD_APPEND_DATA:
            {
                if(!checkAppendData())
                    return sal_False;
                break;
            }
            case OCopyTableWizard::WIZARD_DEF_DATA:
            case OCopyTableWizard::WIZARD_DEF:
            {
                //////////////////////////////////////////////////////////////////////////
                // Immer neue Tabelle erzeugen

            }
        }
    }
    m_pParent->m_sName = m_edTableName.GetText();
    m_edTableName.SaveValue();

    if(!m_pParent->m_sName.getLength())
    {
        ErrorBox(this, ModuleRes(ERROR_INVALID_TABLE_NAME)).Execute();
        return sal_False;
    }

    return sal_True;
}
//------------------------------------------------------------------------
void OCopyTable::ActivatePage()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->GetOKButton().Enable( TRUE );
    m_eOldStyle = m_pParent->getCreateStyle();
    m_edTableName.GrabFocus();
}
//------------------------------------------------------------------------
String OCopyTable::GetTitle() const
{
    DBG_CHKTHIS(OCopyTable,NULL);
    return String(ModuleRes(STR_WIZ_TABLE_COPY));
}
//------------------------------------------------------------------------
void OCopyTable::Reset()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_bFirstTime = sal_False;

    m_edTableName.SetText( m_pParent->m_sName );
    m_edTableName.SaveValue();
}
//------------------------------------------------------------------------
sal_Bool OCopyTable::checkAppendData()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->clearDestColumns();
    m_pParent->m_xDestObject = NULL;
    Reference<XTablesSupplier > xSup(m_pParent->m_xConnection,UNO_QUERY);
    Reference<XNameAccess> xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    if(xTables.is() && xTables->hasByName(m_edTableName.GetText()))
    {
        // set new destination
        xTables->getByName(m_edTableName.GetText()) >>= m_pParent->m_xDestObject;
        m_pParent->loadData(m_pParent->m_xDestObject,m_pParent->m_vDestColumns,m_pParent->m_aDestVec);
        // #90027#
        const ODatabaseExport::TColumnVector* pDestColumns          = m_pParent->getDestVector();
        ODatabaseExport::TColumnVector::const_iterator aDestIter    = pDestColumns->begin();
        m_pParent->m_vColumnPos.reserve(pDestColumns->size()+1);
        m_pParent->m_vColumnTypes.reserve(pDestColumns->size()+1);
        sal_Bool bNotConvert;
        for(sal_Int32 nPos = 1;aDestIter != pDestColumns->end();++aDestIter,++nPos)
        {
            bNotConvert = sal_True;
            m_pParent->m_vColumnPos.push_back( ODatabaseExport::TPositions::value_type(nPos,nPos) );
            TOTypeInfoSP pTypeInfo = m_pParent->convertType((*aDestIter)->second->getTypeInfo(),bNotConvert);
            if ( !bNotConvert )
            {
                m_pParent->showColumnTypeNotSupported((*aDestIter)->first);
                return sal_False;
            }

            if ( pTypeInfo.get() )
                m_pParent->m_vColumnTypes.push_back(pTypeInfo->nType);
            else
                m_pParent->m_vColumnTypes.push_back(DataType::VARCHAR);
        }
    }
    if ( !m_pParent->m_xDestObject.is() )
    {
        ErrorBox(this, ModuleRes(ERROR_INVALID_TABLE_NAME)).Execute();
        return sal_False;
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
