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


#include "vbaheaderfooterhelper.hxx"
#include "wordvbahelper.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1;

// Class HeaderFooterHelper

sal_Bool HeaderFooterHelper::isHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );

    sal_Bool isOn = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn"))) >>= isOn;
    if( !isOn )
        return sal_False;

    sal_Bool isShared = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared"))) >>= isShared;

    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderText") );
    if( !isShared )
    {
        if( 0 == xPageCursor->getPage() % 2 )
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextLeft") );
        }
        else
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderTextRight") );
        }
    }

    uno::Reference< text::XText > xText( xStyleProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    //FIXME: can not compare in this way?
    return ( xText == xCurrentText );
}

sal_Bool HeaderFooterHelper::isFirstPageHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        // FIXME: getPage allways returns 1
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        sal_Bool isShared = sal_False;
        xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared"))) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );

    sal_Bool isOn = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn"))) >>= isOn;
    if( !isOn )
        return sal_False;

    sal_Bool isShared = sal_False;
    xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared"))) >>= isShared;

    rtl::OUString aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterText") );
    if( !isShared )
    {
        if( 0 == xPageCursor->getPage() % 2 )
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextLeft") );
        }
        else
        {
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterTextRight") );
        }
    }

    uno::Reference< text::XText > xText( xStyleProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );

    return ( xText == xCurrentText );
}

sal_Bool HeaderFooterHelper::isFirstPageFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isFooter( xModel, xCurrentText ) )
    {
        uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nPage = xPageCursor->getPage();
        return nPage == FIRST_PAGE;
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isEvenPagesFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isFooter( xModel, xCurrentText ) )
    {
        uno::Reference< beans::XPropertySet > xStyleProps( word::getCurrentPageStyle( xModel ), uno::UNO_QUERY_THROW );
        sal_Bool isShared = sal_False;
        xStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared"))) >>= isShared;
        if( !isShared )
        {
            uno::Reference< text::XPageCursor > xPageCursor( word::getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
            return ( 0 == xPageCursor->getPage() % 2 );
        }
    }
    return sal_False;
}
#ifdef TOMORROW
sal_Bool HeaderFooterHelper::isPrimaryHeader( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        return( !( isFirstPageHeader( xModel, xCurrentText ) && isEvenPagesHeader( xModel, xCurrentText ) ) );
    }
    return sal_False;
}

sal_Bool HeaderFooterHelper::isPrimaryFooter( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XText >& xCurrentText ) throw (uno::RuntimeException)
{
    if( isHeader( xModel, xCurrentText ) )
    {
        return( !( isFirstPageFooter( xModel, xCurrentText ) && isEvenPagesFooter( xModel, xCurrentText ) ) );
    }
    return sal_False;
}
#endif
