/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentcontroller.cxx,v $
 * $Revision: 1.5.178.4 $
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
#include "precompiled_dbaccess.hxx"

#include "documentcontroller.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaui
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;

    //====================================================================
    //= ModelControllerConnector
    //====================================================================
    DBG_NAME( ModelControllerConnector )
    //--------------------------------------------------------------------
    ModelControllerConnector::ModelControllerConnector()
    {
        DBG_CTOR( ModelControllerConnector, NULL );
    }

    //--------------------------------------------------------------------
    ModelControllerConnector::ModelControllerConnector( const Reference< XModel >& _rxModel, const Reference< XController >& _rxController )
        :m_xModel( _rxModel )
        ,m_xController( _rxController )
    {
        DBG_CTOR( ModelControllerConnector, NULL );
        DBG_ASSERT( _rxModel.is() && m_xController.is(), "ModelControllerConnector::ModelControllerConnector: invalid model or controller!" );
        impl_connect();
    }

    //--------------------------------------------------------------------
    ModelControllerConnector::ModelControllerConnector( const ModelControllerConnector& _rSource )
    {
        DBG_CTOR( ModelControllerConnector, NULL );
        impl_copyFrom( _rSource );
    }

    //--------------------------------------------------------------------
    ModelControllerConnector& ModelControllerConnector::operator=( const ModelControllerConnector& _rSource )
    {
        if ( this != &_rSource )
            impl_copyFrom( _rSource );
        return *this;
    }

    //--------------------------------------------------------------------
    void ModelControllerConnector::connect( const Reference< XModel >& _rxModel, const Reference< XController >& _rxController )
    {
        impl_disconnect();

        m_xModel = _rxModel;
        m_xController = _rxController;

        impl_connect();
    }

    //--------------------------------------------------------------------
    void ModelControllerConnector::impl_copyFrom( const ModelControllerConnector& _rSource )
    {
        Model aNewModel( _rSource.m_xModel );
        Controller aNewController( _rSource.m_xController );

        impl_disconnect();

        m_xModel = aNewModel;
        m_xController = aNewController;

        impl_connect();
    }

    //--------------------------------------------------------------------
    ModelControllerConnector::~ModelControllerConnector()
    {
        impl_disconnect();
        DBG_DTOR( ModelControllerConnector, NULL );
    }

    //--------------------------------------------------------------------
    void ModelControllerConnector::impl_connect()
    {
        try
        {
            Reference< XModel > xModel = m_xModel;
            if ( xModel.is() && m_xController.is() )
                xModel->connectController( m_xController );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void ModelControllerConnector::impl_disconnect()
    {
        try
        {
            Reference< XModel > xModel = m_xModel;
            if ( xModel.is() && m_xController.is() )
                xModel->disconnectController( m_xController );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace dbaui
//........................................................................

