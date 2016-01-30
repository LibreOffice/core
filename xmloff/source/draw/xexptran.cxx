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

#include "xexptran.hxx"
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
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/unotools.hxx>

using namespace ::com::sun::star;

// parsing help functions for simple chars
void Imp_SkipSpaces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && ' ' == rStr[rPos])
        rPos++;
}

void Imp_SkipSpacesAndOpeningBraces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (' ' == rStr[rPos] || '(' == rStr[rPos]))
        rPos++;
}

void Imp_SkipSpacesAndCommas(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (' ' == rStr[rPos] || ',' == rStr[rPos]))
        rPos++;
}

void Imp_SkipSpacesAndClosingBraces(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    while(rPos < nLen
        && (' ' == rStr[rPos] || ')' == rStr[rPos]))
        rPos++;
}

// parsing help functions for integer numbers

bool Imp_IsOnUnitChar(const OUString& rStr, const sal_Int32 nPos)
{
    sal_Unicode aChar(rStr[nPos]);

    if(('a' <= aChar && 'z' >= aChar)
        || ('A' <= aChar && 'Z' >= aChar)
        || '%' == aChar
    )
        return true;
    return false;
}

double Imp_GetDoubleChar(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen,
    const SvXMLUnitConverter& rConv, double fRetval, bool bLookForUnits = false)
{
    sal_Unicode aChar(rStr[rPos]);
    OUStringBuffer sNumberString;

    if('+' == aChar || '-' == aChar)
    {
        sNumberString.append(rStr[rPos]);
        ++rPos;
        aChar = rPos >= nLen ? 0 : rStr[rPos];
    }

    while(('0' <= aChar && '9' >= aChar)
        || '.' == aChar)
    {
        sNumberString.append(rStr[rPos]);
        ++rPos;
        aChar = rPos >= nLen ? 0 : rStr[rPos];
    }

    if('e' == aChar || 'E' == aChar)
    {
        sNumberString.append(rStr[rPos]);
        ++rPos;
        aChar = rPos >= nLen ? 0 : rStr[rPos];

        if('+' == aChar || '-' == aChar)
        {
            sNumberString.append(rStr[rPos]);
            ++rPos;
            aChar = rPos >= nLen ? 0 : rStr[rPos];
        }

        while('0' <= aChar && '9' >= aChar)
        {
            sNumberString.append(rStr[rPos]);
            ++rPos;
            aChar = rPos >= nLen ? 0 : rStr[rPos];
        }
    }

    if(bLookForUnits)
    {
        Imp_SkipSpaces(rStr, rPos, nLen);
        while(rPos < nLen && Imp_IsOnUnitChar(rStr, rPos))
            sNumberString.append(rStr[rPos++]);
    }

    if(!sNumberString.isEmpty())
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

void Imp_PutDoubleChar(OUString& rStr, double fValue)
{
    OUStringBuffer sStringBuffer;
    ::sax::Converter::convertDouble(sStringBuffer, fValue);
    rStr += sStringBuffer.makeStringAndClear();
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

    rStr += sStringBuffer.makeStringAndClear();
}

// base class of all 2D transform objects

struct ImpSdXMLExpTransObj2DBase
{
    sal_uInt16                  mnType;
    explicit ImpSdXMLExpTransObj2DBase(sal_uInt16 nType)
    :   mnType(nType) {}
};

// possible object types for 2D

#define IMP_SDXMLEXP_TRANSOBJ2D_ROTATE          0x0000
#define IMP_SDXMLEXP_TRANSOBJ2D_SCALE           0x0001
#define IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE       0x0002
#define IMP_SDXMLEXP_TRANSOBJ2D_SKEWX           0x0003
#define IMP_SDXMLEXP_TRANSOBJ2D_SKEWY           0x0004
#define IMP_SDXMLEXP_TRANSOBJ2D_MATRIX          0x0005

// classes of objects, different sizes

struct ImpSdXMLExpTransObj2DRotate : public ImpSdXMLExpTransObj2DBase
{
    double                      mfRotate;
    explicit ImpSdXMLExpTransObj2DRotate(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_ROTATE), mfRotate(fVal) {}
};
struct ImpSdXMLExpTransObj2DScale : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DTuple         maScale;
    explicit ImpSdXMLExpTransObj2DScale(const ::basegfx::B2DTuple& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj2DTranslate : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DTuple         maTranslate;
    explicit ImpSdXMLExpTransObj2DTranslate(const ::basegfx::B2DTuple& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj2DSkewX : public ImpSdXMLExpTransObj2DBase
{
    double                      mfSkewX;
    explicit ImpSdXMLExpTransObj2DSkewX(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWX), mfSkewX(fVal) {}
};
struct ImpSdXMLExpTransObj2DSkewY : public ImpSdXMLExpTransObj2DBase
{
    double                      mfSkewY;
    explicit ImpSdXMLExpTransObj2DSkewY(double fVal)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWY), mfSkewY(fVal) {}
};
struct ImpSdXMLExpTransObj2DMatrix : public ImpSdXMLExpTransObj2DBase
{
    ::basegfx::B2DHomMatrix     maMatrix;
    explicit ImpSdXMLExpTransObj2DMatrix(const ::basegfx::B2DHomMatrix& rNew)
    :   ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_MATRIX), maMatrix(rNew) {}
};

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
                delete static_cast<ImpSdXMLExpTransObj2DRotate*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                delete static_cast<ImpSdXMLExpTransObj2DScale*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                delete static_cast<ImpSdXMLExpTransObj2DTranslate*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                delete static_cast<ImpSdXMLExpTransObj2DSkewX*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                delete static_cast<ImpSdXMLExpTransObj2DSkewY*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX     :
            {
                delete static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj);
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

// gen string for export
const OUString& SdXMLImExTransform2D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(")");
    OUString aEmptySpace(" ");

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE :
            {
                aNewString += "rotate (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DRotate*>(pObj)->mfRotate);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                aNewString += "scale (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DScale*>(pObj)->maScale.getX());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DScale*>(pObj)->maScale.getY());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                aNewString += "translate (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DTranslate*>(pObj)->maTranslate.getX(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DTranslate*>(pObj)->maTranslate.getY(), true);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                aNewString += "skewX (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DSkewX*>(pObj)->mfSkewX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                aNewString += "skewY (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DSkewY*>(pObj)->mfSkewY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX :
            {
                aNewString += "matrix (";

                // a
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(0, 0));
                aNewString += aEmptySpace;

                // b
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(1, 0));
                aNewString += aEmptySpace;

                // c
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(0, 1));
                aNewString += aEmptySpace;

                // d
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(1, 1));
                aNewString += aEmptySpace;

                // e
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(0, 2), true);
                aNewString += aEmptySpace;

                // f
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix.get(1, 2), true);

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
                rFullTrans.rotate(static_cast<ImpSdXMLExpTransObj2DRotate*>(pObj)->mfRotate * -1.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE      :
            {
                const ::basegfx::B2DTuple& rScale = static_cast<ImpSdXMLExpTransObj2DScale*>(pObj)->maScale;
                rFullTrans.scale(rScale.getX(), rScale.getY());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE  :
            {
                const ::basegfx::B2DTuple& rTranslate = static_cast<ImpSdXMLExpTransObj2DTranslate*>(pObj)->maTranslate;
                rFullTrans.translate(rTranslate.getX(), rTranslate.getY());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX      :
            {
                rFullTrans.shearX(tan(static_cast<ImpSdXMLExpTransObj2DSkewX*>(pObj)->mfSkewX));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY      :
            {
                rFullTrans.shearY(tan(static_cast<ImpSdXMLExpTransObj2DSkewY*>(pObj)->mfSkewY));
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX     :
            {
                rFullTrans *= static_cast<ImpSdXMLExpTransObj2DMatrix*>(pObj)->maMatrix;
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

// base class of all 3D transform objects

struct ImpSdXMLExpTransObj3DBase
{
    sal_uInt16                  mnType;
    explicit ImpSdXMLExpTransObj3DBase(sal_uInt16 nType)
    :   mnType(nType) {}
};

// possible object types for 3D

#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X        0x0000
#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y        0x0001
#define IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z        0x0002
#define IMP_SDXMLEXP_TRANSOBJ3D_SCALE           0x0003
#define IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE       0x0004
#define IMP_SDXMLEXP_TRANSOBJ3D_MATRIX          0x0005

// classes of objects, different sizes

struct ImpSdXMLExpTransObj3DRotateX : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateX;
    explicit ImpSdXMLExpTransObj3DRotateX(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X), mfRotateX(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateY : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateY;
    explicit ImpSdXMLExpTransObj3DRotateY(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y), mfRotateY(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateZ : public ImpSdXMLExpTransObj3DBase
{
    double                      mfRotateZ;
    explicit ImpSdXMLExpTransObj3DRotateZ(double fVal)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z), mfRotateZ(fVal) {}
};
struct ImpSdXMLExpTransObj3DScale : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DTuple         maScale;
    explicit ImpSdXMLExpTransObj3DScale(const ::basegfx::B3DTuple& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj3DTranslate : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DTuple         maTranslate;
    explicit ImpSdXMLExpTransObj3DTranslate(const ::basegfx::B3DTuple& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj3DMatrix : public ImpSdXMLExpTransObj3DBase
{
    ::basegfx::B3DHomMatrix     maMatrix;
    explicit ImpSdXMLExpTransObj3DMatrix(const ::basegfx::B3DHomMatrix& rNew)
    :   ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_MATRIX), maMatrix(rNew) {}
};

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
                delete static_cast<ImpSdXMLExpTransObj3DRotateX*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                delete static_cast<ImpSdXMLExpTransObj3DRotateY*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                delete static_cast<ImpSdXMLExpTransObj3DRotateZ*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                delete static_cast<ImpSdXMLExpTransObj3DScale*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                delete static_cast<ImpSdXMLExpTransObj3DTranslate*>(pObj);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX     :
            {
                delete static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj);
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

// gen string for export
const OUString& SdXMLImExTransform3D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(")");
    OUString aEmptySpace(" ");

    const sal_uInt32 nCount = maList.size();
    for(sal_uInt32 a(0L); a < nCount; a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList[a];
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X   :
            {
                aNewString += "rotatex (";
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( static_cast<ImpSdXMLExpTransObj3DRotateX*>(pObj)->mfRotateX) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                aNewString += "rotatey (";
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( static_cast<ImpSdXMLExpTransObj3DRotateY*>(pObj)->mfRotateY) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                aNewString += "rotatez (";
                Imp_PutDoubleChar(aNewString, rConv, basegfx::rad2deg( static_cast<ImpSdXMLExpTransObj3DRotateZ*>(pObj)->mfRotateZ) );
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                aNewString += "scale (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DScale*>(pObj)->maScale.getX());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DScale*>(pObj)->maScale.getY());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DScale*>(pObj)->maScale.getZ());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                aNewString += "translate (";
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DTranslate*>(pObj)->maTranslate.getX(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DTranslate*>(pObj)->maTranslate.getY(), true);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DTranslate*>(pObj)->maTranslate.getZ(), true);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX :
            {
                aNewString += "matrix (";

                // a
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(0, 0));
                aNewString += aEmptySpace;

                // b
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(1, 0));
                aNewString += aEmptySpace;

                // c
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(2, 0));
                aNewString += aEmptySpace;

                // d
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(0, 1));
                aNewString += aEmptySpace;

                // e
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(1, 1));
                aNewString += aEmptySpace;

                // f
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(2, 1));
                aNewString += aEmptySpace;

                // g
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(0, 2));
                aNewString += aEmptySpace;

                // h
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(1, 2));
                aNewString += aEmptySpace;

                // i
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(2, 2));
                aNewString += aEmptySpace;

                // j
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(0, 3), true);
                aNewString += aEmptySpace;

                // k
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(1, 3), true);
                aNewString += aEmptySpace;

                // l
                Imp_PutDoubleChar(aNewString, rConv, static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix.get(2, 3), true);

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

// for Import: constructor with string, parses it and generates entries
SdXMLImExTransform3D::SdXMLImExTransform3D(const OUString& rNew, const SvXMLUnitConverter& rConv)
{
    SetString(rNew, rConv);
}

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
                rFullTrans.rotate(static_cast<ImpSdXMLExpTransObj3DRotateX*>(pObj)->mfRotateX, 0.0, 0.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y   :
            {
                rFullTrans.rotate(0.0, static_cast<ImpSdXMLExpTransObj3DRotateY*>(pObj)->mfRotateY, 0.0);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z   :
            {
                rFullTrans.rotate(0.0, 0.0, static_cast<ImpSdXMLExpTransObj3DRotateZ*>(pObj)->mfRotateZ);
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE      :
            {
                const ::basegfx::B3DTuple& rScale = static_cast<ImpSdXMLExpTransObj3DScale*>(pObj)->maScale;
                rFullTrans.scale(rScale.getX(), rScale.getY(), rScale.getZ());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE  :
            {
                const ::basegfx::B3DTuple& rTranslate = static_cast<ImpSdXMLExpTransObj3DTranslate*>(pObj)->maTranslate;
                rFullTrans.translate(rTranslate.getX(), rTranslate.getY(), rTranslate.getZ());
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX     :
            {
                rFullTrans *= static_cast<ImpSdXMLExpTransObj3DMatrix*>(pObj)->maMatrix;
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
    if(!msString.isEmpty())
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
    OUString aEmptySpace(" ");

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
