/*************************************************************************
 *
 *  $RCSfile: commonpagesdbp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-21 09:21:36 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#include "commonpagesdbp.hxx"
#endif
#ifndef _EXTENSIONS_DBP_DBPRESID_HRC_
#include "dbpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;

    //=====================================================================
    //= OTableSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    OTableSelectionPage::OTableSelectionPage(OControlWizard* _pParent)
        :OControlWizardPage(_pParent, ModuleRes(RID_PAGE_TABLESELECTION))
        ,m_aData            (this, ResId(FL_DATA))
        ,m_aDatasourceLabel (this, ResId(FT_DATASOURCE))
        ,m_aDatasource      (this, ResId(LB_DATASOURCE))
        ,m_aTableLabel      (this, ResId(FT_TABLE))
        ,m_aTable           (this, ResId(LB_TABLE))
    {
        FreeResource();

        implCollectDatasource();

        m_aDatasource.SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_aTable.SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));

        m_aDatasource.SetDropDownLineCount(10);
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::ActivatePage()
    {
        OControlWizardPage::ActivatePage();
        m_aDatasource.GrabFocus();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OTableSelectionPage, OnListboxSelection, ListBox*, _pBox )
    {
        if (&m_aDatasource == _pBox)
        {   // new data source selected
            implFillTables();
        }
        else
        {
        }
        return 0L;
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::implFillTables()
    {
        m_aTable.Clear();
        if (!m_xDSContext.is())
            return;

        WaitObject aWaitCursor(this);
        // get the default SDB interaction handler
        Reference< XMultiServiceFactory > xORB = getServiceFactory();
        Reference< XInteractionHandler > xHandler;
        const ::rtl::OUString sInteractionHandlerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.InteractionHandler");
        try
        {
            if (xORB.is())
                xHandler = Reference< XInteractionHandler >(xORB->createInstance(sInteractionHandlerServiceName), UNO_QUERY);
        }
        catch(Exception&) { }
        if (!xHandler.is() && xORB.is())
        {
            ShowServiceNotAvailableError(this, sInteractionHandlerServiceName, sal_True);
            return;
        }

        // will be the table tables of the selected data source
        Sequence< ::rtl::OUString > aTableNames;

        // connect tot the data source
        Any aSQLException;
        try
        {
            // obtain the DS object
            Reference< XCompletedConnection > xDatasource;
            m_xDSContext->getByName(m_aDatasource.GetSelectEntry()) >>= xDatasource;

            // connect
            Reference< XConnection > xConn;
            if (xDatasource.is())
                xConn = xDatasource->connectWithCompletion(xHandler);
            else
                DBG_ERROR("OTableSelectionPage::implFillTables: invalid data source object returned by the context");

            // get the tables
            Reference< XTablesSupplier > xSupplTables(xConn, UNO_QUERY);
            if (xSupplTables.is())
            {
                Reference< XNameAccess > xTables(xSupplTables->getTables(), UNO_QUERY);
                if (xTables.is())
                    aTableNames = xTables->getElementNames();
            }

            // TODO: dispose the connection ... event better: share it with the other pages ...
        }
        catch(SQLContext& e) { aSQLException <<= e; }
        catch(SQLWarning& e) { aSQLException <<= e; }
        catch(SQLException& e) { aSQLException <<= e; }
        catch (Exception&)
        {
            DBG_ERROR("OTableSelectionPage::implFillTables: could not fill the table list!");
        }

        if (aSQLException.hasValue() && xHandler.is())
        {   // an SQLException (or derivee) was thrown ...
            Reference< XInteractionRequest > xRequest = new OInteractionRequest(aSQLException);
            try
            {
                xHandler->handle(xRequest);
            }
            catch(Exception&) { }
            return;
        }

        // insert the table names into the list
        const ::rtl::OUString* pTables = aTableNames.getConstArray();
        const ::rtl::OUString* pEnd = pTables + aTableNames.getLength();
        for (; pTables != pEnd; ++pTables)
            m_aTable.InsertEntry(*pTables);
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::implCollectDatasource()
    {
        Reference< XMultiServiceFactory > xORB = getServiceFactory();
        try
        {
            DBG_ASSERT(xORB.is(), "OTableSelectionPage::implCollectDatasource: invalid service factory!");

            Reference< XInterface > xContext;
            if (xORB.is())
                xContext = xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseContext"));
            DBG_ASSERT(xContext.is(), "OTableSelectionPage::implCollectDatasource: invalid database context!");

            m_xDSContext = Reference< XNameAccess >(xContext, UNO_QUERY);
            DBG_ASSERT(m_xDSContext.is() || !xContext.is(), "OTableSelectionPage::implCollectDatasource: invalid database context (missing the XNameAccess)!");
            if (m_xDSContext.is())
            {
                Sequence< ::rtl::OUString > aNames = m_xDSContext->getElementNames();
                const ::rtl::OUString* pLoop = aNames.getConstArray();
                const ::rtl::OUString* pEnd = pLoop + aNames.getLength();
                for (; pLoop != pEnd; ++pLoop)
                    m_aDatasource.InsertEntry(String(*pLoop));
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("OTableSelectionPage::implCollectDatasource: could not collect the data source names!");
        }
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 14.02.01 11:03:33  fs
 ************************************************************************/

