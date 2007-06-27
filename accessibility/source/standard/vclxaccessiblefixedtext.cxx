/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblefixedtext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:38:16 $
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
#include "precompiled_accessibility.hxx"

// includes --------------------------------------------------------------

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDTEXT_HXX
#include <accessibility/standard/vclxaccessiblefixedtext.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


// -----------------------------------------------------------------------------
// VCLXAccessibleFixedText
// -----------------------------------------------------------------------------

VCLXAccessibleFixedText::VCLXAccessibleFixedText( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleFixedText::~VCLXAccessibleFixedText()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedText::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    if ( GetWindow() && GetWindow()->GetStyle() & WB_WORDBREAK )
        rStateSet.AddState( AccessibleStateType::MULTI_LINE );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedText::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
{
    // TODO
    OCommonAccessibleText::implGetLineBoundary( rBoundary, nIndex );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleFixedText::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleFixedText" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleFixedText::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleFixedText" );
    return aNames;
}

// -----------------------------------------------------------------------------
