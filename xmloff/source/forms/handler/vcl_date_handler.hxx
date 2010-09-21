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

#ifndef XMLOFF_VCL_DATE_HANDLER_HXX
#define XMLOFF_VCL_DATE_HANDLER_HXX

#include "property_handler_base.hxx"

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    //==================================================================================================================
    //= VCLDateHandler
    //==================================================================================================================
    class VCLDateHandler : public PropertyHandlerBase
    {
    public:
        VCLDateHandler();

        // IPropertyHandler
        virtual ::rtl::OUString getAttributeValue( const PropertyValues& i_propertyValues ) const;
        virtual ::rtl::OUString getAttributeValue( const ::com::sun::star::uno::Any& i_propertyValue ) const;
        virtual bool getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const;
    };

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................

#endif // XMLOFF_VCL_DATE_HANDLER_HXX
