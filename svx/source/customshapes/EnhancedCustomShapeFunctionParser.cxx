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

#include <sal/config.h>

#include <config_clang.h>
#include <svx/EnhancedCustomShape2d.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <tools/fract.hxx>

// Makes parser a static resource,
// we're synchronized externally.
// But watch out, the parser might have
// state not visible to this code!

#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
#define BOOST_SPIRIT_DEBUG
#endif
#include <boost/spirit/include/classic_core.hpp>

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
        case css::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            if ( nValue & 0x40000000 )
            {
                nValue ^= 0x40000000;
                rDest.nOperation |= 0x20000000 << nDestPara;    // the bit is indicating that this value has to be adjusted later
            }
            nValue |= 0x400;
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT : nValue += DFF_Prop_adjustValue; break;
        case css::drawing::EnhancedCustomShapeParameterType::BOTTOM : nValue = DFF_Prop_geoBottom; break;
        case css::drawing::EnhancedCustomShapeParameterType::RIGHT : nValue = DFF_Prop_geoRight; break;
        case css::drawing::EnhancedCustomShapeParameterType::TOP : nValue = DFF_Prop_geoTop; break;
        case css::drawing::EnhancedCustomShapeParameterType::LEFT : nValue = DFF_Prop_geoLeft; break;
    }
    if ( rSource.Type != css::drawing::EnhancedCustomShapeParameterType::NORMAL )
        rDest.nOperation |= ( 0x2000 << nDestPara );
    rDest.nPara[ nDestPara ] = nValue;
}

ExpressionNode::~ExpressionNode()
{}

namespace
{


// EXPRESSION NODES


class ConstantValueExpression : public ExpressionNode
{
    double  maValue;

public:

    explicit ConstantValueExpression( double rValue ) :
        maValue( rValue )
    {
    }
    virtual double operator()() const override
    {
        return maValue;
    }
    virtual bool isConstant() const override
    {
        return true;
    }
    virtual ExpressionFunct getType() const override
    {
        return ExpressionFunct::Const;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /* pOptionalArg */, sal_uInt32 /* nFlags */ ) override
    {
        EnhancedCustomShapeParameter aRet;
        Fraction aFract( maValue );
        if ( aFract.GetDenominator() == 1 )
        {
            aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            aRet.Value <<= aFract.GetNumerator();
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
    virtual double operator()() const override
    {
        SAL_INFO(
            "svx",
            "$" << mnIndex << " --> "
                << mrCustoShape.GetAdjustValueAsDouble(mnIndex) << " (angle: "
                << 180.0*mrCustoShape.GetAdjustValueAsDouble(mnIndex)/10800000.0
                << ")");
        return mrCustoShape.GetAdjustValueAsDouble( mnIndex );
    }
    virtual bool isConstant() const override
    {
        return false;
    }
    virtual ExpressionFunct getType() const override
    {
        return ExpressionFunct::EnumAdjustment;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& /*rEquations*/, ExpressionNode* /*pOptionalArg*/, sal_uInt32 /*nFlags*/ ) override
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
        aRet.Value <<= mnIndex;
        return aRet;
    }
};

class EquationExpression : public ExpressionNode
{
    const sal_Int32                 mnIndex;
    const EnhancedCustomShape2d&    mrCustoShape;
    mutable bool                    mbGettingValueGuard;

public:

    EquationExpression( const EnhancedCustomShape2d& rCustoShape, sal_Int32 nIndex )
        : mnIndex       ( nIndex )
        , mrCustoShape( rCustoShape )
        , mbGettingValueGuard(false)
    {
    }
    virtual double operator()() const override
    {
        if (mbGettingValueGuard)
            throw ParseError("Loop in Expression");
        mbGettingValueGuard = true;
        double fRet = mrCustoShape.GetEquationValueAsDouble(mnIndex);
        mbGettingValueGuard = false;
        return fRet;
    }
    virtual bool isConstant() const override
    {
        return false;
    }
    virtual ExpressionFunct getType() const override
    {
        return ExpressionFunct::EnumEquation;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& /*rEquations*/, ExpressionNode* /*pOptionalArg*/, sal_uInt32 /*nFlags*/ ) override
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
        aRet.Value <<= mnIndex | 0x40000000;                        // the bit is indicating that this equation needs to be adjusted later
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
    virtual double operator()() const override
    {
        SAL_INFO("svx", meFunct << " --> " << mrCustoShape.GetEnumFunc(meFunct) << "(angle: " <<
                 180.0 * mrCustoShape.GetEnumFunc(meFunct) / 10800000.0 << ")");

        return mrCustoShape.GetEnumFunc( meFunct );
    }
    virtual bool isConstant() const override
    {
        return false;
    }
    virtual ExpressionFunct getType() const override
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags ) override
    {
        EnhancedCustomShapeParameter aRet;

        aRet.Value <<= sal_Int32(1);

        switch( meFunct )
        {
            case ExpressionFunct::EnumWidth :  // TODO: do not use this as constant value
            case ExpressionFunct::EnumHeight :
            case ExpressionFunct::EnumLogWidth :
            case ExpressionFunct::EnumLogHeight :
            case ExpressionFunct::EnumPi :
            {
                ConstantValueExpression aConstantValue( mrCustoShape.GetEnumFunc( meFunct ) );
                aRet = aConstantValue.fillNode( rEquations, nullptr, nFlags );
            }
            break;
            case ExpressionFunct::EnumLeft :   aRet.Type = EnhancedCustomShapeParameterType::LEFT; break;
            case ExpressionFunct::EnumTop :    aRet.Type = EnhancedCustomShapeParameterType::TOP; break;
            case ExpressionFunct::EnumRight :  aRet.Type = EnhancedCustomShapeParameterType::RIGHT; break;
            case ExpressionFunct::EnumBottom : aRet.Type = EnhancedCustomShapeParameterType::BOTTOM; break;

            // not implemented so far
            case ExpressionFunct::EnumXStretch :
            case ExpressionFunct::EnumYStretch :
            case ExpressionFunct::EnumHasStroke :
            case ExpressionFunct::EnumHasFill : aRet.Type = EnhancedCustomShapeParameterType::NORMAL; break;

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
    std::shared_ptr<ExpressionNode> mpArg;

public:
    UnaryFunctionExpression( const ExpressionFunct eFunct, const std::shared_ptr<ExpressionNode>& rArg ) :
        meFunct( eFunct ),
        mpArg( rArg )
    {
    }
    static double getValue( const ExpressionFunct eFunct, const std::shared_ptr<ExpressionNode>& rArg )
    {
        double fRet = 0;
        switch( eFunct )
        {
            case ExpressionFunct::UnaryAbs : fRet = fabs( (*rArg)() ); break;
            case ExpressionFunct::UnarySqrt: fRet = sqrt( (*rArg)() ); break;
            case ExpressionFunct::UnarySin : fRet = sin( (*rArg)() );  break;
            case ExpressionFunct::UnaryCos : fRet = cos( (*rArg)() );  break;
            case ExpressionFunct::UnaryTan : fRet = tan( (*rArg)() );  break;
            case ExpressionFunct::UnaryAtan: fRet = atan( (*rArg)() ); break;
            case ExpressionFunct::UnaryNeg : fRet = ::std::negate<double>()( (*rArg)() ); break;
            default:
                break;
        }
        return fRet;
    }
    virtual double operator()() const override
    {
        return getValue( meFunct, mpArg );
    }
    virtual bool isConstant() const override
    {
        return mpArg->isConstant();
    }
    virtual ExpressionFunct getType() const override
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* pOptionalArg, sal_uInt32 nFlags ) override
    {
        EnhancedCustomShapeParameter aRet;
        switch( meFunct )
        {
            case ExpressionFunct::UnaryAbs :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 3;
                FillEquationParameter( mpArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::UnarySqrt:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 13;
                FillEquationParameter( mpArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::UnarySin :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 9;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, nullptr, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   // sumangle needed :-(
                    EnhancedCustomShapeEquation _aEquation;
                    _aEquation.nOperation |= 0xe;   // sumangle
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
            case ExpressionFunct::UnaryCos :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 10;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, nullptr, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   // sumangle needed :-(
                    EnhancedCustomShapeEquation aTmpEquation;
                    aTmpEquation.nOperation |= 0xe; // sumangle
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
            case ExpressionFunct::UnaryTan :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 16;
                if ( pOptionalArg )
                    FillEquationParameter( pOptionalArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                else
                    aEquation.nPara[ 0 ] = 1;

                EnhancedCustomShapeParameter aSource( mpArg->fillNode( rEquations, nullptr, nFlags | EXPRESSION_FLAG_SUMANGLE_MODE ) );
                if ( aSource.Type == EnhancedCustomShapeParameterType::NORMAL )
                {   // sumangle needed :-(
                    EnhancedCustomShapeEquation aTmpEquation;
                    aTmpEquation.nOperation |= 0xe; // sumangle
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
            case ExpressionFunct::UnaryAtan:
            {
// TODO:
                aRet.Type = EnhancedCustomShapeParameterType::NORMAL;
            }
            break;
            case ExpressionFunct::UnaryNeg:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 1;
                aEquation.nPara[ 1 ] = -1;
                aEquation.nPara[ 2 ] = 1;
                FillEquationParameter( mpArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
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
    std::shared_ptr<ExpressionNode> mpFirstArg;
    std::shared_ptr<ExpressionNode> mpSecondArg;

public:

    BinaryFunctionExpression( const ExpressionFunct eFunct, const std::shared_ptr<ExpressionNode>& rFirstArg, const std::shared_ptr<ExpressionNode>& rSecondArg ) :
        meFunct( eFunct ),
        mpFirstArg( rFirstArg ),
        mpSecondArg( rSecondArg )
    {
    }
#if (defined(__clang__) && CLANG_VERSION >=30700) || (defined (__GNUC__) && __GNUC__ >= 8)
    //GetEquationValueAsDouble calls isFinite on the result
    __attribute__((no_sanitize("float-divide-by-zero")))
#endif
    static double getValue( const ExpressionFunct eFunct, const std::shared_ptr<ExpressionNode>& rFirstArg, const std::shared_ptr<ExpressionNode>& rSecondArg )
    {
        double fRet = 0;
        switch( eFunct )
        {
            case ExpressionFunct::BinaryPlus : fRet = (*rFirstArg)() + (*rSecondArg)(); break;
            case ExpressionFunct::BinaryMinus: fRet = (*rFirstArg)() - (*rSecondArg)(); break;
            case ExpressionFunct::BinaryMul :  fRet = (*rFirstArg)() * (*rSecondArg)(); break;
            case ExpressionFunct::BinaryDiv :  fRet = (*rFirstArg)() / (*rSecondArg)(); break;
            case ExpressionFunct::BinaryMin :  fRet = ::std::min( (*rFirstArg)(), (*rSecondArg)() ); break;
            case ExpressionFunct::BinaryMax :  fRet = ::std::max( (*rFirstArg)(), (*rSecondArg)() ); break;
            case ExpressionFunct::BinaryAtan2: fRet = atan2( (*rFirstArg)(), (*rSecondArg)() ); break;
            default:
                break;
        }
        return fRet;
    }
    virtual double operator()() const override
    {
        return getValue( meFunct, mpFirstArg, mpSecondArg );
    }
    virtual bool isConstant() const override
    {
        return mpFirstArg->isConstant() && mpSecondArg->isConstant();
    }
    virtual ExpressionFunct getType() const override
    {
        return meFunct;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags ) override
    {
        EnhancedCustomShapeParameter aRet;
        switch( meFunct )
        {
            case ExpressionFunct::BinaryPlus :
            {
                if ( nFlags & EXPRESSION_FLAG_SUMANGLE_MODE )
                {
                    if ( mpFirstArg->getType() == ExpressionFunct::EnumAdjustment )
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0xe;    // sumangle
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                    else if ( mpSecondArg->getType() == ExpressionFunct::EnumAdjustment )
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0xe;    // sumangle
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                    else
                    {
                        EnhancedCustomShapeEquation aSumangle1;
                        aSumangle1.nOperation |= 0xe;   // sumangle
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags &~EXPRESSION_FLAG_SUMANGLE_MODE ), 1, aSumangle1 );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aSumangle1 );

                        EnhancedCustomShapeEquation aSumangle2;
                        aSumangle2.nOperation |= 0xe;   // sumangle
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags &~EXPRESSION_FLAG_SUMANGLE_MODE ), 1, aSumangle2 );
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
                        aRet = mpSecondArg->fillNode( rEquations, nullptr, nFlags );
                    else if ( bSecondIsEmpty )
                        aRet = mpFirstArg->fillNode( rEquations, nullptr, nFlags );
                    else
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 0;
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                }
            }
            break;
            case ExpressionFunct::BinaryMinus:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 0;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 2, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::BinaryMul :
            {
                // in the dest. format the cos function is using integer as result :-(
                // so we can't use the generic algorithm
                if ( ( mpFirstArg->getType() == ExpressionFunct::UnarySin ) || ( mpFirstArg->getType() == ExpressionFunct::UnaryCos ) || ( mpFirstArg->getType() == ExpressionFunct::UnaryTan ) )
                    aRet = mpFirstArg->fillNode( rEquations, mpSecondArg.get(), nFlags );
                else if ( ( mpSecondArg->getType() == ExpressionFunct::UnarySin ) || ( mpSecondArg->getType() == ExpressionFunct::UnaryCos ) || ( mpSecondArg->getType() == ExpressionFunct::UnaryTan ) )
                    aRet = mpSecondArg->fillNode( rEquations, mpFirstArg.get(), nFlags );
                else
                {
                    if ( mpFirstArg->isConstant() && (*mpFirstArg)() == 1 )
                        aRet = mpSecondArg->fillNode( rEquations, nullptr, nFlags );
                    else if ( mpSecondArg->isConstant() && (*mpSecondArg)() == 1 )
                        aRet = mpFirstArg->fillNode( rEquations, nullptr, nFlags );
                    else if ( ( mpFirstArg->getType() == ExpressionFunct::BinaryDiv )      // don't care of (pi/180)
                        && ( static_cast<BinaryFunctionExpression*>(mpFirstArg.get())->mpFirstArg.get()->getType() == ExpressionFunct::EnumPi )
                        && ( static_cast<BinaryFunctionExpression*>(mpFirstArg.get())->mpSecondArg.get()->getType() == ExpressionFunct::Const ) )
                    {
                        aRet = mpSecondArg->fillNode( rEquations, nullptr, nFlags );
                    }
                    else if ( ( mpSecondArg->getType() == ExpressionFunct::BinaryDiv )     // don't care of (pi/180)
                        && ( static_cast<BinaryFunctionExpression*>(mpSecondArg.get())->mpFirstArg.get()->getType() == ExpressionFunct::EnumPi )
                        && ( static_cast<BinaryFunctionExpression*>(mpSecondArg.get())->mpSecondArg.get()->getType() == ExpressionFunct::Const ) )
                    {
                        aRet = mpFirstArg->fillNode( rEquations, nullptr, nFlags );
                    }
                    else
                    {
                        EnhancedCustomShapeEquation aEquation;
                        aEquation.nOperation |= 1;
                        FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                        FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                        aEquation.nPara[ 2 ] = 1;
                        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                        aRet.Value <<= (sal_Int32)rEquations.size();
                        rEquations.push_back( aEquation );
                    }
                }
            }
            break;
            case ExpressionFunct::BinaryDiv :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 1;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                aEquation.nPara[ 1 ] = 1;
                FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 2, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::BinaryMin :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 4;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::BinaryMax :
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 5;
                FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
                aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
                aRet.Value <<= (sal_Int32)rEquations.size();
                rEquations.push_back( aEquation );
            }
            break;
            case ExpressionFunct::BinaryAtan2:
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation |= 8;
                FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
                FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 1, aEquation );
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
    std::shared_ptr<ExpressionNode> mpFirstArg;
    std::shared_ptr<ExpressionNode> mpSecondArg;
    std::shared_ptr<ExpressionNode> mpThirdArg;

public:

    IfExpression( const std::shared_ptr<ExpressionNode>& rFirstArg,
                  const std::shared_ptr<ExpressionNode>& rSecondArg,
                  const std::shared_ptr<ExpressionNode>& rThirdArg ) :
        mpFirstArg(  rFirstArg ),
        mpSecondArg( rSecondArg ),
        mpThirdArg(  rThirdArg )
    {
    }
    virtual bool isConstant() const override
    {
        return
            mpFirstArg->isConstant() &&
            mpSecondArg->isConstant() &&
            mpThirdArg->isConstant();
    }
    virtual double operator()() const override
    {
        return (*mpFirstArg)() > 0 ? (*mpSecondArg)() : (*mpThirdArg)();
    }
    virtual ExpressionFunct getType() const override
    {
        return ExpressionFunct::TernaryIf;
    }
    virtual EnhancedCustomShapeParameter fillNode( std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* /*pOptionalArg*/, sal_uInt32 nFlags ) override
    {
        EnhancedCustomShapeParameter aRet;
        aRet.Type = EnhancedCustomShapeParameterType::EQUATION;
        aRet.Value <<= (sal_Int32)rEquations.size();
        {
            EnhancedCustomShapeEquation aEquation;
            aEquation.nOperation |= 6;
            FillEquationParameter( mpFirstArg->fillNode( rEquations, nullptr, nFlags ), 0, aEquation );
            FillEquationParameter( mpSecondArg->fillNode( rEquations, nullptr, nFlags  ), 1, aEquation );
            FillEquationParameter( mpThirdArg->fillNode( rEquations, nullptr, nFlags ), 2, aEquation );
            rEquations.push_back( aEquation );
        }
        return aRet;
    }
};


// FUNCTION PARSER


typedef const sal_Char* StringIteratorT;

struct ParserContext
{
    typedef ::std::stack< std::shared_ptr<ExpressionNode> > OperandStack;

    // stores a stack of not-yet-evaluated operands. This is used
    // by the operators (i.e. '+', '*', 'sin' etc.) to pop their
    // arguments from. If all arguments to an operator are constant,
    // the operator pushes a precalculated result on the stack, and
    // a composite ExpressionNode otherwise.
    OperandStack                maOperandStack;

    const EnhancedCustomShape2d* mpCustoShape;

};

typedef std::shared_ptr< ParserContext > ParserContextSharedPtr;

/** Generate parse-dependent-but-then-constant value
    */
class DoubleConstantFunctor
{
    ParserContextSharedPtr  mxContext;

public:
    explicit DoubleConstantFunctor( const ParserContextSharedPtr& rContext ) :
        mxContext( rContext )
    {
    }
    void operator()( double n ) const
    {
        mxContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( n ) ) );
    }
};

class EnumFunctor
{
    const ExpressionFunct           meFunct;
    ParserContextSharedPtr          mxContext;

public:

    EnumFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext )
    : meFunct( eFunct )
    , mxContext( rContext )
    {
    }
    void operator()( StringIteratorT rFirst, StringIteratorT rSecond ) const
    {
        /*double nVal = mnValue;*/
        switch( meFunct )
        {
            case ExpressionFunct::EnumAdjustment :
            {
                OUString aVal( rFirst + 1, rSecond - rFirst, RTL_TEXTENCODING_UTF8 );
                mxContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new AdjustmentExpression( *mxContext->mpCustoShape, aVal.toInt32() ) ) );
            }
            break;
            case ExpressionFunct::EnumEquation :
                {
                OUString aVal( rFirst + 1, rSecond - rFirst, RTL_TEXTENCODING_UTF8 );
                mxContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new EquationExpression( *mxContext->mpCustoShape, aVal.toInt32() ) ) );
            }
            break;
            default:
                mxContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new EnumValueExpression( *mxContext->mpCustoShape, meFunct ) ) );
        }
    }
};

class UnaryFunctionFunctor
{
    const ExpressionFunct   meFunct;
    ParserContextSharedPtr  mxContext;

public:

    UnaryFunctionFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext ) :
        meFunct( eFunct ),
        mxContext( rContext )
    {
    }
    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mxContext->maOperandStack );

        if( rNodeStack.size() < 1 )
            throw ParseError( "Not enough arguments for unary operator" );

        // retrieve arguments
        std::shared_ptr<ExpressionNode> pArg( rNodeStack.top() );
        rNodeStack.pop();

        if( pArg->isConstant() )    // check for constness
            rNodeStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( UnaryFunctionExpression::getValue( meFunct, pArg ) ) ) );
        else                        // push complex node, that calcs the value on demand
            rNodeStack.push( std::shared_ptr<ExpressionNode>( new UnaryFunctionExpression( meFunct, pArg ) ) );
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
    ParserContextSharedPtr  mxContext;

public:

    BinaryFunctionFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext ) :
        meFunct( eFunct ),
        mxContext( rContext )
    {
    }

    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mxContext->maOperandStack );

        if( rNodeStack.size() < 2 )
            throw ParseError( "Not enough arguments for binary operator" );

        // retrieve arguments
        std::shared_ptr<ExpressionNode> pSecondArg( rNodeStack.top() );
        rNodeStack.pop();
        std::shared_ptr<ExpressionNode> pFirstArg( rNodeStack.top() );
        rNodeStack.pop();

        // create combined ExpressionNode
        std::shared_ptr<ExpressionNode> pNode = std::shared_ptr<ExpressionNode>( new BinaryFunctionExpression( meFunct, pFirstArg, pSecondArg ) );
        // check for constness
        if( pFirstArg->isConstant() && pSecondArg->isConstant() )   // call the operator() at pNode, store result in constant value ExpressionNode.
            rNodeStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( (*pNode)() ) ) );
        else                                                        // push complex node, that calcs the value on demand
            rNodeStack.push( pNode );
    }
};

class IfFunctor
{
    ParserContextSharedPtr  mxContext;

public:

    explicit IfFunctor( const ParserContextSharedPtr& rContext ) :
        mxContext( rContext )
    {
    }
    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mxContext->maOperandStack );

        if( rNodeStack.size() < 3 )
            throw ParseError( "Not enough arguments for ternary operator" );

        // retrieve arguments
        std::shared_ptr<ExpressionNode> pThirdArg( rNodeStack.top() );
        rNodeStack.pop();
        std::shared_ptr<ExpressionNode> pSecondArg( rNodeStack.top() );
        rNodeStack.pop();
        std::shared_ptr<ExpressionNode> pFirstArg( rNodeStack.top() );
        rNodeStack.pop();

        // create combined ExpressionNode
        std::shared_ptr<ExpressionNode> pNode( new IfExpression( pFirstArg, pSecondArg, pThirdArg ) );
        // check for constness
        if( pFirstArg->isConstant() && pSecondArg->isConstant() && pThirdArg->isConstant() )
            rNodeStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( (*pNode)() ) ) );    // call the operator() at pNode, store result in constant value ExpressionNode.
        else
            rNodeStack.push( pNode );                                       // push complex node, that calcs the value on demand
    }
};

// Workaround for MSVC compiler anomaly (stack trashing)

// The default ureal_parser_policies implementation of parse_exp
// triggers a really weird error in MSVC7 (Version 13.00.9466), in
// that the real_parser_impl::parse_main() call of parse_exp()
// overwrites the frame pointer _on the stack_ (EBP of the calling
// function gets overwritten while lying on the stack).

// For the time being, our parser thus can only read the 1.0E10
// notation, not the 1.0e10 one.

// TODO(F1): Also handle the 1.0e10 case here.
template< typename T > struct custom_real_parser_policies : public ::boost::spirit::ureal_parser_policies<T>
{
    template< typename ScannerT >
        static typename ::boost::spirit::parser_result< ::boost::spirit::chlit<>, ScannerT >::type
    parse_exp(ScannerT& scan)
    {
        // as_lower_d somehow breaks MSVC7
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
    explicit ExpressionGrammar( const ParserContextSharedPtr& rParserContext ) :
        mpParserContext( rParserContext )
    {
    }

    template< typename ScannerT > class definition
    {
    public:
        // grammar definition
        explicit definition( const ExpressionGrammar& self )
        {
            using ::boost::spirit::str_p;
            using ::boost::spirit::range_p;
            using ::boost::spirit::lexeme_d;
            using ::boost::spirit::real_parser;

            identifier =
                            str_p( "pi"         )[ EnumFunctor(ExpressionFunct::EnumPi,        self.getContext() ) ]
                    |       str_p( "left"       )[ EnumFunctor(ExpressionFunct::EnumLeft,      self.getContext() ) ]
                    |       str_p( "top"        )[ EnumFunctor(ExpressionFunct::EnumTop,       self.getContext() ) ]
                    |       str_p( "right"      )[ EnumFunctor(ExpressionFunct::EnumRight,     self.getContext() ) ]
                    |       str_p( "bottom"     )[ EnumFunctor(ExpressionFunct::EnumBottom,    self.getContext() ) ]
                    |       str_p( "xstretch"   )[ EnumFunctor(ExpressionFunct::EnumXStretch,  self.getContext() ) ]
                    |       str_p( "ystretch"   )[ EnumFunctor(ExpressionFunct::EnumYStretch,  self.getContext() ) ]
                    |       str_p( "hasstroke"  )[ EnumFunctor(ExpressionFunct::EnumHasStroke, self.getContext() ) ]
                    |       str_p( "hasfill"    )[ EnumFunctor(ExpressionFunct::EnumHasFill,   self.getContext() ) ]
                    |       str_p( "width"      )[ EnumFunctor(ExpressionFunct::EnumWidth,     self.getContext() ) ]
                    |       str_p( "height"     )[ EnumFunctor(ExpressionFunct::EnumHeight,    self.getContext() ) ]
                    |       str_p( "logwidth"   )[ EnumFunctor(ExpressionFunct::EnumLogWidth,  self.getContext() ) ]
                    |       str_p( "logheight"  )[ EnumFunctor(ExpressionFunct::EnumLogHeight, self.getContext() ) ]
                    ;

            unaryFunction =
                    (str_p( "abs"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnaryAbs,  self.getContext()) ]
                |   (str_p( "sqrt" ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnarySqrt, self.getContext()) ]
                |   (str_p( "sin"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnarySin,  self.getContext()) ]
                |   (str_p( "cos"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnaryCos,  self.getContext()) ]
                |   (str_p( "tan"  ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnaryTan,  self.getContext()) ]
                |   (str_p( "atan" ) >> '(' >> additiveExpression >> ')' )[ UnaryFunctionFunctor( ExpressionFunct::UnaryAtan, self.getContext()) ]
                ;

            binaryFunction =
                    (str_p( "min"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( ExpressionFunct::BinaryMin,  self.getContext()) ]
                |   (str_p( "max"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( ExpressionFunct::BinaryMax,  self.getContext()) ]
                |   (str_p( "atan2") >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ BinaryFunctionFunctor( ExpressionFunct::BinaryAtan2,self.getContext()) ]
                ;

            ternaryFunction =
                    (str_p( "if"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ IfFunctor( self.getContext() ) ]
                ;

            funcRef_decl =
                lexeme_d[ +( range_p('a','z') | range_p('A','Z') | range_p('0','9') ) ];

            functionReference =
                (str_p( "?" ) >> funcRef_decl )[ EnumFunctor( ExpressionFunct::EnumEquation, self.getContext() ) ];

            modRef_decl =
                lexeme_d[ +( range_p('0','9') ) ];

            modifierReference =
                (str_p( "$" ) >> modRef_decl )[ EnumFunctor( ExpressionFunct::EnumAdjustment, self.getContext() ) ];

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
                    ('-' >> basicExpression)[ UnaryFunctionFunctor( ExpressionFunct::UnaryNeg, self.getContext()) ]
                |   basicExpression
                ;

            multiplicativeExpression =
                    unaryExpression
                >> *( ('*' >> unaryExpression)[ BinaryFunctionFunctor( ExpressionFunct::BinaryMul, self.getContext()) ]
                    | ('/' >> unaryExpression)[ BinaryFunctionFunctor( ExpressionFunct::BinaryDiv, self.getContext()) ]
                    )
                ;

            additiveExpression =
                    multiplicativeExpression
                >> *( ('+' >> multiplicativeExpression)[ BinaryFunctionFunctor( ExpressionFunct::BinaryPlus,  self.getContext()) ]
                    | ('-' >> multiplicativeExpression)[ BinaryFunctionFunctor( ExpressionFunct::BinaryMinus, self.getContext()) ]
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
        // the constituents of the Spirit arithmetic expression grammar.
        // For the sake of readability, without 'ma' prefix.
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
    ParserContextSharedPtr          mpParserContext; // might get modified during parsing
};

const ParserContextSharedPtr& getParserContext()
{
    static ParserContextSharedPtr lcl_parserContext( new ParserContext );

    // clear node stack (since we reuse the static object, that's
    // the whole point here)
    while( !lcl_parserContext->maOperandStack.empty() )
        lcl_parserContext->maOperandStack.pop();

    return lcl_parserContext;
}

}

namespace EnhancedCustomShape  {


std::shared_ptr<ExpressionNode> FunctionParser::parseFunction( const OUString& rFunction, const EnhancedCustomShape2d& rCustoShape )
{
    // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
    // gives better conversion robustness here (we might want to map space
    // etc. to ASCII space here)
    const OString& rAsciiFunction(
        OUStringToOString( rFunction, RTL_TEXTENCODING_ASCII_US ) );

    StringIteratorT aStart( rAsciiFunction.getStr() );
    StringIteratorT aEnd( rAsciiFunction.getStr()+rAsciiFunction.getLength() );

    ParserContextSharedPtr pContext;

    // static parser context, because the actual
    // Spirit parser is also a static object
    pContext = getParserContext();
    pContext->mpCustoShape = &rCustoShape;

    ExpressionGrammar aExpressionGrammer( pContext );
    const ::boost::spirit::parse_info<StringIteratorT> aParseInfo(
            ::boost::spirit::parse( aStart,
                                    aEnd,
                                    aExpressionGrammer >> ::boost::spirit::end_p,
                                    ::boost::spirit::space_p ) );

    // input fully congested by the parser?
    if( !aParseInfo.full )
        throw ParseError( "EnhancedCustomShapeFunctionParser::parseFunction(): string not fully parseable" );

    // parser's state stack now must contain exactly _one_ ExpressionNode,
    // which represents our formula.
    if( pContext->maOperandStack.size() != 1 )
        throw ParseError( "EnhancedCustomShapeFunctionParser::parseFunction(): incomplete or empty expression" );


    return pContext->maOperandStack.top();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
