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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "formcontrolcontainer.hxx"
#include <tools/debug.hxx>

#include <algorithm>
#include <functional>

//.........................................................................
namespace bib
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;

    //=====================================================================
    //= FormControlContainer
    //=====================================================================
    //---------------------------------------------------------------------
    FormControlContainer::FormControlContainer( )
        :OLoadListener( m_aMutex )
        ,m_pFormAdapter( NULL )
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
            m_pFormAdapter = NULL;
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
        sal_Bool bDesign;
        ControlModeSwitch( sal_Bool _bDesign ) : bDesign( _bDesign ) { }

        void operator() ( const Reference< XControl >& _rxControl ) const
        {
            if ( _rxControl.is() )
                _rxControl->setDesignMode( bDesign );
        }
    };

    void FormControlContainer::implSetDesignMode( sal_Bool _bDesign )
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

    void FormControlContainer::_loaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_False );
    }

    void FormControlContainer::_unloading( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_True );
    }

    void FormControlContainer::_unloaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
    }

    void FormControlContainer::_reloading( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_True );
    }

    void FormControlContainer::_reloaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_False );
    }

//.........................................................................
}   // namespace bib
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
