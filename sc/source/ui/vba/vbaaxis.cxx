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

#include "vbaaxis.hxx"
#include <ooo/vba/excel/XlAxisCrosses.hpp>
#include <ooo/vba/excel/XlAxisType.hpp>
#include <ooo/vba/excel/XlScaleType.hpp>
#include <utility>
#include "vbaaxistitle.hxx"
#include "vbachart.hxx"
using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlAxisCrosses;
using namespace ::ooo::vba::excel::XlAxisType;
using namespace ::ooo::vba::excel::XlScaleType;

constexpr OUString ORIGIN(u"Origin"_ustr);
constexpr OUString AUTOORIGIN(u"AutoOrigin"_ustr);
constexpr OUString VBA_MIN(u"Max"_ustr);
constexpr OUStringLiteral VBA_MAX(u"Min");
ScVbaChart*
ScVbaAxis::getChartPtr()
{
    ScVbaChart* pChart = moChartParent.get();
    if ( !pChart )
        throw uno::RuntimeException(u"Can't access parent chart impl"_ustr );
    return pChart;
}

bool
ScVbaAxis::isValueAxis()
{
    if ( getType() == xlCategory )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return true;
}

ScVbaAxis::ScVbaAxis( const rtl::Reference< ScVbaChart >& xParent,
                      const uno::Reference< uno::XComponentContext > & xContext,
                      uno::Reference< beans::XPropertySet >  _xPropertySet,
                      sal_Int32 _nType, sal_Int32 _nGroup )
  : ScVbaAxis_BASE( xParent, xContext ),
    moChartParent(xParent),
    mxPropertySet(std::move( _xPropertySet )),
    mnType( _nType ), mnGroup( _nGroup ),
    maShapeHelper( uno::Reference< drawing::XShape >( mxPropertySet, uno::UNO_QUERY ) ),
    bCrossesAreCustomized( false )
{
    setType(_nType);
    setCrosses(xlAxisCrossesAutomatic);
}

void SAL_CALL
ScVbaAxis::Delete(  )
{
    uno::Reference< lang::XComponent > xComponent( mxPropertySet, uno::UNO_QUERY_THROW );
    xComponent->dispose();
}

 uno::Reference< ::ooo::vba::excel::XAxisTitle > SAL_CALL
ScVbaAxis::getAxisTitle(  )
{
    uno::Reference< excel::XAxisTitle > xAxisTitle;
    try
    {
        ScVbaChart* pChart = getChartPtr();

        if (getHasTitle() )
        {
            int nType = getType();
            switch(nType)
            {
                case xlCategory:
                    xAxisTitle =  new ScVbaAxisTitle(this, mxContext, pChart->xAxisXSupplier->getXAxisTitle());
                    break;
                case xlSeriesAxis:
                    xAxisTitle = new ScVbaAxisTitle(this, mxContext, pChart->xAxisZSupplier->getZAxisTitle());
                    break;
                default: // xlValue:
                    xAxisTitle = new ScVbaAxisTitle(this, mxContext, pChart->xAxisYSupplier->getYAxisTitle());
                    break;
            }
        }
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
    return xAxisTitle;

}

void SAL_CALL
ScVbaAxis::setDisplayUnit( ::sal_Int32 /*DisplayUnit*/ )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
}

::sal_Int32 SAL_CALL
ScVbaAxis::getDisplayUnit(  )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
    return -1;
}

void SAL_CALL
ScVbaAxis::setCrosses( ::sal_Int32 _nCrosses )
{
    try
    {
        double fNum = 0.0;
        switch (_nCrosses)
        {
            case  xlAxisCrossesAutomatic:       //Microsoft Excel sets the axis crossing point.
                mxPropertySet->setPropertyValue(AUTOORIGIN, uno::Any( true ) );
                bCrossesAreCustomized = false;
                return;
            case xlAxisCrossesMinimum:                     // The axis crosses at the minimum value.
                mxPropertySet->getPropertyValue(VBA_MIN) >>= fNum;
                setCrossesAt( fNum );
                bCrossesAreCustomized = false;
                break;
            case xlAxisCrossesMaximum:                     // The axis crosses at the maximum value.
                mxPropertySet->getPropertyValue(VBA_MAX) >>= fNum;
                setCrossesAt(fNum);
                bCrossesAreCustomized = false;
                break;
            default: //xlAxisCrossesCustom
                bCrossesAreCustomized = true;
                break;
        }
        mxPropertySet->setPropertyValue(AUTOORIGIN, uno::Any(false) );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}
::sal_Int32 SAL_CALL
ScVbaAxis::getCrosses(  )
{
    sal_Int32 nCrosses = xlAxisCrossesCustom;
    try
    {
        bool bisAutoOrigin = false;
        mxPropertySet->getPropertyValue(AUTOORIGIN) >>= bisAutoOrigin;
        if (bisAutoOrigin)
            nCrosses = xlAxisCrossesAutomatic;
        else
        {
            if (bCrossesAreCustomized)
                nCrosses = xlAxisCrossesCustom;
            else
            {
                double forigin = 0.0;
                mxPropertySet->getPropertyValue(ORIGIN) >>= forigin;
                double fmin = 0.0;
                mxPropertySet->getPropertyValue(VBA_MIN) >>= fmin;
                if (forigin == fmin)
                    nCrosses = xlAxisCrossesMinimum;
                else
                    nCrosses = xlAxisCrossesMaximum;
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
    return nCrosses;
}

 void SAL_CALL
ScVbaAxis::setCrossesAt( double _fCrossesAt )
{
    try
    {
        setMaximumScaleIsAuto( false );
        setMinimumScaleIsAuto( false );
        mxPropertySet->setPropertyValue(ORIGIN, uno::Any(_fCrossesAt));
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
}

 double SAL_CALL
ScVbaAxis::getCrossesAt(  )
{
    double fCrosses = 0.0;
    try
    {
        mxPropertySet->getPropertyValue(ORIGIN) >>= fCrosses;
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return fCrosses;
}

void SAL_CALL
ScVbaAxis::setType( ::sal_Int32 _nType )
{
    mnType = _nType;
}

::sal_Int32 SAL_CALL
ScVbaAxis::getType(  )
{
    return mnType;
}

void SAL_CALL
ScVbaAxis::setHasTitle( sal_Bool _bHasTitle )
{
    try
    {
        ScVbaChart* pChart = getChartPtr();
        sal_Int32 nType = getType();
        switch(nType)
        {
            case xlCategory:
                pChart->mxDiagramPropertySet->setPropertyValue(u"HasXAxisTitle"_ustr, uno::Any(_bHasTitle));
                break;
            case xlSeriesAxis:
                pChart->mxDiagramPropertySet->setPropertyValue(u"HasZAxisTitle"_ustr, uno::Any(_bHasTitle));
                break;
            default: // xlValue:
                pChart->mxDiagramPropertySet->setPropertyValue(u"HasYAxisTitle"_ustr, uno::Any(_bHasTitle));
        }

    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
}

 sal_Bool SAL_CALL
ScVbaAxis::getHasTitle(  )
{
    bool bHasTitle = false;
    try
    {
        ScVbaChart* pChart = getChartPtr();
        int nType = getType();
        switch(nType)
        {
            case xlCategory:
                pChart->mxDiagramPropertySet->getPropertyValue(u"HasXAxisTitle"_ustr) >>= bHasTitle;
                break;
            case xlSeriesAxis:
                pChart->mxDiagramPropertySet->getPropertyValue(u"HasZAxisTitle"_ustr) >>= bHasTitle;
                break;
            default: // xlValue:
                pChart->mxDiagramPropertySet->getPropertyValue(u"HasYAxisTitle"_ustr) >>= bHasTitle;
        }
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
    return bHasTitle;
}

void SAL_CALL
ScVbaAxis::setMinorUnit( double _fMinorUnit )
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue(u"StepHelp"_ustr, uno::Any(_fMinorUnit));
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

double SAL_CALL
ScVbaAxis::getMinorUnit(  )
{
    double fMinor = 1.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue(u"StepHelp"_ustr) >>= fMinor;
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return fMinor;
}

void SAL_CALL
ScVbaAxis::setMinorUnitIsAuto( sal_Bool _bMinorUnitIsAuto )
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue(u"AutoStepHelp"_ustr, uno::Any(_bMinorUnitIsAuto));
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
}

 sal_Bool SAL_CALL
ScVbaAxis::getMinorUnitIsAuto(  )
{
    bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue(u"AutoStepHelp"_ustr) >>= bIsAuto;
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setReversePlotOrder( sal_Bool /*ReversePlotOrder*/ )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
}

sal_Bool SAL_CALL
ScVbaAxis::getReversePlotOrder(  )
{
    DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, {});
    return false;
}

void SAL_CALL
ScVbaAxis::setMajorUnit( double _fMajorUnit )
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue(u"StepMain"_ustr, uno::Any(_fMajorUnit));
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

double SAL_CALL
ScVbaAxis::getMajorUnit(  )
{
    double fMax = 1.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue(u"StepMain"_ustr) >>= fMax;
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
    return fMax;
}

void SAL_CALL
ScVbaAxis::setMajorUnitIsAuto( sal_Bool _bMajorUnitIsAuto )
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue(u"AutoStepMain"_ustr, uno::Any( _bMajorUnitIsAuto ));
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

sal_Bool SAL_CALL
ScVbaAxis::getMajorUnitIsAuto(  )
{
    bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue(u"AutoStepMain"_ustr) >>= bIsAuto;
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setMaximumScale( double _fMaximumScale )
{
    try
    {
        if ( isValueAxis() )
        {
            mxPropertySet->setPropertyValue(u"Max"_ustr, uno::Any(_fMaximumScale));
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

double SAL_CALL
ScVbaAxis::getMaximumScale(  )
{
    double fMax = 1.0;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue(u"Max"_ustr) >>= fMax;
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return fMax;

}

void SAL_CALL
ScVbaAxis::setMaximumScaleIsAuto( sal_Bool _bMaximumScaleIsAuto )
{
    try
    {
        if ( isValueAxis() )
            mxPropertySet->setPropertyValue(u"AutoMax"_ustr, uno::Any( _bMaximumScaleIsAuto ));

    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

sal_Bool SAL_CALL
ScVbaAxis::getMaximumScaleIsAuto(  )
{
    bool bIsAuto = false;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue(u"AutoMax"_ustr) >>= bIsAuto;
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception( ERRCODE_BASIC_METHOD_FAILED, {} );
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setMinimumScale( double _fMinimumScale )
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue(u"Min"_ustr, uno::Any( _fMinimumScale )  );
    }
    catch ( uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    }
}

double SAL_CALL
ScVbaAxis::getMinimumScale(  )
{
    double fMin = 0.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue(u"Min"_ustr) >>= fMin;
    }
    catch (const uno::Exception& e)
    {
        DebugHelper::basicexception(e);
    }
    return fMin;
}

void SAL_CALL
ScVbaAxis::setMinimumScaleIsAuto( sal_Bool _bMinimumScaleIsAuto )
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue(u"AutoMin"_ustr, uno::Any(_bMinimumScaleIsAuto));
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

sal_Bool SAL_CALL
ScVbaAxis::getMinimumScaleIsAuto(  )
{
    bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue(u"AutoMin"_ustr) >>= bIsAuto;
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return bIsAuto;
}

::sal_Int32 SAL_CALL
ScVbaAxis::getAxisGroup(  )
{
    return mnGroup;
}

void SAL_CALL
ScVbaAxis::setScaleType( ::sal_Int32 _nScaleType )
{
    try
    {
        if (isValueAxis())
        {
            switch (_nScaleType)
            {
                case xlScaleLinear:
                    mxPropertySet->setPropertyValue(u"Logarithmic"_ustr, uno::Any( false ) );
                    break;
                case xlScaleLogarithmic:
                    mxPropertySet->setPropertyValue(u"Logarithmic"_ustr, uno::Any( true ) );
                    break;
                default:
                    // According to MS the parameter is ignored and no Error is thrown
                    break;
            }
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

::sal_Int32 SAL_CALL
ScVbaAxis::getScaleType(  )
{
    sal_Int32 nScaleType = xlScaleLinear;
    try
    {
        if (isValueAxis())
        {
            bool bisLogarithmic = false;
            mxPropertySet->getPropertyValue( u"Logarithmic"_ustr ) >>= bisLogarithmic;
            if (bisLogarithmic)
                nScaleType = xlScaleLogarithmic;
            else
                nScaleType = xlScaleLinear;
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return nScaleType;
}

double SAL_CALL
ScVbaAxis::getHeight(  )
{
    return maShapeHelper.getHeight();
}

void SAL_CALL ScVbaAxis::setHeight( double height )
{
    maShapeHelper.setHeight( height );
}
double SAL_CALL ScVbaAxis::getWidth(  )
{
    return maShapeHelper.getWidth( );
}
void SAL_CALL ScVbaAxis::setWidth( double width )
{
    maShapeHelper.setWidth( width );
}
double SAL_CALL ScVbaAxis::getTop(  )
{
    return maShapeHelper.getTop( );
}
void SAL_CALL ScVbaAxis::setTop( double top )
{
    maShapeHelper.setTop( top );
}
double SAL_CALL ScVbaAxis::getLeft(  )
{
    return maShapeHelper.getLeft( );
}
void SAL_CALL ScVbaAxis::setLeft( double left )
{
    maShapeHelper.setLeft( left );
}

OUString
ScVbaAxis::getServiceImplName()
{
    return u"ScVbaAxis"_ustr;
}

uno::Sequence< OUString >
ScVbaAxis::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.Axis"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
