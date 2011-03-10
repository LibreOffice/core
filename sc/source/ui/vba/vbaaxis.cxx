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

#include "vbaaxis.hxx"
#include <ooo/vba/excel/XlAxisCrosses.hpp>
#include <ooo/vba/excel/XlAxisType.hpp>
#include <ooo/vba/excel/XlScaleType.hpp>
#include "vbaaxistitle.hxx"
#include "vbachart.hxx"
using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlAxisCrosses;
using namespace ::ooo::vba::excel::XlAxisType;
using namespace ::ooo::vba::excel::XlScaleType;

const rtl::OUString ORIGIN( RTL_CONSTASCII_USTRINGPARAM("Origin") );
const rtl::OUString AUTOORIGIN( RTL_CONSTASCII_USTRINGPARAM("AutoOrigin") );
const rtl::OUString VBA_MIN( RTL_CONSTASCII_USTRINGPARAM("Max") );
const rtl::OUString VBA_MAX( RTL_CONSTASCII_USTRINGPARAM("Min") );
ScVbaChart*
ScVbaAxis::getChartPtr() throw( uno::RuntimeException )
{
    ScVbaChart* pChart = static_cast< ScVbaChart* >( moChartParent.get() );
    if ( !pChart )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Can't access parent chart impl")), uno::Reference< uno::XInterface >() );
    return pChart;
}

sal_Bool
ScVbaAxis::isValueAxis() throw( script::BasicErrorException )
{
    if ( getType() == xlCategory )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return sal_True;
}

ScVbaAxis::ScVbaAxis( const uno::Reference< XHelperInterface >& xParent,const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup  ) : ScVbaAxis_BASE( xParent, xContext ), mxPropertySet( _xPropertySet ), mnType( _nType ), mnGroup( _nGroup ), bCrossesAreCustomized( false )
{
    oShapeHelper.reset( new ShapeHelper( uno::Reference< drawing::XShape >( mxPropertySet, uno::UNO_QUERY ) ) );
    moChartParent.set( xParent, uno::UNO_QUERY_THROW  );
    setType(_nType);
    setCrosses(xlAxisCrossesAutomatic);
}

void SAL_CALL
ScVbaAxis::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< lang::XComponent > xComponent( mxPropertySet, uno::UNO_QUERY_THROW );
    xComponent->dispose();
}

 uno::Reference< ::ooo::vba::excel::XAxisTitle > SAL_CALL
ScVbaAxis::getAxisTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
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
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
    return xAxisTitle;

}

void SAL_CALL
ScVbaAxis::setDisplayUnit( ::sal_Int32 /*DisplayUnit*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
}

::sal_Int32 SAL_CALL
ScVbaAxis::getDisplayUnit(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
    return -1;
}

void SAL_CALL
ScVbaAxis::setCrosses( ::sal_Int32 _nCrosses ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        double fNum = 0.0;
        switch (_nCrosses)
        {
            case  xlAxisCrossesAutomatic:       //Microsoft Excel sets the axis crossing point.
                mxPropertySet->setPropertyValue(AUTOORIGIN, uno::makeAny( sal_True ) );
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
                bCrossesAreCustomized = sal_True;
                break;
        }
        mxPropertySet->setPropertyValue(AUTOORIGIN, uno::makeAny(false) );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}
::sal_Int32 SAL_CALL
ScVbaAxis::getCrosses(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Int32 nCrosses = xlAxisCrossesCustom;
    try
    {
        sal_Bool bisAutoOrigin = false;
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
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return nCrosses;
}

 void SAL_CALL
ScVbaAxis::setCrossesAt( double _fCrossesAt ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        setMaximumScaleIsAuto( false );
        setMinimumScaleIsAuto( false );
        mxPropertySet->setPropertyValue(ORIGIN, uno::makeAny(_fCrossesAt));
    }
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
}

 double SAL_CALL
ScVbaAxis::getCrossesAt(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    double fCrosses = 0.0;
    try
    {
        mxPropertySet->getPropertyValue(ORIGIN) >>= fCrosses;
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return fCrosses;
}

void SAL_CALL
ScVbaAxis::setType( ::sal_Int32 _nType ) throw (script::BasicErrorException, uno::RuntimeException)
{
    mnType = _nType;
}

::sal_Int32 SAL_CALL
ScVbaAxis::getType(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return mnType;
}

void SAL_CALL
ScVbaAxis::setHasTitle( ::sal_Bool _bHasTitle ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        ScVbaChart* pChart = getChartPtr();
        sal_Int32 nType = getType();
        switch(nType)
        {
            case xlCategory:
                pChart->mxDiagramPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasXAxisTitle")), uno::makeAny(_bHasTitle));
                break;
            case xlSeriesAxis:
                pChart->mxDiagramPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasZAxisTitle")), uno::makeAny(_bHasTitle));
                break;
            default: // xlValue:
                pChart->mxDiagramPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasYAxisTitle")), uno::makeAny(_bHasTitle));
        }

    }
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
}

 ::sal_Bool SAL_CALL
ScVbaAxis::getHasTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bHasTitle = false;
    try
    {
        ScVbaChart* pChart = getChartPtr();
        int nType = getType();
        switch(nType)
        {
            case xlCategory:
                pChart->mxDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasXAxisTitle")) ) >>= bHasTitle;
                break;
            case xlSeriesAxis:
                pChart->mxDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasZAxisTitle")) ) >>= bHasTitle;
                break;
            default: // xlValue:
                pChart->mxDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasYAxisTitle")) ) >>= bHasTitle;
        }
    }
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
    return bHasTitle;
}

void SAL_CALL
ScVbaAxis::setMinorUnit( double _fMinorUnit ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StepHelp") ), uno::makeAny(_fMinorUnit));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

double SAL_CALL
ScVbaAxis::getMinorUnit(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    double fMinor = 1.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StepHelp"))) >>= fMinor;
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return fMinor;
}

void SAL_CALL
ScVbaAxis::setMinorUnitIsAuto( ::sal_Bool _bMinorUnitIsAuto ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoStepHelp" ) ), uno::makeAny(_bMinorUnitIsAuto));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

 ::sal_Bool SAL_CALL
ScVbaAxis::getMinorUnitIsAuto(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoStepHelp")) ) >>= bIsAuto;
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setReversePlotOrder( ::sal_Bool /*ReversePlotOrder*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
}

::sal_Bool SAL_CALL
ScVbaAxis::getReversePlotOrder(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
    return false;
}

void SAL_CALL
ScVbaAxis::setMajorUnit( double _fMajorUnit ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StepMain")), uno::makeAny(_fMajorUnit));
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

double SAL_CALL
ScVbaAxis::getMajorUnit(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    double fMax = 1.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StepMain"))) >>= fMax;
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return fMax;
}

void SAL_CALL
ScVbaAxis::setMajorUnitIsAuto( ::sal_Bool _bMajorUnitIsAuto ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoStepMain" ) ), uno::makeAny( _bMajorUnitIsAuto ));
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

::sal_Bool SAL_CALL
ScVbaAxis::getMajorUnitIsAuto(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoStepMain"))) >>= bIsAuto;
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setMaximumScale( double _fMaximumScale ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if ( isValueAxis() )
        {
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Max" ) ), uno::makeAny(_fMaximumScale));
        }
    }
    catch ( uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

double SAL_CALL
ScVbaAxis::getMaximumScale(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    double fMax = 1.0;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Max" ))) >>= fMax;
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return fMax;

}

void SAL_CALL
ScVbaAxis::setMaximumScaleIsAuto( ::sal_Bool _bMaximumScaleIsAuto ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if ( isValueAxis() )
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoMax" ) ), uno::makeAny( _bMaximumScaleIsAuto ));

    }
    catch ( uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}


::sal_Bool SAL_CALL
ScVbaAxis::getMaximumScaleIsAuto(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bIsAuto = false;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoMax" )) ) >>= bIsAuto;
    }
    catch ( uno::Exception& )
    {
        DebugHelper::exception( SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return bIsAuto;
}

void SAL_CALL
ScVbaAxis::setMinimumScale( double _fMinimumScale ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Min") ), uno::makeAny( _fMinimumScale )  );
    }
    catch ( uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

double SAL_CALL
ScVbaAxis::getMinimumScale(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    double fMin = 0.0;
    try
    {
        if (isValueAxis())
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Min") )) >>= fMin;
    }
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }
    return fMin;
}

void SAL_CALL
ScVbaAxis::setMinimumScaleIsAuto( ::sal_Bool _bMinimumScaleIsAuto ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutoMin") ), uno::makeAny(_bMinimumScaleIsAuto));
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

::sal_Bool SAL_CALL
ScVbaAxis::getMinimumScaleIsAuto(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bIsAuto = false;
    try
    {
        if (isValueAxis())
        {
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutoMin")) ) >>= bIsAuto;
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return bIsAuto;
}

::sal_Int32 SAL_CALL
ScVbaAxis::getAxisGroup(  ) throw (uno::RuntimeException)
{
    return mnGroup;
}

void SAL_CALL
ScVbaAxis::setScaleType( ::sal_Int32 _nScaleType ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        if (isValueAxis())
        {
            switch (_nScaleType)
            {
                case xlScaleLinear:
                    mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Logarithmic" ) ), uno::makeAny( false ) );
                    break;
                case xlScaleLogarithmic:
                    mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Logarithmic" ) ), uno::makeAny( sal_True ) );
                    break;
                default:
                    // According to MS the paramenter is ignored and no Error is thrown
                    break;
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

::sal_Int32 SAL_CALL
ScVbaAxis::getScaleType(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Int32 nScaleType = xlScaleLinear;
    try
    {
        if (isValueAxis())
        {
            sal_Bool bisLogarithmic = false;
            mxPropertySet->getPropertyValue( rtl::OUString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Logarithmic"))) ) >>= bisLogarithmic;
            if (bisLogarithmic)
                nScaleType = xlScaleLogarithmic;
            else
                nScaleType = xlScaleLinear;
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return nScaleType;
}

double SAL_CALL
ScVbaAxis::getHeight(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return oShapeHelper->getHeight();
}

void SAL_CALL ScVbaAxis::setHeight( double height ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    oShapeHelper->setHeight( height );
}
double SAL_CALL ScVbaAxis::getWidth(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return oShapeHelper->getWidth( );
}
void SAL_CALL ScVbaAxis::setWidth( double width ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    oShapeHelper->setWidth( width );
}
double SAL_CALL ScVbaAxis::getTop(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return oShapeHelper->getTop( );
}
void SAL_CALL ScVbaAxis::setTop( double top ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    oShapeHelper->setTop( top );
}
double SAL_CALL ScVbaAxis::getLeft(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return oShapeHelper->getLeft( );
}
void SAL_CALL ScVbaAxis::setLeft( double left ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    oShapeHelper->setLeft( left );
}

rtl::OUString&
ScVbaAxis::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAxis") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaAxis::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Axis" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
