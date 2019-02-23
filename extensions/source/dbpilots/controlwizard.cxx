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

#include "controlwizard.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/interaction.hxx>
#include <vcl/stdtext.hxx>
#include <connectivity/conncleanup.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <osl/diagnose.h>

#define WINDOW_SIZE_X   240
#define WINDOW_SIZE_Y   185

namespace dbp
{
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
    using namespace ::dbtools;

    struct OAccessRegulator
    {
        friend class OControlWizardPage;

    protected:
        OAccessRegulator() { }
    };

    OControlWizardPage::OControlWizardPage( OControlWizard* _pParent, const OString& rID, const OUString& rUIXMLDescription )
        :OControlWizardPage_Base( _pParent, rID, rUIXMLDescription )
        ,m_pFormDatasourceLabel(nullptr)
        ,m_pFormDatasource(nullptr)
        ,m_pFormContentTypeLabel(nullptr)
        ,m_pFormContentType(nullptr)
        ,m_pFormTableLabel(nullptr)
        ,m_pFormTable(nullptr)
    {
    }

    OControlWizardPage::~OControlWizardPage()
    {
        disposeOnce();
    }

    void OControlWizardPage::dispose()
    {
        m_pFormDatasourceLabel.clear();
        m_pFormDatasource.clear();
        m_pFormContentTypeLabel.clear();
        m_pFormContentType.clear();
        m_pFormTableLabel.clear();
        m_pFormTable.clear();
        OControlWizardPage_Base::dispose();
    }

    OControlWizard* OControlWizardPage::getDialog()
    {
        return static_cast< OControlWizard* >(GetParent());
    }

    const OControlWizard* OControlWizardPage::getDialog() const
    {
        return static_cast< OControlWizard* >(GetParent());
    }


    bool OControlWizardPage::updateContext()
    {
        return getDialog()->updateContext(OAccessRegulator());
    }


    Reference< XConnection > OControlWizardPage::getFormConnection() const
    {
        return getDialog()->getFormConnection(OAccessRegulator());
    }


    void OControlWizardPage::setFormConnection( const Reference< XConnection >& _rxConn, bool _bAutoDispose )
    {
        getDialog()->setFormConnection( OAccessRegulator(), _rxConn, _bAutoDispose );
    }


    const OControlWizardContext& OControlWizardPage::getContext()
    {
        return getDialog()->getContext();
    }


    void OControlWizardPage::fillListBox(ListBox& _rList, const Sequence< OUString >& _rItems)
    {
        _rList.Clear();
        const OUString* pItems = _rItems.getConstArray();
        const OUString* pEnd = pItems + _rItems.getLength();
        ::svt::WizardTypes::WizardState nPos;
        sal_Int32 nIndex = 0;
        for (;pItems < pEnd; ++pItems, ++nIndex)
        {
            nPos = _rList.InsertEntry(*pItems);
            _rList.SetEntryData(nPos, reinterpret_cast<void*>(nIndex));
        }
    }


    void OControlWizardPage::fillListBox(ComboBox& _rList, const Sequence< OUString >& _rItems)
    {
        _rList.Clear();
        const OUString* pItems = _rItems.getConstArray();
        const OUString* pEnd = pItems + _rItems.getLength();
        ::svt::WizardTypes::WizardState nPos;
        for (;pItems < pEnd; ++pItems)
        {
            nPos = _rList.InsertEntry(*pItems);
            _rList.SetEntryData(nPos, nullptr);
        }
    }


    void OControlWizardPage::enableFormDatasourceDisplay()
    {
        if (m_pFormContentType)
            // nothing to do
            return;

        VclFrame *_pFrame = get<VclFrame>("sourceframe");
        _pFrame->Show();
        get(m_pFormContentType,"contenttype");
        get(m_pFormContentTypeLabel,"contenttypelabel");
        get(m_pFormDatasource, "datasource");
        get(m_pFormDatasourceLabel, "datasourcelabel");
        get(m_pFormTable,"formtable");
        get(m_pFormTableLabel,"formtablelabel");

        const OControlWizardContext& rContext = getContext();
        if ( rContext.bEmbedded )
        {
            m_pFormDatasourceLabel->Hide();
            m_pFormDatasource->Hide();
            m_pFormContentTypeLabel->SetPosPixel(m_pFormDatasourceLabel->GetPosPixel());
            m_pFormContentType->SetPosPixel(m_pFormDatasource->GetPosPixel());
            m_pFormTableLabel->SetPosPixel(::Point(m_pFormDatasourceLabel->GetPosPixel().X(),m_pFormTableLabel->GetPosPixel().Y()));
            m_pFormTable->SetPosPixel(::Point(m_pFormDatasource->GetPosPixel().X(),m_pFormTable->GetPosPixel().Y()));
        }
    }

    void OControlWizardPage::initializePage()
    {
        if (m_pFormDatasource && m_pFormContentTypeLabel && m_pFormTable)
        {
            const OControlWizardContext& rContext = getContext();
            OUString sDataSource;
            OUString sCommand;
            sal_Int32 nCommandType = CommandType::COMMAND;
            try
            {
                rContext.xForm->getPropertyValue("DataSourceName") >>= sDataSource;
                rContext.xForm->getPropertyValue("Command") >>= sCommand;
                rContext.xForm->getPropertyValue("CommandType") >>= nCommandType;
            }
            catch(const Exception&)
            {
                OSL_FAIL("OControlWizardPage::initializePage: caught an exception!");
            }

            INetURLObject aURL( sDataSource );
            if( aURL.GetProtocol() != INetProtocol::NotValid )
                sDataSource = aURL.GetName(INetURLObject::DecodeMechanism::WithCharset);
            m_pFormDatasource->SetText(sDataSource);
            m_pFormTable->SetText(sCommand);

            const char* pCommandTypeResourceId = nullptr;
            switch (nCommandType)
            {
                case CommandType::TABLE:
                    pCommandTypeResourceId = RID_STR_TYPE_TABLE;
                    break;

                case CommandType::QUERY:
                    pCommandTypeResourceId = RID_STR_TYPE_QUERY;
                    break;

                default:
                    pCommandTypeResourceId = RID_STR_TYPE_COMMAND;
                    break;
            }
            m_pFormContentType->SetText(compmodule::ModuleRes(pCommandTypeResourceId));
        }

        OControlWizardPage_Base::initializePage();
    }

    OControlWizard::OControlWizard( vcl::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        :OWizardMachine(_pParent, WizardButtonFlags::CANCEL | WizardButtonFlags::PREVIOUS | WizardButtonFlags::NEXT | WizardButtonFlags::FINISH)
        ,m_xContext(_rxContext)
    {
        m_aContext.xObjectModel = _rxObjectModel;
        initContext();

        SetPageSizePixel(LogicToPixel(::Size(WINDOW_SIZE_X, WINDOW_SIZE_Y), MapMode(MapUnit::MapAppFont)));
        defaultButton(WizardButtonFlags::NEXT);
        enableButtons(WizardButtonFlags::FINISH, false);
    }

    OControlWizard::~OControlWizard()
    {
    }

    short OControlWizard::Execute()
    {
        // get the class id of the control we're dealing with
        sal_Int16 nClassId = FormComponentType::CONTROL;
        try
        {
            getContext().xObjectModel->getPropertyValue("ClassId") >>= nClassId;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::activate: could not obtain the class id!");
        }
        if (!approveControl(nClassId))
        {
            // TODO: MessageBox or exception
            return RET_CANCEL;
        }

        ActivatePage();

        return OControlWizard_Base::Execute();
    }


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


    void OControlWizard::implDetermineForm()
    {
        Reference< XChild > xModelAsChild(m_aContext.xObjectModel, UNO_QUERY);
        Reference< XInterface > xControlParent;
        if (xModelAsChild.is())
            xControlParent = xModelAsChild->getParent();

        m_aContext.xForm.set(xControlParent, UNO_QUERY);
        m_aContext.xRowSet.set(xControlParent, UNO_QUERY);
        DBG_ASSERT(m_aContext.xForm.is() && m_aContext.xRowSet.is(),
            "OControlWizard::implDetermineForm: missing some interfaces of the control parent!");

    }


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
                xModelSearch.set(xModelSearch->getParent(), UNO_QUERY);
                xModel.set(xModelSearch, UNO_QUERY);
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

                    // maybe it's a spreadsheet
                    Reference< XSpreadsheetView > xView(xController, UNO_QUERY);
                    if (xView.is())
                    {   // okay, it is one
                        Reference< XSpreadsheet > xSheet = xView->getActiveSheet();
                        xPageSupp.set(xSheet, UNO_QUERY);
                        DBG_ASSERT(xPageSupp.is(), "OControlWizard::implDeterminePage: a spreadsheet which is no page supplier!");
                        if (xPageSupp.is())
                            xPage = xPageSupp->getDrawPage();
                    }
                    else
                    {   // can be a draw/impress doc only
                        Reference< XDrawView > xDrawView(xController, UNO_QUERY);
                        DBG_ASSERT(xDrawView.is(), "OControlWizard::implDeterminePage: no alternatives left ... can't determine the page!");
                        if (xDrawView.is())
                            xPage = xDrawView->getCurrentPage();
                    }
                }
            }
            else
            {
                DBG_ASSERT(xPage.is(), "OControlWizard::implDeterminePage: can't determine the page (no model)!");
            }
            m_aContext.xDrawPage = xPage;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::implDeterminePage: caught an exception!");
        }
    }


    void OControlWizard::implGetDSContext()
    {
        try
        {
            DBG_ASSERT(m_xContext.is(), "OControlWizard::implGetDSContext: invalid service factory!");

            m_aContext.xDatasourceContext = DatabaseContext::create(m_xContext);
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::implGetDSContext: invalid database context!");
        }
    }


    Reference< XConnection > OControlWizard::getFormConnection(const OAccessRegulator&) const
    {
        return getFormConnection();
    }

    Reference< XConnection > OControlWizard::getFormConnection() const
    {
        Reference< XConnection > xConn;
        try
        {
            if ( !::dbtools::isEmbeddedInDatabase(m_aContext.xForm,xConn) )
                m_aContext.xForm->getPropertyValue("ActiveConnection") >>= xConn;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::getFormConnection: caught an exception!");
        }
        return xConn;
    }


    void OControlWizard::setFormConnection( const OAccessRegulator& _rAccess, const Reference< XConnection >& _rxConn, bool _bAutoDispose )
    {
        try
        {
            Reference< XConnection > xOldConn = getFormConnection(_rAccess);
            if (xOldConn.get() == _rxConn.get())
                return;

            disposeComponent(xOldConn);

            // set the new connection
            if ( _bAutoDispose )
            {
                // for this, use a AutoDisposer (so the conn is cleaned up when the form dies or gets another connection)
                Reference< XRowSet > xFormRowSet( m_aContext.xForm, UNO_QUERY );
                rtl::Reference<OAutoConnectionDisposer> pAutoDispose = new OAutoConnectionDisposer( xFormRowSet, _rxConn );
            }
            else
            {
                m_aContext.xForm->setPropertyValue("ActiveConnection", makeAny( _rxConn ) );
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::setFormConnection: caught an exception!");
        }
    }


    bool OControlWizard::updateContext(const OAccessRegulator&)
    {
        return initContext();
    }

    Reference< XInteractionHandler > OControlWizard::getInteractionHandler(weld::Window* _pWindow) const
    {
        Reference< XInteractionHandler > xHandler;
        try
        {
            xHandler.set( InteractionHandler::createWithParent(m_xContext, nullptr), UNO_QUERY_THROW );
        }
        catch(const Exception&) { }
        if (!xHandler.is())
        {
            const OUString sInteractionHandlerServiceName("com.sun.star.task.InteractionHandler");
            ShowServiceNotAvailableError(_pWindow, sInteractionHandlerServiceName, true);
        }
        return xHandler;
    }

    bool OControlWizard::initContext()
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OGroupBoxWizard::initContext: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return false;

        // reset the context
        m_aContext.xForm.clear();
        m_aContext.xRowSet.clear();
        m_aContext.xDocumentModel.clear();
        m_aContext.xDrawPage.clear();
        m_aContext.xObjectShape.clear();
        m_aContext.aFieldNames.realloc(0);

        m_aContext.xObjectContainer.clear();
        m_aContext.aTypes.clear();
        m_aContext.bEmbedded = false;

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

            // get the columns of the object the settings refer to
            Reference< XNameAccess >  xColumns;

            if (m_aContext.xForm.is())
            {
                // collect some properties of the form
                OUString sObjectName = ::comphelper::getString(m_aContext.xForm->getPropertyValue("Command"));
                sal_Int32 nObjectType = ::comphelper::getINT32(m_aContext.xForm->getPropertyValue("CommandType"));

                // calculate the connection the rowset is working with
                Reference< XConnection > xConnection;
                m_aContext.bEmbedded = ::dbtools::isEmbeddedInDatabase( m_aContext.xForm, xConnection );
                if ( !m_aContext.bEmbedded )
                    xConnection = ::dbtools::connectRowset( m_aContext.xRowSet, m_xContext );

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
                            xStatementProps->setPropertyValue("MaxRows", makeAny(sal_Int32(0)));

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
                const OUString* pBegin = m_aContext.aFieldNames.getConstArray();
                const OUString* pEnd   = pBegin + m_aContext.aFieldNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                {
                    sal_Int32 nFieldType = DataType::OTHER;
                    try
                    {
                        Reference< XPropertySet > xColumn;
                        xColumns->getByName(*pBegin) >>= xColumn;
                        xColumn->getPropertyValue("Type") >>= nFieldType;
                    }
                    catch(const Exception&)
                    {
                        OSL_FAIL("OControlWizard::initContext: unexpected exception while gathering column information!");
                    }
                    m_aContext.aTypes.emplace(*pBegin,nFieldType);
                }
            }
        }
        catch(const SQLContext& e) { aSQLException <<= e; }
        catch(const SQLWarning& e) { aSQLException <<= e; }
        catch(const SQLException& e) { aSQLException <<= e; }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::initContext: could not retrieve the control context (caught an exception)!");
        }

        ::comphelper::disposeComponent(xStatement);

        if (aSQLException.hasValue())
        {   // an SQLException (or derivee) was thrown ...

            // prepend an extra SQLContext explaining what we were doing
            SQLContext aContext;
            aContext.Message = compmodule::ModuleRes(RID_STR_COULDNOTOPENTABLE);
            aContext.NextException = aSQLException;

            // create an interaction handler to display this exception
            Reference< XInteractionHandler > xHandler = getInteractionHandler(GetFrameWeld());
            if ( !xHandler.is() )
                return false;

            Reference< XInteractionRequest > xRequest = new OInteractionRequest(makeAny(aContext));
            try
            {
                xHandler->handle(xRequest);
            }
            catch(const Exception&) { }
            return false;
        }

        return 0 != m_aContext.aFieldNames.getLength();
    }


    void OControlWizard::commitControlSettings(OControlWizardSettings const * _pSettings)
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OControlWizard::commitControlSettings: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return;

        // the only thing we have at the moment is the label
        try
        {
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName("Label"))
            {
                OUString sControlLabel(_pSettings->sControlLabel);
                m_aContext.xObjectModel->setPropertyValue(
                    "Label",
                    makeAny(sControlLabel)
                );
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::commitControlSettings: could not commit the basic control settings!");
        }
    }


    void OControlWizard::initControlSettings(OControlWizardSettings* _pSettings)
    {
        DBG_ASSERT(m_aContext.xObjectModel.is(), "OControlWizard::initControlSettings: have no control model to work with!");
        if (!m_aContext.xObjectModel.is())
            return;

        // initialize some settings from the control model give
        try
        {
            OUString sLabelPropertyName("Label");
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName(sLabelPropertyName))
            {
                OUString sControlLabel;
                m_aContext.xObjectModel->getPropertyValue(sLabelPropertyName) >>= sControlLabel;
                _pSettings->sControlLabel = sControlLabel;
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::initControlSettings: could not retrieve the basic control settings!");
        }
    }


    bool OControlWizard::needDatasourceSelection()
    {
        // lemme see ...
        return (0 == getContext().aFieldNames.getLength());
            // if we got fields, the data source is valid ...
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
