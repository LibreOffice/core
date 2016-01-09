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

#include "adtabdlg.hxx"
#include "browserids.hxx"
#include "dbu_qry.hrc"
#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "defaultobjectnamecheck.hxx"
#include "dlgsave.hxx"
#include "localresaccess.hxx"
#include "uiservices.hxx"
#include "QTableWindow.hxx"
#include "QTableWindowData.hxx"
#include "querycontainerwindow.hxx"
#include "querycontroller.hxx"
#include "QueryDesignView.hxx"
#include "QueryTableView.hxx"
#include "QueryTextView.hxx"
#include "queryview.hxx"
#include "QueryViewSwitch.hxx"
#include "sqlmessage.hxx"
#include "TableConnectionData.hxx"
#include "TableFieldDescription.hxx"
#include "UITools.hxx"
#include "QueryPropertiesDialog.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/streamsection.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sfx2/sfxsids.hrc>
#include <svtools/localresaccess.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <rtl/strbuf.hxx>
#include <vector>

extern "C" void SAL_CALL createRegistryInfo_OQueryControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OQueryController > aAutoRegistration;
}

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    class OViewController : public OQueryController
    {
        virtual OUString SAL_CALL getImplementationName() throw( RuntimeException, std::exception ) override
        {
            return getImplementationName_Static();
        }
        virtual Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override
        {
            return getSupportedServiceNames_Static();
        }

    public:
        explicit OViewController(const Reference< XComponentContext >& _rM) : OQueryController(_rM){}

        // need by registration
        static OUString getImplementationName_Static() throw( RuntimeException )
        {
            return OUString("org.openoffice.comp.dbu.OViewDesign");
        }
        static Sequence< OUString > getSupportedServiceNames_Static() throw( RuntimeException )
        {
            Sequence<OUString> aSupported { "com.sun.star.sdb.ViewDesign" };
            return aSupported;
        }
        static Reference< XInterface > SAL_CALL Create(const Reference< XMultiServiceFactory >& _rM)
        {
            return *(new OViewController(comphelper::getComponentContext(_rM)));
        }
    };
}

extern "C" void SAL_CALL createRegistryInfo_OViewControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OViewController > aAutoRegistration;
}

namespace dbaui
{
    using namespace ::connectivity;
#if OSL_DEBUG_LEVEL > 1
    namespace
    {
        void insertParseTree(SvTreeListBox* _pBox,::connectivity::OSQLParseNode* _pNode,SvTreeListEntry* _pParent = NULL)
        {
            OUString rString;
            if (!_pNode->isToken())
            {
                // rule name as rule: ...
                rString = "RULE_ID: " + OUString::number( (sal_Int32)_pNode->getRuleID() ) +
                          "(" + OSQLParser::RuleIDToStr(_pNode->getRuleID()) + ")";

                _pParent = _pBox->InsertEntry(rString,_pParent);

                // determine how much subtrees this node has
                sal_uInt32 nStop = _pNode->count();
                // fetch first subtree
                for(sal_uInt32 i=0;i<nStop;++i)
                    insertParseTree(_pBox,_pNode->getChild(i),_pParent);
            }
            else
            {
                // token found
                // tabs to insert according to nLevel

                switch (_pNode->getNodeType())
                {

                case SQL_NODE_KEYWORD:
                    {
                        rString += "SQL_KEYWORD:";
                        OString sT = OSQLParser::TokenIDToStr(_pNode->getTokenID());
                        rString += OStringToOUString(sT, RTL_TEXTENCODING_UTF8);
                     break;}

                case SQL_NODE_COMPARISON:
                    {
                        rString += "SQL_COMPARISON:" + _pNode->getTokenValue(); // append Nodevalue
                            // and start new line
                        break;}

                case SQL_NODE_NAME:
                    {
                        rString += "SQL_NAME:\"" + _pNode->getTokenValue() + "\"";
                        break;}

                case SQL_NODE_STRING:
                    {
                        rString += "SQL_STRING:'" + _pNode->getTokenValue();
                        break;}

                case SQL_NODE_INTNUM:
                    {
                        rString += "SQL_INTNUM:" + _pNode->getTokenValue();
                        break;}

                case SQL_NODE_APPROXNUM:
                    {
                        rString += "SQL_APPROXNUM:" + _pNode->getTokenValue();
                        break;}

                case SQL_NODE_PUNCTUATION:
                    {
                        rString += "SQL_PUNCTUATION:" + _pNode->getTokenValue(); // append Nodevalue
                        break;}

                case SQL_NODE_AMMSC:
                    {
                        rString += "SQL_AMMSC:" + _pNode->getTokenValue(); // append Nodevalue
                        break;}

                default:
                    OSL_FAIL("OSQLParser::ShowParseTree: unzulaessiger NodeType");
                    rString += _pNode->getTokenValue();
                }
                _pBox->InsertEntry(rString,_pParent);
            }
        }
    }
#endif // OSL_DEBUG_LEVEL

    namespace
    {
        OUString lcl_getObjectResourceString( sal_uInt16 _nResId, sal_Int32 _nCommandType )
        {
            OUString sMessageText = ModuleRes( _nResId );
            OUString sObjectType;
            {
                LocalResourceAccess aLocalRes( RSC_QUERY_OBJECT_TYPE, RSC_RESOURCE );
                sObjectType = ModuleRes( (sal_uInt16)( _nCommandType + 1 ) );
            }
            sMessageText = sMessageText.replaceFirst( "$object$", sObjectType );
            return sMessageText;
        }
    }

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::awt;
using namespace ::dbtools;

using namespace ::comphelper;

namespace
{
    void ensureToolbars( OQueryController& _rController, bool _bDesign )
    {
        Reference< css::frame::XLayoutManager > xLayoutManager = OGenericUnoController::getLayoutManager( _rController.getFrame() );
        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            static const char s_sDesignToolbar[] = "private:resource/toolbar/designobjectbar";
            static const char s_sSqlToolbar[] = "private:resource/toolbar/sqlobjectbar";
            if ( _bDesign )
            {
                xLayoutManager->destroyElement( s_sSqlToolbar );
                xLayoutManager->createElement( s_sDesignToolbar );
            }
            else
            {
                xLayoutManager->destroyElement( s_sDesignToolbar );
                xLayoutManager->createElement( s_sSqlToolbar );
            }
            xLayoutManager->unlock();
            xLayoutManager->doLayout();
        }
    }

    /**
     * The value of m_nLimit is updated when LimitBox loses its focus
     * So in those case when execution needs recent data, grab the focus
     * (e.g. execute SQL statement, change views)
     */
    void grabFocusFromLimitBox( OQueryController& _rController )
    {
        static const char sResourceURL[] = "private:resource/toolbar/designobjectbar";
        Reference< XLayoutManager > xLayoutManager = OGenericUnoController::getLayoutManager( _rController.getFrame() );
        Reference< XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
        if (xUIElement.is())
        {
            Reference< XWindow > xWindow(xUIElement->getRealInterface(), css::uno::UNO_QUERY);
            vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if( pWindow && pWindow->HasChildPathFocus() )
            {
                pWindow->GrabFocusToDocument();
            }
        }
    }
}

OUString SAL_CALL OQueryController::getImplementationName() throw( RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString OQueryController::getImplementationName_Static() throw( RuntimeException )
{
    return OUString("org.openoffice.comp.dbu.OQueryDesign");
}

Sequence< OUString> OQueryController::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence<OUString> aSupported { "com.sun.star.sdb.QueryDesign" };
    return aSupported;
}

Sequence< OUString> SAL_CALL OQueryController::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > SAL_CALL OQueryController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OQueryController(comphelper::getComponentContext(_rxFactory)));
}

OQueryController::OQueryController(const Reference< XComponentContext >& _rM)
    :OJoinController(_rM)
    ,OQueryController_PBase( getBroadcastHelper() )
    ,m_pParseContext( new svxform::OSystemParseContext )
    ,m_aSqlParser( _rM, m_pParseContext )
    ,m_pSqlIterator(nullptr)
    ,m_nLimit(-1)
    ,m_nVisibleRows(0x400)
    ,m_nSplitPos(-1)
    ,m_nCommandType( CommandType::QUERY )
    ,m_bGraphicalDesign(false)
    ,m_bDistinct(false)
    ,m_bEscapeProcessing(true)
{
    InvalidateAll();

    registerProperty( PROPERTY_ACTIVECOMMAND, PROPERTY_ID_ACTIVECOMMAND, PropertyAttribute::READONLY | PropertyAttribute::BOUND,
        &m_sStatement, cppu::UnoType<decltype(m_sStatement)>::get() );
    registerProperty( PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::READONLY | PropertyAttribute::BOUND,
        &m_bEscapeProcessing, cppu::UnoType<decltype(m_bEscapeProcessing)>::get() );
}

OQueryController::~OQueryController()
{
    if ( !getBroadcastHelper().bDisposed && !getBroadcastHelper().bInDispose )
    {
        OSL_FAIL("Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

IMPLEMENT_FORWARD_XINTERFACE2( OQueryController, OJoinController, OQueryController_PBase )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( OQueryController, OJoinController, OQueryController_PBase )

Reference< XPropertySetInfo > SAL_CALL OQueryController::getPropertySetInfo() throw(RuntimeException, std::exception)
{
    Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

sal_Bool SAL_CALL OQueryController::convertFastPropertyValue( Any& o_rConvertedValue, Any& o_rOldValue, sal_Int32 i_nHandle, const Any& i_rValue ) throw (IllegalArgumentException)
{
    return OPropertyContainer::convertFastPropertyValue( o_rConvertedValue, o_rOldValue, i_nHandle, i_rValue );
}

void SAL_CALL OQueryController::setFastPropertyValue_NoBroadcast( sal_Int32 i_nHandle, const Any& i_rValue ) throw ( Exception, std::exception )
{
    OPropertyContainer::setFastPropertyValue_NoBroadcast( i_nHandle, i_rValue );
}

void SAL_CALL OQueryController::getFastPropertyValue( Any& o_rValue, sal_Int32 i_nHandle ) const
{
    switch ( i_nHandle )
    {
    case PROPERTY_ID_CURRENT_QUERY_DESIGN:
    {
        ::comphelper::NamedValueCollection aCurrentDesign;
        aCurrentDesign.put( "GraphicalDesign", isGraphicalDesign() );
        aCurrentDesign.put( OUString(PROPERTY_ESCAPE_PROCESSING), m_bEscapeProcessing );

        if ( isGraphicalDesign() )
        {
            getContainer()->SaveUIConfig();
            saveViewSettings( aCurrentDesign, true );
            aCurrentDesign.put( "Statement", m_sStatement );
        }
        else
        {
            aCurrentDesign.put( "Statement", getContainer()->getStatement() );
        }

        o_rValue <<= aCurrentDesign.getPropertyValues();
    }
    break;

    default:
        OPropertyContainer::getFastPropertyValue( o_rValue, i_nHandle );
        break;
    }
}

::cppu::IPropertyArrayHelper& OQueryController::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* OQueryController::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );

    // one additional property:
    const sal_Int32 nLength = aProps.getLength();
    aProps.realloc( nLength + 1 );
    aProps[ nLength ] = Property(
        "CurrentQueryDesign",
        PROPERTY_ID_CURRENT_QUERY_DESIGN,
        ::cppu::UnoType< Sequence< PropertyValue > >::get(),
        PropertyAttribute::READONLY
    );

    ::std::sort(
        aProps.getArray(),
        aProps.getArray() + aProps.getLength(),
        ::comphelper::PropertyCompareByName()
    );

    return new ::cppu::OPropertyArrayHelper(aProps);
}

void OQueryController::deleteIterator()
{
    if(m_pSqlIterator)
    {
        delete m_pSqlIterator->getParseTree();
        m_pSqlIterator->dispose();
        delete m_pSqlIterator;
        m_pSqlIterator = nullptr;
    }
}

void OQueryController::disposing()
{
    OQueryController_PBase::disposing();

    deleteIterator();

    delete m_pParseContext;

    clearFields();
    OTableFields().swap(m_vUnUsedFieldsDesc);

    ::comphelper::disposeComponent(m_xComposer);
    OJoinController::disposing();
    OQueryController_PBase::disposing();
}

void OQueryController::clearFields()
{
    OTableFields().swap(m_vTableFieldDesc);
}

FeatureState OQueryController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = true;
        // (disabled automatically)

    switch (_nId)
    {
        case ID_BROWSER_EDITDOC:
            if ( editingCommand() )
                aReturn.bEnabled = false;
            else if ( editingView() && !m_xAlterView.is() )
                aReturn.bEnabled = false;
            else
                aReturn = OJoinController::GetState( _nId );
            break;

        case ID_BROWSER_ESCAPEPROCESSING:
            aReturn.bChecked = !m_bEscapeProcessing;
            aReturn.bEnabled = ( m_pSqlIterator != nullptr ) && !m_bGraphicalDesign;
            break;
        case SID_RELATION_ADD_RELATION:
            aReturn.bEnabled = isEditable() && m_bGraphicalDesign && m_vTableData.size() > 1;
            break;
        case ID_BROWSER_SAVEASDOC:
            aReturn.bEnabled = !editingCommand() && !editingView() && (!m_bGraphicalDesign || !(m_vTableFieldDesc.empty() || m_vTableData.empty()));
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = isEditable() && (!m_bGraphicalDesign || !(m_vTableFieldDesc.empty() || m_vTableData.empty()));
            break;
        case SID_PRINTDOCDIRECT:
            break;
        case ID_BROWSER_CUT:
            aReturn.bEnabled = isEditable() && getContainer() && getContainer()->isCutAllowed();
            break;
        case ID_BROWSER_COPY:
            aReturn.bEnabled = getContainer() && getContainer()->isCopyAllowed();
            break;
        case ID_BROWSER_PASTE:
            aReturn.bEnabled = isEditable() && getContainer() && getContainer()->isPasteAllowed();
            break;
        case ID_BROWSER_SQL:
            aReturn.bEnabled = m_bEscapeProcessing && m_pSqlIterator;
            aReturn.bChecked = m_bGraphicalDesign;
            break;
        case SID_BROWSER_CLEAR_QUERY:
            aReturn.bEnabled = isEditable() && (!m_sStatement.isEmpty() || !m_vTableData.empty());
            break;
        case SID_QUERY_VIEW_FUNCTIONS:
        case SID_QUERY_VIEW_TABLES:
        case SID_QUERY_VIEW_ALIASES:
            aReturn.bChecked = getContainer() && getContainer()->isSlotEnabled(_nId);
            aReturn.bEnabled = m_bGraphicalDesign;
            break;
        case SID_QUERY_DISTINCT_VALUES:
            aReturn.bEnabled = m_bGraphicalDesign && isEditable();
            aReturn.bChecked = m_bDistinct;
            break;
        case SID_QUERY_LIMIT:
            aReturn.bEnabled = m_bGraphicalDesign;
            if( aReturn.bEnabled )
                aReturn.aValue = makeAny( m_nLimit );
            break;
        case SID_QUERY_PROP_DLG:
            aReturn.bEnabled = m_bGraphicalDesign;
            break;
        case ID_BROWSER_QUERY_EXECUTE:
            aReturn.bEnabled = true;
            break;
        case SID_DB_QUERY_PREVIEW:
            aReturn.bEnabled = true;
            aReturn.bChecked = getContainer() && getContainer()->getPreviewFrame().is();
            break;
#if OSL_DEBUG_LEVEL > 1
        case ID_EDIT_QUERY_SQL:
            break;
        case ID_EDIT_QUERY_DESIGN:
            break;
#endif
        case ID_BROWSER_ADDTABLE:
            if ( !m_bGraphicalDesign )
            {
                aReturn.bEnabled = false;
                break;
            }
            // run through
        default:
            aReturn = OJoinController::GetState(_nId);
            break;
    }
    return aReturn;
}

void OQueryController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    switch(_nId)
    {
        case ID_BROWSER_ESCAPEPROCESSING:
            setEscapeProcessing_fireEvent( !m_bEscapeProcessing );
            if ( !editingView() )
                setModified(sal_True);
            InvalidateFeature(ID_BROWSER_SQL);
            break;
        case ID_BROWSER_SAVEASDOC:
        case ID_BROWSER_SAVEDOC:
            grabFocusFromLimitBox(*this);
            doSaveAsDoc(ID_BROWSER_SAVEASDOC == _nId);
            break;
        case SID_RELATION_ADD_RELATION:
            {
                OJoinDesignView* pView = getJoinView();
                if( pView )
                    static_cast<OQueryTableView*>(pView->getTableView())->createNewConnection();
            }
            break;
        case SID_PRINTDOCDIRECT:
            break;
        case ID_BROWSER_CUT:
            getContainer()->cut();
            break;
        case ID_BROWSER_COPY:
            getContainer()->copy();
            break;
        case ID_BROWSER_PASTE:
            getContainer()->paste();
            break;
        case ID_BROWSER_SQL:
        {
            grabFocusFromLimitBox(*this);
            if ( !getContainer()->checkStatement() )
                break;
            SQLExceptionInfo aError;
            try
            {
                OUString aErrorMsg;
                setStatement_fireEvent( getContainer()->getStatement() );
                if(m_sStatement.isEmpty() && m_pSqlIterator)
                {
                    // change the view of the data
                    delete m_pSqlIterator->getParseTree();
                    m_pSqlIterator->setParseTree(nullptr);
                    m_bGraphicalDesign = !m_bGraphicalDesign;
                    impl_setViewMode( &aError );
                }
                else
                {
                    ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree(aErrorMsg,m_sStatement,m_bGraphicalDesign);
                    if ( pNode )
                    {
                        delete m_pSqlIterator->getParseTree();
                        m_pSqlIterator->setParseTree(pNode);
                        m_pSqlIterator->traverseAll();

                        if ( m_pSqlIterator->hasErrors() )
                        {
                            aError = m_pSqlIterator->getErrors();
                        }
                        else
                        {
                            const OSQLTables& xTabs = m_pSqlIterator->getTables();
                            if ( m_pSqlIterator->getStatementType() != SQL_STATEMENT_SELECT || xTabs.begin() == xTabs.end() )
                            {
                                aError = SQLException(
                                    OUString( ModuleRes( STR_QRY_NOSELECT ) ),
                                    nullptr,
                                    "S1000",
                                    1000,
                                    Any()
                                );
                            }
                            else
                            {
                                // change the view of the data
                                m_bGraphicalDesign = !m_bGraphicalDesign;
                                OUString sNewStatement;
                                pNode->parseNodeToStr( sNewStatement, getConnection() );
                                setStatement_fireEvent( sNewStatement );
                                getContainer()->SaveUIConfig();
                                m_vTableConnectionData.clear();
                                impl_setViewMode( &aError );
                            }
                        }
                    }
                    else
                    {
                        aError = SQLException(
                            OUString( ModuleRes( STR_QRY_SYNTAX ) ),
                            nullptr,
                            "S1000",
                            1000,
                            Any()
                        );
                    }
                }
            }
            catch(const SQLException&)
            {
                aError = ::cppu::getCaughtException();
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            if ( aError.isValid() )
                showError( aError );

            if(m_bGraphicalDesign)
            {
                InvalidateFeature(ID_BROWSER_ADDTABLE);
                InvalidateFeature(SID_RELATION_ADD_RELATION);
            }
        }
        break;
        case SID_BROWSER_CLEAR_QUERY:
            {
                GetUndoManager().EnterListAction( OUString( ModuleRes(STR_QUERY_UNDO_TABWINDELETE) ), OUString() );
                getContainer()->clear();
                GetUndoManager().LeaveListAction();

                setStatement_fireEvent( OUString() );
                if(m_bGraphicalDesign)
                    InvalidateFeature(ID_BROWSER_ADDTABLE);
            }
            break;
        case SID_QUERY_VIEW_FUNCTIONS:
        case SID_QUERY_VIEW_TABLES:
        case SID_QUERY_VIEW_ALIASES:
            getContainer()->setSlotEnabled(_nId,!getContainer()->isSlotEnabled(_nId));
            setModified(sal_True);
            break;
        case SID_QUERY_DISTINCT_VALUES:
            m_bDistinct = !m_bDistinct;
            setModified(sal_True);
            break;
        case SID_QUERY_LIMIT:
            if ( aArgs.getLength() >= 1 && aArgs[0].Name == "DBLimit.Value" )
            {
                aArgs[0].Value >>= m_nLimit;
                setModified(sal_True);
            }
            break;
        case SID_QUERY_PROP_DLG:
            grabFocusFromLimitBox(*this);
            execute_QueryPropDlg();
            break;
        case ID_BROWSER_QUERY_EXECUTE:
            grabFocusFromLimitBox(*this);
            if ( getContainer()->checkStatement() )
                executeQuery();
            break;
        case SID_DB_QUERY_PREVIEW:
            try
            {
                Reference< css::util::XCloseable > xCloseFrame( getContainer()->getPreviewFrame(), UNO_QUERY );
                if ( xCloseFrame.is() )
                {
                    try
                    {
                        xCloseFrame->close( sal_True );
                    }
                    catch(const Exception&)
                    {
                        OSL_FAIL( "OQueryController::Execute(SID_DB_QUERY_PREVIEW): *nobody* is expected to veto closing the preview frame!" );
                    }
                }
                else
                    Execute(ID_BROWSER_QUERY_EXECUTE,Sequence< PropertyValue >());
            }
            catch(const Exception&)
            {
            }
            break;
        case ID_QUERY_ZOOM_IN:
            {
            }
            break;
        case ID_QUERY_ZOOM_OUT:
            {
            }
            break;
#if OSL_DEBUG_LEVEL > 1
        case ID_EDIT_QUERY_DESIGN:
        case ID_EDIT_QUERY_SQL:
            {
                OUString aErrorMsg;
                setStatement_fireEvent( getContainer()->getStatement() );
                ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree( aErrorMsg, m_sStatement, m_bGraphicalDesign );
                if ( pNode )
                {
                    vcl::Window* pView = getView();
                    ScopedVclPtrInstance<ModalDialog> pWindow( pView, WB_STDMODAL | WB_SIZEMOVE | WB_CENTER );
                    pWindow->SetSizePixel( ::Size( pView->GetSizePixel().Width() / 2, pView->GetSizePixel().Height() / 2 ) );
                    ScopedVclPtrInstance<SvTreeListBox> pTreeBox( pWindow, WB_BORDER | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_VSCROLL );
                    pTreeBox->SetPosSizePixel( ::Point( 6, 6 ), ::Size( pWindow->GetSizePixel().Width() - 12, pWindow->GetSizePixel().Height() - 12 ));
                    pTreeBox->SetNodeDefaultImages();

                    if ( _nId == ID_EDIT_QUERY_DESIGN )
                    {
                        ::connectivity::OSQLParseNode* pTemp = pNode ? pNode->getChild(3)->getChild(1) : NULL;
                        // no where clause found
                        if ( pTemp && !pTemp->isLeaf() )
                        {
                            ::connectivity::OSQLParseNode * pCondition = pTemp->getChild(1);
                            if ( pCondition ) // no where clause
                            {
                                ::connectivity::OSQLParseNode::negateSearchCondition(pCondition);
                                ::connectivity::OSQLParseNode *pNodeTmp = pTemp->getChild(1);

                                ::connectivity::OSQLParseNode::disjunctiveNormalForm(pNodeTmp);
                                pNodeTmp = pTemp->getChild(1);
                                ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
                                pNodeTmp = pTemp->getChild(1);
                                OSQLParseNode::compress(pNodeTmp);
                                pNodeTmp = pTemp->getChild(1);
                            }
                            OUString sTemp;
                            pNode->parseNodeToStr(sTemp,getConnection());
                            getContainer()->setStatement(sTemp);
                        }
                    }

                    insertParseTree(pTreeBox,pNode);

                    pTreeBox->Show();
                    pWindow->Execute();

                    delete pNode;
                }
                break;
            }
#endif
        default:
            OJoinController::Execute(_nId,aArgs);
            return; // else we would invalidate twice
    }
    InvalidateFeature(_nId);
}

void OQueryController::impl_showAutoSQLViewError( const css::uno::Any& _rErrorDetails )
{
    SQLContext aErrorContext;
    aErrorContext.Message = lcl_getObjectResourceString( STR_ERROR_PARSING_STATEMENT, m_nCommandType );
    aErrorContext.Context = *this;
    aErrorContext.Details = lcl_getObjectResourceString( STR_INFO_OPENING_IN_SQL_VIEW, m_nCommandType );
    aErrorContext.NextException = _rErrorDetails;
    showError( aErrorContext );
}

bool OQueryController::impl_setViewMode( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    OSL_PRECOND( getContainer(), "OQueryController::impl_setViewMode: illegal call!" );

    bool wasModified = isModified();

    SQLExceptionInfo aError;
    bool bSuccess = getContainer()->switchView( &aError );
    if ( !bSuccess )
    {
        m_bGraphicalDesign = !m_bGraphicalDesign;
        // restore old state
        getContainer()->switchView( nullptr );
            // don't pass &aError here, this would overwrite the error which the first switchView call
            // returned in this location.
        if ( _pErrorInfo )
            *_pErrorInfo = aError;
        else
            showError( aError );
    }
    else
    {
        ensureToolbars( *this, m_bGraphicalDesign );
    }

    setModified( wasModified );
    return bSuccess;
}

void OQueryController::impl_initialize()
{
    OJoinController::impl_initialize();

    const NamedValueCollection& rArguments( getInitParams() );

    OUString sCommand;
    m_nCommandType = CommandType::QUERY;

    // reading parameters:

    // legacy parameters first (later overwritten by regular parameters)
    OUString sIndependentSQLCommand;
    if ( rArguments.get_ensureType( "IndependentSQLCommand", sIndependentSQLCommand ) )
    {
        OSL_FAIL( "OQueryController::impl_initialize: IndependentSQLCommand is regognized for compatibility only!" );
        sCommand = sIndependentSQLCommand;
        m_nCommandType = CommandType::COMMAND;
    }

    OUString sCurrentQuery;
    if ( rArguments.get_ensureType( "CurrentQuery", sCurrentQuery ) )
    {
        OSL_FAIL( "OQueryController::impl_initialize: CurrentQuery is regognized for compatibility only!" );
        sCommand = sCurrentQuery;
        m_nCommandType = CommandType::QUERY;
    }

    bool bCreateView( false );
    if ( rArguments.get_ensureType( "CreateView", bCreateView ) && bCreateView )
    {
        OSL_FAIL( "OQueryController::impl_initialize: CurrentQuery is regognized for compatibility only!" );
        m_nCommandType = CommandType::TABLE;
    }

    // non-legacy parameters which overwrite the legacy parameters
    rArguments.get_ensureType( PROPERTY_COMMAND, sCommand );
    rArguments.get_ensureType( PROPERTY_COMMAND_TYPE, m_nCommandType );

    // translate Command/Type into proper members
    // TODO/Later: all this (including those members) should be hidden behind some abstract interface,
    // which is implemented for all the three commands
    switch ( m_nCommandType )
    {
    case CommandType::QUERY:
        m_sName = sCommand;
        break;
    case CommandType::TABLE:
        m_sName = sCommand;
        break;
    case CommandType::COMMAND:
        setStatement_fireEvent( sCommand );
        m_sName.clear();
        break;
    default:
        OSL_FAIL( "OQueryController::impl_initialize: logic error in code!" );
        throw RuntimeException();
    }

    // more legacy parameters
    bool bGraphicalDesign( true );
    if ( rArguments.get_ensureType( PROPERTY_QUERYDESIGNVIEW, bGraphicalDesign ) )
    {
        OSL_FAIL( "OQueryController::impl_initialize: QueryDesignView is regognized for compatibility only!" );
        m_bGraphicalDesign = bGraphicalDesign;
    }

    // more non-legacy
    rArguments.get_ensureType( PROPERTY_GRAPHICAL_DESIGN, m_bGraphicalDesign );

    bool bEscapeProcessing( true );
    if ( rArguments.get_ensureType( PROPERTY_ESCAPE_PROCESSING, bEscapeProcessing ) )
    {
        setEscapeProcessing_fireEvent( bEscapeProcessing );

        OSL_ENSURE( m_bEscapeProcessing || !m_bGraphicalDesign, "OQueryController::impl_initialize: can't do the graphical design without escape processing!" );
        if ( !m_bEscapeProcessing )
            m_bGraphicalDesign = false;
    }

    // initial design
    bool bForceInitialDesign = false;
    Sequence< PropertyValue > aCurrentQueryDesignProps;
    aCurrentQueryDesignProps = rArguments.getOrDefault( "CurrentQueryDesign", aCurrentQueryDesignProps );

    if ( aCurrentQueryDesignProps.getLength() )
    {
        ::comphelper::NamedValueCollection aCurrentQueryDesign( aCurrentQueryDesignProps );
        if ( aCurrentQueryDesign.has( OUString(PROPERTY_GRAPHICAL_DESIGN) ) )
        {
            aCurrentQueryDesign.get_ensureType( PROPERTY_GRAPHICAL_DESIGN, m_bGraphicalDesign );
        }
        if ( aCurrentQueryDesign.has( OUString(PROPERTY_ESCAPE_PROCESSING) ) )
        {
            aCurrentQueryDesign.get_ensureType( PROPERTY_ESCAPE_PROCESSING, m_bEscapeProcessing );
        }
        if ( aCurrentQueryDesign.has( "Statement" ) )
        {
            OUString sStatement;
            aCurrentQueryDesign.get_ensureType( "Statement", sStatement );
            aCurrentQueryDesign.remove( "Statement" );
            setStatement_fireEvent( sStatement );
        }

        loadViewSettings( aCurrentQueryDesign );

        bForceInitialDesign = true;
    }

    if ( !ensureConnected( false ) )
    {   // we have no connection so what else should we do
        m_bGraphicalDesign = false;
        if ( editingView() )
        {
            connectionLostMessage();
            throw SQLException();
        }
    }

    // check the view capabilities
    if ( isConnected() && editingView() )
    {
        Reference< XViewsSupplier > xViewsSup( getConnection(), UNO_QUERY );
        Reference< XNameAccess > xViews;
        if ( xViewsSup.is() )
            xViews = xViewsSup->getViews();

        if ( !xViews.is() )
        {   // we can't create views so we ask if the user wants to create a query instead
            m_nCommandType = CommandType::QUERY;
            bool bClose = false;
            {
                OUString aTitle( ModuleRes( STR_QUERYDESIGN_NO_VIEW_SUPPORT ) );
                OUString aMessage( ModuleRes( STR_QUERYDESIGN_NO_VIEW_ASK ) );
                ODataView* pWindow = getView();
                ScopedVclPtrInstance< OSQLMessageBox > aDlg( pWindow, aTitle, aMessage, WB_YES_NO | WB_DEF_YES, OSQLMessageBox::Query );
                bClose = aDlg->Execute() == RET_NO;
            }
            if ( bClose )
                throw VetoException();
        }

        // now if we are to edit an existing view, check whether this is possible
        if ( !m_sName.isEmpty() )
        {
            Any aView( xViews->getByName( m_sName ) );
                // will throw if there is no such view
            if ( !( aView >>= m_xAlterView ) )
            {
                throw IllegalArgumentException(
                    OUString( ModuleRes( STR_NO_ALTER_VIEW_SUPPORT ) ),
                    *this,
                    1
                );
            }
        }
    }

    OSL_ENSURE(getDataSource().is(),"OQueryController::impl_initialize: need a datasource!");

    try
    {
        getContainer()->initialize();
        impl_reset( bForceInitialDesign );

        SQLExceptionInfo aError;
        const bool bAttemptedGraphicalDesign = m_bGraphicalDesign;

        if ( bForceInitialDesign )
        {
            getContainer()->forceInitialView();
        }
        else
        {
            impl_setViewMode( &aError );
        }

        if ( aError.isValid() && bAttemptedGraphicalDesign && !m_bGraphicalDesign )
        {
            // we tried initializing the graphical view, this failed, and we were automatically switched to SQL
            // view => tell this to the user
            if ( !editingView() )
            {
                impl_showAutoSQLViewError( aError.get() );
            }
        }

        ClearUndoManager();

        if  (  ( m_bGraphicalDesign )
            && (  ( m_sName.isEmpty() && !editingCommand() )
               || ( m_sStatement.isEmpty() && editingCommand() )
               )
            )
        {
            Application::PostUserEvent( LINK( this, OQueryController, OnExecuteAddTable ) );
        }

        setModified(sal_False);
    }
    catch(const SQLException& e)
    {
        DBG_UNHANDLED_EXCEPTION();
        // we caught an exception so we switch to text only mode
        {
            m_bGraphicalDesign = false;
            getContainer()->initialize();
            ODataView* pWindow = getView();
            ScopedVclPtr<OSQLMessageBox>::Create(pWindow,e)->Execute();
        }
        throw;
    }
}

void OQueryController::onLoadedMenu(const Reference< css::frame::XLayoutManager >& /*_xLayoutManager*/)
{
    ensureToolbars( *this, m_bGraphicalDesign );
}

OUString OQueryController::getPrivateTitle( ) const
{
    OUString sName = m_sName;
    if ( sName.isEmpty() )
    {
        if ( !editingCommand() )
        {
            SolarMutexGuard aSolarGuard;
            ::osl::MutexGuard aGuard( getMutex() );
            OUString aDefaultName = ModuleRes( editingView() ? STR_VIEW_TITLE : STR_QRY_TITLE );
            sName = aDefaultName.getToken(0,' ');
            sName += OUString::number(getCurrentStartNumber());
        }
    }
    return sName;
}

void OQueryController::setQueryComposer()
{
    if(isConnected())
    {
        Reference< XSQLQueryComposerFactory >  xFactory(getConnection(), UNO_QUERY);
        OSL_ENSURE(xFactory.is(),"Connection doesn't support a querycomposer");
        if ( xFactory.is() && getContainer() )
        {
            try
            {
                m_xComposer = xFactory->createQueryComposer();
                getContainer()->setStatement(m_sStatement);
            }
            catch(const Exception&)
            {
                m_xComposer = nullptr;
            }
            OSL_ENSURE(m_xComposer.is(),"No querycomposer available!");
            Reference<XTablesSupplier> xTablesSup(getConnection(), UNO_QUERY);
            deleteIterator();
            m_pSqlIterator = new ::connectivity::OSQLParseTreeIterator( getConnection(), xTablesSup->getTables(), m_aSqlParser, nullptr );
        }
    }
}

bool OQueryController::Construct(vcl::Window* pParent)
{
    // TODO: we have to check if we should create the text view or the design view

    setView( VclPtr<OQueryContainerWindow>::Create( pParent, *this, getORB() ) );

    return OJoinController::Construct(pParent);
}

OJoinDesignView* OQueryController::getJoinView()
{
    return getContainer()->getDesignView();
}

void OQueryController::describeSupportedFeatures()
{
    OJoinController::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:SaveAs",            ID_BROWSER_SAVEASDOC,       CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SbaNativeSql",      ID_BROWSER_ESCAPEPROCESSING,CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:DBViewFunctions",   SID_QUERY_VIEW_FUNCTIONS,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewTableNames",  SID_QUERY_VIEW_TABLES,      CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewAliases",     SID_QUERY_VIEW_ALIASES,     CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBDistinctValues",  SID_QUERY_DISTINCT_VALUES,  CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:DBChangeDesignMode",ID_BROWSER_SQL,             CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBClearQuery",      SID_BROWSER_CLEAR_QUERY,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:SbaExecuteSql",     ID_BROWSER_QUERY_EXECUTE,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBAddRelation",     SID_RELATION_ADD_RELATION,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBQueryPreview",    SID_DB_QUERY_PREVIEW,       CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBLimit",           SID_QUERY_LIMIT,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:DBQueryPropertiesDialog", SID_QUERY_PROP_DLG,         CommandGroup::FORMAT );

#if OSL_DEBUG_LEVEL > 1
    implDescribeSupportedFeature( ".uno:DBShowParseTree",   ID_EDIT_QUERY_SQL );
    implDescribeSupportedFeature( ".uno:DBMakeDisjunct",    ID_EDIT_QUERY_DESIGN );
#endif
}

void OQueryController::impl_onModifyChanged()
{
    OJoinController::impl_onModifyChanged();
    InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
    InvalidateFeature(ID_BROWSER_SAVEASDOC);
    InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
}

void SAL_CALL OQueryController::disposing( const EventObject& Source ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( getContainer() && Source.Source.is() )
    {
        if ( Source.Source == m_aCurrentFrame.getFrame() )
        {   // our frame is being disposed -> close the preview window (if we have one)
            Reference< XFrame2 > xPreviewFrame( getContainer()->getPreviewFrame() );
            ::comphelper::disposeComponent( xPreviewFrame );
        }
        else if ( Source.Source == getContainer()->getPreviewFrame() )
        {
            getContainer()->disposingPreview();
        }
    }

    OJoinController::disposing(Source);
}

void OQueryController::reconnect(bool _bUI)
{
    deleteIterator();
    ::comphelper::disposeComponent(m_xComposer);

    OJoinController::reconnect( _bUI );

    if (isConnected())
    {
        setQueryComposer();
    }
    else
    {
        if(m_bGraphicalDesign)
        {
            m_bGraphicalDesign = false;
            // don't call Execute(SQL) because this changes the sql statement
            impl_setViewMode( nullptr );
        }
        InvalidateAll();
    }
}

void OQueryController::saveViewSettings( ::comphelper::NamedValueCollection& o_rViewSettings, const bool i_includingCriteria ) const
{
    saveTableWindows( o_rViewSettings );

    OTableFields::const_iterator field = m_vTableFieldDesc.begin();
    OTableFields::const_iterator fieldEnd = m_vTableFieldDesc.end();

    ::comphelper::NamedValueCollection aAllFieldsData;
    ::comphelper::NamedValueCollection aFieldData;
    for ( sal_Int32 i = 1; field != fieldEnd; ++field, ++i )
    {
        if ( !(*field)->IsEmpty() )
        {
            aFieldData.clear();
            (*field)->Save( aFieldData, i_includingCriteria );

            const OUString sFieldSettingName = "Field" + OUString::number( i );
            aAllFieldsData.put( sFieldSettingName, aFieldData.getPropertyValues() );
        }
    }

    o_rViewSettings.put( "Fields", aAllFieldsData.getPropertyValues() );
    o_rViewSettings.put( "SplitterPosition", m_nSplitPos );
    o_rViewSettings.put( "VisibleRows", m_nVisibleRows );
}

void OQueryController::loadViewSettings( const ::comphelper::NamedValueCollection& o_rViewSettings )
{
    loadTableWindows( o_rViewSettings );

    m_nSplitPos = o_rViewSettings.getOrDefault( "SplitterPosition", m_nSplitPos );
    m_nVisibleRows = o_rViewSettings.getOrDefault( "VisibleRows", m_nVisibleRows );
    m_aFieldInformation = o_rViewSettings.getOrDefault( "Fields", m_aFieldInformation );
}

void OQueryController::execute_QueryPropDlg()
{
    ScopedVclPtrInstance<QueryPropertiesDialog> aQueryPropDlg(
        getContainer(), m_bDistinct, m_nLimit );

    if( aQueryPropDlg->Execute() == RET_OK )
    {
        m_bDistinct = aQueryPropDlg->getDistinct();
        m_nLimit = aQueryPropDlg->getLimit();
        InvalidateFeature( SID_QUERY_DISTINCT_VALUES );
        InvalidateFeature( SID_QUERY_LIMIT, nullptr, true );
    }
}

sal_Int32 OQueryController::getColWidth(sal_uInt16 _nColPos)  const
{
    if ( _nColPos < m_aFieldInformation.getLength() )
    {
        rtl::Reference<OTableFieldDesc> pField( new OTableFieldDesc());
        pField->Load( m_aFieldInformation[ _nColPos ], false );
        return pField->GetColWidth();
    }
    return 0;
}

Reference<XNameAccess> OQueryController::getObjectContainer()  const
{
    Reference< XNameAccess > xElements;
    if ( editingView() )
    {
        Reference< XViewsSupplier > xViewsSupp( getConnection(), UNO_QUERY );
        if ( xViewsSupp.is() )
            xElements = xViewsSupp->getViews();
    }
    else
    {
        Reference< XQueriesSupplier > xQueriesSupp( getConnection(), UNO_QUERY );
        if ( xQueriesSupp.is() )
            xElements = xQueriesSupp->getQueries();
        else
        {
            Reference< XQueryDefinitionsSupplier > xQueryDefsSupp( getDataSource(), UNO_QUERY );
            if ( xQueryDefsSupp.is() )
                xElements = xQueryDefsSupp->getQueryDefinitions();
        }
    }

    OSL_ENSURE( xElements.is(), "OQueryController::getObjectContainer: unable to obtain the container!" );
    return xElements;
}

void OQueryController::executeQuery()
{
    // we don't need to check the connection here because we already check the composer
    // which can't live without his connection
    OUString sTranslatedStmt = translateStatement( false );

    OUString sDataSourceName = getDataSourceName();
    if ( !(sDataSourceName.isEmpty() || sTranslatedStmt.isEmpty()) )
    {
        try
        {
            getContainer()->showPreview( getFrame() );
            InvalidateFeature(SID_DB_QUERY_PREVIEW);

            URL aWantToDispatch;
            aWantToDispatch.Complete = ".component:DB/DataSourceBrowser";

            OUString sFrameName( FRAME_NAME_QUERY_PREVIEW );
            sal_Int32 nSearchFlags = FrameSearchFlag::CHILDREN;

            Reference< XDispatch> xDisp;
            Reference< XDispatchProvider> xProv( getFrame()->findFrame( sFrameName, nSearchFlags ), UNO_QUERY );
            if(!xProv.is())
            {
                xProv.set( getFrame(), UNO_QUERY );
                if (xProv.is())
                    xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, nSearchFlags);
            }
            else
            {
                xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, FrameSearchFlag::SELF);
            }
            if (xDisp.is())
            {
                auto aProps(::comphelper::InitPropertySequence(
                    {
                        { PROPERTY_DATASOURCENAME, makeAny(sDataSourceName) },
                        { PROPERTY_COMMAND_TYPE, makeAny(CommandType::COMMAND) },
                        { PROPERTY_COMMAND, makeAny(sTranslatedStmt) },
                        { PROPERTY_ENABLE_BROWSER, makeAny(false) },
                        { PROPERTY_ACTIVE_CONNECTION, makeAny(getConnection()) },
                        { PROPERTY_UPDATE_CATALOGNAME, makeAny(m_sUpdateCatalogName) },
                        { PROPERTY_UPDATE_SCHEMANAME, makeAny(m_sUpdateSchemaName) },
                        { PROPERTY_UPDATE_TABLENAME, makeAny(m_sUpdateTableName) },
                        { PROPERTY_ESCAPE_PROCESSING, makeAny(m_bEscapeProcessing) }
                    }));

                xDisp->dispatch(aWantToDispatch, aProps);
                // check the state of the beamer
                // be notified when the beamer frame is closed
                Reference< XComponent >  xComponent( getFrame()->findFrame( sFrameName, nSearchFlags ), UNO_QUERY );
                if (xComponent.is())
                {
                    OSL_ENSURE(Reference< XFrame >(xComponent, UNO_QUERY).get() == getContainer()->getPreviewFrame().get(),
                        "OQueryController::executeQuery: oops ... which window do I have here?");
                    Reference< XEventListener> xEvtL(static_cast<cppu::OWeakObject*>(this),UNO_QUERY);
                    xComponent->addEventListener(xEvtL);
                }
            }
            else
            {
                OSL_FAIL("Couldn't create a beamer window!");
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("Couldn't create a beamer window!");
        }
    }
}

bool OQueryController::askForNewName(const Reference<XNameAccess>& _xElements, bool _bSaveAs)
{
    OSL_ENSURE( !editingCommand(), "OQueryController::askForNewName: not to be called when designing an independent statement!" );
    if ( editingCommand() )
        return false;

    OSL_PRECOND( _xElements.is(), "OQueryController::askForNewName: invalid container!" );
    if  ( !_xElements.is() )
        return false;

    bool bRet = true;
    bool bNew = _bSaveAs || !_xElements->hasByName( m_sName );
    if(bNew)
    {
        OUString aDefaultName;
        if ( ( _bSaveAs && !bNew ) || ( bNew && !m_sName.isEmpty() ) )
            aDefaultName = m_sName;
        else
        {
            OUString sName = ModuleRes( editingView() ? STR_VIEW_TITLE : STR_QRY_TITLE );
            aDefaultName = sName.getToken(0,' ');
            aDefaultName = ::dbtools::createUniqueName(_xElements,aDefaultName);
        }

        DynamicTableOrQueryNameCheck aNameChecker( getConnection(), CommandType::QUERY );
        ScopedVclPtrInstance<OSaveAsDlg> aDlg(
                getView(),
                m_nCommandType,
                getORB(),
                getConnection(),
                aDefaultName,
                aNameChecker,
                SAD_DEFAULT );

        bRet = ( aDlg->Execute() == RET_OK );
        if ( bRet )
        {
            m_sName = aDlg->getName();
            if ( editingView() )
            {
                m_sUpdateCatalogName    = aDlg->getCatalog();
                m_sUpdateSchemaName     = aDlg->getSchema();
            }
        }
    }
    return bRet;
}

bool OQueryController::doSaveAsDoc(bool _bSaveAs)
{
    OSL_ENSURE(isEditable(),"Slot ID_BROWSER_SAVEDOC should not be enabled!");
    if ( !editingCommand() && !haveDataSource() )
    {
        OUString aMessage(ModuleRes(STR_DATASOURCE_DELETED));
        ScopedVclPtr<OSQLWarningBox>::Create( getView(), aMessage )->Execute();
        return false;
    }

    Reference< XNameAccess > xElements = getObjectContainer();
    if ( !xElements.is() )
        return false;

    if ( !getContainer()->checkStatement() )
        return false;

    OUString sTranslatedStmt = translateStatement();
    if ( editingCommand() )
    {
        setModified( sal_False );
        // this is all we need to do here. translateStatement implicitly set our m_sStatement, and
        // notified it, and that's all
        return true;
    }

    if ( sTranslatedStmt.isEmpty() )
        return false;

    // first we need a name for our query so ask the user
    // did we get a name
    OUString sOriginalName( m_sName );
    if ( !askForNewName( xElements, _bSaveAs ) || m_sName.isEmpty() )
        return false;

    SQLExceptionInfo aInfo;
    bool bSuccess = false;
    bool bNew = false;
    try
    {
        bNew = ( _bSaveAs )
            || ( !xElements->hasByName( m_sName ) );

        Reference<XPropertySet> xQuery;
        if ( bNew ) // just to make sure the query already exists
        {
            // drop the query, in case it already exists
            if ( xElements->hasByName( m_sName ) )
            {
                Reference< XDrop > xNameCont( xElements, UNO_QUERY );
                if ( xNameCont.is() )
                    xNameCont->dropByName( m_sName );
                else
                {
                    Reference< XNameContainer > xCont( xElements, UNO_QUERY );
                    if ( xCont.is() )
                        xCont->removeByName( m_sName );
                }
            }

            // create a new (empty, uninitialized) query resp. view
            Reference< XDataDescriptorFactory > xFact( xElements, UNO_QUERY );
            if ( xFact.is() )
            {
                xQuery = xFact->createDataDescriptor();
                // to set the name is only allowed when the query is new
                xQuery->setPropertyValue( PROPERTY_NAME, makeAny( m_sName ) );
            }
            else
            {
                Reference< XSingleServiceFactory > xSingleFac( xElements, UNO_QUERY );
                if ( xSingleFac.is() )
                    xQuery.set(xSingleFac->createInstance(), css::uno::UNO_QUERY);
            }
        }
        else
        {
            xElements->getByName( m_sName ) >>= xQuery;
        }
        if ( !xQuery.is() )
            throw RuntimeException();

        // the new commands
        if ( editingView() && !bNew )
        {
            OSL_ENSURE( xQuery == m_xAlterView, "OQueryController::doSaveAsDoc: already have another alterable view ...!?" );
            m_xAlterView.set( xQuery, UNO_QUERY_THROW );
            m_xAlterView->alterCommand( sTranslatedStmt );
        }
        else
        {   // we're creating a query, or a *new* view
            xQuery->setPropertyValue( PROPERTY_COMMAND, makeAny( sTranslatedStmt ) );

            if ( editingView() )
            {
                xQuery->setPropertyValue( PROPERTY_CATALOGNAME, makeAny( m_sUpdateCatalogName ) );
                xQuery->setPropertyValue( PROPERTY_SCHEMANAME, makeAny( m_sUpdateSchemaName ) );
            }

            if ( editingQuery() )
            {
                xQuery->setPropertyValue( PROPERTY_UPDATE_TABLENAME, makeAny( m_sUpdateTableName ) );
                xQuery->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, css::uno::makeAny( m_bEscapeProcessing ) );

                xQuery->setPropertyValue( PROPERTY_LAYOUTINFORMATION, getViewData() );
            }
        }

        if ( bNew )
        {
            Reference< XAppend > xAppend( xElements, UNO_QUERY );
            if ( xAppend.is() )
            {
                xAppend->appendByDescriptor( xQuery );
            }
            else
            {
                Reference< XNameContainer > xCont( xElements, UNO_QUERY );
                if ( xCont.is() )
                    xCont->insertByName( m_sName, makeAny( xQuery ) );
            }

            if ( editingView() )
            {
                Reference< XPropertySet > xViewProps;
                if ( xElements->hasByName( m_sName ) )
                    xViewProps.set( xElements->getByName( m_sName ), UNO_QUERY );

                if ( !xViewProps.is() ) // correct name and try again
                    m_sName = ::dbtools::composeTableName( getMetaData(), xQuery, ::dbtools::eInDataManipulation, false, false, false );

                OSL_ENSURE( xElements->hasByName( m_sName ), "OQueryController::doSaveAsDoc: newly created view does not exist!" );

                if ( xElements->hasByName( m_sName ) )
                    m_xAlterView.set( xElements->getByName( m_sName ), UNO_QUERY );

                // now check if our datasource has set a tablefilter and if so, append the new table name to it
                ::dbaui::appendToFilter( getConnection(), m_sName, getORB(), getView() );
            }
            Reference< XTitleChangeListener> xEventListener(impl_getTitleHelper_throw(),UNO_QUERY);
            if ( xEventListener.is() )
            {
                TitleChangedEvent aEvent;
                xEventListener->titleChanged(aEvent);
            }
            releaseNumberForComponent();
        }

        setModified( sal_False );
        bSuccess = true;

    }
    catch(const SQLException&)
    {
        if ( !bNew )
            m_sName = sOriginalName;
        aInfo = SQLExceptionInfo( ::cppu::getCaughtException() );
    }
    catch(const Exception&)
    {
        if ( !bNew )
            m_sName = sOriginalName;
        DBG_UNHANDLED_EXCEPTION();
    }

    showError( aInfo );

    // if we successfully saved a view we were creating, then close the designer
    if ( bSuccess && editingView() && !m_xAlterView.is() )
    {
        closeTask();
    }

    if ( bSuccess && editingView() )
        InvalidateFeature( ID_BROWSER_EDITDOC );

    return bSuccess;
}

namespace {
struct CommentStrip
{
    OUString maComment;
    bool            mbLastOnLine;
    CommentStrip( const OUString& rComment, bool bLastOnLine )
        : maComment( rComment), mbLastOnLine( bLastOnLine) {}
};

}

/** Obtain all comments in a query.

    See also delComment() implementation for OSQLParser::parseTree().
 */
static ::std::vector< CommentStrip > getComment( const OUString& rQuery )
{
    ::std::vector< CommentStrip > aRet;
    // First a quick search if there is any "--" or "//" or "/*", if not then
    // the whole copying loop is pointless.
    if (rQuery.indexOf( "--" ) < 0 && rQuery.indexOf( "//" ) < 0 &&
            rQuery.indexOf( "/*" ) < 0)
        return aRet;

    const sal_Unicode* pCopy = rQuery.getStr();
    const sal_Int32 nQueryLen = rQuery.getLength();
    bool bIsText1  = false;     // "text"
    bool bIsText2  = false;     // 'text'
    bool bComment2 = false;     // /* comment */
    bool bComment  = false;     // -- or // comment
    OUStringBuffer aBuf;
    for (sal_Int32 i=0; i < nQueryLen; ++i)
    {
        if (bComment2)
        {
            aBuf.append( &pCopy[i], 1);
            if ((i+1) < nQueryLen)
            {
                if (pCopy[i]=='*' && pCopy[i+1]=='/')
                {
                    bComment2 = false;
                    aBuf.append( &pCopy[++i], 1);
                    aRet.push_back( CommentStrip( aBuf.makeStringAndClear(), false));
                }
            }
            else
            {
                // comment can't close anymore, actually an error, but..
                aRet.push_back( CommentStrip( aBuf.makeStringAndClear(), false));
            }
            continue;
        }
        if (pCopy[i] == '\n' || i == nQueryLen-1)
        {
            if (bComment)
            {
                if (i == nQueryLen-1 && pCopy[i] != '\n')
                    aBuf.append( &pCopy[i], 1);
                aRet.push_back( CommentStrip( aBuf.makeStringAndClear(), true));
                bComment = false;
            }
            else if (!aRet.empty())
                aRet.back().mbLastOnLine = true;
        }
        else if (!bComment)
        {
            if (pCopy[i] == '\"' && !bIsText2)
                bIsText1 = !bIsText1;
            else if (pCopy[i] == '\'' && !bIsText1)
                bIsText2 = !bIsText2;
            if (!bIsText1 && !bIsText2 && (i+1) < nQueryLen)
            {
                if ((pCopy[i]=='-' && pCopy[i+1]=='-') || (pCopy[i]=='/' && pCopy[i+1]=='/'))
                    bComment = true;
                else if ((pCopy[i]=='/' && pCopy[i+1]=='*'))
                    bComment2 = true;
            }
        }
        if (bComment || bComment2)
            aBuf.append( &pCopy[i], 1);
    }
    return aRet;
}

/** Concat/insert comments that were previously obtained with getComment().

    NOTE: The current parser implementation does not preserve newlines, so all
    comments are always appended to the entire query, also inline comments
    that would need positioning anyway that can't be obtained after
    recomposition. This is ugly but at least allows commented queries while
    preserving the comments _somehow_.
 */
static OUString concatComment( const OUString& rQuery, const ::std::vector< CommentStrip >& rComments )
{
    // No comments => return query.
    if (rComments.empty())
        return rQuery;

    const sal_Unicode* pBeg = rQuery.getStr();
    const sal_Int32 nLen = rQuery.getLength();
    const size_t nComments = rComments.size();
    // Obtaining the needed size once should be faster than reallocating.
    // Also add a blank or linefeed for each comment.
    sal_Int32 nBufSize = nLen + nComments;
    for (::std::vector< CommentStrip >::const_iterator it( rComments.begin()); it != rComments.end(); ++it)
        nBufSize += (*it).maComment.getLength();
    OUStringBuffer aBuf( nBufSize );
    sal_Int32 nIndBeg = 0;
    sal_Int32 nIndLF = rQuery.indexOf('\n');
    size_t i = 0;
    while (nIndLF >= 0 && i < nComments)
    {
        aBuf.append( pBeg + nIndBeg, nIndLF - nIndBeg);
        do
        {
            aBuf.append( rComments[i].maComment);
        } while (!rComments[i++].mbLastOnLine && i < nComments);
        aBuf.append( pBeg + nIndLF, 1);     // the LF
        nIndBeg = nIndLF + 1;
        nIndLF = (nIndBeg < nLen ? rQuery.indexOf( '\n', nIndBeg) : -1);
    }
    // Append remainder of query.
    if (nIndBeg < nLen)
        aBuf.append( pBeg + nIndBeg, nLen - nIndBeg);
    // Append all remaining comments, preserve lines.
    bool bNewLine = false;
    for ( ; i < nComments; ++i)
    {
        if (!bNewLine)
            aBuf.append( ' ');
        aBuf.append( rComments[i].maComment);
        if (rComments[i].mbLastOnLine)
        {
            aBuf.append( '\n');
            bNewLine = true;
        }
        else
            bNewLine = false;
    }
    return aBuf.makeStringAndClear();
}

OUString OQueryController::translateStatement( bool _bFireStatementChange )
{
    // now set the properties
    setStatement_fireEvent( getContainer()->getStatement(), _bFireStatementChange );
    OUString sTranslatedStmt;
    if(!m_sStatement.isEmpty() && m_xComposer.is() && m_bEscapeProcessing)
    {
        try
        {
            OUString aErrorMsg;

            ::std::vector< CommentStrip > aComments = getComment( m_sStatement);

            ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree( aErrorMsg, m_sStatement, m_bGraphicalDesign );
            if(pNode)
            {
                pNode->parseNodeToStr( sTranslatedStmt, getConnection() );
                delete pNode;
            }

            m_xComposer->setQuery(sTranslatedStmt);
            sTranslatedStmt = m_xComposer->getComposedQuery();
            sTranslatedStmt = concatComment( sTranslatedStmt, aComments);
        }
        catch(const SQLException& e)
        {
            ::dbtools::SQLExceptionInfo aInfo(e);
            showError(aInfo);
            // an error occurred so we clear the statement
            sTranslatedStmt.clear();
        }
    }
    else if(m_sStatement.isEmpty())
    {
        ModuleRes aModuleRes(STR_QRY_NOSELECT);
        OUString sTmpStr(aModuleRes);
        OUString sError(sTmpStr);
        showError(SQLException(sError,nullptr,"S1000",1000,Any()));
    }
    else
        sTranslatedStmt = m_sStatement;

    return sTranslatedStmt;
}

short OQueryController::saveModified()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    short nRet = RET_YES;
    if ( !isConnected() || !isModified() )
        return nRet;

    if  (  !m_bGraphicalDesign
        || (  !m_vTableFieldDesc.empty()
           && !m_vTableData.empty()
           )
        )
    {
        OUString sMessageText( lcl_getObjectResourceString( STR_QUERY_SAVEMODIFIED, m_nCommandType ) );
        ScopedVclPtrInstance< QueryBox > aQry( getView(), WB_YES_NO_CANCEL | WB_DEF_YES, sMessageText );

        nRet = aQry->Execute();
        if  (   ( nRet == RET_YES )
            &&  !doSaveAsDoc( false )
            )
        {
            nRet = RET_CANCEL;
        }
    }
    return nRet;
}

void OQueryController::impl_reset( const bool i_bForceCurrentControllerSettings )
{
    bool bValid = false;

    Sequence< PropertyValue > aLayoutInformation;
    // get command from the query if a query name was supplied
    if ( !i_bForceCurrentControllerSettings && !editingCommand() )
    {
        if ( !m_sName.isEmpty() )
        {
            Reference< XNameAccess > xQueries = getObjectContainer();
            if ( xQueries.is() )
            {
                Reference< XPropertySet > xProp;
                if( xQueries->hasByName( m_sName ) && ( xQueries->getByName( m_sName ) >>= xProp ) && xProp.is() )
                {
                    OUString sNewStatement;
                    xProp->getPropertyValue( PROPERTY_COMMAND ) >>= sNewStatement;
                    setStatement_fireEvent( sNewStatement );

                    if ( editingQuery() )
                    {
                        bool bNewEscapeProcessing( true );
                        xProp->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bNewEscapeProcessing;
                        setEscapeProcessing_fireEvent( bNewEscapeProcessing );
                    }

                    m_bGraphicalDesign = m_bGraphicalDesign && m_bEscapeProcessing;
                    bValid = true;

                    try
                    {
                        if ( editingQuery() )
                            xProp->getPropertyValue( PROPERTY_LAYOUTINFORMATION ) >>= aLayoutInformation;
                    }
                    catch( const Exception& )
                    {
                        OSL_FAIL( "OQueryController::impl_reset: could not retrieve the layout information from the query!" );
                    }
                }
            }
        }
    }
    else
    {
        bValid = true;
        // assume that we got all necessary information during initialization
    }

    if ( bValid )
    {
        // load the layoutInformation
        if ( aLayoutInformation.getLength() )
        {
            try
            {
                loadViewSettings( aLayoutInformation );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if ( !m_sStatement.isEmpty() )
        {
            setQueryComposer();

            bool bError( false );

            if ( !m_pSqlIterator )
            {
                bError = true;
            }
            else if ( m_bEscapeProcessing )
            {
                OUString aErrorMsg;
                ::std::unique_ptr< ::connectivity::OSQLParseNode > pNode(
                    m_aSqlParser.parseTree( aErrorMsg, m_sStatement, m_bGraphicalDesign ) );

                if ( pNode.get() )
                {
                    delete m_pSqlIterator->getParseTree();
                    m_pSqlIterator->setParseTree( pNode.release() );
                    m_pSqlIterator->traverseAll();
                    if ( m_pSqlIterator->hasErrors() )
                    {
                        if ( !i_bForceCurrentControllerSettings && m_bGraphicalDesign && !editingView() )
                        {
                            impl_showAutoSQLViewError( makeAny( m_pSqlIterator->getErrors() ) );
                        }
                        bError = true;
                    }
                }
                else
                {
                    if ( !i_bForceCurrentControllerSettings && !editingView() )
                    {
                        OUString aTitle(ModuleRes(STR_SVT_SQL_SYNTAX_ERROR));
                        ScopedVclPtrInstance< OSQLMessageBox > aDlg(getView(),aTitle,aErrorMsg);
                        aDlg->Execute();
                    }
                    bError = true;
                }
            }

            if ( bError )
            {
                m_bGraphicalDesign = false;
                if ( editingView() )
                    // if we're editing a view whose statement could not be parsed, default to "no escape processing"
                    setEscapeProcessing_fireEvent( false );
            }
        }
    }

    if(!m_pSqlIterator)
        setQueryComposer();
    OSL_ENSURE(m_pSqlIterator,"No SQLIterator set!");

    getContainer()->setNoneVisbleRow(m_nVisibleRows);
}

void OQueryController::reset()
{
    impl_reset();
    getContainer()->reset( nullptr );
    ClearUndoManager();
}

void OQueryController::setStatement_fireEvent( const OUString& _rNewStatement, bool _bFireStatementChange )
{
    Any aOldValue = makeAny( m_sStatement );
    m_sStatement = _rNewStatement;
    Any aNewValue = makeAny( m_sStatement );

    sal_Int32 nHandle = PROPERTY_ID_ACTIVECOMMAND;
    if ( _bFireStatementChange )
        fire( &nHandle, &aNewValue, &aOldValue, 1, sal_False );
}

void OQueryController::setEscapeProcessing_fireEvent( const bool _bEscapeProcessing )
{
    if ( _bEscapeProcessing == m_bEscapeProcessing )
        return;

    Any aOldValue = makeAny( m_bEscapeProcessing );
    m_bEscapeProcessing = _bEscapeProcessing;
    Any aNewValue = makeAny( m_bEscapeProcessing );

    sal_Int32 nHandle = PROPERTY_ID_ESCAPE_PROCESSING;
    fire( &nHandle, &aNewValue, &aOldValue, 1, sal_False );
}

IMPL_LINK_NOARG_TYPED( OQueryController, OnExecuteAddTable, void*, void )
{
    Execute( ID_BROWSER_ADDTABLE,Sequence<PropertyValue>() );
}

bool OQueryController::allowViews() const
{
    return true;
}

bool OQueryController::allowQueries() const
{
    OSL_ENSURE( getSdbMetaData().isConnected(), "OQueryController::allowQueries: illegal call!" );
    if ( !getSdbMetaData().supportsSubqueriesInFrom() )
        return false;

    const NamedValueCollection& rArguments( getInitParams() );
    sal_Int32 nCommandType = rArguments.getOrDefault( PROPERTY_COMMAND_TYPE, (sal_Int32)CommandType::QUERY );
    bool bCreatingView = ( nCommandType == CommandType::TABLE );
    return !bCreatingView;
}

Any SAL_CALL OQueryController::getViewData() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );

    getContainer()->SaveUIConfig();

    ::comphelper::NamedValueCollection aViewSettings;
    saveViewSettings( aViewSettings, false );

    return makeAny( aViewSettings.getPropertyValues() );
}

void SAL_CALL OQueryController::restoreViewData(const Any& /*Data*/) throw( RuntimeException, std::exception )
{
    // TODO
}

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
