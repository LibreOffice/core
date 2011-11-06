/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include <vbahelper/vbapagesetupbase.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

VbaPageSetupBase::VbaPageSetupBase(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException): VbaPageSetupBase_BASE( xParent, xContext )
{
}

double SAL_CALL VbaPageSetupBase::getTopMargin() throw (css::uno::RuntimeException)
{
    sal_Bool headerOn = sal_False;
    sal_Int32 topMargin = 0;
    sal_Int32 headerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")));
        aValue >>= headerOn;

        aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")));
        aValue >>= topMargin;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")));
            aValue >>= headerHeight;
            topMargin = topMargin + headerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( topMargin );
}

void SAL_CALL VbaPageSetupBase::setTopMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 topMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    sal_Bool headerOn = sal_False;
    sal_Int32 headerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")));
        aValue >>= headerOn;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")));
            aValue >>= headerHeight;
            topMargin -= headerHeight;
        }

        aValue <<= topMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getBottomMargin() throw (css::uno::RuntimeException)
{
    sal_Bool footerOn = sal_False;
    sal_Int32 bottomMargin = 0;
    sal_Int32 footerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")));
        aValue >>= footerOn;

        aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")));
        aValue >>= bottomMargin;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight")));
            aValue >>= footerHeight;
            bottomMargin += footerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( bottomMargin );
}

void SAL_CALL VbaPageSetupBase::setBottomMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 bottomMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    sal_Bool footerOn = sal_False;
    sal_Int32 footerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")));
        aValue >>= footerOn;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight")));
            aValue >>= footerHeight;
            bottomMargin -= footerHeight;
        }

        aValue <<= bottomMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL VbaPageSetupBase::getRightMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 rightMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin")));
        aValue >>= rightMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( rightMargin );;
}

void SAL_CALL VbaPageSetupBase::setRightMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 rightMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= rightMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }

}

double SAL_CALL VbaPageSetupBase::getLeftMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 leftMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin")));
        aValue >>= leftMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( leftMargin );;
}

void SAL_CALL VbaPageSetupBase::setLeftMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 leftMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= leftMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin")), aValue );
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
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")));
        aValue >>= headerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( headerMargin );;
}

void SAL_CALL VbaPageSetupBase::setHeaderMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 headerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= headerMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")), aValue );
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
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")));
        aValue >>= footerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( footerMargin );;
}

void SAL_CALL VbaPageSetupBase::setFooterMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 footerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= footerMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL VbaPageSetupBase::getOrientation() throw (css::uno::RuntimeException)
{
    sal_Int32 orientation = mnOrientPortrait;
    try
    {
        sal_Bool isLandscape = sal_False;
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")));
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

void SAL_CALL VbaPageSetupBase::setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException)
{
    if( ( orientation != mnOrientPortrait ) &&
        ( orientation != mnOrientLandscape ) )
    {
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }

    try
    {
        sal_Bool isLandscape = sal_False;
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")));
        aValue >>= isLandscape;

        sal_Bool switchOrientation = sal_False;
        if(( isLandscape && orientation != mnOrientLandscape ) ||
            ( !isLandscape && orientation != mnOrientPortrait ))
        {
            switchOrientation = sal_True;
        }

        if( switchOrientation )
        {
            aValue <<= !isLandscape;
            uno::Any aHeight = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height")));
            uno::Any aWidth = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")));
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")), aValue );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")),  aHeight );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height")), aWidth );
        }

        if( isLandscape )
        {
            orientation = mnOrientLandscape;
        }
    }
    catch( uno::Exception& )
    {
    }
}

