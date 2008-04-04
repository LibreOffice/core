/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: querycontroller.cxx,v $
 *
 *  $Revision: 1.116 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:03:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "adtabdlg.hxx"
#include "browserids.hxx"
#include "dbu_qry.hrc"
#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "defaultobjectnamecheck.hxx"
#include "dlgsave.hxx"
#include "localresaccess.hxx"
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

/** === begin UNO includes === **/
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
/** === end UNO includes === **/

#include <comphelper/basicio.hxx>
#include <comphelper/extract.hxx>
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
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

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
        //------------------------------------------------------------------------------
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( RuntimeException )
        {
            return getImplementationName_Static();
        }
        //-------------------------------------------------------------------------
        virtual Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(RuntimeException)
        {
            return getSupportedServiceNames_Static();
        }
    public:
        OViewController(const Reference< XMultiServiceFactory >& _rM) : OQueryController(_rM){}

        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( RuntimeException )
        {
            return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OViewDesign");
        }
        static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( RuntimeException )
        {
            Sequence< ::rtl::OUString> aSupported(1);
            aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ViewDesign");
            return aSupported;
        }
        static Reference< XInterface > SAL_CALL Create(const Reference< XMultiServiceFactory >& _rM)
        {
            return *(new OViewController(_rM));
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
        // -----------------------------------------------------------------------------
        void insertParseTree(SvTreeListBox* _pBox,::connectivity::OSQLParseNode* _pNode,SvLBoxEntry* _pParent = NULL)
        {
            ::rtl::OUString rString;
            if (!_pNode->isToken())
            {
                // Regelnamen als rule: ...
                rString = ::rtl::OUString::createFromAscii("RULE_ID: ");
                rString += ::rtl::OUString::valueOf( (sal_Int32)_pNode->getRuleID());
                rString+= ::rtl::OUString::createFromAscii("(");
                rString += OSQLParser::RuleIDToStr(_pNode->getRuleID());
                rString+= ::rtl::OUString::createFromAscii(")");


                _pParent = _pBox->InsertEntry(rString,_pParent);

                // einmal auswerten wieviel Subtrees dieser Knoten besitzt
                sal_uInt32 nStop = _pNode->count();
                // hol dir den ersten Subtree
                for(sal_uInt32 i=0;i<nStop;++i)
                    insertParseTree(_pBox,_pNode->getChild(i),_pParent);
            }
            else
            {
                // ein Token gefunden
                // tabs fuer das Einruecken entsprechend nLevel

                switch (_pNode->getNodeType())
                {

                case SQL_NODE_KEYWORD:
                    {
                        rString+= ::rtl::OUString::createFromAscii("SQL_KEYWORD:");
                        ::rtl::OString sT = OSQLParser::TokenIDToStr(_pNode->getTokenID());
                        rString += ::rtl::OUString(sT,sT.getLength(),RTL_TEXTENCODING_UTF8);
                     break;}

                case SQL_NODE_COMPARISON:
                    {rString+= ::rtl::OUString::createFromAscii("SQL_COMPARISON:");
                    rString += _pNode->getTokenValue(); // haenge Nodevalue an
                            // und beginne neu Zeile
                    break;}

                case SQL_NODE_NAME:
                    {rString+= ::rtl::OUString::createFromAscii("SQL_NAME:");
                     rString+= ::rtl::OUString::createFromAscii("\"");
                     rString += _pNode->getTokenValue();
                     rString+= ::rtl::OUString::createFromAscii("\"");

                     break;}

                case SQL_NODE_STRING:
                    {rString += ::rtl::OUString::createFromAscii("SQL_STRING:'");
                     rString += _pNode->getTokenValue();
                     break;}

                case SQL_NODE_INTNUM:
                    {rString += ::rtl::OUString::createFromAscii("SQL_INTNUM:");
                     rString += _pNode->getTokenValue();
                     break;}

                case SQL_NODE_APPROXNUM:
                    {rString += ::rtl::OUString::createFromAscii("SQL_APPROXNUM:");
                     rString += _pNode->getTokenValue();
                     break;}

                case SQL_NODE_PUNCTUATION:
                    {rString += ::rtl::OUString::createFromAscii("SQL_PUNCTUATION:");
                    rString += _pNode->getTokenValue(); // haenge Nodevalue an
                    break;}

                case SQL_NODE_AMMSC:
                    {rString += ::rtl::OUString::createFromAscii("SQL_AMMSC:");
                    rString += _pNode->getTokenValue(); // haenge Nodevalue an

                    break;}

                default:
                    OSL_ASSERT("OSQLParser::ShowParseTree: unzulaessiger NodeType");
                    rString += _pNode->getTokenValue();
                }
                _pBox->InsertEntry(rString,_pParent);
            }
        }
    }
#endif // OSL_DEBUG_LEVEL

    namespace
    {
        // -----------------------------------------------------------------------------
        String lcl_getObjectResourceString( USHORT _nResId, sal_Int32 _nCommandType )
        {
            String sMessageText = String( ModuleRes( _nResId ) );
            String sObjectType;
            {
                LocalResourceAccess aLocalRes( RSC_QUERY_OBJECT_TYPE, RSC_RESOURCE );
                sObjectType = String( ModuleRes( (USHORT)( _nCommandType + 1 ) ) );
            }
            sMessageText.SearchAndReplace( String::CreateFromAscii( "$object$" ), sObjectType );
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
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::awt;
using namespace ::dbtools;

using namespace ::comphelper;

namespace
{
    void ensureToolbars( OQueryController* _pController, sal_Bool _bDesign )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager = _pController->getLayoutManager(_pController->getFrame());
        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            static ::rtl::OUString s_sDesignToolbar(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/designobjectbar"));
            static ::rtl::OUString s_sSqlToolbar(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/sqlobjectbar"));
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

    void switchDesignModeImpl(OQueryController* _pController,OQueryContainerWindow* _pWindow,sal_Bool& _rbDesign)
    {
        bool isModified = _pController->isModified();

        if ( !_pWindow->switchView() )
        {
            _rbDesign = !_rbDesign;
            _pWindow->switchView();
        }
        else
        {
            ensureToolbars( _pController, _rbDesign );
        }

        _pController->setModified( isModified );
    }
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OQueryController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OQueryDesign");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OQueryController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.QueryDesign");
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OQueryController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OQueryController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OQueryController(_rxFactory));
}
DBG_NAME(OQueryController);
// -----------------------------------------------------------------------------
OQueryController::OQueryController(const Reference< XMultiServiceFactory >& _rM)
    :OJoinController(_rM)
    ,OQueryController_PBase( getBroadcastHelper() )
    ,m_pParseContext( new svxform::OSystemParseContext )
    ,m_aSqlParser( _rM, m_pParseContext )
    ,m_pSqlIterator(NULL)
    ,m_nVisibleRows(0x400)
    ,m_nSplitPos(-1)
    ,m_nCommandType( CommandType::QUERY )
    ,m_bGraphicalDesign(sal_False)
    ,m_bDistinct(sal_False)
    ,m_bViewAlias(sal_False)
    ,m_bViewTable(sal_False)
    ,m_bViewFunction(sal_False)
    ,m_bEscapeProcessing(sal_True)
{
    DBG_CTOR(OQueryController,NULL);
    InvalidateAll();

    registerProperty( PROPERTY_ACTIVECOMMAND, PROPERTY_ID_ACTIVECOMMAND, PropertyAttribute::READONLY | PropertyAttribute::BOUND,
        &m_sStatement, ::getCppuType( &m_sStatement ) );
    registerProperty( PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::READONLY | PropertyAttribute::BOUND,
        &m_bEscapeProcessing, ::getCppuType( &m_bEscapeProcessing ) );
}

// -----------------------------------------------------------------------------
OQueryController::~OQueryController()
{
    DBG_DTOR(OQueryController,NULL);
    if ( !getBroadcastHelper().bDisposed && !getBroadcastHelper().bInDispose )
    {
        OSL_ENSURE(0,"Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

IMPLEMENT_FORWARD_XINTERFACE2( OQueryController, OJoinController, OQueryController_PBase )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( OQueryController, OJoinController, OQueryController_PBase )

//-------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OQueryController::getPropertySetInfo() throw(RuntimeException)
{
    Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OQueryController::getInfoHelper()
{
    return *const_cast< OQueryController* >( this )->getArrayHelper();
}

//--------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OQueryController::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -----------------------------------------------------------------------------
void OQueryController::deleteIterator()
{
    if(m_pSqlIterator)
    {
        delete m_pSqlIterator->getParseTree();
        m_pSqlIterator->dispose();
        delete m_pSqlIterator;
        m_pSqlIterator = NULL;
    }
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OQueryController::clearFields()
{
    OTableFields().swap(m_vTableFieldDesc);
}
// -----------------------------------------------------------------------------
FeatureState OQueryController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = sal_True;
        // (disabled automatically)

    switch (_nId)
    {
        case ID_BROWSER_EDITDOC:
            if ( editingCommand() )
                aReturn.bEnabled = sal_False;
            else if ( editingView() && !m_xAlterView.is() )
                aReturn.bEnabled = sal_False;
            else
                aReturn = OJoinController::GetState( _nId );
            break;

        case ID_BROWSER_ESACPEPROCESSING:
            aReturn.bChecked = !m_bEscapeProcessing;
            aReturn.bEnabled = ( m_pSqlIterator != NULL ) && !m_bGraphicalDesign;
            break;
        case SID_RELATION_ADD_RELATION:
            aReturn.bEnabled = isEditable() && m_bGraphicalDesign && m_vTableData.size() > 1;
            break;
        case ID_BROWSER_SAVEASDOC:
            aReturn.bEnabled = !editingCommand() && !editingView() && (!m_bGraphicalDesign || !(m_vTableFieldDesc.empty() || m_vTableData.empty()));
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = isModified() && (!m_bGraphicalDesign || !(m_vTableFieldDesc.empty() || m_vTableData.empty()));
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
            aReturn.bEnabled = isEditable() && (m_sStatement.getLength() || !m_vTableData.empty());
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
        case ID_BROWSER_QUERY_EXECUTE:
            aReturn.bEnabled = sal_True;
            break;
        case SID_DB_QUERY_PREVIEW:
            aReturn.bEnabled = sal_True;
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
                aReturn.bEnabled = sal_False;
                break;
            }
            // run through
        default:
            aReturn = OJoinController::GetState(_nId);
            break;
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OQueryController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    switch(_nId)
    {
        case ID_BROWSER_ESACPEPROCESSING:
            setEscapeProcessing_fireEvent( !m_bEscapeProcessing );
            if ( !editingView() )
                setModified(sal_True);
            InvalidateFeature(ID_BROWSER_SQL);
            break;
        case ID_BROWSER_SAVEASDOC:
        case ID_BROWSER_SAVEDOC:
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
            if ( !getContainer()->checkStatement() )
                break;
            try
            {
                ::rtl::OUString aErrorMsg;
                setStatement_fireEvent( getContainer()->getStatement() );
                if(!m_sStatement.getLength() && m_pSqlIterator)
                {
                    // change the view of the data
                    delete m_pSqlIterator->getParseTree();
                    m_pSqlIterator->setParseTree(NULL);
                    m_bGraphicalDesign = !m_bGraphicalDesign;
                    switchDesignModeImpl(this,getContainer(),m_bGraphicalDesign);
                }
                else
                {
                    ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree(aErrorMsg,m_sStatement,m_bGraphicalDesign);
                    //  m_pParseNode = pNode;
                    if(pNode)
                    {
                        delete m_pSqlIterator->getParseTree();
                        m_pSqlIterator->setParseTree(pNode);
                        m_pSqlIterator->traverseAll();
                        if ( m_pSqlIterator->hasErrors() )
                        {
                            showError( SQLExceptionInfo( m_pSqlIterator->getErrors() ) );
                        }
                        else
                        {
                            const OSQLTables& xTabs = m_pSqlIterator->getTables();
                            if( m_pSqlIterator->getStatementType() != SQL_STATEMENT_SELECT || xTabs.begin() == xTabs.end())
                            {
                                ModuleRes aModuleRes(STR_QRY_NOSELECT);
                                String sTmpStr(aModuleRes);
                                ::rtl::OUString sError(sTmpStr);
                                showError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")),1000,Any()));
                            }
                            else
                            {
                                // change the view of the data
                                m_bGraphicalDesign = !m_bGraphicalDesign;
                                ::rtl::OUString sNewStatement;
                                pNode->parseNodeToStr( sNewStatement, getConnection() );
                                setStatement_fireEvent( sNewStatement );
                                getContainer()->SaveUIConfig();
                                m_vTableConnectionData.clear();
                                switchDesignModeImpl(this,getContainer(),m_bGraphicalDesign);
                            }
                        }
                    }
                    else
                    {
                        ModuleRes aModuleRes(STR_QRY_SYNTAX);
                        String sTmpStr(aModuleRes);
                        ::rtl::OUString sError(sTmpStr);
                        showError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000") ),1000,Any()));
                    }
                }
            }
            catch(SQLException& e)
            {
                ::dbtools::SQLExceptionInfo aInfo(e);
                showError(aInfo);
            }
            catch(Exception&)
            {
            }
            if(m_bGraphicalDesign)
            {
                InvalidateFeature(ID_BROWSER_ADDTABLE);
                InvalidateFeature(SID_RELATION_ADD_RELATION);
            }
            break;
        case SID_BROWSER_CLEAR_QUERY:
            {
                getUndoMgr()->EnterListAction( String( ModuleRes(STR_QUERY_UNDO_TABWINDELETE) ), String() );
                getContainer()->clear();
                getUndoMgr()->LeaveListAction();

                setStatement_fireEvent( ::rtl::OUString() );
                if(m_bGraphicalDesign)
                    InvalidateFeature(ID_BROWSER_ADDTABLE);
            }
            //  InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
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
        case ID_BROWSER_QUERY_EXECUTE:
            if ( getContainer()->checkStatement() )
                executeQuery();
            break;
        case SID_DB_QUERY_PREVIEW:
            try
            {
                Reference< ::com::sun::star::util::XCloseable > xCloseFrame( getContainer()->getPreviewFrame(), UNO_QUERY );
                if ( xCloseFrame.is() )
                {
                    try
                    {
                        xCloseFrame->close( sal_True );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "OQueryController::Execute(SID_DB_QUERY_PREVIEW): *nobody* is expected to veto closing the preview frame!" );
                    }
                }
                else
                    Execute(ID_BROWSER_QUERY_EXECUTE,Sequence< PropertyValue >());
            }
            catch(Exception&)
            {
            }
            break;
        case ID_QUERY_ZOOM_IN:
            {
//              m_aZoom *= Fraction(1,10);
//              static_cast<OQueryViewSwitch*>(getView())->zoomTableView(m_aZoom);
            }
            break;
        case ID_QUERY_ZOOM_OUT:
            {
//              if(m_aZoom != Fraction(1,1))
//                  m_aZoom /= Fraction(1,10);
//              static_cast<OQueryViewSwitch*>(getView())->zoomTableView(m_aZoom);
            }
            break;
#if OSL_DEBUG_LEVEL > 1
        case ID_EDIT_QUERY_DESIGN:
        case ID_EDIT_QUERY_SQL:
            {
                ::rtl::OUString aErrorMsg;
                setStatement_fireEvent( getContainer()->getStatement() );
                ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree( aErrorMsg, m_sStatement, m_bGraphicalDesign );
                if ( pNode )
                {
                    Window* pView = getView();
                    ModalDialog* pWindow = new ModalDialog( pView, WB_STDMODAL | WB_SIZEMOVE | WB_CENTER );
                    pWindow->SetSizePixel( ::Size( pView->GetSizePixel().Width() / 2, pView->GetSizePixel().Height() / 2 ) );
                    SvTreeListBox* pTreeBox = new SvTreeListBox( pWindow, WB_BORDER | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_VSCROLL );
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
                            }
                        }
                    }

                    insertParseTree(pTreeBox,pNode);

                    pTreeBox->Show();
                    pWindow->Execute();

                    delete pTreeBox;
                    delete pWindow;
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
// -----------------------------------------------------------------------------
void OQueryController::impl_initialize()
{
    OJoinController::impl_initialize();

    const NamedValueCollection& rArguments( getInitParams() );

    ::rtl::OUString sCommand;
    m_nCommandType = CommandType::QUERY;

    // legacy parameters first (later overwritten by regular parameters)
    ::rtl::OUString sIndependentSQLCommand;
    if ( rArguments.get_ensureType( "IndependentSQLCommand", sIndependentSQLCommand ) )
    {
        OSL_ENSURE( false, "OQueryController::impl_initialize: IndependentSQLCommand is regognized for compatibility only!" );
        sCommand = sIndependentSQLCommand;
        m_nCommandType = CommandType::COMMAND;
    }

    ::rtl::OUString sCurrentQuery;
    if ( rArguments.get_ensureType( "CurrentQuery", sCurrentQuery ) )
    {
        OSL_ENSURE( false, "OQueryController::impl_initialize: CurrentQuery is regognized for compatibility only!" );
        sCommand = sCurrentQuery;
        m_nCommandType = CommandType::QUERY;
    }

    sal_Bool bCreateView( sal_False );
    if ( rArguments.get_ensureType( "CreateView", bCreateView ) && bCreateView )
    {
        OSL_ENSURE( false, "OQueryController::impl_initialize: CurrentQuery is regognized for compatibility only!" );
        m_nCommandType = CommandType::TABLE;
    }

    // non-legacy parameters which overwrite the legacy parameters
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_COMMAND, sCommand );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_COMMAND_TYPE, m_nCommandType );

    // translate Command/Type into proper members
    // TODO/Later: all this (including those members) should be hidden behind some abstact interface,
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
        m_sName = ::rtl::OUString();
        break;
    default:
        OSL_ENSURE( false, "OQueryController::impl_initialize: logic error in code!" );
        throw RuntimeException();
    }

    // more legacy parameters
    sal_Bool bGraphicalDesign( sal_True );
    if ( rArguments.get_ensureType( (::rtl::OUString)PROPERTY_QUERYDESIGNVIEW, bGraphicalDesign ) )
    {
        OSL_ENSURE( false, "OQueryController::impl_initialize: QueryDesignView is regognized for compatibility only!" );
        m_bGraphicalDesign = bGraphicalDesign;
    }

    // more non-legacy
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_GRAPHICAL_DESIGN, m_bGraphicalDesign );

    bool bEscapeProcessing( sal_True );
    if ( rArguments.get_ensureType( (::rtl::OUString)PROPERTY_ESCAPE_PROCESSING, bEscapeProcessing ) )
    {
        setEscapeProcessing_fireEvent( bEscapeProcessing );

        OSL_ENSURE( m_bEscapeProcessing || !m_bGraphicalDesign, "OQueryController::impl_initialize: can't do the graphical design without escape processing!" );
        if ( !m_bEscapeProcessing )
            m_bGraphicalDesign = false;
    }

    if ( !ensureConnected( sal_False ) )
    {   // we have no connection so what else should we do
        m_bGraphicalDesign = sal_False;
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
            sal_Bool bClose = sal_False;
            {
                String aTitle( ModuleRes( STR_QUERYDESIGN_NO_VIEW_SUPPORT ) );
                String aMessage( ModuleRes( STR_QUERYDESIGN_NO_VIEW_ASK ) );
                ODataView* pWindow = getView();
                OSQLMessageBox aDlg( pWindow, aTitle, aMessage, WB_YES_NO | WB_DEF_YES, OSQLMessageBox::Query );
                bClose = aDlg.Execute() == RET_NO;
            }
            if ( bClose )
                throw VetoException();
        }

        // now if we are to edit an existing view, check whether this is possible
        if ( m_sName.getLength() )
        {
            Any aView( xViews->getByName( m_sName ) );
                // will throw if there is no such view
            if ( !( aView >>= m_xAlterView ) )
            {
                throw IllegalArgumentException(
                    ::rtl::OUString( String( ModuleRes( STR_NO_ALTER_VIEW_SUPPORT ) ) ),
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
        resetImpl();
        switchDesignModeImpl(this,getContainer(),m_bGraphicalDesign);
        getUndoMgr()->Clear();

        if  (  ( m_bGraphicalDesign )
            && (  ( !m_sName.getLength() && !editingCommand() )
               || ( !m_sStatement.getLength() && editingCommand() )
               )
            )
        {
            Application::PostUserEvent( LINK( this, OQueryController, OnExecuteAddTable ) );
        }

        setModified(sal_False);
    }
    catch(SQLException& e)
    {
        OSL_ENSURE(sal_False, "OQueryController::impl_initialize: caught an exception!");
        // we caught an exception so we switch to text only mode
        {
            m_bGraphicalDesign = sal_False;
            getContainer()->initialize();
            ODataView* pWindow = getView();
            OSQLMessageBox(pWindow,e).Execute();
        }
        throw;
    }
}

// -----------------------------------------------------------------------------
void OQueryController::onLoadedMenu(const Reference< ::com::sun::star::frame::XLayoutManager >& /*_xLayoutManager*/)
{
    ensureToolbars( this, m_bGraphicalDesign );
}

// -----------------------------------------------------------------------------
::rtl::OUString OQueryController::getPrivateTitle( ) const
{
    ::rtl::OUString sName = m_sName;
    if ( !sName.getLength() )
    {
        if ( !editingCommand() )
        {
            ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
            ::osl::MutexGuard aGuard(m_aMutex);
            String aDefaultName = String( ModuleRes( editingView() ? STR_VIEW_TITLE : STR_QRY_TITLE ) );
            sName = aDefaultName.GetToken(0,' ');
            sName += ::rtl::OUString::valueOf(getCurrentStartNumber());
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
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
            catch (Exception&)
            {
                m_xComposer = NULL;
            }
            OSL_ENSURE(m_xComposer.is(),"No querycomposer available!");
            Reference<XTablesSupplier> xTablesSup(getConnection(), UNO_QUERY);
            deleteIterator();
            m_pSqlIterator = new ::connectivity::OSQLParseTreeIterator( getConnection(), xTablesSup->getTables(), m_aSqlParser, NULL );
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OQueryController::Construct(Window* pParent)
{
    // TODO: we have to check if we should create the text- or the design- view

    m_pView = new OQueryContainerWindow(pParent,this,getORB());

    return OJoinController::Construct(pParent);
}

// -----------------------------------------------------------------------------
OJoinDesignView* OQueryController::getJoinView()
{
    return getContainer()->getDesignView();
}
// -----------------------------------------------------------------------------
void OQueryController::describeSupportedFeatures()
{
    OJoinController::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:SaveAs",            ID_BROWSER_SAVEASDOC,       CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SbaNativeSql",      ID_BROWSER_ESACPEPROCESSING,CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:DBViewFunctions",   SID_QUERY_VIEW_FUNCTIONS,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewTableNames",  SID_QUERY_VIEW_TABLES,      CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewAliases",     SID_QUERY_VIEW_ALIASES,     CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBDistinctValues",  SID_QUERY_DISTINCT_VALUES,  CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:DBChangeDesignMode",ID_BROWSER_SQL,             CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBClearQuery",      SID_BROWSER_CLEAR_QUERY,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:SbaExecuteSql",     ID_BROWSER_QUERY_EXECUTE,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBAddRelation",     SID_RELATION_ADD_RELATION,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBQueryPreview",    SID_DB_QUERY_PREVIEW,       CommandGroup::VIEW );

#if OSL_DEBUG_LEVEL > 1
    implDescribeSupportedFeature( ".uno:DBShowParseTree",   ID_EDIT_QUERY_SQL );
    implDescribeSupportedFeature( ".uno:DBMakeDisjunct",    ID_EDIT_QUERY_DESIGN );
#endif
}
// -----------------------------------------------------------------------------
void OQueryController::setModified(sal_Bool _bModified)
{
    OJoinController::setModified(_bModified);
    InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
    InvalidateFeature(ID_BROWSER_SAVEASDOC);
    InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::disposing( const EventObject& Source ) throw(RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    if ( getContainer() && Source.Source.is() )
    {
        if ( Source.Source == m_aCurrentFrame.getFrame() )
        {   // our frame is beeing disposed -> close the preview window (if we have one)
            Reference< XFrame > xPreviewFrame( getContainer()->getPreviewFrame() );
            ::comphelper::disposeComponent( xPreviewFrame );
        }
        else if ( Source.Source == getContainer()->getPreviewFrame() )
        {
            getContainer()->disposingPreview();
        }
    }

    OJoinController::disposing(Source);
}
// -----------------------------------------------------------------------------
void OQueryController::reconnect(sal_Bool _bUI)
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
            m_bGraphicalDesign = sal_False;
            // don't call Execute(SQL) because this changes the sql statement
            switchDesignModeImpl(this,getContainer(),m_bGraphicalDesign);
        }
        InvalidateAll();
    }
}
// -----------------------------------------------------------------------------
void OQueryController::saveViewSettings(Sequence<PropertyValue>& _rViewProps)
{
    OTableFields::const_iterator aFieldIter = m_vTableFieldDesc.begin();
    sal_Int32 nCount = 0;
    for(;aFieldIter != m_vTableFieldDesc.end();++aFieldIter)
    {
        if(!(*aFieldIter)->IsEmpty())
            ++nCount;
    }

    sal_Int32 nLen = _rViewProps.getLength();

    _rViewProps.realloc( nLen + 2 + (nCount != 0 ? 1 : 0) );
    PropertyValue *pIter = _rViewProps.getArray() + nLen;

    if ( nCount != 0 )
    {
        pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fields"));

        Sequence<PropertyValue> aFields(nCount);
        PropertyValue *pFieldsIter = aFields.getArray();
        // the fielddata
        aFieldIter = m_vTableFieldDesc.begin();
        for(sal_Int32 i = 1;aFieldIter != m_vTableFieldDesc.end();++aFieldIter,++i)
        {
            if ( !(*aFieldIter)->IsEmpty() )
            {
                pFieldsIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Field")) + ::rtl::OUString::valueOf(i);
                (*aFieldIter)->Save(*pFieldsIter++);
            }
        }
        pIter->Value <<= aFields;
        ++pIter;
    }

    pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SplitterPosition"));
    pIter->Value <<= m_nSplitPos;
    ++pIter;
    pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleRows"));
    pIter->Value <<= m_nVisibleRows;
}
// -----------------------------------------------------------------------------
void OQueryController::loadViewSettings(const Sequence<PropertyValue>& _rViewProps)
{
    //////////////////////////////////////////////////////////////////////
    // Liste loeschen
    OTableFields().swap(m_vTableFieldDesc);

    const PropertyValue *pIter = _rViewProps.getConstArray();
    const PropertyValue *pEnd = pIter + _rViewProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SplitterPosition")) )
        {
            pIter->Value >>= m_nSplitPos;
        }
        else if ( pIter->Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleRows")) )
        {
            pIter->Value >>= m_nVisibleRows;
        }
        else if ( pIter->Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fields")) )
        {
            Sequence<PropertyValue> aFields;
            pIter->Value >>= aFields;
            m_vTableFieldDesc.reserve(aFields.getLength() + 1);
            const PropertyValue *pFieldIter = aFields.getConstArray();
            const PropertyValue *pFieldEnd = pFieldIter + aFields.getLength();
            for (; pFieldIter != pFieldEnd; ++pFieldIter)
            {
                OTableFieldDescRef pData = new OTableFieldDesc();
                pData->Load(*pFieldIter);
                m_vTableFieldDesc.push_back(pData);
            }
        }
    }
}
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
void OQueryController::executeQuery()
{
    // we don't need to check the connection here because we already check the composer
    // which can't live without his connection
    ::rtl::OUString sTranslatedStmt = translateStatement( false );

    ::rtl::OUString sDataSourceName = getDataSourceName();
    if ( sDataSourceName.getLength() && sTranslatedStmt.getLength() )
    {
        try
        {
            getContainer()->showPreview( getFrame() );
            InvalidateFeature(SID_DB_QUERY_PREVIEW);

            URL aWantToDispatch;
            aWantToDispatch.Complete = ::rtl::OUString::createFromAscii(".component:DB/DataSourceBrowser");

            ::rtl::OUString sFrameName( FRAME_NAME_QUERY_PREVIEW );
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
                Sequence< PropertyValue> aProps(9);
                aProps[0].Name = PROPERTY_DATASOURCENAME;
                aProps[0].Value <<= sDataSourceName;

                aProps[1].Name = PROPERTY_COMMAND_TYPE;
                aProps[1].Value <<= CommandType::COMMAND;

                aProps[2].Name = PROPERTY_COMMAND;
                aProps[2].Value <<= sTranslatedStmt;

                aProps[3].Name = PROPERTY_ENABLE_BROWSER;
                aProps[3].Value = ::cppu::bool2any(sal_False);

                aProps[4].Name = PROPERTY_ACTIVE_CONNECTION;
                aProps[4].Value <<= getConnection();

                aProps[5].Name = PROPERTY_UPDATE_CATALOGNAME;
                aProps[5].Value <<= m_sUpdateCatalogName;

                aProps[6].Name = PROPERTY_UPDATE_SCHEMANAME;
                aProps[6].Value <<= m_sUpdateSchemaName;

                aProps[7].Name = PROPERTY_UPDATE_TABLENAME;
                aProps[7].Value <<= m_sUpdateTableName;

                aProps[8].Name = PROPERTY_ESCAPE_PROCESSING;
                aProps[8].Value = ::cppu::bool2any(m_bEscapeProcessing);

                xDisp->dispatch(aWantToDispatch, aProps);
                // check the state of the beamer
                // be notified when the beamer frame is closed
                Reference< XComponent >  xComponent( getFrame()->findFrame( sFrameName, nSearchFlags ), UNO_QUERY );
                if (xComponent.is())
                {
                    OSL_ENSURE(Reference< XFrame >(xComponent, UNO_QUERY).get() == getContainer()->getPreviewFrame().get(),
                        "OQueryController::executeQuery: oops ... which window do I have here?");
                    Reference< XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
                    xComponent->addEventListener(xEvtL);
                }
            }
            else
            {
                OSL_ENSURE(0,"Couldn't create a beamer window!");
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Couldn't create a beamer window!");
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OQueryController::askForNewName(const Reference<XNameAccess>& _xElements,sal_Bool _bSaveAs)
{
    OSL_ENSURE( !editingCommand(), "OQueryController::askForNewName: not to be called when designing an independent statement!" );
    if ( editingCommand() )
        return sal_False;

    OSL_PRECOND( _xElements.is(), "OQueryController::askForNewName: invalid container!" );
    if  ( !_xElements.is() )
        return sal_False;

    sal_Bool bRet = sal_True;
    sal_Bool bNew = _bSaveAs || !_xElements->hasByName( m_sName );
    if(bNew)
    {
        String aDefaultName;
        if ( ( _bSaveAs && !bNew ) || ( bNew && m_sName.getLength() ) )
            aDefaultName = String( m_sName );
        else
            aDefaultName = getPrivateTitle( );

        DynamicTableOrQueryNameCheck aNameChecker( getConnection(), CommandType::QUERY );
        OSaveAsDlg aDlg(
                getView(),
                m_nCommandType,
                getORB(),
                getConnection(),
                aDefaultName,
                aNameChecker,
                SAD_DEFAULT );

        bRet = ( aDlg.Execute() == RET_OK );
        if ( bRet )
        {
            m_sName = aDlg.getName();
            if ( editingView() )
            {
                m_sUpdateCatalogName    = aDlg.getCatalog();
                m_sUpdateSchemaName     = aDlg.getSchema();
            }
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
bool OQueryController::doSaveAsDoc(sal_Bool _bSaveAs)
{
    OSL_ENSURE(isEditable(),"Slot ID_BROWSER_SAVEDOC should not be enabled!");
    if ( !editingCommand() && !haveDataSource() )
    {
        String aMessage(ModuleRes(STR_DATASOURCE_DELETED));
        String sTitle(ModuleRes(STR_STAT_WARNING));
        OSQLMessageBox aMsg(getView(),sTitle,aMessage);
        aMsg.Execute();
        return false;
    }

    Reference< XNameAccess > xElements = getObjectContainer();
    if ( !xElements.is() )
        return false;

    if ( !getContainer()->checkStatement() )
        return false;

    ::rtl::OUString sTranslatedStmt = translateStatement();
    if ( editingCommand() )
    {
        setModified( sal_False );
        // this is all we need to do here. translateStatement implicitly set our m_sStatement, and
        // notified it, and that's all
        return true;
    }

    if ( !sTranslatedStmt.getLength() )
        return false;

    // first we need a name for our query so ask the user
    // did we get a name
    ::rtl::OUString sOriginalName( m_sName );
    if ( !askForNewName( xElements, _bSaveAs ) || !m_sName.getLength() )
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
                    xQuery = xQuery.query( xSingleFac->createInstance() );
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
                xQuery->setPropertyValue( PROPERTY_ESCAPE_PROCESSING,::cppu::bool2any( m_bEscapeProcessing ) );

                // layout information
                getContainer()->SaveUIConfig();
                Sequence< PropertyValue > aLayout;
                saveTableWindows( aLayout );
                saveViewSettings( aLayout );
                xQuery->setPropertyValue( PROPERTY_LAYOUTINFORMATION, makeAny( aLayout ) );
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

                OSL_ENSURE( xElements->hasByName( m_sName ), "OQueryController::doSaveAsDoc: newly creaed view does not exist!" );

                if ( xElements->hasByName( m_sName ) )
                    m_xAlterView.set( xElements->getByName( m_sName ), UNO_QUERY );

                // now check if our datasource has set a tablefilter and if so, append the new table name to it
                ::dbaui::appendToFilter( getConnection(), m_sName, getORB(), getView() );
            }
        }

        setModified( sal_False );
        bSuccess = true;
    }
    catch( const SQLException& )
    {
        if ( !bNew )
            m_sName = sOriginalName;
        aInfo = SQLExceptionInfo( ::cppu::getCaughtException() );
    }
    catch(Exception&)
    {
        if ( !bNew )
            m_sName = sOriginalName;
        DBG_UNHANDLED_EXCEPTION();
    }

    showError( aInfo );

    // update the title of our window
    //updateTitle();

    // if we successfully saved a view we were creating, then close the designer
    if ( bSuccess && editingView() && !m_xAlterView.is() )
    {
        closeTask();
    }

    if ( bSuccess && editingView() )
        InvalidateFeature( ID_BROWSER_EDITDOC );

    return bSuccess;
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryController::translateStatement( bool _bFireStatementChange )
{
    // now set the properties
    setStatement_fireEvent( getContainer()->getStatement(), _bFireStatementChange );
    ::rtl::OUString sTranslatedStmt;
    if(m_sStatement.getLength() && m_xComposer.is() && m_bEscapeProcessing)
    {
        try
        {
            ::rtl::OUString aErrorMsg;

            ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree( aErrorMsg, m_sStatement, m_bGraphicalDesign );
            if(pNode)
            {
                pNode->parseNodeToStr( sTranslatedStmt, getConnection() );
                delete pNode;
            }

            m_xComposer->setQuery(sTranslatedStmt);
            sTranslatedStmt = m_xComposer->getComposedQuery();
        }
        catch(SQLException& e)
        {
            ::dbtools::SQLExceptionInfo aInfo(e);
            showError(aInfo);
            // an error occured so we clear the statement
            sTranslatedStmt = ::rtl::OUString();
        }
    }
    else if(!m_sStatement.getLength())
    {
        ModuleRes aModuleRes(STR_QRY_NOSELECT);
        String sTmpStr(aModuleRes);
        ::rtl::OUString sError(sTmpStr);
        showError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000") ),1000,Any()));
    }
    else
        sTranslatedStmt = m_sStatement;

    return sTranslatedStmt;
}
// -----------------------------------------------------------------------------
short OQueryController::saveModified()
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    short nRet = RET_YES;
    if ( !isConnected() || !isModified() )
        return nRet;

    if  (  !m_bGraphicalDesign
        || (  !m_vTableFieldDesc.empty()
           && !m_vTableData.empty()
           )
        )
    {
        String sMessageText( lcl_getObjectResourceString( STR_QUERY_SAVEMODIFIED, m_nCommandType ) );
        QueryBox aQry( getView(), WB_YES_NO_CANCEL | WB_DEF_YES, sMessageText );

        nRet = aQry.Execute();
        if  (   ( nRet == RET_YES )
            &&  !doSaveAsDoc( sal_False )
            )
        {
            nRet = RET_CANCEL;
        }
    }
    return nRet;
}
// -----------------------------------------------------------------------------
void OQueryController::resetImpl()
{
    bool bValid = false;
    Sequence< PropertyValue > aLayoutInformation;

    // get command from the query if a query name was supplied
    if ( !editingCommand() )
    {
        if ( m_sName.getLength() )
        {
            Reference< XNameAccess > xQueries = getObjectContainer();
            if ( xQueries.is() )
            {
                Reference< XPropertySet > xProp;
                if( xQueries->hasByName( m_sName ) && ( xQueries->getByName( m_sName ) >>= xProp ) && xProp.is() )
                {
                    ::rtl::OUString sNewStatement;
                    xProp->getPropertyValue( PROPERTY_COMMAND ) >>= sNewStatement;
                    setStatement_fireEvent( sNewStatement );

                    sal_Bool bNewEscapeProcessing( sal_True );
                    if ( editingQuery() )
                    {
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
                        OSL_ENSURE( sal_False, "OQueryController::resetImpl: could not retrieve the layout information from the query!" );
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
                // load the layoutInformation
                loadTableWindows(aLayoutInformation);
                loadViewSettings(aLayoutInformation);
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if ( m_sStatement.getLength() )
        {
            setQueryComposer();

            bool bError( false );
            if ( m_bEscapeProcessing )
            {
                ::rtl::OUString aErrorMsg;
                ::connectivity::OSQLParseNode* pNode = m_aSqlParser.parseTree(aErrorMsg,m_sStatement,m_bGraphicalDesign);
                //  m_pParseNode = pNode;
                if(pNode)
                {
                    if(m_pSqlIterator)
                    {
                        delete m_pSqlIterator->getParseTree();
                        m_pSqlIterator->setParseTree(pNode);
                        m_pSqlIterator->traverseAll();
                        if ( m_pSqlIterator->hasErrors() )
                        {
                            SQLContext aErrorContext;
                            aErrorContext.Message = lcl_getObjectResourceString( STR_ERROR_PARSING_STATEMENT, m_nCommandType );
                            aErrorContext.Context = *this;
                            aErrorContext.Details = lcl_getObjectResourceString( STR_INFO_OPENING_IN_SQL_VIEW, m_nCommandType );
                            aErrorContext.NextException <<= m_pSqlIterator->getErrors();
                            showError( aErrorContext );
                            bError = true;
                        }
                    }
                    else
                    {
                        delete pNode;
                        bError = true;
                    }
                }
                else
                {
                    String aTitle(ModuleRes(STR_SVT_SQL_SYNTAX_ERROR));
                    OSQLMessageBox aDlg(getView(),aTitle,aErrorMsg);
                    aDlg.Execute();
                    bError = true;
                }
            }

            if ( bError )
            {
                m_bGraphicalDesign = sal_False;
                if ( editingView() )
                    // if we're editing a view whose statement could not be parsed, default to "no escape processing"
                    setEscapeProcessing_fireEvent( sal_False );
            }
        }
    }

    if(!m_pSqlIterator)
        setQueryComposer();
    OSL_ENSURE(m_pSqlIterator,"No SQLIterator set!");

    getContainer()->setNoneVisbleRow(m_nVisibleRows);
}

// -----------------------------------------------------------------------------
void OQueryController::reset()
{
    resetImpl();
    getContainer()->reset();
    getUndoMgr()->Clear();
}

// -----------------------------------------------------------------------------
void OQueryController::setStatement_fireEvent( const ::rtl::OUString& _rNewStatement, bool _bFireStatementChange )
{
    Any aOldValue = makeAny( m_sStatement );
    m_sStatement = _rNewStatement;
    Any aNewValue = makeAny( m_sStatement );

    sal_Int32 nHandle = PROPERTY_ID_ACTIVECOMMAND;
    if ( _bFireStatementChange )
        fire( &nHandle, &aNewValue, &aOldValue, 1, sal_False );
}

// -----------------------------------------------------------------------------
void OQueryController::setEscapeProcessing_fireEvent( const sal_Bool _bEscapeProcessing )
{
    if ( _bEscapeProcessing == m_bEscapeProcessing )
        return;

    Any aOldValue = makeAny( m_bEscapeProcessing );
    m_bEscapeProcessing = _bEscapeProcessing;
    Any aNewValue = makeAny( m_bEscapeProcessing );

    sal_Int32 nHandle = PROPERTY_ID_ESCAPE_PROCESSING;
    fire( &nHandle, &aNewValue, &aOldValue, 1, sal_False );
}

// -----------------------------------------------------------------------------
IMPL_LINK( OQueryController, OnExecuteAddTable, void*, /*pNotInterestedIn*/ )
{
    Execute( ID_BROWSER_ADDTABLE,Sequence<PropertyValue>() );
    return 0L;
}

// -----------------------------------------------------------------------------
bool OQueryController::allowViews() const
{
    return true;
}

// -----------------------------------------------------------------------------
bool OQueryController::allowQueries() const
{
    DBG_ASSERT( getSdbMetaData().isConnected(), "OQueryController::allowQueries: illegal call!" );
    if ( !getSdbMetaData().supportsSubqueriesInFrom() )
        return false;

    const NamedValueCollection& rArguments( getInitParams() );
    sal_Int32 nCommandType = rArguments.getOrDefault( (::rtl::OUString)PROPERTY_COMMAND_TYPE, (sal_Int32)CommandType::QUERY );
    sal_Bool bCreatingView = ( nCommandType == CommandType::TABLE );
    return !bCreatingView;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace dbaui
// -----------------------------------------------------------------------------

