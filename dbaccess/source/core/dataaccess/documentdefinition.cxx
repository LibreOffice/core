/*************************************************************************
 *
 *  $RCSfile: documentdefinition.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:10:37 $
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

#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#include "documentdefinition.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_XEMBEDOBJECTFACTORY_HPP_
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDOPENMODEEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#endif
#ifndef _COM_SUN_STAR_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_MISSINGPROPERTIESEXCEPTION_HPP_
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_MISSINGINPUTSTREAMEXCEPTION_HPP_
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef DBA_INTERCEPT_HXX
#include "intercept.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONDOCUMENTSAVE_HPP_
#include <com/sun/star/sdb/XInteractionDocumentSave.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_DOCUMENTSAVEREQUEST_HPP_
#include <com/sun/star/sdb/DocumentSaveRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTATECHANGEBROADCASTER_HPP_
#include <com/sun/star/embed/XStateChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

using namespace ::com::sun::star::view;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::task;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
        typedef ::cppu::WeakComponentImplHelper1<   ::com::sun::star::embed::XStateChangeListener > TEmbedObjectHolder;
        class OEmbedObjectHolder :   public ::comphelper::OBaseMutex
                                    ,public TEmbedObjectHolder
        {
            Reference< XStateChangeBroadcaster > m_xBroadCaster;
            ODocumentDefinition*                 m_pDefinition;
        protected:
            virtual void SAL_CALL disposing();
        public:
            OEmbedObjectHolder(const Reference< XStateChangeBroadcaster >& _xBroadCaster,ODocumentDefinition* _pDefinition)
                : TEmbedObjectHolder(m_aMutex)
                ,m_xBroadCaster(_xBroadCaster)
                ,m_pDefinition(_pDefinition)
            {
                osl_incrementInterlockedCount( &m_refCount );
                if ( m_xBroadCaster.is() )
                    m_xBroadCaster->addStateChangeListener(this);
                osl_decrementInterlockedCount( &m_refCount );
            }

            virtual void SAL_CALL changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        };
        //------------------------------------------------------------------
        void SAL_CALL OEmbedObjectHolder::disposing()
        {
            if ( m_xBroadCaster.is() )
                m_xBroadCaster->removeStateChangeListener(this);
            m_xBroadCaster = NULL;
            m_pDefinition = NULL;
        }
        //------------------------------------------------------------------
        void SAL_CALL OEmbedObjectHolder::changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
        {
        }
        //------------------------------------------------------------------
        void SAL_CALL OEmbedObjectHolder::stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
        {
            if ( nNewState == EmbedStates::RUNNING && nOldState == EmbedStates::ACTIVE && m_pDefinition )
            {
                Reference<XInterface> xInt(static_cast< ::cppu::OWeakObject* >(m_pDefinition),UNO_QUERY);
                {
                    Reference<XEmbeddedObject> xEmbeddedObject(aEvent.Source,UNO_QUERY);
                    if ( xEmbeddedObject.is() )
                        xEmbeddedObject->changeState(EmbedStates::LOADED);
                }
            }
        }
        //------------------------------------------------------------------
        void SAL_CALL OEmbedObjectHolder::disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
        {
            m_xBroadCaster = NULL;
        }
//........................................................................
    typedef ::cppu::WeakImplHelper1<        XEmbeddedClient
                                >   EmbeddedClientHelper_BASE;
    class OEmbeddedClientHelper : public EmbeddedClientHelper_BASE
    {
        ODocumentDefinition* m_pClient;
    public:
        OEmbeddedClientHelper(ODocumentDefinition* _pClient) :m_pClient(_pClient) {}

        virtual void SAL_CALL saveObject(  ) throw (ObjectSaveVetoException, Exception, RuntimeException)
        {
            if ( m_pClient )
                m_pClient->saveObject();
        }
        virtual void SAL_CALL onShowWindow( sal_Bool bVisible ) throw (RuntimeException)
        {
            if ( m_pClient )
                m_pClient->onShowWindow(bVisible);
        }
        // XComponentSupplier
        virtual Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent(  ) throw (RuntimeException)
        {
            Reference< ::com::sun::star::util::XCloseable > xRet;
            if ( m_pClient )
                xRet = m_pClient->getComponent();
            return xRet;
        }

        // XEmbeddedClient
        virtual void SAL_CALL visibilityChanged( ::sal_Bool bVisible ) throw (WrongStateException, RuntimeException)
        {
            if ( m_pClient )
                m_pClient->visibilityChanged( bVisible );
        }
        inline void resetClient(ODocumentDefinition* _pClient) { m_pClient = _pClient; }
    };
    //==================================================================
    // ODocumentSaveContinuation
    //==================================================================
    class ODocumentSaveContinuation : public OInteraction< XInteractionDocumentSave >
    {
        ::rtl::OUString     m_sName;
        Reference<XContent> m_xParentContainer;

    public:
        ODocumentSaveContinuation() { }

        inline Reference<XContent>  getContent() const { return m_xParentContainer; }
        inline ::rtl::OUString      getName() const { return m_sName; }

        // XInteractionDocumentSave
        virtual void SAL_CALL setName( const ::rtl::OUString& _sName,const Reference<XContent>& _xParent) throw(RuntimeException);
    };

    //------------------------------------------------------------------
    void SAL_CALL ODocumentSaveContinuation::setName( const ::rtl::OUString& _sName,const Reference<XContent>& _xParent) throw(RuntimeException)
    {
        m_sName = _sName;
        m_xParentContainer = _xParent;
    }

::rtl::OUString lcl_GetDocumentServiceFromMediaType( const Reference< XStorage >& xStorage
                                                    ,const ::rtl::OUString& sEntName )
{
    ::rtl::OUString sResult;
    try
    {
        if ( xStorage->isStorageElement( sEntName ) )
        {
            // the object must be based on storage

            Reference< XPropertySet > xPropSet( xStorage->openStorageElement( sEntName, ElementModes::READ ), UNO_QUERY );
            if ( !xPropSet.is() )
                throw RuntimeException();

            ::rtl::OUString aMediaType;
            try {
                Any aAny = xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
                aAny >>= aMediaType;
            }
            catch ( Exception& )
            {
            }

            if ( aMediaType.equalsAscii( "application/vnd.sun.xml.writer" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Writer.TextDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.writer.global" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Writer.GlobalDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.writer.web" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Writer.WebDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.calc" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Calc.SpreadsheetDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.impress" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Draw.PresentationDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.draw" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Draw.DrawingDocument" );
            else if ( aMediaType.equalsAscii( "application/vnd.sun.xml.math" ) )
                sResult = ::rtl::OUString::createFromAscii( "com.sun.star.comp.Math.FormulaDocument" );

            try
            {
                ::comphelper::disposeComponent(xPropSet);
            }
            catch ( Exception& )
            {
            }
        }
    }
    catch ( Exception& )
    {
    }
    return sResult;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > lcl_GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                    sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                    sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
{
    Sequence< sal_Int8 > aResult( 16 );
    aResult[0] = static_cast<sal_Int8>(n1 >> 24);
    aResult[1] = static_cast<sal_Int8>(( n1 << 8 ) >> 24);
    aResult[2] = static_cast<sal_Int8>(( n1 << 16 ) >> 24);
    aResult[3] = static_cast<sal_Int8>(( n1 << 24 ) >> 24);
    aResult[4] = n2 >> 8;
    aResult[5] = ( n2 << 8 ) >> 8;
    aResult[6] = n3 >> 8;
    aResult[7] = ( n3 << 8 ) >> 8;
    aResult[8] = b8;
    aResult[9] = b9;
    aResult[10] = b10;
    aResult[11] = b11;
    aResult[12] = b12;
    aResult[13] = b13;
    aResult[14] = b14;
    aResult[15] = b15;

    return aResult;
}
//==========================================================================
//= ODocumentDefinition
//==========================================================================
DBG_NAME(ODocumentDefinition)

//--------------------------------------------------------------------------
ODocumentDefinition::ODocumentDefinition(const Reference< XInterface >& _rxContainer
                                         , const Reference< XMultiServiceFactory >& _xORB
                                         ,const TContentPtr& _pImpl
                                         , sal_Bool _bForm
                                         , const Sequence< sal_Int8 >& _aClassID
                                         ,const Reference<XConnection>& _xConnection
                                         )
                                         :OContentHelper(_xORB,_rxContainer,_pImpl)
    ,OPropertyStateContainer(m_aBHelper)
    ,m_pInterceptor(NULL)
    ,m_pClientHelper(NULL)
    ,m_bForm(_bForm)
{
    DBG_CTOR(ODocumentDefinition, NULL);
    registerProperties();
    if ( _aClassID.getLength() )
        loadEmbeddedObject(_aClassID,_xConnection);
}

//--------------------------------------------------------------------------
ODocumentDefinition::~ODocumentDefinition()
{
    DBG_DTOR(ODocumentDefinition, NULL);
    if ( !m_aBHelper.bInDispose && !m_aBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    if ( m_pInterceptor )
    {
        m_pInterceptor->DisconnectContentHolder();
        m_pInterceptor->release();
        m_pInterceptor = NULL;
    }
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::closeObject()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_xEmbeddedObject.is() )
    {
        try
        {
            Reference< com::sun::star::util::XCloseable> xCloseable(m_xEmbeddedObject,UNO_QUERY);
            if ( xCloseable.is() )
                xCloseable->close(sal_True);
        }
        catch(Exception)
        {
        }
        m_xEmbeddedObject = NULL;
        if ( m_pClientHelper )
            m_pClientHelper->resetClient(NULL);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::disposing()
{
    OContentHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    closeObject();
    ::comphelper::disposeComponent(m_xListener);
    m_xFrameLoader = NULL;
}
// -----------------------------------------------------------------------------
IMPLEMENT_TYPEPROVIDER3(ODocumentDefinition,OContentHelper,OPropertyStateContainer,ODocumentDefinition_Base);
IMPLEMENT_FORWARD_XINTERFACE3( ODocumentDefinition,OContentHelper,OPropertyStateContainer,ODocumentDefinition_Base)
IMPLEMENT_SERVICE_INFO1(ODocumentDefinition,"com.sun.star.comp.dba.ODocumentDefinition",SERVICE_SDB_DOCUMENTDEFINITION)
//--------------------------------------------------------------------------
void ODocumentDefinition::registerProperties()
{
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_pImpl->m_aProps.aTitle, ::getCppuType(&m_pImpl->m_aProps.aTitle));
    registerProperty(PROPERTY_AS_TEMPLATE, PROPERTY_ID_AS_TEMPLATE, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_pImpl->m_aProps.bAsTemplate, ::getCppuType(&m_pImpl->m_aProps.bAsTemplate));
    registerProperty(PROPERTY_PERSISTENT_NAME, PROPERTY_ID_PERSISTENT_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_pImpl->m_aProps.sPersistentName, ::getCppuType(&m_pImpl->m_aProps.sPersistentName));
    registerProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsForm")), PROPERTY_ID_IS_FORM, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_bForm, ::getCppuType(&m_bForm));
}
// -----------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL ODocumentDefinition::getPropertySetInfo(  ) throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//--------------------------------------------------------------------------
IPropertyArrayHelper& ODocumentDefinition::getInfoHelper()
{
    return *getArrayHelper();
}


//--------------------------------------------------------------------------
IPropertyArrayHelper* ODocumentDefinition::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new OPropertyArrayHelper(aProps);
}
// -----------------------------------------------------------------------------
Any SAL_CALL ODocumentDefinition::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment ) throw (Exception, CommandAbortedException, RuntimeException)
{
    Any aRet;
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Bool bOpenInDesign;
    if ( aCommand.Name.compareToAscii( "open" ) == 0 || (bOpenInDesign = aCommand.Name.compareToAscii( "openDesign" ) == 0) )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////
        Reference< XConnection> xConnection;
        OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            sal_Bool bFound = sal_False;
            Sequence< PropertyValue > aSeq;
            if ( aCommand.Argument >>= aSeq )
            {
                const PropertyValue* pIter = aSeq.getConstArray();
                const PropertyValue* pEnd  = pIter + aSeq.getLength();
                for(;pIter != pEnd;++pIter)
                {
                    if ( pIter->Name == PROPERTY_ACTIVECONNECTION )
                        xConnection.set(pIter->Value,UNO_QUERY);
                    else if ( !bFound )
                        bFound = ( pIter->Value >>= aOpenCommand );
                }
            }
            if ( !bFound )
            {
                OSL_ENSURE( sal_False, "Wrong argument type!" );
                ucbhelper::cancelCommandExecution(
                    makeAny( IllegalArgumentException(
                                        rtl::OUString(),
                                        static_cast< cppu::OWeakObject * >( this ),
                                        -1 ) ),
                    Environment );
                // Unreachable
            }
        }
        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
              ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );

        if ( bOpenFolder )
        {
            // open as folder - return result set

            ucbhelper::cancelCommandExecution(
                    makeAny( UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
          }
        else
        {
            // Check open mode
            if ( ( aOpenCommand.Mode
                    == OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                    == OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
            {
                // Unsupported.
                ucbhelper::cancelCommandExecution(
                    makeAny( UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
            }

            Reference<XModel> xModel;
            if ( m_pImpl->m_aProps.sPersistentName.getLength() )
            {
                loadEmbeddedObject(Sequence< sal_Int8 >(),xConnection,!bOpenInDesign);
                if ( m_xEmbeddedObject.is() )
                {
                    m_xEmbeddedObject->changeState(EmbedStates::ACTIVE);

                    // object is new, so we an interceptor for save
                    xModel.set(getComponent(),UNO_QUERY);
                    if ( xModel.is() )
                    {
                        if ( m_xListener.is() )
                        {
                            Reference<XFrame> xFrame = xModel->getCurrentController()->getFrame();
                            if ( xFrame.is() )
                            {
                                Reference<XTopWindow> xTopWindow( xFrame->getContainerWindow(),UNO_QUERY );
                                if( xTopWindow.is() )
                                    xTopWindow->toFront();
                            }
                        }
                        else
                        {
                            if ( !m_xFrameLoader.is() )
                            {
                                m_xFrameLoader.set(m_xORB->createInstance(SERVICE_FRAME_DESKTOP),UNO_QUERY);
                            }
                            // remove the frame from the desktop because we need full control of it.
                            if ( m_xFrameLoader.is() )
                            {
                                Reference<XFrame> xFrame = xModel->getCurrentController()->getFrame();
                                Reference<XFramesSupplier> xSup(m_xFrameLoader,UNO_QUERY);
                                if ( xSup.is() )
                                {
                                    Reference<XFrames> xFrames = xSup->getFrames();
                                    xFrames->remove(xFrame);
                                }
                            }

                            if ( bOpenInDesign && m_bForm )
                            {
                                Reference<XViewSettingsSupplier> xViewSup(xModel->getCurrentController(),UNO_QUERY);
                                if ( xViewSup.is() )
                                {
                                    Reference<XPropertySet> xProp = xViewSup->getViewSettings();
                                    if ( xProp.is() )
                                    {
                                        try
                                        {
                                            xProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsRasterVisible")),makeAny(sal_True));
                                            xProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSnapToRaster")),makeAny(sal_True));
                                            xProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowOnlineLayout")),makeAny(sal_True));
                                        }
                                        catch(Exception&)
                                        {
                                        }
                                    }
                                }
                            }
                            Reference<XStateChangeBroadcaster> xBrd(m_xEmbeddedObject,UNO_QUERY);
                            m_xListener = new OEmbedObjectHolder(xBrd,this);

                            fillReportData(!bOpenInDesign);
                        }
                        aRet <<= xModel;
                    }
                }
            }
        }
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "copyTo" ) ) )
    {
        Sequence<Any> aIni;
        aCommand.Argument >>= aIni;
        if ( aIni.getLength() != 2 )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        Reference< XStorage> xStorage(aIni[0],UNO_QUERY);
        ::rtl::OUString sPersistentName;
        aIni[1] >>= sPersistentName;
        loadEmbeddedObject();
        Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
        if ( xPersist.is() )
        {
            xPersist->storeToEntry(xStorage,sPersistentName,Sequence<PropertyValue>(),Sequence<PropertyValue>());
            xPersist->storeOwn();
//          Reference<XTransactedObject> xTransact(xStorage,UNO_QUERY);
//          if ( xTransact.is() )
//              xTransact->commit();
        }
        else
            throw CommandAbortedException();
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "preview" ) ) )
    {
        generateNewImage(aRet);
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        Sequence<Any> aIni;
        aCommand.Argument >>= aIni;
        if ( aIni.getLength() > 0 && aIni.getLength() < 2 )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        ::rtl::OUString sURL;
        aIni[0] >>= sURL;
        insert(sURL,Environment);
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getdocumentinfo" ) ) )
    {
        fillDocumentInfo(aRet);
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////
    }
    else
        aRet = OContentHelper::execute(aCommand,CommandId,Environment);
    return aRet;
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::insert(const ::rtl::OUString& _sURL, const Reference< XCommandEnvironment >& Environment)
    throw( Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( !_sURL.getLength() || m_xEmbeddedObject.is() )
    {
        OSL_ENSURE( sal_False, "Content::insert - property value missing!" );

        Sequence< rtl::OUString > aProps( 1 );
        aProps[ 0 ] = PROPERTY_URL;
        ucbhelper::cancelCommandExecution(
            makeAny( MissingPropertiesException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        // Unreachable
    }


    if ( !m_xEmbeddedObject.is() )
    {
        Reference< XStorage> xStorage = getStorage();
        if ( xStorage.is() )
        {
            Reference< XEmbedObjectCreator> xEmbedFactory( m_xORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator"))) ,UNO_QUERY);
            if ( xEmbedFactory.is() )
            {
                Sequence<PropertyValue> aEmpty,aMediaDesc(1);
                aMediaDesc[0].Name = PROPERTY_URL;
                aMediaDesc[0].Value <<= _sURL;
                m_xEmbeddedObject.set(xEmbedFactory->createInstanceInitFromMediaDescriptor( xStorage
                                                                                ,m_pImpl->m_aProps.sPersistentName
                                                                                ,aMediaDesc
                                                                                ,aEmpty),UNO_QUERY);
                Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
                if ( xPersist.is() )
                {
                    xPersist->storeOwn();
                }
                try
                {
                    Reference< com::sun::star::util::XCloseable> xCloseable(m_xEmbeddedObject,UNO_QUERY);
                    if ( xCloseable.is() )
                        xCloseable->close(sal_True);
                }
                catch(Exception)
                {
                }
                m_xEmbeddedObject = NULL;
              }
        }
    }

//  @@@
//  storeData();

    aGuard.clear();
//  inserted();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::saveObject(  ) throw (ObjectSaveVetoException, Exception, RuntimeException)
{
}
// -----------------------------------------------------------------------------
sal_Bool ODocumentDefinition::save(sal_Bool _bApprove)
{
    // default handling: instantiate an interaction handler and let it handle the parameter request
    try
    {

        {
            ::vos::OGuard aSolarGuard(Application::GetSolarMutex());

            // the request
            Reference<XNameAccess> xName(m_xParentContainer,UNO_QUERY);
            DocumentSaveRequest aRequest;
            aRequest.Name = m_pImpl->m_aProps.aTitle;
            if ( !aRequest.Name.getLength() )
            {
                if ( m_bForm )
                    aRequest.Name = DBACORE_RESSTRING( RID_STR_FORM );
                else
                    aRequest.Name = DBACORE_RESSTRING( RID_STR_REPORT );
                aRequest.Name = ::dbtools::createUniqueName(xName,aRequest.Name);
            }

            aRequest.Content.set(m_xParentContainer,UNO_QUERY);
            OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
            Reference< XInteractionRequest > xRequest(pRequest);
            // some knittings
            // two continuations allowed: OK and Cancel
            ODocumentSaveContinuation* pDocuSave = NULL;

            if ( !m_pImpl->m_aProps.aTitle.getLength() )
            {
                pDocuSave = new ODocumentSaveContinuation;
                pRequest->addContinuation(pDocuSave);
            }
            OInteraction< XInteractionApprove >* pApprove = NULL;
            if ( _bApprove )
            {
                pApprove = new OInteraction< XInteractionApprove >;
                pRequest->addContinuation(pApprove);
            }

            OInteraction< XInteractionDisapprove >* pDisApprove = new OInteraction< XInteractionDisapprove >;
            pRequest->addContinuation(pDisApprove);

            OInteractionAbort* pAbort = new OInteractionAbort;
            pRequest->addContinuation(pAbort);

            // create the handler, let it handle the request
            Reference< XInteractionHandler > xHandler(m_xORB->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
            if ( xHandler.is() )
                xHandler->handle(xRequest);

            if ( pAbort->wasSelected() )
                return sal_False;
            if  ( pDisApprove->wasSelected() )
                return sal_True;
            if ( pDocuSave && pDocuSave->wasSelected() )
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                Reference<XNameContainer> xNC(pDocuSave->getContent(),UNO_QUERY);
                if ( xNC.is() )
                {
                    m_pImpl->m_aProps.aTitle = pDocuSave->getName();
                    Reference< XContent> xContent = this;
                    xNC->insertByName(pDocuSave->getName(),makeAny(xContent));
                }
            }
        }

        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
        if ( xPersist.is() )
        {
            xPersist->storeOwn();
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("ODocumentDefinition::save: caught an Exception (tried to let the InteractionHandler handle it)!");
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::loadEmbeddedObject(const Sequence< sal_Int8 >& _aClassID,const Reference<XConnection>& _xConnection,sal_Bool _bReadOnly)
{
    if ( !m_xEmbeddedObject.is() )
    {
        Reference< XStorage> xStorage = getStorage();
        if ( xStorage.is() )
        {
            Reference< XEmbedObjectFactory> xEmbedFactory( m_xORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.OOoEmbeddedObjectFactory"))) ,UNO_QUERY);
            if ( xEmbedFactory.is() )
            {
                ::rtl::OUString sDocumentService;
                sal_Int32 nEntryConnectionMode = EntryInitModes::DEFAULT_INIT;
                Sequence< sal_Int8 > aClassID = _aClassID;
                if ( aClassID.getLength() )
                {
                    nEntryConnectionMode = EntryInitModes::TRUNCATE_INIT;
                }
                else
                {
                    aClassID = lcl_GetSequenceClassID(SO3_SW_CLASSID);
                    sDocumentService = lcl_GetDocumentServiceFromMediaType(xStorage,m_pImpl->m_aProps.sPersistentName);
                }
                Sequence<PropertyValue> aArgs;
                sal_Int32 nLen = aArgs.getLength();
                if ( _xConnection.is() )
                {
                    Sequence<PropertyValue> aDocumentContext(2);
                    aDocumentContext[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection"));
                    aDocumentContext[0].Value <<= _xConnection;

                    aDocumentContext[1].Name = PROPERTY_APPLYFORMDESIGNMODE;
                    aDocumentContext[1].Value <<= !_bReadOnly;

                    aArgs.realloc(nLen+1);
                    aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ComponentData"));
                    aArgs[nLen++].Value <<= aDocumentContext;
                }

                aArgs.realloc(nLen+3);
                aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
                aArgs[nLen++].Value <<= _bReadOnly;

                aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
                aArgs[nLen++].Value <<= MacroExecMode::USE_CONFIG;

                aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
                aArgs[nLen++].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter"));

                if ( m_pImpl->m_aProps.aTitle.getLength() )
                {
                    aArgs.realloc(nLen+1);
                    aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle"));
                    aArgs[nLen++].Value <<= m_pImpl->m_aProps.aTitle;
                }

                if ( m_pInterceptor )
                {
                    m_pInterceptor->DisconnectContentHolder();
                    m_pInterceptor->release();
                    m_pInterceptor = NULL;
                }

                m_pInterceptor = new OInterceptor( this ,_bReadOnly);
                m_pInterceptor->acquire();
                Reference<XDispatchProviderInterceptor> xInterceptor = m_pInterceptor;

                Sequence<PropertyValue> aEmbeddedObjectDescriptor(1);
                nLen = 0;
                aEmbeddedObjectDescriptor[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutplaceDispatchInterceptor"));
                aEmbeddedObjectDescriptor[nLen++].Value <<= xInterceptor;

                m_xEmbeddedObject.set(xEmbedFactory->createInstanceUserInit(aClassID
                                                                            ,sDocumentService
                                                                            ,xStorage
                                                                            ,m_pImpl->m_aProps.sPersistentName
                                                                            ,nEntryConnectionMode
                                                                            ,aArgs
                                                                            ,aEmbeddedObjectDescriptor
                                                                            ),UNO_QUERY);
                if ( m_xEmbeddedObject.is() )
                {
                    if ( !m_pClientHelper )
                        m_pClientHelper = new OEmbeddedClientHelper(this);
                    Reference<XEmbeddedClient> xClient = m_pClientHelper;
                    m_xEmbeddedObject->setClientSite(xClient);
                }
              }
        }
    }
    else if ( m_xEmbeddedObject->getCurrentState() == EmbedStates::LOADED )
    {
        m_xEmbeddedObject->setClientSite(this);
        m_xEmbeddedObject->changeState(EmbedStates::RUNNING);
    }
    Reference<XModel> xModel(getComponent(),UNO_QUERY);
    if ( xModel.is() )
    {
        Sequence<PropertyValue> aArgs = xModel->getArgs();

        sal_Bool bAddReadOnly = sal_True;
        sal_Bool bMacroExecutionMode = sal_True;
        PropertyValue* pIter = aArgs.getArray();
        PropertyValue* pEnd  = pIter + aArgs.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( pIter->Name.equalsAscii("ReadOnly") )
            {
                pIter->Value <<= _bReadOnly;
                bAddReadOnly = sal_False;
            }
            else if ( pIter->Name.equalsAscii("MacroExecutionMode") )
            {
                pIter->Value <<= MacroExecMode::USE_CONFIG;
                bMacroExecutionMode = sal_False;
            }
        }

        if ( bAddReadOnly )
        {
            sal_Int32 nLen = aArgs.getLength();
            aArgs.realloc(nLen+1);
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[nLen].Value <<= _bReadOnly;
        }

        if ( bMacroExecutionMode )
        {
            sal_Int32 nLen = aArgs.getLength();
            aArgs.realloc(nLen+1);
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
            aArgs[nLen].Value <<= MacroExecMode::USE_CONFIG;
        }

        xModel->attachResource(xModel->getURL(),aArgs);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::onShowWindow( sal_Bool bVisible ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::generateNewImage(Any& _rImage)
{
    loadEmbeddedObject();
    if ( m_xEmbeddedObject.is() )
    {
        try
        {
            Reference<XTransferable> xTransfer(getComponent(),UNO_QUERY);
            if ( xTransfer.is() )
            {
                DataFlavor aFlavor;
                aFlavor.MimeType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("image/png"));
                aFlavor.HumanPresentableName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Portable Network Graphics"));
                aFlavor.DataType = ::getCppuType(static_cast< const Sequence < sal_Int8 >* >(NULL));

                _rImage = xTransfer->getTransferData( aFlavor );
            }
        }
        catch( Exception )
        {
        }
    }
}
// -----------------------------------------------------------------------------
Any ODocumentDefinition::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    return Any();
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::fillDocumentInfo(Any& _rInfo)
{
    loadEmbeddedObject();
    if ( m_xEmbeddedObject.is() )
    {
        try
        {
            Reference<XDocumentInfoSupplier> xDocSup(getComponent(),UNO_QUERY);
            if ( xDocSup.is() )
                _rInfo <<= xDocSup->getDocumentInfo();
        }
        catch( Exception e )
        {
        }
    }
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::util::XCloseable> ODocumentDefinition::getComponent() throw (RuntimeException)
{
    OSL_ENSURE(m_xEmbeddedObject.is(),"Illegal call for embeddedObject");
    Reference< ::com::sun::star::util::XCloseable> xComp;
    if ( m_xEmbeddedObject.is() )
    {
        int nOldState = m_xEmbeddedObject->getCurrentState();
        int nState = nOldState;
        if ( nOldState == EmbedStates::LOADED )
        {
            m_xEmbeddedObject->changeState( EmbedStates::RUNNING );
            nState = EmbedStates::RUNNING;
        }

        if ( nState == EmbedStates::ACTIVE || nState == EmbedStates::RUNNING )
        {
            Reference<XComponentSupplier> xCompProv(m_xEmbeddedObject,UNO_QUERY);
            if ( xCompProv.is() )
            {
                xComp = xCompProv->getComponent();
                OSL_ENSURE(xComp.is(),"No valid component");
            }
        }
    }
    return xComp;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::visibilityChanged( ::sal_Bool bVisible ) throw (WrongStateException, RuntimeException)
{
//  ::osl::MutexGuard aGuard(m_aMutex);
//  if ( m_xEmbeddedObject.is() && !bVisible )
//  {
//      try
//      {
//          Reference< com::sun::star::util::XCloseable> xCloseable(m_xEmbeddedObject,UNO_QUERY);
//          if ( xCloseable.is() )
//              xCloseable->close(sal_True);
//      }
//      catch(Exception)
//      {
//      }
//      m_xEmbeddedObject = NULL;
//      if ( m_pClientHelper )
//          m_pClientHelper->resetClient(NULL);
//  }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
{
    try
    {
        sal_Int32 nHandle = PROPERTY_ID_NAME;
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        Any aOld = makeAny(m_pImpl->m_aProps.aTitle);
        aGuard.clear();
        Any aNew = makeAny(newName);
        fire(&nHandle, &aNew, &aOld, 1, sal_True );
        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, sal_False );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}
// -----------------------------------------------------------------------------
Reference< XStorage> ODocumentDefinition::getStorage() const
{
    static const ::rtl::OUString s_sForms = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("forms"));
    static const ::rtl::OUString s_sReports = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("reports"));
    return m_pImpl->m_pDataSource ? m_pImpl->m_pDataSource->getStorage(m_bForm ? s_sForms : s_sReports) : NULL;
}
// -----------------------------------------------------------------------------
sal_Bool ODocumentDefinition::isModified()
{
    osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
    sal_Bool bRet = sal_False;
    if ( m_xEmbeddedObject.is() )
    {
        Reference<XModifiable> xModel(getComponent(),UNO_QUERY);
        bRet = xModel.is() && xModel->isModified();
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::setModelReadOnly(sal_Bool _bReadOnly)
{
    Reference<XModel> xModel(getComponent(),UNO_QUERY);
    if ( xModel.is() )
    {
        Sequence<PropertyValue> aArgs = xModel->getArgs();

        sal_Bool bAddReadOnly = sal_True;
        sal_Bool bMacroExecutionMode = sal_True;
        PropertyValue* pIter = aArgs.getArray();
        PropertyValue* pEnd  = pIter + aArgs.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( pIter->Name.equalsAscii("ReadOnly") )
            {
                pIter->Value <<= _bReadOnly;
                bAddReadOnly = sal_False;
            }
        }

        if ( bAddReadOnly )
        {
            sal_Int32 nLen = aArgs.getLength();
            aArgs.realloc(nLen+1);
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[nLen].Value <<= _bReadOnly;
        }

        xModel->attachResource(xModel->getURL(),aArgs);
    }
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::fillReportData(sal_Bool _bFill)
{
    if ( !m_bForm && _bFill && m_pImpl->m_aProps.bAsTemplate ) // open a report in alive mode, so we need to fill it
    {
        setModelReadOnly(sal_False);
        Sequence<Any> aArgs(1);
        PropertyValue aValue;
        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TextDocument"));
        aValue.Value <<= getComponent();
        aArgs[0] <<= aValue;
        Reference< XJobExecutor > xExecuteable(m_xORB->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.wizards.report.CallReportWizard")),aArgs),UNO_QUERY);
        if ( xExecuteable.is() )
            xExecuteable->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("fill")));
        setModelReadOnly(sal_True);
    }
}
//........................................................................
}   // namespace dbaccess
//........................................................................

