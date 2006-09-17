/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkvalue.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:28:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    double aDouble;
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
