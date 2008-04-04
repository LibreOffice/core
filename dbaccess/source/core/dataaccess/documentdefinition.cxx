/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentdefinition.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:49:32 $
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

#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#include "documentdefinition.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif
#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
#endif
#ifndef _COMPHELPER_CLASSIDS_HXX
#include <comphelper/classids.hxx>
#endif
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
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
#include <com/sun/star/frame/XTitle.hpp>
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XReportEngine.hpp>
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_XEMBEDOBJECTFACTORY_HPP_
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XMODULE_HPP_
#include <com/sun/star/frame/XModule.hpp>
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
#ifndef _COM_SUN_STAR_EMBED_XCOMMONEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XCommonEmbedPersist.hpp>
#endif
#ifndef DBA_INTERCEPT_HXX
#include "intercept.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_ERRORCONDITION_HPP_
#include <com/sun/star/sdb/ErrorCondition.hpp>
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
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTPROPERTIESSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _COMPHELPER_MIMECONFIGHELPER_HXX_
#include <comphelper/mimeconfighelper.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#include <com/sun/star/io/WrongFormatException.hpp>

using namespace ::com::sun::star;
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
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::drawing;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
namespace css = ::com::sun::star;


#define DEFAULT_WIDTH  15000
#define DEFAULT_HEIGHT 10000
//........................................................................
namespace dbaccess
{
    //==================================================================
    // OEmbedObjectHolder
    //==================================================================
    typedef ::cppu::WeakComponentImplHelper1<   ::com::sun::star::embed::XStateChangeListener > TEmbedObjectHolder;
    class OEmbedObjectHolder :   public ::comphelper::OBaseMutex
                                ,public TEmbedObjectHolder
    {
        Reference< XEmbeddedObject >    m_xBroadCaster;
        ODocumentDefinition*            m_pDefinition;
        bool                            m_bInStateChange;
        bool                            m_bInChangingState;
    protected:
        virtual void SAL_CALL disposing();
    public:
        OEmbedObjectHolder(const Reference< XEmbeddedObject >& _xBroadCaster,ODocumentDefinition* _pDefinition)
            : TEmbedObjectHolder(m_aMutex)
            ,m_xBroadCaster(_xBroadCaster)
            ,m_pDefinition(_pDefinition)
            ,m_bInStateChange(false)
            ,m_bInChangingState(false)
        {
            osl_incrementInterlockedCount( &m_refCount );
            {
                if ( m_xBroadCaster.is() )
                    m_xBroadCaster->addStateChangeListener(this);
            }
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
    void SAL_CALL OEmbedObjectHolder::changingState( const ::com::sun::star::lang::EventObject& /*aEvent*/, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
    {
        if ( !m_bInChangingState && nNewState == EmbedStates::RUNNING && nOldState == EmbedStates::ACTIVE && m_pDefinition )
        {
            m_bInChangingState = true;
            //m_pDefinition->save(sal_False);
            m_bInChangingState = false;
        }
    }
    //------------------------------------------------------------------
    void SAL_CALL OEmbedObjectHolder::stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
    {
        if ( !m_bInStateChange && nNewState == EmbedStates::RUNNING && nOldState == EmbedStates::ACTIVE && m_pDefinition )
        {
            m_bInStateChange = true;
            Reference<XInterface> xInt(static_cast< ::cppu::OWeakObject* >(m_pDefinition),UNO_QUERY);
            {
                Reference<XEmbeddedObject> xEmbeddedObject(aEvent.Source,UNO_QUERY);
                if ( xEmbeddedObject.is() )
                    xEmbeddedObject->changeState(EmbedStates::LOADED);
            }
            m_bInStateChange = false;
        }
    }
    //------------------------------------------------------------------
    void SAL_CALL OEmbedObjectHolder::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_xBroadCaster = NULL;
    }

    //==================================================================
    // OEmbeddedClientHelper
    //==================================================================
    typedef ::cppu::WeakImplHelper1 <   XEmbeddedClient
                                    >   EmbeddedClientHelper_BASE;
    class OEmbeddedClientHelper : public EmbeddedClientHelper_BASE
    {
        ODocumentDefinition* m_pClient;
    public:
        OEmbeddedClientHelper(ODocumentDefinition* _pClient) :m_pClient(_pClient) {}

        virtual void SAL_CALL saveObject(  ) throw (ObjectSaveVetoException, Exception, RuntimeException)
        {
        }
        virtual void SAL_CALL onShowWindow( sal_Bool /*bVisible*/ ) throw (RuntimeException)
        {
        }
        // XComponentSupplier
        virtual Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent(  ) throw (RuntimeException)
        {
            return Reference< css::util::XCloseable >();
        }

        // XEmbeddedClient
        virtual void SAL_CALL visibilityChanged( ::sal_Bool /*bVisible*/ ) throw (WrongStateException, RuntimeException)
        {
        }
        inline void resetClient(ODocumentDefinition* _pClient) { m_pClient = _pClient; }
    };

    //==================================================================
    // LifetimeCoupler
    //==================================================================
    typedef ::cppu::WeakImplHelper1 <   css::lang::XEventListener
                                    >   LifetimeCoupler_Base;
    /** helper class which couples the lifetime of a component to the lifetim
        of another component

        Instances of this class are constructed with two components. The first is
        simply held by reference, and thus kept alive. The second one is observed
        for <code>disposing</code> calls - if they occur, i.e. if the component dies,
        the reference to the first component is cleared.

        This way, you can ensure that a certain component is kept alive as long
        as a second component is not disposed.
    */
    class LifetimeCoupler : public LifetimeCoupler_Base
    {
    private:
        Reference< XInterface > m_xClient;

    public:
        inline static void couple( const Reference< XInterface >& _rxClient, const Reference< XComponent >& _rxActor )
        {
            Reference< css::lang::XEventListener > xEnsureDelete( new LifetimeCoupler( _rxClient, _rxActor ) );
        }

    private:
        inline LifetimeCoupler( const Reference< XInterface >& _rxClient, const Reference< XComponent >& _rxActor )
            :m_xClient( _rxClient )
        {
            DBG_ASSERT( _rxActor.is(), "LifetimeCoupler::LifetimeCoupler: this will crash!" );
            osl_incrementInterlockedCount( &m_refCount );
            {
                _rxActor->addEventListener( this );
            }
            osl_decrementInterlockedCount( &m_refCount );
            DBG_ASSERT( m_refCount, "LifetimeCoupler::LifetimeCoupler: the actor is not holding us by hard ref - this won't work!" );
        }

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (RuntimeException);
    protected:
    };

    //------------------------------------------------------------------
    void SAL_CALL LifetimeCoupler::disposing( const css::lang::EventObject& /*Source*/ ) throw (RuntimeException)
    {
        m_xClient.clear();
    }

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

::rtl::OUString ODocumentDefinition::GetDocumentServiceFromMediaType( const Reference< XStorage >& xStorage
                                                    ,const ::rtl::OUString& sEntName
                                                    ,const Reference< XMultiServiceFactory >& _xORB
                                                    ,Sequence< sal_Int8 >& _rClassId
                                                    )
{
    ::rtl::OUString sResult;
    try
    {
        if ( xStorage->isStorageElement( sEntName ) )
        {
            // the object must be based on storage

            Reference< XPropertySet > xPropSet( xStorage->openStorageElement( sEntName, ElementModes::READ ), UNO_QUERY_THROW );

            ::rtl::OUString aMediaType;
            try {
                Any aAny = xPropSet->getPropertyValue( INFO_MEDIATYPE );
                aAny >>= aMediaType;
            }
            catch ( Exception& )
            {
            }
            ::comphelper::MimeConfigurationHelper aConfigHelper(_xORB);
            sResult = aConfigHelper.GetDocServiceNameFromMediaType(aMediaType);
            _rClassId = aConfigHelper.GetSequenceClassIDRepresentation(aConfigHelper.GetExplicitlyRegisteredObjClassID(aMediaType));
            if ( !_rClassId.getLength() && sResult.getLength() )
            {
                Reference< XNameAccess > xObjConfig = aConfigHelper.GetObjConfiguration();
                if ( xObjConfig.is() )
                {
                    try
                    {
                        Sequence< ::rtl::OUString > aClassIDs = xObjConfig->getElementNames();
                        for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                        {
                            Reference< XNameAccess > xObjectProps;
                            ::rtl::OUString aEntryDocName;

                            if (    ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                                 && ( xObjectProps->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ObjectDocumentServiceName"))
                                              ) >>= aEntryDocName )
                                 && aEntryDocName.equals( sResult ) )
                            {
                                _rClassId = aConfigHelper.GetSequenceClassIDRepresentation(aClassIDs[nInd]);
                                break;
                            }
                        }
                    }
                    catch( Exception& )
                    {}
                }
            }

            ::comphelper::disposeComponent( xPropSet );
        }
    }
    catch ( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sResult;
}
// -----------------------------------------------------------------------------
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
    ,m_bForm(_bForm)
    ,m_bOpenInDesign(sal_False)
    ,m_bInExecute(sal_False)
    ,m_pClientHelper(NULL)
{
    DBG_CTOR(ODocumentDefinition, NULL);
    registerProperties();
    if ( _aClassID.getLength() )
        loadEmbeddedObject( _xConnection, _aClassID, Sequence< PropertyValue >(), false, false );
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
        m_pInterceptor->dispose();
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
        {
            m_pClientHelper->resetClient(NULL);
            m_pClientHelper->release();
            m_pClientHelper = NULL;
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODocumentDefinition::disposing()
{
    OContentHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    closeObject();
    ::comphelper::disposeComponent(m_xListener);
    m_xDesktop = NULL;
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
class OExecuteImpl
{
    sal_Bool& m_rbSet;
public:
    OExecuteImpl(sal_Bool& _rbSet) : m_rbSet(_rbSet){ m_rbSet=sal_True; }
    ~OExecuteImpl(){ m_rbSet = sal_False; }
};
// -----------------------------------------------------------------------------
namespace
{
    bool lcl_extractOpenMode( const Any& _rValue, sal_Int32& _out_rMode )
    {
        OpenCommandArgument aOpenCommand;
        if ( _rValue >>= aOpenCommand )
            _out_rMode = aOpenCommand.Mode;
        else
        {
            OpenCommandArgument2 aOpenCommand2;
            if ( _rValue >>= aOpenCommand2 )
                _out_rMode = aOpenCommand2.Mode;
            else
                return false;
        }
        return true;
    }
}

// -----------------------------------------------------------------------------
void ODocumentDefinition::impl_removeFrameFromDesktop_throw( const Reference< XFrame >& _rxFrame )
{
    if ( !m_xDesktop.is() )
        m_xDesktop.set( m_aContext.createComponent( (::rtl::OUString)SERVICE_FRAME_DESKTOP ), UNO_QUERY_THROW );

    Reference< XFrames > xFrames( m_xDesktop->getFrames(), UNO_QUERY_THROW );
    xFrames->remove( _rxFrame );
}

// -----------------------------------------------------------------------------
void ODocumentDefinition::impl_onActivateEmbeddedObject()
{
    try
    {
        Reference< XModel > xModel( getComponent(), UNO_QUERY );
        Reference< XController > xController( xModel.is() ? xModel->getCurrentController() : Reference< XController >() );
        if ( !xController.is() )
            return;

        if ( !m_xListener.is() )
            // it's the first time the embedded object has been activated
            // create an OEmbedObjectHolder
            m_xListener = new OEmbedObjectHolder(m_xEmbeddedObject,this);

        Reference< XFrame > xFrame( xController->getFrame() );
        if ( xFrame.is() )
        {
            // raise the window to top (especially necessary if this is not the first activation)
            Reference< XTopWindow > xTopWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
            xTopWindow->toFront();

            // remove the frame from the desktop's frame collection because we need full control of it.
            impl_removeFrameFromDesktop_throw( xFrame );
        }

        // ensure that we ourself are kept alive as long as the embedded object's frame is
        // opened
        LifetimeCoupler::couple( *this, Reference< XComponent >( xFrame, UNO_QUERY_THROW ) );

        // init the edit view
        if ( m_bOpenInDesign )
            impl_initObjectEditView( xController );
    }
    catch( const RuntimeException& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
namespace
{
    // =========================================================================
    // = PreserveVisualAreaSize
    // =========================================================================
    /** stack-guard for preserving the size of the VisArea of an XModel
    */
    class PreserveVisualAreaSize
    {
    private:
        Reference< XVisualObject >  m_xVisObject;
        ::com::sun::star::awt::Size m_aOriginalSize;

    public:
        inline PreserveVisualAreaSize( const Reference< XModel >& _rxModel )
            :m_xVisObject( _rxModel, UNO_QUERY )
        {
            if ( m_xVisObject.is() )
            {
                try
                {
                    m_aOriginalSize = m_xVisObject->getVisualAreaSize( Aspects::MSOLE_CONTENT );
                }
                catch ( Exception& )
                {
                    DBG_ERROR( "PreserveVisualAreaSize::PreserveVisualAreaSize: caught an exception!" );
                }
            }
        }

        inline ~PreserveVisualAreaSize()
        {
            if ( m_xVisObject.is() && m_aOriginalSize.Width && m_aOriginalSize.Height )
            {
                try
                {
                    m_xVisObject->setVisualAreaSize( Aspects::MSOLE_CONTENT, m_aOriginalSize );
                }
                catch ( Exception& )
                {
                    DBG_ERROR( "PreserveVisualAreaSize::~PreserveVisualAreaSize: caught an exception!" );
                }
            }
        }
    };

    // =========================================================================
    // = LayoutManagerLock
    // =========================================================================
    /** helper class for stack-usage which during its lifetime locks a layout manager
    */
    class LayoutManagerLock
    {
    private:
        Reference< XLayoutManager > m_xLayoutManager;

    public:
        inline LayoutManagerLock( const Reference< XController >& _rxController )
        {
            DBG_ASSERT( _rxController.is(), "LayoutManagerLock::LayoutManagerLock: this will crash!" );
            Reference< XFrame > xFrame( _rxController->getFrame() );
            try
            {
                Reference< XPropertySet > xPropSet( xFrame, UNO_QUERY_THROW );
                m_xLayoutManager.set(
                    xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) ),
                    UNO_QUERY_THROW );
                m_xLayoutManager->lock();

            }
            catch( Exception& )
            {
                DBG_ERROR( "LayoutManagerLock::LayoutManagerLock: caught an exception!" );
            }
        }

        inline ~LayoutManagerLock()
        {
            try
            {
                // unlock the layout manager
                if ( m_xLayoutManager.is() )
                    m_xLayoutManager->unlock();
            }
            catch( Exception& )
            {
                DBG_ERROR( "LayoutManagerLock::~LayoutManagerLock: caught an exception!" );
            }
        }
    };
}

// -----------------------------------------------------------------------------
void ODocumentDefinition::impl_initObjectEditView( const Reference< XController >& _rxController )
{
    if ( !m_bForm )
        // currently, only forms need to be initialized
        return;

    try
    {
        Reference< XViewSettingsSupplier > xSettingsSupplier( _rxController, UNO_QUERY_THROW );
        Reference< XPropertySet > xViewSettings( xSettingsSupplier->getViewSettings(), UNO_QUERY_THROW );

        // The visual area size can be changed by the setting of the following properties
        // so it should be restored later
        PreserveVisualAreaSize aPreserveVisAreaSize( _rxController->getModel() );

        // Layout manager should not layout while the size is still not restored
        // so it will stay locked for this time
        LayoutManagerLock aLockLayout( _rxController );

        // setting of the visual properties
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowRulers")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowVertRuler")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowHoriRuler")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsRasterVisible")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSnapToRaster")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowOnlineLayout")),makeAny(sal_True));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RasterSubdivisionX")),makeAny(sal_Int32(5)));
        xViewSettings->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RasterSubdivisionY")),makeAny(sal_Int32(5)));

        Reference< XModifiable > xModifiable( _rxController->getModel(), UNO_QUERY_THROW );
        xModifiable->setModified( sal_False );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
void ODocumentDefinition::onCommandOpenSomething( const Any& _rOpenArgument, const bool _bActivate,
    const Reference< XCommandEnvironment >& _rxEnvironment, Any& _out_rComponent )
{
    OExecuteImpl aExecuteGuard(m_bInExecute);

    Reference< XConnection > xConnection;
    sal_Int32 nOpenMode = OpenMode::DOCUMENT;

    ::comphelper::NamedValueCollection aDocumentArgs;

    // for the document, default to the interaction handler as used for loading the DB doc
    // This might be overwritten below, when examining _rOpenArgument.
    ::comphelper::NamedValueCollection aDBDocArgs( m_pImpl->m_pDataSource->m_aArgs );
    aDocumentArgs.put( "InteractionHandler", aDBDocArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );

    ::boost::optional< sal_Int16 > aDocumentMacroMode;

    if ( !lcl_extractOpenMode( _rOpenArgument, nOpenMode ) )
    {
        Sequence< PropertyValue > aArguments;
        if ( _rOpenArgument >>= aArguments )
        {
            const PropertyValue* pIter = aArguments.getConstArray();
            const PropertyValue* pEnd  = pIter + aArguments.getLength();
            for ( ;pIter != pEnd; ++pIter )
            {
                if ( pIter->Name == PROPERTY_ACTIVE_CONNECTION )
                {
                    xConnection.set( pIter->Value, UNO_QUERY );
                    continue;
                }

                if ( lcl_extractOpenMode( pIter->Value, nOpenMode ) )
                    continue;

                if ( pIter->Name.equalsAscii( "MacroExecutionMode" ) )
                {
                    sal_Int16 nMacroExecMode( !aDocumentMacroMode ? MacroExecMode::USE_CONFIG : *aDocumentMacroMode );
                    OSL_VERIFY( pIter->Value >>= nMacroExecMode );
                    aDocumentMacroMode.reset( nMacroExecMode );
                    continue;
                }

                // unknown argument -> pass to the loaded document
                aDocumentArgs.put( pIter->Name, pIter->Value );
            }
        }
    }

    // our database document's macro execution mode
    sal_Int16 nImposedMacroExecMode( m_pImpl->m_pDataSource->getImposedMacroExecMode() );
        // (caching it, since adjustMacroMode will overwrite it)
    bool bExecuteDBDocMacros = m_pImpl->m_pDataSource->adjustMacroMode_AutoReject();
        // Note that we don't pass an interaction handler here. If the user has not been asked/notified
        // by now (i.e. during loading the whole DB document), then this won't happen anymore.

    // allow the command arguments to downgrade the macro execution mode, but not to upgrade
    // it
    if  (   ( nImposedMacroExecMode == MacroExecMode::USE_CONFIG )
        &&  bExecuteDBDocMacros
        )
    {
        // while loading the whole database document, USE_CONFIG, or *no* macro exec mode was passed.
        // Additionally, *by now* executing macros from the DB doc is allowed (this is what bExecuteDBDocMacros
        // indicates). This means either one of:
        // 1. The DB doc or one of the sub docs contained macros and
        // 1a. the user explicitly allowed executing them
        // 1b. the configuration allows executing them without asking the user
        // 2. Neither the DB doc nor the sub docs contained macros, thus macro
        //    execution was silently enabled, assuming that any macro will be a
        //    user-created macro
        //
        // The problem with this: If the to-be-opened sub document has macros embedded in
        // the content.xml (which is valid ODF, but normally not produced by OOo itself),
        // then this has not been detected while loading the database document - it would
        // be too expensive, as it effectively would require loading all forms/reports.
        //
        // So, in such a case, and with 2. above, we would silently execute those macros,
        // regardless of the global security settings - which would be a security issue, of
        // course.
        if ( !m_pImpl->m_pDataSource->hasAnyObjectWithMacros() )
        {
            // this is case 2. from above (not *exactly*, but sufficiently)
            // So, pass a USE_CONFIG to the to-be-loaded document. This means that
            // the user will be prompted with a security message upon opening this
            // sub document, in case the settings require this, *and* the document
            // contains scripts in the content.xml. But this is better than the security
            // issue we had before ...
            aDocumentMacroMode.reset( MacroExecMode::USE_CONFIG );
        }
    }

    if ( !aDocumentMacroMode )
    {
        // nobody so far felt responsible for setting it
        // => use the DBDoc-wide macro exec mode for the document, too
        aDocumentMacroMode.reset( bExecuteDBDocMacros ? MacroExecMode::ALWAYS_EXECUTE_NO_WARN : MacroExecMode::NEVER_EXECUTE );
    }
    aDocumentArgs.put( "MacroExecutionMode", *aDocumentMacroMode );


    if ( xConnection.is() )
        m_xLastKnownConnection = xConnection;

    if  (   ( nOpenMode == OpenMode::ALL )
        ||  ( nOpenMode == OpenMode::FOLDERS )
        ||  ( nOpenMode == OpenMode::DOCUMENTS )
        ||  ( nOpenMode == OpenMode::DOCUMENT_SHARE_DENY_NONE )
        ||  ( nOpenMode == OpenMode::DOCUMENT_SHARE_DENY_WRITE )
        )
    {
        // not supported
        ucbhelper::cancelCommandExecution(
                makeAny( UnsupportedOpenModeException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                sal_Int16( nOpenMode ) ) ),
                _rxEnvironment );
        // Unreachable
        DBG_ERROR( "unreachable" );
      }

    Reference<XModel> xModel;
    if ( m_pImpl->m_aProps.sPersistentName.getLength() )
    {
        Sequence< PropertyValue > aLoadArgs;
        aDocumentArgs >>= aLoadArgs;
        loadEmbeddedObject( xConnection, Sequence< sal_Int8 >(), aLoadArgs, false, !m_bOpenInDesign );
        if ( m_xEmbeddedObject.is() )
        {
            xModel.set(getComponent(),UNO_QUERY);
            Reference< report::XReportDefinition > xReportDefinition(xModel,UNO_QUERY);

            Reference< XModule> xModule(xModel,UNO_QUERY);
            if ( xModule.is() )
            {
                if ( m_bForm )
                    xModule->setIdentifier(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.FormDesign")));
                else if ( !xReportDefinition.is() )
                    xModule->setIdentifier(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.TextReportDesign")));

                updateDocumentTitle();
            }

            bool bIsAliveNewStyleReport = ( !m_bOpenInDesign && xReportDefinition.is() );
            if ( bIsAliveNewStyleReport )
            {
                // we are in ReadOnly mode
                // we would like to open the Writer or Calc with the report direct, without design it.
                Reference< report::XReportEngine > xReportEngine( m_aContext.createComponent( "com.sun.star.comp.report.OReportEngineJFree" ), UNO_QUERY_THROW );

                xReportEngine->setReportDefinition(xReportDefinition);
                xReportEngine->setActiveConnection(m_xLastKnownConnection);
                _out_rComponent <<= xReportEngine->createDocumentAlive(NULL);
                return;
            }

            if ( _bActivate )
            {
                m_xEmbeddedObject->changeState( EmbedStates::ACTIVE );
                impl_onActivateEmbeddedObject();
            }

            fillReportData();
            _out_rComponent <<= xModel;
        }
    }
}

// -----------------------------------------------------------------------------
Any SAL_CALL ODocumentDefinition::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment ) throw (Exception, CommandAbortedException, RuntimeException)
{
    Any aRet;
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_bInExecute )
    {
        sal_Bool bOpen = aCommand.Name.equalsAscii( "open" );
        sal_Bool bOpenInDesign = aCommand.Name.equalsAscii( "openDesign" );
        sal_Bool bOpenForMail = aCommand.Name.equalsAscii( "openForMail" );
        if ( bOpen || bOpenInDesign || bOpenForMail )
        {
            m_bOpenInDesign = bOpenInDesign;
            onCommandOpenSomething( aCommand.Argument, !bOpenForMail, Environment, aRet );
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
                m_xEmbeddedObject->changeState(EmbedStates::LOADED);
            }
            else
                throw CommandAbortedException();
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "preview" ) ) )
        {
            onCommandPreview(aRet);
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
            onCommandInsert( sURL, Environment );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getdocumentinfo" ) ) )
        {
            onCommandGetDocumentProperties( aRet );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
        {
            //////////////////////////////////////////////////////////////////
            // delete
            //////////////////////////////////////////////////////////////////
            closeObject();
            Reference< XStorage> xStorage = getContainerStorage();
            if ( xStorage.is() )
                xStorage->removeElement(m_pImpl->m_aProps.sPersistentName);

            dispose();
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "storeOwn" ) ) )
        {
            Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
            if ( xPersist.is() )
            {
                xPersist->storeOwn();
                notifyDataSourceModified();
            }
        }
        else if ( aCommand.Name.compareToAscii( "shutdown" ) == 0 )
        {
            bool bClose = prepareClose();
            if ( bClose && m_xEmbeddedObject.is() )
                m_xEmbeddedObject->changeState(EmbedStates::LOADED);
            aRet <<= bClose;
        }
        else
            aRet = OContentHelper::execute(aCommand,CommandId,Environment);
    }
    return aRet;
}
// -----------------------------------------------------------------------------
namespace
{
    void lcl_resetChildFormsToEmptyDataSource( const Reference< XIndexAccess>& _rxFormsContainer )
    {
        OSL_PRECOND( _rxFormsContainer.is(), "lcl_resetChildFormsToEmptyDataSource: illegal call!" );
        sal_Int32 count = _rxFormsContainer->getCount();
        for ( sal_Int32 i = 0; i < count; ++i )
        {
            Reference< XForm > xForm( _rxFormsContainer->getByIndex( i ), UNO_QUERY );
            if ( !xForm.is() )
                continue;

            // if the element is a form, reset its DataSourceName property to an empty string
            try
            {
                Reference< XPropertySet > xFormProps( xForm, UNO_QUERY_THROW );
                xFormProps->setPropertyValue( PROPERTY_DATASOURCENAME, makeAny( ::rtl::OUString() ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            // if the element is a container itself, step down the component hierarchy
            Reference< XIndexAccess > xContainer( xForm, UNO_QUERY );
            if ( xContainer.is() )
                lcl_resetChildFormsToEmptyDataSource( xContainer );
        }
    }

    void lcl_resetFormsToEmptyDataSource( const Reference< XEmbeddedObject>& _rxEmbeddedObject )
    {
        try
        {
            Reference< XComponentSupplier > xCompProv( _rxEmbeddedObject, UNO_QUERY_THROW );
            Reference< XDrawPageSupplier > xSuppPage( xCompProv->getComponent(), UNO_QUERY_THROW );
                // if this interface does not exist, then either getComponent returned NULL,
                // or the document is a multi-page document. The latter is allowed, but currently
                // simply not handled by this code, as it would not normally happen.

            Reference< XFormsSupplier > xSuppForms( xSuppPage->getDrawPage(), UNO_QUERY_THROW );
            Reference< XIndexAccess > xForms( xSuppForms->getForms(), UNO_QUERY_THROW );
            lcl_resetChildFormsToEmptyDataSource( xForms );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    }
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::onCommandInsert( const ::rtl::OUString& _sURL, const Reference< XCommandEnvironment >& Environment )
    throw( Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( !_sURL.getLength() || m_xEmbeddedObject.is() )
    {
        OSL_ENSURE( sal_False, "Content::onCommandInsert - property value missing!" );

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
        Reference< XStorage> xStorage = getContainerStorage();
        if ( xStorage.is() )
        {
            Reference< XEmbedObjectCreator> xEmbedFactory( m_aContext.createComponent( "com.sun.star.embed.EmbeddedObjectCreator" ), UNO_QUERY );
            if ( xEmbedFactory.is() )
            {
                Sequence<PropertyValue> aEmpty,aMediaDesc(1);
                aMediaDesc[0].Name = PROPERTY_URL;
                aMediaDesc[0].Value <<= _sURL;
                m_xEmbeddedObject.set(xEmbedFactory->createInstanceInitFromMediaDescriptor( xStorage
                                                                                ,m_pImpl->m_aProps.sPersistentName
                                                                                ,aMediaDesc
                                                                                ,aEmpty),UNO_QUERY);

                lcl_resetFormsToEmptyDataSource( m_xEmbeddedObject );
                // #i57669# / 2005-12-01 / frank.schoenheit@sun.com

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
sal_Bool ODocumentDefinition::save(sal_Bool _bApprove)
{
    // default handling: instantiate an interaction handler and let it handle the parameter request
    if ( !m_bOpenInDesign )
        return sal_False;
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
            else if ( xName->hasByName(aRequest.Name) )
                aRequest.Name = ::dbtools::createUniqueName(xName,aRequest.Name);

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
            Reference< XInteractionHandler > xHandler( m_aContext.createComponent( (::rtl::OUString)SERVICE_SDB_INTERACTION_HANDLER ), UNO_QUERY );
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

                    updateDocumentTitle();
                }
            }
        }

        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
        if ( xPersist.is() )
        {
            xPersist->storeOwn();
            notifyDataSourceModified();
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"ODocumentDefinition::save: caught an Exception (tried to let the InteractionHandler handle it)!");
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool ODocumentDefinition::saveAs()
{
    // default handling: instantiate an interaction handler and let it handle the parameter request
    if ( !m_bOpenInDesign )
        return sal_False;
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
            ODocumentSaveContinuation* pDocuSave = new ODocumentSaveContinuation;
            pRequest->addContinuation(pDocuSave);
            OInteraction< XInteractionDisapprove >* pDisApprove = new OInteraction< XInteractionDisapprove >;
            pRequest->addContinuation(pDisApprove);
            OInteractionAbort* pAbort = new OInteractionAbort;
            pRequest->addContinuation(pAbort);

            // create the handler, let it handle the request
            Reference< XInteractionHandler > xHandler(m_aContext.createComponent(::rtl::OUString(SERVICE_SDB_INTERACTION_HANDLER)), UNO_QUERY);
            if ( xHandler.is() )
                xHandler->handle(xRequest);

            if ( pAbort->wasSelected() )
                return sal_False;
            if  ( pDisApprove->wasSelected() )
                return sal_True;
            if ( pDocuSave->wasSelected() )
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                Reference<XNameContainer> xNC(pDocuSave->getContent(),UNO_QUERY);
                if ( xNC.is() )
                {
                    try
                    {
                        Reference< XStorage> xStorage = getContainerStorage();
                        const static ::rtl::OUString sBaseName(RTL_CONSTASCII_USTRINGPARAM("Obj"));
                        // -----------------------------------------------------------------------------
                        Reference<XNameAccess> xElements(xStorage,UNO_QUERY_THROW);
                        ::rtl::OUString sPersistentName = ::dbtools::createUniqueName(xElements,sBaseName);
                        xStorage->copyElementTo(m_pImpl->m_aProps.sPersistentName,xStorage,sPersistentName);

                        ::rtl::OUString sOldName = m_pImpl->m_aProps.aTitle;
                        rename(pDocuSave->getName());
                        updateDocumentTitle();

                        Sequence< Any > aArguments(3);
                        PropertyValue aValue;
                        // set as folder
                        aValue.Name = PROPERTY_NAME;
                        aValue.Value <<= sOldName;
                        aArguments[0] <<= aValue;

                        aValue.Name = PROPERTY_PERSISTENT_NAME;
                        aValue.Value <<= sPersistentName;
                        aArguments[1] <<= aValue;

                        aValue.Name = PROPERTY_AS_TEMPLATE;
                        aValue.Value <<= m_pImpl->m_aProps.bAsTemplate;
                        aArguments[2] <<= aValue;

                        Reference< XMultiServiceFactory > xORB( m_xParentContainer, UNO_QUERY_THROW );
                        Reference< XInterface > xComponent( xORB->createInstanceWithArguments( SERVICE_SDB_DOCUMENTDEFINITION, aArguments ) );
                        Reference< XNameContainer > xNameContainer( m_xParentContainer, UNO_QUERY_THROW );
                        xNameContainer->insertByName( sOldName, makeAny( xComponent ) );
                    }
                    catch(Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }


    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"ODocumentDefinition::save: caught an Exception (tried to let the InteractionHandler handle it)!");
    }
    return sal_True;
}

namespace
{
    // .........................................................................
    void    lcl_putLoadArgs( ::comphelper::NamedValueCollection& _io_rArgs, const bool _bSuppressMacros, const bool _bReadOnly)
    {
        if ( _bSuppressMacros )
        {
            // if we're to suppress macros, do exactly this
            _io_rArgs.put( "MacroExecutionMode", MacroExecMode::NEVER_EXECUTE );
        }
        else
        {
            // otherwise, put the setting only if not already present
            if ( !_io_rArgs.has( "MacroExecutionMode" ) )
            {
                _io_rArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );
            }
        }

        _io_rArgs.put( "ReadOnly", _bReadOnly );
    }
}

// -----------------------------------------------------------------------------
namespace
{
    Reference< XFrame > lcl_getDatabaseDocumentFrame( ODatabaseModelImpl& _rImpl )
    {
        Reference< XModel > xDatabaseDocumentModel( _rImpl.getModel_noCreate() );

        Reference< XController > xDatabaseDocumentController;
        if ( xDatabaseDocumentModel.is() )
            xDatabaseDocumentController = xDatabaseDocumentModel->getCurrentController();

        Reference< XFrame > xFrame;
        if ( xDatabaseDocumentController.is() )
            xFrame = xDatabaseDocumentController->getFrame();

        return xFrame;
    }
}

// -----------------------------------------------------------------------------
sal_Bool ODocumentDefinition::objectSupportsEmbeddedScripts() const
{
//    bool bAllowDocumentMacros = !m_pImpl->m_pDataSource || m_pImpl->m_pDataSource->hasAnyObjectWithMacros();
    // TODO: revert to the disabled code. The current version is just to be able
    // to integrate an intermediate version of the CWS, which should behave as
    // if no macros in DB docs are allowed
    bool bAllowDocumentMacros = true;

    // if *any* of the objects of the database document already has macros, we continue to allow it
    // to have them, until the user did a migration.
    // If there are no macros, yet, we don't allow to create them

    return bAllowDocumentMacros;
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > ODocumentDefinition::fillLoadArgs( const Reference< XConnection>& _xConnection, const bool _bSuppressMacros, const bool _bReadOnly,
        const Sequence< PropertyValue >& _rAdditionalArgs, Sequence< PropertyValue >& _out_rEmbeddedObjectDescriptor )
{
    // .........................................................................
    // (re-)create interceptor, and put it into the descriptor of the embedded object
    if ( m_pInterceptor )
    {
        m_pInterceptor->dispose();
        m_pInterceptor->release();
        m_pInterceptor = NULL;
    }

    m_pInterceptor = new OInterceptor( this ,_bReadOnly);
    m_pInterceptor->acquire();
    Reference<XDispatchProviderInterceptor> xInterceptor = m_pInterceptor;

    ::comphelper::NamedValueCollection aEmbeddedDescriptor;
    aEmbeddedDescriptor.put( "OutplaceDispatchInterceptor", xInterceptor );

    // .........................................................................
    // create the OutplaceFrameProperties, and put them into the descriptor of the embedded object
    ::comphelper::NamedValueCollection OutplaceFrameProperties;
    OutplaceFrameProperties.put( "TopWindow", (sal_Bool)sal_True );

    Reference< XFrame > xParentFrame;
    if ( m_pImpl->m_pDataSource )
        xParentFrame = lcl_getDatabaseDocumentFrame( *m_pImpl->m_pDataSource );
    OSL_ENSURE( xParentFrame.is(), "ODocumentDefinition::fillLoadArgs: no parent frame!" );
    if  ( xParentFrame.is() )
        OutplaceFrameProperties.put( "ParentFrame", xParentFrame );

    aEmbeddedDescriptor.put( "OutplaceFrameProperties", OutplaceFrameProperties.getNamedValues() );

    // .........................................................................
    // tell the embedded object to have (or not have) script support
    aEmbeddedDescriptor.put( "EmbeddedScriptSupport", (sal_Bool)objectSupportsEmbeddedScripts() );

    // .........................................................................
    // pass the descriptor of the embedded object to the caller
    aEmbeddedDescriptor >>= _out_rEmbeddedObjectDescriptor;

    // .........................................................................
    ::comphelper::NamedValueCollection aMediaDesc( _rAdditionalArgs );

    // .........................................................................
    // create the ComponentData, and put it into the document's media descriptor
    {
        ::comphelper::NamedValueCollection aComponentData;
        aComponentData.put( "ActiveConnection", _xConnection );
        aComponentData.put( "ApplyFormDesignMode", !_bReadOnly );
        aMediaDesc.put( "ComponentData", aComponentData.getPropertyValues() );
    }

    // .........................................................................
    // put the common load arguments into the document's media descriptor
    lcl_putLoadArgs( aMediaDesc, _bSuppressMacros, _bReadOnly, m_pImpl->m_aProps.aTitle );

    return aMediaDesc.getPropertyValues();
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::loadEmbeddedObject( const Reference< XConnection >& _xConnection, const Sequence< sal_Int8 >& _aClassID,
        const Sequence< PropertyValue >& _rAdditionalArgs, const bool _bSuppressMacros, const bool _bReadOnly )
{
    if ( !m_xEmbeddedObject.is() )
    {
        Reference< XStorage> xStorage = getContainerStorage();
        if ( xStorage.is() )
        {
            Reference< XEmbedObjectFactory> xEmbedFactory( m_aContext.createComponent( "com.sun.star.embed.OOoEmbeddedObjectFactory" ), UNO_QUERY );
            if ( xEmbedFactory.is() )
            {
                ::rtl::OUString sDocumentService;
                sal_Bool bSetSize = sal_False;
                sal_Int32 nEntryConnectionMode = EntryInitModes::DEFAULT_INIT;
                Sequence< sal_Int8 > aClassID = _aClassID;
                if ( aClassID.getLength() )
                {
                    nEntryConnectionMode = EntryInitModes::TRUNCATE_INIT;
                    bSetSize = sal_True;
                }
                else
                {
                    sDocumentService = GetDocumentServiceFromMediaType( xStorage, m_pImpl->m_aProps.sPersistentName, m_aContext.getLegacyServiceFactory(), aClassID );
                    // check if we are not a form and
                    // the com.sun.star.report.pentaho.SOReportJobFactory is not present.
                    if ( !m_bForm && !sDocumentService.equalsAscii("com.sun.star.text.TextDocument"))
                    {
                        // we seems to be a new report, check if report extension is present.
                        Reference< XContentEnumerationAccess > xEnumAccess( m_aContext.getLegacyServiceFactory(), UNO_QUERY );
                        const ::rtl::OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(m_aContext.getLegacyServiceFactory());
                        Reference< XEnumeration > xEnumDrivers = xEnumAccess->createContentEnumeration(sReportEngineServiceName);
                        if ( !xEnumDrivers.is() || !xEnumDrivers->hasMoreElements() )
                        {
                            com::sun::star::io::WrongFormatException aWFE;
                            aWFE.Message = ::rtl::OUString::createFromAscii("Extension not present.");
                            throw aWFE;
                        }
                    }
                    if ( !aClassID.getLength() )
                    {
                        if ( m_bForm )
                            aClassID = MimeConfigurationHelper::GetSequenceClassID(SO3_SW_CLASSID);
                        else
                        {
                            aClassID = MimeConfigurationHelper::GetSequenceClassID(SO3_RPT_CLASSID_90);
                        }
                    }
                }

                OSL_ENSURE( aClassID.getLength(),"No Class ID" );

                Sequence< PropertyValue > aEmbeddedObjectDescriptor;
                Sequence< PropertyValue > aLoadArgs( fillLoadArgs(
                    _xConnection, _bSuppressMacros, _bReadOnly, _rAdditionalArgs, aEmbeddedObjectDescriptor ) );

                m_xEmbeddedObject.set(xEmbedFactory->createInstanceUserInit(aClassID
                                                                            ,sDocumentService
                                                                            ,xStorage
                                                                            ,m_pImpl->m_aProps.sPersistentName
                                                                            ,nEntryConnectionMode
                                                                            ,aLoadArgs
                                                                            ,aEmbeddedObjectDescriptor
                                                                            ),UNO_QUERY);
                if ( m_xEmbeddedObject.is() )
                {
                    if ( !m_pClientHelper )
                    {
                        m_pClientHelper = new OEmbeddedClientHelper(this);
                        m_pClientHelper->acquire();
                    }
                    Reference<XEmbeddedClient> xClient = m_pClientHelper;
                    m_xEmbeddedObject->setClientSite(xClient);
                    m_xEmbeddedObject->changeState(EmbedStates::RUNNING);
                    if ( bSetSize )
                    {
                        ::com::sun::star::awt::Size aSize;
                        aSize.Width = DEFAULT_WIDTH;
                        aSize.Height = DEFAULT_HEIGHT;

                        m_xEmbeddedObject->setVisualAreaSize(Aspects::MSOLE_CONTENT,aSize);
                    }
                }
              }
        }
    }
    else if ( m_xEmbeddedObject->getCurrentState() == EmbedStates::LOADED )
    {
        if ( !m_pClientHelper )
        {
            m_pClientHelper = new OEmbeddedClientHelper(this);
            m_pClientHelper->acquire();
        }
        Reference<XEmbeddedClient> xClient = m_pClientHelper;
        m_xEmbeddedObject->setClientSite(xClient);

        Sequence< PropertyValue > aEmbeddedObjectDescriptor;
        Sequence< PropertyValue > aLoadArgs( fillLoadArgs(
            _xConnection, _bSuppressMacros, _bReadOnly, _rAdditionalArgs, aEmbeddedObjectDescriptor ) );

        Reference<XCommonEmbedPersist> xCommon(m_xEmbeddedObject,UNO_QUERY);
        OSL_ENSURE(xCommon.is(),"unsupported interface!");
        if ( xCommon.is() )
            xCommon->reload( aLoadArgs, aEmbeddedObjectDescriptor );
        m_xEmbeddedObject->changeState(EmbedStates::RUNNING);
    }

    Reference<XModel> xModel(getComponent(),UNO_QUERY);
    // set the OfficeDatabaseDocument instance as parent of the embedded document
    // #i40358# / 2005-01-19 / frank.schoenheit@sun.com
    Reference< XChild > xDepdendDocAsChild( xModel, UNO_QUERY );
    if ( xDepdendDocAsChild.is() )
    {
        try
        {
            if ( !xDepdendDocAsChild->getParent().is() )
            {   // first encounter
                xDepdendDocAsChild->setParent( getDataSource( m_xParentContainer ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    if ( xModel.is() )
    {
        Sequence<PropertyValue> aArgs = xModel->getArgs();

        ::comphelper::NamedValueCollection aMediaDesc( aArgs );
        lcl_putLoadArgs( aMediaDesc, _bSuppressMacros, _bReadOnly);

        aMediaDesc >>= aArgs;
        xModel->attachResource( xModel->getURL(), aArgs );
    }
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::onCommandPreview(Any& _rImage)
{
    loadEmbeddedObjectForPreview();
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
void ODocumentDefinition::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::onCommandGetDocumentProperties( Any& _rProps )
{
    loadEmbeddedObjectForPreview();
    if ( m_xEmbeddedObject.is() )
    {
        try
        {
            Reference<XDocumentPropertiesSupplier> xDocSup(
                getComponent(), UNO_QUERY );
            if ( xDocSup.is() )
                _rProps <<= xDocSup->getDocumentProperties();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
void SAL_CALL ODocumentDefinition::rename( const ::rtl::OUString& _rNewName ) throw (SQLException, ElementExistException, RuntimeException)
{
    try
    {
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        if ( _rNewName.equals( m_pImpl->m_aProps.aTitle ) )
            return;

        // document definitions are organized in a hierarchical way, so reject names
        // which contain a /, as this is reserved for hierarchy level separation
        if ( _rNewName.indexOf( '/' ) != -1 )
            m_aErrorHelper.raiseException( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES, *this );

        sal_Int32 nHandle = PROPERTY_ID_NAME;
        Any aOld = makeAny( m_pImpl->m_aProps.aTitle );
        Any aNew = makeAny( _rNewName );

        aGuard.clear();
        fire(&nHandle, &aNew, &aOld, 1, sal_True );
        m_pImpl->m_aProps.aTitle = _rNewName;
        fire(&nHandle, &aNew, &aOld, 1, sal_False );

        ::osl::ClearableGuard< ::osl::Mutex > aGuard2( m_aMutex );
        if ( m_xEmbeddedObject.is() && m_xEmbeddedObject->getCurrentState() == EmbedStates::ACTIVE )
            updateDocumentTitle();
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(_rNewName,*this);
    }
}
// -----------------------------------------------------------------------------
Reference< XStorage> ODocumentDefinition::getContainerStorage() const
{
    return  m_pImpl->m_pDataSource
        ?   m_pImpl->m_pDataSource->getStorage( ODatabaseModelImpl::getObjectContainerStorageName( m_bForm ? ODatabaseModelImpl::E_FORM : ODatabaseModelImpl::E_REPORT ) )
        :   Reference< XStorage>();
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
bool ODocumentDefinition::prepareClose()
{
    if ( !m_xEmbeddedObject.is() )
        return true;

    try
    {
        // suspend the controller. Embedded objects are not allowed to rais
        // own UI on their own decision, instead, this has always to be triggered
        // by the embedding component. Thus, we do the suspend call here.
        // #i49370# / 2005-06-09 / frank.schoenheit@sun.com

        Reference< XModel > xModel( getComponent(), UNO_QUERY );
        Reference< XController > xController;
        if ( xModel.is() )
            xController = xModel->getCurrentController();
        OSL_ENSURE( xController.is(), "ODocumentDefinition::prepareClose: no controller!" );
        if ( !xController.is() )
            return sal_False;

        sal_Bool bCouldSuspend = xController->suspend( sal_True );
        if ( !bCouldSuspend )
            // controller vetoed the closing
            return false;

        if ( isModified() )
        {
            Reference< XFrame > xFrame( xController->getFrame() );
            if ( xFrame.is() )
            {
                Reference< XTopWindow > xTopWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
                xTopWindow->toFront();
            }
            if ( !save( sal_True ) )
            {
                if ( bCouldSuspend )
                    // revert suspension
                    xController->suspend( sal_False );
                // saving failed or was cancelled
                return false;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return true;
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::fillReportData()
{
    if ( !m_bForm && m_pImpl->m_aProps.bAsTemplate && !m_bOpenInDesign ) // open a report in alive mode, so we need to fill it
    {
        Sequence<Any> aArgs(2);
        PropertyValue aValue;
        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TextDocument"));
        aValue.Value <<= getComponent();
        aArgs[0] <<= aValue;
           aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection"));
           aValue.Value <<= m_xLastKnownConnection;
           aArgs[1] <<= aValue;

        Reference< XJobExecutor > xExecuteable( m_aContext.createComponentWithArguments( "com.sun.star.wizards.report.CallReportWizard", aArgs ), UNO_QUERY );
        if ( xExecuteable.is() )
            xExecuteable->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("fill")));
    }
}
// -----------------------------------------------------------------------------
void ODocumentDefinition::updateDocumentTitle()
{
    ::rtl::OUString sName = m_pImpl->m_aProps.aTitle;
    if ( m_pImpl->m_pDataSource )
    {
        if ( !sName.getLength() )
        {
            if ( m_bForm )
                sName = DBACORE_RESSTRING( RID_STR_FORM );
            else
                sName = DBACORE_RESSTRING( RID_STR_REPORT );
            Reference< XUntitledNumbers > xUntitledProvider(m_pImpl->m_pDataSource->getModel_noCreate(), UNO_QUERY      );
            if ( xUntitledProvider.is() )
                sName += ::rtl::OUString::valueOf( xUntitledProvider->leaseNumber(getComponent()) );
        }

        Reference< XTitle > xDatabaseDocumentModel(m_pImpl->m_pDataSource->getModel_noCreate(),uno::UNO_QUERY);
        if ( xDatabaseDocumentModel.is() )
            sName = xDatabaseDocumentModel->getTitle() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" : ")) + sName;
    }
    Reference< XTitle> xTitle(getComponent(),UNO_QUERY);
    if ( xTitle.is() )
        xTitle->setTitle(sName);
}
//........................................................................
}   // namespace dbaccess
//........................................................................

