/*************************************************************************
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

#include "precompiled_xmloff.hxx"

#include "forms/form_handler_factory.hxx"
#include "vcl_date_handler.hxx"
#include "vcl_time_handler.hxx"

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    namespace
    {
        static PPropertyHandler s_pVCLDateHandler = NULL;
        static PPropertyHandler s_pVCLTimeHandler = NULL;
    }

    //==================================================================================================================
    //= FormHandlerFactory
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PPropertyHandler FormHandlerFactory::getFormPropertyHandler( const PropertyId i_propertyId )
    {
        PPropertyHandler pHandler( NULL );

        switch ( i_propertyId )
        {
        case PID_DATE_MIN:
        case PID_DATE_MAX:
        case PID_DEFAULT_DATE:
        case PID_DATE:
            if ( s_pVCLDateHandler.get() == NULL )
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if ( s_pVCLDateHandler == NULL )
                    s_pVCLDateHandler = new VCLDateHandler();
            }
            pHandler = s_pVCLDateHandler;
            break;

        case PID_TIME_MIN:
        case PID_TIME_MAX:
        case PID_DEFAULT_TIME:
        case PID_TIME:
            if ( s_pVCLTimeHandler.get() == NULL )
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if ( s_pVCLTimeHandler == NULL )
                    s_pVCLTimeHandler = new VCLTimeHandler();
            }
            pHandler = s_pVCLTimeHandler;
            break;

        default:
            OSL_ENSURE( false, "FormHandlerFactory::getFormPropertyHandler: unknown property ID!" );
            break;
        }

        return pHandler;
    }

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................
