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

#include "subcomponentmanager.hxx"
#include "AppController.hxx"
#include "dbustrings.hrc"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <algorithm>
#include <functional>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::awt::XTopWindow;
    using ::com::sun::star::embed::XComponentSupplier;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::PropertyChangeEvent;

    // helper structs
    namespace
    {
        struct SubComponentDescriptor
        {
            /// the name of the sub component, empty if it is yet unsaved
            OUString sName;
            /// type of the component - an ElementType value, except for relation design
            sal_Int32       nComponentType;
            /// the mode in which the sub component has been opened
            ElementOpenMode eOpenMode;
            /// the frame which the component resides in. Must not be <NULL/>
            Reference< XFrame >             xFrame;
            /// the controller of the sub component. Must not be <NULL/>
            Reference< XController >        xController;
            /// the model of the sub component. Might be <NULL/>
            Reference< XModel >             xModel;
            /// the document definition which holds the component, if any; as CommandProcessor
            Reference< XCommandProcessor >  xComponentCommandProcessor;
            /// the document definition which holds the component, if any; as PropertySet
            Reference< XPropertySet >       xDocumentDefinitionProperties;

            SubComponentDescriptor()
                :sName()
                ,nComponentType( -1 )
                ,eOpenMode( E_OPEN_NORMAL )
                ,xFrame()
                ,xController()
                ,xModel()
            {
            }

            SubComponentDescriptor( const OUString& i_rName, const sal_Int32 i_nComponentType,
                    const ElementOpenMode i_eOpenMode, const Reference< XComponent >& i_rComponent )
                :sName( i_rName )
                ,nComponentType( i_nComponentType )
                ,eOpenMode( i_eOpenMode )
            {
                if ( !impl_constructFrom( i_rComponent ) )
                {
                    // i_rComponent is neither a model, nor a controller, nor a frame
                    // => it must be a css.sdb.DocumentDefinition
                    Reference< XComponentSupplier > xCompSupp( i_rComponent, UNO_QUERY_THROW );
                    Reference< XComponent > xComponent( xCompSupp->getComponent(), UNO_QUERY_THROW );
                    if ( !impl_constructFrom( xComponent ) )
                        throw RuntimeException("Illegal component type.", NULL );
                    xComponentCommandProcessor.set( i_rComponent, UNO_QUERY_THROW );
                    xDocumentDefinitionProperties.set( i_rComponent, UNO_QUERY_THROW );
                }
            }

            inline bool is() const { return xFrame.is(); }

        private:
            bool impl_constructFrom( const Reference< XComponent >& _rxComponent )
            {
                // is it a model?
                xModel.set( _rxComponent, UNO_QUERY );
                if ( xModel.is() )
                {
                    xController.set( xModel->getCurrentController() );
                    if ( xController.is() )
                        xFrame.set( xController->getFrame(), UNO_SET_THROW );
                }
                else
                {
                    // is it a controller?
                    xController.set( _rxComponent, UNO_QUERY );
                    if ( xController.is() )
                    {
                        xFrame.set( xController->getFrame(), UNO_SET_THROW );
                    }
                    else
                    {
                        // is it a frame?
                        xFrame.set( _rxComponent, UNO_QUERY );
                        if ( !xFrame.is() )
                            return false;

                        // ensure we have a controller
                        xController.set( xFrame->getController(), UNO_SET_THROW );
                    }

                    // check whether there is a model (not required)
                    xModel.set( xController->getModel() );
                }

                return true;
            }
        };

        struct SelectSubComponent : public ::std::unary_function< SubComponentDescriptor, Reference< XComponent > >
        {
            Reference< XComponent > operator()( const SubComponentDescriptor &_desc ) const
            {
                if ( _desc.xModel.is() )
                    return _desc.xModel.get();
                OSL_ENSURE( _desc.xController.is(), "SelectSubComponent::operator(): illegal component!" );
                return _desc.xController.get();
            }
        };

        typedef ::std::vector< SubComponentDescriptor > SubComponents;

        struct SubComponentMatch : public ::std::unary_function< SubComponentDescriptor, bool >
        {
        public:
            SubComponentMatch( const OUString& i_rName, const sal_Int32 i_nComponentType,
                    const ElementOpenMode i_eOpenMode )
                :m_sName( i_rName )
                ,m_nComponentType( i_nComponentType )
                ,m_eOpenMode( i_eOpenMode )
            {
            }

            bool operator()( const SubComponentDescriptor& i_rCompareWith ) const
            {
                return  (   m_sName             ==  i_rCompareWith.sName          )
                    &&  (   m_nComponentType    ==  i_rCompareWith.nComponentType )
                    &&  (   m_eOpenMode         ==  i_rCompareWith.eOpenMode      );
            }
        private:
            const OUString   m_sName;
            const sal_Int32         m_nComponentType;
            const ElementOpenMode   m_eOpenMode;
        };
    }

    // SubComponentManager_Data
    struct SubComponentManager_Data
    {
        SubComponentManager_Data( OApplicationController& _rController, const ::comphelper::SharedMutex& _rMutex )
            :m_rController( _rController )
            ,m_aMutex( _rMutex )
        {
        }

        OApplicationController&             m_rController;
        mutable ::comphelper::SharedMutex   m_aMutex;
        SubComponents                       m_aComponents;

        ::osl::Mutex&   getMutex() const { return m_aMutex; }
    };

    // SubComponentManager
    SubComponentManager::SubComponentManager( OApplicationController& _rController, const ::comphelper::SharedMutex& _rMutex )
        :m_pData( new SubComponentManager_Data( _rController, _rMutex ) )
    {
    }

    SubComponentManager::~SubComponentManager()
    {
    }

    void SubComponentManager::disposing()
    {
        ::osl::MutexGuard aGuard( m_pData->getMutex() );
        m_pData->m_aComponents.clear();
    }

    namespace
    {
        bool lcl_fallbackToAnotherController( SubComponentDescriptor& _rCompDesc )
        {
            Reference< XController > xFallback;
            OSL_PRECOND( _rCompDesc.xModel.is(), "lcl_fallbackToAnotherController: illegal call!" );
            if ( !_rCompDesc.xModel.is() )
                return false;

            xFallback.set( _rCompDesc.xModel->getCurrentController() );
            if ( xFallback == _rCompDesc.xController )
                // don't accept the very same controller as fallback
                xFallback.clear();

            if ( !xFallback.is() )
            {
                // perhaps XModel2 can be of help here
                Reference< XModel2 > xModel2( _rCompDesc.xModel, UNO_QUERY );
                Reference< XEnumeration > xControllerEnum;
                if ( xModel2.is() )
                    xControllerEnum = xModel2->getControllers();
                while ( xControllerEnum.is() && xControllerEnum->hasMoreElements() )
                {
                    xFallback.set( xControllerEnum->nextElement(), UNO_QUERY );
                    if ( xFallback == _rCompDesc.xController )
                        xFallback.clear();
                }
            }

            if ( xFallback.is() )
            {
                _rCompDesc.xController = xFallback;
                _rCompDesc.xFrame.set( xFallback->getFrame(), UNO_SET_THROW );
                return true;
            }

            return false;
        }

        bool lcl_closeComponent( const Reference< XCommandProcessor >& _rxCommandProcessor )
        {
            bool bSuccess = false;
            try
            {
                sal_Int32 nCommandIdentifier = _rxCommandProcessor->createCommandIdentifier();

                Command aCommand;
                aCommand.Name = OUString( "close" );
                _rxCommandProcessor->execute( aCommand, nCommandIdentifier, NULL );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return bSuccess;
        }

        bool lcl_closeComponent( const SubComponentDescriptor& _rComponent )
        {
            if ( _rComponent.xComponentCommandProcessor.is() )
                return lcl_closeComponent( _rComponent.xComponentCommandProcessor );

            Reference< XController > xController( _rComponent.xController );
            OSL_ENSURE( xController.is(), "lcl_closeComponent: invalid controller!" );

            // suspend the controller in the document
            if ( xController.is() )
                if ( !xController->suspend( sal_True ) )
                    return false;

            bool bSuccess = false;
            try
            {
                Reference< XCloseable > xCloseable( _rComponent.xFrame, UNO_QUERY_THROW );
                xCloseable->close( sal_True );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return bSuccess;
        }

        void lcl_notifySubComponentEvent( const SubComponentManager_Data& _rData, const sal_Char* _pAsciiEventName,
                const SubComponentDescriptor& _rComponent )
        {
            try
            {
                Reference< XDocumentEventBroadcaster > xBroadcaster( _rData.m_rController.getModel(), UNO_QUERY_THROW );
                xBroadcaster->notifyDocumentEvent(
                    OUString::createFromAscii( _pAsciiEventName ),
                    &_rData.m_rController,
                    makeAny( _rComponent.xFrame )
                );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    void SAL_CALL SubComponentManager::propertyChange( const PropertyChangeEvent& i_rEvent ) throw (RuntimeException)
    {
        if ( i_rEvent.PropertyName != PROPERTY_NAME )
            // by definition, it's allowed to broadcast more than what we've registered for
            return;

        // find the sub component whose name changed
        for (   SubComponents::iterator comp = m_pData->m_aComponents.begin();
                comp != m_pData->m_aComponents.end();
                ++comp
            )
        {
            if ( comp->xDocumentDefinitionProperties != i_rEvent.Source )
                continue;

            OUString sNewName;
            OSL_VERIFY( i_rEvent.NewValue >>= sNewName );

        #if OSL_DEBUG_LEVEL > 0
            OUString sOldKnownName( comp->sName );
            OUString sOldName;
            OSL_VERIFY( i_rEvent.OldValue >>= sOldName );
            OSL_ENSURE( sOldName == sOldKnownName, "SubComponentManager::propertyChange: inconsistency in the old names!" );
        #endif

            comp->sName = sNewName;
            break;
        }
    }

    void SAL_CALL SubComponentManager::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_pData->getMutex() );

        SubComponentDescriptor aClosedComponent;

        for (   SubComponents::iterator comp = m_pData->m_aComponents.begin();
                comp != m_pData->m_aComponents.end();
                ++comp
            )
        {
            bool bRemove = false;

            if ( comp->xController == _rSource.Source )
            {
                if ( !comp->xModel.is() )
                {
                    bRemove = true;
                }
                else
                {
                    // maybe this is just one view to the sub document, and only this view is closed
                    if ( !lcl_fallbackToAnotherController( *comp ) )
                    {
                        bRemove = true;
                    }
                }
            }
            else if ( comp->xModel == _rSource.Source )
            {
                bRemove = true;
            }

            if ( bRemove )
            {
                aClosedComponent = *comp;
                m_pData->m_aComponents.erase( comp );
                break;
            }
        }

        if ( aClosedComponent.is() )
        {
            aGuard.clear();
            lcl_notifySubComponentEvent( *m_pData, "OnSubComponentClosed", aClosedComponent );
        }
    }

    Sequence< Reference< XComponent> > SubComponentManager::getSubComponents() const
    {
        ::osl::MutexGuard aGuard( m_pData->getMutex() );

        Sequence< Reference< XComponent > > aComponents( m_pData->m_aComponents.size() );
        ::std::transform(
            m_pData->m_aComponents.begin(),
            m_pData->m_aComponents.end(),
            aComponents.getArray(),
            SelectSubComponent()
        );
        return aComponents;
    }

    sal_Bool SubComponentManager::closeSubComponents()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_pData->getMutex() );

        try
        {
            SubComponents aWorkingCopy( m_pData->m_aComponents );
            for (   SubComponents::const_iterator comp = aWorkingCopy.begin();
                    comp != aWorkingCopy.end();
                    ++comp
                )
            {
                lcl_closeComponent( *comp );
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return empty();
    }

    bool SubComponentManager::empty() const
    {
        ::osl::MutexGuard aGuard( m_pData->getMutex() );
        return m_pData->m_aComponents.empty();
    }

    void SubComponentManager::onSubComponentOpened( const OUString&  _rName, const sal_Int32 _nComponentType,
        const ElementOpenMode _eOpenMode, const Reference< XComponent >& _rxComponent )
    {
        ::osl::ClearableMutexGuard aGuard( m_pData->getMutex() );

#if OSL_DEBUG_LEVEL > 0
        if ( !_rName.isEmpty() )
        {
            // check there does not already exist such a component
            SubComponents::const_iterator existentPos = ::std::find_if(
                m_pData->m_aComponents.begin(),
                m_pData->m_aComponents.end(),
                SubComponentMatch( _rName, _nComponentType, _eOpenMode )
            );
            OSL_ENSURE( existentPos == m_pData->m_aComponents.end(), "already existent!" );
        }
#endif
        SubComponentDescriptor aElement( _rName, _nComponentType, _eOpenMode, _rxComponent );
        ENSURE_OR_THROW( aElement.xModel.is() || aElement.xController.is(), "illegal component" );

        m_pData->m_aComponents.push_back( aElement );

        // add as listener
        if ( aElement.xController.is() )
            aElement.xController->addEventListener( this );
        if ( aElement.xModel.is() )
            aElement.xModel->addEventListener( this );
        if ( aElement.xDocumentDefinitionProperties.is() )
            aElement.xDocumentDefinitionProperties->addPropertyChangeListener( PROPERTY_NAME, this );

        // notify this to interested parties
        aGuard.clear();
        lcl_notifySubComponentEvent( *m_pData, "OnSubComponentOpened", aElement );
    }

    bool SubComponentManager::activateSubFrame( const OUString& _rName, const sal_Int32 _nComponentType,
        const ElementOpenMode _eOpenMode, Reference< XComponent >& o_rComponent ) const
    {
        ::osl::MutexGuard aGuard( m_pData->getMutex() );

        SubComponents::const_iterator pos = ::std::find_if(
            m_pData->m_aComponents.begin(),
            m_pData->m_aComponents.end(),
            SubComponentMatch( _rName, _nComponentType, _eOpenMode )
        );
        if ( pos == m_pData->m_aComponents.end() )
            // no component with this name/type/open mode
            return false;

        const Reference< XFrame > xFrame( pos->xFrame, UNO_SET_THROW );
        const Reference< XTopWindow > xTopWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        xTopWindow->toFront();

        if ( pos->xModel.is() )
            o_rComponent = pos->xModel.get();
        else if ( pos->xController.is() )
            o_rComponent = pos->xController.get();
        else
            o_rComponent = pos->xFrame.get();

        return true;
    }

    bool SubComponentManager::closeSubFrames( const OUString& i_rName, const sal_Int32 _nComponentType )
    {
        ::osl::MutexGuard aGuard( m_pData->getMutex() );
        ENSURE_OR_RETURN_FALSE( !i_rName.isEmpty(), "SubComponentManager::closeSubFrames: illegal name!" );

        SubComponents aWorkingCopy( m_pData->m_aComponents );
        for (   SubComponents::const_iterator comp = aWorkingCopy.begin();
                comp != aWorkingCopy.end();
                ++comp
            )
        {
            if ( ( comp->sName != i_rName ) || ( comp->nComponentType != _nComponentType ) )
                continue;

            if ( !lcl_closeComponent( *comp ) )
                return false;
        }

        return true;
    }

    bool SubComponentManager::lookupSubComponent( const Reference< XComponent >& i_rComponent,
            OUString& o_rName, sal_Int32& o_rComponentType )
    {
        for (   SubComponents::const_iterator comp = m_pData->m_aComponents.begin();
                comp != m_pData->m_aComponents.end();
                ++comp
            )
        {
            if  (   (   comp->xModel.is()
                    &&  ( comp->xModel == i_rComponent )
                    )
                ||  (   comp->xController.is()
                    &&  ( comp->xController == i_rComponent )
                    )
                ||  (   comp->xFrame.is()
                    &&  ( comp->xFrame == i_rComponent )
                    )
                )
            {
                o_rName = comp->sName;
                o_rComponentType = comp->nComponentType;
                return true;
            }
        }
        return false;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
