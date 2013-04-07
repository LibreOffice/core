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


#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include "formula/errorcodes.hxx"
#include "interpre.hxx"
#include <svl/zforlist.hxx>
#include <tools/stream.hxx>
#include <rtl/math.hxx>

#include <math.h>

#include <vector>

#include <mdds/multi_type_matrix.hpp>
#include <mdds/multi_type_vector_types.hpp>
#include <mdds/multi_type_vector_trait.hpp>

using ::std::pair;
using ::std::for_each;
using ::std::count_if;
using ::std::advance;
using ::std::unary_function;

const mdds::mtv::element_t element_type_custom_string = mdds::mtv::element_type_user_start;
typedef mdds::mtv::default_element_block<element_type_custom_string, OUString> custom_string_block;

namespace rtl {

// Callback functions required for supporting OUString in
// mdds::multi_type_vector.  They must be in the rtl namespace to satisfy
// argument dependent lookup that mdds::multi_type_vector requires.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(OUString, element_type_custom_string, OUString(), custom_string_block)

}

/**
 * Custom string trait struct to tell mdds::multi_type_matrix about the
 * custom string type and how to handle blocks storing them.
 */
struct custom_string_trait
{
    typedef OUString string_type;
    typedef custom_string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = element_type_custom_string;

    struct element_block_func
    {
        static mdds::mtv::base_element_block* create_new_block(
            mdds::mtv::element_t type, size_t init_size)
        {
            switch (type)
            {
                case element_type_custom_string:
                    return string_element_block::create_block(init_size);
                default:
                    return mdds::mtv::element_block_func::create_new_block(type, init_size);
            }
        }

        static mdds::mtv::base_element_block* clone_block(const mdds::mtv::base_element_block& block)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    return string_element_block::clone_block(block);
                default:
                    return mdds::mtv::element_block_func::clone_block(block);
            }
        }

        static void delete_block(mdds::mtv::base_element_block* p)
        {
            if (!p)
                return;

            switch (mdds::mtv::get_block_type(*p))
            {
                case element_type_custom_string:
                    string_element_block::delete_block(p);
                break;
                default:
                    mdds::mtv::element_block_func::delete_block(p);
            }
        }

        static void resize_block(mdds::mtv::base_element_block& block, size_t new_size)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::resize_block(block, new_size);
                break;
                default:
                    mdds::mtv::element_block_func::resize_block(block, new_size);
            }
        }

        static void print_block(const mdds::mtv::base_element_block& block)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::print_block(block);
                break;
                default:
                    mdds::mtv::element_block_func::print_block(block);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::erase_block(block, pos);
                break;
                default:
                    mdds::mtv::element_block_func::erase(block, pos);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos, size_t size)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::erase_block(block, pos, size);
                break;
                default:
                    mdds::mtv::element_block_func::erase(block, pos, size);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src)
        {
            switch (mdds::mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::append_values_from_block(dest, src);
                break;
                default:
                    mdds::mtv::element_block_func::append_values_from_block(dest, src);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mdds::mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::append_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::element_block_func::append_values_from_block(dest, src, begin_pos, len);
            }
        }

        static void assign_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mdds::mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::assign_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::element_block_func::assign_values_from_block(dest, src, begin_pos, len);
            }
        }

        static bool equal_block(
            const mdds::mtv::base_element_block& left, const mdds::mtv::base_element_block& right)
        {
            if (mdds::mtv::get_block_type(left) == element_type_custom_string)
            {
                if (mdds::mtv::get_block_type(right) != element_type_custom_string)
                    return false;

                return string_element_block::get(left) == string_element_block::get(right);
            }
            else if (mdds::mtv::get_block_type(right) == element_type_custom_string)
                return false;

            return mdds::mtv::element_block_func::equal_block(left, right);
        }

        static void overwrite_values(mdds::mtv::base_element_block& block, size_t pos, size_t len)
        {
            switch (mdds::mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    // Do nothing.  One needs to handle this only when the
                    // block stores pointers and manages their life cycles.
                break;
                default:
                    mdds::mtv::element_block_func::overwrite_values(block, pos, len);
            }
        }
    };
};

// ============================================================================

namespace {

typedef mdds::multi_type_matrix<custom_string_trait> MatrixImplType;

struct ElemEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val == 0.0;
    }
};

struct ElemNotEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val != 0.0;
    }
};

struct ElemGreaterZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val > 0.0;
    }
};

struct ElemLessZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val < 0.0;
    }
};

struct ElemGreaterEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val >= 0.0;
    }
};

struct ElemLessEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val <= 0.0;
    }
};

template<typename _Comp>
void compareMatrix(MatrixImplType& rMat)
{
    MatrixImplType::size_pair_type aDim = rMat.size();
    MatrixImplType aNewMat(aDim.row, aDim.column, false); // initialize with boolean block.  faster this way.

    _Comp aComp;
    for (size_t i = 0; i < aDim.row; ++i)
    {
        for (size_t j = 0; j < aDim.column; ++j)
        {
            mdds::mtm::element_t eType = rMat.get_type(i, j);
            if (eType != mdds::mtm::element_numeric && eType != mdds::mtm::element_boolean)
                // must be of numeric type (boolean can be numeric).
                continue;

            double fVal = rMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                /* FIXME: this silently skips an error instead of propagating it! */
                continue;

            bool b = aComp(fVal);
            aNewMat.set(i, j, b);
        }
    }
    aNewMat.swap(rMat);
}

}

class ScMatrixImpl
{
    MatrixImplType maMat;
    MatrixImplType maMatFlag;
    ScInterpreter* pErrorInterpreter;
    bool            mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.
    MatrixImplType::size_pair_type  maCachedSize;

    ScMatrixImpl();
    ScMatrixImpl(const ScMatrixImpl&);
public:
    ScMatrixImpl(SCSIZE nC, SCSIZE nR);
    ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal);
    ~ScMatrixImpl();

    void Clear();
    void SetImmutable(bool bVal);
    bool IsImmutable() const;
    void Resize(SCSIZE nC, SCSIZE nR);
    void Resize(SCSIZE nC, SCSIZE nR, double fVal);
    void SetErrorInterpreter( ScInterpreter* p);
    ScInterpreter* GetErrorInterpreter() const { return pErrorInterpreter; }

    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;
    bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    void SetErrorAtInterpreter( sal_uInt16 nError ) const;

    void PutDouble(double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const OUString& rStr, SCSIZE nIndex);
    void PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    sal_uInt16 GetError( SCSIZE nC, SCSIZE nR) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    OUString GetString(SCSIZE nC, SCSIZE nR) const;
    OUString GetString( SCSIZE nIndex) const;
    OUString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
    ScMatrixValue Get(SCSIZE nC, SCSIZE nR) const;
    bool IsString( SCSIZE nIndex ) const;
    bool IsString( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const;
    bool IsValue( SCSIZE nIndex ) const;
    bool IsValue( SCSIZE nC, SCSIZE nR ) const;
    bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsBoolean( SCSIZE nC, SCSIZE nR ) const;
    bool IsNumeric() const;
    void MatCopy(ScMatrixImpl& mRes) const;
    void MatTrans(ScMatrixImpl& mRes) const;
    void FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    void CompareEqual();
    void CompareNotEqual();
    void CompareLess();
    void CompareGreater();
    void CompareLessEqual();
    void CompareGreaterEqual();
    double And() const;
    double Or() const;
    double Xor() const;

    ScMatrix::IterateResult Sum(bool bTextAsZero) const;
    ScMatrix::IterateResult SumSquare(bool bTextAsZero) const;
    ScMatrix::IterateResult Product(bool bTextAsZero) const;
    size_t Count(bool bCountStrings) const;

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR) :
    maMat(nR, nC), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal) :
    maMat(nR, nC, fInitVal), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::~ScMatrixImpl()
{
    Clear();
}

void ScMatrixImpl::Clear()
{
    maMat.clear();
    maMatFlag.clear();
}

void ScMatrixImpl::SetImmutable(bool bVal)
{
    mbCloneIfConst = bVal;
}

bool ScMatrixImpl::IsImmutable() const
{
    return mbCloneIfConst;
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR)
{
    maMat.resize(nR, nC);
    maMatFlag.resize(nR, nC);
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    maMat.resize(nR, nC, fVal);
    maMatFlag.resize(nR, nC);
}

void ScMatrixImpl::SetErrorInterpreter( ScInterpreter* p)
{
    pErrorInterpreter = p;
}

void ScMatrixImpl::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    rR = aSize.row;
    rC = aSize.column;
}

SCSIZE ScMatrixImpl::GetElementCount() const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    return aSize.row * aSize.column;
}

bool ScMatrixImpl::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    return nR < aSize.row && nC < aSize.column;
}

bool ScMatrixImpl::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    if (aSize.column == 1 && aSize.row == 1)
    {
        rC = 0;
        rR = 0;
        return true;
    }
    else if (aSize.column == 1 && rR < aSize.row)
    {
        // single column matrix.
        rC = 0;
        return true;
    }
    else if (aSize.row == 1 && rC < aSize.column)
    {
        // single row matrix.
        rR = 0;
        return true;
    }
    return false;
}

bool ScMatrixImpl::ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return ValidColRow( rC, rR) || ValidColRowReplicated( rC, rR);
}

void ScMatrixImpl::SetErrorAtInterpreter( sal_uInt16 nError ) const
{
    if ( pErrorInterpreter )
        pErrorInterpreter->SetError( nError);
}

void ScMatrixImpl::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, fVal);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutDouble: dimension error");
    }
}

void ScMatrixImpl::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, pArray, pArray + nLen);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutDouble: dimension error");
    }
}

void ScMatrixImpl::PutDouble( double fVal, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutDouble(fVal, nC, nR);
}

void ScMatrixImpl::PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, rStr);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, pArray, pArray + nLen);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const OUString& rStr, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutString(rStr, nC, nR);
}

void ScMatrixImpl::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
    {
        maMat.set_empty(nR, nC);
        maMatFlag.set(nR, nC, false); // zero flag to indicate that this is 'empty', not 'empty path'.
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmpty: dimension error");
    }
}

void ScMatrixImpl::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
    {
        maMat.set_empty(nR, nC);
        maMatFlag.set(nR, nC, true); // non-zero flag to indicate empty 'path'.
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmptyPath: dimension error");
    }
}

void ScMatrixImpl::PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR )
{
    maMat.set(nR, nC, CreateDoubleError(nErrorCode));
}

void ScMatrixImpl::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, bVal);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutBoolean: dimension error");
    }
}

sal_uInt16 ScMatrixImpl::GetError( SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        return GetDoubleErrorValue(fVal);
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetError: dimension error");
        return errNoValue;
    }
}

double ScMatrixImpl::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        if ( pErrorInterpreter )
        {
            sal_uInt16 nError = GetDoubleErrorValue(fVal);
            if ( nError )
                SetErrorAtInterpreter( nError);
        }
        return fVal;
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetDouble: dimension error");
        return CreateDoubleError( errNoValue);
    }
}

double ScMatrixImpl::GetDouble( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetDouble(nC, nR);
}

OUString ScMatrixImpl::GetString(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fErr = 0.0;
        switch (maMat.get_type(nR, nC))
        {
            case mdds::mtm::element_string:
                return maMat.get<OUString>(nR, nC);
            case mdds::mtm::element_empty:
                return EMPTY_OUSTRING;
            case mdds::mtm::element_numeric:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
                fErr = maMat.get<double>(nR, nC);
            break;
            case mdds::mtm::element_boolean:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
                fErr = maMat.get<bool>(nR, nC);
            break;
            default:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
        }
        SetErrorAtInterpreter(GetDoubleErrorValue(fErr));
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetString: dimension error");
    }
    return EMPTY_OUSTRING;
}

OUString ScMatrixImpl::GetString( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetString(nC, nR);
}

OUString ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    if (!ValidColRowOrReplicated( nC, nR ))
    {
        OSL_FAIL("ScMatrixImpl::GetString: dimension error");
        return OUString();
    }

    double fVal = 0.0;
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_string:
            return maMat.get<OUString>(nR, nC);
        case mdds::mtm::element_empty:
        {
            if (!maMatFlag.get<bool>(nR, nC))
                // not an empty path.
                break;

            // result of empty FALSE jump path
            sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_LOGICAL,
                    ScGlobal::eLnge);
            OUString aStr;
            Color* pColor = NULL;
            rFormatter.GetOutputString( 0.0, nKey, aStr, &pColor);
            return aStr;
        }
        case mdds::mtm::element_numeric:
            fVal = maMat.get<double>(nR, nC);
        break;
        case mdds::mtm::element_boolean:
            fVal = maMat.get<bool>(nR, nC);
        break;
        default:
            ;
    }

    sal_uInt16 nError = GetDoubleErrorValue(fVal);
    if (nError)
    {
        SetErrorAtInterpreter( nError);
        return ScGlobal::GetErrorString( nError);
    }

    sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_NUMBER,
            ScGlobal::eLnge);
    OUString aStr;
    rFormatter.GetInputLineString( fVal, nKey, aStr);
    return aStr;
}

ScMatrixValue ScMatrixImpl::Get(SCSIZE nC, SCSIZE nR) const
{
    ScMatrixValue aVal;
    if (ValidColRowOrReplicated(nC, nR))
    {
        mdds::mtm::element_t eType = maMat.get_type(nR, nC);
        switch (eType)
        {
            case mdds::mtm::element_boolean:
                aVal.nType = SC_MATVAL_BOOLEAN;
                aVal.fVal = maMat.get_boolean(nR, nC);
            break;
            case mdds::mtm::element_numeric:
                aVal.nType = SC_MATVAL_VALUE;
                aVal.fVal = maMat.get_numeric(nR, nC);
            break;
            case mdds::mtm::element_string:
                aVal.nType = SC_MATVAL_STRING;
                aVal.aStr = maMat.get_string(nR, nC);
            break;
            case mdds::mtm::element_empty:
                // Empty path equals empty plus flag.
                aVal.nType = maMatFlag.get<bool>(nR, nC) ? SC_MATVAL_EMPTYPATH : SC_MATVAL_EMPTY;
                aVal.fVal = 0.0;
            default:
                ;
        }
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::Get: dimension error");
    }
    return aVal;
}

bool ScMatrixImpl::IsString( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsString(nC, nR);
}

bool ScMatrixImpl::IsString( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_empty:
        case mdds::mtm::element_string:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must be zero for this to be an empty element, instead of being an
    // empty path element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty && !maMatFlag.get<bool>(nR, nC);
}

bool ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // 'Empty path' is empty plus non-zero flag.
    if (ValidColRowOrReplicated( nC, nR ))
        return maMat.get_type(nR, nC) == mdds::mtm::element_empty && maMatFlag.get<bool>(nR, nC);
    else
        return true;
}

bool ScMatrixImpl::IsValue( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsValue(nC, nR);
}

bool ScMatrixImpl::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated(nC, nR);
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_boolean:
        case mdds::mtm::element_numeric:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated(nC, nR);
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_boolean:
        case mdds::mtm::element_numeric:
        case mdds::mtm::element_empty:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_boolean;
}

bool ScMatrixImpl::IsNumeric() const
{
    return maMat.numeric();
}

void ScMatrixImpl::MatCopy(ScMatrixImpl& mRes) const
{
    if (maMat.size().row > mRes.maMat.size().row || maMat.size().column > mRes.maMat.size().column)
    {
        // destination matrix is not large enough.
        OSL_FAIL("ScMatrixImpl::MatCopy: dimension error");
        return;
    }

    mRes.maMat.copy(maMat);
}

void ScMatrixImpl::MatTrans(ScMatrixImpl& mRes) const
{
    mRes.maMat = maMat;
    mRes.maMat.transpose();
}

void ScMatrixImpl::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    if (ValidColRow( nC1, nR1) && ValidColRow( nC2, nR2))
    {
        for (SCSIZE j = nC1; j <= nC2; ++j)
        {
            // Passing value array is much faster.
            std::vector<double> aVals(nR2-nR1+1, fVal);
            maMat.set(nR1, j, aVals.begin(), aVals.end());
        }
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::FillDouble: dimension error");
    }
}

void ScMatrixImpl::CompareEqual()
{
    compareMatrix<ElemEqualZero>(maMat);
}

void ScMatrixImpl::CompareNotEqual()
{
    compareMatrix<ElemNotEqualZero>(maMat);
}

void ScMatrixImpl::CompareLess()
{
    compareMatrix<ElemLessZero>(maMat);
}

void ScMatrixImpl::CompareGreater()
{
    compareMatrix<ElemGreaterZero>(maMat);
}

void ScMatrixImpl::CompareLessEqual()
{
    compareMatrix<ElemLessEqualZero>(maMat);
}

void ScMatrixImpl::CompareGreaterEqual()
{
    compareMatrix<ElemGreaterEqualZero>(maMat);
}

namespace {

struct AndEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult &= (fVal != 0.0); }
    bool result() const { return mbResult; }
    AndEvaluator() : mbResult(true) {}
};

struct OrEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult |= (fVal != 0.0); }
    bool result() const { return mbResult; }
    OrEvaluator() : mbResult(false) {}
};

struct XorEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult ^= (fVal != 0.0); }
    bool result() const { return mbResult; }
    XorEvaluator() : mbResult(false) {}
};

// Do not short circuit logical operations, in case there are error values
// these need to be propagated even if the result was determined earlier.
template <typename _Evaluator>
double EvalMatrix(const MatrixImplType& rMat)
{
    _Evaluator aEval;
    size_t nRows = rMat.size().row, nCols = rMat.size().column;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            mdds::mtm::element_t eType = rMat.get_type(i, j);
            if (eType != mdds::mtm::element_numeric && eType != mdds::mtm::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(errIllegalArgument);

            double fVal = rMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                // DoubleError
                return fVal;

            aEval.operate(fVal);
        }
    }
    return aEval.result();
}

}

double ScMatrixImpl::And() const
{
    // All elements must be of value type.
    // True only if all the elements have non-zero values.
    return EvalMatrix<AndEvaluator>(maMat);
}

double ScMatrixImpl::Or() const
{
    // All elements must be of value type.
    // True if at least one element has a non-zero value.
    return EvalMatrix<OrEvaluator>(maMat);
}

double ScMatrixImpl::Xor() const
{
    // All elements must be of value type.
    // True if an odd number of elements have a non-zero value.
    return EvalMatrix<XorEvaluator>(maMat);
}

namespace {

struct SumOp
{
    static const int InitVal = 0;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal;
    }
};

struct SumSquareOp
{
    static const int InitVal = 0;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal*fVal;
    }
};

struct ProductOp
{
    static const int InitVal = 1;

    void operator() (double& rAccum, double fVal)
    {
        rAccum *= fVal;
    }
};

template<typename _Op>
class WalkElementBlocks : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    _Op maOp;

    ScMatrix::IterateResult maRes;
    bool mbFirst:1;
    bool mbTextAsZero:1;
public:
    WalkElementBlocks(bool bTextAsZero) : maRes(0.0, _Op::InitVal, 0), mbFirst(true), mbTextAsZero(bTextAsZero) {}

    const ScMatrix::IterateResult& getResult() const { return maRes; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                mdds::mtv::numeric_element_block::const_iterator it = mdds::mtv::numeric_element_block::begin(*node.data);
                mdds::mtv::numeric_element_block::const_iterator itEnd = mdds::mtv::numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    if (mbFirst)
                    {
                        maOp(maRes.mfFirst, *it);
                        mbFirst = false;
                    }
                    else
                        maOp(maRes.mfRest, *it);
                }
                maRes.mnCount += node.size;
            }
            break;
            case mdds::mtm::element_boolean:
            {
                mdds::mtv::boolean_element_block::const_iterator it = mdds::mtv::boolean_element_block::begin(*node.data);
                mdds::mtv::boolean_element_block::const_iterator itEnd = mdds::mtv::boolean_element_block::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    if (mbFirst)
                    {
                        maOp(maRes.mfFirst, *it);
                        mbFirst = false;
                    }
                    else
                        maOp(maRes.mfRest, *it);
                }
                maRes.mnCount += node.size;
            }
            break;
            case mdds::mtm::element_string:
                if (mbTextAsZero)
                    maRes.mnCount += node.size;
            break;
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};

class CountElements : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    size_t mnCount;
    bool mbCountString;
public:
    CountElements(bool bCountString) : mnCount(0), mbCountString(bCountString) {}

    size_t getCount() const { return mnCount; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            case mdds::mtm::element_boolean:
                mnCount += node.size;
            break;
            case mdds::mtm::element_string:
                if (mbCountString)
                    mnCount += node.size;
            break;
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};

}

ScMatrix::IterateResult ScMatrixImpl::Sum(bool bTextAsZero) const
{
    WalkElementBlocks<SumOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

ScMatrix::IterateResult ScMatrixImpl::SumSquare(bool bTextAsZero) const
{
    WalkElementBlocks<SumSquareOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

ScMatrix::IterateResult ScMatrixImpl::Product(bool bTextAsZero) const
{
    WalkElementBlocks<ProductOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    ScMatrix::IterateResult aRes = aFunc.getResult();
    return aRes;
}

size_t ScMatrixImpl::Count(bool bCountStrings) const
{
    CountElements aFunc(bCountStrings);
    maMat.walk(aFunc);
    return aFunc.getCount();
}

void ScMatrixImpl::CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const
{
    SCSIZE nRowSize = maMat.size().row;
    rC = nIndex / nRowSize;
    rR = nIndex - rC*nRowSize;
}

// ============================================================================

ScMatrix::ScMatrix( SCSIZE nC, SCSIZE nR) :
    pImpl(new ScMatrixImpl(nC, nR)), nRefCnt(0) {}

ScMatrix::ScMatrix(SCSIZE nC, SCSIZE nR, double fInitVal) :
    pImpl(new ScMatrixImpl(nC, nR, fInitVal)), nRefCnt(0) {}

ScMatrix::~ScMatrix()
{
    delete pImpl;
}

ScMatrix* ScMatrix::Clone() const
{
    SCSIZE nC, nR;
    pImpl->GetDimensions(nC, nR);
    ScMatrix* pScMat = new ScMatrix(nC, nR);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());    // TODO: really?
    return pScMat;
}

ScMatrix* ScMatrix::CloneIfConst()
{
    return pImpl->IsImmutable() ? Clone() : this;
}

void ScMatrix::SetImmutable( bool bVal )
{
    pImpl->SetImmutable(bVal);
}

void ScMatrix::Resize( SCSIZE nC, SCSIZE nR)
{
    pImpl->Resize(nC, nR);
}

void ScMatrix::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    pImpl->Resize(nC, nR, fVal);
}

ScMatrix* ScMatrix::CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const
{
    ScMatrix* pScMat = new ScMatrix(nNewCols, nNewRows);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());
    return pScMat;
}

void ScMatrix::SetErrorInterpreter( ScInterpreter* p)
{
    pImpl->SetErrorInterpreter(p);
}

void ScMatrix::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    pImpl->GetDimensions(rC, rR);
}

SCSIZE ScMatrix::GetElementCount() const
{
    return pImpl->GetElementCount();
}

bool ScMatrix::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->ValidColRow(nC, nR);
}

bool ScMatrix::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return pImpl->ValidColRowReplicated(rC, rR);
}

bool ScMatrix::ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return ValidColRow( rC, rR) || ValidColRowReplicated( rC, rR);
}

void ScMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(fVal, nC, nR);
}

void ScMatrix::PutDouble( double fVal, SCSIZE nIndex)
{
    pImpl->PutDouble(fVal, nIndex);
}

void ScMatrix::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(pArray, nLen, nC, nR);
}

void ScMatrix::PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(rStr, nC, nR);
}

void ScMatrix::PutString(const OUString& rStr, SCSIZE nIndex)
{
    pImpl->PutString(rStr, nIndex);
}

void ScMatrix::PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(pArray, nLen, nC, nR);
}

void ScMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmpty(nC, nR);
}

void ScMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmptyPath(nC, nR);
}

void ScMatrix::PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutError(nErrorCode, nC, nR);
}

void ScMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutBoolean(bVal, nC, nR);
}

sal_uInt16 ScMatrix::GetError( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetError(nC, nR);
}

double ScMatrix::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetDouble(nC, nR);
}

double ScMatrix::GetDouble( SCSIZE nIndex) const
{
    return pImpl->GetDouble(nIndex);
}

OUString ScMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(nC, nR);
}

OUString ScMatrix::GetString( SCSIZE nIndex) const
{
    return pImpl->GetString(nIndex);
}

OUString ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->Get(nC, nR);
}

sal_Bool ScMatrix::IsString( SCSIZE nIndex ) const
{
    return pImpl->IsString(nIndex);
}

sal_Bool ScMatrix::IsString( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsString(nC, nR);
}

sal_Bool ScMatrix::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmpty(nC, nR);
}

sal_Bool ScMatrix::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyPath(nC, nR);
}

sal_Bool ScMatrix::IsValue( SCSIZE nIndex ) const
{
    return pImpl->IsValue(nIndex);
}

sal_Bool ScMatrix::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValue(nC, nR);
}

sal_Bool ScMatrix::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValueOrEmpty(nC, nR);
}

sal_Bool ScMatrix::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsBoolean(nC, nR);
}

sal_Bool ScMatrix::IsNumeric() const
{
    return pImpl->IsNumeric();
}

void ScMatrix::MatCopy(ScMatrix& mRes) const
{
    pImpl->MatCopy(*mRes.pImpl);
}

void ScMatrix::MatTrans(ScMatrix& mRes) const
{
    pImpl->MatTrans(*mRes.pImpl);
}

void ScMatrix::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    pImpl->FillDouble(fVal, nC1, nR1, nC2, nR2);
}

void ScMatrix::CompareEqual()
{
    pImpl->CompareEqual();
}

void ScMatrix::CompareNotEqual()
{
    pImpl->CompareNotEqual();
}

void ScMatrix::CompareLess()
{
    pImpl->CompareLess();
}

void ScMatrix::CompareGreater()
{
    pImpl->CompareGreater();
}

void ScMatrix::CompareLessEqual()
{
    pImpl->CompareLessEqual();
}

void ScMatrix::CompareGreaterEqual()
{
    pImpl->CompareGreaterEqual();
}

double ScMatrix::And() const
{
    return pImpl->And();
}

double ScMatrix::Or() const
{
    return pImpl->Or();
}

double ScMatrix::Xor() const
{
    return pImpl->Xor();
}

ScMatrix::IterateResult ScMatrix::Sum(bool bTextAsZero) const
{
    return pImpl->Sum(bTextAsZero);
}

ScMatrix::IterateResult ScMatrix::SumSquare(bool bTextAsZero) const
{
    return pImpl->SumSquare(bTextAsZero);
}

ScMatrix::IterateResult ScMatrix::Product(bool bTextAsZero) const
{
    return pImpl->Product(bTextAsZero);
}

size_t ScMatrix::Count(bool bCountStrings) const
{
    return pImpl->Count(bCountStrings);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
