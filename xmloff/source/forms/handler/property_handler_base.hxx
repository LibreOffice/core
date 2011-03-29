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

#ifndef XMLOFF_PROPERTY_HANDLER_BASE_HXX
#define XMLOFF_PROPERTY_HANDLER_BASE_HXX

#include "forms/property_handler.hxx"

#include <osl/interlck.h>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    //==================================================================================================================
    //= PropertyHandlerBase
    //==================================================================================================================
    class PropertyHandlerBase : public IPropertyHandler
    {
    protected:
        PropertyHandlerBase()
            :m_refCount( 0 )
        {
        }

        virtual ~PropertyHandlerBase();

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        oslInterlockedCount m_refCount;
    };

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................

#endif // XMLOFF_PROPERTY_HANDLER_BASE_HXX
