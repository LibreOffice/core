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

#include "documentdefinition.hxx"
#include <stringconstants.hxx>
#include <sdbcoretools.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XReportEngine.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#include <com/sun/star/embed/EmbeddedObjectCreator.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/OOoEmbeddedObjectFactory.hpp>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XCommonEmbedPersist.hpp>
#include "intercept.hxx"
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdb/XInteractionDocumentSave.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/sdb/DocumentSaveRequest.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <sal/macros.h>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <core_resource.hxx>
#include <strings.hrc>
#include "datasource.hxx"
#include <com/sun/star/embed/XStateChangeBroadcaster.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/util/XModifiable2.hpp>

using namespace ::com::sun::star;
using namespace view;
using namespace uno;
using namespace util;
using namespace ucb;
using namespace beans;
using namespace lang;
using namespace awt;
using namespace embed;
using namespace frame;
using namespace document;
using namespace sdbc;
using namespace sdb;
using namespace io;
using namespace container;
using namespace datatransfer;
using namespace task;
using namespace form;
using namespace drawing;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

using sdb::application::XDatabaseDocumentUI;
namespace DatabaseObject = sdb::application::DatabaseObject;

#define DEFAULT_WIDTH  10000
#define DEFAULT_HEIGHT  7500

namespace dbaccess
{

    typedef ::boost::optional< bool > optional_bool;

    // helper
    namespace
    {
        OUString lcl_determineContentType_nothrow( const Reference< XStorage >& _rxContainerStorage,
            const OUString& _rEntityName )
        {
            OUString sContentType;
            try
            {
                ::utl::SharedUNOComponent< XPropertySet > xStorageProps(
                    _rxContainerStorage->openStorageElement( _rEntityName, ElementModes::READ ), UNO_QUERY_THROW );
                OSL_VERIFY( xStorageProps->getPropertyValue( INFO_MEDIATYPE ) >>= sContentType );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
            return sContentType;
        }
    }

    // OEmbedObjectHolder
    typedef ::cppu::WeakComponentImplHelper<   embed::XStateChangeListener > TEmbedObjectHolder;
    class OEmbedObjectHolder :   public ::cppu::BaseMutex
                                ,public TEmbedObjectHolder
    {
        Reference< XEmbeddedObject >    m_xBroadCaster;
        ODocumentDefinition*            m_pDefinition;
        bool                            m_bInStateChange;
    protected:
        virtual void SAL_CALL disposing() override;
    public:
        OEmbedObjectHolder(const Reference< XEmbeddedObject >& _xBroadCaster,ODocumentDefinition* _pDefinition)
            : TEmbedObjectHolder(m_aMutex)
            ,m_xBroadCaster(_xBroadCaster)
            ,m_pDefinition(_pDefinition)
            ,m_bInStateChange(false)
        {
            osl_atomic_increment( &m_refCount );
            {
                if ( m_xBroadCaster.is() )
                    m_xBroadCaster->addStateChangeListener(this);
            }
            osl_atomic_decrement( &m_refCount );
        }

        virtual void SAL_CALL changingState( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
        virtual void SAL_CALL stateChanged( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
        virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;
    };

    void SAL_CALL OEmbedObjectHolder::disposing()
    {
        if ( m_xBroadCaster.is() )
            m_xBroadCaster->removeStateChangeListener(this);
        m_xBroadCaster = nullptr;
        m_pDefinition = nullptr;
    }

    void SAL_CALL OEmbedObjectHolder::changingState( const lang::EventObject& /*aEvent*/, ::sal_Int32 /*nOldState*/, ::sal_Int32 /*nNewState*/ )
    {
    }

    void SAL_CALL OEmbedObjectHolder::stateChanged( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState )
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

    void SAL_CALL OEmbedObjectHolder::disposing( const lang::EventObject& /*Source*/ )
    {
        m_xBroadCaster = nullptr;
    }

    // OEmbeddedClientHelper
    class OEmbeddedClientHelper : public ::cppu::WeakImplHelper<XEmbeddedClient>
    {
    public:
        virtual void SAL_CALL saveObject(  ) override
        {
        }
        // XComponentSupplier
        virtual Reference< util::XCloseable > SAL_CALL getComponent(  ) override
        {
            return Reference< css::util::XCloseable >();
        }

        // XEmbeddedClient
        virtual void SAL_CALL visibilityChanged( sal_Bool /*bVisible*/ ) override
        {
        }
    };

    // LockModifiable
    class LockModifiable
    {
    public:
        explicit LockModifiable( const Reference< XInterface >& i_rModifiable )
            :m_xModifiable( i_rModifiable, UNO_QUERY )
        {
            OSL_ENSURE( m_xModifiable.is(), "LockModifiable::LockModifiable: invalid component!" );
            if ( m_xModifiable.is() )
            {
                if ( !m_xModifiable->isSetModifiedEnabled() )
                {
                    // somebody already locked that, no need to lock it, again, and no need to unlock it later
                    m_xModifiable.clear();
                }
                else
                {
                    m_xModifiable->disableSetModified();
                }
            }
        }

        ~LockModifiable()
        {
            if ( m_xModifiable.is() )
                m_xModifiable->enableSetModified();
        }

    private:
        Reference< XModifiable2 >   m_xModifiable;
    };

    // LifetimeCoupler
    typedef ::cppu::WeakImplHelper<   css::lang::XEventListener
                                  >   LifetimeCoupler_Base;
    /** helper class which couples the lifetime of a component to the lifetime
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
        static void couple( const Reference< XInterface >& _rxClient, const Reference< XComponent >& _rxActor )
        {
            Reference< css::lang::XEventListener > xEnsureDelete( new LifetimeCoupler( _rxClient, _rxActor ) );
        }

    private:
        LifetimeCoupler( const Reference< XInterface >& _rxClient, const Reference< XComponent >& _rxActor )
            :m_xClient( _rxClient )
        {
            OSL_ENSURE( _rxActor.is(), "LifetimeCoupler::LifetimeCoupler: this will crash!" );
            osl_atomic_increment( &m_refCount );
            {
                _rxActor->addEventListener( this );
            }
            osl_atomic_decrement( &m_refCount );
            OSL_ENSURE( m_refCount, "LifetimeCoupler::LifetimeCoupler: the actor is not holding us by hard ref - this won't work!" );
        }

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    protected:
    };

    void SAL_CALL LifetimeCoupler::disposing( const css::lang::EventObject& /*Source*/ )
    {
        m_xClient.clear();
    }

    // ODocumentSaveContinuation
    class ODocumentSaveContinuation : public OInteraction< XInteractionDocumentSave >
    {
        OUString     m_sName;
        Reference<XContent> m_xParentContainer;

    public:
        ODocumentSaveContinuation() { }

        const Reference<XContent>& getContent() const { return m_xParentContainer; }
        const OUString&       getName() const { return m_sName; }

        // XInteractionDocumentSave
        virtual void SAL_CALL setName( const OUString& _sName,const Reference<XContent>& _xParent) override;
    };

    void SAL_CALL ODocumentSaveContinuation::setName( const OUString& _sName,const Reference<XContent>& _xParent)
    {
        m_sName = _sName;
        m_xParentContainer = _xParent;
    }

OUString ODocumentDefinition::GetDocumentServiceFromMediaType( const Reference< XStorage >& _rxContainerStorage,
    const OUString& _rEntityName, const Reference< XComponentContext >& _rContext,
    Sequence< sal_Int8 >& _rClassId )
{
    return GetDocumentServiceFromMediaType(
        lcl_determineContentType_nothrow( _rxContainerStorage, _rEntityName ),
        _rContext, _rClassId );
}

OUString ODocumentDefinition::GetDocumentServiceFromMediaType( const OUString& _rMediaType,
    const Reference< XComponentContext >& _rContext, Sequence< sal_Int8 >& _rClassId )
{
    OUString sResult;
    try
    {
        ::comphelper::MimeConfigurationHelper aConfigHelper( _rContext );
        sResult = aConfigHelper.GetDocServiceNameFromMediaType( _rMediaType );
        _rClassId = comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation(aConfigHelper.GetExplicitlyRegisteredObjClassID( _rMediaType ));
        if ( !_rClassId.getLength() && !sResult.isEmpty() )
        {
            Reference< XNameAccess > xObjConfig = aConfigHelper.GetObjConfiguration();
            if ( xObjConfig.is() )
            {
                Sequence< OUString > aClassIDs = xObjConfig->getElementNames();
                for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                {
                    Reference< XNameAccess > xObjectProps;
                    OUString aEntryDocName;

                    if (    ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                         && ( xObjectProps->getByName("ObjectDocumentServiceName") >>= aEntryDocName )
                         && aEntryDocName == sResult )
                    {
                        _rClassId = comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation(aClassIDs[nInd]);
                        break;
                    }
                }
            }
        }
#if OSL_DEBUG_LEVEL > 0
        // alternative, shorter approach
        const Sequence< NamedValue > aProps( aConfigHelper.GetObjectPropsByMediaType( _rMediaType ) );
        const ::comphelper::NamedValueCollection aMediaTypeProps( aProps );
        const OUString sAlternativeResult = aMediaTypeProps.getOrDefault( "ObjectDocumentServiceName", OUString() );
        OSL_ENSURE( sAlternativeResult == sResult, "ODocumentDefinition::GetDocumentServiceFromMediaType: failed, this approach is *not* equivalent (1)!" );
        const Sequence< sal_Int8 > aAlternativeClassID = aMediaTypeProps.getOrDefault( "ClassID", Sequence< sal_Int8 >() );
        OSL_ENSURE( aAlternativeClassID == _rClassId, "ODocumentDefinition::GetDocumentServiceFromMediaType: failed, this approach is *not* equivalent (2)!" );
#endif
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return sResult;
}

// ODocumentDefinition

ODocumentDefinition::ODocumentDefinition( const Reference< XInterface >& _rxContainer, const Reference< XComponentContext >& _xORB,
                                          const TContentPtr& _pImpl, bool _bForm )
    :OContentHelper(_xORB,_rxContainer,_pImpl)
    ,OPropertyStateContainer(OContentHelper::rBHelper)
    ,m_bForm(_bForm)
    ,m_bOpenInDesign(false)
    ,m_bInExecute(false)
    ,m_bRemoveListener(false)
{
    registerProperties();
}

void ODocumentDefinition::initialLoad( const Sequence< sal_Int8 >& i_rClassID, const Sequence< PropertyValue >& i_rCreationArgs,
                                       const Reference< XConnection >& i_rConnection )
{
    OSL_ENSURE( i_rClassID.getLength(), "ODocumentDefinition::initialLoad: illegal class ID!" );
    if ( !i_rClassID.getLength() )
        return;

    loadEmbeddedObject( i_rConnection, i_rClassID, i_rCreationArgs, false, false );
}

ODocumentDefinition::~ODocumentDefinition()
{
    if ( !OContentHelper::rBHelper.bInDispose && !OContentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    if ( m_pInterceptor.is() )
    {
        m_pInterceptor->dispose();
        m_pInterceptor.clear();
    }
}

void ODocumentDefinition::closeObject()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_xEmbeddedObject.is() )
    {
        try
        {
            Reference< css::util::XCloseable> xCloseable(m_xEmbeddedObject,UNO_QUERY);
            if ( xCloseable.is() )
                xCloseable->close(true);
        }
        catch(const Exception&)
        {
        }
        m_xEmbeddedObject = nullptr;
        m_pClientHelper.clear();
    }
}

void SAL_CALL ODocumentDefinition::disposing()
{
    OContentHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    closeObject();
    ::comphelper::disposeComponent(m_xListener);
    if ( m_bRemoveListener )
    {
        Reference<util::XCloseable> xCloseable(m_pImpl->m_pDataSource->getModel_noCreate(),UNO_QUERY);
        if ( xCloseable.is() )
            xCloseable->removeCloseListener(this);
    }
}

css::uno::Sequence<sal_Int8> ODocumentDefinition::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES3(ODocumentDefinition,OContentHelper,OPropertyStateContainer,ODocumentDefinition_Base);
IMPLEMENT_FORWARD_XINTERFACE3( ODocumentDefinition,OContentHelper,OPropertyStateContainer,ODocumentDefinition_Base)

void ODocumentDefinition::registerProperties()
{
#define REGISTER_PROPERTY( name, location ) \
    registerProperty(   PROPERTY_##name, PROPERTY_ID_##name, PropertyAttribute::READONLY, &location, cppu::UnoType<decltype(location)>::get() );

#define REGISTER_PROPERTY_BV( name, location ) \
    registerProperty(   PROPERTY_##name, PROPERTY_ID_##name, PropertyAttribute::CONSTRAINED | PropertyAttribute::BOUND | PropertyAttribute::READONLY, &location, cppu::UnoType<decltype(location)>::get() );

    REGISTER_PROPERTY_BV( NAME,            m_pImpl->m_aProps.aTitle            );
    REGISTER_PROPERTY   ( AS_TEMPLATE,     m_pImpl->m_aProps.bAsTemplate       );
    REGISTER_PROPERTY   ( PERSISTENT_NAME, m_pImpl->m_aProps.sPersistentName   );
    REGISTER_PROPERTY   ( IS_FORM,         m_bForm                             );
}

void SAL_CALL ODocumentDefinition::getFastPropertyValue( Any& o_rValue, sal_Int32 i_nHandle ) const
{
    if ( i_nHandle == PROPERTY_ID_PERSISTENT_PATH )
    {
        OUString sPersistentPath;
        if ( !m_pImpl->m_aProps.sPersistentName.isEmpty() )
        {
            OUStringBuffer aBuffer;
            aBuffer.append( ODatabaseModelImpl::getObjectContainerStorageName( m_bForm ? ODatabaseModelImpl::E_FORM : ODatabaseModelImpl::E_REPORT ) );
            aBuffer.append( '/' );
            aBuffer.append( m_pImpl->m_aProps.sPersistentName );
            sPersistentPath = aBuffer.makeStringAndClear();
        }
        o_rValue <<= sPersistentPath;
        return;
    }

    OPropertyStateContainer::getFastPropertyValue( o_rValue, i_nHandle );
}

Reference< XPropertySetInfo > SAL_CALL ODocumentDefinition::getPropertySetInfo(  )
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

IPropertyArrayHelper& ODocumentDefinition::getInfoHelper()
{
    return *getArrayHelper();
}

IPropertyArrayHelper* ODocumentDefinition::createArrayHelper( ) const
{
    // properties maintained by our base class (see registerProperties)
    Sequence< Property > aProps;
    describeProperties( aProps );

    // properties not maintained by our base class
    Sequence< Property > aManualProps( 1 );
    aManualProps[0].Name = PROPERTY_PERSISTENT_PATH;
    aManualProps[0].Handle = PROPERTY_ID_PERSISTENT_PATH;
    aManualProps[0].Type = ::cppu::UnoType<OUString>::get();
    aManualProps[0].Attributes = PropertyAttribute::READONLY;

    return new OPropertyArrayHelper( ::comphelper::concatSequences( aProps, aManualProps ) );
}

class OExecuteImpl
{
    bool& m_rbSet;
public:
    explicit OExecuteImpl(bool& _rbSet) : m_rbSet(_rbSet){ m_rbSet=true; }
    ~OExecuteImpl(){ m_rbSet = false; }
};

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

void ODocumentDefinition::impl_removeFrameFromDesktop_throw( const Reference<XComponentContext> & _rxContext, const Reference< XFrame >& _rxFrame )
{
    Reference< XDesktop2 > xDesktop = Desktop::create( _rxContext );
    Reference< XFrames > xFrames( xDesktop->getFrames(), UNO_QUERY_THROW );
    xFrames->remove( _rxFrame );
}

void ODocumentDefinition::impl_onActivateEmbeddedObject_nothrow( const bool i_bReactivated )
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
            m_xListener = new OEmbedObjectHolder( m_xEmbeddedObject, this );

        // raise the window to top (especially necessary if this is not the first activation)
        Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
        Reference< XTopWindow > xTopWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        xTopWindow->toFront();

        // remove the frame from the desktop's frame collection because we need full control of it.
        impl_removeFrameFromDesktop_throw( m_aContext, xFrame );

        // ensure that we ourself are kept alive as long as the embedded object's frame is
        // opened
        LifetimeCoupler::couple( *this, xFrame.get() );

        // init the edit view
        if ( m_bForm && m_bOpenInDesign && !i_bReactivated )
            impl_initFormEditView( xController );
    }
    catch( const RuntimeException& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

namespace
{
    // PreserveVisualAreaSize
    /** stack-guard for preserving the size of the VisArea of an XModel
    */
    class PreserveVisualAreaSize
    {
    private:
        Reference< XVisualObject >  m_xVisObject;
        awt::Size m_aOriginalSize;

    public:
        explicit PreserveVisualAreaSize( const Reference< XModel >& _rxModel )
            :m_xVisObject( _rxModel, UNO_QUERY )
        {
            if ( m_xVisObject.is() )
            {
                try
                {
                    m_aOriginalSize = m_xVisObject->getVisualAreaSize( Aspects::MSOLE_CONTENT );
                }
                catch ( const Exception& )
                {
                    OSL_FAIL( "PreserveVisualAreaSize::PreserveVisualAreaSize: caught an exception!" );
                }
            }
        }

        ~PreserveVisualAreaSize()
        {
            if ( m_xVisObject.is() && m_aOriginalSize.Width && m_aOriginalSize.Height )
            {
                try
                {
                    m_xVisObject->setVisualAreaSize( Aspects::MSOLE_CONTENT, m_aOriginalSize );
                }
                catch ( const Exception& )
                {
                    OSL_FAIL( "PreserveVisualAreaSize::~PreserveVisualAreaSize: caught an exception!" );
                }
            }
        }
    };

    // LayoutManagerLock
    /** helper class for stack-usage which during its lifetime locks a layout manager
    */
    class LayoutManagerLock
    {
    private:
        Reference< XLayoutManager > m_xLayoutManager;

    public:
        explicit LayoutManagerLock( const Reference< XController >& _rxController )
        {
            OSL_ENSURE( _rxController.is(), "LayoutManagerLock::LayoutManagerLock: this will crash!" );
            Reference< XFrame > xFrame( _rxController->getFrame() );
            try
            {
                Reference< XPropertySet > xPropSet( xFrame, UNO_QUERY_THROW );
                m_xLayoutManager.set(
                    xPropSet->getPropertyValue( "LayoutManager" ),
                    UNO_QUERY_THROW );
                m_xLayoutManager->lock();

            }
            catch( const Exception& )
            {
                OSL_FAIL( "LayoutManagerLock::LayoutManagerLock: caught an exception!" );
            }
        }

        ~LayoutManagerLock()
        {
            try
            {
                // unlock the layout manager
                if ( m_xLayoutManager.is() )
                    m_xLayoutManager->unlock();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "LayoutManagerLock::~LayoutManagerLock: caught an exception!" );
            }
        }
    };
}

void ODocumentDefinition::impl_initFormEditView( const Reference< XController >& _rxController )
{
    try
    {
        Reference< XViewSettingsSupplier > xSettingsSupplier( _rxController, UNO_QUERY_THROW );
        Reference< XPropertySet > xViewSettings( xSettingsSupplier->getViewSettings(), UNO_QUERY_THROW );

        // the below code could indirectly tamper with the "modified" flag of the model, temporarily disable this
        LockModifiable aLockModify( _rxController->getModel() );

        // The visual area size can be changed by the setting of the following properties
        // so it should be restored later
        PreserveVisualAreaSize aPreserveVisAreaSize( _rxController->getModel() );

        // Layout manager should not layout while the size is still not restored
        // so it will stay locked for this time
        LayoutManagerLock aLockLayout( _rxController );

        // setting of the visual properties
        xViewSettings->setPropertyValue("ShowRulers",makeAny(true));
        xViewSettings->setPropertyValue("ShowVertRuler",makeAny(true));
        xViewSettings->setPropertyValue("ShowHoriRuler",makeAny(true));
        xViewSettings->setPropertyValue("IsRasterVisible",makeAny(true));
        xViewSettings->setPropertyValue("IsSnapToRaster",makeAny(true));
        xViewSettings->setPropertyValue("ShowOnlineLayout",makeAny(true));
        xViewSettings->setPropertyValue("RasterSubdivisionX",makeAny(sal_Int32(5)));
        xViewSettings->setPropertyValue("RasterSubdivisionY",makeAny(sal_Int32(5)));
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void ODocumentDefinition::impl_showOrHideComponent_throw( const bool i_bShow )
{
    const sal_Int32 nCurrentState = m_xEmbeddedObject.is() ? m_xEmbeddedObject->getCurrentState() : EmbedStates::LOADED;
    switch ( nCurrentState )
    {
    default:
    case EmbedStates::LOADED:
        throw embed::WrongStateException( OUString(), *this );

    case EmbedStates::RUNNING:
        if ( !i_bShow )
            // fine, a running (and not yet active) object is never visible
            return;
        {
            LockModifiable aLockModify( impl_getComponent_throw() );
            m_xEmbeddedObject->changeState( EmbedStates::ACTIVE );
            impl_onActivateEmbeddedObject_nothrow( false );
        }
        break;

    case EmbedStates::ACTIVE:
    {
        Reference< XModel > xEmbeddedDoc( impl_getComponent_throw(), UNO_QUERY_THROW );
        Reference< XController > xEmbeddedController( xEmbeddedDoc->getCurrentController(), UNO_SET_THROW );
        Reference< XFrame > xEmbeddedFrame( xEmbeddedController->getFrame(), UNO_SET_THROW );
        Reference< XWindow > xEmbeddedWindow( xEmbeddedFrame->getContainerWindow(), UNO_SET_THROW );
        xEmbeddedWindow->setVisible( i_bShow );
    }
    break;
    }
}

Any ODocumentDefinition::onCommandOpenSomething( const Any& _rOpenArgument, const bool _bActivate,
        const Reference< XCommandEnvironment >& _rxEnvironment )
{
    OExecuteImpl aExecuteGuard( m_bInExecute );

    Reference< XConnection > xConnection;
    sal_Int32 nOpenMode = OpenMode::DOCUMENT;

    ::comphelper::NamedValueCollection aDocumentArgs;

    // for the document, default to the interaction handler as used for loading the DB doc
    // This might be overwritten below, when examining _rOpenArgument.
    const ::comphelper::NamedValueCollection& aDBDocArgs( m_pImpl->m_pDataSource->getMediaDescriptor() );
    Reference< XInteractionHandler > xHandler( aDBDocArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );
    if ( xHandler.is() )
        aDocumentArgs.put( "InteractionHandler", xHandler );

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

                if ( pIter->Name == "MacroExecutionMode" )
                {
                    sal_Int16 nMacroExecMode( !aDocumentMacroMode ? MacroExecMode::USE_CONFIG : *aDocumentMacroMode );
                    OSL_VERIFY( pIter->Value >>= nMacroExecMode );
                    aDocumentMacroMode = nMacroExecMode;
                    continue;
                }

                // unknown argument -> pass to the loaded document
                aDocumentArgs.put( pIter->Name, pIter->Value );
            }
        }
    }

    bool bExecuteDBDocMacros = m_pImpl->m_pDataSource->checkMacrosOnLoading();
        // Note that this call implies the user might be asked for the macro execution mode.
        // Normally, this would happen when the database document is loaded, and subsequent calls
        // will simply use the user's decision from this point in time.
        // However, it is possible to programmatically load forms/reports, without actually
        // loading the database document into a frame. In this case, the user will be asked
        // here and now.
        // #i87741#

    // allow the command arguments to downgrade the macro execution mode, but not to upgrade
    // it
    if  (   ( m_pImpl->m_pDataSource->getImposedMacroExecMode() == MacroExecMode::USE_CONFIG )
        &&  bExecuteDBDocMacros
        )
    {
        // while loading the whole database document, USE_CONFIG, was passed.
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
        if ( m_pImpl->m_pDataSource->determineEmbeddedMacros() == ODatabaseModelImpl::eNoMacros )
        {
            // this is case 2. from above
            // So, pass a USE_CONFIG to the to-be-loaded document. This means that
            // the user will be prompted with a security message upon opening this
            // sub document, in case the settings require this, *and* the document
            // contains scripts in the content.xml. But this is better than the security
            // issue we had before ...
            aDocumentMacroMode = MacroExecMode::USE_CONFIG;
        }
    }

    if ( !aDocumentMacroMode )
    {
        // nobody so far felt responsible for setting it
        // => use the DBDoc-wide macro exec mode for the document, too
        aDocumentMacroMode = bExecuteDBDocMacros ? MacroExecMode::ALWAYS_EXECUTE_NO_WARN
                                                 : MacroExecMode::NEVER_EXECUTE;
    }
    aDocumentArgs.put( "MacroExecutionMode", *aDocumentMacroMode );

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
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                sal_Int16( nOpenMode ) ) ),
                _rxEnvironment );
        // Unreachable
        OSL_FAIL( "unreachable" );
      }

    OSL_ENSURE( !m_pImpl->m_aProps.sPersistentName.isEmpty(),
        "ODocumentDefinition::onCommandOpenSomething: no persistent name - cannot load!" );
    if ( m_pImpl->m_aProps.sPersistentName.isEmpty() )
        return Any();

    // embedded objects themself do not support the hidden flag. We implement support for
    // it by changing the STATE to RUNNING only, instead of ACTIVE.
    bool bOpenHidden = aDocumentArgs.getOrDefault( "Hidden", false );
    aDocumentArgs.remove( "Hidden" );

    loadEmbeddedObject( xConnection, Sequence< sal_Int8 >(), aDocumentArgs.getPropertyValues(), false, !m_bOpenInDesign );
    OSL_ENSURE( m_xEmbeddedObject.is(), "ODocumentDefinition::onCommandOpenSomething: what's this?" );
    if ( !m_xEmbeddedObject.is() )
        return Any();

    Reference< XModel > xModel( getComponent(), UNO_QUERY );
    Reference< report::XReportDefinition > xReportDefinition(xModel,UNO_QUERY);

    Reference< XModule > xModule( xModel, UNO_QUERY );
    if ( xModule.is() )
    {
        if ( m_bForm )
            xModule->setIdentifier( "com.sun.star.sdb.FormDesign" );
        else if ( !xReportDefinition.is() )
            xModule->setIdentifier( "com.sun.star.text.TextDocument" );

        updateDocumentTitle();
    }

    bool bIsAliveNewStyleReport = ( !m_bOpenInDesign && xReportDefinition.is() );
    if ( bIsAliveNewStyleReport )
    {
        // we are in ReadOnly mode
        // we would like to open the Writer or Calc with the report direct, without design it.
        Reference< report::XReportEngine > xReportEngine( m_aContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.report.OReportEngineJFree", m_aContext), UNO_QUERY_THROW );

        xReportEngine->setReportDefinition(xReportDefinition);
        xReportEngine->setActiveConnection(m_xLastKnownConnection);
        if ( bOpenHidden )
            return makeAny( xReportEngine->createDocumentModel() );
        return makeAny( xReportEngine->createDocumentAlive( nullptr ) );
    }

    if ( _bActivate && !bOpenHidden )
    {
        LockModifiable aLockModify( impl_getComponent_throw() );
        m_xEmbeddedObject->changeState( EmbedStates::ACTIVE );
        impl_onActivateEmbeddedObject_nothrow( false );
    }
    else
    {
        // ensure that we ourself are kept alive as long as the document is open
        LifetimeCoupler::couple( *this, xModel.get() );
    }

    if ( !m_bForm && m_pImpl->m_aProps.bAsTemplate && !m_bOpenInDesign )
        ODocumentDefinition::fillReportData( m_aContext, getComponent(), xConnection );

    return makeAny( xModel );
}

Any SAL_CALL ODocumentDefinition::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment )
{
    Any aRet;

    bool bOpen = aCommand.Name == "open";
    bool bOpenInDesign = aCommand.Name == "openDesign";
    bool bOpenForMail = aCommand.Name == "openForMail";
    if ( bOpen || bOpenInDesign || bOpenForMail )
    {
        // opening the document involves a lot of VCL code, which is not thread-safe, but needs the SolarMutex locked.
        // Unfortunately, the DocumentDefinition, as well as the EmbeddedObject implementation, calls into VCL-dependent
        // components *without* releasing the own mutex, which is a guaranteed recipe for deadlocks.
        // We have control over this implementation here, and in modifying it to release the own mutex before calling into
        // the VCL-dependent components is not too difficult (was there, seen it).
        // However, we do /not/ have control over the EmbeddedObject implementation, and from a first look, it seems as
        // making it release the own mutex before calling SolarMutex-code is ... difficult, at least.
        // So, to be on the same side, we lock the SolarMutex here. Yes, it sucks.
        ::SolarMutexGuard aSolarGuard;
        ::osl::ClearableMutexGuard aGuard(m_aMutex);
        if ( m_bInExecute )
            return aRet;

        bool bActivateObject = true;
        if ( bOpenForMail )
        {
            OSL_FAIL( "ODocumentDefinition::execute: 'openForMail' should not be used anymore - use the 'Hidden' parameter instead!" );
            bActivateObject = false;
        }

        // if the object is already opened, do nothing
        if ( m_xEmbeddedObject.is() )
        {
            sal_Int32 nCurrentState = m_xEmbeddedObject->getCurrentState();
            bool bIsActive = ( nCurrentState == EmbedStates::ACTIVE );

            if ( bIsActive )
            {
                // exception: new-style reports always create a new document when "open" is executed
                Reference< report::XReportDefinition > xReportDefinition( impl_getComponent_throw( false ), UNO_QUERY );
                bool bIsAliveNewStyleReport = ( xReportDefinition.is() && ( bOpen || bOpenForMail ) );

                if ( !bIsAliveNewStyleReport )
                {
                    impl_onActivateEmbeddedObject_nothrow( true );
                    return makeAny( getComponent() );
                }
            }
        }

        m_bOpenInDesign = bOpenInDesign || bOpenForMail;
        return onCommandOpenSomething( aCommand.Argument, bActivateObject, Environment );
    }

    ::osl::ClearableMutexGuard aGuard(m_aMutex);
    if ( m_bInExecute )
        return aRet;

    if ( aCommand.Name == "copyTo" )
    {
        Sequence<Any> aIni;
        aCommand.Argument >>= aIni;
        if ( aIni.getLength() != 2 )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        Reference< XStorage> xDest(aIni[0],UNO_QUERY);
        OUString sPersistentName;
        aIni[1] >>= sPersistentName;
        Reference< XStorage> xStorage = getContainerStorage();

        xStorage->copyElementTo(m_pImpl->m_aProps.sPersistentName,xDest,sPersistentName);
    }
    else if ( aCommand.Name == "preview" )
    {
        onCommandPreview(aRet);
    }
    else if ( aCommand.Name == "insert" )
    {
        Sequence<Any> aIni;
        aCommand.Argument >>= aIni;
        if ( !aIni.getLength() )
        {
            OSL_FAIL( "Wrong argument count!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        OUString sURL;
        aIni[0] >>= sURL;
        onCommandInsert( sURL, Environment );
    }
    else if (   aCommand.Name == "getdocumentinfo"   // compatibility
            ||  aCommand.Name == "getDocumentInfo" )
    {
        onCommandGetDocumentProperties( aRet );
    }
    else if ( aCommand.Name == "delete" )
    {
        // delete
        closeObject();
        Reference< XStorage> xStorage = getContainerStorage();
        if ( xStorage.is() )
            xStorage->removeElement(m_pImpl->m_aProps.sPersistentName);

        dispose();

    }
    else if (   aCommand.Name == "storeOwn" // compatibility
            ||  aCommand.Name == "store"
            )
    {
        impl_store_throw();
    }
    else if (   aCommand.Name == "shutdown" // compatibility
            ||  aCommand.Name == "close"
            )
    {
        aRet <<= impl_close_throw();
    }
    else if ( aCommand.Name == "show" )
    {
        impl_showOrHideComponent_throw( true );
    }
    else if ( aCommand.Name == "hide" )
    {
        impl_showOrHideComponent_throw( false );
    }
    else
    {
        aRet = OContentHelper::execute(aCommand,CommandId,Environment);
    }

    return aRet;
}

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
                xFormProps->setPropertyValue( PROPERTY_DATASOURCENAME, makeAny( OUString() ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
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
            Reference< XDrawPageSupplier > xSuppPage( _rxEmbeddedObject->getComponent(), UNO_QUERY_THROW );
                // if this interface does not exist, then either getComponent returned NULL,
                // or the document is a multi-page document. The latter is allowed, but currently
                // simply not handled by this code, as it would not normally happen.

            Reference< XFormsSupplier > xSuppForms( xSuppPage->getDrawPage(), UNO_QUERY_THROW );
            Reference< XIndexAccess > xForms( xSuppForms->getForms(), UNO_QUERY_THROW );
            lcl_resetChildFormsToEmptyDataSource( xForms );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

    }
}

void ODocumentDefinition::onCommandInsert( const OUString& _sURL, const Reference< XCommandEnvironment >& Environment )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( _sURL.isEmpty() || m_xEmbeddedObject.is() )
    {
        OSL_FAIL( "Content::onCommandInsert - property value missing!" );

        Sequence<OUString> aProps { PROPERTY_URL };
        ucbhelper::cancelCommandExecution(
            makeAny( MissingPropertiesException(
                                OUString(),
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
            Reference< XEmbeddedObjectCreator> xEmbedFactory = EmbeddedObjectCreator::create(m_aContext);
            Sequence<PropertyValue> aEmpty,aMediaDesc(1);
            aMediaDesc[0].Name = PROPERTY_URL;
            aMediaDesc[0].Value <<= _sURL;
            m_xEmbeddedObject.set(xEmbedFactory->createInstanceInitFromMediaDescriptor( xStorage
                                                                            ,m_pImpl->m_aProps.sPersistentName
                                                                            ,aMediaDesc
                                                                            ,aEmpty),UNO_QUERY);

            lcl_resetFormsToEmptyDataSource( m_xEmbeddedObject );
            // #i57669#

            Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
            if ( xPersist.is() )
            {
                xPersist->storeOwn();
            }
            try
            {
                Reference< css::util::XCloseable> xCloseable(m_xEmbeddedObject,UNO_QUERY);
                if ( xCloseable.is() )
                    xCloseable->close(true);
            }
            catch(const Exception&)
            {
            }
            m_xEmbeddedObject = nullptr;
        }
    }

    aGuard.clear();
}

bool ODocumentDefinition::save(bool _bApprove, const css::uno::Reference<css::awt::XTopWindow>& rDialogParent)
{
    // default handling: instantiate an interaction handler and let it handle the parameter request
    if ( !m_bOpenInDesign )
        return false;
    try
    {

        {
            ::SolarMutexGuard aSolarGuard;

            // the request
            Reference<XNameAccess> xName(m_xParentContainer,UNO_QUERY);
            DocumentSaveRequest aRequest;
            aRequest.Name = m_pImpl->m_aProps.aTitle;
            if ( aRequest.Name.isEmpty() )
            {
                if ( m_bForm )
                    aRequest.Name = DBA_RES( RID_STR_FORM );
                else
                    aRequest.Name = DBA_RES( RID_STR_REPORT );
                aRequest.Name = ::dbtools::createUniqueName(xName,aRequest.Name);
            }

            aRequest.Content.set(m_xParentContainer,UNO_QUERY);
            OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
            Reference< XInteractionRequest > xRequest(pRequest);
            // some knittings
            // two continuations allowed: OK and Cancel
            ODocumentSaveContinuation* pDocuSave = nullptr;

            if ( m_pImpl->m_aProps.aTitle.isEmpty() )
            {
                pDocuSave = new ODocumentSaveContinuation;
                pRequest->addContinuation(pDocuSave);
            }
            if ( _bApprove )
            {
                OInteraction< XInteractionApprove >* pApprove = new OInteraction< XInteractionApprove >;
                pRequest->addContinuation(pApprove);
            }

            OInteraction< XInteractionDisapprove >* pDisApprove = new OInteraction< XInteractionDisapprove >;
            pRequest->addContinuation(pDisApprove);

            OInteractionAbort* pAbort = new OInteractionAbort;
            pRequest->addContinuation(pAbort);

            Reference<XWindow> xDialogParent(rDialogParent, UNO_QUERY);

            // create the handler, let it handle the request
            Reference<XInteractionHandler2> xHandler(InteractionHandler::createWithParent(m_aContext, xDialogParent));
            xHandler->handle(xRequest);

            if ( pAbort->wasSelected() )
                return false;
            if  ( pDisApprove->wasSelected() )
                return true;
            if ( pDocuSave && pDocuSave->wasSelected() )
            {
                Reference<XNameContainer> xNC( pDocuSave->getContent(), UNO_QUERY_THROW );

                ::osl::ResettableMutexGuard aGuard( m_aMutex );
                NameChangeNotifier aNameChangeAndNotify( *this, pDocuSave->getName(), aGuard );
                m_pImpl->m_aProps.aTitle = pDocuSave->getName();

                Reference< XContent> xContent = this;
                xNC->insertByName(pDocuSave->getName(),makeAny(xContent));

                updateDocumentTitle();
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
    catch(const Exception&)
    {
        OSL_FAIL("ODocumentDefinition::save: caught an Exception (tried to let the InteractionHandler handle it)!");
    }
    return true;
}

void ODocumentDefinition::saveAs()
{
    // default handling: instantiate an interaction handler and let it handle the parameter request
    if ( !m_bOpenInDesign )
        return;

    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
        if ( m_pImpl->m_aProps.aTitle.isEmpty() )
        {
            aGuard.clear();
            save(false, css::uno::Reference<css::awt::XTopWindow>()); // (sal_False) : we don't want an approve dialog
            return;
        }
    }
    try
    {
        ::SolarMutexGuard aSolarGuard;

        // the request
        DocumentSaveRequest aRequest;
        aRequest.Name = m_pImpl->m_aProps.aTitle;

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
        Reference< XInteractionHandler2 > xHandler( InteractionHandler::createWithParent(m_aContext, nullptr) );
        xHandler->handle(xRequest);

        if ( pAbort->wasSelected() )
            return;
        if  ( pDisApprove->wasSelected() )
            return;
        if ( pDocuSave->wasSelected() )
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            Reference<XNameContainer> xNC(pDocuSave->getContent(),UNO_QUERY);
            if ( xNC.is() )
            {
                if ( m_pImpl->m_aProps.aTitle != pDocuSave->getName() )
                {
                    try
                    {
                        Reference< XStorage> xStorage = getContainerStorage();

                        OUString sPersistentName = ::dbtools::createUniqueName(xStorage,"Obj");
                        xStorage->copyElementTo(m_pImpl->m_aProps.sPersistentName,xStorage,sPersistentName);

                        OUString sOldName = m_pImpl->m_aProps.aTitle;
                        rename(pDocuSave->getName());
                        updateDocumentTitle();

                        uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
                        {
                            {PROPERTY_NAME, uno::Any(sOldName)}, // set as folder
                            {PROPERTY_PERSISTENT_NAME, uno::Any(sPersistentName)},
                            {PROPERTY_AS_TEMPLATE, uno::Any(m_pImpl->m_aProps.bAsTemplate)},
                        }));
                        Reference< XMultiServiceFactory > xORB( m_xParentContainer, UNO_QUERY_THROW );
                        Reference< XInterface > xComponent( xORB->createInstanceWithArguments( SERVICE_SDB_DOCUMENTDEFINITION, aArguments ) );
                        Reference< XNameContainer > xNameContainer( m_xParentContainer, UNO_QUERY_THROW );
                        xNameContainer->insertByName( sOldName, makeAny( xComponent ) );
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                }
                Reference<XEmbedPersist> xPersist(m_xEmbeddedObject,UNO_QUERY);
                if ( xPersist.is() )
                {
                    xPersist->storeOwn();
                    notifyDataSourceModified();
                }
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("ODocumentDefinition::save: caught an Exception (tried to let the InteractionHandler handle it)!");
    }
}

namespace
{
    void    lcl_putLoadArgs( ::comphelper::NamedValueCollection& _io_rArgs, const optional_bool& _bSuppressMacros, const optional_bool& _bReadOnly )
    {
        if ( !!_bSuppressMacros )
        {
            if ( *_bSuppressMacros )
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
        }

        if ( !!_bReadOnly )
            _io_rArgs.put( "ReadOnly", *_bReadOnly );
    }
}

namespace
{
    Reference< XFrame > lcl_getDatabaseDocumentFrame( ODatabaseModelImpl const & _rImpl )
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

bool ODocumentDefinition::objectSupportsEmbeddedScripts() const
{
    bool bAllowDocumentMacros = !m_pImpl->m_pDataSource
                            ||  ( m_pImpl->m_pDataSource->determineEmbeddedMacros() == ODatabaseModelImpl::eSubDocumentMacros );

    // if *any* of the objects of the database document already has macros, we
    // continue to allow it to have them, until the user does a migration.
    // If there are no macros, we don't allow them to be created.

    return bAllowDocumentMacros;
}

OUString ODocumentDefinition::determineContentType() const
{
    return lcl_determineContentType_nothrow( getContainerStorage(), m_pImpl->m_aProps.sPersistentName );
}

void ODocumentDefinition::separateOpenCommandArguments( const Sequence< PropertyValue >& i_rOpenCommandArguments,
        ::comphelper::NamedValueCollection& o_rDocumentLoadArgs, ::comphelper::NamedValueCollection& o_rEmbeddedObjectDescriptor )
{
    ::comphelper::NamedValueCollection aOpenCommandArguments( i_rOpenCommandArguments );

    const char* const pObjectDescriptorArgs[] =
    {
        "RecoveryStorage"
    };
    for (const char* pObjectDescriptorArg : pObjectDescriptorArgs)
    {
        if ( aOpenCommandArguments.has( pObjectDescriptorArg ) )
        {
            o_rEmbeddedObjectDescriptor.put( pObjectDescriptorArg, aOpenCommandArguments.get( pObjectDescriptorArg ) );
            aOpenCommandArguments.remove( pObjectDescriptorArg );
        }
    }

    o_rDocumentLoadArgs.merge( aOpenCommandArguments, false );
}

Sequence< PropertyValue > ODocumentDefinition::fillLoadArgs( const Reference< XConnection>& _xConnection, const bool _bSuppressMacros, const bool _bReadOnly,
        const Sequence< PropertyValue >& i_rOpenCommandArguments, Sequence< PropertyValue >& _out_rEmbeddedObjectDescriptor )
{
    // (re-)create interceptor, and put it into the descriptor of the embedded object
    if ( m_pInterceptor.is() )
    {
        m_pInterceptor->dispose();
        m_pInterceptor.clear();
    }

    m_pInterceptor = new OInterceptor( this );
    Reference<XDispatchProviderInterceptor> xInterceptor = m_pInterceptor.get();

    ::comphelper::NamedValueCollection aEmbeddedDescriptor;
    aEmbeddedDescriptor.put( "OutplaceDispatchInterceptor", xInterceptor );

    ::comphelper::NamedValueCollection aMediaDesc;
    separateOpenCommandArguments( i_rOpenCommandArguments, aMediaDesc, aEmbeddedDescriptor );

    // create the OutplaceFrameProperties, and put them into the descriptor of the embedded object
    ::comphelper::NamedValueCollection OutplaceFrameProperties;
    OutplaceFrameProperties.put( "TopWindow", true );
    OutplaceFrameProperties.put( "SupportPersistentWindowState", true );

    Reference< XFrame > xParentFrame;
    if ( m_pImpl->m_pDataSource )
        xParentFrame = lcl_getDatabaseDocumentFrame( *m_pImpl->m_pDataSource );
    if ( !xParentFrame.is() )
    { // i87957 we need a parent frame
        Reference< XDesktop2 > xDesktop = Desktop::create( m_aContext );
        xParentFrame.set( xDesktop, UNO_QUERY_THROW );
        Reference<util::XCloseable> xCloseable(m_pImpl->m_pDataSource->getModel_noCreate(),UNO_QUERY);
        if ( xCloseable.is() )
        {
            xCloseable->addCloseListener(this);
            m_bRemoveListener = true;
        }
    }
    OSL_ENSURE( xParentFrame.is(), "ODocumentDefinition::fillLoadArgs: no parent frame!" );
    if  ( xParentFrame.is() )
        OutplaceFrameProperties.put( "ParentFrame", xParentFrame );

    aEmbeddedDescriptor.put( "OutplaceFrameProperties", OutplaceFrameProperties.getNamedValues() );

    // tell the embedded object to have (or not have) script support
    aEmbeddedDescriptor.put( "EmbeddedScriptSupport", objectSupportsEmbeddedScripts() );

    // tell the embedded object to not participate in the document recovery game - the DB doc will handle it
    aEmbeddedDescriptor.put( "DocumentRecoverySupport", false );

    // pass the descriptor of the embedded object to the caller
    aEmbeddedDescriptor >>= _out_rEmbeddedObjectDescriptor;

    // create the ComponentData, and put it into the document's media descriptor
    {
        ::comphelper::NamedValueCollection aComponentData;
        aComponentData.put( "ActiveConnection", _xConnection );
        aComponentData.put( "ApplyFormDesignMode", !_bReadOnly );
        aMediaDesc.put( "ComponentData", aComponentData.getPropertyValues() );
    }

    if ( !m_pImpl->m_aProps.aTitle.isEmpty() )
        aMediaDesc.put( "DocumentTitle", m_pImpl->m_aProps.aTitle );

    aMediaDesc.put( "DocumentBaseURL", m_pImpl->m_pDataSource->getURL() );

    // put the common load arguments into the document's media descriptor
    lcl_putLoadArgs( aMediaDesc, optional_bool( _bSuppressMacros ), optional_bool( _bReadOnly ) );

    return aMediaDesc.getPropertyValues();
}

void ODocumentDefinition::loadEmbeddedObject( const Reference< XConnection >& i_rConnection, const Sequence< sal_Int8 >& _aClassID,
        const Sequence< PropertyValue >& i_rOpenCommandArguments, const bool _bSuppressMacros, const bool _bReadOnly )
{
    if ( !m_xEmbeddedObject.is() )
    {
        Reference< XStorage> xStorage = getContainerStorage();
        if ( xStorage.is() )
        {
            Reference< XEmbeddedObjectCreator> xEmbedFactory = OOoEmbeddedObjectFactory::create(m_aContext);
            OUString sDocumentService;
            bool bSetSize = false;
            sal_Int32 nEntryConnectionMode = EntryInitModes::DEFAULT_INIT;
            Sequence< sal_Int8 > aClassID = _aClassID;
            if ( aClassID.getLength() )
            {
                nEntryConnectionMode = EntryInitModes::TRUNCATE_INIT;
                bSetSize = true;
            }
            else
            {
                sDocumentService = GetDocumentServiceFromMediaType( getContentType(), m_aContext, aClassID );
                // check if we are not a form and
                // the org.libreoffice.report.pentaho.SOReportJobFactory is not present.
                if ( !m_bForm && sDocumentService != "com.sun.star.text.TextDocument")
                {
                    // we seem to be a "new style" report, check if report extension is present.
                    Reference< XContentEnumerationAccess > xEnumAccess( m_aContext->getServiceManager(), UNO_QUERY );
                    const OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(m_aContext);
                    Reference< XEnumeration > xEnumDrivers = xEnumAccess->createContentEnumeration(sReportEngineServiceName);
                    if ( !xEnumDrivers.is() || !xEnumDrivers->hasMoreElements() )
                    {
                        css::io::WrongFormatException aWFE;
                        aWFE.Message = DBA_RES( RID_STR_MISSING_EXTENSION );
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
                i_rConnection, _bSuppressMacros, _bReadOnly, i_rOpenCommandArguments, aEmbeddedObjectDescriptor ) );

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
                if ( !m_pClientHelper.is() )
                {
                    m_pClientHelper = new OEmbeddedClientHelper;
                }
                Reference<XEmbeddedClient> xClient = m_pClientHelper.get();
                m_xEmbeddedObject->setClientSite(xClient);
                m_xEmbeddedObject->changeState(EmbedStates::RUNNING);
                if ( bSetSize )
                {
                    LockModifiable aLockModify( impl_getComponent_throw( false ) );

                    awt::Size aSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
                    m_xEmbeddedObject->setVisualAreaSize(Aspects::MSOLE_CONTENT,aSize);
                }
            }
        }
    }
    else
    {
        sal_Int32 nCurrentState = m_xEmbeddedObject->getCurrentState();
        if ( nCurrentState == EmbedStates::LOADED )
        {
            if ( !m_pClientHelper.is() )
            {
                m_pClientHelper = new OEmbeddedClientHelper;
            }
            Reference<XEmbeddedClient> xClient = m_pClientHelper.get();
            m_xEmbeddedObject->setClientSite(xClient);

            Sequence< PropertyValue > aEmbeddedObjectDescriptor;
            Sequence< PropertyValue > aLoadArgs( fillLoadArgs(
                i_rConnection, _bSuppressMacros, _bReadOnly, i_rOpenCommandArguments, aEmbeddedObjectDescriptor ) );

            Reference<XCommonEmbedPersist> xCommon(m_xEmbeddedObject,UNO_QUERY);
            OSL_ENSURE(xCommon.is(),"unsupported interface!");
            if ( xCommon.is() )
                xCommon->reload( aLoadArgs, aEmbeddedObjectDescriptor );
            m_xEmbeddedObject->changeState(EmbedStates::RUNNING);
        }
        else
        {
            OSL_ENSURE( ( nCurrentState == EmbedStates::RUNNING ) || ( nCurrentState == EmbedStates::ACTIVE ),
                "ODocumentDefinition::loadEmbeddedObject: unexpected state!" );

            // if the document was already loaded (which means the embedded object is in state RUNNING or ACTIVE),
            // then just re-set some model parameters
            try
            {
                // ensure the media descriptor doesn't contain any values which are intended for the
                // EmbeddedObjectDescriptor only
                ::comphelper::NamedValueCollection aEmbeddedObjectDescriptor;
                ::comphelper::NamedValueCollection aNewMediaDesc;
                separateOpenCommandArguments( i_rOpenCommandArguments, aNewMediaDesc, aEmbeddedObjectDescriptor );

                // merge the new media descriptor into the existing media descriptor
                const Reference< XModel > xModel( getComponent(), UNO_QUERY_THROW );
                const Sequence< PropertyValue > aArgs = xModel->getArgs();
                ::comphelper::NamedValueCollection aExistentMediaDesc( aArgs );
                aExistentMediaDesc.merge( aNewMediaDesc, false );

                lcl_putLoadArgs( aExistentMediaDesc, optional_bool(), optional_bool() );
                    // don't put _bSuppressMacros and _bReadOnly here - if the document was already
                    // loaded, we should not tamper with its settings.
                    // #i88977# #i86872#

                xModel->attachResource( xModel->getURL(), aExistentMediaDesc.getPropertyValues() );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }

    // set the OfficeDatabaseDocument instance as parent of the embedded document
    // #i40358#
    Reference< XChild > xDepdendDocAsChild( getComponent(), UNO_QUERY );
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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    if ( i_rConnection.is() )
        m_xLastKnownConnection = i_rConnection;
}

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
                aFlavor.MimeType = "image/png";
                aFlavor.HumanPresentableName = "Portable Network Graphics";
                aFlavor.DataType = cppu::UnoType<Sequence < sal_Int8 >>::get();

                _rImage = xTransfer->getTransferData( aFlavor );
            }
        }
        catch( const Exception& )
        {
        }
    }
}

void ODocumentDefinition::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}

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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

Reference< util::XCloseable > ODocumentDefinition::impl_getComponent_throw( const bool i_ForceCreate )
{
    OSL_ENSURE(m_xEmbeddedObject.is(),"Illegal call for embeddedObject");
    Reference< util::XCloseable > xComp;
    if ( m_xEmbeddedObject.is() )
    {
        int nState = m_xEmbeddedObject->getCurrentState();
        if ( ( nState == EmbedStates::LOADED ) && i_ForceCreate )
        {
            m_xEmbeddedObject->changeState( EmbedStates::RUNNING );
            nState = m_xEmbeddedObject->getCurrentState();
            OSL_ENSURE( nState == EmbedStates::RUNNING, "ODocumentDefinition::impl_getComponent_throw: could not switch to RUNNING!" );
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

Reference< util::XCloseable > ODocumentDefinition::getComponent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return impl_getComponent_throw();
}

namespace
{
    Reference< XDatabaseDocumentUI > lcl_getDatabaseDocumentUI( ODatabaseModelImpl const & _rModelImpl )
    {
        Reference< XDatabaseDocumentUI > xUI;

        Reference< XModel > xModel( _rModelImpl.getModel_noCreate() );
        if ( xModel.is() )
            xUI.set( xModel->getCurrentController(), UNO_QUERY );
        return xUI;
    }
}

Reference< XComponent > ODocumentDefinition::impl_openUI_nolck_throw( bool _bForEditing )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( !m_pImpl || !m_pImpl->m_pDataSource )
        throw DisposedException();

    Reference< XComponent > xComponent;
    try
    {
        Reference< XDatabaseDocumentUI > xUI( lcl_getDatabaseDocumentUI( *m_pImpl->m_pDataSource ) );
        if ( !xUI.is() )
        {
            // no XDatabaseDocumentUI -> just execute the respective command
            m_bOpenInDesign = _bForEditing;
            xComponent = Reference<XComponent>(onCommandOpenSomething(Any(), true, nullptr), UNO_QUERY);
            OSL_ENSURE( xComponent.is(), "ODocumentDefinition::impl_openUI_nolck_throw: opening the thingie failed." );
            return xComponent;
        }


        OUString sName( impl_getHierarchicalName( false ) );
        sal_Int32 nObjectType = m_bForm ? DatabaseObject::FORM : DatabaseObject::REPORT;
        aGuard.clear();

        xComponent = xUI->loadComponent(
            nObjectType, sName, _bForEditing
        );
    }
    catch( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        throw WrappedTargetException(
            OUString(), *this, ::cppu::getCaughtException() );
    }

    return xComponent;
}

void ODocumentDefinition::impl_store_throw()
{
    Reference<XEmbedPersist> xPersist( m_xEmbeddedObject, UNO_QUERY );
    if ( xPersist.is() )
    {
        xPersist->storeOwn();
        notifyDataSourceModified();
    }
}

bool ODocumentDefinition::impl_close_throw()
{
    bool bSuccess = prepareClose();
    if ( bSuccess && m_xEmbeddedObject.is() )
    {
        m_xEmbeddedObject->changeState( EmbedStates::LOADED );
        bSuccess = m_xEmbeddedObject->getCurrentState() == EmbedStates::LOADED;
    }
    return bSuccess;
}

Reference< XComponent > SAL_CALL ODocumentDefinition::open(  )
{
    return impl_openUI_nolck_throw( false );
}

Reference< XComponent > SAL_CALL ODocumentDefinition::openDesign(  )
{
    return impl_openUI_nolck_throw( true );
}

void SAL_CALL ODocumentDefinition::store(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    try
    {
        impl_store_throw();
    }
    catch( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        throw WrappedTargetException(
            OUString(), *this, ::cppu::getCaughtException() );
    }
}

sal_Bool SAL_CALL ODocumentDefinition::close(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bSuccess = false;
    try
    {
        bSuccess = impl_close_throw();
    }
    catch( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        throw WrappedTargetException(
            OUString(), *this, ::cppu::getCaughtException() );
    }
    return bSuccess;
}

OUString SAL_CALL ODocumentDefinition::getHierarchicalName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return impl_getHierarchicalName( false );
}

OUString SAL_CALL ODocumentDefinition::composeHierarchicalName( const OUString& i_rRelativeName )
{
    OUStringBuffer aBuffer;
    aBuffer.append( getHierarchicalName() );
    aBuffer.append( '/' );
    aBuffer.append( i_rRelativeName );
    return aBuffer.makeStringAndClear();
}

void SAL_CALL ODocumentDefinition::rename( const OUString& _rNewName )
{
    try
    {
        ::osl::ResettableMutexGuard aGuard(m_aMutex);
        if ( _rNewName == m_pImpl->m_aProps.aTitle )
            return;

        // document definitions are organized in a hierarchical way, so reject names
        // which contain a /, as this is reserved for hierarchy level separation
        if ( _rNewName.indexOf( '/' ) != -1 )
            m_aErrorHelper.raiseException( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES, *this );

        NameChangeNotifier aNameChangeAndNotify( *this, _rNewName, aGuard );
        m_pImpl->m_aProps.aTitle = _rNewName;

        if ( m_xEmbeddedObject.is() && m_xEmbeddedObject->getCurrentState() == EmbedStates::ACTIVE )
            updateDocumentTitle();
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(_rNewName,*this);
    }
}

Reference< XStorage> ODocumentDefinition::getContainerStorage() const
{
    return  m_pImpl->m_pDataSource
        ?   m_pImpl->m_pDataSource->getStorage( m_bForm ? ODatabaseModelImpl::E_FORM : ODatabaseModelImpl::E_REPORT )
        :   Reference< XStorage>();
}

bool ODocumentDefinition::isModified()
{
    osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
    bool bRet = false;
    if ( m_xEmbeddedObject.is() )
    {
        Reference<XModifiable> xModel(getComponent(),UNO_QUERY);
        bRet = xModel.is() && xModel->isModified();
    }
    return bRet;
}

bool ODocumentDefinition::prepareClose()
{
    if ( !m_xEmbeddedObject.is() )
        return true;

    try
    {
        // suspend the controller. Embedded objects are not allowed to raise
        // own UI at their own discretion, instead, this has always to be triggered
        // by the embedding component. Thus, we do the suspend call here.
        // #i49370#

        Reference< util::XCloseable > xComponent( impl_getComponent_throw( false ) );
        if ( !xComponent.is() )
            return true;

        Reference< XModel > xModel( xComponent, UNO_QUERY );
        Reference< XController > xController;
        if ( xModel.is() )
            xController = xModel->getCurrentController();

        OSL_ENSURE( xController.is() || ( m_xEmbeddedObject->getCurrentState() < EmbedStates::ACTIVE ),
            "ODocumentDefinition::prepareClose: no controller!" );
        if ( !xController.is() )
            // document has not yet been activated, i.e. has no UI, yet
            return true;

        if (!xController->suspend(true))
            // controller vetoed the closing
            return false;

        if ( isModified() )
        {
            Reference< XFrame > xFrame( xController->getFrame() );
            Reference<XTopWindow> xTopWindow;
            if ( xFrame.is() )
            {
                xTopWindow = Reference<XTopWindow>(xFrame->getContainerWindow(), UNO_QUERY_THROW);
                xTopWindow->toFront();
            }
            if (!save(true, xTopWindow))
            {
                // revert suspension
                xController->suspend(false);
                // saving failed or was cancelled
                return false;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return true;
}

void ODocumentDefinition::fillReportData( const Reference< XComponentContext >& _rContext,
                                               const Reference< util::XCloseable >& _rxComponent,
                                               const Reference< XConnection >& _rxActiveConnection )
{
    uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
    {
        {"TextDocument", uno::Any(_rxComponent)},
        {"ActiveConnection", uno::Any(_rxActiveConnection)}
    }));
    try
    {
        Reference< XJobExecutor > xExecuteable(
            _rContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.wizards.report.CallReportWizard", aArgs, _rContext), UNO_QUERY_THROW );
        xExecuteable->trigger( "fill" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void ODocumentDefinition::updateDocumentTitle()
{
    OUString sName = m_pImpl->m_aProps.aTitle;
    if ( m_pImpl->m_pDataSource )
    {
        if ( sName.isEmpty() )
        {
            if ( m_bForm )
                sName = DBA_RES( RID_STR_FORM );
            else
                sName = DBA_RES( RID_STR_REPORT );
            Reference< XUntitledNumbers > xUntitledProvider(m_pImpl->m_pDataSource->getModel_noCreate(), UNO_QUERY      );
            if ( xUntitledProvider.is() )
                sName += OUString::number( xUntitledProvider->leaseNumber(getComponent()) );
        }

        Reference< XTitle > xDatabaseDocumentModel(m_pImpl->m_pDataSource->getModel_noCreate(),uno::UNO_QUERY);
        if ( xDatabaseDocumentModel.is() )
            sName = xDatabaseDocumentModel->getTitle() + " : " + sName;
    }
    Reference< XTitle> xTitle(getComponent(),UNO_QUERY);
    if ( xTitle.is() )
        xTitle->setTitle(sName);
}

void SAL_CALL ODocumentDefinition::queryClosing( const lang::EventObject&, sal_Bool )
{
    try
    {
        if ( !close() )
            throw util::CloseVetoException();
    }
    catch(const lang::WrappedTargetException&)
    {
        throw util::CloseVetoException();
    }
}

void SAL_CALL ODocumentDefinition::notifyClosing( const lang::EventObject& /*Source*/ )
{
}

void SAL_CALL ODocumentDefinition::disposing( const lang::EventObject& /*Source*/ )
{
}

void ODocumentDefinition::firePropertyChange( sal_Int32 i_nHandle, const Any& i_rNewValue, const Any& i_rOldValue,
        bool i_bVetoable, const NotifierAccess& )
{
    fire( &i_nHandle, &i_rNewValue, &i_rOldValue, 1, i_bVetoable );
}

// NameChangeNotifier
NameChangeNotifier::NameChangeNotifier( ODocumentDefinition& i_rDocumentDefinition, const OUString& i_rNewName,
                                  ::osl::ResettableMutexGuard& i_rClearForNotify )
    :m_rDocumentDefinition( i_rDocumentDefinition )
    ,m_aOldValue( makeAny( i_rDocumentDefinition.getCurrentName() ) )
    ,m_aNewValue( makeAny( i_rNewName ) )
    ,m_rClearForNotify( i_rClearForNotify )
{
    impl_fireEvent_throw( true );
}

NameChangeNotifier::~NameChangeNotifier()
{
    impl_fireEvent_throw( false );
}

void NameChangeNotifier::impl_fireEvent_throw( const bool i_bVetoable )
{
    m_rClearForNotify.clear();
    m_rDocumentDefinition.firePropertyChange(
        PROPERTY_ID_NAME, m_aNewValue, m_aOldValue, i_bVetoable, ODocumentDefinition::NotifierAccess() );
    m_rClearForNotify.reset();
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
