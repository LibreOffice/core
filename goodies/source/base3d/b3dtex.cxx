/*************************************************************************
 *
 *  $RCSfile: b3dtex.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _B3D_B3DTEX_HXX
#include "b3dtex.hxx"
#endif

#ifndef _B3D_B3DOPNGL_HXX
#include "b3dopngl.hxx"
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif

/*************************************************************************
|*
|* Klassen fuer TexturAttribute beim Anfordern von Texturen
|*
\************************************************************************/

TextureAttributes::TextureAttributes()
{
}

// Fuer Bitmaps

TextureAttributesBitmap::TextureAttributesBitmap(Bitmap aBmp)
:   TextureAttributes(),
    aBitmapAttribute(aBmp)
{
}

BOOL TextureAttributesBitmap::operator==(const TextureAttributes& rAtt) const
{
    if(GetTextureAttributeType() == rAtt.GetTextureAttributeType())
    {
        const TextureAttributesBitmap& rAttBmp = (const TextureAttributesBitmap&)rAtt;

        if(rAttBmp.aBitmapAttribute == aBitmapAttribute)
            return TRUE;
    }
    return FALSE;
}

UINT16 TextureAttributesBitmap::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_BITMAP;
}

// Fuer Gradientfills

TextureAttributesGradient::TextureAttributesGradient(void* pF, void *pSC)
:   TextureAttributes(),
    pFill(pF),
    pStepCount(pSC)
{
}

BOOL TextureAttributesGradient::operator==(const TextureAttributes& rAtt) const
{
    if(GetTextureAttributeType() == rAtt.GetTextureAttributeType())
    {
        const TextureAttributesGradient& rAttGra = (const TextureAttributesGradient&)rAtt;

        if(rAttGra.pFill == pFill
            && rAttGra.pStepCount == pStepCount)
            return TRUE;
    }
    return FALSE;
}

UINT16 TextureAttributesGradient::GetTextureAttributeType() const
{
    return TEXTURE_ATTRIBUTE_TYPE_GRADIENT;
}

// Fuer Hatchfills

TextureAttributesHatch::TextureAttributesHatch(void* pF)
:   TextureAttributes(),
    pFill(pF)
{
}

BOOL TextureAttributesHatch::operator==(const TextureAttributes& rAtt) const
{
    if(GetTextureAttributeType() == rAtt.GetTextureAttributeType())
    {
        const TextureAttributesHatch& rAttHat = (const TextureAttributesHatch&)rAtt;

        if(rAttHat.pFill == pFill)
            return TRUE;
    }
    return FALSE;
}

UINT16 TextureAttributesHatch::GetTextureAttributeType() const
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
    Bitmap& rBmp,
    Base3DTextureKind eKnd,
    Base3DTextureMode eMod,
    Base3DTextureFilter eFlt,
    Base3DTextureWrap eS,
    Base3DTextureWrap eT)
:   aBitmap(rBmp),
    pReadAccess(NULL),
    nUsageCount(B3D_TEXTURE_LIFETIME),
    eKind(eKnd),
    eMode(eMod),
    eFilter(eFlt),
    eWrapS(eS),
    eWrapT(eT),
    nSwitchVal(0),
    bTextureKindChanged(FALSE)
{
    // ReadAccess auf Textur anfordern
    pReadAccess = aBitmap.AcquireReadAccess();
    DBG_ASSERT(pReadAccess, "AW: Keinen Lesezugriff auf Textur-Bitmap bekommen");

    // Attribute kopieren
    switch(rAtt.GetTextureAttributeType())
    {
        case TEXTURE_ATTRIBUTE_TYPE_BITMAP :
            pAttributes = new TextureAttributesBitmap(
                ((TextureAttributesBitmap&)rAtt).GetBitmapAttribute());
            break;

        case TEXTURE_ATTRIBUTE_TYPE_GRADIENT :
            pAttributes = new TextureAttributesGradient(
                ((TextureAttributesGradient&)rAtt).GetFillAttribute(),
                ((TextureAttributesGradient&)rAtt).GetStepCountAttribute());
            break;

        case TEXTURE_ATTRIBUTE_TYPE_HATCH :
            pAttributes = new TextureAttributesHatch(
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
        bTextureKindChanged = TRUE;
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
        bTextureKindChanged = TRUE;
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
            bTextureKindChanged = TRUE;
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
            bTextureKindChanged = TRUE;
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

const BitmapColor B3dTexture::GetBitmapColor(long nX, long nY)
{
    return pReadAccess->GetColor(nY, nX);
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
        bTextureKindChanged = TRUE;
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
    BOOL bOnTexture(TRUE);

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
            bOnTexture = FALSE;
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
                bOnTexture = FALSE;
        }

        if(bOnTexture)
            aBmCol = pReadAccess->GetColor(nY, nX);
    }

    // Falls die Position nicht innerhalb der Textur ist, auch das Filtern
    // unterdruecken um keine falschen BitmapAcesses zu bekommen
    UINT8 nLocalSwitchVal(nSwitchVal);
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
            rCol.SetRed((UINT8)( ((UINT16)rCol.GetRed() * (UINT16)aBmCol.GetRed())>>8 ));
            rCol.SetGreen((UINT8)( ((UINT16)rCol.GetGreen() * (UINT16)aBmCol.GetGreen())>>8 ));
            rCol.SetBlue((UINT8)( ((UINT16)rCol.GetBlue() * (UINT16)aBmCol.GetBlue())>>8 ));
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_COL) :
        {
            rCol.SetRed((UINT8)( ((UINT16)rCol.GetRed() * (0x00ff - (UINT16)aBmCol.GetRed()))
                + ((UINT16)aColBlend.GetRed() * (UINT16)aBmCol.GetRed()) ));
            rCol.SetGreen((UINT8)( ((UINT16)rCol.GetGreen() * (0x00ff - (UINT16)aBmCol.GetGreen()))
                + ((UINT16)aColBlend.GetGreen() * (UINT16)aBmCol.GetGreen()) ));
            rCol.SetBlue((UINT8)( ((UINT16)rCol.GetBlue() * (0x00ff - (UINT16)aBmCol.GetBlue()))
                + ((UINT16)aColBlend.GetBlue() * (UINT16)aBmCol.GetBlue()) ));
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
            UINT16 nMidCol = (aBmCol.GetRed() + aBmCol.GetGreen() + aBmCol.GetBlue()) / 3;
            rCol.SetRed((UINT8)( ((UINT16)rCol.GetRed() * nMidCol)>>8 ));
            rCol.SetGreen((UINT8)( ((UINT16)rCol.GetGreen() * nMidCol)>>8 ));
            rCol.SetBlue((UINT8)( ((UINT16)rCol.GetBlue() * nMidCol)>>8 ));
            break;
        }
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_INT) :
        case (B3D_TXT_FLTR_NEA|B3D_TXT_MODE_BND|B3D_TXT_KIND_LUM) :
        {
            UINT16 nMidCol = (aBmCol.GetRed() + aBmCol.GetGreen() + aBmCol.GetBlue()) / 3;
            UINT16 nInvMidCol = 0x00ff - nMidCol;
            rCol.SetRed((UINT8)( ((UINT16)rCol.GetRed() * nInvMidCol)
                + ((UINT16)aColBlend.GetRed() * nMidCol) ));
            rCol.SetGreen((UINT8)( ((UINT16)rCol.GetGreen() * nInvMidCol)
                + ((UINT16)aColBlend.GetGreen() * nMidCol) ));
            rCol.SetBlue((UINT8)( ((UINT16)rCol.GetBlue() * nInvMidCol)
                + ((UINT16)aColBlend.GetBlue() * nMidCol) ));
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

            rCol.SetRed((UINT8)(((double)aColTL.GetRed() * fS + (double)aColTR.GetRed() * fRight) * fT
                + ((double)aColBL.GetRed() * fS + (double)aColBR.GetRed() * fRight) * fBottom));
            rCol.SetGreen((UINT8)(((double)aColTL.GetGreen() * fS + (double)aColTR.GetGreen() * fRight) * fT
                + ((double)aColBL.GetGreen() * fS + (double)aColBR.GetGreen() * fRight) * fBottom));
            rCol.SetBlue((UINT8)(((double)aColTL.GetBlue() * fS + (double)aColTR.GetBlue() * fRight) * fT
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

            rCol.SetRed((UINT8)(((double)rCol.GetRed() * fRed) / 255.0));
            rCol.SetGreen((UINT8)(((double)rCol.GetGreen() * fGreen) / 255.0));
            rCol.SetBlue((UINT8)(((double)rCol.GetBlue() * fBlue) / 255.0));
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

            rCol.SetRed((UINT8)((((double)rCol.GetRed() * (255.0 - fRed)) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
            rCol.SetGreen((UINT8)((((double)rCol.GetGreen() * (255.0 - fGreen)) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
            rCol.SetBlue((UINT8)((((double)rCol.GetBlue() * (255.0 - fBlue)) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
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

            UINT8 nMidVal = (UINT8)((
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

            rCol.SetRed((UINT8)(((double)rCol.GetRed() * fMidVal) / 255.0));
            rCol.SetGreen((UINT8)(((double)rCol.GetGreen() * fMidVal) / 255.0));
            rCol.SetBlue((UINT8)(((double)rCol.GetBlue() * fMidVal) / 255.0));
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

            rCol.SetRed((UINT8)((((double)rCol.GetRed() * fInvMidVal) + ((double)aColBlend.GetRed() * fMidVal)) / 255.0));
            rCol.SetGreen((UINT8)((((double)rCol.GetGreen() * fInvMidVal) + ((double)aColBlend.GetGreen() * fMidVal)) / 255.0));
            rCol.SetBlue((UINT8)((((double)rCol.GetBlue() * fInvMidVal) + ((double)aColBlend.GetBlue() * fMidVal)) / 255.0));
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
//          rCol.SetRed((UINT8)fRed);
//          rCol.SetGreen((UINT8)fGreen);
//          rCol.SetBlue((UINT8)fBlue);
//      }
//      else if(eMode == Base3DTextureModulate)
//      {
//          rCol.SetRed((UINT8)(((double)rCol.GetRed() * fRed) / 255.0));
//          rCol.SetGreen((UINT8)(((double)rCol.GetGreen() * fGreen) / 255.0));
//          rCol.SetBlue((UINT8)(((double)rCol.GetBlue() * fBlue) / 255.0));
//      }
//      else // Base3DTextureBlend
//      {
//          rCol.SetRed((UINT8)((((double)rCol.GetRed() * (255.0 - fRed)) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
//          rCol.SetGreen((UINT8)((((double)rCol.GetGreen() * (255.0 - fGreen)) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
//          rCol.SetBlue((UINT8)((((double)rCol.GetBlue() * (255.0 - fBlue)) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
//      }
//  }
//  else
//  {
//      double fMidVal((fRed + fGreen + fBlue) / 3.0);
//      if(eMode == Base3DTextureReplace)
//      {
//          rCol.SetRed((UINT8)fMidVal);
//          rCol.SetGreen((UINT8)fMidVal);
//          rCol.SetBlue((UINT8)fMidVal);
//      }
//      else if(eMode == Base3DTextureModulate)
//      {
//          rCol.SetRed((UINT8)(((double)rCol.GetRed() * fMidVal) / 255.0));
//          rCol.SetGreen((UINT8)(((double)rCol.GetGreen() * fMidVal) / 255.0));
//          rCol.SetBlue((UINT8)(((double)rCol.GetBlue() * fMidVal) / 255.0));
//      }
//      else // Base3DTextureBlend
//      {
//          double fInvMidVal(255.0 - fMidVal);
//          rCol.SetRed((UINT8)((((double)rCol.GetRed() * fInvMidVal) + ((double)aColBlend.GetRed() * fRed)) / 255.0));
//          rCol.SetGreen((UINT8)((((double)rCol.GetGreen() * fInvMidVal) + ((double)aColBlend.GetGreen() * fGreen)) / 255.0));
//          rCol.SetBlue((UINT8)((((double)rCol.GetBlue() * fInvMidVal) + ((double)aColBlend.GetBlue() * fBlue)) / 255.0));
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
    Bitmap& rBmp,
    OpenGL& rOGL,
    Base3DTextureKind eKnd,
    Base3DTextureMode eMod,
    Base3DTextureFilter eFlt,
    Base3DTextureWrap eS,
    Base3DTextureWrap eT)
:   B3dTexture(rAtt, rBmp, eKnd, eMod, eFlt, eS, eT),
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
    for(UINT16 a=1;a<0x0400 && (!aSize.Width() || !aSize.Height());a<<=1)
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
    Bitmap aBitmap(GetBitmap());
    if(aSize != GetBitmapSize())
        aBitmap.Scale((double)aSize.Width() / (double)GetBitmapSize().Width(),
            (double)aSize.Height() / (double)GetBitmapSize().Height());

    // Falls es sich um eine nur einmal zu wiederholende Bitmap
    // handelt, lege nun eine mit einem definierten Rand an
    if(GetTextureWrapS() == Base3DTextureSingle
        || GetTextureWrapT() == Base3DTextureSingle)
    {
        Bitmap aHelpBitmap(aBitmap);
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
        aBitmap.Erase(aEraseCol);
        Point aPoint;
        Rectangle aCopySrc(aPoint, aNewSize);
        Rectangle aCopyDest(aNewPos, aNewSize);
        aBitmap.CopyPixel(aCopyDest, aCopySrc, &aHelpBitmap);
    }

    // Lesezugriff auf neue Bitmap holen
    BitmapReadAccess* pReadAccess = aBitmap.AcquireReadAccess();
    if(pReadAccess)
    {
        // Buffer holen
        long nSize = aSize.Width() * aSize.Height();
        if(GetTextureKind() == Base3DTextureColor)
            nSize *= 3;
        GL_UINT8 pBuffer = (GL_UINT8)SvMemAlloc(nSize);

        if(pBuffer)
        {
            // Daten kopieren
            GL_UINT8 pRunner = pBuffer;
            if(GetTextureKind() == Base3DTextureColor)
            {
                if(pReadAccess->HasPalette())
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            const BitmapColor& rCol = pReadAccess->GetPaletteColor(
                                pReadAccess->GetPixel(a, b));
                            *pRunner++ = rCol.GetRed();
                            *pRunner++ = rCol.GetGreen();
                            *pRunner++ = rCol.GetBlue();
                        }
                    }
                }
                else
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            const BitmapColor& rCol = pReadAccess->GetPixel(a, b);
                            *pRunner++ = rCol.GetRed();
                            *pRunner++ = rCol.GetGreen();
                            *pRunner++ = rCol.GetBlue();
                        }
                    }
                }
            }
            else
            {
                if(pReadAccess->HasPalette())
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            const BitmapColor& rCol = pReadAccess->GetPaletteColor(
                                pReadAccess->GetPixel(a, b));
                            *pRunner++ = (rCol.GetRed() + rCol.GetGreen() + rCol.GetBlue()) / 3;
                        }
                    }
                }
                else
                {
                    for(long a=0;a<aSize.Height();a++)
                    {
                        for(long b=0;b<aSize.Width();b++)
                        {
                            const BitmapColor& rCol = pReadAccess->GetPixel(a, b);
                            *pRunner++ = (rCol.GetRed() + rCol.GetGreen() + rCol.GetBlue()) / 3;
                        }
                    }
                }
            }

            // Textur das erste mal binden und damit initialisieren
            rOpenGL.BindTexture(GL_TEXTURE_2D, nTextureName);

            // Jetzt ein glTexImage2D() ausfuehren
            GLint nInternalFormat;
            GLint nFormat;

            switch(GetTextureKind())
            {
                case Base3DTextureLuminance:
                    nInternalFormat = GL_LUMINANCE;
                    nFormat = GL_LUMINANCE;
                    break;
                case Base3DTextureIntensity:
                    nInternalFormat = GL_INTENSITY;
                    nFormat = GL_LUMINANCE;
                    break;
                case Base3DTextureColor:
                    nInternalFormat = GL_RGB;
                    nFormat = GL_RGB;
                    break;
            }

            rOpenGL.TexImage2D(GL_TEXTURE_2D, 0, nInternalFormat,
                (GLsizei)aSize.Width(),
                (GLsizei)aSize.Height(),
                0, nFormat, GL_UNSIGNED_BYTE, pBuffer);

            SvMemFree(pBuffer);
        }

        // Lesezugriff freigeben
        aBitmap.ReleaseAccess(pReadAccess);
    }

    // Hinweis auf Veraenderung der Texturart auf jeden Fall elliminieren
    bTextureKindChanged = FALSE;
}

