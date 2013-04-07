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
    OUString sType = mxFormField->getFieldType();
    if( !sType.equalsIgnoreAsciiCaseAscii( ECMA_FORMCHECKBOX ) )
    {
        throw uno::RuntimeException( OUString(
            "It is not a CheckBox"), uno::Reference< uno::XInterface >() );
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
            if( mxFormField->getParamValue(i).equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("on")) )
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
    OUString sValue = value ? OUString("on") : OUString("off");
    mxFormField->addParam( OUString( ECMA_FORMCHECKBOX_CHECKED ), sValue, sal_True );
}

OUString
SwVbaCheckBox::getServiceImplName()
{
    return OUString("SwVbaCheckBox");
}

uno::Sequence< OUString >
SwVbaCheckBox::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString( "ooo.vba.word.CheckBox"  );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
