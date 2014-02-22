/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "svx/EnhancedCustomShape2d.hxx"
#include <rtl/ustring.hxx>
#include <tools/fract.hxx>






#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
#include <typeinfo>
#define BOOST_SPIRIT_DEBUG
#endif
#include <boost/spirit/include/classic_core.hpp>

#if (OSL_DEBUG_LEVEL > 0)
#include <iostream>
#endif
#include <functional>
#include <algorithm>
#include <stack>

#include <math.h>
using namespace EnhancedCustomShape;
using namespace com::sun::star;
using namespace com::sun::star::drawing;

void EnhancedCustomShape::FillEquationParameter( const EnhancedCustomShapeParameter& rSource, const sal_Int32 nDestPara, EnhancedCustomShapeEquation& rDest )
{
    sal_Int32 nValue = 0;
    if ( rSource.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rSource.Value >>= fValue )
            nValue = (sal_Int32)fValue;
    }
    else
        rSource.Value >>= nValue;

    switch( rSource.Type )
    {
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            if ( nValue & 0x40000000 )
            {
                nValue ^= 0x40000000;
                rDest.nOperation |= 0x20000000 << nDestPara;    
            }
            nValue |= 0x400;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT : nValue += DFF_Prop_adjustValue; break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM : nValue = DFF_Prop_geoBottom; break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT : nValue = DFF_Prop_geoRight; break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP : nValue = DFF_Prop_geoTop; break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT : nValue = DFF_Prop_geoLeft; break;
    }
    if ( rSource.Type != com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL )
        rDest.nOperation |= ( 0x2000 << nDestPara );
    rDest.nPara[ nDestPara ] = nValue;
}

ExpressionNode::~ExpressionNode()
{}

namespace
{






class ConstantValueExpression : public ExpressionNode
{
    double  maValue;

public:

    ConstantValueExpression( double rValue ) :
        maValue( rValue )
    {
    }
    virtual double operator()() const
    {
        return maValue;
    }
    virtual bool isConstant() const
    {
        return true;
    }
    virtual ExpressionFunct getType() const
    {
        return FUNC_CONST;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /* pOptionalArg */, sal_uInt32 /* nFlags */ )
    {
        EnhancedCustomShapeParameter aRet;
        Fraction aFract( maValue );
        if ( aFract.GetDenominator() == 1 )
        {
            aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            aRet.Value <<= (sal_Int32)aFract.GetNumerator();
        }
        else
        {
            EnhancedCustomShapeEquation aEquation;
            aEquation.nOperation = 1;
            aEquation.nPara[ 0 ] = 1;
            aEquation.nPara[ 1 ] = (sal_Int16)aFract.GetNumerator();
            aEquation.nPara[ 2 ] = (sal_Int16)aFract.GetDenominator();
            aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
            aRet.Value <<= (sal_Int32)rEquations.size();
            rEquations.push_back( aEquation );
        }
        return aRet;
    }
};

class AdjustmentExpression : public ExpressionNode
{
    sal_Int32                       mnIndex;
    const EnhancedCustomShape2d&    mrCustoShape;

public:

    AdjustmentExpression( const EnhancedCustomShape2d& rCustoShape, sal_Int32 nIndex )
    : mnIndex       ( nIndex )
    , mrCustoShape( rCustoShape )

    {
    }
    virtual double operator()() const
    {
        OSL_TRACE("  $%d --> %f (angle: %f)", mnIndex, mrCustoShape.GetAdjustValueAsDouble( mnIndex ), 180.0*mrCustoShape.GetAdjustValueAsDouble( mnIndex )/10800000.0);
        return mrCustoShape.GetAdjustValueAsDouble( mnIndex );
    }
    virtual bool isConstant() const
    {
        return false;
    }
    virtual ExpressionFunct getType() const
    {
        return ENUM_FUNC_ADJUSTMENT;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& /*rEquations*/, ExpressionNode* /*pOptionalArg*/, sal_uInt32 /*nFlags*/ )
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
        aRet.Value <<= mnIndex;
        return aRet;
    }
};

class EquationExpression : public ExpressionNode
{
    sal_Int32                       mnIndex;
    const EnhancedCustomShape2d&    mrCustoShape;

public:

    EquationExpression( const EnhancedCustomShape2d& rCustoShape, sal_Int32 nIndex )
        : mnIndex       ( nIndex )
        , mrCustoShape( rCustoShape )
    {
    }
    virtual double operator()() const
    {
        return mrCustoShape.GetEquationValueAsDouble( mnIndex );
    }
    virtual bool isConstant() const
    {
        return false;
    }
    virtual ExpressionFunct getType() const
    {
        return ENUM_FUNC_EQUATION;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& /*rEquations*/, ExpressionNode* /*pOptionalArg*/, sal_uInt32 /*nFlags*/ )
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
        aRet.Value <<= mnIndex | 0x40000000;                        
        return aRet;
    }
};

class EnumValueExpression : public ExpressionNode
{
    const ExpressionFunct           meFunct;
    const EnhancedCustomShape2d&    mrCustoShape;

public:

    EnumValueExpression( const EnhancedCustomShape2d& rCustoShape, const ExpressionFunct eFunct )
        : meFunct       ( eFunct )
        , mrCustoShape  ( rCustoShape )
    {
    }
    static double getValue( const EnhancedCustomShape2d& rCustoShape, const ExpressionFunct eFunc )
    {
        EnhancedCustomShape2d::EnumFunc eF;
        switch( eFunc )
        {
            case ENUM_FUNC_PI :         eF = EnhancedCustomShape2d::ENUM_FUNC_PI; break;
            case ENUM_FUNC_LEFT :       eF = EnhancedCustomShape2d::ENUM_FUNC_LEFT; break;
            case ENUM_FUNC_TOP :        eF = EnhancedCustomShape2d::ENUM_FUNC_TOP; break;
            case ENUM_FUNC_RIGHT :      eF = EnhancedCustomShape2d::ENUM_FUNC_RIGHT; break;
            case ENUM_FUNC_BOTTOM :     eF = EnhancedCustomShape2d::ENUM_FUNC_BOTTOM; break;
            case ENUM_FUNC_XSTRETCH :   eF = EnhancedCustomShape2d::ENUM_FUNC_XSTRETCH; break;
            case ENUM_FUNC_YSTRETCH :   eF = EnhancedCustomShape2d::ENUM_FUNC_YSTRETCH; break;
            case ENUM_FUNC_HASSTROKE :  eF = EnhancedCustomShape2d::ENUM_FUNC_HASSTROKE; break;
            case ENUM_FUNC_HASFILL :    eF = EnhancedCustomShape2d::ENUM_FUNC_HASFILL; break;
            case ENUM_FUNC_WIDTH :      eF = EnhancedCustomShape2d::ENUM_FUNC_WIDTH; break;
            case ENUM_FUNC_HEIGHT :     eF = EnhancedCustomShape2d::ENUM_FUNC_HEIGHT; break;
            case ENUM_FUNC_LOGWIDTH :   eF = EnhancedCustomShape2d::ENUM_FUNC_LOGWIDTH; break;
            case ENUM_FUNC_LOGHEIGHT :  eF = EnhancedCustomShape2d::ENUM_FUNC_LOGHEIGHT; break;

            default :
                return 0.0;
        }
        return rCustoShape.GetEnumFunc( eF );
    }
    virtual double operator()() const
    {
#if OSL_DEBUG_LEVEL > 1
        const char *funcName;

        switch (meFunct) {
            case ENUM_FUNC_PI :         funcName = "pi"; break;
            case ENUM_FUNC_LEFT :       funcName = "left"; break;
            case ENUM_FUNC_TOP :        funcName = "top"; break;
            case ENUM_FUNC_RIGHT :      funcName = "right"; break;
            case ENUM_FUNC_BOTTOM :     funcName = "bottom"; break;
            case ENUM_FUNC_XSTRETCH :   funcName = "xstretch"; break;
            case ENUM_FUNC_YSTRETCH :   funcName = "ystretch"; break;
            case ENUM_FUNC_HASSTROKE :  funcName = "hasstroke"; break;
            case ENUM_FUNC_HASFILL :    funcName = "hasfill"; break;
            case ENUM_FUNC_WIDTH :      funcName = "width"; break;
            case ENUM_FUNC_HEIGHT :     funcName = "height"; break;
            case ENUM_FUNC_LOGWIDTH :   funcName = "logwidth"; break;
            case ENUM_FUNC_LOGHEIGHT :  funcName = "logheight"; break;
            default:                    funcName = "???"; break;
        }

        OSL_TRACE("  %s --> %f (angle: %f)", funcName, getValue( mrCustoShape, meFunct ), 180.0*getValue( mrCustoShape, meFunct )/10800000.0);
#endif

        return getValue( mrCustoShape, meFunct );
    }
    virtual bool isConstant() const
    {
        return false;
    }
    virtual ExpressionFunct getType() const
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags )
    {
        EnhancedCustomShapeParameter aRet;

        sal_Int32 nDummy = 1;
        aRet.Value <<= nDummy;

        switch( meFunct )
        {
            case ENUM_FUNC_WIDTH :  
            case ENUM_FUNC_HEIGHT :
            case ENUM_FUNC_LOGWIDTH :
            case ENUM_FUNC_LOGHEIGHT :
            case ENUM_FUNC_PI :
            {
                ConstantValueExpression aConstantValue( getValue( mrCustoShape, meFunct ) );
                aRet = aConstantValue.fillNode( rEquations, NULL, nFlags );
            }
            break;
            case ENUM_FUNC_LEFT :   aRet.Type = EnhancedCustomShapeParameterType::LEFT; break;
            case ENUM_FUNC_TOP :    aRet.Type = EnhancedCustomShapeParameterType::TOP; break;
            case ENUM_FUNC_RIGHT :  aRet.Type = EnhancedCustomShapeParameterType::RIGHT; break;
            case ENUM_FUNC_BOTTOM : aRet.Type = EnhancedCustomShapeParameterType::BOTTOM; break;

            
            case ENUM_FUNC_XSTRETCH :
            case ENUM_FUNC_YSTRETCH :
            case ENUM_FUNC_HASSTROKE :
            case ENUM_FUNC_HASFILL : aRet.Type = EnhancedCustomShapeParameterType::NORMAL; break;

            default:
                break;
        }
        return aRet;
    }
};

/** ExpressionNode implementation for unary
    function over one ExpressionNode
    */
class UnaryFunctionExpression : public ExpressionNode
{
    const ExpressionFunct   meFunct;
    ExpressionNodeSharedPtr mpArg;

public:
    UnaryFunctionExpression( const ExpressionFunct eFunct, const ExpressionNodeSharedPtr& rArg ) :
        meFunct( eFunct ),
        mpArg( rArg )
    {
    }
    static double getValue( const ExpressionFunct eFunct, const ExpressionNodeSharedPtr& rArg )
    {
        double fRet = 0;
        switch( eFunct )
        {
            case UNARY_FUNC_ABS : fRet = fabs( (*rArg)() ); break;
            case UNARY_FUNC_SQRT: fRet = sqrt( (*rArg)() ); break;
            case UNARY_FUNC_SIN : fRet = sin( (*rArg)() );  break;
            case UNARY_FUNC_COS : fRet = cos( (*rArg)() );  break;
            case UNARY_FUNC_TAN : fRet = tan( (*rArg)() );  break;
            case UNARY_FUNC_ATAN: fRet = atan( (*rArg)() ); break;
            case UNARY_FUNC_NEG : fRet = ::std::negate<double>()( (*rArg)() ); break;
            default:
                break;
        }
        return fRet;
    }
    virtual double operator()() const
    {
        return getValue( meFunct, mpArg );
    }
    virtual bool isConstant() const
    {
        return mpArg->isConstant();
    }
    virtual ExpressionFunct getType() const
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* pOptionalArg, sal_uInt32 nFlags )
    {
        EnhancedCustomShapeParameter aRet;
        switch( meFunct )
        {
            case UNARY_FUNC_ABS :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 3;
                FillEquationParameter( mpArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case UNARY_FUNC_SQRT:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 13;
                FillEquationParameter( mpArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case UNARY_FUNC_SIN :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 9;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, NULL, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   
                    EnhancedCustomShapeEquation _aEquation;
                    _aEquation.nOperation |= 0xe;   
                    FillEquationParameter( aSource, 1, _aEquation );
                    aSource.Type = EnhancedCustomShapeParameterType::EQUATION;
                    aSource.Value <<= (sal_Int32)rEquations.size();
                    rEquations.push_back( _aEquation );
                }
                FillEquationParameter( aSource, 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case UNARY_FUNC_COS :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 10;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, NULL, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   
                    EnhancedCustomShapeEquation aTmpEquation;
                    aTmpEquation.nOperation |= 0xe; 
                    FillEquationParameter( aSource, 1, aTmpEquation );
                    aSource.Type = EnhancedCustomShapeParameterType::EQUATION;
                    aSource.Value <<= (sal_Int32)rEquations.size();
                    rEquations.push_back( aTmpEquation );
                }
                FillEquationParameter( aSource, 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case UNARY_FUNC_TAN :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 16;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, NULL, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   
                    EnhancedCustomShapeEquation aTmpEquation;
                    aTmpEquation.nOperation |= 0xe; 
                    FillEquationParameter( aSource, 1, aTmpEquation );
                    aSource.Type = EnhancedCustomShapeParameterType::EQUATION;
                    aSource.Value <<= (sal_Int32)rEquations.size();
                    rEquations.push_back( aTmpEquation );
                }
                FillEquationParameter( aSource, 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case UNARY_FUNC_ATAN:
            {

                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
            break;
            case UNARY_FUNC_NEG:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 1;
                aEquation.nPara[ 1 ] = -1;
                aEquation.nPara[ 2 ] = 1;
                FillEquationParameter( mpArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            default:
                break;
        }
        return aRet;
    }
};

/** ExpressionNode implementation for unary
    function over two ExpressionNodes
    */
class BinaryFunctionExpression : public ExpressionNode
{
    const ExpressionFunct   meFunct;
    ExpressionNodeSharedPtr mpFirstArg;
    ExpressionNodeSharedPtr mpSecondArg;

public:

    BinaryFunctionExpression( const ExpressionFunct eFunct, const ExpressionNodeSharedPtr& rFirstArg, const ExpressionNodeSharedPtr& rSecondArg ) :
        meFunct( eFunct ),
        mpFirstArg( rFirstArg ),
        mpSecondArg( rSecondArg )
    {
    }
    static double getValue( const ExpressionFunct eFunct, const ExpressionNodeSharedPtr& rFirstArg, const ExpressionNodeSharedPtr& rSecondArg )
    {
        double fRet = 0;
        switch( eFunct )
        {
            case BINARY_FUNC_PLUS : fRet = (*rFirstArg)() + (*rSecondArg)(); break;
            case BINARY_FUNC_MINUS: fRet = (*rFirstArg)() - (*rSecondArg)(); break;
            case BINARY_FUNC_MUL :  fRet = (*rFirstArg)() * (*rSecondArg)(); break;
            case BINARY_FUNC_DIV :  fRet = (*rFirstArg)() / (*rSecondArg)(); break;
            case BINARY_FUNC_MIN :  fRet = ::std::min( (*rFirstArg)(), (*rSecondArg)() ); break;
            case BINARY_FUNC_MAX :  fRet = ::std::max( (*rFirstArg)(), (*rSecondArg)() ); break;
            case BINARY_FUNC_ATAN2: fRet = atan2( (*rFirstArg)(), (*rSecondArg)() ); break;
            default:
                break;
        }
        return fRet;
    }
    virtual double operator()() const
    {
        return getValue( meFunct, mpFirstArg, mpSecondArg );
    }
    virtual bool isConstant() const
    {
        return mpFirstArg->isConstant() && mpSecondArg->isConstant();
    }
    virtual ExpressionFunct getType() const
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags )
    {
        EnhancedCustomShapeParameter aRet;
        switch( meFunct )
        {
            case BINARY_FUNC_PLUS :
            {
                if ( nFlags & EXPRESSION_FLAG_SUMANGLE_MODE )
                {
                    if ( mpFirstArg->getType() == ENUM_FUNC_ADJUSTMENT )
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0xe;    
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                    else if ( mpSecondArg->getType() == ENUM_FUNC_ADJUSTMENT )
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0xe;    
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                    else
                    {
                        EnhancedCustomShapeEquation aSumangle1;
                        aSumangle1.nOperation |= 0xe;   
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags &~EXPRESSION_FLAG_SUMANGLE_MODE ), 1, aSumangle1 );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aSumangle1 );

                        EnhancedCustomShapeEquation aSumangle2;
                        aSumangle2.nOperation |= 0xe;   
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags &~EXPRESSION_FLAG_SUMANGLE_MODE ), 1, aSumangle2 );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aSumangle2 );

                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0;
                        aEquation.nPara[ 0 ] = ( rEquations.size() - 2 ) | 0x400;
                        aEquation.nPara[ 1 ] = ( rEquations.size() - 1 ) | 0x400;
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                }
                else
                {
                    bool bFirstIsEmpty = mpFirstArg->isConstant() && ( (*mpFirstArg)() == 0 );
                    bool bSecondIsEmpty = mpSecondArg->isConstant() && ( (*mpSecondArg)() == 0 );

                    if ( bFirstIsEmpty )
                        aRet = mpSecondArg->fillNode( rEquations, NULL, nFlags );
                    else if ( bSecondIsEmpty )
                        aRet = mpFirstArg->fillNode( rEquations, NULL, nFlags );
                    else
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0;
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                }
            }
            break;
            case BINARY_FUNC_MINUS:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 0;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 2, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case BINARY_FUNC_MUL :
            {
                
                
                if ( ( mpFirstArg->getType() == UNARY_FUNC_SIN ) || ( mpFirstArg->getType() == UNARY_FUNC_COS ) || ( mpFirstArg->getType() == UNARY_FUNC_TAN ) )
                    aRet = mpFirstArg->fillNode( rEquations, mpSecondArg.get(), nFlags );
                else if ( ( mpSecondArg->getType() == UNARY_FUNC_SIN ) || ( mpSecondArg->getType() == UNARY_FUNC_COS ) || ( mpSecondArg->getType() == UNARY_FUNC_TAN ) )
                    aRet = mpSecondArg->fillNode( rEquations, mpFirstArg.get(), nFlags );
                else
                {
                    if ( mpFirstArg->isConstant() && (*mpFirstArg)() == 1 )
                        aRet = mpSecondArg->fillNode( rEquations, NULL, nFlags );
                    else if ( mpSecondArg->isConstant() && (*mpSecondArg)() == 1 )
                        aRet = mpFirstArg->fillNode( rEquations, NULL, nFlags );
                    else if ( ( mpFirstArg->getType() == BINARY_FUNC_DIV )      
                        && ( ((BinaryFunctionExpression*)((BinaryFunctionExpression*)mpFirstArg.get())->mpFirstArg.get())->getType() == ENUM_FUNC_PI )
                        && ( ((BinaryFunctionExpression*)((BinaryFunctionExpression*)mpFirstArg.get())->mpSecondArg.get())->getType() == FUNC_CONST ) )
                    {
                        aRet = mpSecondArg->fillNode( rEquations, NULL, nFlags );
                    }
                    else if ( ( mpSecondArg->getType() == BINARY_FUNC_DIV )     
                        && ( ((BinaryFunctionExpression*)((BinaryFunctionExpression*)mpSecondArg.get())->mpFirstArg.get())->getType() == ENUM_FUNC_PI )
                        && ( ((BinaryFunctionExpression*)((BinaryFunctionExpression*)mpSecondArg.get())->mpSecondArg.get())->getType() == FUNC_CONST ) )
                    {
                        aRet = mpFirstArg->fillNode( rEquations, NULL, nFlags );
                    }
                    else
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 1;
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                        aEquation.nPara[ 2 ] = 1;
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                }
            }
            break;
            case BINARY_FUNC_DIV :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 1;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                aEquation.nPara[ 1 ] = 1;
                FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 2, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case BINARY_FUNC_MIN :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 4;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case BINARY_FUNC_MAX :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 5;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case BINARY_FUNC_ATAN2:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 8;
                FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
                FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            default:
                break;
        }
        return aRet;
    }
};

class IfExpression : public ExpressionNode
{
    ExpressionNodeSharedPtr mpFirstArg;
    ExpressionNodeSharedPtr mpSecondArg;
    ExpressionNodeSharedPtr mpThirdArg;

public:

    IfExpression( const ExpressionNodeSharedPtr& rFirstArg,
                  const ExpressionNodeSharedPtr& rSecondArg,
                  const ExpressionNodeSharedPtr& rThirdArg ) :
        mpFirstArg(  rFirstArg ),
        mpSecondArg( rSecondArg ),
        mpThirdArg(  rThirdArg )
    {
    }
    virtual bool isConstant() const
    {
        return
            mpFirstArg->isConstant() &&
            mpSecondArg->isConstant() &&
            mpThirdArg->isConstant();
    }
    virtual double operator()() const
    {
        return (*mpFirstArg)() > 0 ? (*mpSecondArg)() : (*mpThirdArg)();
    }
    virtual ExpressionFunct getType() const
    {
        return TERNARY_FUNC_IF;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags )
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
        aRet.Value <<= (sal_Int32)rEquations.size();
        {
            EnhancedCustomShapeEquation aEquation;
            aEquation.nOperation |= 6;
            FillEquationParameter( mpFirstArg->fillNode( rEquations, NULL, nFlags ), 0, aEquation );
            FillEquationParameter( mpSecondArg->fillNode( rEquations, NULL, nFlags  ), 1, aEquation );
            FillEquationParameter( mpThirdArg->fillNode( rEquations, NULL, nFlags ), 2, aEquation );
            rEquations.push_back( aEquation );
        }
        return aRet;
    }
};







typedef const sal_Char* StringIteratorT;

struct ParserContext
{
    typedef ::std::stack< ExpressionNodeSharedPtr > OperandStack;

    
    
    
    
    
    OperandStack                maOperandStack;

    const EnhancedCustomShape2d* mpCustoShape;

};

typedef ::boost::shared_ptr< ParserContext > ParserContextSharedPtr;

/** Generate parse-dependent-but-then-constant value
    */
class DoubleConstantFunctor
{
    ParserContextSharedPtr  mpContext;

public:
    DoubleConstantFunctor( const ParserContextSharedPtr& rContext ) :
        mpContext( rContext )
    {
    }
    void operator()( double n ) const
    {
        mpContext->maOperandStack.push( ExpressionNodeSharedPtr( new ConstantValueExpression( n ) ) );
    }
};

class EnumFunctor
{
    const ExpressionFunct           meFunct;
    double                          mnValue;
    ParserContextSharedPtr          mpContext;

public:

    EnumFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext )
    : meFunct( eFunct )
    , mnValue( 0 )
    , mpContext( rContext )
    {
    }
    void operator()( StringIteratorT rFirst, StringIteratorT rSecond ) const
    {
        /*double nVal = mnValue;*/
        switch( meFunct )
        {
            case ENUM_FUNC_ADJUSTMENT :
            {
                OUString aVal( rFirst + 1, rSecond - rFirst, RTL_TEXTENCODING_UTF8 );
                mpContext->maOperandStack.push( ExpressionNodeSharedPtr( new AdjustmentExpression( *mpContext->mpCustoShape, aVal.toInt32() ) ) );
            }
            break;
            case ENUM_FUNC_EQUATION :
                {
                OUString aVal( rFirst + 1, rSecond - rFirst, RTL_TEXTENCODING_UTF8 );
                mpContext->maOperandStack.push( ExpressionNodeSharedPtr( new EquationExpression( *mpContext->mpCustoShape, aVal.toInt32() ) ) );
            }
            break;
            default:
                mpContext->maOperandStack.push( ExpressionNodeSharedPtr( new EnumValueExpression( *mpContext->mpCustoShape, meFunct ) ) );
        }
    }
};

class UnaryFunctionFunctor
{
    const ExpressionFunct   meFunct;
    ParserContextSharedPtr  mpContext;

public :

    UnaryFunctionFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext ) :
        meFunct( eFunct ),
        mpContext( rContext )
    {
    }
    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

        if( rNodeStack.size() < 1 )
            throw ParseError( "Not enough arguments for unary operator" );

        
        ExpressionNodeSharedPtr pArg( rNodeStack.top() );
        rNodeStack.pop();

        if( pArg->isConstant() )    
            rNodeStack.push( ExpressionNodeSharedPtr( new ConstantValueExpression( UnaryFunctionExpression::getValue( meFunct, pArg ) ) ) );
        else                        
            rNodeStack.push( ExpressionNodeSharedPtr( new UnaryFunctionExpression( meFunct, pArg ) ) );
    }
};

/** Implements a binary function over two ExpressionNodes

    @tpl Generator
    Generator functor, to generate an ExpressionNode of
    appropriate type

    */
class BinaryFunctionFunctor
{
    const ExpressionFunct   meFunct;
    ParserContextSharedPtr  mpContext;

public:

    BinaryFunctionFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext ) :
        meFunct( eFunct ),
        mpContext( rContext )
    {
    }

    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

        if( rNodeStack.size() < 2 )
            throw ParseError( "Not enough arguments for binary operator" );

        
        ExpressionNodeSharedPtr pSecondArg( rNodeStack.top() );
        rNodeStack.pop();
        ExpressionNodeSharedPtr pFirstArg( rNodeStack.top() );
        rNodeStack.pop();

        
        ExpressionNodeSharedPtr pNode = ExpressionNodeSharedPtr( new BinaryFunctionExpression( meFunct, pFirstArg, pSecondArg ) );
        
        if( pFirstArg->isConstant() && pSecondArg->isConstant() )   
            rNodeStack.push( ExpressionNodeSharedPtr( new ConstantValueExpression( (*pNode)() ) ) );
        else                                                        
            rNodeStack.push( pNode );
    }
};

class IfFunctor
{
    ParserContextSharedPtr  mpContext;

public :

    IfFunctor( const ParserContextSharedPtr& rContext ) :
        mpContext( rContext )
    {
    }
    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

        if( rNodeStack.size() < 3 )
            throw ParseError( "Not enough arguments for ternary operator" );

        
        ExpressionNodeSharedPtr pThirdArg( rNodeStack.top() );
        rNodeStack.pop();
        ExpressionNodeSharedPtr pSecondArg( rNodeStack.top() );
        rNodeStack.pop();
        ExpressionNodeSharedPtr pFirstArg( rNodeStack.top() );
        rNodeStack.pop();

        
        ExpressionNodeSharedPtr pNode( new IfExpression( pFirstArg, pSecondArg, pThirdArg ) );
        
        if( pFirstArg->isConstant() && pSecondArg->isConstant() && pThirdArg->isConstant() )
            rNodeStack.push( ExpressionNodeSharedPtr( new ConstantValueExpression( (*pNode)() ) ) );    
        else
            rNodeStack.push( pNode );                                       
    }
};


//





//


//

template< typename T > struct custom_real_parser_policies : public ::boost::spirit::ureal_parser_policies<T>
{
    template< typename ScannerT >
        static typename ::boost::spirit::parser_result< ::boost::spirit::chlit<>, ScannerT >::type
    parse_exp(ScannerT& scan)
    {
        
        return ::boost::spirit::ch_p('E').parse(scan);
    }
};

/* This class implements the following grammar (more or
    less literally written down below, only slightly
    obfuscated by the parser actions):

    identifier = '$'|'pi'|'e'|'X'|'Y'|'Width'|'Height'

    function = 'abs'|'sqrt'|'sin'|'cos'|'tan'|'atan'|'acos'|'asin'|'exp'|'log'

    basic_expression =
                       number |
                       identifier |
                       function '(' additive_expression ')' |
                       '(' additive_expression ')'

    unary_expression =
                       '-' basic_expression |
                    basic_expression

    multiplicative_expression =
                       unary_expression ( ( '*' unary_expression )* |
                                        ( '/' unary_expression )* )

    additive_expression =
                       multiplicative_expression ( ( '+' multiplicative_expression )* |
                                                   ( '-' multiplicative_expression )* )

    */

class ExpressionGrammar : public ::boost::spirit::grammar< ExpressionGrammar >
{
public:
    /** Create an arithmetic expression grammar

        @param rParserContext
        Contains context info for the parser
        */
    ExpressionGrammar( const ParserContextSharedPtr& rParserContext ) :
        mpParserContext( rParserContext )
    {
    }

    template< typename ScannerT > class definition
    {
    public:
        
        definition( const ExpressionGrammar& self )
        {
            using ::boost::spirit::str_p;
            using ::boost::spirit::range_p;
            using ::boost::spirit::lexeme_d;
            using ::boost::spirit::real_parser;
            using ::boost::spirit::chseq_p;

            identifier =
                            str_p( "pi"         )[ EnumFunctor(ENUM_FUNC_PI,        self.getContext() ) ]
                    |       str_p( "left"       )[ EnumFunctor(ENUM_FUNC_LEFT,      self.getContext() ) ]
                    |       str_p( "top"        )[ EnumFunctor(ENUM_FUNC_TOP,       self.getContext() ) ]
                    |       str_p( "right"      )[ EnumFunctor(ENUM_FUNC_RIGHT,     self.getContext() ) ]
                    |       str_p( "bottom"     )[ EnumFunctor(ENUM_FUNC_BOTTOM,    self.getContext() ) ]
                    |       str_p( "xstretch"   )[ EnumFunctor(ENUM_FUNC_XSTRETCH,  self.getContext() ) ]
                    |       str_p( "ystretch"   )[ EnumFunctor(ENUM_FUNC_YSTRETCH,  self.getContext() ) ]
                    |       str_p( "hasstroke"  )[ EnumFunctor(ENUM_FUNC_HASSTROKE, self.getContext() ) ]
                    |       str_p( "hasfill"    )[ EnumFunctor(ENUM_FUNC_HASFILL,   self.getContext() ) ]
                    |       str_p( "width"      )[ EnumFunctor(ENUM_FUNC_WIDTH,     self.getContext() ) ]
                    |       str_p( "height"     )[ EnumFunctor(ENUM_FUNC_HEIGHT,    self.getContext() ) ]
                    |       str_p( "logwidth"   )[ EnumFunctor(ENUM_FUNC_LOGWIDTH,  self.getContext() ) ]
                    |       str_p( "logheight"  )[ EnumFunctor(ENUM_FUNC_LOGHEIGHT, self.getContext() ) ]
                    ;

            unaryFunction =
                    (str_p( "abs"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_ABS,  self.getContext()) ]
                |   (str_p( "sqrt" ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_SQRT, self.getContext()) ]
                |   (str_p( "sin"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_SIN,  self.getContext()) ]
                |   (str_p( "cos"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_COS,  self.getContext()) ]
                |   (str_p( "tan"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_TAN,  self.getContext()) ]
                |   (str_p( "atan" ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( UNARY_FUNC_ATAN, self.getContext()) ]
                ;

            binaryFunction =
                    (str_p( "min"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( BINARY_FUNC_MIN,  self.getContext()) ]
                |   (str_p( "max"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( BINARY_FUNC_MAX,  self.getContext()) ]
                |   (str_p( "atan2") >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( BINARY_FUNC_ATAN2,self.getContext()) ]
                ;

            ternaryFunction =
                    (str_p( "if"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ IfFunctor( self.getContext() ) ]
                ;

            funcRef_decl =
                lexeme_d[ +( range_p('a','z') | range_p('A','Z') | range_p('0','9') ) ];

            functionReference =
                (str_p( "?" ) >> funcRef_decl )[ EnumFunctor( ENUM_FUNC_EQUATION, self.getContext() ) ];

            modRef_decl =
                lexeme_d[ +( range_p('0','9') ) ];

            modifierReference =
                (str_p( "$" ) >> modRef_decl )[ EnumFunctor( ENUM_FUNC_ADJUSTMENT, self.getContext() ) ];

            basicExpression =
                    real_parser<double, custom_real_parser_policies<double> >()[ DoubleConstantFunctor(self.getContext()) ]
                |   identifier
                |   functionReference
                |   modifierReference
                |   unaryFunction
                |   binaryFunction
                |   ternaryFunction
                |   '(' >> additiveExpression >> ')'
                ;

            unaryExpression =
                    ('-' >> basicExpression)[ UnaryFunctionFunctor( UNARY_FUNC_NEG, self.getContext()) ]
                |   basicExpression
                ;

            multiplicativeExpression =
                    unaryExpression
                >> *( ('*' >> unaryExpression)[ BinaryFunctionFunctor( BINARY_FUNC_MUL, self.getContext()) ]
                    | ('/' >> unaryExpression)[ BinaryFunctionFunctor( BINARY_FUNC_DIV, self.getContext()) ]
                    )
                ;

            additiveExpression =
                    multiplicativeExpression
                >> *( ('+' >> multiplicativeExpression)[ BinaryFunctionFunctor( BINARY_FUNC_PLUS,  self.getContext()) ]
                    | ('-' >> multiplicativeExpression)[ BinaryFunctionFunctor( BINARY_FUNC_MINUS, self.getContext()) ]
                    )
                ;

            BOOST_SPIRIT_DEBUG_RULE(additiveExpression);
            BOOST_SPIRIT_DEBUG_RULE(multiplicativeExpression);
            BOOST_SPIRIT_DEBUG_RULE(unaryExpression);
            BOOST_SPIRIT_DEBUG_RULE(basicExpression);
            BOOST_SPIRIT_DEBUG_RULE(unaryFunction);
            BOOST_SPIRIT_DEBUG_RULE(binaryFunction);
            BOOST_SPIRIT_DEBUG_RULE(ternaryFunction);
            BOOST_SPIRIT_DEBUG_RULE(identifier);
        }

        const ::boost::spirit::rule< ScannerT >& start() const
        {
            return additiveExpression;
        }

    private:
        
        
        ::boost::spirit::rule< ScannerT >   additiveExpression;
        ::boost::spirit::rule< ScannerT >   multiplicativeExpression;
        ::boost::spirit::rule< ScannerT >   unaryExpression;
        ::boost::spirit::rule< ScannerT >   basicExpression;
        ::boost::spirit::rule< ScannerT >   unaryFunction;
        ::boost::spirit::rule< ScannerT >   binaryFunction;
        ::boost::spirit::rule< ScannerT >   ternaryFunction;
        ::boost::spirit::rule< ScannerT >   funcRef_decl;
        ::boost::spirit::rule< ScannerT >   functionReference;
        ::boost::spirit::rule< ScannerT >   modRef_decl;
        ::boost::spirit::rule< ScannerT >   modifierReference;
        ::boost::spirit::rule< ScannerT >   identifier;
    };

    const ParserContextSharedPtr& getContext() const
    {
        return mpParserContext;
    }

private:
    ParserContextSharedPtr          mpParserContext; 
};

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
const ParserContextSharedPtr& getParserContext()
{
    static ParserContextSharedPtr lcl_parserContext( new ParserContext() );

    
    
    while( !lcl_parserContext->maOperandStack.empty() )
        lcl_parserContext->maOperandStack.pop();

    return lcl_parserContext;
}
#endif

}

namespace EnhancedCustomShape  {



ExpressionNodeSharedPtr FunctionParser::parseFunction( const OUString& rFunction, const EnhancedCustomShape2d& rCustoShape )
{
    
    
    
    const OString& rAsciiFunction(
        OUStringToOString( rFunction, RTL_TEXTENCODING_ASCII_US ) );

    StringIteratorT aStart( rAsciiFunction.getStr() );
    StringIteratorT aEnd( rAsciiFunction.getStr()+rAsciiFunction.getLength() );

    ParserContextSharedPtr pContext;

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
    
    
    pContext = getParserContext();
#else
    pContext.reset( new ParserContext() );
#endif
    pContext->mpCustoShape = &rCustoShape;

    ExpressionGrammar aExpressionGrammer( pContext );
    const ::boost::spirit::parse_info<StringIteratorT> aParseInfo(
            ::boost::spirit::parse( aStart,
                                    aEnd,
                                    aExpressionGrammer >> ::boost::spirit::end_p,
                                    ::boost::spirit::space_p ) );
    OSL_DEBUG_ONLY(::std::cout.flush()); 



    
    if( !aParseInfo.full )
        throw ParseError( "EnhancedCustomShapeFunctionParser::parseFunction(): string not fully parseable" );

    
    
    if( pContext->maOperandStack.size() != 1 )
        throw ParseError( "EnhancedCustomShapeFunctionParser::parseFunction(): incomplete or empty expression" );


    return pContext->maOperandStack.top();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
