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
#include "vbareplacement.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaReplacement::SwVbaReplacement( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< util::XPropertyReplace >& xPropertyReplace ) throw ( uno::RuntimeException ) :
    SwVbaReplacement_BASE( rParent, rContext ), mxPropertyReplace( xPropertyReplace )
{
}

SwVbaReplacement::~SwVbaReplacement()
{
}

::rtl::OUString SAL_CALL SwVbaReplacement::getText() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getReplaceString();
}

void SAL_CALL SwVbaReplacement::setText( const ::rtl::OUString& _text ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setReplaceString( _text );
}

void SAL_CALL SwVbaReplacement::ClearFormatting( ) throw (uno::RuntimeException)
{
    uno::Sequence< beans::PropertyValue > aPropValues;
    mxPropertyReplace->setReplaceAttributes( aPropValues );
}

rtl::OUString&
SwVbaReplacement::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaReplacement") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaReplacement::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Replacement" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
