/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
                if( mxFormField->getParamValue(i).equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("on")) )
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
        throw uno::RuntimeException( rtl::OUString("Not implemented"), uno::Reference< uno::XInterface >() );
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
        throw uno::RuntimeException( rtl::OUString("Not implemented"), uno::Reference< uno::XInterface >() );
    }
}

sal_Bool SAL_CALL SwVbaFormField::getEnabled() throw ( uno::RuntimeException )
{
    //always return true
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

rtl::OUString
SwVbaFormField::getServiceImplName()
{
    return rtl::OUString("SwVbaFormField");
}

uno::Sequence< rtl::OUString >
SwVbaFormField::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.FormField" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
