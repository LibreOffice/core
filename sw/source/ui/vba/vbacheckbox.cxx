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
#include "vbacheckbox.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <ecmaflds.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaCheckBox::SwVbaCheckBox( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& rModel, const uno::Reference< text::XFormField >& xFormField ) throw ( uno::RuntimeException ) : SwVbaCheckBox_BASE( rParent, rContext ), mxModel( rModel ), mxFormField( xFormField )
{
    rtl::OUString sType = mxFormField->getFieldType();
    if( !sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX ) )
    {
        throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("It is not a CheckBox")), uno::Reference< uno::XInterface >() );
    }
}

SwVbaCheckBox::~SwVbaCheckBox()
{
}

sal_Bool SAL_CALL SwVbaCheckBox::getValue() throw ( uno::RuntimeException )
{
    sal_Bool bValue = sal_False;
    sal_Int32 nCount = mxFormField->getParamCount();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( mxFormField->getParamName(i).equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX_CHECKED ) )
        {
            if( mxFormField->getParamValue(i).equalsIgnoreAsciiCaseAscii("on") )
                bValue = sal_True;
            else
                bValue = sal_False;
            break;
        }
    }
    return bValue;
}

void SAL_CALL SwVbaCheckBox::setValue( sal_Bool value ) throw ( uno::RuntimeException )
{
    rtl::OUString sValue = value ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("on")) : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("off"));
    mxFormField->addParam( rtl::OUString::createFromAscii( ECMA_FORMCHECKBOX_CHECKED ), sValue, sal_True );
}

rtl::OUString&
SwVbaCheckBox::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaCheckBox") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaCheckBox::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.CheckBox" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
