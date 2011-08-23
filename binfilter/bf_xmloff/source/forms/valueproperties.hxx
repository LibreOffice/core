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

#ifndef _XMLOFF_FORMS_VALUEPROPERTIES_HXX_
#define _XMLOFF_FORMS_VALUEPROPERTIES_HXX_

#include "controlelement.hxx"
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OValuePropertiesMetaData
    //=====================================================================
    class OValuePropertiesMetaData
    {
    protected:
        OValuePropertiesMetaData() { }

    public:
        /** calculate the property names for the <em>current-value</em> and the <em>value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpCurrentValuePropertyName,
            sal_Char const * & _rpValuePropertyName);

        /** calculate the property names for the <em>min-value</em> and the <em>max-value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValueLimitPropertyNames(
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpMinValuePropertyName,
            sal_Char const * & _rpMaxValuePropertyName);

        /** calculate the names of the properties which, at runtime, are used for <em>value</em> and
            <em>default value</em>.
        */ 
        static void getRuntimeValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpValuePropertyName,
            sal_Char const * & _rpDefaultValuePropertyName);
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_VALUEPROPERTIES_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
