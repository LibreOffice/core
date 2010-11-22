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

#include "PrintOptTest.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <unotools/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>

namespace css = ::com::sun::star;

// using test  only
#define ROOTNODE_PRINTOPTION                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common/Print/Option"))
#define PROPERTYNAME_REDUCETRANSPARENCY                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceTransparency"))
#define PROPERTYNAME_REDUCEDTRANSPARENCYMODE            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedTransparencyMode"))
#define PROPERTYNAME_REDUCEGRADIENTS                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceGradients"))
#define PROPERTYNAME_REDUCEDGRADIENTMODE                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientMode"))
#define PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT           rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientStepCount"))
#define PROPERTYNAME_REDUCEBITMAPS                      rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceBitmaps"))
#define PROPERTYNAME_REDUCEDBITMAPMODE                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapMode"))
#define PROPERTYNAME_REDUCEDBITMAPRESOLUTION            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapResolution"))
#define PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapIncludesTransparency"))
#define PROPERTYNAME_CONVERTTOGREYSCALES                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConvertToGreyscales"))

PrintOptTest::PrintOptTest()
{
    m_xCfg = css::uno::Reference< css::container::XNameAccess >(
            ::comphelper::ConfigurationHelper::openConfig(
            ::utl::getProcessServiceFactory(),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common/Print/Option")),
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY);

        if (m_xCfg.is())
        {
            //UniString  sTmp = UniString("printer");
            //xub_StrLen nTokenCount = sTmp.GetTokenCount('/');
            //sTmp = sTmp.GetToken(nTokenCount - 1, '/');
            m_xCfg->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Printer"))) >>= m_xNode;
        }
}

sal_Int16 PrintOptTest::impl_GetReducedTransparencyMode() const
{
    sal_Int16 nRet = 0;
    if (m_xNode.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDTRANSPARENCYMODE) >>= nRet;
    }
    return  nRet;
}
void PrintOptTest::impl_SetReducedTransparencyMode(sal_Int16 nMode )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Int16 nUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDTRANSPARENCYMODE) >>= nUpdate;
            if (nUpdate != nMode)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDTRANSPARENCYMODE, css::uno::makeAny(nMode));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Bool PrintOptTest::impl_IsReduceTransparency() const
{
    sal_Bool bRet = sal_False;
    if (m_xNode.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
            xSet->getPropertyValue(PROPERTYNAME_REDUCETRANSPARENCY) >>= bRet;
    }
    return bRet;
}
void PrintOptTest::impl_SetReduceTransparency(sal_Bool bState )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Bool bUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCETRANSPARENCY) >>= bUpdate;
            if (bUpdate != bState)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCETRANSPARENCY, css::uno::makeAny(bState));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Bool PrintOptTest::impl_IsReduceGradients() const
{
    sal_Bool bRet = sal_False;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEGRADIENTS) >>= bRet;
        }
    }
    return bRet;
}

void PrintOptTest::impl_SetReduceGradients(sal_Bool bState )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Bool bUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEGRADIENTS) >>= bUpdate;
            if (bUpdate != bState)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEGRADIENTS, css::uno::makeAny(bState));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Int16 PrintOptTest::impl_GetReducedGradientMode() const
{
    sal_Int16 nRet = 0;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTMODE) >>= nRet;
        }
    }
    return nRet;
}

void PrintOptTest::impl_SetReducedGradientMode(sal_Int16 nMode )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Int16 nUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTMODE) >>= nUpdate;
            if (nUpdate != nMode)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDGRADIENTMODE, css::uno::makeAny(nMode));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Int16 PrintOptTest::impl_GetReducedGradientStepCount() const
{
    sal_Int16 nRet = 64;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT) >>= nRet;
        }
    }
    return nRet;
}
void PrintOptTest::impl_SetReducedGradientStepCount(sal_Int16 nStepCount )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Int16 nUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT) >>= nUpdate;
            if (nUpdate != nStepCount)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT, css::uno::makeAny(nStepCount));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Bool PrintOptTest::impl_IsReduceBitmaps() const
{
    sal_Bool bRet = sal_False;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEBITMAPS) >>= bRet;
        }
    }
    return bRet;
}

void PrintOptTest::impl_SetReduceBitmaps(sal_Bool bState )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Bool bUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEBITMAPS) >>= bUpdate;
            if (bUpdate != bState)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEBITMAPS, css::uno::makeAny(bState));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Int16 PrintOptTest::impl_GetReducedBitmapMode() const
{
    sal_Int16 nRet = 1;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPMODE) >>= nRet;
        }
    }
    return nRet;
}

void PrintOptTest::impl_SetReducedBitmapMode(sal_Int16 nMode )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Int16 nUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPMODE) >>= nUpdate;
            if (nUpdate != nMode)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDBITMAPMODE, css::uno::makeAny(nMode));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Int16  PrintOptTest::impl_GetReducedBitmapResolution() const
{
    sal_Int16 nRet = 3;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPRESOLUTION) >>= nRet;
        }
    }
    return  nRet;
}

void PrintOptTest::impl_SetReducedBitmapResolution(sal_Int16 nResolution )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Int16 nUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPRESOLUTION) >>= nUpdate;
            if (nUpdate != nResolution)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDBITMAPRESOLUTION, css::uno::makeAny(nResolution));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Bool PrintOptTest::impl_IsReducedBitmapIncludesTransparency() const
{
    sal_Bool bRet = sal_True;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY) >>= bRet;
        }
    }
    return  bRet;
}

void PrintOptTest::impl_SetReducedBitmapIncludesTransparency(sal_Bool bState )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Bool bUpdate;
            xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY) >>= bUpdate;
            if (bUpdate != bState)
            {
                xSet->setPropertyValue( PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY, css::uno::makeAny(bState));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}

sal_Bool PrintOptTest::impl_IsConvertToGreyscales() const
{
    sal_Bool bRet = sal_False;
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->getPropertyValue(PROPERTYNAME_CONVERTTOGREYSCALES) >>= bRet;
        }
    }
    return  bRet;
}

void PrintOptTest::impl_SetConvertToGreyscales(sal_Bool bState )
{
    if (m_xNode.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if (xSet.is())
        {
            sal_Bool bUpdate;
            xSet->getPropertyValue(PROPERTYNAME_CONVERTTOGREYSCALES) >>= bUpdate;
            if (bUpdate != bState)
            {
                xSet->setPropertyValue( PROPERTYNAME_CONVERTTOGREYSCALES, css::uno::makeAny(bState));
                ::comphelper::ConfigurationHelper::flush(m_xCfg);
            }
        }
    }
}


PrintOptTest::~PrintOptTest()
{
}

void PrintOptTest::impl_checkPrint()
{
    //test SetReduceTransparency()
    sal_Bool bNewValue = sal_False;
    sal_Bool bOldValue = sal_False;
    bOldValue = PrintOptTest::impl_IsReduceTransparency();
    bNewValue = !bOldValue;
    aPrintOpt.SetReduceTransparency(bNewValue) ;
    bNewValue = impl_IsReduceTransparency();
   // if(bNewValue != bOldValue) // test the old source
    if ( bNewValue == bOldValue ) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReduceTransparency() error!"),
            0);
    }

    //test IsReduceTransparemcy()
    bNewValue = bOldValue = sal_False;
    bOldValue = impl_IsReduceTransparency();
    bNewValue = !bOldValue;
    impl_SetReduceTransparency(bNewValue);
    bNewValue = aPrintOpt.IsReduceTransparency();
    //if(bNewValue != bOldValue) // test the old source
    if(bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsReduceTransparency() error!"),
            0);
    }

    // test SetReducedTransparencyMode()
    sal_Int16 nOldMode, nNewMode;
    nOldMode = nNewMode = 0;
    nOldMode = impl_GetReducedTransparencyMode();
    nNewMode = nOldMode + 1;
    aPrintOpt.SetReducedTransparencyMode( nNewMode );
    nNewMode = impl_GetReducedTransparencyMode();
    //if(nNewMode != nOldMode)      // test the old source
    if ( nNewMode == nOldMode ) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReducedTransparencyMode() error!"),
            0);
    }

    //test IsReducedTransparencyMode()
    nOldMode = nNewMode = 0;
    nOldMode = impl_GetReducedTransparencyMode();
    nNewMode = nOldMode + 1;
    impl_SetReducedTransparencyMode(nNewMode);
    nNewMode = aPrintOpt.GetReducedTransparencyMode();
    //if(nNewMode != nOldMode)    // test the old source
    if(nNewMode == nOldMode)  // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsReducedTransparencyMode() error!"
            "nOldMode's value is :"),
            0);
    }

    // test the SetReduceGradients()
    bNewValue = bOldValue = sal_False;
    bOldValue = impl_IsReduceGradients();
    bNewValue = !bOldValue;
    aPrintOpt.SetReduceGradients(bNewValue);
    bNewValue = impl_IsReduceGradients();
    //if (bNewValue != bOldValue)   //test the old source
    if (bNewValue == bOldValue)   //test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReduceGradients() error!"),
            0);

    }

    // test the IsReduceGradients()
    bNewValue = bOldValue = sal_False;
    bOldValue = impl_IsReduceGradients();
    bNewValue = !bOldValue;
    this->impl_SetReduceGradients(bNewValue);
    bNewValue = aPrintOpt.IsReduceGradients();
   // if (bNewValue != bOldValue)   // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsReduceGradients() error!"),
            0);

    }

    //test SetRedecedGradientMode()
    nOldMode = nNewMode = 0;
    nOldMode = this->impl_GetReducedGradientMode();
    nNewMode = nOldMode + 1;
    aPrintOpt.SetReducedGradientMode(nNewMode);
    nNewMode = this->impl_GetReducedGradientMode();
    //if (nNewMode != nOldMode)  // test the old source
    if (nNewMode == nOldMode)// test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetRedecedGradientMode() error!"),
            0);
    }

    // test GetReducedGradientMode()
    nOldMode = nNewMode = 0;
    nOldMode = this->impl_GetReducedGradientMode();
    nNewMode = nOldMode + 1;
    this->impl_SetReducedGradientMode(nNewMode);
    nNewMode = aPrintOpt.GetReducedGradientMode();
    //if (nNewMode != nOldMode) // test the old source
    if (nNewMode == nOldMode) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the GetReducedGradientMode() error!"),
            0);

    }

    //test the SetReducedGradientStepCount()
    sal_Int16 nNewStepCount;
    sal_Int16 nOldStepCount;
    nNewStepCount = nOldStepCount = 0;
    nOldStepCount = this->impl_GetReducedGradientStepCount();
    nNewStepCount = nOldStepCount + 1;
    aPrintOpt.SetReducedGradientStepCount(nNewStepCount);
    nNewStepCount = this->impl_GetReducedGradientStepCount();
   // if (nNewStepCount != nOldStepCount) // test the old source
    if (nNewStepCount == nOldStepCount) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReducedGradientStepCount() error!"),
            0);

    }

    // test the GetReduceGradientStepCount()
    nNewStepCount = nOldStepCount = 0;
    nOldStepCount = this->impl_GetReducedGradientStepCount();
    nNewStepCount = nOldStepCount + 1;
    this->impl_SetReducedGradientStepCount(nNewStepCount);
    nNewStepCount = aPrintOpt.GetReducedGradientStepCount();
   // if (nNewStepCount != nOldStepCount)  //test the old source
    if (nNewStepCount == nOldStepCount)  //test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the GetReduceGradientStepCount() error!"),
            0);
    }

    // test the SetReduceBitmaps()
    bNewValue = bOldValue = sal_False;
    bOldValue = this->impl_IsReduceBitmaps();
    bNewValue = !bOldValue;
    aPrintOpt.SetReduceBitmaps(bNewValue);
    bNewValue = this->impl_IsReduceBitmaps();
    //if (bNewValue != bOldValue) // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReduceBitmaps() error!"),
            0);
    }

    // test the IsReduceBitmaps()
    bNewValue = bOldValue = sal_False;
    bOldValue = this->impl_IsReduceBitmaps();
    bNewValue = !bOldValue;
    this->impl_SetReduceBitmaps(bNewValue);
    bNewValue = aPrintOpt.IsReduceBitmaps();
    //if (bNewValue != bOldValue)   // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsReduceBitmaps() error!"),
            0);
    }

    // test the SetReduceBitmap()
    nNewMode = nOldMode = 0;
    nOldMode = impl_GetReducedBitmapMode();
    nNewMode = nOldMode + 1;
    aPrintOpt.SetReducedBitmapMode(nNewMode);
    nNewMode = impl_GetReducedBitmapMode();
    //if (nNewMode != nOldMode)  // test the old source
    if (nNewMode == nOldMode)// test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReduceBitmap() error!"),
            0);
    }

    // test the SetReduceBitmapMode()
    nNewMode = nOldMode = 0;
    nOldMode = this->impl_GetReducedBitmapMode();
    nNewMode = nOldMode + 1;
    aPrintOpt.SetReducedBitmapMode(nNewMode);
    nNewMode = this->impl_GetReducedBitmapMode();
    //if (nNewMode != nOldMode) // test the old source
    if (nNewMode == nOldMode)  // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReduceBitmapMode() error!"),
            0);
    }

    // test the GetReduceBitmapMode()
    nNewMode = nOldMode = 0;
    nOldMode = this->impl_GetReducedBitmapMode();
    nNewMode = nOldMode + 1;
    this->impl_SetReducedBitmapMode(nNewMode);
    nNewMode = aPrintOpt.GetReducedBitmapMode();
    //if (nNewMode != nOldMode)  // test the old source
    if (nNewMode == nOldMode)// test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the GetReduceBitmapMode() error!"),
            0);

    }

    // test the SetReducedBitmapResolution()
    sal_Int16 nOldResolution ;
    sal_Int16 nNewResolution ;
    nNewResolution = nOldResolution = 0;
    nOldResolution = impl_GetReducedBitmapResolution();
    nNewResolution = nOldResolution + 1;
    aPrintOpt.SetReducedBitmapResolution(nNewResolution);
    nNewResolution = impl_GetReducedBitmapResolution();
    //if (nNewResolution != nOldResolution)  // test the old source
    if (nNewResolution == nOldResolution)// test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReducedBitmapResolution() error!"),
            0);
    }

    // test the GetReduceBitmapResolution()
    nNewResolution = nOldResolution = 0;
    nOldResolution = impl_GetReducedBitmapResolution();
    nNewResolution = nOldResolution + 1;
    impl_SetReducedBitmapResolution(nNewResolution);
    nNewResolution = impl_GetReducedBitmapResolution();
    //if (nNewResolution != nOldResolution)   // test the old source
    if (nNewResolution == nOldResolution) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the GetReduceBitmapResolution() error!"),
            0);
    }

    // test SetReducedBitmapIncludesTransparency()
    bNewValue = bOldValue = sal_False;
    bOldValue = impl_IsReducedBitmapIncludesTransparency();
    bNewValue = !bOldValue;
    aPrintOpt.SetReducedBitmapIncludesTransparency(bNewValue);
    bNewValue = impl_IsReducedBitmapIncludesTransparency();
    //if (bNewValue != bOldValue) // test the new source
    if (bNewValue == bOldValue) // test the old source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetReducedBitmapIncludesTransparency() error!"),
            0);
    }

    // test the IsReducedBitmapIncludesTransparency()
    bNewValue = bOldValue = sal_False;
    bOldValue = impl_IsReducedBitmapIncludesTransparency();
    bNewValue = !bOldValue;
    impl_SetReducedBitmapIncludesTransparency(bNewValue);
    bNewValue = aPrintOpt.IsReducedBitmapIncludesTransparency();
    //if (bNewValue != bOldValue)   // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsReducedBitmapIncludesTransparency() error!"),
            0);
    }

    // test the SetConvertToGreyscales()
    bNewValue = bOldValue = sal_False;
    bOldValue = this->impl_IsConvertToGreyscales();
    bNewValue = !bOldValue;
    aPrintOpt.SetConvertToGreyscales(bNewValue);
    bNewValue = this->impl_IsConvertToGreyscales();
    //if (bNewValue != bOldValue) // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the SetConvertToGreyscales() error!"),
            0);
    }

    // test the IsConvertToGreyscales()
    bNewValue = bOldValue = sal_False;
    bOldValue = this->impl_IsConvertToGreyscales();
    bNewValue = !bOldValue;
    impl_SetConvertToGreyscales(bNewValue);
    bNewValue = aPrintOpt.IsConvertToGreyscales();
    //if (bNewValue != bOldValue) // test the old source
    if (bNewValue == bOldValue) // test the new source
    {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
            "null com.sun.star.configuration."
            "the IsConvertToGreyscales() error!"),
            0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
