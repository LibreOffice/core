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

#include "formcontrolcontainer.hxx"
#include <tools/debug.hxx>

#include <algorithm>
#include <functional>


namespace bib
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;

    FormControlContainer::FormControlContainer( )
        :OLoadListener( m_aMutex )
        ,m_pFormAdapter( nullptr )
    {
    }

    FormControlContainer::~FormControlContainer( )
    {
        DBG_ASSERT( !isFormConnected(), "FormControlContainer::~FormControlContainer: you should disconnect in your derived class!" );
        if ( isFormConnected() )
            disconnectForm();
    }

    void FormControlContainer::disconnectForm()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_ASSERT( isFormConnected(), "FormControlContainer::connectForm: not connected!" );
        if ( isFormConnected() )
        {
            m_pFormAdapter->dispose();
            m_pFormAdapter->release();
            m_pFormAdapter = nullptr;
        }
    }

    void FormControlContainer::connectForm( const Reference< XLoadable >& _rxForm )
    {
        DBG_ASSERT( !isFormConnected(), "FormControlContainer::connectForm: already connected!" );

        DBG_ASSERT( _rxForm.is(), "FormControlContainer::connectForm: invalid form!" );
        if ( !isFormConnected() && _rxForm.is() )
        {
            m_pFormAdapter = new OLoadListenerAdapter( _rxForm );
            m_pFormAdapter->acquire();
            m_pFormAdapter->Init( this );

            ensureDesignMode();
        }

        m_xForm = _rxForm;
    }

    struct ControlModeSwitch : public ::std::unary_function< Reference< XControl >, void >
    {
        bool bDesign;
        explicit ControlModeSwitch( bool _bDesign ) : bDesign( _bDesign ) { }

        void operator() ( const Reference< XControl >& _rxControl ) const
        {
            if ( _rxControl.is() )
                _rxControl->setDesignMode( bDesign );
        }
    };

    void FormControlContainer::implSetDesignMode( bool _bDesign )
    {
        try
        {
            Reference< XControlContainer > xControlCont = getControlContainer();
            Sequence< Reference< XControl > > aControls;
            if ( xControlCont.is() )
                aControls = xControlCont->getControls();

            ::std::for_each(
                aControls.getConstArray(),
                aControls.getConstArray() + aControls.getLength(),
                ControlModeSwitch( _bDesign )
            );
        }
        catch( const Exception& e)
        {
            (void) e;   // make compiler happy
            OSL_FAIL( "FormControlContainer::implSetDesignMode: caught an exception!" );
        }
    }

    void FormControlContainer::ensureDesignMode()
    {
        implSetDesignMode( !m_xForm.is() || !m_xForm->isLoaded() );
    }

    void FormControlContainer::_loaded( const css::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( false );
    }

    void FormControlContainer::_unloading( const css::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( true );
    }

    void FormControlContainer::_unloaded( const css::lang::EventObject& /*_rEvent*/ )
    {
    }

    void FormControlContainer::_reloading( const css::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( true );
    }

    void FormControlContainer::_reloaded( const css::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( false );
    }


}   // namespace bib


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
