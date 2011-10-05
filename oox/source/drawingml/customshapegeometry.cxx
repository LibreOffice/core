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

#include "oox/drawingml/customshapegeometry.hxx"

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <comphelper/stl_types.hxx>
#include <boost/unordered_map.hpp>
#include "oox/helper/helper.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

enum FormularCommand
{
    FC_MULDIV = 0,
    FC_PLUSMINUS,
    FC_PLUSDIV,
    FC_IFELSE,
    FC_IFELSE1,
    FC_ABS,
    FC_AT2,
    FC_CAT2,
    FC_COS,
    FC_MAX,
    FC_MIN,
    FC_MOD,
    FC_PIN,
    FC_SAT2,
    FC_SIN,
    FC_SQRT,
    FC_TAN,
    FC_VAL,
    FC_LAST
};
struct FormularCommandNameTable
{
    const char*     pS;
    FormularCommand pE;
};
static FormularCommandNameTable pFormularCommandNameTable[] =
{
    { "*/",     FC_MULDIV },
    { "+-",     FC_PLUSMINUS },
    { "+/",     FC_PLUSDIV },
    { "ifelse", FC_IFELSE },
    { "?:",     FC_IFELSE1 },
    { "abs",    FC_ABS },
    { "at2",    FC_AT2 },
    { "cat2",   FC_CAT2 },
    { "cos",    FC_COS },
    { "max",    FC_MAX },
    { "min",    FC_MIN },
    { "mod",    FC_MOD },
    { "pin",    FC_PIN },
    { "sat2",   FC_SAT2 },
    { "sin",    FC_SIN },
    { "sqrt",   FC_SQRT },
    { "tan",    FC_TAN },
    { "val",    FC_VAL }

};
typedef boost::unordered_map< rtl::OUString, FormularCommand, rtl::OUStringHash, comphelper::UStringEqual > FormulaCommandHMap;

static const FormulaCommandHMap* pCommandHashMap;

//
rtl::OUString GetFormulaParameter( const EnhancedCustomShapeParameter& rParameter )
{
    rtl::OUString aRet;
    switch( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::NORMAL :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue = 0.0;
                if ( rParameter.Value >>= fValue )
                    aRet = rtl::OUString::valueOf( fValue );
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                    aRet = rtl::OUString::valueOf( nValue );
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_LONG )
            {
                sal_Int32 nFormulaIndex;
                if ( rParameter.Value >>= nFormulaIndex )
                {
                    aRet = CREATE_OUSTRING( "?" )
                        + rtl::OUString::valueOf( nFormulaIndex )
                            + CREATE_OUSTRING( " " );
                }
            }
            else
            {
                // ups... we should have an index here and not the formula name
            }
        }
        break;
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_LONG )
            {
                sal_Int32 nAdjustmentIndex;
                if ( rParameter.Value >>= nAdjustmentIndex )
                {
                    aRet = CREATE_OUSTRING( "$" )
                        + rtl::OUString::valueOf( nAdjustmentIndex )
                            + CREATE_OUSTRING( " " );
                }
            }
            else
            {
                // ups... we should have an index here and not the formula name
            }
        }
        break;
        case EnhancedCustomShapeParameterType::LEFT :
        {
            const rtl::OUString sLeft( CREATE_OUSTRING( "left" ) );
            aRet = sLeft;
        }
        break;
        case EnhancedCustomShapeParameterType::TOP :
        {
            const rtl::OUString sTop( CREATE_OUSTRING( "top" ) );
            aRet = sTop;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            const rtl::OUString sRight( CREATE_OUSTRING( "right" ) );
            aRet = sRight;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            const rtl::OUString sBottom( CREATE_OUSTRING( "bottom" ) );
            aRet = sBottom;
        }
        break;
        case EnhancedCustomShapeParameterType::XSTRETCH :
        {
            const rtl::OUString sXStretch( CREATE_OUSTRING( "xstretch" ) );
            aRet = sXStretch;
        }
        break;
        case EnhancedCustomShapeParameterType::YSTRETCH :
        {
            const rtl::OUString sYStretch( CREATE_OUSTRING( "ystretch" ) );
            aRet = sYStretch;
        }
        break;
        case EnhancedCustomShapeParameterType::HASSTROKE :
        {
            const rtl::OUString sHasStroke( CREATE_OUSTRING( "hasstroke" ) );
            aRet = sHasStroke;
        }
        break;
        case EnhancedCustomShapeParameterType::HASFILL :
        {
            const rtl::OUString sHasFill( CREATE_OUSTRING( "hasfill" ) );
            aRet = sHasFill;
        }
        break;
        case EnhancedCustomShapeParameterType::WIDTH :
        {
            const rtl::OUString sWidth( CREATE_OUSTRING( "width" ) );
            aRet = sWidth;
        }
        break;
        case EnhancedCustomShapeParameterType::HEIGHT :
        {
            const rtl::OUString sHeight( CREATE_OUSTRING( "height" ) );
            aRet = sHeight;
        }
        break;
        case EnhancedCustomShapeParameterType::LOGWIDTH :
        {
            const rtl::OUString sLogWidth( CREATE_OUSTRING( "logwidth" ) );
            aRet = sLogWidth;
        }
        break;
        case EnhancedCustomShapeParameterType::LOGHEIGHT :
        {
            const rtl::OUString sLogHeight( CREATE_OUSTRING( "logheight" ) );
            aRet = sLogHeight;
        }
        break;
    }
    return aRet;
}

// ---------------------------------------------------------------------

static EnhancedCustomShapeParameter GetAdjCoordinate( CustomShapeProperties& rCustomShapeProperties, const::rtl::OUString& rValue, sal_Bool bNoSymbols = sal_True )
{
    com::sun::star::drawing::EnhancedCustomShapeParameter aRet;
    if ( rValue.getLength() )
    {
        sal_Bool    bConstant = sal_True;
        sal_Int32   nConstant = -1;
        sal_Int32   nIntVal = 0;

        // first check if its a constant value
        switch( AttributeConversion::decodeToken( rValue ) )
        {
            case XML_3cd4 : nConstant = 270 * 60000; break;
            case XML_3cd8 : nConstant = 135 * 60000; break;
            case XML_5cd8 : nConstant = 225 * 60000; break;
            case XML_7cd8 : nConstant = 315 * 60000; break;
            case XML_cd2  : nConstant = 180 * 60000; break;
            case XML_cd3  : nConstant = 120 * 60000; break;
            case XML_cd4  : nConstant =  90 * 60000; break;
            case XML_cd8  : nConstant =  45 * 60000; break;

            case XML_b :    // variable height of the shape defined in spPr
            case XML_h :
            {
                if ( bNoSymbols )
                {
                    CustomShapeGuide aGuide;
                    aGuide.maName = rValue;
                    aGuide.maFormula = CREATE_OUSTRING( "height" );

                    aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                    aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                }
                else
                    aRet.Type = EnhancedCustomShapeParameterType::HEIGHT;   // TODO: HEIGHT needs to be implemented
            }
            break;


            case XML_hd10 :   // !!PASSTHROUGH INTENDED
                nIntVal += 2; // */ h 1.0 10.0
            case XML_hd8 :    // */ h 1.0 8.0
                nIntVal += 2;
            case XML_hd6 :    // */ h 1.0 6.0
                nIntVal++;
            case XML_hd5 :    // */ h 1.0 5.0
                nIntVal++;
            case XML_hd4 :    // */ h 1.0 4.0
                nIntVal++;
            case XML_hd3 :    // */ h 1.0 3.0
                nIntVal++;
            case XML_hd2 :    // */ h 1.0 2.0
            case XML_vc :     // */ h 1.0 2.0
            {
                nIntVal += 2;

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = CREATE_OUSTRING( "height/" ) + rtl::OUString::valueOf( nIntVal );

                aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;

            case XML_t :
            case XML_l :
            {
                nConstant = 0;
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
            break;

            case XML_ls :   // longest side: max w h
            {
                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = CREATE_OUSTRING( "max(width,height)" );

                aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;
            case XML_ss :   // shortest side: min w h
            {
                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = CREATE_OUSTRING( "min(width,height)" );

                aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;
            case XML_ssd32 : // */ ss 1.0 32.0
                nIntVal += 16;
            case XML_ssd16 : // */ ss 1.0 16.0
                nIntVal += 8;
            case XML_ssd8 :  // */ ss 1.0 8.0
                nIntVal += 2;
            case XML_ssd6 :  // */ ss 1.0 6.0
                nIntVal += 2;
            case XML_ssd4 :  // */ ss 1.0 4.0
                nIntVal += 2;
            case XML_ssd2 :  // */ ss 1.0 2.0
            {
                nIntVal += '2';

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = CREATE_OUSTRING( "min(width,height)/" ) + rtl::OUString::valueOf( nIntVal );

                aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;

            case XML_r :    // variable width of the shape defined in spPr
            case XML_w :
            {
                if ( bNoSymbols )
                {
                    CustomShapeGuide aGuide;
                    aGuide.maName = rValue;
                    aGuide.maFormula = CREATE_OUSTRING( "width" );

                    aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                    aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                }
                else
                    aRet.Type = EnhancedCustomShapeParameterType::WIDTH;
            }
            break;

            case XML_wd32 : // */ w 1.0 32.0
                nIntVal += 20;
            case XML_wd12 : // */ w 1.0 12.0
                nIntVal += 2;
            case XML_wd10 : // */ w 1.0 10.0
                nIntVal += 2;
            case XML_wd8 :  // */ w 1.0 8.0
                nIntVal += 2;
            case XML_wd6 :  // */ w 1.0 6.0
                nIntVal++;
            case XML_wd5 :  // */ w 1.0 5.0
                nIntVal++;
            case XML_wd4 :  // */ w 1.0 4.0
                nIntVal++;
            case XML_wd3 :  // */ w 1.0 3.0
                nIntVal++;
            case XML_hc :   // */ w 1.0 2.0
            case XML_wd2 :  // */ w 1.0 2.0
            {
                nIntVal += 2;

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = CREATE_OUSTRING( "width/" ) + rtl::OUString::valueOf( nIntVal );

                aRet.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), aGuide ) );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;

            default:
                bConstant = sal_False;
            break;
        }
        if ( bConstant )
        {
            if (nConstant != -1) {
                aRet.Value = Any( nConstant );
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
        }
        else
        {
            sal_Unicode n = rValue[ 0 ];
            if ( ( n == '+' ) || ( n == '-' ) )
            {
                if ( rValue.getLength() > 0 )
                    n = rValue[ 1 ];
            }
            if ( ( n >= '0' ) && ( n <= '9' ) )
            {   // seems to be a ST_Coordinate
                aRet.Value = Any( (sal_Int32)(rValue.toInt32() ) );
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
            else
            {
                sal_Int32 nGuideIndex = CustomShapeProperties::GetCustomShapeGuideValue( rCustomShapeProperties.getAdjustmentGuideList(), rValue );
                if ( nGuideIndex >= 0 )
                {
                    aRet.Value = Any( nGuideIndex );
                    aRet.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
                }
                else
                {
                    nGuideIndex = CustomShapeProperties::GetCustomShapeGuideValue( rCustomShapeProperties.getGuideList(), rValue );
                    if ( nGuideIndex >= 0 )
                    {
                        aRet.Value = Any( nGuideIndex );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                    }
                    else
                    {
                        OSL_TRACE("error: unhandled value '%s'", OUStringToOString( rValue, RTL_TEXTENCODING_ASCII_US ).getStr());
                        aRet.Value = Any( rValue );
                    }
                }
            }
        }
    }
    return aRet;
}

// ---------------------------------------------------------------------
// CT_GeomGuideList
class GeomGuideListContext : public ContextHandler
{
public:
    GeomGuideListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< CustomShapeGuide >& rGuideList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    std::vector< CustomShapeGuide >&    mrGuideList;
    CustomShapeProperties&              mrCustomShapeProperties;
};

GeomGuideListContext::GeomGuideListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< CustomShapeGuide >& rGuideList )
: ContextHandler( rParent )
, mrGuideList( rGuideList )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

static rtl::OUString convertToOOEquation( CustomShapeProperties& rCustomShapeProperties, const rtl::OUString& rSource )
{
    if ( !pCommandHashMap )
    {
        FormulaCommandHMap* pHM = new FormulaCommandHMap();
        for( sal_Int32 i = 0; i < FC_LAST; i++ )
            (*pHM)[ OUString::createFromAscii( pFormularCommandNameTable[ i ].pS ) ] =  pFormularCommandNameTable[ i ].pE;
        pCommandHashMap = pHM;
    }

    std::vector< rtl::OUString > aTokens;
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OUString aToken( rSource.getToken( 0, ' ', nIndex ) );
        if ( aToken.getLength() )
            aTokens.push_back( aToken );
    }
    while ( nIndex >= 0 );

    rtl::OUString aEquation;
    if ( !aTokens.empty() )
    {
        sal_Int32 i, nParameters = aTokens.size() - 1;
        if ( nParameters > 3 )
            nParameters = 3;

        rtl::OUString sParameters[ 3 ];

        for ( i = 0; i < nParameters; i++ )
            sParameters[ i ] = GetFormulaParameter( GetAdjCoordinate( rCustomShapeProperties, aTokens[ i + 1 ], sal_False ) );

        const FormulaCommandHMap::const_iterator aIter( pCommandHashMap->find( aTokens[ 0 ] ) );
        if ( aIter != pCommandHashMap->end() )
        {
            switch( aIter->second )
            {
                case FC_MULDIV :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*" ) + sParameters[ 1 ]
                            + CREATE_OUSTRING( "/" ) + sParameters[ 2 ];
                }
                break;
                case FC_PLUSMINUS :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "+" ) + sParameters[ 1 ]
                            + CREATE_OUSTRING( "-" ) + sParameters[ 2 ];
                }
                break;
                case FC_PLUSDIV :
                {
                    if ( nParameters == 3 )
                        aEquation = CREATE_OUSTRING( "(" ) + sParameters[ 0 ] + CREATE_OUSTRING( "+" )
                            + sParameters[ 1 ] + CREATE_OUSTRING( ")/" ) + sParameters[ 2 ];
                }
                break;
                case FC_IFELSE :
                case FC_IFELSE1 :
                {
                    if ( nParameters == 3 )
                        aEquation = CREATE_OUSTRING( "if(" ) + sParameters[ 0 ] + CREATE_OUSTRING( "," )
                            + sParameters[ 1 ] + CREATE_OUSTRING( "," ) + sParameters[ 2 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_ABS :
                {
                    if ( nParameters == 1 )
                        aEquation = CREATE_OUSTRING( "abs(" ) + sParameters[ 0 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_AT2 :
                {
                    if ( nParameters == 2 )
                        aEquation = CREATE_OUSTRING( "(10800000*atan2(" ) + sParameters[ 1 ] + CREATE_OUSTRING( "," )
                        + sParameters[ 0 ] + CREATE_OUSTRING( "))/pi" );
                }
                break;
                case FC_CAT2 :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*(cos(atan2(" ) +
                            sParameters[ 2 ] + CREATE_OUSTRING( "," ) + sParameters[ 1 ] + CREATE_OUSTRING( ")))" );
                }
                break;
                case FC_COS :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*cos(pi*(" ) +
                        sParameters[ 1 ] + CREATE_OUSTRING( ")/10800000)" );
                }
                break;
                case FC_MAX :
                {
                    if ( nParameters == 2 )
                        aEquation = CREATE_OUSTRING( "max(" ) + sParameters[ 0 ] + CREATE_OUSTRING( "," ) +
                            sParameters[ 1 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_MIN :
                {
                    if ( nParameters == 2 )
                        aEquation = CREATE_OUSTRING( "min(" ) + sParameters[ 0 ] + CREATE_OUSTRING( "," ) +
                            sParameters[ 1 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_MOD :
                {
                    if ( nParameters == 3 )
                        aEquation = CREATE_OUSTRING( "sqrt(" )
                            + sParameters[ 0 ] + CREATE_OUSTRING( "*" ) + sParameters[ 0 ] + CREATE_OUSTRING( "+" )
                            + sParameters[ 1 ] + CREATE_OUSTRING( "*" ) + sParameters[ 1 ] + CREATE_OUSTRING( "+" )
                            + sParameters[ 2 ] + CREATE_OUSTRING( "*" ) + sParameters[ 2 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_PIN :
                {
                    if ( nParameters == 3 ) // if(x-y,x,if(y-z,z,y))
                        aEquation = CREATE_OUSTRING( "if(" ) + sParameters[ 0 ] + CREATE_OUSTRING( "-" ) + sParameters[ 1 ]
                            + CREATE_OUSTRING( "," ) + sParameters[ 0 ] + CREATE_OUSTRING( ",if(" ) + sParameters[ 2 ]
                            + CREATE_OUSTRING( "-" ) + sParameters[ 1 ] + CREATE_OUSTRING( "," ) + sParameters[ 1 ]
                            + CREATE_OUSTRING( "," ) + sParameters[ 2 ] + CREATE_OUSTRING( "))" );
                }
                break;
                case FC_SAT2 :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*(sin(atan2(" ) +
                            sParameters[ 2 ] + CREATE_OUSTRING( "," ) + sParameters[ 1 ] + CREATE_OUSTRING( ")))" );
                }
                break;
                case FC_SIN :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*sin(pi*(" ) +
                        sParameters[ 1 ] + CREATE_OUSTRING( ")/10800000)" );
                }
                break;
                case FC_SQRT :
                {
                    if ( nParameters == 1 )
                        aEquation = CREATE_OUSTRING( "sqrt(" ) + sParameters[ 0 ] + CREATE_OUSTRING( ")" );
                }
                break;
                case FC_TAN :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + CREATE_OUSTRING( "*tan(pi*(" ) +
                        sParameters[ 1 ] + CREATE_OUSTRING( ")/10800000)" );
                }
                break;
                case FC_VAL :
                {
                    if ( nParameters == 1 )
                        aEquation = sParameters[ 0 ];
                }
                break;
                default :
                    break;
            }
        }
    }
    return aEquation;
}

Reference< XFastContextHandler > GeomGuideListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    if ( aElementToken == A_TOKEN( gd ) )   // CT_GeomGuide
    {
        CustomShapeGuide aGuide;
        aGuide.maName = xAttribs->getOptionalValue( XML_name );
        aGuide.maFormula = convertToOOEquation( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_fmla ) );
        mrGuideList.push_back( aGuide );
    }
    return this;
}

// ---------------------------------------------------------------------

static const rtl::OUString GetGeomGuideName( const ::rtl::OUString& rValue )
{
    return rValue;
}

// ---------------------------------------------------------------------
// CT_AdjPoint2D
class AdjPoint2DContext : public ContextHandler
{
public:
    AdjPoint2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
};

AdjPoint2DContext::AdjPoint2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler( rParent )
{
    rAdjPoint2D.First = GetAdjCoordinate( rCustomShapeProperties, xAttribs->getOptionalValue( XML_x ), sal_True );
    rAdjPoint2D.Second = GetAdjCoordinate( rCustomShapeProperties, xAttribs->getOptionalValue( XML_y ), sal_True );
}

// ---------------------------------------------------------------------
// CT_XYAdjustHandle
class XYAdjustHandleContext : public ContextHandler
{
public:
    XYAdjustHandleContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    AdjustHandle& mrAdjustHandle;
    CustomShapeProperties& mrCustomShapeProperties;
};

XYAdjustHandleContext::XYAdjustHandleContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle )
: ContextHandler( rParent )
, mrAdjustHandle( rAdjustHandle )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    const rtl::OUString aEmptyDefault;
    AttributeList aAttribs( xAttribs );
    if ( aAttribs.hasAttribute( XML_gdRefX ) )
    {
        mrAdjustHandle.gdRef1 = GetGeomGuideName( aAttribs.getString( XML_gdRefX, aEmptyDefault ) );
    }
    if ( aAttribs.hasAttribute( XML_minX ) )
    {
        mrAdjustHandle.min1 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_minX, aEmptyDefault ), sal_True );
    }
    if ( aAttribs.hasAttribute( XML_maxX ) )
    {
        mrAdjustHandle.max1 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_maxX, aEmptyDefault ), sal_True );
    }
    if ( aAttribs.hasAttribute( XML_gdRefY ) )
    {
        mrAdjustHandle.gdRef2 = GetGeomGuideName( aAttribs.getString( XML_gdRefY, aEmptyDefault ) );
    }
    if ( aAttribs.hasAttribute( XML_minY ) )
    {
        mrAdjustHandle.min2 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_minY, aEmptyDefault ), sal_True );
    }
    if ( aAttribs.hasAttribute( XML_maxY ) )
    {
        mrAdjustHandle.max2 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_maxY, aEmptyDefault ), sal_True );
    }
}

Reference< XFastContextHandler > XYAdjustHandleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pos ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, mrAdjustHandle.pos );   // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_PolarAdjustHandle
class PolarAdjustHandleContext : public ContextHandler
{
public:
    PolarAdjustHandleContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    AdjustHandle& mrAdjustHandle;
    CustomShapeProperties& mrCustomShapeProperties;
};

PolarAdjustHandleContext::PolarAdjustHandleContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle )
: ContextHandler( rParent )
, mrAdjustHandle( rAdjustHandle )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    const rtl::OUString aEmptyDefault;
    AttributeList aAttribs( xAttribs );
    if ( aAttribs.hasAttribute( XML_gdRefR ) )
    {
        mrAdjustHandle.gdRef1 = GetGeomGuideName( aAttribs.getString( XML_gdRefR, aEmptyDefault ) );
    }
    if ( aAttribs.hasAttribute( XML_minR ) )
    {
        mrAdjustHandle.min1 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_minR, aEmptyDefault ), sal_True );
    }
    if ( aAttribs.hasAttribute( XML_maxR ) )
    {
        mrAdjustHandle.max1 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_maxR, aEmptyDefault ), sal_True );
    }
    if ( aAttribs.hasAttribute( XML_gdRefAng ) )
    {
        mrAdjustHandle.gdRef2 = GetGeomGuideName( aAttribs.getString( XML_gdRefAng, aEmptyDefault ) );
    }
    if ( aAttribs.hasAttribute( XML_minAng ) )
    {
        mrAdjustHandle.min2 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_minAng, aEmptyDefault ) );
    }
    if ( aAttribs.hasAttribute( XML_maxAng ) )
    {
        mrAdjustHandle.max2 = GetAdjCoordinate( mrCustomShapeProperties, aAttribs.getString( XML_maxAng, aEmptyDefault ) );
    }
}

Reference< XFastContextHandler > PolarAdjustHandleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pos ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, mrAdjustHandle.pos );   // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_AdjustHandleList
class AdjustHandleListContext : public ContextHandler
{
public:
    AdjustHandleListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< AdjustHandle >& rAdjustHandleList );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    std::vector< AdjustHandle >& mrAdjustHandleList;
    CustomShapeProperties& mrCustomShapeProperties;
};

AdjustHandleListContext::AdjustHandleListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< AdjustHandle >& rAdjustHandleList )
: ContextHandler( rParent )
, mrAdjustHandleList( rAdjustHandleList )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > AdjustHandleListContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( ahXY ) )         // CT_XYAdjustHandle
    {
        AdjustHandle aAdjustHandle( sal_False );
        mrAdjustHandleList.push_back( aAdjustHandle );
        xContext = new XYAdjustHandleContext( *this, xAttribs, mrCustomShapeProperties, mrAdjustHandleList.back() );
    }
    else if ( aElementToken == A_TOKEN( ahPolar ) ) // CT_PolarAdjustHandle
    {
        AdjustHandle aAdjustHandle( sal_True );
        mrAdjustHandleList.push_back( aAdjustHandle );
        xContext = new PolarAdjustHandleContext( *this, xAttribs, mrCustomShapeProperties, mrAdjustHandleList.back() );
    }
    return xContext;
}

// ---------------------------------------------------------------------
// CT_ConnectionSite
class ConnectionSiteContext : public ContextHandler
{
public:
    ConnectionSiteContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, ConnectionSite& rConnectionSite );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    ConnectionSite& mrConnectionSite;
    CustomShapeProperties& mrCustomShapeProperties;
};

ConnectionSiteContext::ConnectionSiteContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, ConnectionSite& rConnectionSite )
: ContextHandler( rParent )
, mrConnectionSite( rConnectionSite )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    mrConnectionSite.ang = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_ang ) );
}

Reference< XFastContextHandler > ConnectionSiteContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pos ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, mrConnectionSite.pos ); // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DMoveTo
class Path2DMoveToContext : public ContextHandler
{
public:
    Path2DMoveToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    EnhancedCustomShapeParameterPair& mrAdjPoint2D;
    CustomShapeProperties& mrCustomShapeProperties;
};

Path2DMoveToContext::Path2DMoveToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler( rParent )
, mrAdjPoint2D( rAdjPoint2D )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > Path2DMoveToContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pt ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, mrAdjPoint2D );     // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DLineTo
class Path2DLineToContext : public ContextHandler
{
public:
    Path2DLineToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    EnhancedCustomShapeParameterPair& mrAdjPoint2D;
    CustomShapeProperties& mrCustomShapeProperties;
};

Path2DLineToContext::Path2DLineToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler( rParent )
, mrAdjPoint2D( rAdjPoint2D )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > Path2DLineToContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pt ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, mrAdjPoint2D );     // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DQuadBezierTo
class Path2DQuadBezierToContext : public ContextHandler
{
public:
    Path2DQuadBezierToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rPt1, EnhancedCustomShapeParameterPair& rPt2 );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    EnhancedCustomShapeParameterPair& mrPt1;
    EnhancedCustomShapeParameterPair& mrPt2;
    int nCount;
    CustomShapeProperties& mrCustomShapeProperties;
};

Path2DQuadBezierToContext::Path2DQuadBezierToContext( ContextHandler& rParent,
    CustomShapeProperties& rCustomShapeProperties,
        EnhancedCustomShapeParameterPair& rPt1,
            EnhancedCustomShapeParameterPair& rPt2 )
: ContextHandler( rParent )
, mrPt1( rPt1 )
, mrPt2( rPt2 )
, nCount( 0 )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > Path2DQuadBezierToContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pt ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties, nCount++ ? mrPt2 : mrPt1 ); // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DCubicBezierTo
class Path2DCubicBezierToContext : public ContextHandler
{
public:
    Path2DCubicBezierToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties,
        EnhancedCustomShapeParameterPair&, EnhancedCustomShapeParameterPair&, EnhancedCustomShapeParameterPair& );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    CustomShapeProperties& mrCustomShapeProperties;
    EnhancedCustomShapeParameterPair& mrControlPt1;
    EnhancedCustomShapeParameterPair& mrControlPt2;
    EnhancedCustomShapeParameterPair& mrEndPt;
    int nCount;
};

Path2DCubicBezierToContext::Path2DCubicBezierToContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties,
    EnhancedCustomShapeParameterPair& rControlPt1,
        EnhancedCustomShapeParameterPair& rControlPt2,
            EnhancedCustomShapeParameterPair& rEndPt )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
, mrControlPt1( rControlPt1 )
, mrControlPt2( rControlPt2 )
, mrEndPt( rEndPt )
, nCount( 0 )
{
}

Reference< XFastContextHandler > Path2DCubicBezierToContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( pt ) )
        xContext = new AdjPoint2DContext( *this, xAttribs, mrCustomShapeProperties,
            nCount++ ? nCount == 2 ? mrControlPt2 : mrEndPt : mrControlPt1 );   // CT_AdjPoint2D
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DContext
class Path2DContext : public ContextHandler
{
public:
    Path2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& rSegments, Path2D& rPath2D );
    virtual ~Path2DContext();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    Path2D& mrPath2D;
    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& mrSegments;
    CustomShapeProperties& mrCustomShapeProperties;
};

Path2DContext::Path2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties, std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& rSegments, Path2D& rPath2D )
: ContextHandler( rParent )
, mrPath2D( rPath2D )
, mrSegments( rSegments )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    const rtl::OUString aEmptyString;

    AttributeList aAttribs( xAttribs );
    rPath2D.w = aAttribs.getString( XML_w, aEmptyString ).toInt64();
    rPath2D.h = aAttribs.getString( XML_h, aEmptyString ).toInt64();
    rPath2D.fill = aAttribs.getToken( XML_fill, XML_norm );
    rPath2D.stroke = aAttribs.getBool( XML_stroke, sal_True );
    rPath2D.extrusionOk = aAttribs.getBool( XML_extrusionOk, sal_True );
}

Path2DContext::~Path2DContext()
{
    EnhancedCustomShapeSegment aNewSegment;
    if ( mrPath2D.fill == XML_none )
    {
        aNewSegment.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
        aNewSegment.Count = 0;
        mrSegments.push_back( aNewSegment );
    }
    aNewSegment.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
    aNewSegment.Count = 0;
    mrSegments.push_back( aNewSegment );
}

Reference< XFastContextHandler > Path2DContext::createFastChildContext( sal_Int32 aElementToken,
    const Reference< XFastAttributeList >& xAttribs ) throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xContext;
    switch( aElementToken )
    {
        case A_TOKEN( close ) :
        {
            EnhancedCustomShapeSegment aNewSegment;
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
            aNewSegment.Count = 0;
            mrSegments.push_back( aNewSegment );
        }
        break;
        case A_TOKEN( moveTo ) :
        {
            EnhancedCustomShapeSegment aNewSegment;
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
            aNewSegment.Count = 1;
            mrSegments.push_back( aNewSegment );

            EnhancedCustomShapeParameterPair aAdjPoint2D;
            mrPath2D.parameter.push_back( aAdjPoint2D );
            xContext = new Path2DMoveToContext( *this, mrCustomShapeProperties, mrPath2D.parameter.back() );
        }
        break;
        case A_TOKEN( lnTo ) :
        {

            if ( !mrSegments.empty() && ( mrSegments.back().Command == EnhancedCustomShapeSegmentCommand::LINETO ) )
                mrSegments.back().Count++;
            else
            {
                EnhancedCustomShapeSegment aSegment;
                aSegment.Command = EnhancedCustomShapeSegmentCommand::LINETO;
                aSegment.Count = 1;
                mrSegments.push_back( aSegment );
            }
            EnhancedCustomShapeParameterPair aAdjPoint2D;
            mrPath2D.parameter.push_back( aAdjPoint2D );
            xContext = new Path2DLineToContext( *this, mrCustomShapeProperties, mrPath2D.parameter.back() );
        }
        break;
        case A_TOKEN( arcTo ) : // CT_Path2DArcTo
        {
            if ( !mrSegments.empty() && ( mrSegments.back().Command == EnhancedCustomShapeSegmentCommand::ARCANGLETO ) )
                mrSegments.back().Count++;
            else
            {
                EnhancedCustomShapeSegment aSegment;
                aSegment.Command = EnhancedCustomShapeSegmentCommand::ARCANGLETO;
                aSegment.Count = 1;
                mrSegments.push_back( aSegment );
            }

            EnhancedCustomShapeParameterPair aScale;
            EnhancedCustomShapeParameterPair aAngles;

            aScale.First = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_wR ), sal_True );
            aScale.Second = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_hR ), sal_True );

            CustomShapeGuide aGuide;
            sal_Int32 nArcNum = mrCustomShapeProperties.getArcNum();

            // start angle
            aGuide.maName = CREATE_OUSTRING("arctosa") + rtl::OUString::valueOf( nArcNum );
            aGuide.maFormula = CREATE_OUSTRING( "(")
                + GetFormulaParameter( GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_stAng ) ) )
                + CREATE_OUSTRING( ")/60000.0" );
            aAngles.First.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( mrCustomShapeProperties.getGuideList(), aGuide ) );
            aAngles.First.Type = EnhancedCustomShapeParameterType::EQUATION;

            // swing angle
            aGuide.maName = CREATE_OUSTRING("arctosw") + rtl::OUString::valueOf( nArcNum );
            aGuide.maFormula = CREATE_OUSTRING( "(")
                + GetFormulaParameter( GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_swAng ) ) )
                + CREATE_OUSTRING( ")/60000.0" );
            aAngles.Second.Value = Any( CustomShapeProperties::SetCustomShapeGuideValue( mrCustomShapeProperties.getGuideList(), aGuide ) );
            aAngles.Second.Type = EnhancedCustomShapeParameterType::EQUATION;

            mrPath2D.parameter.push_back( aScale );
            mrPath2D.parameter.push_back( aAngles );
        }
        break;
        case A_TOKEN( quadBezTo ) :
        {
            if ( !mrSegments.empty() && ( mrSegments.back().Command == EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO ) )
                mrSegments.back().Count++;
            else
            {
                EnhancedCustomShapeSegment aSegment;
                aSegment.Command = EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO;
                aSegment.Count = 1;
                mrSegments.push_back( aSegment );
            }
            EnhancedCustomShapeParameterPair aPt1;
            EnhancedCustomShapeParameterPair aPt2;
            mrPath2D.parameter.push_back( aPt1 );
            mrPath2D.parameter.push_back( aPt2 );
            xContext = new Path2DQuadBezierToContext( *this, mrCustomShapeProperties,
                            mrPath2D.parameter[ mrPath2D.parameter.size() - 2 ],
                                mrPath2D.parameter.back() );
        }
        break;
        case A_TOKEN( cubicBezTo ) :
        {
            if ( !mrSegments.empty() && ( mrSegments.back().Command == EnhancedCustomShapeSegmentCommand::CURVETO ) )
                mrSegments.back().Count++;
            else
            {
                EnhancedCustomShapeSegment aSegment;
                aSegment.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
                aSegment.Count = 1;
                mrSegments.push_back( aSegment );
            }
            EnhancedCustomShapeParameterPair aControlPt1;
            EnhancedCustomShapeParameterPair aControlPt2;
            EnhancedCustomShapeParameterPair aEndPt;
            mrPath2D.parameter.push_back( aControlPt1 );
            mrPath2D.parameter.push_back( aControlPt2 );
            mrPath2D.parameter.push_back( aEndPt );
            xContext = new Path2DCubicBezierToContext( *this, mrCustomShapeProperties,
                            mrPath2D.parameter[ mrPath2D.parameter.size() - 3 ],
                                mrPath2D.parameter[ mrPath2D.parameter.size() - 2 ],
                                    mrPath2D.parameter.back() );
        }
        break;
    }
    return xContext;
}

// ---------------------------------------------------------------------
// CT_Path2DList
class Path2DListContext : public ContextHandler
{
public:
    Path2DListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< EnhancedCustomShapeSegment >& rSegments,
        std::vector< Path2D >& rPath2DList );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:

    CustomShapeProperties& mrCustomShapeProperties;
    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& mrSegments;
    std::vector< Path2D >& mrPath2DList;
};

Path2DListContext::Path2DListContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< EnhancedCustomShapeSegment >& rSegments,
                                        std::vector< Path2D >& rPath2DList )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
, mrSegments( rSegments )
, mrPath2DList( rPath2DList )
{
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL Path2DListContext::createFastChildContext( sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    if ( aElementToken == A_TOKEN( path ) )
    {
        Path2D aPath2D;
        mrPath2DList.push_back( aPath2D );
        xContext = new Path2DContext( *this, xAttribs, mrCustomShapeProperties,  mrSegments, mrPath2DList.back() );
    }
    return xContext;
}

// ---------------------------------------------------------------------

OUString GetShapePresetType( sal_Int32 nType )
{
    OSL_ASSERT((nType & sal_Int32(0xFFFF0000))==0);
    OUString sType;
     switch( nType )
    {
        case XML_lineInv:   // TODO
        case XML_line: {
            static const OUString sLine = CREATE_OUSTRING( "mso-spt20" );
            sType = sLine;
            } break;
        case XML_triangle: {
            static const OUString sTriangle = CREATE_OUSTRING( "isosceles-triangle" );
            sType = sTriangle;
            } break;
        case XML_rtTriangle: {
            static const OUString sRtTriangle = CREATE_OUSTRING( "right-triangle" );
            sType = sRtTriangle;
            } break;
        case XML_rect: {
            static const OUString sRectangle = CREATE_OUSTRING( "rectangle" );
            sType = sRectangle;
            } break;
        case XML_diamond: {
            static const OUString sDiamond = CREATE_OUSTRING( "diamond" );
            sType = sDiamond;
            } break;
        case XML_parallelogram: {
            static const OUString sParallelogram = CREATE_OUSTRING( "parallelogram" );
            sType = sParallelogram;
            } break;
        case XML_nonIsoscelesTrapezoid:     // TODO
        case XML_trapezoid: {
            static const OUString sTrapezoid = CREATE_OUSTRING( "trapezoid" );
            sType = sTrapezoid;
            } break;
        case XML_pentagon: {
            static const OUString sPentagon = CREATE_OUSTRING( "pentagon" );
            sType = sPentagon;
            } break;
        case XML_heptagon:                  // TODO
        case XML_hexagon: {
            static const OUString sHexagon = CREATE_OUSTRING( "hexagon" );
            sType = sHexagon;
            } break;
        case XML_decagon:                   // TODO
        case XML_dodecagon:                 // TODO
        case XML_octagon: {
            static const OUString sOctagon = CREATE_OUSTRING( "octagon" );
            sType = sOctagon;
            } break;
        case XML_star4: {
            static const OUString sStar4 = CREATE_OUSTRING( "star4" );
            sType = sStar4;
            } break;
        case XML_star6:                     // TODO
        case XML_star7:                     // TODO
        case XML_star5: {
            static const OUString sStar5 = CREATE_OUSTRING( "star5" );
            sType = sStar5;
            } break;
        case XML_star10:                    // TODO
        case XML_star12:                    // TODO
        case XML_star16:                    // TODO
        case XML_star8: {
            static const OUString sStar8 = CREATE_OUSTRING( "star8" );
            sType = sStar8;
            } break;
        case XML_star32:                    // TODO
        case XML_star24: {
            static const OUString sStar24 = CREATE_OUSTRING( "star24" );
            sType = sStar24;
            } break;
        case XML_round1Rect:                // TODO
        case XML_round2SameRect:            // TODO
        case XML_round2DiagRect:            // TODO
        case XML_snipRoundRect:             // TODO
        case XML_snip1Rect:                 // TODO
        case XML_snip2SameRect:             // TODO
        case XML_snip2DiagRect:             // TODO
        case XML_roundRect: {
            static const OUString sRoundRect = CREATE_OUSTRING( "round-rectangle" );
            sType = sRoundRect;
            } break;
        case XML_plaque: {
            static const OUString sPlaque = CREATE_OUSTRING( "mso-spt21" );
            sType = sPlaque;
            } break;
        case XML_teardrop:                  // TODO
        case XML_ellipse: {
            static const OUString sEllipse = CREATE_OUSTRING( "ellipse" );
            sType = sEllipse;
            } break;
        case XML_homePlate: {
            static const OUString sHomePlate = CREATE_OUSTRING( "pentagon-right" );
            sType = sHomePlate;
            } break;
        case XML_chevron: {
            static const OUString sChevron = CREATE_OUSTRING( "chevron" );
            sType = sChevron;
            } break;
        case XML_pieWedge:                  // TODO
        case XML_pie:                       // TODO
        case XML_blockArc: {
            static const OUString sBlockArc = CREATE_OUSTRING( "block-arc" );
            sType = sBlockArc;
            } break;
        case XML_donut: {
            static const OUString sDonut = CREATE_OUSTRING( "ring" );
            sType = sDonut;
            } break;
        case XML_noSmoking: {
            static const OUString sNoSmoking = CREATE_OUSTRING( "forbidden" );
            sType = sNoSmoking;
            } break;
        case XML_rightArrow: {
            static const OUString sRightArrow = CREATE_OUSTRING( "right-arrow" );
            sType = sRightArrow;
            } break;
        case XML_leftArrow: {
            static const OUString sLeftArrow = CREATE_OUSTRING( "left-arrow" );
            sType = sLeftArrow;
            } break;
        case XML_upArrow: {
            static const OUString sUpArrow = CREATE_OUSTRING( "up-arrow" );
            sType = sUpArrow;
            } break;
        case XML_downArrow: {
            static const OUString sDownArrow = CREATE_OUSTRING( "down-arrow" );
            sType = sDownArrow;
            } break;
        case XML_stripedRightArrow: {
            static const OUString sStripedRightArrow = CREATE_OUSTRING( "striped-right-arrow" );
            sType = sStripedRightArrow;
            } break;
        case XML_notchedRightArrow: {
            static const OUString sNotchedRightArrow = CREATE_OUSTRING( "notched-right-arrow" );
            sType = sNotchedRightArrow;
            } break;
        case XML_bentUpArrow: {
            static const OUString sBentUpArrow = CREATE_OUSTRING( "mso-spt90" );
            sType = sBentUpArrow;
            } break;
        case XML_leftRightArrow: {
            static const OUString sLeftRightArrow = CREATE_OUSTRING( "left-right-arrow" );
            sType = sLeftRightArrow;
            } break;
        case XML_upDownArrow: {
            static const OUString sUpDownArrow = CREATE_OUSTRING( "up-down-arrow" );
            sType = sUpDownArrow;
            } break;
        case XML_leftUpArrow: {
            static const OUString sLeftUpArrow = CREATE_OUSTRING( "mso-spt89" );
            sType = sLeftUpArrow;
            } break;
        case XML_leftRightUpArrow: {
            static const OUString sLeftRightUpArrow = CREATE_OUSTRING( "mso-spt182" );
            sType = sLeftRightUpArrow;
            } break;
        case XML_quadArrow: {
            static const OUString sQuadArrow = CREATE_OUSTRING( "quad-arrow" );
            sType = sQuadArrow;
            } break;
        case XML_leftArrowCallout: {
            static const OUString sLeftArrowCallout = CREATE_OUSTRING( "left-arrow-callout" );
            sType = sLeftArrowCallout;
            } break;
        case XML_rightArrowCallout: {
            static const OUString sRightArrowCallout = CREATE_OUSTRING( "right-arrow-callout" );
            sType = sRightArrowCallout;
            } break;
        case XML_upArrowCallout: {
            static const OUString sUpArrowCallout = CREATE_OUSTRING( "up-arrow-callout" );
            sType = sUpArrowCallout;
            } break;
        case XML_downArrowCallout: {
            static const OUString sDownArrowCallout = CREATE_OUSTRING( "down-arrow-callout" );
            sType = sDownArrowCallout;
            } break;
        case XML_leftRightArrowCallout: {
            static const OUString sLeftRightArrowCallout = CREATE_OUSTRING( "left-right-arrow-callout" );
            sType = sLeftRightArrowCallout;
            } break;
        case XML_upDownArrowCallout: {
            static const OUString sUpDownArrowCallout = CREATE_OUSTRING( "up-down-arrow-callout" );
            sType = sUpDownArrowCallout;
            } break;
        case XML_quadArrowCallout: {
            static const OUString sQuadArrowCallout = CREATE_OUSTRING( "quad-arrow-callout" );
            sType = sQuadArrowCallout;
            } break;
        case XML_bentArrow: {
            static const OUString sBentArrow = CREATE_OUSTRING( "mso-spt91" );
            sType = sBentArrow;
            } break;
        case XML_uturnArrow: {
            static const OUString sUTurnArrow = CREATE_OUSTRING( "mso-spt101" );
            sType = sUTurnArrow;
            } break;
        case XML_leftCircularArrow:         // TODO
        case XML_leftRightCircularArrow:    // TODO
        case XML_circularArrow: {
            static const OUString sCircularArrow = CREATE_OUSTRING( "circular-arrow" );
            sType = sCircularArrow;
            } break;
        case XML_curvedRightArrow: {
            static const OUString sCurvedRightArrow = CREATE_OUSTRING( "mso-spt102" );
            sType = sCurvedRightArrow;
            } break;
        case XML_curvedLeftArrow: {
            static const OUString sCurvedLeftArrow = CREATE_OUSTRING( "mso-spt103" );
            sType = sCurvedLeftArrow;
            } break;
        case XML_curvedUpArrow: {
            static const OUString sCurvedUpArrow = CREATE_OUSTRING( "mso-spt104" );
            sType = sCurvedUpArrow;
            } break;
        case XML_swooshArrow:               // TODO
        case XML_curvedDownArrow: {
            static const OUString sCurvedDownArrow = CREATE_OUSTRING( "mso-spt105" );
            sType = sCurvedDownArrow;
            } break;
        case XML_cube: {
            static const OUString sCube = CREATE_OUSTRING( "cube" );
            sType = sCube;
            } break;
        case XML_can: {
            static const OUString sCan = CREATE_OUSTRING( "can" );
            sType = sCan;
            } break;
        case XML_lightningBolt: {
            static const OUString sLightningBolt = CREATE_OUSTRING( "lightning" );
            sType = sLightningBolt;
            } break;
        case XML_heart: {
            static const OUString sHeart = CREATE_OUSTRING( "heart" );
            sType = sHeart;
            } break;
        case XML_sun: {
            static const OUString sSun = CREATE_OUSTRING( "sun" );
            sType = sSun;
            } break;
        case XML_moon: {
            static const OUString sMoon = CREATE_OUSTRING( "moon" );
            sType = sMoon;
            } break;
        case XML_smileyFace: {
            static const OUString sSmileyFace = CREATE_OUSTRING( "smiley" );
            sType = sSmileyFace;
            } break;
        case XML_irregularSeal1: {
            static const OUString sIrregularSeal1 = CREATE_OUSTRING( "mso-spt71" );
            sType = sIrregularSeal1;
            } break;
        case XML_irregularSeal2: {
            static const OUString sIrregularSeal2 = CREATE_OUSTRING( "bang" );
            sType = sIrregularSeal2;
            } break;
        case XML_foldedCorner: {
            static const OUString sFoldedCorner = CREATE_OUSTRING( "paper" );
            sType = sFoldedCorner;
            } break;
        case XML_bevel: {
            static const OUString sBevel = CREATE_OUSTRING( "quad-bevel" );
            sType = sBevel;
            } break;
        case XML_halfFrame:                 // TODO
        case XML_corner:                    // TODO
        case XML_diagStripe:                // TODO
        case XML_chord:                     // TODO
        case XML_frame: {
            static const OUString sFrame = CREATE_OUSTRING( "mso-spt75" );
            sType = sFrame;
            } break;
        case XML_arc: {
            static const OUString sArc = CREATE_OUSTRING( "mso-spt19" );
            sType = sArc;
            } break;
        case XML_leftBracket: {
            static const OUString sLeftBracket = CREATE_OUSTRING( "left-bracket" );
            sType = sLeftBracket;
            } break;
        case XML_rightBracket: {
            static const OUString sRightBracket = CREATE_OUSTRING( "right-bracket" );
            sType = sRightBracket;
            } break;
        case XML_leftBrace: {
            static const OUString sLeftBrace = CREATE_OUSTRING( "left-brace" );
            sType = sLeftBrace;
            } break;
        case XML_rightBrace: {
            static const OUString sRightBrace = CREATE_OUSTRING( "right-brace" );
            sType = sRightBrace;
            } break;
        case XML_bracketPair: {
            static const OUString sBracketPair = CREATE_OUSTRING( "bracket-pair" );
            sType = sBracketPair;
            } break;
        case XML_bracePair: {
            static const OUString sBracePair = CREATE_OUSTRING( "brace-pair" );
            sType = sBracePair;
            } break;
        case XML_straightConnector1: {
            static const OUString sStraightConnector1 = CREATE_OUSTRING( "ooxml-straight-connector-1" );
            OSL_TRACE("preset resolved as: ooxml-straight-connector-1");
            sType = sStraightConnector1;
            } break;
        case XML_bentConnector2: {
            static const OUString sBentConnector2 = CREATE_OUSTRING( "mso-spt33" );
            sType = sBentConnector2;
            } break;
        case XML_bentConnector3: {
            static const OUString sBentConnector3 = CREATE_OUSTRING( "mso-spt34" );
            sType = sBentConnector3;
            } break;
        case XML_bentConnector4: {
            static const OUString sBentConnector4 = CREATE_OUSTRING( "mso-spt35" );
            sType = sBentConnector4;
            } break;
        case XML_bentConnector5: {
            static const OUString sBentConnector5 = CREATE_OUSTRING( "mso-spt36" );
            sType = sBentConnector5;
            } break;
        case XML_curvedConnector2: {
            static const OUString sCurvedConnector2 = CREATE_OUSTRING( "mso-spt37" );
            sType = sCurvedConnector2;
            } break;
        case XML_curvedConnector3: {
            static const OUString sCurvedConnector3 = CREATE_OUSTRING( "mso-spt38" );
            sType = sCurvedConnector3;
            } break;
        case XML_curvedConnector4: {
            static const OUString sCurvedConnector4 = CREATE_OUSTRING( "mso-spt39" );
            sType = sCurvedConnector4;
            } break;
        case XML_curvedConnector5: {
            static const OUString sCurvedConnector5 = CREATE_OUSTRING( "mso-spt40" );
            sType = sCurvedConnector5;
            } break;
        case XML_callout1: {
            static const OUString sCallout1 = CREATE_OUSTRING( "mso-spt41" );
            sType = sCallout1;
            } break;
        case XML_callout2: {
            static const OUString sCallout2 = CREATE_OUSTRING( "mso-spt42" );
            sType = sCallout2;
            } break;
        case XML_callout3: {
            static const OUString sCallout3 = CREATE_OUSTRING( "mso-spt43" );
            sType = sCallout3;
            } break;
        case XML_accentCallout1: {
            static const OUString sAccentCallout1 = CREATE_OUSTRING( "mso-spt44" );
            sType = sAccentCallout1;
            } break;
        case XML_accentCallout2: {
            static const OUString sAccentCallout2 = CREATE_OUSTRING( "mso-spt45" );
            sType = sAccentCallout2;
            } break;
        case XML_accentCallout3: {
            static const OUString sAccentCallout3 = CREATE_OUSTRING( "mso-spt46" );
            sType = sAccentCallout3;
            } break;
        case XML_borderCallout1: {
            static const OUString sBorderCallout1 = CREATE_OUSTRING( "line-callout-1" );
            sType = sBorderCallout1;
            } break;
        case XML_borderCallout2: {
            static const OUString sBorderCallout2 = CREATE_OUSTRING( "line-callout-2" );
            sType = sBorderCallout2;
            } break;
        case XML_borderCallout3: {
            static const OUString sBorderCallout3 = CREATE_OUSTRING( "mso-spt49" );
            sType = sBorderCallout3;
            } break;
        case XML_accentBorderCallout1: {
            static const OUString sAccentBorderCallout1 = CREATE_OUSTRING( "mso-spt50" );
            sType = sAccentBorderCallout1;
            } break;
        case XML_accentBorderCallout2: {
            static const OUString sAccentBorderCallout2 = CREATE_OUSTRING( "mso-spt51" );
            sType = sAccentBorderCallout2;
            } break;
        case XML_accentBorderCallout3: {
            static const OUString sAccentBorderCallout3 = CREATE_OUSTRING( "mso-spt52" );
            sType = sAccentBorderCallout3;
            } break;
        case XML_wedgeRectCallout: {
            static const OUString sWedgeRectCallout = CREATE_OUSTRING( "rectangular-callout" );
            sType = sWedgeRectCallout;
            } break;
        case XML_wedgeRoundRectCallout: {
            static const OUString sWedgeRoundRectCallout = CREATE_OUSTRING( "round-rectangular-callout" );
            sType = sWedgeRoundRectCallout;
            } break;
        case XML_wedgeEllipseCallout: {
            static const OUString sWedgeEllipseCallout = CREATE_OUSTRING( "round-callout" );
            sType = sWedgeEllipseCallout;
            } break;
        case XML_cloud:                     // TODO
        case XML_cloudCallout: {
            static const OUString sCloudCallout = CREATE_OUSTRING( "cloud-callout" );
            sType = sCloudCallout;
            } break;
        case XML_ribbon: {
            static const OUString sRibbon = CREATE_OUSTRING( "mso-spt53" );
            sType = sRibbon;
            } break;
        case XML_ribbon2: {
            static const OUString sRibbon2 = CREATE_OUSTRING( "mso-spt54" );
            sType = sRibbon2;
            } break;
        case XML_ellipseRibbon: {
            static const OUString sEllipseRibbon = CREATE_OUSTRING( "mso-spt107" );
            sType = sEllipseRibbon;
            } break;
        case XML_leftRightRibbon:           // TODO
        case XML_ellipseRibbon2: {
            static const OUString sEllipseRibbon2 = CREATE_OUSTRING( "mso-spt108" );
            sType = sEllipseRibbon2;
            } break;
        case XML_verticalScroll: {
            static const OUString sVerticalScroll = CREATE_OUSTRING( "vertical-scroll" );
            sType = sVerticalScroll;
            } break;
        case XML_horizontalScroll: {
            static const OUString sHorizontalScroll = CREATE_OUSTRING( "horizontal-scroll" );
            sType = sHorizontalScroll;
            } break;
        case XML_wave: {
            static const OUString sWave = CREATE_OUSTRING( "mso-spt64" );
            sType = sWave;
            } break;
        case XML_doubleWave: {
            static const OUString sDoubleWave = CREATE_OUSTRING( "mso-spt188" );
            sType = sDoubleWave;
            } break;
        case XML_plus: {
            static const OUString sPlus = CREATE_OUSTRING( "cross" );
            sType = sPlus;
            } break;
        case XML_flowChartProcess: {
            static const OUString sFlowChartProcess = CREATE_OUSTRING( "flowchart-process" );
            sType = sFlowChartProcess;
            } break;
        case XML_flowChartDecision: {
            static const OUString sFlowChartDecision = CREATE_OUSTRING( "flowchart-decision" );
            sType = sFlowChartDecision;
            } break;
        case XML_flowChartInputOutput: {
            static const OUString sFlowChartInputOutput = CREATE_OUSTRING( "flowchart-data" );
            sType = sFlowChartInputOutput;
            } break;
        case XML_flowChartPredefinedProcess: {
            static const OUString sFlowChartPredefinedProcess = CREATE_OUSTRING( "flowchart-predefined-process" );
            sType = sFlowChartPredefinedProcess;
            } break;
        case XML_flowChartInternalStorage: {
            static const OUString sFlowChartInternalStorage = CREATE_OUSTRING( "flowchart-internal-storage" );
            sType = sFlowChartInternalStorage;
            } break;
        case XML_flowChartDocument: {
            static const OUString sFlowChartDocument = CREATE_OUSTRING( "flowchart-document" );
            sType = sFlowChartDocument;
            } break;
        case XML_flowChartMultidocument: {
            static const OUString sFlowChartMultidocument = CREATE_OUSTRING( "flowchart-multidocument" );
            sType = sFlowChartMultidocument;
            } break;
        case XML_flowChartTerminator: {
            static const OUString sFlowChartTerminator = CREATE_OUSTRING( "flowchart-terminator" );
            sType = sFlowChartTerminator;
            } break;
        case XML_flowChartPreparation : {
            static const OUString sFlowChartPreparation = CREATE_OUSTRING( "flowchart-preparation" );
            sType = sFlowChartPreparation;
            } break;
        case XML_flowChartManualInput: {
            static const OUString sFlowChartManualInput = CREATE_OUSTRING( "flowchart-manual-input" );
            sType = sFlowChartManualInput;
            } break;
        case XML_flowChartManualOperation: {
            static const OUString sFlowChartManualOperation = CREATE_OUSTRING( "flowchart-manual-operation" );
            sType = sFlowChartManualOperation;
            } break;
        case XML_flowChartConnector: {
            static const OUString sFlowChartConnector = CREATE_OUSTRING( "flowchart-connector" );
            sType = sFlowChartConnector;
            } break;
        case XML_flowChartPunchedCard: {
            static const OUString sFlowChartPunchedCard = CREATE_OUSTRING( "flowchart-card" );
            sType = sFlowChartPunchedCard;
            } break;
        case XML_flowChartPunchedTape: {
            static const OUString sFlowChartPunchedTape = CREATE_OUSTRING( "flowchart-punched-tape" );
            sType = sFlowChartPunchedTape;
            } break;
        case XML_flowChartSummingJunction: {
            static const OUString sFlowChartSummingJunction = CREATE_OUSTRING( "flowchart-summing-junction" );
            sType = sFlowChartSummingJunction;
            } break;
        case XML_flowChartOr: {
            static const OUString sFlowChartOr = CREATE_OUSTRING( "flowchart-or" );
            sType = sFlowChartOr;
            } break;
        case XML_flowChartCollate: {
            static const OUString sFlowChartCollate = CREATE_OUSTRING( "flowchart-collate" );
            sType = sFlowChartCollate;
            } break;
        case XML_flowChartSort: {
            static const OUString sFlowChartSort = CREATE_OUSTRING( "flowchart-sort" );
            sType = sFlowChartSort;
            } break;
        case XML_flowChartExtract: {
            static const OUString sFlowChartExtract = CREATE_OUSTRING( "flowchart-extract" );
            sType = sFlowChartExtract;
            } break;
        case XML_flowChartMerge: {
            static const OUString sFlowChartMerge = CREATE_OUSTRING( "flowchart-merge" );
            sType = sFlowChartMerge;
            } break;
        case XML_flowChartOfflineStorage: {
            static const OUString sFlowChartOfflineStorage = CREATE_OUSTRING( "mso-spt129" );
            sType = sFlowChartOfflineStorage;
            } break;
        case XML_flowChartOnlineStorage: {
            static const OUString sFlowChartOnlineStorage = CREATE_OUSTRING( "flowchart-stored-data" );
            sType = sFlowChartOnlineStorage;
            } break;
        case XML_flowChartMagneticTape: {
            static const OUString sFlowChartMagneticTape = CREATE_OUSTRING( "flowchart-sequential-access" );
            sType = sFlowChartMagneticTape;
            } break;
        case XML_flowChartMagneticDisk: {
            static const OUString sFlowChartMagneticDisk = CREATE_OUSTRING( "flowchart-magnetic-disk" );
            sType = sFlowChartMagneticDisk;
            } break;
        case XML_flowChartMagneticDrum: {
            static const OUString sFlowChartMagneticDrum = CREATE_OUSTRING( "flowchart-direct-access-storage" );
            sType = sFlowChartMagneticDrum;
            } break;
        case XML_flowChartDisplay: {
            static const OUString sFlowChartDisplay = CREATE_OUSTRING( "flowchart-display" );
            sType = sFlowChartDisplay;
            } break;
        case XML_flowChartDelay: {
            static const OUString sFlowChartDelay = CREATE_OUSTRING( "flowchart-delay" );
            sType = sFlowChartDelay;
            } break;
        case XML_flowChartAlternateProcess: {
            static const OUString sFlowChartAlternateProcess = CREATE_OUSTRING( "flowchart-alternate-process" );
            sType = sFlowChartAlternateProcess;
            } break;
        case XML_flowChartOffpageConnector: {
            static const OUString sFlowChartOffpageConnector = CREATE_OUSTRING( "flowchart-off-page-connector" );
            sType = sFlowChartOffpageConnector;
            } break;
        case XML_actionButtonBlank: {
            static const OUString sActionButtonBlank = CREATE_OUSTRING( "mso-spt189" );
            sType = sActionButtonBlank;
            } break;
        case XML_actionButtonHome: {
            static const OUString sActionButtonHome = CREATE_OUSTRING( "mso-spt190" );
            sType = sActionButtonHome;
            } break;
        case XML_actionButtonHelp: {
            static const OUString sActionButtonHelp = CREATE_OUSTRING( "mso-spt191" );
            sType = sActionButtonHelp;
            } break;
        case XML_actionButtonInformation: {
            static const OUString sActionButtonInformation = CREATE_OUSTRING( "mso-spt192" );
            sType = sActionButtonInformation;
            } break;
        case XML_actionButtonForwardNext: {
            static const OUString sActionButtonForwardNext = CREATE_OUSTRING( "mso-spt193" );
            sType = sActionButtonForwardNext;
            } break;
        case XML_actionButtonBackPrevious: {
            static const OUString sActionButtonBackPrevious = CREATE_OUSTRING( "mso-spt194" );
            sType = sActionButtonBackPrevious;
            } break;
        case XML_actionButtonEnd: {
            static const OUString sActionButtonEnd = CREATE_OUSTRING( "mso-spt195" );
            sType = sActionButtonEnd;
            } break;
        case XML_actionButtonBeginning: {
            static const OUString sActionButtonBeginning = CREATE_OUSTRING( "mso-spt196" );
            sType = sActionButtonBeginning;
            } break;
        case XML_actionButtonReturn: {
            static const OUString sActionButtonReturn = CREATE_OUSTRING( "mso-spt197" );
            sType = sActionButtonReturn;
            } break;
        case XML_actionButtonDocument: {
            static const OUString sActionButtonDocument = CREATE_OUSTRING( "mso-spt198" );
            sType = sActionButtonDocument;
            } break;
        case XML_actionButtonSound: {
            static const OUString sActionButtonSound = CREATE_OUSTRING( "mso-spt199" );
            sType = sActionButtonSound;
            } break;
        case XML_actionButtonMovie: {
            static const OUString sActionButtonMovie = CREATE_OUSTRING( "mso-spt200" );
            sType = sActionButtonMovie;
            } break;
        case XML_gear6:                     // TODO
        case XML_gear9:                     // TODO
        case XML_funnel:                    // TODO
        case XML_mathPlus:                  // TODO
        case XML_mathMinus:                 // TODO
        case XML_mathMultiply:              // TODO
        case XML_mathDivide:                // TODO
        case XML_mathEqual:                 // TODO
        case XML_mathNotEqual:              // TODO
        case XML_cornerTabs:                // TODO
        case XML_squareTabs:                // TODO
        case XML_plaqueTabs:                // TODO
        case XML_chartX:                    // TODO
        case XML_chartStar:                 // TODO
        case XML_chartPlus: {               // TODO
            static const OUString sRectangle = CREATE_OUSTRING( "rectangle" );
            sType = sRectangle;
            } break;
        default:
            break;
    }
    return sType;
}

// ---------------------------------------------------------------------
// CT_CustomGeometry2D
CustomShapeGeometryContext::CustomShapeGeometryContext( ContextHandler& rParent, const Reference< XFastAttributeList >& /* xAttribs */, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > CustomShapeGeometryContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xContext;
    switch( aElementToken )
    {
        case A_TOKEN( avLst ):          // CT_GeomGuideList adjust value list
            xContext = new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
        break;
        case A_TOKEN( gdLst ):          // CT_GeomGuideList guide list
            xContext = new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getGuideList() );
        break;
        case A_TOKEN( ahLst ):          // CT_AdjustHandleList adjust handle list
            xContext = new AdjustHandleListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustHandleList() );
        break;
        case A_TOKEN( cxnLst ):         // CT_ConnectionSiteList connection site list
            xContext = this;
        break;
        case A_TOKEN( rect ):           // CT_GeomRectList geometry rect list
        {
            GeomRect aGeomRect;
            aGeomRect.l = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_l ), sal_True );
            aGeomRect.t = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_t ), sal_True );
            aGeomRect.r = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_r ), sal_True );
            aGeomRect.b = GetAdjCoordinate( mrCustomShapeProperties, xAttribs->getOptionalValue( XML_b ), sal_True );
            mrCustomShapeProperties.getTextRect() = aGeomRect;
        }
        break;
        case A_TOKEN( pathLst ):        // CT_Path2DList 2d path list
            xContext = new Path2DListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getSegments(), mrCustomShapeProperties.getPath2DList() );
        break;

        // from cxnLst:
        case A_TOKEN( cxn ):                // CT_ConnectionSite
        {
            ConnectionSite aConnectionSite;
            mrCustomShapeProperties.getConnectionSiteList().push_back( aConnectionSite );
            xContext = new ConnectionSiteContext( *this, xAttribs, mrCustomShapeProperties, mrCustomShapeProperties.getConnectionSiteList().back() );
        }
        break;
    }
    return xContext;
}

// ---------------------------------------------------------------------
// CT_PresetGeometry2D
PresetShapeGeometryContext::PresetShapeGeometryContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    sal_Int32 nShapeType = xAttribs->getOptionalValueToken( XML_prst, FastToken::DONTKNOW );
    OSL_ENSURE( nShapeType != FastToken::DONTKNOW, "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( nShapeType );
}

Reference< XFastContextHandler > PresetShapeGeometryContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    if ( aElementToken == A_TOKEN( avLst ) )
        return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
    else
        return this;
}

// ---------------------------------------------------------------------
// CT_PresetTextShape
PresetTextShapeContext::PresetTextShapeContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    sal_Int32 nShapeType = xAttribs->getOptionalValueToken( XML_prst, FastToken::DONTKNOW );
    OSL_ENSURE( nShapeType != FastToken::DONTKNOW, "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( nShapeType );
}

Reference< XFastContextHandler > PresetTextShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    if ( aElementToken == A_TOKEN( avLst ) )
        return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
    else
        return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
