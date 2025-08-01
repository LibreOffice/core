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

#include <drawingml/customshapegeometry.hxx>
#include <drawingml/customshapeproperties.hxx>

#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <unordered_map>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;

namespace oox::drawingml {

namespace {

enum FormulaCommand
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
    FC_VAL
};

struct FormulaCommandNameTable
{
    const char*     pS;
    FormulaCommand pE;
};

}

const FormulaCommandNameTable pFormulaCommandNameTable[] =
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
typedef std::unordered_map< OUString, FormulaCommand > FormulaCommandHMap;

static const FormulaCommandHMap* pCommandHashMap;

static OUString GetFormulaParameter( const EnhancedCustomShapeParameter& rParameter )
{
    OUString aRet;
    switch( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::NORMAL :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue = 0.0;
                if ( rParameter.Value >>= fValue )
                    aRet = OUString::number( fValue );
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                    aRet = OUString::number( nValue );
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
                    aRet = "?"
                        + OUString::number( nFormulaIndex )
                            + " ";
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
                    aRet = "$"
                        + OUString::number( nAdjustmentIndex )
                            + " ";
                }
            }
            else
            {
                // ups... we should have an index here and not the formula name
            }
        }
        break;
        case EnhancedCustomShapeParameterType::LEFT :
            aRet = "left";
        break;
        case EnhancedCustomShapeParameterType::TOP :
            aRet = "top";
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
            aRet = "right";
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
            aRet = "bottom";
        break;
        case EnhancedCustomShapeParameterType::XSTRETCH :
            aRet = "xstretch";
        break;
        case EnhancedCustomShapeParameterType::YSTRETCH :
            aRet = "ystretch";
        break;
        case EnhancedCustomShapeParameterType::HASSTROKE :
            aRet = "hasstroke";
        break;
        case EnhancedCustomShapeParameterType::HASFILL :
            aRet = "hasfill";
        break;
        case EnhancedCustomShapeParameterType::WIDTH :
            aRet = "width";
        break;
        case EnhancedCustomShapeParameterType::HEIGHT :
            aRet = "height";
        break;
        case EnhancedCustomShapeParameterType::LOGWIDTH :
            aRet = "logwidth";
        break;
        case EnhancedCustomShapeParameterType::LOGHEIGHT :
            aRet = "logheight";
        break;
    }
    return aRet;
}

static EnhancedCustomShapeParameter GetAdjCoordinate( CustomShapeProperties& rCustomShapeProperties, const OUString& rValue, bool bNoSymbols = true )
{
    css::drawing::EnhancedCustomShapeParameter aRet;
    if ( !rValue.isEmpty() )
    {
        bool        bConstant = true;
        sal_Int32   nConstant = -1;
        sal_Int32   nIntVal = 0;

        // first check if it's a constant value
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
                    aGuide.maFormula = "logheight" ;

                    aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
                    aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                }
                else
                    aRet.Type = EnhancedCustomShapeParameterType::LOGHEIGHT;   // TODO: HEIGHT needs to be implemented
            }
            break;

            case XML_hd10 :
                nIntVal += 2; // */ h 1.0 10.0
                [[fallthrough]];
            case XML_hd8 :    // */ h 1.0 8.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_hd6 :    // */ h 1.0 6.0
                nIntVal++;
                [[fallthrough]];
            case XML_hd5 :    // */ h 1.0 5.0
                nIntVal++;
                [[fallthrough]];
            case XML_hd4 :    // */ h 1.0 4.0
                nIntVal++;
                [[fallthrough]];
            case XML_hd3 :    // */ h 1.0 3.0
                nIntVal++;
                [[fallthrough]];
            case XML_hd2 :    // */ h 1.0 2.0
            case XML_vc :     // */ h 1.0 2.0
            {
                nIntVal += 2;

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = "logheight/" + OUString::number( nIntVal );

                aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
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
                aGuide.maFormula = "max(logwidth,logheight)";

                aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;
            case XML_ss :   // shortest side: min w h
            {
                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = "min(logwidth,logheight)";

                aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;
            case XML_ssd32 : // */ ss 1.0 32.0
                nIntVal += 16;
                [[fallthrough]];
            case XML_ssd16 : // */ ss 1.0 16.0
                nIntVal += 8;
                [[fallthrough]];
            case XML_ssd8 :  // */ ss 1.0 8.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_ssd6 :  // */ ss 1.0 6.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_ssd4 :  // */ ss 1.0 4.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_ssd2 :  // */ ss 1.0 2.0
            {
                nIntVal += 2;

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = "min(logwidth,logheight)/" + OUString::number( nIntVal );

                aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
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
                    aGuide.maFormula = "logwidth" ;

                    aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
                    aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                }
                else
                    aRet.Type = EnhancedCustomShapeParameterType::LOGWIDTH;
            }
            break;

            case XML_wd32 : // */ w 1.0 32.0
                nIntVal += 20;
                [[fallthrough]];
            case XML_wd12 : // */ w 1.0 12.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_wd10 : // */ w 1.0 10.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_wd8 :  // */ w 1.0 8.0
                nIntVal += 2;
                [[fallthrough]];
            case XML_wd6 :  // */ w 1.0 6.0
                nIntVal++;
                [[fallthrough]];
            case XML_wd5 :  // */ w 1.0 5.0
                nIntVal++;
                [[fallthrough]];
            case XML_wd4 :  // */ w 1.0 4.0
                nIntVal++;
                [[fallthrough]];
            case XML_wd3 :  // */ w 1.0 3.0
                nIntVal++;
                [[fallthrough]];
            case XML_hc :   // */ w 1.0 2.0
            case XML_wd2 :  // */ w 1.0 2.0
            {
                nIntVal += 2;

                CustomShapeGuide aGuide;
                aGuide.maName = rValue;
                aGuide.maFormula = "logwidth/" + OUString::number( nIntVal );

                aRet.Value <<= rCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );;
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            }
            break;

            default:
                bConstant = false;
            break;
        }
        if ( bConstant )
        {
            if (nConstant != -1) {
                aRet.Value <<= nConstant;
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
        }
        else
        {
            // Check if rValue is a number
            sal_Unicode n = rValue[ 0 ];
            bool bHasSign =  ( n == '+' ) || ( n == '-' );
            // "+" or "-" is formally allowed as guide name and works in app 'SoftMaker Office NX'.
            bool bIsNumber = !(bHasSign && (rValue.getLength() == 1));
            for (sal_Int32 i = bHasSign ? 1 : 0; i < rValue.getLength(); ++i)
            {
                if (!isdigit(rValue[i]))
                {
                    bIsNumber = false;
                    break;
                }
            }
            if (bIsNumber)
            {   // seems to be a ST_Coordinate
                aRet.Value <<= rValue.toInt32();
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
            else
            {
                sal_Int32 nGuideIndex = rCustomShapeProperties.getAdjustmentGuideList().GetCustomShapeGuideValue( rValue );
                if ( nGuideIndex >= 0 )
                {
                    aRet.Value <<= nGuideIndex;
                    aRet.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
                }
                else
                {
                    nGuideIndex = rCustomShapeProperties.getGuideList().GetCustomShapeGuideValue( rValue );
                    if ( nGuideIndex >= 0 )
                    {
                        aRet.Value <<= nGuideIndex;
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                    }
                    else
                    {
                        SAL_WARN("oox", "error: unhandled value " << rValue);
                        aRet.Value <<= rValue;
                    }
                }
            }
        }
    }
    return aRet;
}

namespace {

// CT_GeomGuideList
class GeomGuideListContext : public ContextHandler2
{
public:
    GeomGuideListContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, CustomShapeGuideContainer& rGuideList );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    CustomShapeGuideContainer&    mrGuideList;
    CustomShapeProperties&        mrCustomShapeProperties;
};

}

GeomGuideListContext::GeomGuideListContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, CustomShapeGuideContainer& rGuideList )
: ContextHandler2( rParent )
, mrGuideList( rGuideList )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

static OUString convertToOOEquation( CustomShapeProperties& rCustomShapeProperties, std::u16string_view rSource )
{
    if ( !pCommandHashMap )
    {
        FormulaCommandHMap* pHM = new FormulaCommandHMap;
        for(const FormulaCommandNameTable& i : pFormulaCommandNameTable)
            (*pHM)[ OUString::createFromAscii( i.pS ) ] =  i.pE;
        pCommandHashMap = pHM;
    }

    std::vector< OUString > aTokens;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken( o3tl::getToken(rSource, 0, ' ', nIndex ) );
        if ( !aToken.isEmpty() )
            aTokens.push_back( aToken );
    }
    while ( nIndex >= 0 );

    OUString aEquation;
    if ( !aTokens.empty() )
    {
        sal_Int32 i, nParameters = aTokens.size() - 1;
        if ( nParameters > 3 )
            nParameters = 3;

        OUString sParameters[ 3 ];

        for ( i = 0; i < nParameters; i++ )
            sParameters[ i ] = GetFormulaParameter( GetAdjCoordinate( rCustomShapeProperties, aTokens[ i + 1 ], false ) );

        const FormulaCommandHMap::const_iterator aIter( pCommandHashMap->find( aTokens[ 0 ] ) );
        if ( aIter != pCommandHashMap->end() )
        {
            switch( aIter->second )
            {
                case FC_MULDIV :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + "*" + sParameters[ 1 ]
                            + "/" + sParameters[ 2 ];
                }
                break;
                case FC_PLUSMINUS :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + "+" + sParameters[ 1 ]
                            + "-" + sParameters[ 2 ];
                }
                break;
                case FC_PLUSDIV :
                {
                    if ( nParameters == 3 )
                        aEquation = "(" + sParameters[ 0 ] + "+"
                            + sParameters[ 1 ] + ")/" + sParameters[ 2 ];
                }
                break;
                case FC_IFELSE :
                case FC_IFELSE1 :
                {
                    if ( nParameters == 3 )
                        aEquation = "if(" + sParameters[ 0 ] + ","
                            + sParameters[ 1 ] + "," + sParameters[ 2 ] + ")";
                }
                break;
                case FC_ABS :
                {
                    if ( nParameters == 1 )
                        aEquation = "abs(" + sParameters[ 0 ] + ")";
                }
                break;
                case FC_AT2 :
                {
                    if ( nParameters == 2 )
                        aEquation = "(10800000*atan2(" + sParameters[ 1 ] + ","
                        + sParameters[ 0 ] + "))/pi";
                }
                break;
                case FC_CAT2 :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + "*(cos(atan2(" +
                            sParameters[ 2 ] + "," + sParameters[ 1 ] + ")))";
                }
                break;
                case FC_COS :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + "*cos(pi*(" +
                        sParameters[ 1 ] + ")/10800000)";
                }
                break;
                case FC_MAX :
                {
                    if ( nParameters == 2 )
                        aEquation = "max(" + sParameters[ 0 ] + "," +
                            sParameters[ 1 ] + ")";
                }
                break;
                case FC_MIN :
                {
                    if ( nParameters == 2 )
                        aEquation = "min(" + sParameters[ 0 ] + "," +
                            sParameters[ 1 ] + ")";
                }
                break;
                case FC_MOD :
                {
                    if ( nParameters == 3 )
                        aEquation = "sqrt("
                            + sParameters[ 0 ] + "*" + sParameters[ 0 ] + "+"
                            + sParameters[ 1 ] + "*" + sParameters[ 1 ] + "+"
                            + sParameters[ 2 ] + "*" + sParameters[ 2 ] + ")";
                }
                break;
                case FC_PIN :
                {
                    if ( nParameters == 3 ) // if(x-y,x,if(y-z,z,y))
                        aEquation = "if(" + sParameters[ 0 ] + "-" + sParameters[ 1 ]
                            + "," + sParameters[ 0 ] + ",if(" + sParameters[ 2 ]
                            + "-" + sParameters[ 1 ] + "," + sParameters[ 1 ]
                            + "," + sParameters[ 2 ] + "))";
                }
                break;
                case FC_SAT2 :
                {
                    if ( nParameters == 3 )
                        aEquation = sParameters[ 0 ] + "*(sin(atan2(" +
                            sParameters[ 2 ] + "," + sParameters[ 1 ] + ")))";
                }
                break;
                case FC_SIN :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + "*sin(pi*(" +
                        sParameters[ 1 ] + ")/10800000)";
                }
                break;
                case FC_SQRT :
                {
                    if ( nParameters == 1 )
                        aEquation = "sqrt(" + sParameters[ 0 ] + ")";
                }
                break;
                case FC_TAN :
                {
                    if ( nParameters == 2 )
                        aEquation = sParameters[ 0 ] + "*tan(pi*(" +
                        sParameters[ 1 ] + ")/10800000)";
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

ContextHandlerRef GeomGuideListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( gd ) )   // CT_GeomGuide
    {
        CustomShapeGuide aGuide;
        aGuide.maName = rAttribs.getStringDefaulted( XML_name );
        aGuide.maFormula = convertToOOEquation( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_fmla ) );
        mrGuideList.push_back( aGuide );
    }
    return this;
}

static const OUString& GetGeomGuideName( const OUString& rValue )
{
    return rValue;
}

namespace {

// CT_AdjPoint2D
class AdjPoint2DContext : public ContextHandler2
{
public:
    AdjPoint2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
};

}

AdjPoint2DContext::AdjPoint2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler2( rParent )
{
    rAdjPoint2D.First = GetAdjCoordinate( rCustomShapeProperties, rAttribs.getStringDefaulted( XML_x ) );
    rAdjPoint2D.Second = GetAdjCoordinate( rCustomShapeProperties, rAttribs.getStringDefaulted( XML_y ) );
}

namespace {

// CT_XYAdjustHandle
class XYAdjustHandleContext : public ContextHandler2
{
public:
    XYAdjustHandleContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    AdjustHandle& mrAdjustHandle;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

XYAdjustHandleContext::XYAdjustHandleContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle )
: ContextHandler2( rParent )
, mrAdjustHandle( rAdjustHandle )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    if ( rAttribs.hasAttribute( XML_gdRefX ) )
    {
        mrAdjustHandle.gdRef1 = GetGeomGuideName( rAttribs.getStringDefaulted( XML_gdRefX) );
    }
    if ( rAttribs.hasAttribute( XML_minX ) )
    {
        mrAdjustHandle.min1 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_minX) );
    }
    if ( rAttribs.hasAttribute( XML_maxX ) )
    {
        mrAdjustHandle.max1 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_maxX) );
    }
    if ( rAttribs.hasAttribute( XML_gdRefY ) )
    {
        mrAdjustHandle.gdRef2 = GetGeomGuideName( rAttribs.getStringDefaulted( XML_gdRefY) );
    }
    if ( rAttribs.hasAttribute( XML_minY ) )
    {
        mrAdjustHandle.min2 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_minY) );
    }
    if ( rAttribs.hasAttribute( XML_maxY ) )
    {
        mrAdjustHandle.max2 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_maxY) );
    }
}

ContextHandlerRef XYAdjustHandleContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pos ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, mrAdjustHandle.pos );   // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_PolarAdjustHandle
class PolarAdjustHandleContext : public ContextHandler2
{
public:
    PolarAdjustHandleContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    AdjustHandle& mrAdjustHandle;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

PolarAdjustHandleContext::PolarAdjustHandleContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, AdjustHandle& rAdjustHandle )
: ContextHandler2( rParent )
, mrAdjustHandle( rAdjustHandle )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    if ( rAttribs.hasAttribute( XML_gdRefR ) )
    {
        mrAdjustHandle.polar = true ;
        mrAdjustHandle.gdRef1 = GetGeomGuideName( rAttribs.getStringDefaulted( XML_gdRefR) );
    }
    if ( rAttribs.hasAttribute( XML_minR ) )
    {
        mrAdjustHandle.min1 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_minR) );
    }
    if ( rAttribs.hasAttribute( XML_maxR ) )
    {
        mrAdjustHandle.max1 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_maxR) );
    }
    if ( rAttribs.hasAttribute( XML_gdRefAng ) )
    {
        mrAdjustHandle.polar = true ;
        mrAdjustHandle.gdRef2 = GetGeomGuideName( rAttribs.getStringDefaulted( XML_gdRefAng) );
    }
    if ( rAttribs.hasAttribute( XML_minAng ) )
    {
        mrAdjustHandle.min2 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_minAng) );
    }
    if ( rAttribs.hasAttribute( XML_maxAng ) )
    {
        mrAdjustHandle.max2 = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_maxAng) );
    }
}

ContextHandlerRef PolarAdjustHandleContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    // mrAdjustHandle.pos uses planar coordinates.
    if ( aElementToken == A_TOKEN( pos ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, mrAdjustHandle.pos );   // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_AdjustHandleList
class AdjustHandleListContext : public ContextHandler2
{
public:
    AdjustHandleListContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< AdjustHandle >& rAdjustHandleList );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    std::vector< AdjustHandle >& mrAdjustHandleList;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

AdjustHandleListContext::AdjustHandleListContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< AdjustHandle >& rAdjustHandleList )
: ContextHandler2( rParent )
, mrAdjustHandleList( rAdjustHandleList )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

ContextHandlerRef AdjustHandleListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( ahXY ) )         // CT_XYAdjustHandle
    {
        AdjustHandle aAdjustHandle( false );
        mrAdjustHandleList.push_back( aAdjustHandle );
        return new XYAdjustHandleContext( *this, rAttribs, mrCustomShapeProperties, mrAdjustHandleList.back() );
    }
    else if ( aElementToken == A_TOKEN( ahPolar ) ) // CT_PolarAdjustHandle
    {
        AdjustHandle aAdjustHandle( true );
        mrAdjustHandleList.push_back( aAdjustHandle );
        return new PolarAdjustHandleContext( *this, rAttribs, mrCustomShapeProperties, mrAdjustHandleList.back() );
    }
    return nullptr;
}

namespace {

// CT_ConnectionSite
class ConnectionSiteContext : public ContextHandler2
{
public:
    ConnectionSiteContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, ConnectionSite& rConnectionSite );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    ConnectionSite& mrConnectionSite;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

ConnectionSiteContext::ConnectionSiteContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, ConnectionSite& rConnectionSite )
: ContextHandler2( rParent )
, mrConnectionSite( rConnectionSite )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    mrConnectionSite.ang = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_ang ) );
}

ContextHandlerRef ConnectionSiteContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pos ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, mrConnectionSite.pos ); // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_Path2DMoveTo
class Path2DMoveToContext : public ContextHandler2
{
public:
    Path2DMoveToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    EnhancedCustomShapeParameterPair& mrAdjPoint2D;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

Path2DMoveToContext::Path2DMoveToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler2( rParent )
, mrAdjPoint2D( rAdjPoint2D )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

ContextHandlerRef Path2DMoveToContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pt ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, mrAdjPoint2D );     // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_Path2DLineTo
class Path2DLineToContext : public ContextHandler2
{
public:
    Path2DLineToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    EnhancedCustomShapeParameterPair& mrAdjPoint2D;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

Path2DLineToContext::Path2DLineToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rAdjPoint2D )
: ContextHandler2( rParent )
, mrAdjPoint2D( rAdjPoint2D )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

ContextHandlerRef Path2DLineToContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pt ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, mrAdjPoint2D );     // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_Path2DQuadBezierTo
class Path2DQuadBezierToContext : public ContextHandler2
{
public:
    Path2DQuadBezierToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, EnhancedCustomShapeParameterPair& rPt1, EnhancedCustomShapeParameterPair& rPt2 );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    EnhancedCustomShapeParameterPair& mrPt1;
    EnhancedCustomShapeParameterPair& mrPt2;
    int nCount;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

Path2DQuadBezierToContext::Path2DQuadBezierToContext( ContextHandler2Helper const & rParent,
    CustomShapeProperties& rCustomShapeProperties,
        EnhancedCustomShapeParameterPair& rPt1,
            EnhancedCustomShapeParameterPair& rPt2 )
: ContextHandler2( rParent )
, mrPt1( rPt1 )
, mrPt2( rPt2 )
, nCount( 0 )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

ContextHandlerRef Path2DQuadBezierToContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pt ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties, nCount++ ? mrPt2 : mrPt1 ); // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_Path2DCubicBezierTo
class Path2DCubicBezierToContext : public ContextHandler2
{
public:
    Path2DCubicBezierToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties,
        EnhancedCustomShapeParameterPair&, EnhancedCustomShapeParameterPair&, EnhancedCustomShapeParameterPair& );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    CustomShapeProperties& mrCustomShapeProperties;
    EnhancedCustomShapeParameterPair& mrControlPt1;
    EnhancedCustomShapeParameterPair& mrControlPt2;
    EnhancedCustomShapeParameterPair& mrEndPt;
    int nCount;
};

}

Path2DCubicBezierToContext::Path2DCubicBezierToContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties,
    EnhancedCustomShapeParameterPair& rControlPt1,
        EnhancedCustomShapeParameterPair& rControlPt2,
            EnhancedCustomShapeParameterPair& rEndPt )
: ContextHandler2( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
, mrControlPt1( rControlPt1 )
, mrControlPt2( rControlPt2 )
, mrEndPt( rEndPt )
, nCount( 0 )
{
}

ContextHandlerRef Path2DCubicBezierToContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( pt ) )
        return new AdjPoint2DContext( *this, rAttribs, mrCustomShapeProperties,
            nCount++ ? nCount == 2 ? mrControlPt2 : mrEndPt : mrControlPt1 );   // CT_AdjPoint2D
    return nullptr;
}

namespace {

// CT_Path2DContext
class Path2DContext : public ContextHandler2
{
public:
    Path2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, std::vector< css::drawing::EnhancedCustomShapeSegment >& rSegments, Path2D& rPath2D );
    virtual void onEndElement() override;
    virtual ::oox::core::ContextHandlerRef
        onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:
    Path2D& mrPath2D;
    std::vector< css::drawing::EnhancedCustomShapeSegment >& mrSegments;
    CustomShapeProperties& mrCustomShapeProperties;
};

}

Path2DContext::Path2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties, std::vector< css::drawing::EnhancedCustomShapeSegment >& rSegments, Path2D& rPath2D )
: ContextHandler2( rParent )
, mrPath2D( rPath2D )
, mrSegments( rSegments )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    rPath2D.w = rAttribs.getHyper( XML_w, 0 );
    rPath2D.h = rAttribs.getHyper( XML_h, 0 );
    rPath2D.fill = rAttribs.getToken( XML_fill, XML_norm );
    rPath2D.stroke = rAttribs.getBool( XML_stroke, true );
    rPath2D.extrusionOk = rAttribs.getBool( XML_extrusionOk, true );
}

void Path2DContext::onEndElement()
{
    EnhancedCustomShapeSegment aNewSegment;
    switch ( mrPath2D.fill )
    {
        case XML_none:
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
            break;
        case XML_darken:
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::DARKEN;
            break;
        case XML_darkenLess:
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::DARKENLESS;
            break;
        case XML_lighten:
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::LIGHTEN;
            break;
        case XML_lightenLess:
            aNewSegment.Command = EnhancedCustomShapeSegmentCommand::LIGHTENLESS;
            break;
    }
    if (mrPath2D.fill != XML_norm) {
        aNewSegment.Count = 0;
        mrSegments.push_back( aNewSegment );
    }
    if ( !mrPath2D.stroke )
    {
        aNewSegment.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
        aNewSegment.Count = 0;
        mrSegments.push_back( aNewSegment );
    }
    aNewSegment.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
    aNewSegment.Count = 0;
    mrSegments.push_back( aNewSegment );
}


ContextHandlerRef Path2DContext::onCreateContext( sal_Int32 aElementToken,
    const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( close ) :
        {
            // ignore close after move to (ppt does seems to do the same, see accentCallout2 preset for example)
            if ( mrSegments.empty() || ( mrSegments.back().Command != EnhancedCustomShapeSegmentCommand::MOVETO ) ) {
                EnhancedCustomShapeSegment aNewSegment;
                aNewSegment.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                aNewSegment.Count = 0;
                mrSegments.push_back( aNewSegment );
            }
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
            return new Path2DMoveToContext( *this, mrCustomShapeProperties, mrPath2D.parameter.back() );
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
            return new Path2DLineToContext( *this, mrCustomShapeProperties, mrPath2D.parameter.back() );
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

            aScale.First = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_wR ) );
            aScale.Second = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_hR ) );

            CustomShapeGuide aGuide;
            sal_Int32 nArcNum = mrCustomShapeProperties.getArcNum();

            // start angle
            aGuide.maName = "arctosa" + OUString::number( nArcNum );
            aGuide.maFormula = "("
                + GetFormulaParameter( GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_stAng ) ) )
                + ")/60000.0";
            aAngles.First.Value <<= mrCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
            aAngles.First.Type = EnhancedCustomShapeParameterType::EQUATION;

            // swing angle
            aGuide.maName = "arctosw" + OUString::number( nArcNum );
            aGuide.maFormula = "("
                + GetFormulaParameter( GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_swAng ) ) )
                + ")/60000.0";
            aAngles.Second.Value <<= mrCustomShapeProperties.getGuideList().SetCustomShapeGuideValue( aGuide );
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
            return new Path2DQuadBezierToContext( *this, mrCustomShapeProperties,
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
            return new Path2DCubicBezierToContext( *this, mrCustomShapeProperties,
                            mrPath2D.parameter[ mrPath2D.parameter.size() - 3 ],
                                mrPath2D.parameter[ mrPath2D.parameter.size() - 2 ],
                                    mrPath2D.parameter.back() );
        }
        break;
    }
    return nullptr;
}

namespace {

// CT_Path2DList
class Path2DListContext : public ContextHandler2
{
public:
    Path2DListContext( ContextHandler2Helper const & rParent, CustomShapeProperties & rCustomShapeProperties, std::vector< EnhancedCustomShapeSegment >& rSegments,
        std::vector< Path2D >& rPath2DList );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const ::oox::AttributeList& rAttribs ) override;

protected:

    CustomShapeProperties& mrCustomShapeProperties;
    std::vector< css::drawing::EnhancedCustomShapeSegment >& mrSegments;
    std::vector< Path2D >& mrPath2DList;
};

}

Path2DListContext:: Path2DListContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties, std::vector< EnhancedCustomShapeSegment >& rSegments,
                                        std::vector< Path2D >& rPath2DList )
: ContextHandler2( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
, mrSegments( rSegments )
, mrPath2DList( rPath2DList )
{
}

ContextHandlerRef Path2DListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if ( aElementToken == A_TOKEN( path ) )
    {
        mrPath2DList.push_back(Path2D());
        return new Path2DContext( *this, rAttribs, mrCustomShapeProperties,  mrSegments, mrPath2DList.back() );
    }
    return nullptr;
}

// CT_CustomGeometry2D
CustomShapeGeometryContext::CustomShapeGeometryContext( ContextHandler2Helper const & rParent, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler2( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

ContextHandlerRef CustomShapeGeometryContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( avLst ):          // CT_GeomGuideList adjust value list
            return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
        case A_TOKEN( gdLst ):          // CT_GeomGuideList guide list
            return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getGuideList() );
        case A_TOKEN( ahLst ):          // CT_AdjustHandleList adjust handle list
            return new AdjustHandleListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustHandleList() );
        case A_TOKEN( cxnLst ):         // CT_ConnectionSiteList connection site list
            return this;
        case A_TOKEN( rect ):           // CT_GeomRectList geometry rect list
        {
            GeomRect aGeomRect;
            aGeomRect.l = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_l ) );
            aGeomRect.t = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_t ) );
            aGeomRect.r = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_r ) );
            aGeomRect.b = GetAdjCoordinate( mrCustomShapeProperties, rAttribs.getStringDefaulted( XML_b ) );
            mrCustomShapeProperties.getTextRect() = aGeomRect;
        }
        break;
        case A_TOKEN( pathLst ):        // CT_Path2DList 2d path list
            return new Path2DListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getSegments(), mrCustomShapeProperties.getPath2DList() );

        // from cxnLst:
        case A_TOKEN( cxn ):                // CT_ConnectionSite
        {
            ConnectionSite aConnectionSite;
            mrCustomShapeProperties.getConnectionSiteList().push_back( aConnectionSite );
            return new ConnectionSiteContext( *this, rAttribs, mrCustomShapeProperties, mrCustomShapeProperties.getConnectionSiteList().back() );
        }
    }
    return nullptr;
}

// CT_PresetGeometry2D
PresetShapeGeometryContext::PresetShapeGeometryContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler2( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    sal_Int32 nShapeType = rAttribs.getToken( XML_prst, FastToken::DONTKNOW );
    OSL_ENSURE( nShapeType != FastToken::DONTKNOW, "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( nShapeType );
}

ContextHandlerRef PresetShapeGeometryContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& )
{
    if ( aElementToken == A_TOKEN( avLst ) )
        return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
    else
        return this;
}

// CT_PresetTextShape
PresetTextShapeContext::PresetTextShapeContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler2( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    sal_Int32 nShapeType = rAttribs.getToken( XML_prst, FastToken::DONTKNOW );
    OSL_ENSURE( nShapeType != FastToken::DONTKNOW, "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( nShapeType );
}

ContextHandlerRef PresetTextShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& )
{
    if ( aElementToken == A_TOKEN( avLst ) )
        return new GeomGuideListContext( *this, mrCustomShapeProperties, mrCustomShapeProperties.getAdjustmentGuideList() );
    else
        return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
