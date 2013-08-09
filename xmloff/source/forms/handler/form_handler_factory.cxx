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

#include "forms/form_handler_factory.hxx"
#include "vcl_date_handler.hxx"
#include "vcl_time_handler.hxx"

namespace xmloff
{

    namespace
    {
        static PPropertyHandler s_pVCLDateHandler = NULL;
        static PPropertyHandler s_pVCLTimeHandler = NULL;
    }

    //= FormHandlerFactory
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

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
