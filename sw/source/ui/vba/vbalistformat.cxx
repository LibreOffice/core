/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbalistformat.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdListApplyTo.hpp>
#include <ooo/vba/word/WdDefaultListBehavior.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "vbalisttemplate.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaListFormat::SwVbaListFormat( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextRange >& xTextRange ) throw ( uno::RuntimeException ) : SwVbaListFormat_BASE( rParent, rContext ), mxTextRange( xTextRange )
{
}

SwVbaListFormat::~SwVbaListFormat()
{
}

void SAL_CALL SwVbaListFormat::ApplyListTemplate( const css::uno::Reference< word::XListTemplate >& ListTemplate, const css::uno::Any& ContinuePreviousList, const css::uno::Any& ApplyTo, const css::uno::Any& DefaultListBehavior ) throw (css::uno::RuntimeException)
{
    sal_Bool bContinuePreviousList = sal_True;
    if( ContinuePreviousList.hasValue() )
        ContinuePreviousList >>= bContinuePreviousList;

    // "applyto" must be current selection
    sal_Int32 bApplyTo = word::WdListApplyTo::wdListApplyToSelection;
    if( ApplyTo.hasValue() )
        ApplyTo >>= bApplyTo;
    if( bApplyTo != word::WdListApplyTo::wdListApplyToSelection )
        throw uno::RuntimeException();

    // default behaviour must be wdWord8ListBehavior
    sal_Int32 nDefaultListBehavior = word::WdDefaultListBehavior::wdWord8ListBehavior;
    if( DefaultListBehavior.hasValue() )
        DefaultListBehavior >>= nDefaultListBehavior;
    if( nDefaultListBehavior != word::WdDefaultListBehavior::wdWord8ListBehavior )
        throw uno::RuntimeException();

    SwVbaListTemplate* pListTemplate = dynamic_cast< SwVbaListTemplate* >( ListTemplate.get() );

    uno::Reference< container::XEnumerationAccess > xEnumAccess( mxTextRange, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    sal_Bool isFirstElement = sal_True;
    while( xEnum->hasMoreElements() )
    {
        uno::Reference< beans::XPropertySet > xProps( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        if( isFirstElement )
        {
            sal_Bool isNumberingRestart = !bContinuePreviousList;
            xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart") ), uno::makeAny( isNumberingRestart ) );
            if( isNumberingRestart )
            {
                sal_Int16 nStartValue = 1;
                xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue") ), uno::makeAny( nStartValue ) );
            }
            isFirstElement = sal_False;
        }
        else
        {
            xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart") ), uno::makeAny( sal_False ) );
        }
        pListTemplate->applyListTemplate( xProps );
    }
}

void SAL_CALL SwVbaListFormat::ConvertNumbersToText(  ) throw (css::uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

rtl::OUString&
SwVbaListFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaListFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaListFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.ListFormat" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
