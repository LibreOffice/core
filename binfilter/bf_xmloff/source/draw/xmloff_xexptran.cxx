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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _B3D_HMATRIX_HXX
#include <bf_goodies/hmatrix.hxx>
#endif

// #100617# FRound
#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// Defines

#define BORDER_INTEGERS_ARE_EQUAL		(4)

//////////////////////////////////////////////////////////////////////////////
// Predeclarations

void Imp_SkipDouble(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen);
void Imp_CalcVectorValues(Vector2D& aVec1, Vector2D& aVec2, sal_Bool& bSameLength, sal_Bool& bSameDirection);

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

sal_Bool Imp_IsOnNumberChar(const OUString& rStr, const sal_Int32 nPos, BOOL bSignAllowed = TRUE)
{
    sal_Unicode aChar(rStr[nPos]);

    if((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
        || (bSignAllowed && sal_Unicode('+') == aChar)
        || (bSignAllowed && sal_Unicode('-') == aChar)
    )
        return TRUE;
    return FALSE;
}

sal_Bool Imp_IsOnUnitChar(const OUString& rStr, const sal_Int32 nPos)
{
    sal_Unicode aChar(rStr[nPos]);

    if((sal_Unicode('a') <= aChar && sal_Unicode('z') >= aChar)
        || (sal_Unicode('A') <= aChar && sal_Unicode('Z') >= aChar)
        || sal_Unicode('%') == aChar
    )
        return TRUE;
    return FALSE;
}

void Imp_SkipNumber(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
{
    BOOL bSignAllowed(TRUE);

    while(rPos < nLen && Imp_IsOnNumberChar(rStr, rPos, bSignAllowed))
    {
        bSignAllowed = FALSE;
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

void Imp_PutNumberChar(OUString& rStr, const SvXMLUnitConverter& rConv, sal_Int32 nValue)
{
    OUStringBuffer sStringBuffer;
    rConv.convertNumber(sStringBuffer, nValue);
    rStr += OUString(sStringBuffer.makeStringAndClear());
}

void Imp_PutNumberCharWithSpace(OUString& rStr, const SvXMLUnitConverter& rConv, sal_Int32 nValue)
{
    const sal_Int32 aLen(rStr.getLength());
    if(aLen)
        if(Imp_IsOnNumberChar(rStr, aLen - 1, FALSE) && nValue >= 0)
            rStr += OUString(sal_Unicode(' '));
    Imp_PutNumberChar(rStr, rConv, nValue);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// parsing help functions for double numbers

void Imp_SkipDouble(const OUString& rStr, sal_Int32& rPos, const sal_Int32 nLen)
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
    const SvXMLUnitConverter& rConv, double fRetval, BOOL bLookForUnits = FALSE)
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
            rConv.convertDouble(fRetval, sNumberString.makeStringAndClear(), TRUE);
        else
            rConv.convertDouble(fRetval, sNumberString.makeStringAndClear());
    }

    return fRetval;
}

void Imp_PutDoubleChar(OUString& rStr, const SvXMLUnitConverter& rConv, double fValue, 
    BOOL bConvertUnits = FALSE)
{
    OUStringBuffer sStringBuffer;

    if(bConvertUnits)
        rConv.convertDouble(sStringBuffer, fValue, TRUE);
    else
        rConv.convertDouble(sStringBuffer, fValue);

    rStr += OUString(sStringBuffer.makeStringAndClear());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// base class of all 2D transform objects

struct ImpSdXMLExpTransObj2DBase
{
    sal_uInt16					mnType;
    ImpSdXMLExpTransObj2DBase(sal_uInt16 nType) 
    :	mnType(nType) {}
};

//////////////////////////////////////////////////////////////////////////////
// possible object types for 2D

#define	IMP_SDXMLEXP_TRANSOBJ2D_ROTATE			0x0000
#define	IMP_SDXMLEXP_TRANSOBJ2D_SCALE			0x0001
#define	IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE		0x0002
#define	IMP_SDXMLEXP_TRANSOBJ2D_SKEWX			0x0003
#define	IMP_SDXMLEXP_TRANSOBJ2D_SKEWY			0x0004
#define	IMP_SDXMLEXP_TRANSOBJ2D_MATRIX			0x0005

//////////////////////////////////////////////////////////////////////////////
// classes of objects, different sizes

struct ImpSdXMLExpTransObj2DRotate : public ImpSdXMLExpTransObj2DBase
{
    double						mfRotate;
    ImpSdXMLExpTransObj2DRotate(double fVal) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_ROTATE), mfRotate(fVal) {}
};
struct ImpSdXMLExpTransObj2DScale : public ImpSdXMLExpTransObj2DBase
{
    Vector2D					maScale;
    ImpSdXMLExpTransObj2DScale(const Vector2D& rNew) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj2DTranslate : public ImpSdXMLExpTransObj2DBase
{
    Vector2D					maTranslate;
    ImpSdXMLExpTransObj2DTranslate(const Vector2D& rNew) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj2DSkewX : public ImpSdXMLExpTransObj2DBase
{
    double						mfSkewX;
    ImpSdXMLExpTransObj2DSkewX(double fVal) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWX), mfSkewX(fVal) {}
};
struct ImpSdXMLExpTransObj2DSkewY : public ImpSdXMLExpTransObj2DBase
{
    double						mfSkewY;
    ImpSdXMLExpTransObj2DSkewY(double fVal) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_SKEWY), mfSkewY(fVal) {}
};
struct ImpSdXMLExpTransObj2DMatrix : public ImpSdXMLExpTransObj2DBase
{
    Matrix3D					maMatrix;
    ImpSdXMLExpTransObj2DMatrix(const Matrix3D& rNew) 
    :	ImpSdXMLExpTransObj2DBase(IMP_SDXMLEXP_TRANSOBJ2D_MATRIX), maMatrix(rNew) {}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// delete all entries in list

void SdXMLImExTransform2D::EmptyList()
{
    while(maList.Count())
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList.Remove(maList.Count() - 1);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE		: delete (ImpSdXMLExpTransObj2DRotate*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE		: delete (ImpSdXMLExpTransObj2DScale*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE	: delete (ImpSdXMLExpTransObj2DTranslate*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX		: delete (ImpSdXMLExpTransObj2DSkewX*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY		: delete (ImpSdXMLExpTransObj2DSkewY*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX		: delete (ImpSdXMLExpTransObj2DMatrix*)pObj; break;
            default : DBG_ERROR("SdXMLImExTransform2D: impossible entry!"); break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// add members

void SdXMLImExTransform2D::AddRotate(double fNew)
{
    if(fNew != 0.0)
        maList.Insert(new ImpSdXMLExpTransObj2DRotate(fNew), LIST_APPEND);
}

void SdXMLImExTransform2D::AddTranslate(const Vector2D& rNew)
{
    if(rNew.X() != 0.0 || rNew.Y() != 0.0)
        maList.Insert(new ImpSdXMLExpTransObj2DTranslate(rNew), LIST_APPEND);
}

void SdXMLImExTransform2D::AddSkewX(double fNew)
{
    if(fNew != 0.0)
        maList.Insert(new ImpSdXMLExpTransObj2DSkewX(fNew), LIST_APPEND);
}

//////////////////////////////////////////////////////////////////////////////
// gen string for export
const OUString& SdXMLImExTransform2D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(sal_Unicode(')'));
    OUString aEmptySpace(sal_Unicode(' '));
    
    for(sal_uInt32 a(0L); a < maList.Count(); a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList.GetObject(a);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE	: 
            {
                aNewString += OUString::createFromAscii("rotate (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DRotate*)pObj)->mfRotate);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE		: 
            {
                aNewString += OUString::createFromAscii("scale (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.X());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DScale*)pObj)->maScale.Y());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE	: 
            {
                aNewString += OUString::createFromAscii("translate (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.X(), TRUE);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate.Y(), TRUE);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX		: 
            {
                aNewString += OUString::createFromAscii("skewX (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY		: 
            {
                aNewString += OUString::createFromAscii("skewY (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX	: 
            {
                aNewString += OUString::createFromAscii("matrix (");

                // a
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[0][0]);
                aNewString += aEmptySpace;

                // b
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[1][0]);
                aNewString += aEmptySpace;

                // c
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[0][1]);
                aNewString += aEmptySpace;

                // d
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[1][1]);
                aNewString += aEmptySpace;

                // e
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[0][2], TRUE);
                aNewString += aEmptySpace;

                // f
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix[1][2], TRUE);

                aNewString += aClosingBrace;
                break;
            }
            default : DBG_ERROR("SdXMLImExTransform2D: impossible entry!"); break;
        }

        // if not the last entry, add one space to next tag
        if(a+1 != maList.Count())
            aNewString += aEmptySpace;
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
                        maList.Insert(new ImpSdXMLExpTransObj2DRotate(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_scale, nPos))
                { 
                    Vector2D aValue(1.0, 1.0);
                    nPos += 5; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.X() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.X());
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Y() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Y());

                    if(aValue.X() != 1.0 || aValue.Y() != 1.0)
                        maList.Insert(new ImpSdXMLExpTransObj2DScale(aValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_translate, nPos))
                { 
                    Vector2D aValue;
                    nPos += 9; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.X() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.X(), TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Y() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Y(), TRUE);

                    if(aValue.X() != 0.0 || aValue.Y() != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj2DTranslate(aValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_skewX, nPos))
                { 
                    double fValue(0.0);
                    nPos += 5; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj2DSkewX(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_skewY, nPos))
                { 
                    double fValue(0.0);
                    nPos += 5; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj2DSkewY(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_matrix, nPos))
                { 
                    Matrix3D aValue;

                    nPos += 6; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);

                    // a
                    aValue[0][0] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][0]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // b
                    aValue[1][0] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][0]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // c
                    aValue[0][1] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][1]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // d
                    aValue[1][1] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][1]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // e
                    aValue[0][2] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][2], TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // f
                    aValue[1][2] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][2], TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    maList.Insert(new ImpSdXMLExpTransObj2DMatrix(aValue), LIST_APPEND);
                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else
                    nPos++;
            }
        }
    }
}

void SdXMLImExTransform2D::GetFullTransform(Matrix3D& rFullTrans)
{
    rFullTrans.Identity();

    for(sal_uInt32 a(0L); a < maList.Count(); a++)
    {
        ImpSdXMLExpTransObj2DBase* pObj = maList.GetObject(a);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ2D_ROTATE		: 
                rFullTrans.Rotate(((ImpSdXMLExpTransObj2DRotate*)pObj)->mfRotate); 
                break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SCALE		: 
                rFullTrans.Scale(((ImpSdXMLExpTransObj2DScale*)pObj)->maScale); 
                break;
            case IMP_SDXMLEXP_TRANSOBJ2D_TRANSLATE	: 
                rFullTrans.Translate(((ImpSdXMLExpTransObj2DTranslate*)pObj)->maTranslate); 
                break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWX		: 
                rFullTrans.ShearX(tan(((ImpSdXMLExpTransObj2DSkewX*)pObj)->mfSkewX)); 
                break;
            case IMP_SDXMLEXP_TRANSOBJ2D_SKEWY		: 
                rFullTrans.ShearY(tan(((ImpSdXMLExpTransObj2DSkewY*)pObj)->mfSkewY)); 
                break;
            case IMP_SDXMLEXP_TRANSOBJ2D_MATRIX		: 
                rFullTrans *= ((ImpSdXMLExpTransObj2DMatrix*)pObj)->maMatrix; 
                break;
            default : 
                DBG_ERROR("SdXMLImExTransform2D: impossible entry!"); 
                break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// base class of all 3D transform objects

struct ImpSdXMLExpTransObj3DBase
{
    sal_uInt16					mnType;
    ImpSdXMLExpTransObj3DBase(sal_uInt16 nType) 
    :	mnType(nType) {}
};

//////////////////////////////////////////////////////////////////////////////
// possible object types for 3D

#define	IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X		0x0000
#define	IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y		0x0001
#define	IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z		0x0002
#define	IMP_SDXMLEXP_TRANSOBJ3D_SCALE			0x0003
#define	IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE		0x0004
#define	IMP_SDXMLEXP_TRANSOBJ3D_MATRIX			0x0005

//////////////////////////////////////////////////////////////////////////////
// classes of objects, different sizes

struct ImpSdXMLExpTransObj3DRotateX : public ImpSdXMLExpTransObj3DBase
{
    double						mfRotateX;
    ImpSdXMLExpTransObj3DRotateX(double fVal) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X), mfRotateX(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateY : public ImpSdXMLExpTransObj3DBase
{
    double						mfRotateY;
    ImpSdXMLExpTransObj3DRotateY(double fVal) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y), mfRotateY(fVal) {}
};
struct ImpSdXMLExpTransObj3DRotateZ : public ImpSdXMLExpTransObj3DBase
{
    double						mfRotateZ;
    ImpSdXMLExpTransObj3DRotateZ(double fVal) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z), mfRotateZ(fVal) {}
};
struct ImpSdXMLExpTransObj3DScale : public ImpSdXMLExpTransObj3DBase
{
    Vector3D					maScale;
    ImpSdXMLExpTransObj3DScale(const Vector3D& rNew) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_SCALE), maScale(rNew) {}
};
struct ImpSdXMLExpTransObj3DTranslate : public ImpSdXMLExpTransObj3DBase
{
    Vector3D					maTranslate;
    ImpSdXMLExpTransObj3DTranslate(const Vector3D& rNew) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE), maTranslate(rNew) {}
};
struct ImpSdXMLExpTransObj3DMatrix : public ImpSdXMLExpTransObj3DBase
{
    Matrix4D					maMatrix;
    ImpSdXMLExpTransObj3DMatrix(const Matrix4D& rNew) 
    :	ImpSdXMLExpTransObj3DBase(IMP_SDXMLEXP_TRANSOBJ3D_MATRIX), maMatrix(rNew) {}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// delete all entries in list

void SdXMLImExTransform3D::EmptyList()
{
    while(maList.Count())
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList.Remove(maList.Count() - 1);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X	: delete (ImpSdXMLExpTransObj3DRotateX*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y	: delete (ImpSdXMLExpTransObj3DRotateY*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z	: delete (ImpSdXMLExpTransObj3DRotateZ*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE		: delete (ImpSdXMLExpTransObj3DScale*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE	: delete (ImpSdXMLExpTransObj3DTranslate*)pObj; break;
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX		: delete (ImpSdXMLExpTransObj3DMatrix*)pObj; break;
            default : DBG_ERROR("SdXMLImExTransform3D: impossible entry!"); break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// add members

void SdXMLImExTransform3D::AddMatrix(const Matrix4D& rNew)
{
    if(rNew[0][0] != 1.0 || rNew[1][1] != 1.0 || rNew[2][2] != 1.0
        || rNew[0][1] != 0.0 || rNew[0][2] != 0.0 || rNew[0][3] != 0.0 
        || rNew[1][0] != 0.0 || rNew[1][2] != 0.0 || rNew[1][3] != 0.0 
        || rNew[2][0] != 0.0 || rNew[2][1] != 0.0 || rNew[2][3] != 0.0)
        maList.Insert(new ImpSdXMLExpTransObj3DMatrix(rNew), LIST_APPEND);
}

void SdXMLImExTransform3D::AddHomogenMatrix(const drawing::HomogenMatrix& xHomMat)
{
    Matrix4D aExportMatrix;

    aExportMatrix[0][0] = xHomMat.Line1.Column1;
    aExportMatrix[0][1] = xHomMat.Line1.Column2;
    aExportMatrix[0][2] = xHomMat.Line1.Column3;
    aExportMatrix[0][3] = xHomMat.Line1.Column4;
    aExportMatrix[1][0] = xHomMat.Line2.Column1;
    aExportMatrix[1][1] = xHomMat.Line2.Column2;
    aExportMatrix[1][2] = xHomMat.Line2.Column3;
    aExportMatrix[1][3] = xHomMat.Line2.Column4;
    aExportMatrix[2][0] = xHomMat.Line3.Column1;
    aExportMatrix[2][1] = xHomMat.Line3.Column2;
    aExportMatrix[2][2] = xHomMat.Line3.Column3;
    aExportMatrix[2][3] = xHomMat.Line3.Column4;

    AddMatrix(aExportMatrix);
}

//////////////////////////////////////////////////////////////////////////////
// gen string for export
const OUString& SdXMLImExTransform3D::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aClosingBrace(sal_Unicode(')'));
    OUString aEmptySpace(sal_Unicode(' '));
    
    for(sal_uInt32 a(0L); a < maList.Count(); a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList.GetObject(a);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X	: 
            {
                aNewString += OUString::createFromAscii("rotatex (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateX*)pObj)->mfRotateX);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y	: 
            {
                aNewString += OUString::createFromAscii("rotatey (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateY*)pObj)->mfRotateY);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z	: 
            {
                aNewString += OUString::createFromAscii("rotatez (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DRotateZ*)pObj)->mfRotateZ);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE		: 
            {
                aNewString += OUString::createFromAscii("scale (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.X());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.Y());
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DScale*)pObj)->maScale.Z());
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE	: 
            {
                aNewString += OUString::createFromAscii("translate (");
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.X(), TRUE);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.Y(), TRUE);
                aNewString += aEmptySpace;
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate.Z(), TRUE);
                aNewString += aClosingBrace;
                break;
            }
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX	: 
            {
                aNewString += OUString::createFromAscii("matrix (");

                // a
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[0][0]);
                aNewString += aEmptySpace;
                
                // b
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[1][0]);
                aNewString += aEmptySpace;
                
                // c
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[2][0]);
                aNewString += aEmptySpace;
                
                // d
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[0][1]);
                aNewString += aEmptySpace;
                
                // e
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[1][1]);
                aNewString += aEmptySpace;
                
                // f
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[2][1]);
                aNewString += aEmptySpace;
                
                // g
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[0][2]);
                aNewString += aEmptySpace;
                
                // h
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[1][2]);
                aNewString += aEmptySpace;
                
                // i
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[2][2]);
                aNewString += aEmptySpace;
                
                // j
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[0][3], TRUE);
                aNewString += aEmptySpace;
                
                // k
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[1][3], TRUE);
                aNewString += aEmptySpace;
                
                // l
                Imp_PutDoubleChar(aNewString, rConv, ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix[2][3], TRUE);

                aNewString += aClosingBrace;
                break;
            }
            default : DBG_ERROR("SdXMLImExTransform3D: impossible entry!"); break;
        }

        // if not the last entry, add one space to next tag
        if(a+1 != maList.Count())
            aNewString += aEmptySpace;
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
                        maList.Insert(new ImpSdXMLExpTransObj3DRotateX(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatey, nPos))
                { 
                    double fValue(0.0);

                    nPos += 7; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj3DRotateY(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_rotatez, nPos))
                { 
                    double fValue(0.0);

                    nPos += 7; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    fValue = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, fValue);
                    if(fValue != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj3DRotateZ(fValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_scale, nPos))
                { 
                    Vector3D aValue(1.0, 1.0, 1.0);

                    nPos += 5; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.X() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.X());
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Y() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Y());
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Z() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Z());

                    if(aValue.X() != 1.0 || aValue.Y() != 1.0 || aValue.Z() != 1.0)
                        maList.Insert(new ImpSdXMLExpTransObj3DScale(aValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_translate, nPos))
                { 
                    Vector3D aValue;

                    nPos += 9; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);
                    aValue.X() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.X(), TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Y() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Y(), TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);
                    aValue.Z() = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue.Z(), TRUE);

                    if(aValue.X() != 0.0 || aValue.Y() != 0.0 || aValue.Z() != 0.0)
                        maList.Insert(new ImpSdXMLExpTransObj3DTranslate(aValue), LIST_APPEND);

                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else if(nPos == aStr.indexOf(aString_matrix, nPos))
                { 
                    Matrix4D aValue;

                    nPos += 6; 
                    Imp_SkipSpacesAndOpeningBraces(aStr, nPos, nLen);

                    // a
                    aValue[0][0] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][0]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // b
                    aValue[1][0] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][0]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // c
                    aValue[2][0] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[2][0]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // d
                    aValue[0][1] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][1]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // e
                    aValue[1][1] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][1]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // f
                    aValue[2][1] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[2][1]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // g
                    aValue[0][2] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][2]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // h
                    aValue[1][2] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][2]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // i
                    aValue[2][2] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[2][2]);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // j
                    aValue[0][3] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[0][3], TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // k
                    aValue[1][3] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[1][3], TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    // l
                    aValue[2][3] = Imp_GetDoubleChar(aStr, nPos, nLen, rConv, aValue[2][3], TRUE);
                    Imp_SkipSpacesAndCommas(aStr, nPos, nLen);

                    maList.Insert(new ImpSdXMLExpTransObj3DMatrix(aValue), LIST_APPEND);
                    Imp_SkipSpacesAndClosingBraces(aStr, nPos, nLen);
                }
                else
                    nPos++;
            }
        }
    }
}

BOOL SdXMLImExTransform3D::GetFullHomogenTransform( ::com::sun::star::drawing::HomogenMatrix& xHomMat)
{
    Matrix4D aFullTransform;
    GetFullTransform(aFullTransform);

    if(aFullTransform[0][0] != 1.0 || aFullTransform[1][1] != 1.0 || aFullTransform[2][2] != 1.0
        || aFullTransform[0][1] != 0.0 || aFullTransform[0][2] != 0.0 || aFullTransform[0][3] != 0.0 
        || aFullTransform[1][0] != 0.0 || aFullTransform[1][2] != 0.0 || aFullTransform[1][3] != 0.0 
        || aFullTransform[2][0] != 0.0 || aFullTransform[2][1] != 0.0 || aFullTransform[2][3] != 0.0)
    {
        xHomMat.Line1.Column1 = aFullTransform[0][0];
        xHomMat.Line1.Column2 = aFullTransform[0][1];
        xHomMat.Line1.Column3 = aFullTransform[0][2];
        xHomMat.Line1.Column4 = aFullTransform[0][3];

        xHomMat.Line2.Column1 = aFullTransform[1][0];
        xHomMat.Line2.Column2 = aFullTransform[1][1];
        xHomMat.Line2.Column3 = aFullTransform[1][2];
        xHomMat.Line2.Column4 = aFullTransform[1][3];

        xHomMat.Line3.Column1 = aFullTransform[2][0];
        xHomMat.Line3.Column2 = aFullTransform[2][1];
        xHomMat.Line3.Column3 = aFullTransform[2][2];
        xHomMat.Line3.Column4 = aFullTransform[2][3];

        xHomMat.Line4.Column1 = aFullTransform[3][0];
        xHomMat.Line4.Column2 = aFullTransform[3][1];
        xHomMat.Line4.Column3 = aFullTransform[3][2];
        xHomMat.Line4.Column4 = aFullTransform[3][3];

        return TRUE;
    }

    return FALSE;
}

void SdXMLImExTransform3D::GetFullTransform(Matrix4D& rFullTrans)
{
    rFullTrans.Identity();

    for(sal_uInt32 a(0L); a < maList.Count(); a++)
    {
        ImpSdXMLExpTransObj3DBase* pObj = maList.GetObject(a);
        switch(pObj->mnType)
        {
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_X	: 
                rFullTrans.RotateX(((ImpSdXMLExpTransObj3DRotateX*)pObj)->mfRotateX); break;
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Y	: 
                rFullTrans.RotateY(((ImpSdXMLExpTransObj3DRotateY*)pObj)->mfRotateY); break;
            case IMP_SDXMLEXP_TRANSOBJ3D_ROTATE_Z	: 
                rFullTrans.RotateZ(((ImpSdXMLExpTransObj3DRotateZ*)pObj)->mfRotateZ); break;
            case IMP_SDXMLEXP_TRANSOBJ3D_SCALE		: 
                rFullTrans.Scale(((ImpSdXMLExpTransObj3DScale*)pObj)->maScale); break;
            case IMP_SDXMLEXP_TRANSOBJ3D_TRANSLATE	: 
                rFullTrans.Translate(((ImpSdXMLExpTransObj3DTranslate*)pObj)->maTranslate); break;
            case IMP_SDXMLEXP_TRANSOBJ3D_MATRIX		: 
                rFullTrans *= ((ImpSdXMLExpTransObj3DMatrix*)pObj)->maMatrix; break;
            default : DBG_ERROR("SdXMLImExTransform3D: impossible entry!"); break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImExViewBox::SdXMLImExViewBox(sal_Int32 nX, sal_Int32 nY, sal_Int32 nW, sal_Int32 nH)
:	mnX( nX ),
    mnY( nY ),
    mnW( nW ),
    mnH( nH )
{
}

// #100617# Asked vincent hardy: svg:viewBox values may be double precision.
SdXMLImExViewBox::SdXMLImExViewBox(const OUString& rNew, const SvXMLUnitConverter& rConv)
:	mnX( 0L ),
    mnY( 0L ),
    mnW( 1000L ),
    mnH( 1000L ),
    msString(rNew)
{
    if(msString.getLength())
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

const OUString& SdXMLImExViewBox::GetExportString(const SvXMLUnitConverter& rConv)
{
    OUString aNewString;
    OUString aEmptySpace(sal_Unicode(' '));

    Imp_PutNumberChar(aNewString, rConv, mnX);
    aNewString += aEmptySpace;
    
    Imp_PutNumberChar(aNewString, rConv, mnY);
    aNewString += aEmptySpace;
    
    Imp_PutNumberChar(aNewString, rConv, mnW);
    aNewString += aEmptySpace;
    
    Imp_PutNumberChar(aNewString, rConv, mnH);

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
    const SvXMLUnitConverter& rConv,
    // #96328#
    const sal_Bool bClosed)
:	maPoly( 0L )
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
        sal_Bool bScale(rObjectSize.Width != rViewBox.GetWidth() 
            || rObjectSize.Height != rViewBox.GetHeight());
        sal_Bool bTranslate(rViewBox.GetX() != 0L || rViewBox.GetY() != 0L);

        for(sal_Int32 a(0L); a < nCnt; a++)
        {
            // prepare coordinates
            sal_Int32 nX( pArray->X - rObjectPos.X );
            sal_Int32 nY( pArray->Y - rObjectPos.Y );

            if(bScale)
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
            Imp_PutNumberChar(aNewString, rConv, nX);
            aNewString += OUString(sal_Unicode(','));

            // Y and space (not for last)
            Imp_PutNumberChar(aNewString, rConv, nY);
            if(a + 1 != nCnt)
                aNewString += OUString(sal_Unicode(' '));

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
:	msString( rNew ),
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
        nPos = 0L;
        maPoly.realloc(1L);
        drawing::PointSequence* pOuterSequence = maPoly.getArray();
        pOuterSequence->realloc(nNumPoints);
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        // object size and ViewBox size different?
        sal_Bool bScale(rObjectSize.Width != rViewBox.GetWidth() 
            || rObjectSize.Height != rViewBox.GetHeight());
        sal_Bool bTranslate(rViewBox.GetX() != 0L || rViewBox.GetY() != 0L);

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

            if(bScale)
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

SdXMLImExSvgDElement::SdXMLImExSvgDElement(const SdXMLImExViewBox& rViewBox)
:	mrViewBox( rViewBox ),
    mbIsClosed( FALSE ),
    mbIsCurve( FALSE ),
    mnLastX( 0L ),
    mnLastY( 0L ),
    maPoly( 0L ),
    maFlag( 0L )
{
}

void Imp_GetPrevPos(awt::Point*& pPrevPos1, 
    drawing::PolygonFlags& aPrevFlag1, 
    const BOOL bClosed, awt::Point* pPoints, 
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
    const BOOL bScale, const BOOL bTranslate)
{
    nX = pPointArray->X - rObjectPos.X;
    nY = pPointArray->Y - rObjectPos.Y;

    if(bScale)
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
static BOOL bDoTestHere(TRUE);
#endif // TEST_QUADRATIC_CURVES

void SdXMLImExSvgDElement::AddPolygon(
    drawing::PointSequence* pPoints, 
    drawing::FlagSequence* pFlags,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    const SvXMLUnitConverter& rConv, 
    sal_Bool bClosed, sal_Bool bRelative)
{
    DBG_ASSERT(pPoints, "Empty PointSequence handed over to SdXMLImExSvgDElement(!)");

    sal_Int32 nCnt(pPoints->getLength());

    // #104076# Convert to string only when at last one point included
    if(nCnt > 0)
    {
        // append polygon to string
        OUString aNewString;
        sal_Unicode aLastCommand;
        awt::Point* pPointArray = pPoints->getArray();

        // are the flags used at all? If not forget about them
        if(pFlags)
        {
            sal_Int32 nFlagCnt(pFlags->getLength());

            if(nFlagCnt)
            {
                sal_Bool bFlagsUsed(sal_False);
                drawing::PolygonFlags* pFlagArray = pFlags->getArray();

                for(sal_Int32 a(0); !bFlagsUsed && a < nFlagCnt; a++)
                    if(drawing::PolygonFlags_NORMAL != *pFlagArray++)
                        bFlagsUsed = sal_True;

                if(!bFlagsUsed)
                    pFlags = 0L;
            }
            else
            {
                pFlags = 0L;
            }
        }

        // object size and ViewBox size different?
        sal_Bool bScale(rObjectSize.Width != mrViewBox.GetWidth() 
            || rObjectSize.Height != mrViewBox.GetHeight());
        sal_Bool bTranslate(mrViewBox.GetX() != 0L || mrViewBox.GetY() != 0L);

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
        BOOL bDidWriteStart(FALSE);

        for(sal_Int32 a(0L); a < nCnt; a++)
        {
            if(!pFlags || drawing::PolygonFlags_CONTROL != *pFlagArray)
            {
                BOOL bDidWriteAsCurve(FALSE);

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
                                    sal_Bool bIsQuadratic(sal_False);
                                    const sal_Bool bEnableSaveQuadratic(sal_False);

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
                                            bIsQuadratic = sal_True;
                                        }
                                    }

#ifdef TEST_QUADRATIC_CURVES
                                    if(bDoTestHere)
                                    {
                                        bIsQuadratic = sal_False;

                                        if(pPrevPos1->X == pPrevPos2->X && pPrevPos1->Y == pPrevPos2->Y)
                                            bIsQuadratic = sal_True;
                                    }
#endif // TEST_QUADRATIC_CURVES

                                    if(bIsQuadratic)
                                    {
#ifdef TEST_QUADRATIC_CURVES
                                        if(bDoTestHere)
                                        {
                                            sal_Bool bPrevPointIsSymmetric(sal_False);

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
                                                    bPrevPointIsSymmetric = sal_True;
                                                }
                                            }

                                            if(bPrevPointIsSymmetric)
                                            {
                                                // write a shorthand/smooth quadratic curveto entry (T)
                                                if(bRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('t'))
                                                        aNewString += OUString(sal_Unicode('t'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('t');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('T'))
                                                        aNewString += OUString(sal_Unicode('T'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

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
                                                if(bRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('q'))
                                                        aNewString += OUString(sal_Unicode('q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX1 - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY1 - mnLastY);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('q');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('Q'))
                                                        aNewString += OUString(sal_Unicode('Q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX1);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY1);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                                                    aLastCommand = sal_Unicode('Q');
                                                }
                                            }
                                        }
                                        else
                                        {
#endif // TEST_QUADRATIC_CURVES
                                            awt::Point aNewPoint(nPX_L, nPY_L);
                                            sal_Bool bPrevPointIsSmooth(sal_False);

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
                                                    Vector2D aVec1(pPrevPos4->X - pPrevPos3->X, pPrevPos4->Y - pPrevPos3->Y);
                                                    Vector2D aVec2(aNewPoint.X - pPrevPos3->X, aNewPoint.Y - pPrevPos3->Y);
                                                    sal_Bool bSameLength(FALSE);
                                                    sal_Bool bSameDirection(FALSE);

                                                    // get vector values
                                                    Imp_CalcVectorValues(aVec1, aVec2, bSameLength, bSameDirection);

                                                    if(bSameLength && bSameDirection)
                                                        bPrevPointIsSmooth = sal_True;
                                                }
                                            }

                                            if(bPrevPointIsSmooth)
                                            {
                                                // write a shorthand/smooth quadratic curveto entry (T)
                                                if(bRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('t'))
                                                        aNewString += OUString(sal_Unicode('t'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('t');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('T'))
                                                        aNewString += OUString(sal_Unicode('T'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

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
                                                if(bRelative)
                                                {
                                                    if(aLastCommand != sal_Unicode('q'))
                                                        aNewString += OUString(sal_Unicode('q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX1 - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY1 - mnLastY);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                    aLastCommand = sal_Unicode('q');
                                                }
                                                else
                                                {
                                                    if(aLastCommand != sal_Unicode('Q'))
                                                        aNewString += OUString(sal_Unicode('Q'));

                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX1);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY1);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                    Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                                                    aLastCommand = sal_Unicode('Q');
                                                }
                                            }
#ifdef TEST_QUADRATIC_CURVES
                                        }
#endif // TEST_QUADRATIC_CURVES
                                    }
                                    else
                                    {
                                        sal_Bool bPrevPointIsSymmetric(sal_False);

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
                                                bPrevPointIsSymmetric = sal_True;
                                            }
                                        }

                                        // prepare coordinates
                                        sal_Int32 nX2, nY2;

                                        Imp_PrepareCoorExport(nX2, nY2, pPrevPos1, rObjectPos, rObjectSize, 
                                            mrViewBox, bScale, bTranslate);

                                        if(bPrevPointIsSymmetric)
                                        {
                                            // write a shorthand/smooth curveto entry (S)
                                            if(bRelative)
                                            {
                                                if(aLastCommand != sal_Unicode('s'))
                                                    aNewString += OUString(sal_Unicode('s'));

                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX2 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY2 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                aLastCommand = sal_Unicode('s');
                                            }
                                            else
                                            {
                                                if(aLastCommand != sal_Unicode('S'))
                                                    aNewString += OUString(sal_Unicode('S'));

                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX2);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY2);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

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
                                            if(bRelative)
                                            {
                                                if(aLastCommand != sal_Unicode('c'))
                                                    aNewString += OUString(sal_Unicode('c'));

                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX1 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY1 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX2 - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY2 - mnLastY);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                                aLastCommand = sal_Unicode('c');
                                            }
                                            else
                                            {
                                                if(aLastCommand != sal_Unicode('C'))
                                                    aNewString += OUString(sal_Unicode('C'));
                                                
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX1);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY1);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX2);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY2);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                                                aLastCommand = sal_Unicode('C');
                                            }
                                        }
                                    }

                                    // remember that current point IS written
                                    bDidWriteAsCurve = TRUE;

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
                            if(bRelative)
                            {
                                if(aLastCommand != sal_Unicode('v'))
                                    aNewString += OUString(sal_Unicode('v'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                aLastCommand = sal_Unicode('v');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('V'))
                                    aNewString += OUString(sal_Unicode('V'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                                aLastCommand = sal_Unicode('V');
                            }
                        }
                        else if(mnLastY == nY)
                        {
                            if(bRelative)
                            {
                                if(aLastCommand != sal_Unicode('h'))
                                    aNewString += OUString(sal_Unicode('h'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);

                                aLastCommand = sal_Unicode('h');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('H'))
                                    aNewString += OUString(sal_Unicode('H'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX);

                                aLastCommand = sal_Unicode('H');
                            }
                        }
                        else
                        {
                            if(bRelative)
                            {
                                if(aLastCommand != sal_Unicode('l'))
                                    aNewString += OUString(sal_Unicode('l'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                                aLastCommand = sal_Unicode('l');
                            }
                            else
                            {
                                if(aLastCommand != sal_Unicode('L'))
                                    aNewString += OUString(sal_Unicode('L'));

                                Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                                Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                                aLastCommand = sal_Unicode('L');
                            }
                        }
                    }
                    else
                    {
                        // write as start point
                        if(bRelative)
                        {
                            aNewString += OUString(sal_Unicode('m'));

                            Imp_PutNumberCharWithSpace(aNewString, rConv, nX - mnLastX);
                            Imp_PutNumberCharWithSpace(aNewString, rConv, nY - mnLastY);

                            aLastCommand = sal_Unicode('l');
                        }
                        else
                        {
                            aNewString += OUString(sal_Unicode('M'));

                            Imp_PutNumberCharWithSpace(aNewString, rConv, nX);
                            Imp_PutNumberCharWithSpace(aNewString, rConv, nY);

                            aLastCommand = sal_Unicode('L');
                        }

                        // remember start written
                        bDidWriteStart = TRUE;
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
            if(bRelative)
                aNewString += OUString(sal_Unicode('z'));
            else
                aNewString += OUString(sal_Unicode('Z'));
        }

        // append new string
        msString += aNewString;
    }
}

// #100617# Linear double reader
double Imp_ImportDoubleAndSpaces(
    double fRetval, const OUString& rStr, sal_Int32& rPos, 
    const sal_Int32 nLen, const SvXMLUnitConverter& rConv)
{
    fRetval = Imp_GetDoubleChar(rStr, rPos, nLen, rConv, fRetval);
    Imp_SkipSpacesAndCommas(rStr, rPos, nLen);
    return fRetval;
}

// #100617# Allow to read doubles, too. This will need to be changed to
// the usage of Imp_ImportDoubleAndSpaces(...). For now, this is sufficient
// since the interface cannot transport doubles.
sal_Int32 Imp_ImportNumberAndSpaces(
    sal_Int32 nRetval, const OUString& rStr, sal_Int32& rPos, 
    const sal_Int32 nLen, const SvXMLUnitConverter& rConv)
{
    nRetval = FRound(Imp_ImportDoubleAndSpaces(double(nRetval), rStr, rPos, nLen, rConv));
    Imp_SkipSpacesAndCommas(rStr, rPos, nLen);
    return nRetval;
}

void Imp_PrepareCoorImport(sal_Int32& nX, sal_Int32& nY, 
    const awt::Point& rObjectPos, const awt::Size& rObjectSize, 
    const SdXMLImExViewBox& rViewBox, const BOOL bScale, const BOOL bTranslate)
{
    if(bTranslate)
    {
        nX -= rViewBox.GetX();
        nY -= rViewBox.GetY();
    }

    if(bScale)
    {
        nX = (nX * rObjectSize.Width) / rViewBox.GetWidth();
        nY = (nY * rObjectSize.Height) / rViewBox.GetHeight();
    }

    nX += rObjectPos.X;
    nY += rObjectPos.Y;
}

void Imp_AddExportPoints(sal_Int32 nX, sal_Int32 nY, 
    awt::Point* pPoints, drawing::PolygonFlags* pFlags,
    const sal_Int32 nInnerIndex,
    drawing::PolygonFlags eFlag)
{
    if(pPoints)
        pPoints[nInnerIndex] = awt::Point( nX, nY );

    if(pFlags)
        pFlags[nInnerIndex] = eFlag;
}

void Imp_CalcVectorValues(Vector2D& aVec1, Vector2D& aVec2, sal_Bool& bSameLength, sal_Bool& bSameDirection)
{
    sal_Int32 nLen1 = (sal_Int32)(aVec1.GetLength() + 0.5);
    sal_Int32 nLen2 = (sal_Int32)(aVec2.GetLength() + 0.5);
    aVec1.Normalize(); 
    aVec2.Normalize();
    aVec1 += aVec2;
    sal_Int32 nLen3 = (sal_Int32)((aVec1.GetLength() * ((nLen1 + nLen2) / 2.0))+ 0.5);

    bSameLength = (abs(nLen1 - nLen2) <= BORDER_INTEGERS_ARE_EQUAL);
    bSameDirection = (nLen3 <= BORDER_INTEGERS_ARE_EQUAL);
}

void Imp_CorrectPolygonFlag(const sal_uInt32 nInnerIndex, const awt::Point* const pInnerSequence,
    drawing::PolygonFlags* const pInnerFlags, const sal_Int32 nX1, const sal_Int32 nY1)
{
    if(nInnerIndex)
    {
        const awt::Point aPPrev1 = pInnerSequence[nInnerIndex - 1];

        if(nInnerIndex > 1)
        {
            const awt::Point aPPrev2 = pInnerSequence[nInnerIndex - 2];
            const drawing::PolygonFlags aFPrev2 = pInnerFlags[nInnerIndex - 2];
            Vector2D aVec1(aPPrev2.X - aPPrev1.X, aPPrev2.Y - aPPrev1.Y);
            Vector2D aVec2(nX1 - aPPrev1.X, nY1 - aPPrev1.Y);
            sal_Bool bSameLength(FALSE);
            sal_Bool bSameDirection(FALSE);

            // get vector values
            Imp_CalcVectorValues(aVec1, aVec2, bSameLength, bSameDirection);

            if(drawing::PolygonFlags_CONTROL == aFPrev2)
            {
                // point before is a control point
                if(bSameDirection)
                {
                    if(bSameLength)
                    {
                        // set to PolygonFlags_SYMMETRIC
                        pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_SYMMETRIC;
                    }
                    else
                    {
                        // set to PolygonFlags_SMOOTH
                        pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_SMOOTH;
                    }
                }
                else
                {
                    // set to PolygonFlags_NORMAL
                    pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_NORMAL;
                }
            }
            else
            {
                // point before is a simple curve point
                if(bSameDirection)
                {
                    // set to PolygonFlags_SMOOTH
                    pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_SMOOTH;
                }
                else
                {
                    // set to PolygonFlags_NORMAL
                    pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_NORMAL;
                }
            }
        }
        else
        {
            // no point before starpoint, set type to PolygonFlags_NORMAL
            pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_NORMAL;
        }
    }
}

SdXMLImExSvgDElement::SdXMLImExSvgDElement(const OUString& rNew, 
    const SdXMLImExViewBox& rViewBox,
    const awt::Point& rObjectPos,
    const awt::Size& rObjectSize,
    const SvXMLUnitConverter& rConv)
:	msString( rNew ),
    mrViewBox( rViewBox ),
    mbIsClosed( FALSE ),
    mbIsCurve( FALSE ),
    mnLastX( 0L ),
    mnLastY( 0L ),
    maPoly( 0L ),
    maFlag( 0L )
{
    // convert string to polygon
    const OUString aStr(msString.getStr(), msString.getLength());
    const sal_Int32 nLen(aStr.getLength());
    sal_Int32 nPos(0);
    sal_Int32 nNumPolys(0L);
    sal_Bool bEllipticalArc(FALSE);

    // object size and ViewBox size different?
    sal_Bool bScale(rObjectSize.Width != mrViewBox.GetWidth() 
        || rObjectSize.Height != mrViewBox.GetHeight());
    sal_Bool bTranslate(mrViewBox.GetX() != 0L || mrViewBox.GetY() != 0L);

    // first loop: count polys and get flags
    Imp_SkipSpaces(aStr, nPos, nLen);

    while(nPos < nLen)
    {
        switch(aStr[nPos++])
        {
            case 'Z' :
            case 'z' : 
            { 
                break; 
            }
            case 'M' :
            case 'm' : 
            { 
                nNumPolys++; 
                break; 
            }
            case 'S' :
            case 's' :
            case 'C' :
            case 'c' : 
            case 'Q' :
            case 'q' :
            case 'T' :
            case 't' :
            { 
                mbIsCurve = TRUE; 
                break; 
            }
            case 'L' :
            case 'l' :
            case 'H' :
            case 'h' :
            case 'V' :
            case 'v' :
            {
                // normal, interpreted values. All okay.
                break;
            }
            case 'A' :
            case 'a' :
            {
                // Not yet interpreted value.
                bEllipticalArc = TRUE;
                break;
            }
        }
    }

    DBG_ASSERT(!bEllipticalArc, "XMLIMP: non-interpreted tags in svg:d element!");

    if(nNumPolys)
    {
        // alloc arrays
        maPoly.realloc(nNumPolys);
        if(IsCurve())
            maFlag.realloc(nNumPolys);

        // get outer sequences
        drawing::PointSequence* pOuterSequence = maPoly.getArray();
        drawing::FlagSequence* pOuterFlags = (IsCurve()) ? maFlag.getArray() : 0L;

        // prepare new loop, count
        sal_uInt32 nPointCount(0L);
        nPos = 0;
        Imp_SkipSpaces(aStr, nPos, nLen);

        // #104076# reset closed flag for next to be started polygon
        mbIsClosed = FALSE;

        while(nPos < nLen)
        {
            switch(aStr[nPos])
            {
                case 'z' :
                case 'Z' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    // #104076# remember closed state of current polygon
                    mbIsClosed = TRUE;
                    
                    break;
                }
                case 'm' :
                case 'M' :
                {
                    // new poly starts, end-process current poly
                    if(nPointCount)
                    {
                        // #104076# If this partial polygon is closed, use one more point
                        // to represent that
                        if(mbIsClosed)
                        {
                            nPointCount++;
                        }

                        pOuterSequence->realloc(nPointCount);
                        pOuterSequence++;
                        
                        if(pOuterFlags)
                        {
                            pOuterFlags->realloc(nPointCount);
                            pOuterFlags++;
                        }
                        
                        // reset point count for next polygon
                        nPointCount = 0L;
                    }
                    
                    // #104076# reset closed flag for next to be started polygon
                    mbIsClosed = FALSE;
                    
                    // NO break, continue in next case
                }
                case 'L' :
                case 'l' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        nPointCount++;
                    }
                    break;
                }
                case 'H' :
                case 'h' :
                case 'V' :
                case 'v' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        nPointCount++;
                    }
                    break;
                }
                case 'S' :
                case 's' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        nPointCount += 3;
                    }
                    break;
                }
                case 'C' :
                case 'c' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        nPointCount += 3;
                    }
                    break;
                }

                // #100617# quadratic beziers, supported as cubic ones
                case 'Q' :
                case 'q' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        
                        // use three points since quadratic is imported as cubic
                        nPointCount += 3;
                    }
                    break;
                }

                // #100617# relative quadratic beziers, supported as cubic ones
                case 'T' :
                case 't' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        
                        // use three points since quadratic is imported as cubic
                        nPointCount += 3;
                    }
                    break;
                }

                // #100617# not yet supported: elliptical arc
                case 'A' :
                case 'a' :
                {
                    DBG_ERROR("XMLIMP: non-interpreted tags in svg:d element (elliptical arc)!");
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipNumberAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipNumberAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                    }
                    break;
                }

                default:
                {
                    nPos++;
                    DBG_ERROR("XMLIMP: non-interpreted tags in svg:d element (unknown)!");
                    break;
                }
            }
        }

        // alloc last poly (when points used)
        if(nPointCount)
        {
            // #104076# If this partial polygon is closed, use one more point
            // to represent that
            if(mbIsClosed)
            {
                nPointCount++;
            }

            pOuterSequence->realloc(nPointCount);
            pOuterSequence++;

            if(pOuterFlags)
            {
                pOuterFlags->realloc(nPointCount);
                pOuterFlags++;
            }
        }

        // set pointers back
        pOuterSequence = maPoly.getArray();
        pOuterFlags = (IsCurve()) ? maFlag.getArray() : 0L;
        awt::Point* pInnerSequence = 0L;
        drawing::PolygonFlags* pInnerFlags = 0L;
        sal_uInt32 nInnerIndex(0L);

        // prepare new loop, read points
        nPos = 0;
        Imp_SkipSpaces(aStr, nPos, nLen);

        // #104076# reset closed flag for next to be started polygon
        mbIsClosed = FALSE;

        while(nPos < nLen)
        {
            BOOL bRelative(FALSE);

            switch(aStr[nPos])
            {
                case 'z' :
                case 'Z' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    // #104076# remember closed state of current polygon
                    mbIsClosed = TRUE;

                    // closed: add first point again
                    // sal_Int32 nX(pInnerSequence[0].X);
                    // sal_Int32 nY(pInnerSequence[0].Y);
                    // Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                    
                    break;
                }
                
                case 'm' :
                {
                    bRelative = TRUE;
                }
                case 'M' :
                {
                    // #104076# end-process current poly
                    if(mbIsClosed)
                    {
                        if(pInnerSequence)
                        {
                            // closed: add first point again
                            sal_Int32 nX(pInnerSequence[0].X);
                            sal_Int32 nY(pInnerSequence[0].Y);
                            Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                        }

                        // reset closed flag for next to be started polygon
                        mbIsClosed = FALSE;
                    }

                    // next poly
                    pInnerSequence = pOuterSequence->getArray();
                    pOuterSequence++;

                    if(pOuterFlags)
                    {
                        pInnerFlags = pOuterFlags->getArray();
                        pOuterFlags++;
                    }

                    nInnerIndex = 0L;

                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;
                        
                        // calc transform and add point and flag
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                    }
                    break;
                }

                case 'l' :
                {
                    bRelative = TRUE;
                }
                case 'L' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);
                    
                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;
                        
                        // calc transform and add point and flag
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                    }
                    break;
                }
                
                case 'h' :
                {
                    bRelative = TRUE;
                }
                case 'H' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(mnLastY);

                        if(bRelative)
                            nX += mnLastX;

                        // set last position
                        mnLastX = nX;
                        
                        // calc transform and add point and flag
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                    }
                    break;
                }
                
                case 'v' :
                {
                    bRelative = TRUE;
                }
                case 'V' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX(mnLastX);
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                            nY += mnLastY;

                        // set last position
                        mnLastY = nY;
                        
                        // calc transform and add point and flag
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
                    }
                    break;
                }
                
                case 's' :
                {
                    bRelative = TRUE;
                }
                case 'S' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX1;
                        sal_Int32 nY1;
                        sal_Int32 nX2(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY2(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nX2 += mnLastX;
                            nY2 += mnLastY;
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;

                        // calc transform for new points
                        Imp_PrepareCoorImport(nX2, nY2, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);

                        // one more thing is known: the previous real point is PolygonFlags_SYMMETRIC
                        // and the Point X1,Y1 can be constructed by mirroring the point before it.
                        nX1 = nX2;
                        nY1 = nY2;
                        if(nInnerIndex)
                        {
                            awt::Point aPPrev1 = pInnerSequence[nInnerIndex - 1];

                            if(nInnerIndex > 1)
                            {
                                awt::Point aPPrev2 = pInnerSequence[nInnerIndex - 2];
                                nX1 = aPPrev1.X -(aPPrev2.X - aPPrev1.X);
                                nY1 = aPPrev1.Y -(aPPrev2.Y - aPPrev1.Y);
                            }

                            // set curve point to symmetric
                            pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_SYMMETRIC;
                        }

                        // add calculated control point
                        Imp_AddExportPoints(nX1, nY1, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);

                        // add new points and set flags
                        Imp_AddExportPoints(nX2, nY2, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_SMOOTH);
                    }
                    break;
                }
                
                case 'c' :
                {
                    bRelative = TRUE;
                }
                case 'C' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nX1(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY1(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nX2(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY2(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nX1 += mnLastX;
                            nY1 += mnLastY;
                            nX2 += mnLastX;
                            nY2 += mnLastY;
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;

                        // calc transform for new points
                        Imp_PrepareCoorImport(nX1, nY1, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_PrepareCoorImport(nX2, nY2, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);

                        // correct polygon flag for previous point
                        Imp_CorrectPolygonFlag(nInnerIndex, pInnerSequence, pInnerFlags, nX1, nY1);

                        // add new points and set flags
                        Imp_AddExportPoints(nX1, nY1, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX2, nY2, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_SMOOTH);
                    }
                    break;
                }
                
                // #100617# quadratic beziers are imported as cubic
                case 'q' :
                {
                    bRelative = TRUE;
                }
                case 'Q' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nXX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nYY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nXX += mnLastX;
                            nYY += mnLastY;
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;

                        // calc transform for new points
                        Imp_PrepareCoorImport(nXX, nYY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);
                        
                        // calculate X1,X2
                        awt::Point aPPrev1 = (nInnerIndex) ? pInnerSequence[nInnerIndex-1] : pInnerSequence[0];
                        sal_Int32 nX1 = FRound((double)((nXX * 2) + aPPrev1.X) / 3.0);
                        sal_Int32 nY1 = FRound((double)((nYY * 2) + aPPrev1.Y) / 3.0);
                        sal_Int32 nX2 = FRound((double)((nXX * 2) + nX) / 3.0);
                        sal_Int32 nY2 = FRound((double)((nYY * 2) + nY) / 3.0);

                        // correct polygon flag for previous point
                        Imp_CorrectPolygonFlag(nInnerIndex, pInnerSequence, pInnerFlags, nX1, nY1);
                        
                        // add new points and set flags
                        Imp_AddExportPoints(nX1, nY1, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX2, nY2, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_SMOOTH);
                    }
                    break;
                }
                
                // #100617# relative quadratic beziers are imported as cubic
                case 't' :
                {
                    bRelative = TRUE;
                }
                case 'T' :
                {
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        sal_Int32 nXX;
                        sal_Int32 nYY;
                        sal_Int32 nX(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));
                        sal_Int32 nY(Imp_ImportNumberAndSpaces(0L, aStr, nPos, nLen, rConv));

                        if(bRelative)
                        {
                            nX += mnLastX;
                            nY += mnLastY;
                        }

                        // set last position
                        mnLastX = nX;
                        mnLastY = nY;

                        // calc transform for new points
                        Imp_PrepareCoorImport(nX, nY, rObjectPos, rObjectSize, mrViewBox, bScale, bTranslate);

                        // one more thing is known: the previous real point is PolygonFlags_SYMMETRIC
                        // and the Point X1,Y1 can be constructed by mirroring the point before it.
                        nXX = nX;
                        nYY = nY;
                        awt::Point aPPrev1 = pInnerSequence[0];
                        
                        if(nInnerIndex)
                        {
                            aPPrev1 = pInnerSequence[nInnerIndex - 1];

                            if(nInnerIndex > 1)
                            {
                                awt::Point aPPrev2 = pInnerSequence[nInnerIndex - 2];
                                nXX = aPPrev1.X -(aPPrev2.X - aPPrev1.X);
                                nYY = aPPrev1.Y -(aPPrev2.Y - aPPrev1.Y);
                            }

                            // set curve point to smooth here, since length
                            // is changed and thus only c1 can be used.
                            pInnerFlags[nInnerIndex - 1] = drawing::PolygonFlags_SMOOTH;
                        }

                        // calculate X1,X2
                        sal_Int32 nX1 = FRound((double)((nXX * 2) + aPPrev1.X) / 3.0);
                        sal_Int32 nY1 = FRound((double)((nYY * 2) + aPPrev1.Y) / 3.0);
                        sal_Int32 nX2 = FRound((double)((nXX * 2) + nX) / 3.0);
                        sal_Int32 nY2 = FRound((double)((nYY * 2) + nY) / 3.0);

                        // correct polygon flag for previous point
                        Imp_CorrectPolygonFlag(nInnerIndex, pInnerSequence, pInnerFlags, nX1, nY1);
                        
                        // add new points and set flags
                        Imp_AddExportPoints(nX1, nY1, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX2, nY2, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_CONTROL);
                        Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_SMOOTH);
                    }
                    break;
                }

                // #100617# not yet supported: elliptical arc
                case 'A' :
                case 'a' :
                {
                    DBG_ERROR("XMLIMP: non-interpreted tags in svg:d element (elliptical arc)!");
                    nPos++;
                    Imp_SkipSpaces(aStr, nPos, nLen);

                    while(nPos < nLen && Imp_IsOnNumberChar(aStr, nPos))
                    {
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipNumberAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipNumberAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                        Imp_SkipDoubleAndSpacesAndCommas(aStr, nPos, nLen);
                    }
                    break;
                }

                default:
                {
                    nPos++;
                    DBG_ERROR("XMLIMP: non-interpreted tags in svg:d element (unknown)!");
                    break;
                }
            }
        }

        // #104076# end-process closed state of last poly
        if(mbIsClosed)
        {
            if(pInnerSequence)
            {
                // closed: add first point again
                sal_Int32 nX(pInnerSequence[0].X);
                sal_Int32 nY(pInnerSequence[0].Y);
                Imp_AddExportPoints(nX, nY, pInnerSequence, pInnerFlags, nInnerIndex++, drawing::PolygonFlags_NORMAL);
            }
        }

        // #87202# If it's a curve and it's closed the last point maybe too much
        // and just exported since SVG does not allow special handling of same
        // start and end point, remove this last point.
        // Evtl. correct the last curve flags, too.
        if(IsCurve() && IsClosed())
        {
            // make one more loop over the PolyPolygon
            pOuterSequence = maPoly.getArray();
            pOuterFlags = maFlag.getArray();
            sal_Int32 nOuterCnt(maPoly.getLength());

            for(sal_Int32 a(0); a < nOuterCnt; a++)
            {
                // get Polygon pointers
                awt::Point* pInnerSequence = pOuterSequence->getArray();
                drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();
                sal_Int32 nInnerCnt(pOuterSequence->getLength());

                while( nInnerCnt >= 2
                    && ((pInnerSequence + (nInnerCnt - 2))->X == (pInnerSequence + (nInnerCnt - 1))->X) 
                    && ((pInnerSequence + (nInnerCnt - 2))->Y == (pInnerSequence + (nInnerCnt - 1))->Y)
                    && drawing::PolygonFlags_CONTROL != *(pInnerFlags + (nInnerCnt - 2)))
                {
                    // remove last point from array
                    pOuterSequence->realloc(nInnerCnt - 1);
                    pOuterFlags->realloc(nInnerCnt - 1);

                    // get new pointers
                    pInnerSequence = pOuterSequence->getArray();
                    pInnerFlags = pOuterFlags->getArray();
                    nInnerCnt = pOuterSequence->getLength();
                }

                // now evtl. correct the last curve flags
                if(nInnerCnt >= 4)
                {
                    if(	pInnerSequence->X == (pInnerSequence + (nInnerCnt - 1))->X
                        && pInnerSequence->Y == (pInnerSequence + (nInnerCnt - 1))->Y
                        && drawing::PolygonFlags_CONTROL == *(pInnerFlags + 1)
                        && drawing::PolygonFlags_CONTROL == *(pInnerFlags + (nInnerCnt - 2)))
                    {
                        awt::Point aPrev = *(pInnerSequence + (nInnerCnt - 2));
                        awt::Point aCurr = *pInnerSequence;
                        awt::Point aNext = *(pInnerSequence + 1);
                        Vector2D aVec1(aPrev.X - aCurr.X, aPrev.Y - aCurr.Y);
                        Vector2D aVec2(aNext.X - aCurr.X, aNext.Y - aCurr.Y);
                        sal_Bool bSameLength(FALSE);
                        sal_Bool bSameDirection(FALSE);

                        // get vector values
                        Imp_CalcVectorValues(aVec1, aVec2, bSameLength, bSameDirection);

                        // set correct flag value
                        if(bSameDirection)
                        {
                            if(bSameLength)
                            {
                                // set to PolygonFlags_SYMMETRIC
                                *pInnerFlags = drawing::PolygonFlags_SYMMETRIC;
                                *(pInnerFlags + (nInnerCnt - 1)) = drawing::PolygonFlags_SYMMETRIC;
                            }
                            else
                            {
                                // set to PolygonFlags_SMOOTH
                                *pInnerFlags = drawing::PolygonFlags_SMOOTH;
                                *(pInnerFlags + (nInnerCnt - 1)) = drawing::PolygonFlags_SMOOTH;
                            }
                        }
                        else
                        {
                            // set to PolygonFlags_NORMAL
                            *pInnerFlags = drawing::PolygonFlags_NORMAL;
                            *(pInnerFlags + (nInnerCnt - 1)) = drawing::PolygonFlags_NORMAL;
                        }
                    }
                }
                
                // switch to next Polygon
                pOuterSequence++;
                pOuterFlags++;
            }
        }
    }
}


}//end of namespace binfilter
