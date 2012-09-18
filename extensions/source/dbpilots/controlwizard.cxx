/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <comphelper/componentcontext.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/interaction.hxx>
#include <vcl/stdtext.hxx>
#include <svtools/localresaccess.hxx>
#include <connectivity/conncleanup.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <tools/urlobj.hxx>

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
    using namespace ::dbtools;

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
        :OControlWizardPage_Base( _pParent, _rResId )
        ,m_pFormSettingsSeparator(NULL)
        ,m_pFormDatasourceLabel(NULL)
        ,m_pFormDatasource(NULL)
        ,m_pFormContentTypeLabel(NULL)
        ,m_pFormContentType(NULL)
        ,m_pFormTableLabel(NULL)
        ,m_pFormTable(NULL)
    {
    }

    //---------------------------------------------------------------------
    OControlWizardPage::~OControlWizardPage()
    {
        delete m_pFormSettingsSeparator;
        delete m_pFormDatasourceLabel;
        delete m_pFormDatasource;
        delete m_pFormContentTypeLabel;
        delete m_pFormContentType;
        delete m_pFormTableLabel;
        delete m_pFormTable;
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
    void OControlWizardPage::setFormConnection( const Reference< XConnection >& _rxConn, sal_Bool _bAutoDispose )
    {
        getDialog()->setFormConnection( OAccessRegulator(), _rxConn, _bAutoDispose );
    }

    //---------------------------------------------------------------------
    const OControlWizardContext& OControlWizardPage::getContext()
    {
        return getDialog()->getContext();
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::fillListBox(ListBox& _rList, const Sequence< ::rtl::OUString >& _rItems, sal_Bool _bClear)
    {
        if (_bClear)
            _rList.Clear();
        const ::rtl::OUString* pItems = _rItems.getConstArray();
        const ::rtl::OUString* pEnd = pItems + _rItems.getLength();
        ::svt::WizardTypes::WizardState nPos;
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
        ::svt::WizardTypes::WizardState nPos;
        sal_Int32 nIndex = 0;
        for (;pItems < pEnd; ++pItems)
        {
            nPos = _rList.InsertEntry(*pItems);
            _rList.SetEntryData(nPos, reinterpret_cast<void*>(nIndex));
        }
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::enableFormDatasourceDisplay()
    {
        if (m_pFormSettingsSeparator)
            // nothing to do
            return;

        ModuleRes aModuleRes(RID_PAGE_FORM_DATASOURCE_STATUS);
        OLocalResourceAccess aLocalControls(aModuleRes, RSC_TABPAGE);

        m_pFormSettingsSeparator    = new FixedLine(this,  ModuleRes(FL_FORMSETINGS));
        m_pFormDatasourceLabel      = new FixedText(this,  ModuleRes(FT_FORMDATASOURCELABEL));
        m_pFormDatasource           = new FixedText(this,  ModuleRes(FT_FORMDATASOURCE));
        m_pFormContentTypeLabel     = new FixedText(this,  ModuleRes(FT_FORMCONTENTTYPELABEL));
        m_pFormContentType          = new FixedText(this,  ModuleRes(FT_FORMCONTENTTYPE));
        m_pFormTableLabel           = new FixedText(this,  ModuleRes(FT_FORMTABLELABEL));
        m_pFormTable                = new FixedText(this,  ModuleRes(FT_FORMTABLE));

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

    //---------------------------------------------------------------------
    void OControlWizardPage::adjustControlForNoDSDisplay(Control* _pControl, sal_Bool _bConstLowerDistance)
    {
        ::Size aDistanceToMove = LogicToPixel( ::Size( 0, 37 ), MAP_APPFONT );

        ::Point aPos = _pControl->GetPosPixel();
        aPos.Y() -= aDistanceToMove.Height();
        _pControl->SetPosPixel(aPos);

        if (_bConstLowerDistance)
        {
            ::Size aSize = _pControl->GetSizePixel();
            aSize.Height() += aDistanceToMove.Height();
            _pControl->SetSizePixel(aSize);
        }
    }

    //---------------------------------------------------------------------
    void OControlWizardPage::initializePage()
    {
        if (m_pFormDatasource && m_pFormContentTypeLabel && m_pFormTable)
        {
            const OControlWizardContext& rContext = getContext();
            ::rtl::OUString sDataSource;
            ::rtl::OUString sCommand;
            sal_Int32 nCommandType = CommandType::COMMAND;
            try
            {
                rContext.xForm->getPropertyValue(::rtl::OUString("DataSourceName")) >>= sDataSource;
                rContext.xForm->getPropertyValue(::rtl::OUString("Command")) >>= sCommand;
                rContext.xForm->getPropertyValue(::rtl::OUString("CommandType")) >>= nCommandType;
            }
            catch(const Exception&)
            {
                OSL_FAIL("OControlWizardPage::initializePage: caught an exception!");
            }

            INetURLObject aURL( sDataSource );
            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                sDataSource = aURL.GetName(INetURLObject::DECODE_WITH_CHARSET);
            m_pFormDatasource->SetText(sDataSource);
            m_pFormTable->SetText(sCommand);

            ::svt::WizardTypes::WizardState nCommandTypeResourceId = 0;
            switch (nCommandType)
            {
                case CommandType::TABLE:
                    nCommandTypeResourceId = RID_STR_TYPE_TABLE;
                    break;

                case CommandType::QUERY:
                    nCommandTypeResourceId = RID_STR_TYPE_QUERY;
                    break;

                default:
                    nCommandTypeResourceId = RID_STR_TYPE_COMMAND;
                    break;
            }
            m_pFormContentType->SetText(String(ModuleRes(nCommandTypeResourceId)));
        }

        OControlWizardPage_Base::initializePage();
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
            getContext().xObjectModel->getPropertyValue(::rtl::OUString("ClassId")) >>= nClassId;
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

    //---------------------------------------------------------------------
    void OControlWizard::implGetDSContext()
    {
        Reference< XMultiServiceFactory > xORB = getServiceFactory();
        try
        {
            DBG_ASSERT(xORB.is(), "OControlWizard::implGetDSContext: invalid service factory!");

            m_aContext.xDatasourceContext = Reference<XNameAccess>(
                DatabaseContext::create(comphelper::ComponentContext(xORB).getUNOContext()),
                UNO_QUERY_THROW);
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::implGetDSContext: invalid database context!");
        }
    }

    //---------------------------------------------------------------------
    Reference< XConnection > OControlWizard::getFormConnection(const OAccessRegulator&) const
    {
        return getFormConnection();
    }
    //---------------------------------------------------------------------
    Reference< XConnection > OControlWizard::getFormConnection() const
    {
        Reference< XConnection > xConn;
        try
        {
            if ( !::dbtools::isEmbeddedInDatabase(m_aContext.xForm,xConn) )
                m_aContext.xForm->getPropertyValue(::rtl::OUString("ActiveConnection")) >>= xConn;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::getFormConnection: caught an exception!");
        }
        return xConn;
    }

    //---------------------------------------------------------------------
    void OControlWizard::setFormConnection( const OAccessRegulator& _rAccess, const Reference< XConnection >& _rxConn, sal_Bool _bAutoDispose )
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
                // for this, use a AutoDisposer (so the conn is cleaned up when the form dies or get's another connection)
                Reference< XRowSet > xFormRowSet( m_aContext.xForm, UNO_QUERY );
                OAutoConnectionDisposer* pAutoDispose = new OAutoConnectionDisposer( xFormRowSet, _rxConn );
                Reference< XPropertyChangeListener > xEnsureDelete( pAutoDispose );
            }
            else
            {
                m_aContext.xForm->setPropertyValue( ::rtl::OUString("ActiveConnection"), makeAny( _rxConn ) );
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::setFormConnection: caught an exception!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizard::updateContext(const OAccessRegulator&)
    {
        return initContext();
    }
    //---------------------------------------------------------------------
    Reference< XInteractionHandler > OControlWizard::getInteractionHandler(Window* _pWindow) const
    {
        const ::rtl::OUString sInteractionHandlerServiceName("com.sun.star.task.InteractionHandler");
        Reference< XInteractionHandler > xHandler;
        try
        {
            if (getServiceFactory().is())
                xHandler = Reference< XInteractionHandler >(getServiceFactory()->createInstance(sInteractionHandlerServiceName), UNO_QUERY);
        }
        catch(const Exception&) { }
        if (!xHandler.is())
            ShowServiceNotAvailableError(_pWindow, sInteractionHandlerServiceName, sal_True);
        return xHandler;
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
        m_aContext.aTypes.clear();
        m_aContext.bEmbedded = sal_False;

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
                ::rtl::OUString sObjectName = ::comphelper::getString(m_aContext.xForm->getPropertyValue(::rtl::OUString("Command")));
                sal_Int32 nObjectType = ::comphelper::getINT32(m_aContext.xForm->getPropertyValue(::rtl::OUString("CommandType")));

                // calculate the connection the rowset is working with
                Reference< XConnection > xConnection;
                m_aContext.bEmbedded = ::dbtools::isEmbeddedInDatabase( m_aContext.xForm, xConnection );
                if ( !m_aContext.bEmbedded )
                    xConnection = ::dbtools::connectRowset( m_aContext.xRowSet, getServiceFactory(), sal_True );

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
                            xStatementProps->setPropertyValue(::rtl::OUString("MaxRows"), makeAny(sal_Int32(0)));

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
                static const ::rtl::OUString s_sFieldTypeProperty("Type");
                const ::rtl::OUString* pBegin = m_aContext.aFieldNames.getConstArray();
                const ::rtl::OUString* pEnd   = pBegin + m_aContext.aFieldNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                {
                    sal_Int32 nFieldType = DataType::OTHER;
                    try
                    {
                        Reference< XPropertySet > xColumn;
                        xColumns->getByName(*pBegin) >>= xColumn;
                        xColumn->getPropertyValue(s_sFieldTypeProperty) >>= nFieldType;
                    }
                    catch(const Exception&)
                    {
                        OSL_FAIL("OControlWizard::initContext: unexpected exception while gathering column information!");
                    }
                    m_aContext.aTypes.insert(OControlWizardContext::TNameTypeMap::value_type(*pBegin,nFieldType));
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
            aContext.Message = String(ModuleRes(RID_STR_COULDNOTOPENTABLE));
            aContext.NextException = aSQLException;

            // create an interaction handler to display this exception
            Reference< XInteractionHandler > xHandler = getInteractionHandler(this);
            if ( !xHandler.is() )
                return sal_False;

            Reference< XInteractionRequest > xRequest = new OInteractionRequest(makeAny(aContext));
            try
            {
                xHandler->handle(xRequest);
            }
            catch(const Exception&) { }
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
            ::rtl::OUString sLabelPropertyName("Label");
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName(sLabelPropertyName))
            {
                ::rtl::OUString sControlLabel(_pSettings->sControlLabel);
                m_aContext.xObjectModel->setPropertyValue(
                    ::rtl::OUString("Label"),
                    makeAny(sControlLabel)
                );
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::commitControlSettings: could not commit the basic control settings!");
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
            ::rtl::OUString sLabelPropertyName("Label");
            Reference< XPropertySetInfo > xInfo = m_aContext.xObjectModel->getPropertySetInfo();
            if (xInfo.is() && xInfo->hasPropertyByName(sLabelPropertyName))
            {
                ::rtl::OUString sControlLabel;
                m_aContext.xObjectModel->getPropertyValue(sLabelPropertyName) >>= sControlLabel;
                _pSettings->sControlLabel = sControlLabel;
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OControlWizard::initControlSettings: could not retrieve the basic control settings!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OControlWizard::needDatasourceSelection()
    {
        // lemme see ...
        return (0 == getContext().aFieldNames.getLength());
            // if we got fields, the data source is valid ...
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
