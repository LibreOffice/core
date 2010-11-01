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
#include "vbaformfield.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <ecmaflds.hxx>
#include "vbacheckbox.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFormField::SwVbaFormField( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& rModel, const uno::Reference< text::XFormField >& xFormField ) throw ( uno::RuntimeException ) : SwVbaFormField_BASE( rParent, rContext ), mxModel( rModel ), mxFormField( xFormField )
{
}

SwVbaFormField::~SwVbaFormField()
{
}

rtl::OUString SAL_CALL SwVbaFormField::getResult() throw ( uno::RuntimeException )
{
    rtl::OUString sResult;
    rtl::OUString sType = mxFormField->getFieldType();
    if( sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMTEXT ) )
    {
        uno::Reference< text::XTextContent > xTextContent( mxFormField, uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextRange > xTextRange = xTextContent->getAnchor();
        sResult = xTextRange->getString();
    }
    else if( sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX ) )
    {
        sal_Int32 nValue = 0;
        sal_Int32 nCount = mxFormField->getParamCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            if( mxFormField->getParamName(i).equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX_CHECKED ) )
            {
                if( mxFormField->getParamValue(i).equalsIgnoreAsciiCaseAscii("on") )
                    nValue = 1;
                else
                    nValue = 0;
                break;
            }

        }
        sResult = rtl::OUString::valueOf( nValue );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
    return sResult;
}

void SAL_CALL SwVbaFormField::setResult( const rtl::OUString& result ) throw ( uno::RuntimeException )
{
    rtl::OUString sType = mxFormField->getFieldType();
    if( sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMTEXT ) )
    {
        uno::Reference< text::XTextContent > xTextContent( mxFormField, uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextRange > xTextRange = xTextContent->getAnchor();
        xTextRange->setString( result );
    }
    else if( sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX ) )
    {
        // do nothing
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
}

sal_Bool SAL_CALL SwVbaFormField::getEnabled() throw ( uno::RuntimeException )
{
    //allways return true
    return sal_True;
}

void SAL_CALL SwVbaFormField::setEnabled( sal_Bool/* enabled */ ) throw ( uno::RuntimeException )
{
    // not support in Writer
}

uno::Any SAL_CALL SwVbaFormField::CheckBox() throw ( uno::RuntimeException )
{
    return uno::makeAny( uno::Reference< word::XCheckBox >( new SwVbaCheckBox( this, mxContext, mxModel, mxFormField ) ) );
}

rtl::OUString&
SwVbaFormField::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaFormField") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaFormField::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.FormField" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
