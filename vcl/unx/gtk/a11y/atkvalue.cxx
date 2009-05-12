/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkvalue.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_vcl.hxx"

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star;

static accessibility::XAccessibleValue*
    getValue( AtkValue *pValue ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pValue );
    if( pWrap )
    {
        if( !pWrap->mpValue && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleValue::static_type(NULL) );
            pWrap->mpValue = reinterpret_cast< accessibility::XAccessibleValue * > (any.pReserved);
            pWrap->mpValue->acquire();
        }

        return pWrap->mpValue;
    }

    return NULL;
}

static void anyToGValue( uno::Any aAny, GValue *pValue )
{
    // FIXME: expand to lots of types etc.
    double aDouble=0;
    aAny >>= aDouble;

    memset( pValue,  0, sizeof( GValue ) );
    g_value_init( pValue, G_TYPE_DOUBLE );
    g_value_set_double( pValue, aDouble );
}

extern "C" {

static void
value_wrapper_get_current_value( AtkValue *value,
                                 GValue   *gval )
{
    try {
        accessibility::XAccessibleValue* pValue = getValue( value );
        if( pValue )
            anyToGValue( pValue->getCurrentValue(), gval );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCurrentValue()" );
    }
}

static void
value_wrapper_get_maximum_value( AtkValue *value,
                                 GValue   *gval )
{
    try {
        accessibility::XAccessibleValue* pValue = getValue( value );
        if( pValue )
            anyToGValue( pValue->getMaximumValue(), gval );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCurrentValue()" );
    }
}

static void
value_wrapper_get_minimum_value( AtkValue *value,
                                 GValue   *gval )
{
    try {
        accessibility::XAccessibleValue* pValue = getValue( value );
        if( pValue )
            anyToGValue( pValue->getMinimumValue(), gval );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCurrentValue()" );
    }
}

static gboolean
value_wrapper_set_current_value( AtkValue     *value,
                                 const GValue *gval )
{
    try {
        accessibility::XAccessibleValue* pValue = getValue( value );
        if( pValue )
        {
            // FIXME - this needs expanding
            double aDouble = g_value_get_double( gval );
            uno::Any aAny;
            aAny <<= aDouble;
            return pValue->setCurrentValue( aAny );
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCurrentValue()" );
    }

    return FALSE;
}

} // extern "C"

void
valueIfaceInit (AtkValueIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_current_value = value_wrapper_get_current_value;
  iface->get_maximum_value = value_wrapper_get_maximum_value;
  iface->get_minimum_value = value_wrapper_get_minimum_value;
  iface->set_current_value = value_wrapper_set_current_value;
}
