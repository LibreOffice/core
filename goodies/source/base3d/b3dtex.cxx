/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dtex.cxx,v $
 * $Revision: 1.12.38.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#include "b3dtex.hxx"
#include "b3dopngl.hxx"
#include <vcl/bmpacc.hxx>

#include "rtl/alloc.h"

/*************************************************************************
|*
|* Klassen fuer TexturAttribute beim Anfordern von Texturen
|*
\************************************************************************/

TextureAttributes::TextureAttributes(sal_Bool bGhosted, void* pFT)
:   mpFloatTrans(pFT),
    mbGhosted(bGhosted)
{
}

sal_Bool TextureAttributes::operator==(const TextureAttributes& rAtt) const
{
    return ( GetTextureAttributeType() == rAtt.GetTextureAttributeType()
        && rAtt.mbGhosted == mbGhosted
        && rAtt.mpFloatTrans == mpFloatTrans);
}

// Fuer Colors

TextureAttributesColor::TextureAttributesColor(sal_Bool bGhosted, void* pFT, Color aColor)
:   TextureAttributes(bGhosted, pFT),
    maColorAttribute(aColor)
{
}

sal_Bool TextureAttributesColor::operator==(const TextureAttributes& rAtt) const
{
    if(TextureAttributes::operator==(rAtt))
    {
        const TextureAttributesColor& rAttCol = (const TextureAttributesColor&)rAtt;
        if(rAttCol.maColorAttribute == maColorAttribute)
            return sal_True;
    }
    return sal_False;
}

sal_uInt16 TextureAttributesColor::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_COLOR;
}

// Fuer Bitmaps

TextureAttributesBitmap::TextureAttributesBitmap(sal_Bool bGhosted, void* pFT, Bitmap aBmp)
:   TextureAttributes(bGhosted, pFT),
    maBitmapAttribute(aBmp)
{
}

TextureAttributesBitmap::~TextureAttributesBitmap()
{
}

sal_Bool TextureAttributesBitmap::operator==(const TextureAttributes& rAtt) const
{
    if(TextureAttributes::operator==(rAtt))
    {
        const TextureAttributesBitmap& rAttBmp = (const TextureAttributesBitmap&)rAtt;
        if(rAttBmp.maBitmapAttribute == maBitmapAttribute)
            return sal_True;
    }
    return sal_False;
}

sal_uInt16 TextureAttributesBitmap::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_BITMAP;
}

// Fuer Gradientfills

TextureAttributesGradient::TextureAttributesGradient(sal_Bool bGhosted, void* pFT, void* pF, void *pSC)
:   TextureAttributes(bGhosted, pFT),
    mpFill(pF),
    mpStepCount(pSC)
{
}

sal_Bool TextureAttributesGradient::operator==(const TextureAttributes& rAtt) const
{
    if(TextureAttributes::operator==(rAtt))
    {
        const TextureAttributesGradient& rAttGra = (const TextureAttributesGradient&)rAtt;

        if(rAttGra.mpFill == mpFill
            && rAttGra.mpStepCount == mpStepCount)
            return sal_True;
    }
    return sal_False;
}

sal_uInt16 TextureAttributesGradient::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_GRADIENT;
}

// Fuer Hatchfills

TextureAttributesHatch::TextureAttributesHatch(sal_Bool bGhosted, void* pFT, void* pF)
:   TextureAttributes(bGhosted, pFT),
    mpFill(pF)
{
}

sal_Bool TextureAttributesHatch::operator==(const TextureAttributes& rAtt) const
{
    if(TextureAttributes::operator==(rAtt))
    {
        const TextureAttributesHatch& rAttHat = (const TextureAttributesHatch&)rAtt;

        if(rAttHat.mpFill == mpFill)
            return sal_True;
    }
    return sal_False;
}

sal_uInt16 TextureAttributesHatch::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_HATCH;
}

/*************************************************************************
|*
|* Konstruktor Textur
|*
\************************************************************************/

B3dTexture::B3dTexture(
    TextureAttributes& rAtt,
    BitmapEx& rBmpEx,
    Base3DTextureKind eKnd,
    Base3DTextureMode eMod,
    Base3DTextureFilter eFlt,
    Base3DTextureWrap eS,
    Base3DTextureWrap eT)
:   aBitmap(rBmpEx.GetBitmap()),
    aAlphaMask(rBmpEx.GetAlpha()),
    aBitmapSize(rBmpEx.GetSizePixel()), // #i72895#
    pReadAccess(NULL),
    pAlphaReadAccess(NULL),
    eKind(eKnd),
    eMode(eMod),
    eFilter(eFlt),
    eWrapS(eS),
    eWrapT(eT),
    nSwitchVal(0),
    bTextureKindChanged(sal_False)
{
    // ReadAccess auf Textur anfordern
    pReadAccess = aBitmap.AcquireReadAccess();
    pAlphaReadAccess = (!aAlphaMask) ? NULL : aAlphaMask.AcquireReadAccess();
    DBG_ASSERT(pReadAccess, "AW: Keinen Lesezugriff auf Textur-Bitmap bekommen");

    // Attribute kopieren
    switch(rAtt.GetTextureAttributeType())
    {
        case TEXTURE_ATTRIBUTE_TYPE_COLOR :
            pAttributes = new TextureAttributesColor(
                rAtt.GetGhostedAttribute(),
                rAtt.GetFloatTransAttribute(),
                ((TextureAttributesColor&)rAtt).GetColorAttribute());
            break;

        case TEXTURE_ATTRIBUTE_TYPE_BITMAP :
            pAttributes = new TextureAttributesBitmap(
                rAtt.GetGhostedAttribute(),
                rAtt.GetFloatTransAttribute(),
                ((TextureAttributesBitmap&)rAtt).GetBitmapAttribute());
            break;

        case TEXTURE_ATTRIBUTE_TYPE_GRADIENT :
            pAttributes = new TextureAttributesGradient(
                rAtt.GetGhostedAttribute(),
                rAtt.GetFloatTransAttribute(),
                ((TextureAttributesGradient&)rAtt).GetFillAttribute(),
                ((TextureAttributesGradient&)rAtt).GetStepCountAttribute());
            break;

        case TEXTURE_ATTRIBUTE_TYPE_HATCH :
            pAttributes = new TextureAttributesHatch(
                rAtt.GetGhostedAttribute(),
                rAtt.GetFloatTransAttribute(),
                ((TextureAttributesHatch&)rAtt).GetHatchFillAttribute());
            break;
    }

    // SwitchVal setzen
    SetSwitchVal();
}

/*************************************************************************
|*
|* Destruktor Textur
|*
\************************************************************************/

B3dTexture::~B3dTexture()
{
    // ReadAccess auf Textur freigeben
    if(pReadAccess)
    {
        aBitmap.ReleaseAccess(pReadAccess);
        pReadAccess = NULL;
    }

    // free ReadAccess to transparency bitmap
    if(pAlphaReadAccess)
    {
        aAlphaMask.ReleaseAccess(pAlphaReadAccess);
        pAlphaReadAccess = NULL;
    }

    // Attribute wegschmeissen
    if(pAttributes)
        delete pAttributes;
    pAttributes = NULL;
}

// Zugriff auf die Attribute der Textur
TextureAttributes& B3dTexture::GetAttributes()
{
    return *pAttributes;
}

/*************************************************************************
|*
|* Art des Wrappings in X setzen
|*
\************************************************************************/

void B3dTexture::SetTextureWrapS(Base3DTextureWrap eNew)
{
    if(eNew != eWrapS)
    {
        eWrapS = eNew;
        bTextureKindChanged = sal_True;
    }
}

/*************************************************************************
|*
|* Art des Wrappings in Y setzen
|*
\************************************************************************/

void B3dTexture::SetTextureWrapT(Base3DTextureWrap eNew)
{
    if(eNew != eWrapT)
    {
        eWrapT = eNew;
        bTextureKindChanged = sal_True;
    }
}

/*************************************************************************
|*
|* Blend-Color lesen/bestimmen
|*
\************************************************************************/

void B3dTexture::SetBlendColor(Color rNew)
{
    if(rNew.GetRed() != aColBlend.GetRed()
        || rNew.GetGreen() != aColBlend.GetGreen()
        || rNew.GetBlue() != aColBlend.GetBlue())
    {
        aColBlend.SetRed(rNew.GetRed());
        aColBlend.SetGreen(rNew.GetGreen());
        aColBlend.SetBlue(rNew.GetBlue());
        if(eWrapS == Base3DTextureSingle || eWrapT == Base3DTextureSingle)
            bTextureKindChanged = sal_True;
    }
}

Color B3dTexture::GetBlendColor()
{
    Color aRetval(aColBlend.GetRed(),
        aColBlend.GetGreen(),
        aColBlend.GetBlue());
    return aRetval;
}

/*************************************************************************
|*
|* Textur-Ersatz-Color lesen/bestimmen
|*
\************************************************************************/

void B3dTexture::SetTextureColor(Color rNew)
{
    if(rNew.GetRed() != aColTexture.GetRed()
        || rNew.GetGreen() != aColTexture.GetGreen()
        || rNew.GetBlue() != aColTexture.GetBlue())
    {
        aColTexture.SetRed(rNew.GetRed());
        aColTexture.SetGreen(rNew.GetGreen());
        aColTexture.SetBlue(rNew.GetBlue());
        if(eWrapS == Base3DTextureSingle || eWrapT == Base3DTextureSingle)
            bTextureKindChanged = sal_True;
    }
}

Color B3dTexture::GetTextureColor()
{
    Color aRetval(aColTexture.GetRed(),
        aColTexture.GetGreen(),
        aColTexture.GetBlue());
    return aRetval;
}

/*************************************************************************
|*
|* Internen Verteilungswert setzen
|*
\************************************************************************/

void B3dTexture::SetSwitchVal()
{
    nSwitchVal = 0;

    // Kind
    if(GetTextureKind() == Base3DTextureLuminance)
        nSwitchVal |= B3D_TXT_KIND_LUM;
    else if(GetTextureKind() == Base3DTextureIntensity)
        nSwitchVal |= B3D_TXT_KIND_INT;
    else if(GetTextureKind() == Base3DTextureColor)
        nSwitchVal |= B3D_TXT_KIND_COL;

    // Mode
    if(GetTextureMode() == Base3DTextureReplace)
        nSwitchVal |= B3D_TXT_MODE_REP;
    else if(GetTextureMode() == Base3DTextureModulate)
        nSwitchVal |= B3D_TXT_MODE_MOD;
    else if(GetTextureMode() == Base3DTextureBlend)
        nSwitchVal |= B3D_TXT_MODE_BND;

    // Filter
    if(GetTextureFilter() == Base3DTextureNearest)
        nSwitchVal |= B3D_TXT_FLTR_NEA;
}

/*************************************************************************
|*
|* Zugriffsfunktion auf die Farben der Bitmap
|*
\************************************************************************/

BitmapColor B3dTexture::GetBitmapColor(long nX, long nY) const
{
    return pReadAccess->GetColor(nY, nX);
}

sal_uInt8 B3dTexture::GetBitmapTransparency(long nX, long nY) const
{
    if(pAlphaReadAccess)
        return pAlphaReadAccess->GetColor(nY, nX).GetIndex();
    return 0;
}

/*************************************************************************
|*
|* Art der Pixeldaten lesen/bestimmen
|*
\************************************************************************/

void B3dTexture::SetTextureKind(Base3DTextureKind eNew)
{
    if(eKind != eNew)
    {
        eKind = eNew;
        bTextureKindChanged = sal_True;
    }
    SetSwitchVal();
}

/*************************************************************************
|*
|* Texturmodus lesen/bestimmen
|*
\************************************************************************/

void B3dTexture::SetTextureMode(Base3DTextureMode eNew)
{
    eMode = eNew;
    SetSwitchVal();
}

/*************************************************************************
|*
|* Filtermodus lesen/bestimmen
|*
\************************************************************************/

void B3dTexture::SetTextureFilter(Base3DTextureFilter eNew)
{
    eFilter = eNew;
    SetSwitchVal();
}

/*************************************************************************
|*
|* Die Texturfunktion selbst. Die ursruengliche Farbe des Punktes
|* innerhalb der Grenzen des Parameterbereiches S,T (in Bitmapkoordinaten)
|* wird modifiziert.
|*
\************************************************************************/

void B3dTexture::ModifyColor(Color& rCol, double fS, double fT)
{
    // Integer-Koordinaten der Texturposition bilden
    long nX((long)fS);
    long nY((long)fT);
    BitmapColor aBmCol = aColTexture;
    sal_Bool bOnTexture(sal_True);

    // Wrapping in S-Richtung
    if(eWrapS == Base3DTextureClamp)
    {
        // Clamping
        if(nX < 0)
            nX = 0;
        else if(nX >= GetBitmapSize().Width())
            nX = GetBitmapSize().Width() - 1;
    }
    else if(eWrapS == Base3DTextureRepeat)
    {
        // Repeating
        nX %= GetBitmapSize().Width();
        if(nX < 0)
            nX += GetBitmapSize().Width();
    }
    else
    {
        // Single
        if(nX < 0 || nX >= GetBitmapSize().Width())
            bOnTexture = sal_False;
    }

    // Wrapping in T-Richtung
    if(bOnTexture)
    {
        if(eWrapT == Base3DTextureClamp)
        {
            // Clamping
            if(nY < 0)
                nY = 0;
            else if(nY >= GetBitmapSize().Height())
                nY = GetBitmapSize().Height() - 1;
        }
        else if(eWrapT == Base3DTextureRepeat)
        {
            // Repeating
            nY %= GetBitmapSize().Height();
            if(nY < 0)
                nY += GetBitmapSize().Height();
        }
        else
        {
            // Single
            if(nY < 0 || nY >= GetBitmapSize().Height())
                bOnTexture = sal_False;
        }

        if(bOnTexture)
            aBmCol = pReadAccess->GetColor(nY, nX);
    }

    // transparence
    if(bOnTexture && pAlphaReadAccess)
        rCol.SetTransparency(pAlphaReadAccess->GetLuminance(nY, nX));

    // Falls die Position nicht innerhalb der Textur ist, auch das Filtern
    // unterdruecken um keine falschen BitmapAcesses zu bekommen
    sal_uInt8 nLocalSwitchVal(nSwitchVal);
    if(!bOnTexture)
        nLocalSwitchVal |= B3D_TXT_FLTR_NEA;

    switch(nLocalSwitchVal)
    {
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_REP|B3D_TXT_KIND_COL) :
        {
            rCol.SetRed(aBmCol.GetRed());
            rCol.SetGreen(aBmCol.GetGreen());
            rCol.SetBlue(aBmCol.GetBlue());
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_MOD|B3D_TXT_KIND_COL) :
        {
            rCol.SetRed((sal_uInt8)( ((sal_uInt16)rCol.GetRed() * (sal_uInt16)aBmCol.GetRed())>>8 ));
            rCol.SetGreen((sal_uInt8)( ((sal_uInt16)rCol.GetGreen() * (sal_uInt16)aBmCol.GetGreen())>>8 ));
            rCol.SetBlue((sal_uInt8)( ((sal_uInt16)rCol.GetBlue() * (sal_uInt16)aBmCol.GetBlue())>>8 ));
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_COL) :
        {
            rCol.SetRed((sal_uInt8)( ((sal_uInt16)rCol.GetRed() * (0x00ff - (sal_uInt16)aBmCol.GetRed()))
                + ((sal_uInt16)aColBlend.GetRed() * (sal_uInt16)aBmCol.GetRed()) ));
            rCol.SetGreen((sal_uInt8)( ((sal_uInt16)rCol.GetGreen() * (0x00ff - (sal_uInt16)aBmCol.GetGreen()))
                + ((sal_uInt16)aColBlend.GetGreen() * (sal_uInt16)aBmCol.GetGreen()) ));
            rCol.SetBlue((sal_uInt8)( ((sal_uInt16)rCol.GetBlue() * (0x00ff - (sal_uInt16)aBmCol.GetBlue()))
                + ((sal_uInt16)aColBlend.GetBlue() * (sal_uInt16)aBmCol.GetBlue()) ));
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_REP|B3D_TXT_KIND_INT) :
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_REP|B3D_TXT_KIND_LUM) :
        {
            rCol.SetRed((aBmCol.GetRed() + aBmCol.GetGreen() + aBmCol.GetBlue()) / 3);
            rCol.SetGreen(rCol.GetRed());
            rCol.SetBlue(rCol.GetRed());
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_MOD|B3D_TXT_KIND_INT) :
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_MOD|B3D_TXT_KIND_LUM) :
        {
            sal_uInt16 nMidCol = (aBmCol.GetRed() + aBmCol.GetGreen() + aBmCol.GetBlue()) / 3;
            rCol.SetRed((sal_uInt8)( ((sal_uInt16)rCol.GetRed() * nMidCol)>>8 ));
            rCol.SetGreen((sal_uInt8)( ((sal_uInt16)rCol.GetGreen() * nMidCol)>>8 ));
            rCol.SetBlue((sal_uInt8)( ((sal_uInt16)rCol.GetBlue() * nMidCol)>>8 ));
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_INT) :
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_LUM) :
        {
            sal_uInt16 nMidCol = (aBmCol.GetRed() + aBmCol.GetGreen() + aBmCol.GetBlue()) / 3;
            sal_uInt16 nInvMidCol = 0x00ff - nMidCol;
            rCol.SetRed((sal_uInt8)( ((sal_uInt16)rCol.GetRed() * nInvMidCol)
                + ((sal_uInt16)aColBlend.GetRed() * nMidCol) ));
            rCol.SetGreen((sal_uInt8)( ((sal_uInt16)rCol.GetGreen() * nInvMidCol)
                + ((sal_uInt16)aColBlend.GetGreen() * nMidCol) ));
            rCol.SetBlue((sal_uInt8)( ((sal_uInt16)rCol.GetBlue() * nInvMidCol)
                + ((sal_uInt16)aColBlend.GetBlue() * nMidCol) ));
            break;
        }
        case (B3D_TXT_MODE_REP|B3D_TXT_KIND_COL) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            BitmapColor aColTL = pReadAccess->GetColor(nY, nX);
            BitmapColor aColTR = pReadAccess->GetColor(nY, nX2);
            BitmapColor aColBL = pReadAccess->GetColor(nY2, nX);
            BitmapColor aColBR = pReadAccess->GetColor(nY2, nX2);

            rCol.SetRed((sal_uInt8)(((double)aColTL.GetRed() * fS + (double)aColTR.GetRed() * fRight) * fT
                + ((double)aColBL.GetRed() * fS + (double)aColBR.GetRed() * fRight) * fBottom));
            rCol.SetGreen((sal_uInt8)(((double)aColTL.GetGreen() * fS + (double)aColTR.GetGreen() * fRight) * fT
                + ((double)aColBL.GetGreen() * fS + (double)aColBR.GetGreen() * fRight) * fBottom));
            rCol.SetBlue((sal_uInt8)(((double)aColTL.GetBlue() * fS + (double)aColTR.GetBlue() * fRight) * fT
                + ((double)aColBL.GetBlue() * fS + (double)aColBR.GetBlue() * fRight) * fBottom));
            break;
        }
        case (B3D_TXT_MODE_MOD|B3D_TXT_KIND_COL) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            BitmapColor aColTL = pReadAccess->GetColor(nY, nX);
            BitmapColor aColTR = pReadAccess->GetColor(nY, nX2);
            BitmapColor aColBL = pReadAccess->GetColor(nY2, nX);
            BitmapColor aColBR = pReadAccess->GetColor(nY2, nX2);

            double fRed = ((double)aColTL.GetRed() * fS + (double)aColTR.GetRed() * fRight) * fT
                + ((double)aColBL.GetRed() * fS + (double)aColBR.GetRed() * fRight) * fBottom;
            double fGreen = ((double)aColTL.GetGreen() * fS + (double)aColTR.GetGreen() * fRight) * fT
                + ((double)aColBL.GetGreen() * fS + (double)aColBR.GetGreen() * fRight) * fBottom;
            double fBlue = ((double)aColTL.GetBlue() * fS + (double)aColTR.GetBlue() * fRight) * fT
                + ((double)aColBL.GetBlue() * fS + (double)aColBR.GetBlue() * fRight) * fBottom;

            rCol.SetRed((sal_uInt8)(((double)rCol.GetRed() * fRed) / 255.0));
            rCol.SetGreen((sal_uInt8)(((double)rCol.GetGreen() * fGreen) / 255.0));
            rCol.SetBlue((sal_uInt8)(((double)rCol.GetBlue() * fBlue) / 255.0));
            break;
        }
        case (B3D_TXT_MODE_BND|B3D_TXT_KIND_COL) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            BitmapColor aColTL = pReadAccess->GetColor(nY, nX);
            BitmapColor aColTR = pReadAccess->GetColor(nY, nX2);
            BitmapColor aColBL = pReadAccess->GetColor(nY2, nX);
            BitmapColor aColBR = pReadAccess->GetColor(nY2, nX2);

            double fRed = ((double)aColTL.GetRed() * fS + (double)aColTR.GetRed() * fRight) * fT
                + ((double)aColBL.GetRed() * fS + (double)aColBR.GetRed() * fRight) * fBottom;
            double fGreen = ((double)aColTL.GetGreen() * fS + (double)aColTR.GetGreen() * fRight) * fT
                + ((double)aColBL.GetGreen() * fS + (double)aColBR.GetGreen() * fRight) * fBottom;
            double fBlue = ((double)aColTL.GetBlue() * fS + (double)aColTR.GetBlue() * fRight) * fT
                + ((double)aColBL.GetBlue() * fS + (double)aColBR.GetBlue() * fRight) * fBottom;

            rCol.SetRed((sal_uInt8)((((double)rCol.GetRed() * (255.0 - fRed)) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
            rCol.SetGreen((sal_uInt8)((((double)rCol.GetGreen() * (255.0 - fGreen)) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
            rCol.SetBlue((sal_uInt8)((((double)rCol.GetBlue() * (255.0 - fBlue)) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
            break;
        }
        case (B3D_TXT_MODE_REP|B3D_TXT_KIND_INT) :
        case (B3D_TXT_MODE_REP|B3D_TXT_KIND_LUM) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            sal_uInt8 nMidVal = (sal_uInt8)((
                (double)pReadAccess->GetLuminance(nY, nX) * fS +
                (double)pReadAccess->GetLuminance(nY, nX2) * fRight) * fT + (
                (double)pReadAccess->GetLuminance(nY2, nX) * fS +
                (double)pReadAccess->GetLuminance(nY2, nX2) * fRight) * fBottom);

            rCol.SetRed(nMidVal);
            rCol.SetGreen(nMidVal);
            rCol.SetBlue(nMidVal);
            break;
        }
        case (B3D_TXT_MODE_MOD|B3D_TXT_KIND_INT) :
        case (B3D_TXT_MODE_MOD|B3D_TXT_KIND_LUM) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            double fMidVal = (
                (double)pReadAccess->GetLuminance(nY, nX) * fS +
                (double)pReadAccess->GetLuminance(nY, nX2) * fRight) * fT + (
                (double)pReadAccess->GetLuminance(nY2, nX) * fS +
                (double)pReadAccess->GetLuminance(nY2, nX2) * fRight) * fBottom;

            rCol.SetRed((sal_uInt8)(((double)rCol.GetRed() * fMidVal) / 255.0));
            rCol.SetGreen((sal_uInt8)(((double)rCol.GetGreen() * fMidVal) / 255.0));
            rCol.SetBlue((sal_uInt8)(((double)rCol.GetBlue() * fMidVal) / 255.0));
            break;
        }
        case (B3D_TXT_MODE_BND|B3D_TXT_KIND_INT) :
        case (B3D_TXT_MODE_BND|B3D_TXT_KIND_LUM) :
        {
            fS = fS - floor(fS);
            fT = fT - floor(fT);
            long nX2, nY2;

            if(fS > 0.5) {
                nX2 = (nX + 1) % GetBitmapSize().Width();
                fS = 1.0 - fS;
            } else
                nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;

            if(fT > 0.5) {
                nY2 = (nY + 1) % GetBitmapSize().Height();
                fT = 1.0 - fT;
            } else
                nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;

            fS += 0.5;
            fT += 0.5;
            double fRight = 1.0 - fS;
            double fBottom = 1.0 - fT;

            double fMidVal = (
                (double)pReadAccess->GetLuminance(nY, nX) * fS +
                (double)pReadAccess->GetLuminance(nY, nX2) * fRight) * fT + (
                (double)pReadAccess->GetLuminance(nY2, nX) * fS +
                (double)pReadAccess->GetLuminance(nY2, nX2) * fRight) * fBottom;
            double fInvMidVal(255.0 - fMidVal);

            rCol.SetRed((sal_uInt8)((((double)rCol.GetRed() * fInvMidVal) + ((double)aColBlend.GetRed() * fMidVal)) / 255.0));
            rCol.SetGreen((sal_uInt8)((((double)rCol.GetGreen() * fInvMidVal) + ((double)aColBlend.GetGreen() * fMidVal)) / 255.0));
            rCol.SetBlue((sal_uInt8)((((double)rCol.GetBlue() * fInvMidVal) + ((double)aColBlend.GetBlue() * fMidVal)) / 255.0));
            break;
        }
    }

// Funktionsfaehige Version komplett auf double precision und
// kuerzesten Pfaden
//
//  // Parameter fuer Farbe der Texturstelle bereitstellen
//  double fRed, fGreen, fBlue;
//
//  // Eventuell glaetten?
//  if(GetTextureFilter() == Base3DTextureLinear)
//  {
//      // Filtern
//      double fLeft = fS - floor(fS);
//      double fTop = fT - floor(fT);
//      double fRight, fBottom;
//      long nX2, nY2;
//
//      if(fLeft > 0.5)
//      {
//          nX2 = (nX + 1) % GetBitmapSize().Width();
//          fLeft = (1.0 - fLeft) + 0.5;
//          fRight = 1.0 - fLeft;
//      }
//      else
//      {
//          nX2 = nX ? nX - 1 : GetBitmapSize().Width() - 1;
//          fLeft = fLeft + 0.5;
//          fRight = 1.0 - fLeft;
//      }
//
//
//      if(fTop > 0.5)
//      {
//          nY2 = (nY + 1) % GetBitmapSize().Height();
//          fTop = (1.0 - fTop) + 0.5;
//          fBottom = 1.0 - fTop;
//      }
//      else
//      {
//          nY2 = nY ? nY - 1 : GetBitmapSize().Height() - 1;
//          fTop = fTop + 0.5;
//          fBottom = 1.0 - fTop;
//      }
//
//      const BitmapColor& rColTL = GetBitmapColor(nX, nY);
//      const BitmapColor& rColTR = GetBitmapColor(nX2, nY);
//      const BitmapColor& rColBL = GetBitmapColor(nX, nY2);
//      const BitmapColor& rColBR = GetBitmapColor(nX2, nY2);
//
//      fRed = ((double)rColTL.GetRed() * fLeft + (double)rColTR.GetRed() * fRight) * fTop
//      + ((double)rColBL.GetRed() * fLeft + (double)rColBR.GetRed() * fRight) * fBottom;
//
//      fGreen = ((double)rColTL.GetGreen() * fLeft + (double)rColTR.GetGreen() * fRight) * fTop
//      + ((double)rColBL.GetGreen() * fLeft + (double)rColBR.GetGreen() * fRight) * fBottom;
//
//      fBlue = ((double)rColTL.GetBlue() * fLeft + (double)rColTR.GetBlue() * fRight) * fTop
//      + ((double)rColBL.GetBlue() * fLeft + (double)rColBR.GetBlue() * fRight) * fBottom;
//  }
//  else
//  {
//      // Nearest Pixel
//      const BitmapColor& rBmCol = GetBitmapColor(nX, nY);
//      fRed = (double)rBmCol.GetRed();
//      fGreen = (double)rBmCol.GetGreen();
//      fBlue = (double)rBmCol.GetBlue();
//  }
//
//  // Jetzt anhand der TextureKind entscheiden
//  if(eKind == Base3DTextureColor)
//  {
//      if(eMode == Base3DTextureReplace)
//      {
//          rCol.SetRed((sal_uInt8)fRed);
//          rCol.SetGreen((sal_uInt8)fGreen);
//          rCol.SetBlue((sal_uInt8)fBlue);
//      }
//      else if(eMode == Base3DTextureModulate)
//      {
//          rCol.SetRed((sal_uInt8)(((double)rCol.GetRed() * fRed) / 255.0));
//          rCol.SetGreen((sal_uInt8)(((double)rCol.GetGreen() * fGreen) / 255.0));
//          rCol.SetBlue((sal_uInt8)(((double)rCol.GetBlue() * fBlue) / 255.0));
//      }
//      else // Base3DTextureBlend
//      {
//          rCol.SetRed((sal_uInt8)((((double)rCol.GetRed() * (255.0 - fRed)) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
//          rCol.SetGreen((sal_uInt8)((((double)rCol.GetGreen() * (255.0 - fGreen)) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
//          rCol.SetBlue((sal_uInt8)((((double)rCol.GetBlue() * (255.0 - fBlue)) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
//      }
//  }
//  else
//  {
//      double fMidVal((fRed + fGreen + fBlue) / 3.0);
//      if(eMode == Base3DTextureReplace)
//      {
//          rCol.SetRed((sal_uInt8)fMidVal);
//          rCol.SetGreen((sal_uInt8)fMidVal);
//          rCol.SetBlue((sal_uInt8)fMidVal);
//      }
//      else if(eMode == Base3DTextureModulate)
//      {
//          rCol.SetRed((sal_uInt8)(((double)rCol.GetRed() * fMidVal) / 255.0));
//          rCol.SetGreen((sal_uInt8)(((double)rCol.GetGreen() * fMidVal) / 255.0));
//          rCol.SetBlue((sal_uInt8)(((double)rCol.GetBlue() * fMidVal) / 255.0));
//      }
//      else // Base3DTextureBlend
//      {
//          double fInvMidVal(255.0 - fMidVal);
//          rCol.SetRed((sal_uInt8)((((double)rCol.GetRed() * fInvMidVal) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
//          rCol.SetGreen((sal_uInt8)((((double)rCol.GetGreen() * fInvMidVal) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
//          rCol.SetBlue((sal_uInt8)((((double)rCol.GetBlue() * fInvMidVal) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
//      }
//  }
}

/*************************************************************************
|*
|* Konstruktor TexturOpenGL
|*
\************************************************************************/

B3dTextureOpenGL::B3dTextureOpenGL(
    TextureAttributes& rAtt,
    BitmapEx& rBmpEx,
    OpenGL& rOGL,
    Base3DTextureKind eKnd,
    Base3DTextureMode eMod,
    Base3DTextureFilter eFlt,
    Base3DTextureWrap eS,
    Base3DTextureWrap eT)
:   B3dTexture(rAtt, rBmpEx, eKnd, eMod, eFlt, eS, eT),
    nTextureName(0)
{
    // TextureName anfordern
    rOGL.GenTextures(1, &nTextureName);
}

/*************************************************************************
|*
|* Destruktor TexturOpenGL
|*
\************************************************************************/

B3dTextureOpenGL::~B3dTextureOpenGL()
{
}

/*************************************************************************
|*
|* In OpenGL die Textur zerstoeren
|*
\************************************************************************/

void B3dTextureOpenGL::DestroyOpenGLTexture(OpenGL& rOpenGL)
{
    // OpenGL Textur wieder freigeben
    rOpenGL.DeleteTextures(1, &nTextureName);
}

/*************************************************************************
|*
|* Setze diese Textur in OpenGL als aktuelle Textur
|*
\************************************************************************/

void B3dTextureOpenGL::MakeCurrentTexture(OpenGL& rOpenGL)
{
    // Eventuell Textur erst erzeugen?
    if(!rOpenGL.IsTexture(nTextureName) || bTextureKindChanged)
    {
        // Textur erzeugen und binden
        CreateOpenGLTexture(rOpenGL);
    }
    else
    {
        // Jetzt Textur binden
        rOpenGL.BindTexture(GL_TEXTURE_2D, nTextureName);
    }

    // Nun die Parameter an der Textur setzen
    switch(GetTextureWrapS())
    {
        case Base3DTextureSingle :
        case Base3DTextureClamp :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            break;
        case Base3DTextureRepeat :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            break;
    }
    switch(GetTextureWrapT())
    {
        case Base3DTextureSingle :
        case Base3DTextureClamp :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            break;
        case Base3DTextureRepeat :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
    }
    switch(GetTextureFilter())
    {
        case Base3DTextureNearest :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case Base3DTextureLinear :
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            rOpenGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
    }
    switch(GetTextureMode())
    {
        case Base3DTextureReplace :
            rOpenGL.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            break;
        case Base3DTextureModulate :
            rOpenGL.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            break;
        case Base3DTextureBlend :
        {
            rOpenGL.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
            float fArray[4] = {
                ((float)GetBlendColor().GetRed()) / (float)255.0,
                ((float)GetBlendColor().GetGreen()) / (float)255.0,
                ((float)GetBlendColor().GetBlue()) / (float)255.0,
                ((float)GetBlendColor().GetTransparency()) / (float)255.0
            };
            rOpenGL.TexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, fArray);
            break;
        }
    }
}

/*************************************************************************
|*
|* Erzeuge diese Textur als OpenGL-Textur
|*
\************************************************************************/

void B3dTextureOpenGL::CreateOpenGLTexture(OpenGL& rOpenGL)
{
    Size aSize(0,0);

    // Groesse entscheiden (auf 1024 begrenzen)
    for(sal_uInt16 a=1;a<0x0400 && (!aSize.Width() || !aSize.Height());a<<=1)
    {
        if(!aSize.Width() && (a>=GetBitmapSize().Width()))
            aSize.Width() = a;
        if(!aSize.Height() && (a>=GetBitmapSize().Height()))
            aSize.Height() = a;
    }

    // begrenzen falls groesser als 1024
    if(!aSize.Width())
        aSize.Width() = 0x0400;
    if(!aSize.Height())
        aSize.Height() = 0x0400;

    // Minimalgroesse garantieren, auch an 4Byte-Alignment denken
    // falls hier mal geaendert wird... (siehe OpenGL Bitmap Befehle)
    if(aSize.Width() < 8)
        aSize.Width() = 8;
    if(aSize.Height() < 8)
        aSize.Height() = 8;

    // Skalierte Bitmap anlegen
    sal_Bool bUsesAlpha(!!GetAlphaMask());
    Bitmap aLocalBitmap(GetBitmap());
    AlphaMask aTransAlphaMask;

    if(bUsesAlpha)
    {
        aTransAlphaMask = GetAlphaMask();
    }

    if(aSize != GetBitmapSize())
    {
        aLocalBitmap.Scale((double)aSize.Width() / (double)GetBitmapSize().Width(),
            (double)aSize.Height() / (double)GetBitmapSize().Height());

        if(bUsesAlpha)
        {
            aTransAlphaMask.Scale((double)aSize.Width() / (double)GetBitmapSize().Width(),
                (double)aSize.Height() / (double)GetBitmapSize().Height());
        }
    }

    // Falls es sich um eine nur einmal zu wiederholende Bitmap
    // handelt, lege nun eine mit einem definierten Rand an
    if(GetTextureWrapS() == Base3DTextureSingle || GetTextureWrapT() == Base3DTextureSingle)
    {
        Bitmap aHelpBitmap(aLocalBitmap);
        AlphaMask aTransAlphaHelpMask;

        if(bUsesAlpha)
        {
            aTransAlphaHelpMask = aTransAlphaMask;
        }

        Size aNewSize(aSize);
        Point aNewPos(0, 0);

        if(GetTextureWrapS() == Base3DTextureSingle)
        {
            aNewSize.Width() -= 4;
            aNewPos.X() = 2;
        }
        if(GetTextureWrapT() == Base3DTextureSingle)
        {
            aNewSize.Height() -= 4;
            aNewPos.Y() = 2;
        }

        aHelpBitmap.Scale((double)aNewSize.Width() / (double)aSize.Width(),
            (double)aNewSize.Height() / (double)aSize.Height());
        Color aEraseCol = GetTextureColor();
        aLocalBitmap.Erase(aEraseCol);
        Point aPoint;
        Rectangle aCopySrc(aPoint, aNewSize);
        Rectangle aCopyDest(aNewPos, aNewSize);
        aLocalBitmap.CopyPixel(aCopyDest, aCopySrc, &aHelpBitmap);

        if(bUsesAlpha)
        {
            aTransAlphaHelpMask.Scale((double)aNewSize.Width() / (double)aSize.Width(),
                (double)aNewSize.Height() / (double)aSize.Height());
            aTransAlphaMask.Erase(0);
            aTransAlphaMask.CopyPixel(aCopyDest, aCopySrc, &aTransAlphaHelpMask);
        }
    }

    // Lesezugriff auf neue Bitmap holen
    BitmapReadAccess* pLocalReadAccess = aLocalBitmap.AcquireReadAccess();
    BitmapReadAccess* pLocalAlphaReadAccess = (bUsesAlpha) ? aTransAlphaMask.AcquireReadAccess() : NULL;
    sal_Bool bGotReadAccess((bUsesAlpha)
        ? (pLocalReadAccess != 0 && pLocalAlphaReadAccess != 0) : pLocalReadAccess != 0);

    if(bGotReadAccess)
    {
        // Buffer holen
        sal_Int32 nSize(aSize.Width() * aSize.Height());
        sal_Int32 nAllocSize(nSize);

        if(GetTextureKind() == Base3DTextureColor)
        {
            nAllocSize += (2 * nSize);
        }
        if(bUsesAlpha)
        {
            nAllocSize += nSize;
        }

        GL_UINT8 pBuffer = (GL_UINT8) rtl_allocateMemory(nAllocSize);

        if(pBuffer)
        {
            // Daten kopieren
            GL_UINT8 pRunner = pBuffer;
            if(GetTextureKind() == Base3DTextureColor)
            {
                if(pLocalReadAccess->HasPalette())
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            BitmapColor rCol = pLocalReadAccess->GetPaletteColor(pLocalReadAccess->GetPixel(a, b));
                            *pRunner++ = rCol.GetRed();
                            *pRunner++ = rCol.GetGreen();
                            *pRunner++ = rCol.GetBlue();

                            if(bUsesAlpha)
                            {
                                BitmapColor rTrn = pLocalAlphaReadAccess->GetPixel(a, b);
                                *pRunner++ = (BYTE)255 - rTrn.GetIndex();
                            }
                        }
                    }
                }
                else
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            BitmapColor rCol = pLocalReadAccess->GetPixel(a, b);
                            *pRunner++ = rCol.GetRed();
                            *pRunner++ = rCol.GetGreen();
                            *pRunner++ = rCol.GetBlue();

                            if(bUsesAlpha)
                            {
                                BitmapColor rTrn = pLocalAlphaReadAccess->GetPixel(a, b);
                                *pRunner++ = (BYTE)255 - rTrn.GetIndex();
                            }
                        }
                    }
                }
            }
            else
            {
                if(pLocalReadAccess->HasPalette())
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            BitmapColor rCol = pLocalReadAccess->GetPaletteColor(pLocalReadAccess->GetPixel(a, b));
                            *pRunner++ = (rCol.GetRed() + rCol.GetGreen() + rCol.GetBlue()) / 3;

                            if(bUsesAlpha)
                            {
                                BitmapColor rTrn = pLocalAlphaReadAccess->GetPixel(a, b);
                                *pRunner++ = (BYTE)255 - rTrn.GetIndex();
                            }
                        }
                    }
                }
                else
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            BitmapColor rCol = pLocalReadAccess->GetPixel(a, b);
                            *pRunner++ = (rCol.GetRed() + rCol.GetGreen() + rCol.GetBlue()) / 3;

                            if(bUsesAlpha)
                            {
                                BitmapColor rTrn = pLocalAlphaReadAccess->GetPixel(a, b);
                                *pRunner++ = (BYTE)255 - rTrn.GetIndex();
                            }
                        }
                    }
                }
            }

            // Textur das erste mal binden und damit initialisieren
            rOpenGL.BindTexture(GL_TEXTURE_2D, nTextureName);

            // Jetzt ein glTexImage2D() ausfuehren
            GLint nInternalFormat = 0;
            GLint nFormat = 0;

            switch(GetTextureKind())
            {
                case Base3DTextureLuminance:
                {
                    if(bUsesAlpha)
                    {
                        nInternalFormat = GL_LUMINANCE_ALPHA;
                        nFormat = GL_LUMINANCE_ALPHA;
                    }
                    else
                    {
                        nInternalFormat = GL_LUMINANCE;
                        nFormat = GL_LUMINANCE;
                    }
                    break;
                }
                case Base3DTextureIntensity:
                {
                    if(bUsesAlpha)
                    {
                        nInternalFormat = GL_LUMINANCE_ALPHA;
                        nFormat = GL_LUMINANCE_ALPHA;
                    }
                    else
                    {
                        nInternalFormat = GL_INTENSITY;
                        nFormat = GL_LUMINANCE;
                    }
                    break;
                }
                case Base3DTextureColor:
                {
                    if(bUsesAlpha)
                    {
                        nInternalFormat = GL_RGBA;
                        nFormat = GL_RGBA;
                    }
                    else
                    {
                        nInternalFormat = GL_RGB;
                        nFormat = GL_RGB;
                    }
                    break;
                }
            }

            rOpenGL.TexImage2D(GL_TEXTURE_2D, 0, nInternalFormat,
                (GLsizei)aSize.Width(),
                (GLsizei)aSize.Height(),
                0, nFormat, GL_UNSIGNED_BYTE, pBuffer);

            rtl_freeMemory(pBuffer);
        }
    }

    // Lesezugriff freigeben
    if (pLocalReadAccess) aLocalBitmap.ReleaseAccess(pLocalReadAccess);
    if (pLocalAlphaReadAccess) aTransAlphaMask.ReleaseAccess(pLocalAlphaReadAccess);

    // Hinweis auf Veraenderung der Texturart auf jeden Fall elliminieren
    bTextureKindChanged = sal_False;
}

// eof
