/*************************************************************************
 *
 *  $RCSfile: controlwizard.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-03 12:42:48 $
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

#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#include "controlwizard.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETVIEW_HPP_
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::task;
    using namespace ::svt;
    using namespace ::comphelper;

    //=====================================================================
    //= OAccessRegulator
    //=====================================================================
    struct OAccessRegulator
    {
        friend class OControlWizardPage;

    protected:
        OAccessRegulator() { }
    };

    //=====================================================================
    //= OControlWizardPage
    //=====================================================================
    //---------------------------------------------------------------------
    OControlWizardPage::OControlWizardPage( OControlWizard* _pParent, const ResId& _rResId )
        :OWizardPage( _pParent, _rResId )
    {
    }

    //---------------------------------------------------------------------
    OControlWizard* OControlWizardPage::getDialog()
    {
        return static_cast< OControlWizard* >(GetParent());
    }

    //---------------------------------------------------------------------
    const OControlWizard* OControlWizardPage::getDialog() const
    {
        return static_cast< OControlWizard* >(GetParent());
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizardPage::updateContext()
    {
        return getDialog()->updateContext(OAccessRegulator());
    }

    //---------------------------------------------------------------------
    Reference< XConnection > OControlWizardPage::getFormConnection() const
    {
        return getDialog()->getFormConnection(OAccessRegulator());
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::setFormConnection(const Reference< XConnection >& _rxConn)
    {
        getDialog()->setFormConnection(OAccessRegulator(), _rxConn);
    }

    //---------------------------------------------------------------------
    const OControlWizardContext& OControlWizardPage::getContext()
    {
        return getDialog()->getContext();
    }

    //---------------------------------------------------------------------
    Reference< XMultiServiceFactory > OControlWizardPage::getServiceFactory()
    {
        return getDialog()->getServiceFactory();
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::fillListBox(ListBox& _rList, const Sequence< ::rtl::OUString >& _rItems, sal_Bool _bClear)
    {
        if (_bClear)
            _rList.Clear();
        const ::rtl::OUString* pItems = _rItems.getConstArray();
        const ::rtl::OUString* pEnd = pItems + _rItems.getLength();
        sal_uInt16 nPos;
        sal_Int32 nIndex = 0;
        for (;pItems < pEnd; ++pItems, ++nIndex)
        {
            nPos = _rList.InsertEntry(*pItems);
            _rList.SetEntryData(nPos, reinterpret_cast<void*>(nIndex));
        }
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::fillListBox(ComboBox& _rList, const Sequence< ::rtl::OUString >& _rItems, sal_Bool _bClear)
    {
        if (_bClear)
            _rList.Clear();
        const ::rtl::OUString* pItems = _rItems.getConstArray();
        const ::rtl::OUString* pEnd = pItems + _rItems.getLength();
        sal_uInt16 nPos;
        sal_Int32 nIndex = 0;
        for (;pItems < pEnd; ++pItems)
        {
            nPos = _rList.InsertEntry(*pItems);
            _rList.SetEntryData(nPos, reinterpret_cast<void*>(nIndex));
        }
    }

    //=====================================================================
    //= OControlWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OControlWizard::OControlWizard( Window* _pParent, const ResId& _rId,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XMultiServiceFactory >& _rxORB )
        :OWizardMachine(_pParent, _rId, WZB_CANCEL | WZB_PREVIOUS | WZB_NEXT | WZB_FINISH)
        ,m_xORB(_rxORB)
    {
        m_aContext.xObjectModel = _rxObjectModel;
        initContext();

        SetPageSizePixel(LogicToPixel(::Size(WINDOW_SIZE_X, WINDOW_SIZE_Y), MAP_APPFONT));
        ShowButtonFixedLine(sal_True);
        defaultButton(WZB_NEXT);
        enableButtons(WZB_FINISH, sal_False);
    }

    //---------------------------------------------------------------------
    OControlWizard::~OControlWizard()
    {
    }

    //---------------------------------------------------------------------
    short OControlWizard::Execute()
    {
        // get the class id of the control we're dealing with
        sal_Int16 nClassId = FormComponentType::CONTROL;
        try
        {
            getContext().xObjectModel->getPropertyValue(::rtl::OUString::createFromAscii("ClassId")) >>= nClassId;
        }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::activate: could not obtain the class id!");
        }
        if (!approveControl(nClassId))
        {
            // TODO: MessageBox or exception
            return RET_CANCEL;
        }

        ActivatePage();

        return OControlWizard_Base::Execute();
    }

    //---------------------------------------------------------------------
    void OControlWizard::ActivatePage()
    {
        OControlWizard_Base::ActivatePage();
    }

    //---------------------------------------------------------------------
    void OControlWizard::implDetermineShape()
    {
        Reference< XIndexAccess > xPageObjects(m_aContext.xDrawPage, UNO_QUERY);
        DBG_ASSERT(xPageObjects.is(), "OControlWizard::implDetermineShape: invalid page!");

        // for comparing the model
        Reference< XControlModel > xModelCompare(m_aContext.xObjectModel, UNO_QUERY);

        if (xPageObjects.is())
        {
            // loop through all objects of the page
            sal_Int32 nObjects = xPageObjects->getCount();
            Reference< XControlShape > xControlShape;
            Reference< XControlModel > xControlModel;
            for (sal_Int32 i=0; i<nObjects; ++i)
            {
                if (xPageObjects->getByIndex(i) >>= xControlShape)
                {   // it _is_ a control shape
                    xControlModel = xControlShape->getControl();
                    DBG_ASSERT(xControlModel.is(), "OControlWizard::implDetermineShape: control shape without model!");
                    if (xModelCompare.get() == xControlModel.get())
                    {
                        m_aContext.xObjectShape = xControlShape;
                        break;
                    }
                }
            }
        }
    }

    //---------------------------------------------------------------------
    void OControlWizard::implDetermineForm()
    {
        Reference< XChild > xModelAsChild(m_aContext.xObjectModel, UNO_QUERY);
        Reference< XInterface > xControlParent;
        if (xModelAsChild.is())
            xControlParent = xModelAsChild->getParent();

        m_aContext.xForm = Reference< XPropertySet >(xControlParent, UNO_QUERY);
        m_aContext.xRowSet = Reference< XRowSet >(xControlParent, UNO_QUERY);
        DBG_ASSERT(m_aContext.xForm.is() && m_aContext.xRowSet.is(),
            "OControlWizard::implDetermineForm: missing some interfaces of the control parent!");

    }

    //---------------------------------------------------------------------
    void OControlWizard::implDeterminePage()
    {
        try
        {
            // get the document model
            Reference< XChild > xControlAsChild(m_aContext.xObjectModel, UNO_QUERY);
            Reference< XChild > xModelSearch(xControlAsChild->getParent(), UNO_QUERY);

            Reference< XModel > xModel(xModelSearch, UNO_QUERY);
            while (xModelSearch.is() && !xModel.is())
            {
                xModelSearch = Reference< XChild >(xModelSearch->getParent(), UNO_QUERY);
                xModel = Reference< XModel >(xModelSearch, UNO_QUERY);
            }

            Reference< XDrawPage > xPage;
            if (xModel.is())
            {
                m_aContext.xDocumentModel = xModel;

                Reference< XDrawPageSupplier > xPageSupp(xModel, UNO_QUERY);
                if (xPageSupp.is())
                {   // it's a document with only one page -> Writer
                    xPage = xPageSupp->getDrawPage();
                }
                else
                {
                    // get the controller currently working on this model
                    Reference< XController > xController = xModel->getCurrentController();
                    DBG_ASSERT(xController.is(), "OControlWizard::implDeterminePage: no current controller!");

                    // maybe it's a spredsheet
                    Reference< XSpreadsheetView > xView(xController, UNO_QUERY);
                    if (xView.is())
                    {   // okay, it is one
                        Reference< XSpreadsheet > xSheet = xView->getActiveSheet();
                        xPageSupp = Reference< XDrawPageSupplier >(xSheet, UNO_QUERY);
                        DBG_ASSERT(xPageSupp.is(), "OControlWizard::implDeterminePage: a spreadsheet which is no page supplier!");
                        if (xPageSupp.is())
                            xPage = xPageSupp->getDrawPage();
                    }
                    else
                    {   // can be a draw/impress doc only
                        Reference< XDrawView > xView(xController, UNO_QUERY);
                        DBG_ASSERT(xView.is(), "OControlWizard::implDeterminePage: no alternatives left ... can't determine the page!");
                        if (xView.is())
                            xPage = xView->getCurrentPage();
                    }
                }
            }
            else
                DBG_ASSERT(xPage.is(), "OControlWizard::implDeterminePage: can't determine the page (no model)!");

            m_aContext.xDrawPage = xPage;
        }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::implDeterminePage: caught an exception!");
        }
    }

    //---------------------------------------------------------------------
    void OControlWizard::implGetDSContext()
    {
        Reference< XMultiServiceFactory > xORB = getServiceFactory();
        try
        {
            DBG_ASSERT(xORB.is(), "OControlWizard::implGetDSContext: invalid service factory!");

            Reference< XInterface > xContext;
            if (xORB.is())
                xContext = xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseContext"));
            DBG_ASSERT(xContext.is(), "OControlWizard::implGetDSContext: invalid database context!");

            m_aContext.xDatasourceContext = Reference< XNameAccess >(xContext, UNO_QUERY);
            DBG_ASSERT(m_aContext.xDatasourceContext.is() || !xContext.is(), "OControlWizard::implGetDSContext: invalid database context (missing the XNameAccess)!");
        }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::implGetDSContext: invalid database context!");
        }
    }

    //---------------------------------------------------------------------
    Reference< XConnection > OControlWizard::getFormConnection(const OAccessRegulator&) const
    {
        Reference< XConnection > xConn;
        try
        {
            m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) >>= xConn;
        }
        catch(const Exception&)
        {
            DBG_ERROR("OControlWizard::getFormConnection: caught an exception!");
        }
        return xConn;
    }

    //---------------------------------------------------------------------
    void OControlWizard::setFormConnection(const OAccessRegulator& _rAccess, const Reference< XConnection >& _rxConn)
    {
        try
        {
            Reference< XConnection > xOldConn = getFormConnection(_rAccess);
            if (xOldConn.get() == _rxConn.get())
                return;

            disposeComponent(xOldConn);

            m_aContext.xForm->setPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection"), makeAny(_rxConn));
        }
        catch(const Exception&)
        {
            DBG_ERROR("OControlWizard::setFormConnection: caught an exception!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizard::updateContext(const OAccessRegulator&)
    {
        return initContext();
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizard::initContext()
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OGroupBoxWizard::initContext: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return sal_False;

        // reset the context
        m_aContext.xForm.clear();
        m_aContext.xRowSet.clear();
        m_aContext.xDocumentModel.clear();
        m_aContext.xDrawPage.clear();
        m_aContext.xObjectShape.clear();
        m_aContext.aFieldNames.realloc(0);

        m_aContext.xObjectContainer.clear();
        m_aContext.xFields.clear();

        Any aSQLException;
        Reference< XPreparedStatement >  xStatement;
        try
        {
            // get the datasource context
            implGetDSContext();

            // first, determine the form the control belongs to
            implDetermineForm();

            // need the page, too
            implDeterminePage();

            // the shape of the control
            implDetermineShape();

            // get the columns of the object the settins refer to
            Reference< XNameAccess >  xColumns;

            if (m_aContext.xForm.is())
            {
                // collect some properties of the form
                ::rtl::OUString sDataSourceName = ::comphelper::getString(m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("DataSourceName")));
                ::rtl::OUString sObjectName = ::comphelper::getString(m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("Command")));
                sal_Int32 nObjectType = ::comphelper::getINT32(m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("CommandType")));

                // calculate the connection the rowset is working with
                Reference< XConnection >   xConnection = ::dbtools::calcConnection(m_aContext.xRowSet, getServiceFactory());

                // get the fields
                if (xConnection.is())
                {
                    switch (nObjectType)
                    {
                        case 0:
                        {
                            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
                            if (xSupplyTables.is() && xSupplyTables->getTables().is() && xSupplyTables->getTables()->hasByName(sObjectName))
                            {
                                Reference< XColumnsSupplier >  xSupplyColumns;
                                m_aContext.xObjectContainer = xSupplyTables->getTables();
                                m_aContext.xObjectContainer->getByName(sObjectName) >>= xSupplyColumns;
                                DBG_ASSERT(xSupplyColumns.is(), "OControlWizard::initContext: invalid table columns!");
                                xColumns = xSupplyColumns->getColumns();
                            }
                        }
                        break;
                        case 1:
                        {
                            Reference< XQueriesSupplier >  xSupplyQueries(xConnection, UNO_QUERY);
                            if (xSupplyQueries.is() && xSupplyQueries->getQueries().is() && xSupplyQueries->getQueries()->hasByName(sObjectName))
                            {
                                Reference< XColumnsSupplier >  xSupplyColumns;
                                m_aContext.xObjectContainer = xSupplyQueries->getQueries();
                                m_aContext.xObjectContainer->getByName(sObjectName) >>= xSupplyColumns;
                                DBG_ASSERT(xSupplyColumns.is(), "OControlWizard::initContext: invalid query columns!");
                                xColumns  = xSupplyColumns->getColumns();
                            }
                        }
                        break;
                        default:
                        {
                            xStatement = xConnection->prepareStatement(sObjectName);

                            // not interested in any results, only in the fields
                            Reference< XPropertySet > xStatementProps(xStatement, UNO_QUERY);
                            xStatementProps->setPropertyValue(::rtl::OUString::createFromAscii("MaxRows"), makeAny(sal_Int32(0)));

                            // TODO: think about handling local SQLExceptions here ...
                            Reference< XColumnsSupplier >  xSupplyCols(xStatement->executeQuery(), UNO_QUERY);
                            if (xSupplyCols.is())
                                xColumns = xSupplyCols->getColumns();
                        }
                    }
                }
            }

            if (xColumns.is())
            {
                m_aContext.aFieldNames = xColumns->getElementNames();
                m_aContext.xFields = xColumns;
            }
        }
        catch(SQLContext& e) { aSQLException <<= e; }
        catch(SQLWarning& e) { aSQLException <<= e; }
        catch(SQLException& e) { aSQLException <<= e; }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::initContext: could not retrieve the control context (caught an exception)!");
        }

        ::comphelper::disposeComponent(xStatement);

        if (aSQLException.hasValue())
        {   // an SQLException (or derivee) was thrown ...

            // prepend an extra SQLContext explaining what we were doing
            SQLContext aContext;
            aContext.Message = String(ModuleRes(RID_STR_COULDNOTOPENTABLE));
            aContext.NextException = aSQLException;

            // create an interaction handler to display this exception
            const ::rtl::OUString sInteractionHandlerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.InteractionHandler");
            Reference< XInteractionHandler > xHandler;
            try
            {
                if (getServiceFactory().is())
                    xHandler = Reference< XInteractionHandler >(getServiceFactory()->createInstance(sInteractionHandlerServiceName), UNO_QUERY);
            }
            catch(Exception&) { }
            if (!xHandler.is())
            {
                ShowServiceNotAvailableError(this, sInteractionHandlerServiceName, sal_True);
                return sal_False;
            }

            Reference< XInteractionRequest > xRequest = new OInteractionRequest(makeAny(aContext));
            try
            {
                xHandler->handle(xRequest);
            }
            catch(Exception&) { }
            return sal_False;
        }

        return 0 != m_aContext.aFieldNames.getLength();
    }

    //---------------------------------------------------------------------
    void OControlWizard::commitControlSettings(OControlWizardSettings* _pSettings)
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OControlWizard::commitControlSettings: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return;

        // the only thing we have at the moment is the label
        try
        {
            ::rtl::OUString sLabelPropertyName = ::rtl::OUString::createFromAscii("Label");
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName(sLabelPropertyName))
            {
                ::rtl::OUString sControlLabel(_pSettings->sControlLabel);
                m_aContext.xObjectModel->setPropertyValue(
                    ::rtl::OUString::createFromAscii("Label"),
                    makeAny(sControlLabel)
                );
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::commitControlSettings: could not commit the basic control settings!");
        }
    }

    //---------------------------------------------------------------------
    void OControlWizard::initControlSettings(OControlWizardSettings* _pSettings)
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OControlWizard::initControlSettings: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return;

        // initialize some settings from the control model give
        try
        {
            ::rtl::OUString sLabelPropertyName = ::rtl::OUString::createFromAscii("Label");
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName(sLabelPropertyName))
            {
                ::rtl::OUString sControlLabel;
                m_aContext.xObjectModel->getPropertyValue(sLabelPropertyName) >>= sControlLabel;
                _pSettings->sControlLabel = sControlLabel;
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OControlWizard::initControlSettings: could not retrieve the basic control settings!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizard::needDatasourceSelection()
    {
        // lemme see ...
        return (0 == getContext().aFieldNames.getLength());
            // if we got fields, the data source is valid ...
//      try
//      {
//          // first, we need a valid data source name
//          ::rtl::OUString sDataSourceName;
//          m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("DataSourceName")) >>= sDataSourceName;
//          if (m_aContext.xDatasourceContext.is() && m_aContext.xDatasourceContext->hasByName(sDataSourceName))
//          {   // at least the data source name is valid ...
//              // then, a CommandType "table" would be nice ...
//              sal_Int32 nCommandType = CommandType::COMMAND;
//              m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("CommandType")) >>= nCommandType;
//              if (CommandType::TABLE == nCommandType)
//              {   // okay ....
//                  // now the table itself should be valid
//                  ::rtl::OUString sTableName;
//                  m_aContext.xForm->getPropertyValue(::rtl::OUString::createFromAscii("Command")) >>= sTableName;
//                  if (m_aContext.xObjectContainer.is() && m_aContext.xObjectContainer->hasByName(sTableName))
//                      return sal_False;
//              }
//          }
//      }
//      catch(Exception&)
//      {
//          DBG_ERROR("OControlWizard::needDatasourceSelection: caught an exception while checking the form settings!");
//      }
//      return sal_True;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2001/03/05 14:53:13  fs
 *  finished the grid control wizard
 *
 *  Revision 1.3  2001/02/28 09:18:30  fs
 *  finalized the list/combo wizard
 *
 *  Revision 1.2  2001/02/23 15:19:08  fs
 *  some changes / centralizations - added the list-/combobox wizard
 *
 *  Revision 1.1  2001/02/21 09:22:07  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:02:44  fs
 ************************************************************************/

