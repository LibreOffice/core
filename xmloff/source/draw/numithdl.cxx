/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numithdl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:40:19 $
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
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_PROPERTYHANDLER_NUMRULE_HXX
#include <numithdl.hxx>
#endif

using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLNumRulePropHdl
//

XMLNumRulePropHdl::XMLNumRulePropHdl( ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > xNumRuleCompare )
: mxNumRuleCompare( xNumRuleCompare )
{
}

XMLNumRulePropHdl::~XMLNumRulePropHdl()
{
    // Nothing to do
}

bool XMLNumRulePropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    return mxNumRuleCompare.is() && mxNumRuleCompare->compare( r1, r2 ) == 0;
}

sal_Bool XMLNumRulePropHdl::importXML( const ::rtl::OUString& /*rStrImpValue*/, ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

sal_Bool XMLNumRulePropHdl::exportXML( ::rtl::OUString& /*rStrExpValue*/, const ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

