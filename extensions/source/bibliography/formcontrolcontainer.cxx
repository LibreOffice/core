/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formcontrolcontainer.cxx,v $
 * $Revision: 1.8 $
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

    //---------------------------------------------------------------------
    //--- 18.10.01 18:54:57 -----------------------------------------------
    FormControlContainer::~FormControlContainer( )
    {
        DBG_ASSERT( !isFormConnected(), "FormControlContainer::~FormControlContainer: you should disconnect in your derived class!" );
        if ( isFormConnected() )
            disconnectForm();
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 17:03:14 -----------------------------------------------
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

    //---------------------------------------------------------------------
    //--- 18.10.01 16:56:01 -----------------------------------------------
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

    //---------------------------------------------------------------------
    //--- 18.10.01 18:50:14 -----------------------------------------------
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

    //---------------------------------------------------------------------
    //--- 18.10.01 18:49:57 -----------------------------------------------
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
            DBG_ERROR( "FormControlContainer::implSetDesignMode: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 18:16:54 -----------------------------------------------
    void FormControlContainer::ensureDesignMode()
    {
        implSetDesignMode( !m_xForm.is() || !m_xForm->isLoaded() );
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 16:45:33 -----------------------------------------------
    void FormControlContainer::_loaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_False );
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 16:45:35 -----------------------------------------------
    void FormControlContainer::_unloading( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_True );
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 16:45:36 -----------------------------------------------
    void FormControlContainer::_unloaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 16:45:36 -----------------------------------------------
    void FormControlContainer::_reloading( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_True );
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 16:45:37 -----------------------------------------------
    void FormControlContainer::_reloaded( const ::com::sun::star::lang::EventObject& /*_rEvent*/ )
    {
        implSetDesignMode( sal_False );
    }

//.........................................................................
}   // namespace bib
//.........................................................................

