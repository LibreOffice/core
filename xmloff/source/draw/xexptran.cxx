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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "xexptran.hxx"
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmluconv.hxx>
#include <tools/gen.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <tools/string.hxx>
#include <basegfx/matrix/b3dhommatrixtools.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// Defines

#define BORDER_INTEGERS_ARE_EQUAL       (4)

//////////////////////////////////////////////////////////////////////////////
// Predeclarations

void Imp_SkipDouble(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen);
void Imp_CalcVectorValues(::basegfx::B2DVector& aVec1, ::basegfx::B2DVector& aVec2, bool& bSameLength, bool& bSameDirection);

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

// #100617# Allow to skip doubles, too.
void Imp_SkipDoubleAndSpacesAndCommas(const OUString& rStr, sal_Int32& rPos,
    const sal_Int32 nLen)
{
    Imp_SkipDouble(rStr, rPos, nLen);
    Imp_SkipSpacesAndCommas(rStr, rPos, nLen);
}

void Imp_PutNumberChar(OUString& rStr, sal_Int32 nValue)
{
    OUStringBuffer sStringBuffer;
    SvXMLUnitConverter::convertNumber(sStringBuffer, nValue);
    rStr += OUString(sStringBuffer.makeStringAndClear());
}

void Imp_PutDoubleChar(OUString& rStr, double fValue)
{
    OUStringBuffer sStringBuffer;
    SvXMLUnitConverter::convertDouble(sStringBuffer, fValue);
    rStr += OUString(sStringBuffer.makeStringAndClear());
}

void Imp_PutNumberCharWithSpace(OUString& rStr, sal_Int32 nValue)
{
    const sal_Int32 aLen(rStr.getLength());
    if(aLen)
        if(Imp_IsOnNumberChar(rStr, aLen - 1, false) && nValue >= 0)
            rStr += String(sal_Unicode(' '));
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
            rConv.convertDoubleAndUnit(fRetval, sNumberString.makeStringAndClear());
        else
            rConv.convertDouble(fRetval, sNumberString.makeStringAndClear());
    }

    return fRetval;
}

void Imp_PutDoubleChar(OUString& rStr, const SvXMLUnitConverter& rConv, double fValue,
    bool bConvertUnits = false)
{
    OUStringBuffer sStringBuffer;

    if(bConvertUnits)
        rConv.convertDoubleAndUnit(sStringBuffer, fValue);
    else
        rConv.convertDouble(sStringBuffer, fValue);

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
                DBG_ERROR("SdXMLImExTransform2D: impossible entry!");
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

void SdXMLImExTransform2D::AddScale(const ::basegfx::B2DTuple& rNew)
{
    if(1.0 != rNew.getX() || 1.0 != rNew.getY())
        maList.push_back(new ImpSdXMLExpTransObj2DScale(rNew));
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

void SdXMLImExTransform2D::AddSkewY(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj2DSkewY(fNew));
}

void SdXMLImExTransform2D::AddMatrix(const ::basegfx::B2DHomMatrix& rNew)
{
    if(!rNew.isIdentity())
        maList.push_back(new ImpSdXMLExpTransObj2DMatrix(rNew));
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
                aNewString += OUString::createFromAscii("rotate (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DRotate*)pObj)->mfRotate);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                aNewString += OUString::createFromAscii("scale (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.getX());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.getY());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                aNewString += OUString::createFromAscii("translate (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.getX(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.getY(), true);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                aNewString += OUString::createFromAscii("skewX (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                aNewString += OUString::createFromAscii("skewY (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX :
            {
                aNewString += OUString::createFromAscii("matrix (");

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
                DBG_ERROR("SdXMLImExTransform2D: impossible entry!");
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
SdXMLImExTransform2D::SdXMLImExTransform2D(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    SetString(rNew, rConv);
}

//////////////////////////////////////////////////////////////////////////////
// sets new string, parses it and generates entries
void SdXMLImExTransform2D::SetString(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    msString = rNew;
    EmptyList();

    if(msString.getLength())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());

        const OUString aString_rotate(OUString::createFromAscii("rotate"));
        const OUString aString_scale(OUString::createFromAscii("scale"));
        const OUString aString_translate(OUString::createFromAscii("translate"));
        const OUString aString_skewX(OUString::createFromAscii("skewX"));
        const OUString aString_skewY(OUString::createFromAscii("skewY"));
        const OUString aString_matrix(OUString::createFromAscii("matrix"));

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
                rFullTrans.scale(rScale);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                const ::basegfx::B2DTuple& rTranslate = ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate;
                rFullTrans.translate(rTranslate);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                // correct shear by mirroring; ODF 1.3 uses mirrored values
                rFullTrans.shearX(tan(((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX * -1.0));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                // correct shear by mirroring; ODF 1.3 uses mirrored values
                rFullTrans.shearY(tan(((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY * -1.0));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX     :
            {
                rFullTrans *= ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix;
                break;
            }
            default :
            {
                DBG_ERROR("SdXMLImExTransform2D: impossible entry!");
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
                DBG_ERROR("SdXMLImExTransform3D: impossible entry!");
                break;
            }
        }
    }

    maList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// add members

void SdXMLImExTransform3D::AddRotateX(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj3DRotateX(fNew));
}

void SdXMLImExTransform3D::AddRotateY(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj3DRotateY(fNew));
}

void SdXMLImExTransform3D::AddRotateZ(double fNew)
{
    if(fNew != 0.0)
        maList.push_back(new ImpSdXMLExpTransObj3DRotateZ(fNew));
}

void SdXMLImExTransform3D::AddScale(const ::basegfx::B3DTuple& rNew)
{
    if(1.0 != rNew.getX() || 1.0 != rNew.getY() || 1.0 != rNew.getZ())
        maList.push_back(new ImpSdXMLExpTransObj3DScale(rNew));
}

void SdXMLImExTransform3D::AddTranslate(const ::basegfx::B3DTuple& rNew)
{
    if(!rNew.equalZero())
        maList.push_back(new ImpSdXMLExpTransObj3DTranslate(rNew));
}

void SdXMLImExTransform3D::AddMatrix(const ::basegfx::B3DHomMatrix& rNew)
{
    if(!rNew.isIdentity())
        maList.push_back(new ImpSdXMLExpTransObj3DMatrix(rNew));
}

void SdXMLImExTransform3D::AddHomogenMatrix(const drawing::HomogenMatrix& xHomMat)
{
    AddMatrix(basegfx::tools::UnoHomogenMatrixToB3DHomMatrix(xHomMat));
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
                aNewString += OUString::createFromAscii("rotatex (");
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( ((ImpSdXMLExpTransObj3DRotateX*)pObj)->mfRotateX) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                aNewString += OUString::createFromAscii("rotatey (");
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( ((ImpSdXMLExpTransObj3DRotateY*)pObj)->mfRotateY) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                aNewString += OUString::createFromAscii("rotatez (");
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( ((ImpSdXMLExpTransObj3DRotateZ*)pObj)->mfRotateZ) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                aNewString += OUString::createFromAscii("scale (");
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
                aNewString += OUString::createFromAscii("translate (");
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
                aNewString += OUString::createFromAscii("matrix (");

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
                DBG_ERROR("SdXMLImExTransform3D: impossible entry!");
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

    if(msString.getLength())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());

        const OUString aString_rotatex(OUString::createFromAscii("rotatex"));
        const OUString aString_rotatey(OUString::createFromAscii("rotatey"));
        const OUString aString_rotatez(OUString::createFromAscii("rotatez"));
        const OUString aString_scale(OUString::createFromAscii("scale"));
        const OUString aString_translate(OUString::createFromAscii("translate"));
        const OUString aString_matrix(OUString::createFromAscii("matrix"));

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
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateX(basegfx::deg2rad(fValue)));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatey, nPos))
                {
                    double fValue(0.0);

                    nPos += 7;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateY(basegfx::deg2rad(fValue)));

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatez, nPos))
                {
                    double fValue(0.0);

                    nPos += 7;
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.push_back(new ImpSdXMLExpTransObj3DRotateZ(basegfx::deg2rad(fValue)));

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
    basegfx::B3DHomMatrix aFullTransform;
    GetFullTransform(aFullTransform);

    if(!aFullTransform.isIdentity())
    {
        basegfx::tools::B3DHomMatrixToUnoHomogenMatrix(aFullTransform, xHomMat);
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
                rFullTrans.scale(rScale);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                const ::basegfx::B3DTuple& rTranslate = ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate;
                rFullTrans.translate(rTranslate);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX     :
            {
                rFullTrans *= ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix;
                break;
            }
            default :
            {
                DBG_ERROR("SdXMLImExTransform3D: impossible entry!");
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImExViewBox::SdXMLImExViewBox(double fX, double fY, double fW, double fH)
:   mfX( fX ),
    mfY( fY ),
    mfW( fW ),
    mfH( fH )
{
}

// #100617# Asked vincent hardy: svg:viewBox values may be double precision.
SdXMLImExViewBox::SdXMLImExViewBox(const OUString& rNew, const SvXMLUnitConverter& rConv)
:   msString(rNew),
    mfX( 0.0 ),
    mfY( 0.0 ),
    mfW( 1000.0 ),
    mfH( 1000.0 )
{
    if(msString.getLength())
    {
        const OUString aStr(msString.getStr(), (sal_uInt16)msString.getLength());
        const sal_Int32 nLen(aStr.getLength());
        sal_Int32 nPos(0);

        // skip starting spaces
        Imp_SkipSpaces(aStr, nPos, nLen);

        // get mX, #100617# be prepared for doubles
        mfX = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, mfX);

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mY, #100617# be prepared for doubles
        mfY = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, mfY);

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mW, #100617# be prepared for doubles
        mfW = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, mfW);

        // skip spaces and commas
        Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

        // get mH, #100617# be prepared for doubles
        mfH = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, mfH);
    }
}

const OUString& SdXMLImExViewBox::GetExportString()
{
    OUString aNewString;
    OUString aEmptySpace(sal_Unicode(' '));

    Imp_PutDoubleChar(aNewString, mfX);
    aNewString += aEmptySpace;

    Imp_PutDoubleChar(aNewString, mfY);
    aNewString += aEmptySpace;

    Imp_PutDoubleChar(aNewString, mfW);
    aNewString += aEmptySpace;

    Imp_PutDoubleChar(aNewString, mfH);

    // set new string
    msString = aNewString;

    return msString;
}

// eof
