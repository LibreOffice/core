/*************************************************************************
 *
 *  $RCSfile: addresstemplate.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2000-11-28 14:41:53 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#include <stdio.h>


#ifndef _SVT_ADDRESSTEMPLATE_HXX_
#include "addresstemplate.hxx"
#endif
#ifndef _SVT_ADDRESSTEMPLATE_HRC_
#include "addresstemplate.hrc"
#endif
#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif

// .......................................................................
namespace svt
{
// .......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::ui;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;
    using namespace ::utl;

    // ===================================================================
    // = AddressBookAssignment
    // ===================================================================
    // -------------------------------------------------------------------
    AddressBookAssignment::AddressBookAssignment()
        :ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.DataAccess/AddressBook" )))
    {
        Sequence< ::rtl::OUString > aStoredNames = GetNodeNames(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fields")));
        const ::rtl::OUString* pStoredNames = aStoredNames.getConstArray();
        for (sal_Int32 i=0; i<aStoredNames.getLength(); ++i, ++pStoredNames)
            m_aStoredFields.insert(*pStoredNames);
    }

    // -------------------------------------------------------------------
    AddressBookAssignment::~AddressBookAssignment()
    {
        Commit();
    }

    // -------------------------------------------------------------------
    sal_Bool AddressBookAssignment::hasFieldAssignment(const ::rtl::OUString& _rLogicalName)
    {
        return (m_aStoredFields.end() != m_aStoredFields.find(_rLogicalName));
    }

    // -------------------------------------------------------------------
    ::rtl::OUString AddressBookAssignment::getFieldAssignment(const ::rtl::OUString& _rLogicalName)
    {
        ::rtl::OUString sAssignment;
        if (hasFieldAssignment(_rLogicalName))
        {
            ::rtl::OUString sFieldPath(RTL_CONSTASCII_USTRINGPARAM("Fields/"));
            sFieldPath += _rLogicalName;
            sFieldPath += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/DatasourceFieldName"));
            sAssignment = getStringProperty(sFieldPath);
        }
        return sAssignment;
    }

    // -------------------------------------------------------------------
    Any AddressBookAssignment::getProperty(const sal_Char* _pLocalName)
    {
        return getProperty(::rtl::OUString::createFromAscii(_pLocalName));
    }

    // -------------------------------------------------------------------
    Any AddressBookAssignment::getProperty(const ::rtl::OUString& _rLocalName)
    {
        Sequence< ::rtl::OUString > aProperties(&_rLocalName, 1);
        Sequence< Any > aValues = GetProperties(aProperties);
        DBG_ASSERT(aValues.getLength() == 1, "AddressBookAssignment::getProperty: invalid sequence length!");
        return aValues[0];
    }

    // -------------------------------------------------------------------
    ::rtl::OUString AddressBookAssignment::getStringProperty(const ::rtl::OUString& _rLocalName)
    {
        ::rtl::OUString sReturn;
        getProperty( _rLocalName ) >>= sReturn;
        return sReturn;
    }

    // -------------------------------------------------------------------
    ::rtl::OUString AddressBookAssignment::getStringProperty(const sal_Char* _pLocalName)
    {
        ::rtl::OUString sReturn;
        getProperty( _pLocalName ) >>= sReturn;
        return sReturn;
    }

    // -------------------------------------------------------------------
    sal_Int32 AddressBookAssignment::getInt32Property(const sal_Char* _pLocalName)
    {
        sal_Int32 nReturn;
        getProperty( _pLocalName ) >>= nReturn;
        return nReturn;
    }

    // -------------------------------------------------------------------
    void AddressBookAssignment::setStringProperty(const sal_Char* _pLocalName, const ::rtl::OUString& _rValue)
    {
        Sequence< ::rtl::OUString > aNames(1);
        Sequence< Any > aValues(1);
        aNames[0] = ::rtl::OUString::createFromAscii(_pLocalName);
        aValues[0] <<= _rValue;
        PutProperties(aNames, aValues);
    }

    // -------------------------------------------------------------------
    void AddressBookAssignment::setFieldAssignment(const ::rtl::OUString& _rLogicalName, const ::rtl::OUString& _rAssignment)
    {
        if (!_rAssignment.getLength())
        {
            if (hasFieldAssignment(_rLogicalName))
                // the assignment exists but it should be reset
                clearFieldAssignment(_rLogicalName);
                return;
        }

        // Fields
        ::rtl::OUString sDescriptionNodePath(RTL_CONSTASCII_USTRINGPARAM("Fields"));

        // Fields/<field>
        ::rtl::OUString sFieldElementNodePath(sDescriptionNodePath);
        sFieldElementNodePath += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        sFieldElementNodePath += _rLogicalName;

        Sequence< PropertyValue > aNewFieldDescription(2);
        // Fields/<field>/FieldName
        aNewFieldDescription[0].Name = sFieldElementNodePath;
        aNewFieldDescription[0].Name += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/FieldName"));
        aNewFieldDescription[0].Value <<= _rLogicalName;
        // Fields/<field>/DatabaseFieldName
        aNewFieldDescription[1].Name = sFieldElementNodePath;
        aNewFieldDescription[1].Name += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/DatasourceFieldName"));
        aNewFieldDescription[1].Value <<= _rAssignment;

        // just set the new value
#ifdef DBG_UTIL
        sal_Bool bSuccess =
#endif
        SetSetProperties(sDescriptionNodePath, aNewFieldDescription);
        DBG_ASSERT(bSuccess, "AddressBookAssignment::setFieldAssignment: could not commit the changes a field!");
    }

    // -------------------------------------------------------------------
    void AddressBookAssignment::clearFieldAssignment(const ::rtl::OUString& _rLogicalName)
    {
        if (!hasFieldAssignment(_rLogicalName))
            // nothing to do
            return;

        ::rtl::OUString sDescriptionNodePath(RTL_CONSTASCII_USTRINGPARAM("Fields"));
        Sequence< ::rtl::OUString > aNames(&_rLogicalName, 1);
        ClearNodeElements(sDescriptionNodePath, aNames);
    }

    // -------------------------------------------------------------------
    ::rtl::OUString AddressBookAssignment::getDatasourceName()
    {
        return getStringProperty( "DataSourceName" );
    }

    // -------------------------------------------------------------------
    ::rtl::OUString AddressBookAssignment::getCommand()
    {
        return getStringProperty( "Command" );
    }

    // -------------------------------------------------------------------
    void AddressBookAssignment::setDatasourceName(const ::rtl::OUString& _rName)
    {
        setStringProperty( "DataSourceName", _rName );
    }

    // -------------------------------------------------------------------
    void AddressBookAssignment::setCommand(const ::rtl::OUString& _rCommand)
    {
        setStringProperty( "Command", _rCommand );
    }

    // -------------------------------------------------------------------
    sal_Int32 AddressBookAssignment::getCommandType()
    {
        return getInt32Property( "CommandType" );
    }

    // ===================================================================
    // = AddressBookSourceDialog
    // ===================================================================
    // -------------------------------------------------------------------
    AddressBookSourceDialog::AddressBookSourceDialog(Window* _pParent,
            const Reference< XMultiServiceFactory >& _rxORB )
        :ModalDialog(_pParent, SvtResId( DLG_ADDRESSBOOKSOURCE ))
        ,m_aDatasourceFrame         (this, ResId(GB_DATASOURCEFRAME))
        ,m_aDatasourceLabel         (this, ResId(FT_DATASOURCE))
        ,m_aDatasource              (this, ResId(CB_DATASOURCE))
        ,m_aAdministrateDatasources (this, ResId(PB_ADMINISTATE_DATASOURCES))
        ,m_aTableLabel              (this, ResId(FT_TABLE))
        ,m_aTable                   (this, ResId(CB_TABLE))
        ,m_aFieldsFrame             (this, ResId(GB_FIELDS))
        ,m_nFieldScrollPos(-1)
        ,m_aFieldScroller           (this, ResId(SB_FIELDSCROLLER))
        ,m_aOK                      (this, ResId(PB_OK))
        ,m_aCancel                  (this, ResId(PB_CANCEL))
        ,m_sNoFieldSelection(ResId(STR_NO_FIELD_SELECTION))
        ,m_xORB(_rxORB)
        ,m_bOddFieldNumber(sal_False)
        ,m_nLastVisibleListIndex(0)
    {
        DBG_ASSERT(sizeof(m_pFieldLabels)/sizeof(m_pFieldLabels[0]) == FIELD_CONTROLS_VISIBLE,
            "AddressBookSourceDialog::AddressBookSourceDialog: invalid member count!");
            // we export our header, and FIELD_PAIRS_VISIBLE is defined in the .hrc, which is not
            // exported, so we have to hard-code the value of FIELD_PAIRS_VISIBLE in our header ...

        for (sal_Int32 row=0; row<FIELD_PAIRS_VISIBLE; ++row)
        {
            for (sal_Int32 column=0; column<2; ++column)
            {
                // the label
                m_pFieldLabels[row * 2 + column] = new FixedText(this, ResId((USHORT)(FT_FIELD_BASE + row * 2 + column)));
                // the listbox
                m_pFields[row * 2 + column] = new ListBox(this, ResId((USHORT)(LB_FIELD_BASE + row * 2 + column)));
                m_pFields[row * 2 + column]->SetDropDownLineCount(8);
                m_pFields[row * 2 + column]->SetSelectHdl(LINK(this, AddressBookSourceDialog, OnFieldSelect));
            }
        }

        // correct the z-order
        m_aFieldScroller.SetZOrder(m_pFields[FIELD_CONTROLS_VISIBLE - 1], WINDOW_ZORDER_BEHIND);
        m_aOK.SetZOrder(&m_aFieldScroller, WINDOW_ZORDER_BEHIND);
        m_aCancel.SetZOrder(&m_aOK, WINDOW_ZORDER_BEHIND);

        initializeDatasources();

        // for the moment, we have a hard coded list of all known fields.
        // A better solution would be to store all known field translations in the configuration, which could be
        // extensible by the user in an arbitrary way.
        // But for the moment we need a quick solution ...
        // (the main thing would be to store the translations to use here in the user interface, besides that, the code
        // should be adjustable with a rather small effort.)

        // initialize the strings for the field labels
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_COMPANY)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_DEPARTMENT )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_FIRSTNAME )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_LASTNAME )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_STREET )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_COUNTRY )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_ZIPCODE )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_CITY )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_TITLE )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_POSITION )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_ADDRFORM )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_INITIALS )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_SALUTATION )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_HOMETEL )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_WORKTEL )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_FAX )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_EMAIL )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_URL )) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_NOTE)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_USER1)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_USER2)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_USER3)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_USER4)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_ID)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_STATE)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_OFFICETEL)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_PAGER)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_MOBILE)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_TELOTHER)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_CALENDAR)) );
        m_aFieldLabels.push_back( String(ResId( STR_FIELD_INVITE)) );

        // force a even number of known fields
        m_bOddFieldNumber = (m_aFieldLabels.size() % 2) != 0;
        if (m_bOddFieldNumber)
            m_aFieldLabels.push_back( String() );

        // limit the scrollbar range accordingly
        sal_Int32 nOverallFieldPairs = m_aFieldLabels.size() / 2;
        m_aFieldScroller.SetRange( Range(0, nOverallFieldPairs - FIELD_PAIRS_VISIBLE) );
        m_aFieldScroller.SetLineSize(1);
        m_aFieldScroller.SetPageSize(FIELD_PAIRS_VISIBLE);

        // reset the current field assignments
        m_aFieldAssignments.resize(m_aFieldLabels.size());
            // (empty strings mean "no assignment")

        // some knittings
        m_aFieldScroller.SetScrollHdl(LINK(this, AddressBookSourceDialog, OnFieldScroll));
        m_aAdministrateDatasources.SetClickHdl(LINK(this, AddressBookSourceDialog, OnAdministrateDatasources));
        m_aDatasource.EnableAutocomplete(sal_True);
        m_aTable.EnableAutocomplete(sal_True);
        m_aTable.SetGetFocusHdl(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_aDatasource.SetGetFocusHdl(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_aTable.SetLoseFocusHdl(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_aDatasource.SetLoseFocusHdl(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_aTable.SetSelectHdl(LINK(this, AddressBookSourceDialog, OnComboSelect));
        m_aDatasource.SetSelectHdl(LINK(this, AddressBookSourceDialog, OnComboSelect));
        m_aOK.SetClickHdl(LINK(this, AddressBookSourceDialog, OnOkClicked));

        // initialize the field controls
        resetFields();
        m_aFieldScroller.SetThumbPos(0);
        implScrollFields(0, sal_False, sal_False);

        // the logical names
        String sLogicalFieldNames(ResId(STR_LOCAGICAL_FIELD_NAMES));
        sal_Int32 nAdjustedTokenCount = sLogicalFieldNames.GetTokenCount(';') + (m_bOddFieldNumber ? 1 : 0);
        DBG_ASSERT(nAdjustedTokenCount == m_aFieldLabels.size(),
            "AddressBookSourceDialog::AddressBookSourceDialog: inconsistence between logical and UI field names!");
        m_aLogicalFieldNames.reserve(nAdjustedTokenCount);
        for (sal_Int32 i = 0; i<nAdjustedTokenCount; ++i)
            m_aLogicalFieldNames.push_back(sLogicalFieldNames.GetToken(i, ';'));

        PostUserEvent(LINK(this, AddressBookSourceDialog, OnDelayedInitialize));
            // so the dialog will at least show up before we do the loading of the
            // configuration data and the (maybe time consuming) analysis of the data source/table to select

        FreeResource();
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::loadConfiguration()
    {
        m_aDatasource.SetText(m_aConfigData.getDatasourceName());
        m_aTable.SetText(m_aConfigData.getCommand());
        // we ignore the CommandType: only tables are supported

        // the logical names for the fields
        DBG_ASSERT(m_aLogicalFieldNames.size() == m_aFieldAssignments.size(),
            "AddressBookSourceDialog::loadConfiguration: inconsistence between field names and field assignments!");

        ConstStringArrayIterator aLogical = m_aLogicalFieldNames.begin();
        StringArrayIterator aAssignment = m_aFieldAssignments.begin();
        for (   ;
                aLogical < m_aLogicalFieldNames.end();
                ++aLogical, ++aAssignment
            )
            *aAssignment = m_aConfigData.getFieldAssignment(*aLogical);
    }

    // -------------------------------------------------------------------
    AddressBookSourceDialog::~AddressBookSourceDialog()
    {
        sal_Int32 i;
        for (i=0; i<FIELD_CONTROLS_VISIBLE; ++i)
        {
            delete m_pFieldLabels[i];
            delete m_pFields[i];
        }
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::initializeDatasources()
    {
        DBG_ASSERT(m_xORB.is(), "AddressBookSourceDialog::initializeDatasources: no service factory!");
        if (!m_xORB.is())
            return;

        const String sContextServiceName = String::CreateFromAscii("com.sun.star.sdb.DatabaseContext");
        try
        {
            m_xDatabaseContext = Reference< XNameAccess >(m_xORB->createInstance(sContextServiceName), UNO_QUERY);
        }
        catch(Exception&) { }
        if (!m_xDatabaseContext.is())
        {
            ShowServiceNotAvailableError( this, sContextServiceName, sal_False);
            return;
        }
        // fill the datasources listbox
        Sequence< ::rtl::OUString > aDatasourceNames;
        try
        {
            aDatasourceNames = m_xDatabaseContext->getElementNames();
        }
        catch(Exception&)
        {
            DBG_ERROR("AddressBookSourceDialog::initializeDatasources: caught an exception while asking for the data source names!");
        }
        const ::rtl::OUString* pDatasourceNames = aDatasourceNames.getConstArray();
        const ::rtl::OUString* pEnd = pDatasourceNames + aDatasourceNames.getLength();
        for (; pDatasourceNames < pEnd; ++pDatasourceNames)
            m_aDatasource.InsertEntry(*pDatasourceNames);
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnFieldScroll, ScrollBar*, _pScrollBar)
    {
        implScrollFields( _pScrollBar->GetThumbPos(), sal_True, sal_True );
        return 0L;
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::resetTables()
    {
        if (!m_xDatabaseContext.is())
            return;

        WaitObject aWaitCursor(this);

        // no matter what we do here, we handled the currently selected data source (no matter if successfull or not)
        m_aDatasource.SaveValue();

        // create an interaction handler (may be needed for connecting)
        const String sInteractionHandlerServiceName = String::CreateFromAscii("com.sun.star.sdb.InteractionHandler");
        Reference< XInteractionHandler > xHandler;
        try
        {
            xHandler = Reference< XInteractionHandler >(m_xORB->createInstance(sInteractionHandlerServiceName), UNO_QUERY);
        }
        catch(Exception&) { }
        if (!xHandler.is())
        {
            ShowServiceNotAvailableError(this, sInteractionHandlerServiceName, sal_True);
            return;
        }

        m_aTable.Clear();

        m_xCurrentDatasourceTables= NULL;

        // get the tables of the connection
        Sequence< ::rtl::OUString > aTableNames;
        String sSelectedDS = m_aDatasource.GetText();
        Any aException;
        try
        {
            Reference< XConnection > xConn;
            // get the data source the user has chosen and let it build a connection
            if (m_xDatabaseContext->hasByName(sSelectedDS))
            {
                // build the connection
                Reference< XCompletedConnection > xDS;
                ::cppu::extractInterface(xDS, m_xDatabaseContext->getByName(sSelectedDS));
                if (xDS.is())
                    xConn = xDS->connectWithCompletion(xHandler);

                // get the table names
                Reference< XTablesSupplier > xSupplTables(xConn, UNO_QUERY);
                if (xSupplTables.is())
                {
                    m_xCurrentDatasourceTables = Reference< XNameAccess >(xSupplTables->getTables(), UNO_QUERY);
                    if (m_xCurrentDatasourceTables.is())
                        aTableNames = m_xCurrentDatasourceTables->getElementNames();
                }
            }
        }
        catch(SQLContext& e) { aException <<= e; }
        catch(SQLWarning& e) { aException <<= e; }
        catch(SQLException& e) { aException <<= e; }
        catch(Exception&)
        {
            DBG_ERROR("AddressBookSourceDialog::resetTables: could not retrieve the table!");
        }

        if (aException.hasValue())
        {
            Reference< XInteractionRequest > xRequest = new OInteractionRequest(aException);
            try
            {
                xHandler->handle(xRequest);
            }
            catch(Exception&) { }
            return;
        }

        // fill the table list
        const ::rtl::OUString* pTableNames = aTableNames.getConstArray();
        const ::rtl::OUString* pEnd = pTableNames + aTableNames.getLength();
        for (;pTableNames != pEnd; ++pTableNames)
            m_aTable.InsertEntry(*pTableNames);

        resetFields();
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::resetFields()
    {
        WaitObject aWaitCursor(this);

        // no matter what we do here, we handled the currently selected table (no matter if successfull or not)
        m_aDatasource.SaveValue();

        String sSelectedTable = m_aTable.GetText();
        Sequence< ::rtl::OUString > aColumnNames;
        try
        {
            if (m_xCurrentDatasourceTables.is())
            {
                // get the table and the columns
                Reference< XColumnsSupplier > xSuppTableCols;
                if (m_xCurrentDatasourceTables->hasByName(sSelectedTable))
                    ::cppu::extractInterface(xSuppTableCols, m_xCurrentDatasourceTables->getByName(sSelectedTable));
                Reference< XNameAccess > xColumns;
                if (xSuppTableCols.is())
                    xColumns = xSuppTableCols->getColumns();
                if (xColumns.is())
                    aColumnNames = xColumns->getElementNames();
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("AddressBookSourceDialog::resetFields: could not retrieve the table columns!");
        }


        const ::rtl::OUString* pColumnNames = aColumnNames.getConstArray();
        const ::rtl::OUString* pEnd = pColumnNames + aColumnNames.getLength();

        // for quicker access
        ::std::set< String > aColumnNameSet;
        for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
            aColumnNameSet.insert(*pColumnNames);

        ListBox** pListbox = m_pFields;
        String sSaveSelection;
        for (sal_Int32 i=0; i<FIELD_CONTROLS_VISIBLE; ++i, ++pListbox)
        {
            sSaveSelection = (*pListbox)->GetSelectEntry();

            (*pListbox)->Clear();

            // the one entry for "no selection"
            (*pListbox)->InsertEntry(m_sNoFieldSelection, 0);
            // as it's entry data, set the index of the list box in our array
            (*pListbox)->SetEntryData(0, reinterpret_cast<void*>(i));

            // the field names
            for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
                (*pListbox)->InsertEntry(*pColumnNames);

            // try to restore the selection
            if (aColumnNameSet.end() != aColumnNameSet.find(sSaveSelection))
                (*pListbox)->SelectEntry(sSaveSelection);
            else
                (*pListbox)->SelectEntryPos(0);
        }

        // adjust m_aFieldAssignments
        for (   StringArrayIterator aAdjust = m_aFieldAssignments.begin();
                aAdjust != m_aFieldAssignments.end();
                ++aAdjust
            )
            if (aAdjust->Len())
                if (aColumnNameSet.end() == aColumnNameSet.find(*aAdjust))
                    aAdjust->Erase();
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnFieldSelect, ListBox*, _pListbox)
    {
        // the index of the affected list box in our array
        sal_Int32 nListBoxIndex = reinterpret_cast<sal_Int32>(_pListbox->GetEntryData(0));
        DBG_ASSERT(nListBoxIndex >= 0 && nListBoxIndex < FIELD_CONTROLS_VISIBLE,
            "AddressBookSourceDialog::OnFieldScroll: invalid list box entry!");

        // update the array where we remember the field selections
        if (0 == _pListbox->GetSelectEntryPos())
            // it's the "no field selection" entry
            m_aFieldAssignments[m_nFieldScrollPos * 2 + nListBoxIndex] = String();
        else
            // it's a regular field entry
            m_aFieldAssignments[m_nFieldScrollPos * 2 + nListBoxIndex] = _pListbox->GetSelectEntry();

        return 0L;
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::implScrollFields(sal_Int32 _nPos, sal_Bool _bAdjustFocus, sal_Bool _bAdjustScrollbar)
    {
        if (_nPos == m_nFieldScrollPos)
            // nothing to do
            return;

        // loop through our field control rows and do some adjustments
        // for the new texts
        FixedText** pLeftLabelControl = m_pFieldLabels;
        FixedText** pRightLabelControl = pLeftLabelControl + 1;
        ConstStringArrayIterator pLeftColumnLabel = m_aFieldLabels.begin() + 2 * _nPos;
        ConstStringArrayIterator pRightColumnLabel = pLeftColumnLabel + 1;

        // for the focus movement and the selection scroll
        ListBox** pLeftListControl = m_pFields;
        ListBox** pRightListControl = pLeftListControl + 1;

        // for the focus movement
        sal_Int32 nOldFocusRow = -1;
        sal_Int32 nOldFocusColumn = 0;

        // for the selection scroll
        ConstStringArrayIterator pLeftAssignment = m_aFieldAssignments.begin() + 2 * _nPos;
        ConstStringArrayIterator pRightAssignment = pLeftAssignment + 1;

        m_nLastVisibleListIndex = -1;
        // loop
        for (sal_Int32 i=0; i<FIELD_PAIRS_VISIBLE; ++i)
        {
            if ((*pLeftListControl)->HasChildPathFocus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 0;
            }
            else if ((*pRightListControl)->HasChildPathFocus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 1;
            }

            // the new texts of the label controls
            (*pLeftLabelControl)->SetText(*pLeftColumnLabel);
            (*pRightLabelControl)->SetText(*pRightColumnLabel);

            // we may have to hide the controls in the right column, if we have no label text for it
            // (which means we have an odd number of fields, though we forced our internal arrays to
            // be even-sized for easier handling)
            // (If sometimes we support an arbitrary number of field assignments, we would have to care for
            // an invisible left hand side column, too. But right now, the left hand side controls are always
            // visible)
            sal_Bool bHideRightColumn = (0 == pRightColumnLabel->Len());
            (*pRightLabelControl)->Show(!bHideRightColumn);
            (*pRightListControl)->Show(!bHideRightColumn);
            // the new selections of the listboxes
            implSelectField(*pLeftListControl, *pLeftAssignment);
            implSelectField(*pRightListControl, *pRightAssignment);

            // the index of the last visible list box
            ++m_nLastVisibleListIndex;  // the left hand side box is always visible
            if (!bHideRightColumn)
                ++m_nLastVisibleListIndex;

            // increment ...
            ++ ++ pLeftLabelControl;
            ++ ++ pRightLabelControl;
            ++ ++ pLeftColumnLabel;
            ++ ++ pRightColumnLabel;

            ++ ++ pLeftListControl;
            ++ ++ pRightListControl;
            ++ ++ pLeftAssignment;
            ++ ++ pRightAssignment;
        }

        if (_bAdjustFocus && (nOldFocusRow >= 0))
        {   // we have to adjust the focus and one of the list boxes has the focus
            sal_Int32 nDelta = m_nFieldScrollPos - _nPos;
            // the new row for the focus
            sal_Int32 nNewFocusRow = nOldFocusRow + nDelta;
            // normalize
            nNewFocusRow = min(nNewFocusRow, (sal_Int32)(FIELD_PAIRS_VISIBLE - 1), ::std::less< sal_Int32 >());
            nNewFocusRow = max(nNewFocusRow, (sal_Int32)0, ::std::less< sal_Int32 >());
            // set the new focus (in the same column)
            m_pFields[nNewFocusRow * 2 + nOldFocusColumn]->GrabFocus();
        }

        m_nFieldScrollPos = _nPos;

        if (_bAdjustScrollbar)
            m_aFieldScroller.SetThumbPos(m_nFieldScrollPos);
    }

    // -------------------------------------------------------------------
    void AddressBookSourceDialog::implSelectField(ListBox* _pBox, const String& _rText)
    {
        if (_rText.Len())
            // a valid field name
            _pBox->SelectEntry(_rText);
        else
            // no selection for this item
            _pBox->SelectEntryPos(0);
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnDelayedInitialize, void*, EMPTYARG)
    {
        // load the initial data from the configuration
        loadConfiguration();
        resetTables();
            // will reset the tables/fields implicitly

        return 0L;
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnComboSelect, ComboBox*, _pBox)
    {
        if (_pBox == &m_aDatasource)
            resetTables();
        else
            resetFields();
        return 0;
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnComboGetFocus, ComboBox*, _pBox)
    {
        _pBox->SaveValue();
        return 0L;
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnComboLoseFocus, ComboBox*, _pBox)
    {
        if (_pBox->GetSavedValue() != _pBox->GetText())
        {
            if (_pBox == &m_aDatasource)
                resetTables();
            else
                resetFields();
        }
        return 0L;
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnOkClicked, Button*, _pButton)
    {
        m_aConfigData.setDatasourceName(m_aDatasource.GetText());
        m_aConfigData.setCommand(m_aTable.GetText());

        // set the field assignments
        ConstStringArrayIterator aLogical = m_aLogicalFieldNames.begin();
        ConstStringArrayIterator aAssignment = m_aFieldAssignments.begin();
        for (   ;
                aLogical < m_aLogicalFieldNames.end();
                ++aLogical, ++aAssignment
            )
            m_aConfigData.setFieldAssignment(*aLogical, *aAssignment);


        EndDialog(RET_OK);
        return 0L;
    }

    // -------------------------------------------------------------------
    IMPL_LINK(AddressBookSourceDialog, OnAdministrateDatasources, void*, EMPTYARG)
    {
        // collect some initial arguments for the dialog
        Sequence< Any > aArgs(2);
        aArgs[0] <<= PropertyValue(::rtl::OUString::createFromAscii("InitialSelection"), 0, makeAny(::rtl::OUString(m_aDatasource.GetText())), PropertyState_DIRECT_VALUE);
        aArgs[1] <<= PropertyValue(::rtl::OUString::createFromAscii("ParentWindow"), 0, makeAny(VCLUnoHelper::GetInterface(this)), PropertyState_DIRECT_VALUE);

        // create the dialog object
        const String sDialogServiceName = String::CreateFromAscii("com.sun.star.sdb.DatasourceAdministrationDialog");
        Reference< XExecutableDialog > xAdminDialog;
        try
        {
            xAdminDialog = Reference< XExecutableDialog >(m_xORB->createInstanceWithArguments(sDialogServiceName, aArgs), UNO_QUERY);
        }
        catch(Exception&) { }
        if (!xAdminDialog.is())
        {
            ShowServiceNotAvailableError(this, sDialogServiceName, sal_True);
            return 1L;
        }

        // excute the dialog
        try
        {
            xAdminDialog->execute();
        }
        catch(Exception&)
        {
            DBG_ERROR("AddressBookSourceDialog::OnAdministrateDatasources: an error occured while executing the administration dialog!");
        }

        return 0L;
    }

    // -------------------------------------------------------------------
    long AddressBookSourceDialog::PreNotify( NotifyEvent& _rNEvt )
    {
        switch (_rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
            {
                const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
                sal_uInt16 nCode  = pKeyEvent->GetKeyCode().GetCode();
                sal_Bool   bShift = pKeyEvent->GetKeyCode().IsShift();
                sal_Bool   bCtrl  = pKeyEvent->GetKeyCode().IsMod1();
                sal_Bool   bAlt =   pKeyEvent->GetKeyCode().IsMod2();

                if (KEY_TAB == nCode)
                {   // somebody pressed the tab key
                    if (!bAlt && !bCtrl && !bShift)
                    {   // it's really the only the key (no modifiers)
                        if (m_pFields[m_nLastVisibleListIndex]->HasChildPathFocus())
                            // the last of our visible list boxes has the focus
                            if (m_nFieldScrollPos < m_aFieldScroller.GetRangeMax())
                            {   // we can still scroll down
                                sal_Int32 nNextFocusList = m_nLastVisibleListIndex + 1 - 2;
                                // -> scroll down
                                implScrollFields(m_nFieldScrollPos + 1, sal_False, sal_True);
                                // give the left control in the "next" line the focus
                                m_pFields[nNextFocusList]->GrabFocus();
                                // return saying "have handled this"
                                return 1;
                            }
                    }
                    else if (!bAlt && !bCtrl && bShift)
                    {   // it's shift-tab
                        if (m_pFields[0]->HasChildPathFocus())
                            // our first list box has the focus
                            if (m_nFieldScrollPos > 0)
                            {   // we can still scroll up
                                // -> scroll up
                                implScrollFields(m_nFieldScrollPos - 1, sal_False, sal_True);
                                // give the right control in the "prebious" line the focus
                                m_pFields[0 - 1 + 2]->GrabFocus();
                                // return saying "have handled this"
                                return 1;
                            }
                    }
                }
            }
            break;
        }
        return ModalDialog::PreNotify(_rNEvt);
    }

// .......................................................................
}   // namespace svt
// .......................................................................

