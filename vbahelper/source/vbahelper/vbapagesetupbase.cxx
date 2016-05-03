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
#include <vbahelper/vbapagesetupbase.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

VbaPageSetupBase::VbaPageSetupBase(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
    : VbaPageSetupBase_BASE( xParent, xContext )
    , mnOrientLandscape(0)
    , mnOrientPortrait(0)
{
}

double SAL_CALL VbaPageSetupBase::getTopMargin() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 topMargin = 0;

    try
    {
        bool headerOn = false;
        sal_Int32 headerHeight = 0;

        uno::Any aValue = mxPageProps->getPropertyValue( "HeaderIsOn" );
        aValue >>= headerOn;

        aValue = mxPageProps->getPropertyValue( "TopMargin" );
        aValue >>= topMargin;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( "HeaderHeight" );
            aValue >>= headerHeight;
            topMargin = topMargin + headerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( topMargin );
}

void SAL_CALL VbaPageSetupBase::setTopMargin( double margin ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 topMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );

    try
    {
        bool headerOn = false;
        sal_Int32 headerHeight = 0;

        uno::Any aValue = mxPageProps->getPropertyValue( "HeaderIsOn" );
        aValue >>= headerOn;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( "HeaderHeight" );
            aValue >>= headerHeight;
            topMargin -= headerHeight;
        }

        mxPageProps->setPropertyValue( "TopMargin" , uno::Any(topMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getBottomMargin() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 bottomMargin = 0;

    try
    {
        bool footerOn = false;
        sal_Int32 footerHeight = 0;

        uno::Any aValue = mxPageProps->getPropertyValue( "FooterIsOn" );
        aValue >>= footerOn;

        aValue = mxPageProps->getPropertyValue( "BottomMargin" );
        aValue >>= bottomMargin;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( "FooterHeight" );
            aValue >>= footerHeight;
            bottomMargin += footerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( bottomMargin );
}

void SAL_CALL VbaPageSetupBase::setBottomMargin( double margin ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 bottomMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );

    try
    {
        bool footerOn = false;
        sal_Int32 footerHeight = 0;

        uno::Any aValue = mxPageProps->getPropertyValue( "FooterIsOn" );
        aValue >>= footerOn;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( "FooterHeight" );
            aValue >>= footerHeight;
            bottomMargin -= footerHeight;
        }

        mxPageProps->setPropertyValue( "BottomMargin", uno::Any(bottomMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getRightMargin() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 rightMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( "RightMargin" );
        aValue >>= rightMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( rightMargin );
}

void SAL_CALL VbaPageSetupBase::setRightMargin( double margin ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 rightMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( "RightMargin", uno::Any(rightMargin) );
    }
    catch( uno::Exception& )
    {
    }

}

double SAL_CALL VbaPageSetupBase::getLeftMargin() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 leftMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( "LeftMargin" );
        aValue >>= leftMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( leftMargin );
}

void SAL_CALL VbaPageSetupBase::setLeftMargin( double margin ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 leftMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( "LeftMargin", uno::Any(leftMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getHeaderMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 headerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( "TopMargin" );
        aValue >>= headerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( headerMargin );
}

void SAL_CALL VbaPageSetupBase::setHeaderMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 headerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( "TopMargin", uno::Any(headerMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getFooterMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 footerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( "BottomMargin" );
        aValue >>= footerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( footerMargin );
}

void SAL_CALL VbaPageSetupBase::setFooterMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 footerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( "BottomMargin", uno::Any(footerMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL VbaPageSetupBase::getOrientation() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 orientation = mnOrientPortrait;
    try
    {
        bool isLandscape = false;
        uno::Any aValue = mxPageProps->getPropertyValue( "IsLandscape" );
        aValue >>= isLandscape;

        if( isLandscape )
        {
            orientation = mnOrientLandscape;
        }
    }
    catch( uno::Exception& )
    {
    }
    return orientation;
}

void SAL_CALL VbaPageSetupBase::setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException, std::exception)
{
    if( ( orientation != mnOrientPortrait ) &&
        ( orientation != mnOrientLandscape ) )
    {
        DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER, OUString() );
    }

    try
    {
        bool isLandscape = false;
        uno::Any aValue = mxPageProps->getPropertyValue( "IsLandscape" );
        aValue >>= isLandscape;

        bool switchOrientation = false;
        if(( isLandscape && orientation != mnOrientLandscape ) ||
            ( !isLandscape && orientation != mnOrientPortrait ))
        {
            switchOrientation = true;
        }

        if( switchOrientation )
        {
            uno::Any aHeight = mxPageProps->getPropertyValue( "Height" );
            uno::Any aWidth = mxPageProps->getPropertyValue( "Width" );
            mxPageProps->setPropertyValue( "IsLandscape", uno::Any(!isLandscape) );
            mxPageProps->setPropertyValue( "Width" ,  aHeight );
            mxPageProps->setPropertyValue( "Height" , aWidth );
        }
    }
    catch( uno::Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
