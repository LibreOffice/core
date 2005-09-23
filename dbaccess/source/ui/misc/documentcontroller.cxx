/*************************************************************************
 *
 *  $RCSfile: documentcontroller.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:39:23 $
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

#ifndef DBACCESS_SOURCE_UI_INC_DOCUMENTCONTROLLER_HXX
#include "documentcontroller.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
        DBG_ASSERT( m_xModel.is() && m_xController.is(), "ModelControllerConnector::ModelControllerConnector: invalid model or controller!" );
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
    void ModelControllerConnector::swap( ModelControllerConnector& _rSource )
    {
        ModelControllerConnector aTemp( _rSource );
        _rSource.impl_copyFrom( *this );
        impl_copyFrom( aTemp );
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
            if ( m_xModel.is() && m_xController.is() )
                m_xModel->connectController( m_xController );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ModelControllerConnector::impl_connect: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ModelControllerConnector::impl_disconnect()
    {
        try
        {
            if ( m_xModel.is() && m_xController.is() )
                m_xModel->disconnectController( m_xController );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ModelControllerConnector::impl_disconnect: caught an exception!" );
        }
    }

//........................................................................
} // namespace dbaui
//........................................................................

