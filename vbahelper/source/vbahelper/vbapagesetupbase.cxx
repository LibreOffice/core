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
#include <basic/sberrors.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

VbaPageSetupBase::VbaPageSetupBase(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext )
    : VbaPageSetupBase_BASE( xParent, xContext )
    , mnOrientLandscape(0)
    , mnOrientPortrait(0)
{
}

double SAL_CALL VbaPageSetupBase::getTopMargin()
{
    sal_Int32 topMargin = 0;

    try
    {
        bool headerOn = false;

        uno::Any aValue = mxPageProps->getPropertyValue( u"HeaderIsOn"_ustr );
        aValue >>= headerOn;

        aValue = mxPageProps->getPropertyValue( u"TopMargin"_ustr );
        aValue >>= topMargin;

        if( headerOn )
        {
            sal_Int32 headerHeight = 0;
            aValue = mxPageProps->getPropertyValue( u"HeaderHeight"_ustr );
            aValue >>= headerHeight;
            topMargin = topMargin + headerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( topMargin );
}

void SAL_CALL VbaPageSetupBase::setTopMargin( double margin )
{
    sal_Int32 topMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );

    try
    {
        bool headerOn = false;

        uno::Any aValue = mxPageProps->getPropertyValue( u"HeaderIsOn"_ustr );
        aValue >>= headerOn;

        if( headerOn )
        {
            sal_Int32 headerHeight = 0;
            aValue = mxPageProps->getPropertyValue( u"HeaderHeight"_ustr );
            aValue >>= headerHeight;
            topMargin -= headerHeight;
        }

        mxPageProps->setPropertyValue( u"TopMargin"_ustr , uno::Any(topMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getBottomMargin()
{
    sal_Int32 bottomMargin = 0;

    try
    {
        bool footerOn = false;

        uno::Any aValue = mxPageProps->getPropertyValue( u"FooterIsOn"_ustr );
        aValue >>= footerOn;

        aValue = mxPageProps->getPropertyValue( u"BottomMargin"_ustr );
        aValue >>= bottomMargin;

        if( footerOn )
        {
            sal_Int32 footerHeight = 0;
            aValue = mxPageProps->getPropertyValue( u"FooterHeight"_ustr );
            aValue >>= footerHeight;
            bottomMargin += footerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( bottomMargin );
}

void SAL_CALL VbaPageSetupBase::setBottomMargin( double margin )
{
    sal_Int32 bottomMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );

    try
    {
        bool footerOn = false;

        uno::Any aValue = mxPageProps->getPropertyValue( u"FooterIsOn"_ustr );
        aValue >>= footerOn;

        if( footerOn )
        {
            sal_Int32 footerHeight = 0;
            aValue = mxPageProps->getPropertyValue( u"FooterHeight"_ustr );
            aValue >>= footerHeight;
            bottomMargin -= footerHeight;
        }

        mxPageProps->setPropertyValue( u"BottomMargin"_ustr, uno::Any(bottomMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getRightMargin()
{
    sal_Int32 rightMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( u"RightMargin"_ustr );
        aValue >>= rightMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( rightMargin );
}

void SAL_CALL VbaPageSetupBase::setRightMargin( double margin )
{
    sal_Int32 rightMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( u"RightMargin"_ustr, uno::Any(rightMargin) );
    }
    catch( uno::Exception& )
    {
    }

}

double SAL_CALL VbaPageSetupBase::getLeftMargin()
{
    sal_Int32 leftMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( u"LeftMargin"_ustr );
        aValue >>= leftMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( leftMargin );
}

void SAL_CALL VbaPageSetupBase::setLeftMargin( double margin )
{
    sal_Int32 leftMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( u"LeftMargin"_ustr, uno::Any(leftMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double VbaPageSetupBase::getHeaderMargin()
{
    sal_Int32 headerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( u"TopMargin"_ustr );
        aValue >>= headerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( headerMargin );
}

void VbaPageSetupBase::setHeaderMargin( double margin )
{
    sal_Int32 headerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( u"TopMargin"_ustr, uno::Any(headerMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

double VbaPageSetupBase::getFooterMargin()
{
    sal_Int32 footerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( u"BottomMargin"_ustr );
        aValue >>= footerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( footerMargin );
}

void VbaPageSetupBase::setFooterMargin( double margin )
{
    sal_Int32 footerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        mxPageProps->setPropertyValue( u"BottomMargin"_ustr, uno::Any(footerMargin) );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL VbaPageSetupBase::getOrientation()
{
    sal_Int32 orientation = mnOrientPortrait;
    try
    {
        bool isLandscape = false;
        uno::Any aValue = mxPageProps->getPropertyValue( u"IsLandscape"_ustr );
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

void SAL_CALL VbaPageSetupBase::setOrientation( sal_Int32 orientation )
{
    if( ( orientation != mnOrientPortrait ) &&
        ( orientation != mnOrientLandscape ) )
    {
        DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER );
    }

    try
    {
        bool isLandscape = false;
        uno::Any aValue = mxPageProps->getPropertyValue( u"IsLandscape"_ustr );
        aValue >>= isLandscape;

        bool switchOrientation = false;
        if(( isLandscape && orientation != mnOrientLandscape ) ||
            ( !isLandscape && orientation != mnOrientPortrait ))
        {
            switchOrientation = true;
        }

        if( switchOrientation )
        {
            uno::Any aHeight = mxPageProps->getPropertyValue( u"Height"_ustr );
            uno::Any aWidth = mxPageProps->getPropertyValue( u"Width"_ustr );
            mxPageProps->setPropertyValue( u"IsLandscape"_ustr, uno::Any(!isLandscape) );
            mxPageProps->setPropertyValue( u"Width"_ustr ,  aHeight );
            mxPageProps->setPropertyValue( u"Height"_ustr , aWidth );
        }
    }
    catch( uno::Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
