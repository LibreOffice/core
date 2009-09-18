/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbaautotextentry.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAutoTextEntry::SwVbaAutoTextEntry( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XAutoTextEntry >& xEntry ) throw ( uno::RuntimeException ) :
    SwVbaAutoTextEntry_BASE( rParent, rContext ), mxEntry( xEntry )
{
}

SwVbaAutoTextEntry::~SwVbaAutoTextEntry()
{
}

uno::Reference< word::XRange > SAL_CALL SwVbaAutoTextEntry::Insert( const uno::Reference< word::XRange >& _where, const uno::Any& _richtext ) throw ( uno::RuntimeException )
{
    SwVbaRange* pWhere = dynamic_cast<SwVbaRange*>( _where.get() );
    if( pWhere )
    {
        uno::Reference< text::XTextRange > xTextRange = pWhere->getXTextRange();
        xTextRange->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("x") ) ); // set marker
        uno::Reference< text::XTextRange > xEndMarker = xTextRange->getEnd();
        xEndMarker->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("x") ) ); // set marker
        uno::Reference< text::XText > xText = pWhere->getXText();
        mxEntry->applyTo( xEndMarker->getStart() );
        uno::Reference< text::XTextCursor > xTC = xText->createTextCursorByRange( xTextRange->getStart() );
        xTC->goRight( 1, sal_True );
        xTC->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("") ) ); // remove marker
        xEndMarker->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("") ) ); // remove marker
        xTC->gotoRange( xEndMarker, sal_True );
        pWhere->setXTextCursor( xTC );
    }
    return uno::Reference< word::XRange >( pWhere );
}

rtl::OUString&
SwVbaAutoTextEntry::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAutoTextEntry") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaAutoTextEntry::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.AutoTextEntry" ) );
    }
    return aServiceNames;
}


SwVbaAutoTextEntries::SwVbaAutoTextEntries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess ) throw (uno::RuntimeException) : SwVbaAutoTextEntries_BASE( xParent, xContext, xIndexAccess ), mxAutoTextEntryAccess( xIndexAccess )
{
}

// XEnumerationAccess
uno::Type
SwVbaAutoTextEntries::getElementType() throw (uno::RuntimeException)
{
    return word::XAutoTextEntry::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaAutoTextEntries::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaAutoTextEntries::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XAutoTextEntry > xEntry( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XAutoTextEntry >( new SwVbaAutoTextEntry( this, mxContext, xEntry ) ) );
}

rtl::OUString&
SwVbaAutoTextEntries::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAutoTextEntries") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaAutoTextEntries::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.AutoTextEntries") );
    }
    return sNames;
}
