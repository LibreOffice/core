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

#include "xexptran.hxx"
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <tools/helpers.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/unotools.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// Defines

#define BORDER_INTEGERS_ARE_EQUAL       (4)

//////////////////////////////////////////////////////////////////////////////
// Predeclarations

void Imp_SkipDouble(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen);
void Imp_CalcVectorValues(::basegfx::B2DVector& aVec1, ::basegfx::B2DVector& aVec2, bool& bSameLength, bool& bSameDirection)
{
    const sal_Int32 nLen1(FRound(aVec1.getLength()));
    const sal_Int32 nLen2(FRound(aVec2.getLength()));
    aVec1.normalize();
    aVec2.normalize();
    aVec1 += aVec2;
    const sal_Int32 nLen3(FRound(aVec1.getLength() * ((nLen1 + nLen2) / 2.0)));

    bSameLength = (abs(nLen1 - nLen2) <= BORDER_INTEGERS_ARE_EQUAL);
    bSameDirection = (nLen3 <= BORDER_INTEGERS_ARE_EQUAL);
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// parsing help functions for simple chars
void Imp_SkipSpaces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && sal_Unicode(' ') == rStr[rPos])
        rPos++;
}

void Imp_SkipSpacesAndOpeningBraces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (sal_Unicode(' ') == rStr[rPos] || sal_Unicode('(') == rStr[rPos]))
        rPos++;
}

void Imp_SkipSpacesAndCommas(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (sal_Unicode(' ') == rStr[rPos] || sal_Unicode(',') == rStr[rPos]))
        rPos++;
}

void Imp_SkipSpacesAndClosingBraces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (sal_Unicode(' ') == rStr[rPos] || sal_Unicode(')') == rStr[rPos]))
        rPos++;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// parsing help functions for integer numbers

bool Imp_IsOnNumberChar(const OUString& rStr, const sal_Int32 nPos, bool bSignAllowed = true)
{
    sal_Unicode aChar(rStr[nPos]);

    if((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        || (bSignAllowed && sal_Unicode('+') == aChar)
        || (bSignAllowed && sal_Unicode('-') == aChar)
    )
        return true;
    return false;
}

bool Imp_IsOnUnitChar(const OUString& rStr, const sal_Int32 nPos)
{
    sal_Unicode aChar(rStr[nPos]);

    if((sal_Unicode('a') <= aChar && sal_Unicode('z') >= aChar)
        || (sal_Unicode('A') <= aChar && sal_Unicode('Z') >= aChar)
        || sal_Unicode('%') == aChar
    )
        return true;
    return false;
}

void Imp_SkipNumber(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    bool bSignAllowed(true);

    while(rPos < nLen && Imp_IsOnNumberChar(rStr, rPos, bSignAllowed))
    {
        bSignAllowed = false;
        rPos++;
    }
}

void Imp_SkipNumberAndSpacesAndCommas(const OUString& rStr, sal_Int32& rPos,
    const sal_Int32 nLen)
{
    Imp_SkipNumber(rStr, rPos, nLen);
    Imp_SkipSpacesAndCommas(rStr, rPos, nLen);
}

void Imp_PutNumberChar(OUString& rStr, sal_Int32 nValue)
{
    OUStringBuffer sStringBuffer;
    ::sax::Converter::convertNumber(sStringBuffer, nValue);
    rStr += OUString(sStringBuffer.makeStringAndClear());
}

void Imp_PutNumberCharWithSpace(OUString& rStr, sal_Int32 nValue)
{
    const sal_Int32 aLen(rStr.getLength());
    if(aLen)
        if(Imp_IsOnNumberChar(rStr, aLen - 1, false) && nValue >= 0)
            rStr += rtl::OUString(static_cast<sal_Unicode>(' '));
    Imp_PutNumberChar(rStr, nValue);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// parsing help functions for double numbers

void Imp_SkipDouble(const OUString& rStr, sal_Int32& rPos, const sal_Int32)
{
    sal_Unicode aChar(rStr[rPos]);

    if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
        aChar = rStr[++rPos];

    while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        || sal_Unicode('.') == aChar)
    {
        aChar = rStr[++rPos];
    }

    if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
    {
        aChar = rStr[++rPos];

        if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
            aChar = rStr[++rPos];

        while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        {
            aChar = rStr[++rPos];
        }
    }
}

double Imp_GetDoubleChar(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen,
    const SvXMLUnitConverter& rConv, double fRetval, bool bLookForUnits = false)
{
    sal_Unicode aChar(rStr[rPos]);
    OUStringBuffer sNumberString;

    if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
    {
        sNumberString.append(rStr[rPos]);
        aChar = rStr[++rPos];
    }

    while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        || sal_Unicode('.') == aChar)
    {
        sNumberString.append(rStr[rPos]);
        aChar = rStr[++rPos];
    }

    if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
    {
        sNumberString.append(rStr[rPos]);
        aChar = rStr[++rPos];

        if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
        {
            sNumberString.append(rStr[rPos]);
            aChar = rStr[++rPos];
        }

        while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        {
            sNumberString.append(rStr[rPos]);
            aChar = rStr[++rPos];
        }
    }

    if(bLookForUnits)
    {
        Imp_SkipSpaces(rStr, rPos, nLen);
        while(rPos < nLen && Imp_IsOnUnitChar(rStr, rPos))
            sNumberString.append(rStr[rPos++]);
    }

    if(sNumberString.getLength())
    {
        if(bLookForUnits)
            rConv.convertDouble(fRetval, sNumberString.makeStringAndClear(), true);
        else
        {
            ::sax::Converter::convertDouble(fRetval,
                    sNumberString.makeStringAndClear());
        }
    }

    return fRetval;
}

void Imp_PutDoubleChar(OUString& rStr, const SvXMLUnitConverter& rConv, double fValue,
    bool bConvertUnits = false)
{
    OUStringBuffer sStringBuffer;

    if(bConvertUnits)
        rConv.convertDouble(sStringBuffer, fValue, true);
    else
    {
        ::sax::Converter::convertDouble(sStringBuffer, fValue);
    }

    rStr += OUString(sStringBuffer.makeStringAndClear());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// base class of all 2D transform objects

struct ImpSdXMLExpTransObj2DBase
{
    sal_uInt16                  mnType;
    ImpSdXMLExpTransObj2DBase(sal_uInt16 nType)
    :   mnType(nType) {}
};

//////////////////////////////////////////////////////////////////////////////
// possible object types for 2D

#define IMP_SDXMLEXP_TRANSOBJ2D_ROTATE          0x0000
#define IMP_SDXMLEXP_TRANSOBJ2D_SCALE           0x0001
#define IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE       0x0002
#define IMP_SDXMLEXP_TRANSOBJ2D_SKEWX           0x0003
#define IMP_SDXMLEXP_TRANSOBJ2D_SKEWY           0x0004
#define IMP_SDXMLEXP_TRANSOBJ2D_MATRIX          0x0005

//////////////////////////////////////////////////////////////////////////////
// classes of objects, different sizes

struct ImpSdXMLExpTransObj2DRotate : public ImpSdXMLExpTransObj2DBase
{
    double                      mfRotate;
    ImpSdXMLExpTransObj2DRotate(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_ROTATE), mfRotate(fVal) {}
};
struct ImpSdXMLExpTransObj2DScale : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DTuple         maScale;
    ImpSdXMLExpTransObj2DScale(const ::basegfx::B2DTuple& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj2DTranslate : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DTuple         maTranslate;
    ImpSdXMLExpTransObj2DTranslate(const ::basegfx::B2DTuple& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj2DSkewX : public ImpSdXMLExpTransObj2DBase
{
    double                      mfSkewX;
    ImpSdXMLExpTransObj2DSkewX(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWX), mfSkewX(fVal) {}
};
struct ImpSdXMLExpTransObj2DSkewY : public ImpSdXMLExpTransObj2DBase
{
    double                      mfSkewY;
    ImpSdXMLExpTransObj2DSkewY(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWY), mfSkewY(fVal) {}
};
struct ImpSdXMLExpTransObj2DMatrix : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DHomMatrix     maMatrix;
    ImpSdXMLExpTransObj2DMatrix(const ::basegfx::B2DHomMatrix& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_MATRIX), maMatrix(rNew) {}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// delete all entries in list

void SdXMLImExTransform2D::EmptyList()
{
    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList[a];

        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE     :
            {
                delete (ImpSdXMLExpTransObj2DRotate*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                delete (ImpSdXMLExpTransObj2DScale*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                delete (ImpSdXMLExpTransObj2DTranslate*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                delete (ImpSdXMLExpTransObj2DSkewX*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                delete (ImpSdXMLExpTransObj2DSkewY*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX     :
            {
                delete (ImpSdXMLExpTransObj2DMatrix*)pObj;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform2D: impossible entry!");
                break;
            }
        }
    }

    maList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// add members

void SdXMLImExTransform2D::AddRotate(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj2DRotate(fNew));
}

void SdXMLImExTransform2D::AddTranslate(const ::basegfx::B2DTuple& rNew)
{
    if(!rNew.equalZero())
        maList.push_back(new ImpSdXMLExpTransObj2DTranslate(rNew));
}

void SdXMLImExTransform2D::AddSkewX(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj2DSkewX(fNew));
}

//////////////////////////////////////////////////////////////////////////////
// gen string for export
const OUString& SdXMLImExTransform2D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(sal_Unicode(')'));
    OUString aEmptySpace(sal_Unicode(' '));

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE :
            {
                aNewString += OUString( "rotate (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DRotate*)pObj)->mfRotate);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                aNewString += OUString( "scale (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.getX());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.getY());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                aNewString += OUString( "translate (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.getX(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.getY(), true);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                aNewString += OUString( "skewX (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                aNewString += OUString( "skewY (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX :
            {
                aNewString += OUString( "matrix (" );

                // a
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(0, 0));
                aNewString += aEmptySpace;

                // b
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(1, 0));
                aNewString += aEmptySpace;

                // c
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(0, 1));
                aNewString += aEmptySpace;

                // d
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(1, 1));
                aNewString += aEmptySpace;

                // e
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(0, 2), true);
                aNewString += aEmptySpace;

                // f
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix.get(1, 2), true);

                aNewString += aClosingBrace;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform2D: impossible entry!");
                break;
            }
        }

        // if not the last entry, add one space to next tag
        if(a + 1UL != maList.size())
        {
            aNewString += aEmptySpace;
        }
    }

    // fill string form OUString
    msString = aNewString;

    return msString;
}

//////////////////////////////////////////////////////////////////////////////
// sets new string, parses it and generates entries
void SdXMLImExTransform2D::SetString(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    msString = rNew;
    EmptyList();

    if(!msString.isEmpty())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());

        const OUString aString_rotate( "rotate" );
        const OUString aString_scale( "scale" );
        const OUString aString_translate( "translate" );
        const OUString aString_skewX( "skewX" );
        const OUString aString_skewY( "skewY" );
        const OUString aString_matrix( "matrix" );

        sal_Int32 nPos(0);

        while(nPos < nLen)
        {
            // skip spaces
            Imp_SkipSpaces(aStr, nPos, nLen);

            // look for tag
            if(nPos < nLen)
            {
                if(nPos == aStr.indexOf(aString_rotate, nPos))
                {
                    double fValue(0.0);
                    nPos += 6;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj2DRotate(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_scale, nPos))
                {
                    ::basegfx::B2DTuple aValue(1.0, 1.0);
                    nPos += 5;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.setX(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getX()));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setY(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getY()));

                    if(aValue.getX() != 1.0 || aValue.getY() != 1.0)
                        maList.push_back(new ImpSdXMLExpTransObj2DScale(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_translate, nPos))
                {
                    ::basegfx::B2DTuple aValue;
                    nPos += 9;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.setX(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getX(), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setY(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getY(), true));

                    if(!aValue.equalZero())
                        maList.push_back(new ImpSdXMLExpTransObj2DTranslate(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_skewX, nPos))
                {
                    double fValue(0.0);
                    nPos += 5;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj2DSkewX(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_skewY, nPos))
                {
                    double fValue(0.0);
                    nPos += 5;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj2DSkewY(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_matrix, nPos))
                {
                    ::basegfx::B2DHomMatrix aValue;

                    nPos += 6;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);

                    // a
                    aValue.set(0, 0, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 0)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // b
                    aValue.set(1, 0, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 0)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // c
                    aValue.set(0, 1, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 1)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // d
                    aValue.set(1, 1, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 1)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // e
                    aValue.set(0, 2, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 2), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // f
                    aValue.set(1, 2, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 2), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    if(!aValue.isIdentity())
                        maList.push_back(new ImpSdXMLExpTransObj2DMatrix(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else
                {
                    nPos++;
                }
            }
        }
    }
}

void SdXMLImExTransform2D::GetFullTransform(::basegfx::B2DHomMatrix& rFullTrans)
{
    rFullTrans.identity();

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE     :
            {
                // #i78696#
                // mfRotate is mathematically wrong oriented since we export/import the angle
                // values mirrored. This error is fixed in the API, but not yet in the FileFormat.
                // For the FileFormat there is a follow-up task (#i78698#) to fix this in the next
                // ODF FileFormat version. For now - to emulate the old behaviour - it is necessary
                // to mirror the value here
                rFullTrans.rotate(((ImpSdXMLExpTransObj2DRotate*)pObj)->mfRotate * -1.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                const ::basegfx::B2DTuple& rScale = ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale;
                rFullTrans.scale(rScale.getX(), rScale.getY());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                const ::basegfx::B2DTuple& rTranslate = ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate;
                rFullTrans.translate(rTranslate.getX(), rTranslate.getY());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                rFullTrans.shearX(tan(((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                rFullTrans.shearY(tan(((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX     :
            {
                rFullTrans *= ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform2D: impossible entry!");
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// base class of all 3D transform objects

struct ImpSdXMLExpTransObj3DBase
{
    sal_uInt16                  mnType;
    ImpSdXMLExpTransObj3DBase(sal_uInt16 nType)
    :   mnType(nType) {}
};

//////////////////////////////////////////////////////////////////////////////
// possible object types for 3D

#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X        0x0000
#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y        0x0001
#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z        0x0002
#define IMP_SDXMLEXP_TRANSOBJ3D_SCALE           0x0003
#define IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE       0x0004
#define IMP_SDXMLEXP_TRANSOBJ3D_MATRIX          0x0005

//////////////////////////////////////////////////////////////////////////////
// classes of objects, different sizes

struct ImpSdXMLExpTransObj3DRotateX : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateX;
    ImpSdXMLExpTransObj3DRotateX(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X), mfRotateX(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateY : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateY;
    ImpSdXMLExpTransObj3DRotateY(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y), mfRotateY(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateZ : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateZ;
    ImpSdXMLExpTransObj3DRotateZ(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z), mfRotateZ(fVal) {}
};
struct ImpSdXMLExpTransObj3DScale : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DTuple         maScale;
    ImpSdXMLExpTransObj3DScale(const ::basegfx::B3DTuple& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj3DTranslate : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DTuple         maTranslate;
    ImpSdXMLExpTransObj3DTranslate(const ::basegfx::B3DTuple& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj3DMatrix : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DHomMatrix     maMatrix;
    ImpSdXMLExpTransObj3DMatrix(const ::basegfx::B3DHomMatrix& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_MATRIX), maMatrix(rNew) {}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// delete all entries in list

void SdXMLImExTransform3D::EmptyList()
{
    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList[a];

        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X   :
            {
                delete (ImpSdXMLExpTransObj3DRotateX*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                delete (ImpSdXMLExpTransObj3DRotateY*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                delete (ImpSdXMLExpTransObj3DRotateZ*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                delete (ImpSdXMLExpTransObj3DScale*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                delete (ImpSdXMLExpTransObj3DTranslate*)pObj;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX     :
            {
                delete (ImpSdXMLExpTransObj3DMatrix*)pObj;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform3D: impossible entry!");
                break;
            }
        }
    }

    maList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// add members

void SdXMLImExTransform3D::AddMatrix(const ::basegfx::B3DHomMatrix& rNew)
{
    if(!rNew.isIdentity())
        maList.push_back(new ImpSdXMLExpTransObj3DMatrix(rNew));
}

void SdXMLImExTransform3D::AddHomogenMatrix(const drawing::HomogenMatrix& xHomMat)
{
    ::basegfx::B3DHomMatrix aExportMatrix;

    aExportMatrix.set(0, 0, xHomMat.Line1.Column1);
    aExportMatrix.set(0, 1, xHomMat.Line1.Column2);
    aExportMatrix.set(0, 2, xHomMat.Line1.Column3);
    aExportMatrix.set(0, 3, xHomMat.Line1.Column4);
    aExportMatrix.set(1, 0, xHomMat.Line2.Column1);
    aExportMatrix.set(1, 1, xHomMat.Line2.Column2);
    aExportMatrix.set(1, 2, xHomMat.Line2.Column3);
    aExportMatrix.set(1, 3, xHomMat.Line2.Column4);
    aExportMatrix.set(2, 0, xHomMat.Line3.Column1);
    aExportMatrix.set(2, 1, xHomMat.Line3.Column2);
    aExportMatrix.set(2, 2, xHomMat.Line3.Column3);
    aExportMatrix.set(2, 3, xHomMat.Line3.Column4);

    AddMatrix(aExportMatrix);
}

//////////////////////////////////////////////////////////////////////////////
// gen string for export
const OUString& SdXMLImExTransform3D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(sal_Unicode(')'));
    OUString aEmptySpace(sal_Unicode(' '));

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X   :
            {
                aNewString += OUString( "rotatex (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateX*)pObj)->mfRotateX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                aNewString += OUString( "rotatey (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateY*)pObj)->mfRotateY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                aNewString += OUString( "rotatez (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateZ*)pObj)->mfRotateZ);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                aNewString += OUString( "scale (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.getX());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.getY());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.getZ());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                aNewString += OUString( "translate (" );
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.getX(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.getY(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.getZ(), true);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX :
            {
                aNewString += OUString( "matrix (" );

                // a
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(0, 0));
                aNewString += aEmptySpace;

                // b
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(1, 0));
                aNewString += aEmptySpace;

                // c
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(2, 0));
                aNewString += aEmptySpace;

                // d
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(0, 1));
                aNewString += aEmptySpace;

                // e
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(1, 1));
                aNewString += aEmptySpace;

                // f
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(2, 1));
                aNewString += aEmptySpace;

                // g
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(0, 2));
                aNewString += aEmptySpace;

                // h
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(1, 2));
                aNewString += aEmptySpace;

                // i
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(2, 2));
                aNewString += aEmptySpace;

                // j
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(0, 3), true);
                aNewString += aEmptySpace;

                // k
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(1, 3), true);
                aNewString += aEmptySpace;

                // l
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix.get(2, 3), true);

                aNewString += aClosingBrace;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform3D: impossible entry!");
                break;
            }
        }

        // if not the last entry, add one space to next tag
        if(a + 1UL != maList.size())
        {
            aNewString += aEmptySpace;
        }
    }

    // fill string form OUString
    msString = aNewString;

    return msString;
}

//////////////////////////////////////////////////////////////////////////////
// for Import: constructor with string, parses it and generates entries
SdXMLImExTransform3D::SdXMLImExTransform3D(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    SetString(rNew, rConv);
}

//////////////////////////////////////////////////////////////////////////////
// sets new string, parses it and generates entries
void SdXMLImExTransform3D::SetString(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    msString = rNew;
    EmptyList();

    if(!msString.isEmpty())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());

        const OUString aString_rotatex( "rotatex" );
        const OUString aString_rotatey( "rotatey" );
        const OUString aString_rotatez( "rotatez" );
        const OUString aString_scale( "scale" );
        const OUString aString_translate( "translate" );
        const OUString aString_matrix( "matrix" );

        sal_Int32 nPos(0);

        while(nPos < nLen)
        {
            // skip spaces
            Imp_SkipSpaces(aStr, nPos, nLen);

            // look for tag
            if(nPos < nLen)
            {
                if(nPos == aStr.indexOf(aString_rotatex, nPos))
                {
                    double fValue(0.0);

                    nPos += 7;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateX(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatey, nPos))
                {
                    double fValue(0.0);

                    nPos += 7;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateY(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatez, nPos))
                {
                    double fValue(0.0);

                    nPos += 7;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateZ(fValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_scale, nPos))
                {
                    ::basegfx::B3DTuple aValue(1.0, 1.0, 1.0);

                    nPos += 5;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.setX(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getX()));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setY(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getY()));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setZ(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getZ()));

                    if(1.0 != aValue.getX() || 1.0 != aValue.getY() || 1.0 != aValue.getZ())
                        maList.push_back(new ImpSdXMLExpTransObj3DScale(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_translate, nPos))
                {
                    ::basegfx::B3DTuple aValue;

                    nPos += 9;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.setX(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getX(), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setY(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getY(), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.setZ(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.getZ(), true));

                    if(!aValue.equalZero())
                        maList.push_back(new ImpSdXMLExpTransObj3DTranslate(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_matrix, nPos))
                {
                    ::basegfx::B3DHomMatrix aValue;

                    nPos += 6;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);

                    // a
                    aValue.set(0, 0, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 0)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // b
                    aValue.set(1, 0, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 0)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // c
                    aValue.set(2, 0, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(2, 0)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // d
                    aValue.set(0, 1, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 1)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // e
                    aValue.set(1, 1, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 1)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // f
                    aValue.set(2, 1, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(2, 1)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // g
                    aValue.set(0, 2, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 2)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // h
                    aValue.set(1, 2, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 2)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // i
                    aValue.set(2, 2, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(2, 2)));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // j
                    aValue.set(0, 3, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(0, 3), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // k
                    aValue.set(1, 3, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(1, 3), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // l
                    aValue.set(2, 3, Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.get(2, 3), true));
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    if(!aValue.isIdentity())
                        maList.push_back(new ImpSdXMLExpTransObj3DMatrix(aValue));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else
                {
                    nPos++;
                }
            }
        }
    }
}

bool SdXMLImExTransform3D::GetFullHomogenTransform(com::sun::star::drawing::HomogenMatrix& xHomMat)
{
    ::basegfx::B3DHomMatrix aFullTransform;
    GetFullTransform(aFullTransform);

    if(!aFullTransform.isIdentity())
    {
        xHomMat.Line1.Column1 = aFullTransform.get(0, 0);
        xHomMat.Line1.Column2 = aFullTransform.get(0, 1);
        xHomMat.Line1.Column3 = aFullTransform.get(0, 2);
        xHomMat.Line1.Column4 = aFullTransform.get(0, 3);

        xHomMat.Line2.Column1 = aFullTransform.get(1, 0);
        xHomMat.Line2.Column2 = aFullTransform.get(1, 1);
        xHomMat.Line2.Column3 = aFullTransform.get(1, 2);
        xHomMat.Line2.Column4 = aFullTransform.get(1, 3);

        xHomMat.Line3.Column1 = aFullTransform.get(2, 0);
        xHomMat.Line3.Column2 = aFullTransform.get(2, 1);
        xHomMat.Line3.Column3 = aFullTransform.get(2, 2);
        xHomMat.Line3.Column4 = aFullTransform.get(2, 3);

        xHomMat.Line4.Column1 = aFullTransform.get(3, 0);
        xHomMat.Line4.Column2 = aFullTransform.get(3, 1);
        xHomMat.Line4.Column3 = aFullTransform.get(3, 2);
        xHomMat.Line4.Column4 = aFullTransform.get(3, 3);

        return true;
    }

    return false;
}

void SdXMLImExTransform3D::GetFullTransform(::basegfx::B3DHomMatrix& rFullTrans)
{
    rFullTrans.identity();

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X   :
            {
                rFullTrans.rotate(((ImpSdXMLExpTransObj3DRotateX*)pObj)->mfRotateX, 0.0, 0.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                rFullTrans.rotate(0.0, ((ImpSdXMLExpTransObj3DRotateY*)pObj)->mfRotateY, 0.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                rFullTrans.rotate(0.0, 0.0, ((ImpSdXMLExpTransObj3DRotateZ*)pObj)->mfRotateZ);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                const ::basegfx::B3DTuple& rScale = ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale;
                rFullTrans.scale(rScale.getX(), rScale.getY(), rScale.getZ());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                const ::basegfx::B3DTuple& rTranslate = ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate;
                rFullTrans.translate(rTranslate.getX(), rTranslate.getY(), rTranslate.getZ());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX     :
            {
                rFullTrans *= ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix;
                break;
            }
            default :
            {
                OSL_FAIL("SdXMLImExTransform3D: impossible entry!");
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImExViewBox::SdXMLImExViewBox(sal_Int32 nX, sal_Int32 nY, sal_Int32 nW, sal_Int32 nH)
:   mnX( nX ),
    mnY( nY ),
    mnW( nW ),
    mnH( nH )
{
}

// #100617# Asked vincent hardy: svg:viewBox values may be double precision.
SdXMLImExViewBox::SdXMLImExViewBox(const OUString& rNew, const SvXMLUnitConverter& rConv)
:   msString(rNew),
    mnX( 0L ),
    mnY( 0L ),
    mnW( 1000L ),
    mnH( 1000L )
{
    if(!msString.isEmpty())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());
        sal_Int32 nPos(0);

        // skip starting spaces
        Imp_SkipSpaces(aStr, nPos, nLen);

        // get mX, #100617# be prepared for doubles
        mnX = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)mnX));

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mY, #100617# be prepared for doubles
        mnY = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)mnY));

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mW, #100617# be prepared for doubles
        mnW = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)mnW));

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mH, #100617# be prepared for doubles
        mnH = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)mnH));
    }
}

const OUString& SdXMLImExViewBox::GetExportString()
{
    OUString aNewString;
    OUString aEmptySpace(sal_Unicode(' '));

    Imp_PutNumberChar(aNewString, mnX);
    aNewString += aEmptySpace;

    Imp_PutNumberChar(aNewString, mnY);
    aNewString += aEmptySpace;

    Imp_PutNumberChar(aNewString, mnW);
    aNewString += aEmptySpace;

    Imp_PutNumberChar(aNewString, mnH);

    // set new string
    msString = aNewString;

    return msString;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImExPointsElement::SdXMLImExPointsElement(drawing::PointSequence* pPoints,
    const SdXMLImExViewBox& rViewBox,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    // #96328#
    const bool bClosed)
:   maPoly( 0L )
{
    DBG_ASSERT(pPoints, "Empty PointSequence handed over to SdXMLImExPointsElement(!)");

    // add polygon to string
    sal_Int32 nCnt(pPoints->getLength());

    // #104076# Convert to string only when at last one point included
    if(nCnt > 0)
    {
        OUString aNewString;
        awt::Point* pArray = pPoints->getArray();

        // last point same? Ignore it.
        // #96328# ...but only when polygon is CLOSED
        if(bClosed && (pArray->X == (pArray + (nCnt - 1))->X) && (pArray->Y == (pArray + (nCnt - 1))->Y))
            nCnt--;

        // object size and ViewBox size different?
        bool bScale(rObjectSize.Width != rViewBox.GetWidth()
            || rObjectSize.Height != rViewBox.GetHeight());
        bool bTranslate(rViewBox.GetX() != 0L || rViewBox.GetY() != 0L);

        for(sal_Int32 a(0L); a < nCnt; a++)
        {
            // prepare coordinates
            sal_Int32 nX( pArray->X - rObjectPos.X );
            sal_Int32 nY( pArray->Y - rObjectPos.Y );

            if(bScale && rObjectSize.Width && rObjectSize.Height)
            {
                nX = (nX * rViewBox.GetWidth()) / rObjectSize.Width;
                nY = (nY * rViewBox.GetHeight()) / rObjectSize.Height;
            }

            if(bTranslate)
            {
                nX += rViewBox.GetX();
                nY += rViewBox.GetY();
            }

            // X and comma
            Imp_PutNumberChar(aNewString, nX);
            aNewString += rtl::OUString(static_cast<sal_Unicode>(','));

            // Y and space (not for last)
            Imp_PutNumberChar(aNewString, nY);
            if(a + 1 != nCnt)
                aNewString += rtl::OUString(static_cast<sal_Unicode>(' '));

            // next point
            pArray++;
        }

        // set new string
        msString = aNewString;
    }
}

// #100617# svg:polyline or svg:polygon values may be double precision.
SdXMLImExPointsElement::SdXMLImExPointsElement(const OUString& rNew,
    const SdXMLImExViewBox& rViewBox,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    const SvXMLUnitConverter& rConv)
:   msString( rNew ),
    maPoly( 0L )
{
    // convert string to polygon
    const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
    const sal_Int32 nLen(aStr.getLength());
    sal_Int32 nPos(0);
    sal_Int32 nNumPoints(0L);

    // skip starting spaces
    Imp_SkipSpaces(aStr, nPos, nLen);

    // count points in first loop
    while(nPos < nLen)
    {
        // skip number, #100617# be prepared for doubles
        Imp_SkipDouble(aStr, nPos, nLen);

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // skip number, #100617# be prepared for doubles
        Imp_SkipDouble(aStr, nPos, nLen);

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // one more point
        nNumPoints++;
    }

    // second loop
    if(nNumPoints)
    {
        nPos = 0;
        maPoly.realloc(1);
        drawing::PointSequence* pOuterSequence = maPoly.getArray();
        pOuterSequence->realloc(nNumPoints);
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        // object size and ViewBox size different?
        bool bScale(rObjectSize.Width != rViewBox.GetWidth()
            || rObjectSize.Height != rViewBox.GetHeight());
        bool bTranslate(rViewBox.GetX() != 0L || rViewBox.GetY() != 0L);

        // skip starting spaces
        Imp_SkipSpaces(aStr, nPos, nLen);

        while(nPos < nLen)
        {
            // prepare new parameter pair
            sal_Int32 nX(0L);
            sal_Int32 nY(0L);

            // get mX, #100617# be prepared for doubles
            nX = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)nX));

            // skip spaces and commas
            Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

            // get mY, #100617# be prepared for doubles
            nY = FRound(Imp_GetDoubleChar(aStr, nPos, nLen, rConv, (double)nY));

            // skip spaces and commas
            Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

            // prepare parameters
            if(bTranslate)
            {
                nX -= rViewBox.GetX();
                nY -= rViewBox.GetY();
            }

            if(bScale && rViewBox.GetWidth() && rViewBox.GetHeight() )
            {
                nX = (nX * rObjectSize.Width) / rViewBox.GetWidth();
                nY = (nY * rObjectSize.Height) / rViewBox.GetHeight();
            }

            nX += rObjectPos.X;
            nY += rObjectPos.Y;

            // add new point
            *pInnerSequence = awt::Point( nX, nY );
            pInnerSequence++;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImExSvgDElement::SdXMLImExSvgDElement(const SdXMLImExViewBox& rViewBox,
                                           const SvXMLExport&      rExport)
:   mrViewBox( rViewBox ),
    mbIsClosed( false ),
    mbIsCurve( false ),
    // fdo#47406 - handle writing svg:d path slightly different for
    // old odf versions and ODF1.2 compat mode - since ~all the legacy
    // ODF ecosystem interprets relative svg:d paths incorrectly,
    // write out absolute paths in those cases.
    mbRelative( rExport.getDefaultVersion() >= SvtSaveOptions::ODFVER_012 &&
                rExport.getDefaultVersion() != SvtSaveOptions::ODFVER_012_EXT_COMPAT ),
    mnLastX( 0L ),
    mnLastY( 0L ),
    maPoly()
{
}

void Imp_GetPrevPos(awt::Point*& pPrevPos1,
    drawing::PolygonFlags& aPrevFlag1,
    const bool bClosed, awt::Point* pPoints,
    drawing::PolygonFlags* pFlags, const sal_Int32 nPos,
    const sal_Int32 nCnt, const sal_Int32 nAdd)
{
    if(bClosed)
    {
        pPrevPos1 = pPoints + ((nPos + nCnt - nAdd) % nCnt);
        aPrevFlag1 = *(pFlags + ((nPos + nCnt - nAdd) % nCnt));
    }
    else if(nPos > (nAdd - 1))
    {
        pPrevPos1 = pPoints + (nPos - nAdd);
        aPrevFlag1 = *(pFlags + (nPos - nAdd));
    }
    else
        pPrevPos1 = 0L;
}

void Imp_PrepareCoorExport(sal_Int32& nX, sal_Int32& nY,
    const awt::Point* pPointArray, const awt::Point& rObjectPos,
    const awt::Size& rObjectSize, const SdXMLImExViewBox& mrViewBox,
    const bool bScale, const bool bTranslate)
{
    nX = pPointArray->X - rObjectPos.X;
    nY = pPointArray->Y - rObjectPos.Y;

    if(bScale && rObjectSize.Width && rObjectSize.Height )
    {
        nX = (nX * mrViewBox.GetWidth()) / rObjectSize.Width;
        nY = (nY * mrViewBox.GetHeight()) / rObjectSize.Height;
    }

    if(bTranslate)
    {
        nX += mrViewBox.GetX();
        nY += mrViewBox.GetY();
    }
}

//#define TEST_QUADRATIC_CURVES
#ifdef TEST_QUADRATIC_CURVES
// To be able to test quadratic curve code: The code concerning to
// bDoTestHere can be used (see below). Construct shapes which have their control
// points on equal coordinates. When these are written, they can be
// forced to create correct 'Q' and 'T' statements using this flag.
// These may then be tested for import/exporting.
static bool bDoTestHere(true);
#endif // TEST_QUADRATIC_CURVES

void SdXMLImExSvgDElement::AddPolygon(
    drawing::PointSequence* pPoints,
    drawing::FlagSequence* pFlags,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    bool bClosed)
{
    // Leaving the export stuff for the while, should eventually also
    // consolidated with basegfx svg support
    DBG_ASSERT(pPoints, "Empty PointSequence handed over to SdXMLImExSvgDElement(!)");

    sal_Int32 nCnt(pPoints->getLength());

    // #104076# Convert to string only when at last one point included
    if(nCnt > 0)
    {
        // append polygon to string
        OUString aNewString;
        sal_Unicode aLastCommand = ' ';
        awt::Point* pPointArray = pPoints->getArray();

        // are the flags used at all? If not forget about them
        if(pFlags)
        {
            sal_Int32 nFlagCnt(pFlags->getLength());

            if(nFlagCnt)
            {
                bool bFlagsUsed(false);
                drawing::PolygonFlags* pFlagArray = pFlags->getArray();

                for(sal_Int32 a(0); !bFlagsUsed && a < nFlagCnt; a++)
                    if(drawing::PolygonFlags_NORMAL != *pFlagArray++)
                        bFlagsUsed = true;

                if(!bFlagsUsed)
                    pFlags = 0L;
            }
            else
            {
                pFlags = 0L;
            }
        }

        // object size and ViewBox size different?
        bool bScale(rObjectSize.Width != mrViewBox.GetWidth()
            || rObjectSize.Height != mrViewBox.GetHeight());
        bool bTranslate(mrViewBox.GetX() != 0L || mrViewBox.GetY() != 0L);

        // #87202# rework of point reduction:
        // Test for Last point same -> closed, ignore last point. Take
        // some more circumstances in account when looking at curve segments.
        drawing::PolygonFlags* pFlagArray = (pFlags) ? pFlags->getArray() : 0L;
        if((pPointArray->X == (pPointArray + (nCnt - 1))->X) && (pPointArray->Y == (pPointArray + (nCnt - 1))->Y))
        {
            if(pFlags)
            {
                // point needs to be ignored if point before it is
                // NO control point. Else the last point is needed
                // for exporting the last segment of the curve. That means
                // that the last and the first point will be saved double,
                // but SVG does not support a better solution here.
                if(nCnt >= 2 && drawing::PolygonFlags_CONTROL != *(pFlagArray + (nCnt - 2)))
                {
                    nCnt--;
                }
            }
            else
            {
                // no curve, ignore last point
                nCnt--;
            }
        }

        // bezier poly, handle curves
        bool  bDidWriteStart(false);
        sal_Int32 nStartX(0), nStartY(0);

        for(sal_Int32 a(0L); a < nCnt; a++)
        {
            if(!pFlags || drawing::PolygonFlags_CONTROL != *pFlagArray)
            {
                bool bDidWriteAsCurve(false);

                if(bDidWriteStart)
                {
                    if(pFlags)
                    {
                        // real curve point, get previous to see if it's a control point
                        awt::Point* pPrevPos1;
                        drawing::PolygonFlags aPrevFlag1;

                        Imp_GetPrevPos(pPrevPos1, aPrevFlag1, bClosed, pPoints->getArray(),
                            pFlags->getArray(), a, nCnt, 1);

                        if(pPrevPos1 && drawing::PolygonFlags_CONTROL == aPrevFlag1)
                        {
                            // get previous2 to see if it's a control point, too
                            awt::Point* pPrevPos2;
                            drawing::PolygonFlags aPrevFlag2;

                            Imp_GetPrevPos(pPrevPos2, aPrevFlag2, bClosed, pPoints->getArray(),
                                pFlags->getArray(), a, nCnt, 2);

                            if(pPrevPos2 && drawing::PolygonFlags_CONTROL == aPrevFlag2)
                            {
                                // get previous3 to see if it's a curve point and if,
                                // if it is fully symmetric or not
                                awt::Point* pPrevPos3;
                                drawing::PolygonFlags aPrevFlag3;

                                Imp_GetPrevPos(pPrevPos3, aPrevFlag3, bClosed, pPoints->getArray(),
                                    pFlags->getArray(), a, nCnt, 3);

                                if(pPrevPos3)
                                {
                                    // prepare coordinates
                                    sal_Int32 nX, nY;

                                    Imp_PrepareCoorExport(nX, nY, pPointArray, rObjectPos, rObjectSize,
                                        mrViewBox, bScale, bTranslate);

                                    // #100617# test if this curve segment may be written as
                                    // a quadratic bezier
                                    // That's the case if both control points are in the same place
                                    // when they are prolonged to the common quadratic control point
                                    // Left:  P = (3P1 - P0) / 2
                                    // Right: P = (3P2 - P3) / 2
                                    bool bIsQuadratic(false);
                                    const bool bEnableSaveQuadratic(false);

                                    sal_Int32 nPX_L(FRound((double)((3 * pPrevPos2->X) - pPrevPos3->X) / 2.0));
                                    sal_Int32 nPY_L(FRound((double)((3 * pPrevPos2->Y) - pPrevPos3->Y) / 2.0));
                                    sal_Int32 nPX_R(FRound((double)((3 * pPrevPos1->X) - pPointArray->X) / 2.0));
                                    sal_Int32 nPY_R(FRound((double)((3 * pPrevPos1->Y) - pPointArray->Y) / 2.0));
                                    sal_Int32 nDist(0);

                                    if(nPX_L != nPX_R)
                                    {
                                        nDist += abs(nPX_L - nPX_R);
                                    }

                                    if(nPY_L != nPY_R)
                                    {
                                        nDist += abs(nPY_L - nPY_R);
                                    }

                                    if(nDist <= BORDER_INTEGERS_ARE_EQUAL)
                                    {
                                        if(bEnableSaveQuadratic)
                                        {
                                            bIsQuadratic = true;
                                        }
                                    }

#ifdef TEST_QUADRATIC_CURVES
                                    if(bDoTestHere)
                                    {
                                        bIsQuadratic = false;

                                        if(pPrevPos1->X == pPrevPos2->X && pPrevPos1->Y == pPrevPos2->Y)
                                            bIsQuadratic = true;
                                    }
#endif // TEST_QUADRATIC_CURVES

                                    if(bIsQuadratic)
                                    {
#ifdef TEST_QUADRATIC_CURVES
                                        if(bDoTestHere)
                                        {
                                            bool bPrevPointIsSymmetric(false);

                                            if(drawing::PolygonFlags_SYMMETRIC == aPrevFlag3)
                                            {
                                                // get previous4 to see if it's a control point
                                                awt::Point* pPrevPos4;
                                                drawing::PolygonFlags aPrevFlag4;

                                                Imp_GetPrevPos(pPrevPos4, aPrevFlag4, bClosed, pPoints->getArray(),
                                                    pFlags->getArray(), a, nCnt, 4);

                                                if(drawing::PolygonFlags_CONTROL == aPrevFlag4)
                                                {
                                                    // okay, prevPos3 is symmetric (c2) and prevPos4
                                                    // is existing control point, the 's' statement can be used
                                                    bPrevPointIsSymmetric = true;
                                                }
                                            }

                                            if(bPrevPointIsSymmetric)
                                            {
                                                // write a shorthand/smooth quadratic curveto entry (T)
                                                if(mbRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('t'))
                                                        aNewString += OUString(sal_Unicode('t'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('t');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('T'))
                                                        aNewString += OUString(sal_Unicode('T'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY);

                                                    aLastCommand = sal_Unicode('T');
                                                }
                                            }
                                            else
                                            {
                                                // prepare coordinates
                                                sal_Int32 nX1, nY1;

                                                Imp_PrepareCoorExport(nX1, nY1, pPrevPos1, rObjectPos, rObjectSize,
                                                    mrViewBox, bScale, bTranslate);

                                                // write a quadratic curveto entry (Q)
                                                if(mbRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('q'))
                                                        aNewString += OUString(sal_Unicode('q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX1 - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY1 - mnLastY);
                                                    Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('q');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('Q'))
                                                        aNewString += OUString(sal_Unicode('Q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX1);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY1);
                                                    Imp_PutNumberCharWithSpace(aNewString, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY);

                                                    aLastCommand = sal_Unicode('Q');
                                                }
                                            }
                                        }
                                        else
                                        {
#endif // TEST_QUADRATIC_CURVES
                                            awt::Point aNewPoint(nPX_L, nPY_L);
                                            bool bPrevPointIsSmooth(false);

                                            if(drawing::PolygonFlags_SMOOTH == aPrevFlag3)
                                            {
                                                // get previous4 to see if it's a control point
                                                awt::Point* pPrevPos4;
                                                drawing::PolygonFlags aPrevFlag4;

                                                Imp_GetPrevPos(pPrevPos4, aPrevFlag4, bClosed, pPoints->getArray(),
                                                    pFlags->getArray(), a, nCnt, 4);

                                                if(drawing::PolygonFlags_CONTROL == aPrevFlag4)
                                                {
                                                    // okay, prevPos3 is smooth (c1) and prevPos4
                                                    // is existing control point. Test if it's even symmetric
                                                    // and thus the 'T' statement may be used.
                                                    ::basegfx::B2DVector aVec1(pPrevPos4->X - pPrevPos3->X, pPrevPos4->Y - pPrevPos3->Y);
                                                    ::basegfx::B2DVector aVec2(aNewPoint.X - pPrevPos3->X, aNewPoint.Y - pPrevPos3->Y);
                                                    bool bSameLength(false);
                                                    bool bSameDirection(false);

                                                    // get vector values
                                                    Imp_CalcVectorValues(aVec1, aVec2, bSameLength, bSameDirection);

                                                    if(bSameLength && bSameDirection)
                                                        bPrevPointIsSmooth = true;
                                                }
                                            }

                                            if(bPrevPointIsSmooth)
                                            {
                                                // write a shorthand/smooth quadratic curveto entry (T)
                                                if(mbRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('t'))
                                                        aNewString += rtl::OUString(static_cast<sal_Unicode>('t'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('t');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('T'))
                                                        aNewString += rtl::OUString(static_cast<sal_Unicode>('T'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY);

                                                    aLastCommand = sal_Unicode('T');
                                                }
                                            }
                                            else
                                            {
                                                // prepare coordinates
                                                sal_Int32 nX1, nY1;

                                                Imp_PrepareCoorExport(nX1, nY1, &aNewPoint, rObjectPos, rObjectSize,
                                                    mrViewBox, bScale, bTranslate);

                                                // write a quadratic curveto entry (Q)
                                                if(mbRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('q'))
                                                        aNewString += rtl::OUString(static_cast<sal_Unicode>('q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX1 - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY1 - mnLastY);
                                                    Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('q');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('Q'))
                                                        aNewString += rtl::OUString(static_cast<sal_Unicode>('Q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, nX1);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY1);
                                                    Imp_PutNumberCharWithSpace(aNewString, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, nY);

                                                    aLastCommand = sal_Unicode('Q');
                                                }
                                            }
#ifdef TEST_QUADRATIC_CURVES
                                        }
#endif // TEST_QUADRATIC_CURVES
                                    }
                                    else
                                    {
                                        bool bPrevPointIsSymmetric(false);

                                        if(drawing::PolygonFlags_SYMMETRIC == aPrevFlag3)
                                        {
                                            // get previous4 to see if it's a control point
                                            awt::Point* pPrevPos4;
                                            drawing::PolygonFlags aPrevFlag4;

                                            Imp_GetPrevPos(pPrevPos4, aPrevFlag4, bClosed, pPoints->getArray(),
                                                pFlags->getArray(), a, nCnt, 4);

                                            if(drawing::PolygonFlags_CONTROL == aPrevFlag4)
                                            {
                                                // okay, prevPos3 is symmetric (c2) and prevPos4
                                                // is existing control point, the 's' statement can be used
                                                bPrevPointIsSymmetric = true;
                                            }
                                        }

                                        // prepare coordinates
                                        sal_Int32 nX2, nY2;

                                        Imp_PrepareCoorExport(nX2, nY2, pPrevPos1, rObjectPos, rObjectSize,
                                            mrViewBox, bScale, bTranslate);

                                        if(bPrevPointIsSymmetric)
                                        {
                                            // write a shorthand/smooth curveto entry (S)
                                            if(mbRelative)
                                            {
                                                if(aLastCommand != sal_Unicode('s'))
                                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('s'));

                                                Imp_PutNumberCharWithSpace(aNewString, nX2 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY2 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                aLastCommand = sal_Unicode('s');
                                            }
                                            else
                                            {
                                                if(aLastCommand != sal_Unicode('S'))
                                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('S'));

                                                Imp_PutNumberCharWithSpace(aNewString, nX2);
                                                Imp_PutNumberCharWithSpace(aNewString, nY2);
                                                Imp_PutNumberCharWithSpace(aNewString, nX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY);

                                                aLastCommand = sal_Unicode('S');
                                            }
                                        }
                                        else
                                        {
                                            // prepare coordinates
                                            sal_Int32 nX1, nY1;

                                            Imp_PrepareCoorExport(nX1, nY1, pPrevPos2, rObjectPos, rObjectSize,
                                                mrViewBox, bScale, bTranslate);

                                            // write a curveto entry (C)
                                            if(mbRelative)
                                            {
                                                if(aLastCommand != sal_Unicode('c'))
                                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('c'));

                                                Imp_PutNumberCharWithSpace(aNewString, nX1 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY1 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, nX2 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY2 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                                aLastCommand = sal_Unicode('c');
                                            }
                                            else
                                            {
                                                if(aLastCommand != sal_Unicode('C'))
                                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('C'));

                                                Imp_PutNumberCharWithSpace(aNewString, nX1);
                                                Imp_PutNumberCharWithSpace(aNewString, nY1);
                                                Imp_PutNumberCharWithSpace(aNewString, nX2);
                                                Imp_PutNumberCharWithSpace(aNewString, nY2);
                                                Imp_PutNumberCharWithSpace(aNewString, nX);
                                                Imp_PutNumberCharWithSpace(aNewString, nY);

                                                aLastCommand = sal_Unicode('C');
                                            }
                                        }
                                    }

                                    // remember that current point IS written
                                    bDidWriteAsCurve = true;

                                    // remember new last position
                                    mnLastX = nX;
                                    mnLastY = nY;
                                }
                            }
                        }
                    }
                }

                if(!bDidWriteAsCurve)
                {
                    // current point not yet written, prepare coordinates
                    sal_Int32 nX, nY;

                    Imp_PrepareCoorExport(nX, nY, pPointArray, rObjectPos, rObjectSize,
                        mrViewBox, bScale, bTranslate);

                    if(bDidWriteStart)
                    {
                        // write as normal point
                        if(mnLastX == nX)
                        {
                            if(mbRelative)
                            {
                                if(aLastCommand != sal_Unicode('v'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('v'));

                                Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                aLastCommand = sal_Unicode('v');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('V'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('V'));

                                Imp_PutNumberCharWithSpace(aNewString, nY);

                                aLastCommand = sal_Unicode('V');
                            }
                        }
                        else if(mnLastY == nY)
                        {
                            if(mbRelative)
                            {
                                if(aLastCommand != sal_Unicode('h'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('h'));

                                Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);

                                aLastCommand = sal_Unicode('h');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('H'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('H'));

                                Imp_PutNumberCharWithSpace(aNewString, nX);

                                aLastCommand = sal_Unicode('H');
                            }
                        }
                        else
                        {
                            if(mbRelative)
                            {
                                if(aLastCommand != sal_Unicode('l'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('l'));

                                Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                                Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                                aLastCommand = sal_Unicode('l');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('L'))
                                    aNewString += rtl::OUString(static_cast<sal_Unicode>('L'));

                                Imp_PutNumberCharWithSpace(aNewString, nX);
                                Imp_PutNumberCharWithSpace(aNewString, nY);

                                aLastCommand = sal_Unicode('L');
                            }
                        }
                    }
                    else
                    {
                        // write as start point
                        if(mbRelative)
                        {
                            aNewString += rtl::OUString(static_cast<sal_Unicode>('m'));

                            Imp_PutNumberCharWithSpace(aNewString, nX - mnLastX);
                            Imp_PutNumberCharWithSpace(aNewString, nY - mnLastY);

                            aLastCommand = sal_Unicode('l');
                        }
                        else
                        {
                            aNewString += rtl::OUString(static_cast<sal_Unicode>('M'));

                            Imp_PutNumberCharWithSpace(aNewString, nX);
                            Imp_PutNumberCharWithSpace(aNewString, nY);

                            aLastCommand = sal_Unicode('L');
                        }

                        // remember start written
                        bDidWriteStart = true;
                        nStartX = nX;
                        nStartY = nY;
                    }

                    // remember new last position
                    mnLastX = nX;
                    mnLastY = nY;
                }
            }

            // next point
            pPointArray++;
            pFlagArray++;
        }

        // close path if closed poly
        if(bClosed)
        {
            if(mbRelative)
                aNewString += rtl::OUString(static_cast<sal_Unicode>('z'));
            else
                aNewString += rtl::OUString(static_cast<sal_Unicode>('Z'));

            // update current point - we're back at the start
            if( bDidWriteStart )
            {
                mnLastX = nStartX;
                mnLastY = nStartY;
            }
        }

        // append new string
        msString += aNewString;
    }
}

SdXMLImExSvgDElement::SdXMLImExSvgDElement(const OUString& rNew,
    const SdXMLImExViewBox& rViewBox,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    const SvXMLImport& rImport)
:   msString( rNew ),
    mrViewBox( rViewBox ),
    mbIsClosed( false ),
    mbIsCurve( false ),
    mbRelative( true ),
    mnLastX( 0L ),
    mnLastY( 0L ),
    maPoly()
{
    bool bWrongPositionAfterZ( false );
	sal_Int32 nUPD( 0 );
	sal_Int32 nBuildId( 0 );
	if ( rImport.getBuildIds( nUPD, nBuildId ) &&
       ( ( nUPD == 641 ) || ( nUPD == 645 ) || ( nUPD == 680 ) || ( nUPD == 300 ) ||
         ( nUPD == 310 ) || ( nUPD == 320 ) || ( nUPD == 330 ) || ( nUPD == 340 ) ||
         ( nUPD == 350 && nBuildId < 202 ) ) )
    {
        bWrongPositionAfterZ = true;
    }

    // convert string to polygon
    basegfx::B2DPolyPolygon aPoly;
    basegfx::tools::importFromSvgD(aPoly,msString,bWrongPositionAfterZ);

    mbIsCurve = aPoly.areControlPointsUsed();
    mbIsClosed = aPoly.isClosed();

    // object size and ViewBox size different?
    basegfx::B2DHomMatrix aTransform;
    const bool bScale(rObjectSize.Width != mrViewBox.GetWidth()
                || rObjectSize.Height != mrViewBox.GetHeight());
    const bool bTranslate(mrViewBox.GetX() != 0L || mrViewBox.GetY() != 0L);

    if( bTranslate )
        aTransform.translate(
            -mrViewBox.GetX(),
            -mrViewBox.GetY());
    if( bScale )
        aTransform.scale(
            (mrViewBox.GetWidth() ? rObjectSize.Width / mrViewBox.GetWidth() : 0),
            (mrViewBox.GetHeight() ? rObjectSize.Height / mrViewBox.GetHeight() : 0));
    aTransform.translate( rObjectPos.X, rObjectPos.Y );
    aPoly.transform(aTransform);

    basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier(aPoly,maPoly);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
