/*************************************************************************
 *
 *  $RCSfile: b3ddeflt.cxx,v $
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

#ifndef _B3D_B3DDEFLT_HXX
#include "b3ddeflt.hxx"
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _B3D_B3DTEX_HXX
#include "b3dtex.hxx"
#endif

/*************************************************************************
|*
|* Konstruktor Base3DDefault
|*
\************************************************************************/

Base3DDefault::Base3DDefault(OutputDevice* pOutDev)
:   Base3DCommon(pOutDev),
    aZBuffer(),
    aPicture(),
    aMonoTransparence(),
    aAlphaTransparence(),
    aClearValue(Color(0x00ffffff)),
    pZBufferWrite(NULL),
    pPictureWrite(NULL),
    pTransparenceWrite(NULL),
    fDetail(1.0),
    bReducedDetail(FALSE),
    bDetailBackedup(FALSE),
    fDetailBackup( -1.0 ),
    nMaxPixels(500000)
{
}

/*************************************************************************
|*
|* Destruktor Base3DDefault
|*
\************************************************************************/

Base3DDefault::~Base3DDefault()
{
    // Alle Bitmap-Zugriffe freigeben
    ReleaseAccess();
}

/*************************************************************************
|*
|* Typbestimmung
|*
\************************************************************************/

UINT16 Base3DDefault::GetBase3DType()
{
    return BASE3D_TYPE_DEFAULT;
}

/*************************************************************************
|*
|* Darstellungsqualitaet setzen
|*
\************************************************************************/

void Base3DDefault::SetDisplayQuality(UINT8 nNew)
{
    // Entsprechende PixelGrenze setzen
    SetMaxPixels(((long)nNew * 3500) + 3500);

    // call parent
    Base3D::SetDisplayQuality(nNew);
}

/*************************************************************************
|*
|* Vergroeberungsstufe setzen
|*
\************************************************************************/

void Base3DDefault::SetDetail(double fNew)
{
    // nach unten begrenzen
    if(fNew > 1.0)
        fNew = 1.0;

    fDetail = fNew;
    if(fDetail < 1.0)
    {
        bReducedDetail = TRUE;
    }
    else
    {
        bReducedDetail = FALSE;
    }
}

/*************************************************************************
|*
|* BitmapAccess holen
|*
\************************************************************************/

void Base3DDefault::AcquireAccess()
{
    // Alle accesses holen
    pZBufferWrite = aZBuffer.AcquireWriteAccess();
    pPictureWrite = aPicture.AcquireWriteAccess();
    pTransparenceWrite = (GetTransparentPartsContainedHint())
        ? aAlphaTransparence.AcquireWriteAccess()
        : aMonoTransparence.AcquireWriteAccess();
}

/*************************************************************************
|*
|* BitmapAccess freigeben
|*
\************************************************************************/

void Base3DDefault::ReleaseAccess()
{
    // Alle accesses wieder freigeben
    if(pZBufferWrite)
    {
        delete pZBufferWrite;
        pZBufferWrite = NULL;
    }

    if(pPictureWrite)
    {
        delete pPictureWrite;
        pPictureWrite = NULL;
    }

    if(pTransparenceWrite)
    {
        delete pTransparenceWrite;
        pTransparenceWrite = NULL;
    }
}

/*************************************************************************
|*
|* Start der Szenenbeschreibung:
|*
\************************************************************************/

void Base3DDefault::StartScene()
{
    // Zugriffe freigeben
    ReleaseAccess();

    // Groesse der Bitmaps anpassen?
    BOOL bSizeHasChanged = (aLocalSizePixel.GetSize() != aPicture.GetSizePixel());

    // Neue BitMaps fuer ZBuffer und Picture allokieren
    if(bSizeHasChanged || !aZBuffer || !aPicture)
    {
        aZBuffer = Bitmap(aLocalSizePixel.GetSize(), 24);
        aPicture = Bitmap(aLocalSizePixel.GetSize(), 24);
    }

    // ZBuffer loeschen
    aZBuffer.Erase(aClearValue);

    // Bild loeschen
    aPicture.Erase( GetOutputDevice()->GetBackground().GetColor() );

    // Neue Transparenz-Bitmap allokieren
    if(GetTransparentPartsContainedHint())
    {
        // Alpha-Channel
        if(bSizeHasChanged || !aAlphaTransparence)
        {
            aAlphaTransparence = AlphaMask(aLocalSizePixel.GetSize());
            if(!!aMonoTransparence)
                aMonoTransparence = Bitmap();
        }

        // zu Beginn alles transparent
        aAlphaTransparence.Erase(BYTE(0xff));
    }
    else
    {
        // Mono-Channel
        if(bSizeHasChanged || !aMonoTransparence)
        {
            aMonoTransparence = Bitmap(aLocalSizePixel.GetSize(), 1);
            if(!!aAlphaTransparence)
                aAlphaTransparence = AlphaMask();
        }

        // zu Beginn alles transparent
        Color aEraseCol(COL_WHITE);
        aMonoTransparence.Erase(aEraseCol);
    }

    // Zugriffe wieder holen
    AcquireAccess();

    // lokale ClipRegion anpassen
    if(IsScissorRegionActive())
    {
        // Default specifics for scissoring
        aDefaultScissorRectangle = GetScissorRegionPixel();
        aDefaultScissorRectangle -= aSizePixel.TopLeft();

        // Detailstufe beachten
        if(bReducedDetail && fDetail != 0.0)
        {
            long nReducedWidth = (long)((double)(aDefaultScissorRectangle.GetWidth() - 1) * fDetail);
            long nReducedHeight = (long)((double)(aDefaultScissorRectangle.GetHeight() - 1)* fDetail);
            aDefaultScissorRectangle.SetSize(Size(nReducedWidth + 1, nReducedHeight + 1));
        }
    }

    // call parent
    Base3DCommon::StartScene();
}

/*************************************************************************
|*
|* Ende der Szenenbeschreibung:
|*
\************************************************************************/

void Base3DDefault::EndScene()
{
    // Zugriffe freigeben
    ReleaseAccess();

    // Ausgabe der erzeugten BitMap
    BitmapEx aBitmapEx;

    if(GetTransparentPartsContainedHint())
    {
        // Alpha-Transparenz
        aBitmapEx = BitmapEx(aPicture, aAlphaTransparence);
    }
    else
    {
        // Mono-Transparenz
        aBitmapEx = BitmapEx(aPicture, aMonoTransparence);
    }

    // Dithern
    UINT16 nBitCount = GetOutputDevice()->GetBitCount();
    if( GetOutputDevice()->GetOutDevType() != OUTDEV_PRINTER && nBitCount <= 16 && GetDither())
    {
        aBitmapEx.Dither(nBitCount <= 8
            ? BMP_DITHER_MATRIX
            : BMP_DITHER_FLOYD_16);
    }

    if(GetOutputDevice()->GetConnectMetaFile() != NULL)
    {
        Rectangle aLogicRect;
        aLogicRect = GetOutputDevice()->PixelToLogic(aSizePixel);
        aBitmapEx.Draw(GetOutputDevice(), aLogicRect.TopLeft(), aLogicRect.GetSize());
    }
    else
    {
        BOOL bWasEnabled = GetOutputDevice()->IsMapModeEnabled();
        GetOutputDevice()->EnableMapMode(FALSE);

#ifdef DBG_UTIL     // draw for testing
        static BOOL bDoDrawBitmapForTesting(FALSE);
        if(bDoDrawBitmapForTesting)
        {
            Bitmap aBmp( aBitmapEx.GetMask() );
            aBmp.Convert( BMP_CONVERSION_4BIT_COLORS );
            aBmp.Replace( COL_WHITE, COL_LIGHTRED );
            GetOutputDevice()->DrawBitmap( aSizePixel.TopLeft(), aSizePixel.GetSize(), aBmp );
            GetOutputDevice()->SetFillColor( COL_LIGHTRED );
            GetOutputDevice()->SetLineColor( COL_LIGHTRED );
            GetOutputDevice()->DrawRect( aSizePixel );
        }
#endif

        aBitmapEx.Draw(GetOutputDevice(), aSizePixel.TopLeft(), aSizePixel.GetSize());
        GetOutputDevice()->EnableMapMode(bWasEnabled);
    }

    // Zugriffe wieder holen
    AcquireAccess();

    // eventuelle temporaere Reduzierung der Aufloesung zuruecknehmen
    if(bDetailBackedup)
    {
        SetDetail(fDetailBackup);
        bDetailBackedup = FALSE;
    }

    // call parent
    Base3DCommon::EndScene();
}

/*************************************************************************
|*
|* Callbacks bei Matrixaenderungen
|*
|* Ausgaberechteck innerhalb des OutputDevice festlegen. Die Koordinaten
|* sind device-spezifisch, muessen also evtl. erst auf pixelkoordinaten
|* umgerechnet werden
|*
\************************************************************************/

void Base3DDefault::SetTransformationSet(B3dTransformationSet* pSet)
{
    // call parent
    Base3DCommon::SetTransformationSet(pSet);

    if(GetTransformationSet())
    {
        // eventuelle temporaere Reduzierung der Aufloesung zuruecknehmen
        if(bDetailBackedup)
        {
            SetDetail(fDetailBackup);
            bDetailBackedup = FALSE;
        }

        // Neue Groesse fuer die Ausgabe
        aSizePixel = GetOutputDevice()->LogicToPixel(
            GetTransformationSet()->GetLogicalViewportBounds());

        // Eventuell durch ClipRegion eingeschraenkt? Dies
        // muss beachtet werden
        if(IsScissorRegionActive())
        {
            // draw region even smaller
            aSizePixel.Intersection(GetScissorRegionPixel());
        }

        // Testen, ob die Bitmap zu gross wird
        aLocalSizePixel = aSizePixel;
        long nQuadSize = aLocalSizePixel.GetWidth() * aLocalSizePixel.GetHeight();

        if(nQuadSize > GetMaxPixels())
        {
            // Groesse reduzieren
            double fFactor = sqrt((double)GetMaxPixels() / (double)nQuadSize);

            // Bei Druckjobs die Reduzierung einschraenken
            if(fFactor < 0.25 && GetOutputDevice()->GetOutDevType() == OUTDEV_PRINTER)
                fFactor = 0.25;

            // Wird hier mehr reduziert als sowieso schon eingestellt ist?
            if(fFactor < fDetail)
            {
                fDetailBackup = GetDetail();
                bDetailBackedup = TRUE;
                SetDetail(fFactor);
            }
        }

        // Detailstufe beachten
        if(bReducedDetail && fDetail != 0.0)
        {
            long nReducedWidth = (long)((double)(aLocalSizePixel.GetWidth() - 1) * fDetail);
            long nReducedHeight = (long)((double)(aLocalSizePixel.GetHeight() - 1)* fDetail);
            aLocalSizePixel.SetSize(Size(nReducedWidth + 1, nReducedHeight + 1));
        }

        // Falls die Groesse null ist, groesse auf 1,1 setzen
        if(aLocalSizePixel.GetSize().Width() < 1)
            aLocalSizePixel.SetSize(Size(1 , aLocalSizePixel.GetSize().Height()));
        if(aLocalSizePixel.GetSize().Height() < 1)
            aLocalSizePixel.SetSize(Size(aLocalSizePixel.GetSize().Width(), 1));
    }
}

/*************************************************************************
|*
|* Pixelkoordinaten des Punktes innerhalb der Bitmap holen
|*
\************************************************************************/

Point Base3DDefault::GetPixelCoor(B3dEntity& rEntity)
{
    if(bReducedDetail && fDetail != 0.0)
    {
        Point aRetval = GetOutputDevice()->LogicToPixel(
            Point((long)(rEntity.Point().X()),
            (long)(rEntity.Point().Y()))) - aSizePixel.TopLeft();
        aRetval.X() = (long)((double)aRetval.X() * fDetail);
        aRetval.Y() = (long)((double)aRetval.Y() * fDetail);
        return aRetval;
    }
    else
    {
        return GetOutputDevice()->LogicToPixel(
            Point((long)(rEntity.Point().X()),
            (long)(rEntity.Point().Y()))) - aSizePixel.TopLeft();
    }
}

/*************************************************************************
|*
|* 3DPunkt aus Pixelkoordinaten und Tiefe rekonstruieren
|*
\************************************************************************/

Vector3D Base3DDefault::Get3DCoor(Point& rPnt, double fDepth)
{
    if(bReducedDetail && fDetail != 0.0)
    {
        Point aPnt(rPnt);
        aPnt.X() = (long)((double)aPnt.X() / fDetail);
        aPnt.Y() = (long)((double)aPnt.Y() / fDetail);
        aPnt = GetOutputDevice()->PixelToLogic(aPnt + aSizePixel.TopLeft());
        return Vector3D(aPnt.X(), aPnt.Y(), fDepth);
    }
    else
    {
        Point aPnt = GetOutputDevice()->PixelToLogic(rPnt + aSizePixel.TopLeft());
        return Vector3D(aPnt.X(), aPnt.Y(), fDepth);
    }
}

/*************************************************************************
|*
|* ZBuffer Sichtbarkeitstest
|*
\************************************************************************/

BOOL Base3DDefault::IsVisibleAndScissor(long nX, long nY, UINT32 nDepth)
{
    if(!IsScissorRegionActive() || IsInScissorRegion(nX, nY))
    {
        const BitmapColor& rBmCol = pZBufferWrite->GetPixel(nY, nX);
        Color aColor(rBmCol.GetRed(), rBmCol.GetGreen(), rBmCol.GetBlue());
        return (aColor.GetColor() >= nDepth);
    }
    return FALSE;
}

/*************************************************************************
|*
|* Scissoring Sichtbarkeitstest
|*
\************************************************************************/

BOOL Base3DDefault::IsInScissorRegion(long nX, long nY)
{
    if(nX < aDefaultScissorRectangle.Left())
        return FALSE;
    if(nY < aDefaultScissorRectangle.Top())
        return FALSE;
    if(nX > aDefaultScissorRectangle.Right())
        return FALSE;
    if(nY > aDefaultScissorRectangle.Bottom())
        return FALSE;
    return TRUE;
}

/*************************************************************************
|*
|* Pixel setzen in allen Buffern
|*
\************************************************************************/

void Base3DDefault::WritePixel(long nX, long nY, Color aColor, UINT32 nDepth)
{
    // In Transparenz-Map eintragen
    if(GetTransparentPartsContainedHint())
    {
        if(aColor.GetTransparency())
        {
            BYTE nOldTrans = pTransparenceWrite->GetPixel(nY, nX).GetIndex();

            if(nOldTrans != (BYTE)0xff)
            {
                // Farbe mischen
                BitmapColor aOldCol = pPictureWrite->GetPixel(nY, nX);
                UINT16 nNegTrans = 0x0100 - (UINT16)aColor.GetTransparency();
                aColor.SetRed((BYTE)((((UINT16)aOldCol.GetRed() * (UINT16)aColor.GetTransparency())
                    + (aColor.GetRed() * nNegTrans)) >> 8));
                aColor.SetGreen((BYTE)((((UINT16)aOldCol.GetGreen() * (UINT16)aColor.GetTransparency())
                    + (aColor.GetGreen() * nNegTrans)) >> 8));
                aColor.SetBlue((BYTE)((((UINT16)aOldCol.GetBlue() * (UINT16)aColor.GetTransparency())
                    + (aColor.GetBlue() * nNegTrans)) >> 8));
                pPictureWrite->SetPixel(nY, nX, aColor);

                // Transparenz mischen
                pTransparenceWrite->SetPixel(nY, nX,
                    (BYTE)(((UINT16)(nOldTrans+1) * (UINT16)aColor.GetTransparency()) >> 8));
            }
            else
            {
                // Pixel setzen
                pPictureWrite->SetPixel(nY, nX, aColor);

                // Alpha-Wert setzen
                pTransparenceWrite->SetPixel(nY, nX, aColor.GetTransparency());
            }
        }
        else
        {
            // Pixel setzen
            pPictureWrite->SetPixel(nY, nX, aColor);

            // Alpha-Wert setzen
            pTransparenceWrite->SetPixel(nY, nX, (BYTE)0x00);

            // Z-Buffer setzen
            Color aZBufCol(nDepth);
            pZBufferWrite->SetPixel(nY, nX, aZBufCol);
        }
    }
    else
    {
        // Dieser Punkt in der Mono-Transparenz ist nicht transparent
        BitmapColor aColBlack(BYTE(0));
        pTransparenceWrite->SetPixel(nY, nX, aColBlack);

        // Pixel setzen
        pPictureWrite->SetPixel(nY, nX, aColor);

        // Z-Buffer setzen
        Color aZBufCol(nDepth);
        pZBufferWrite->SetPixel(nY, nX, aZBufCol);
    }
}

/*************************************************************************
|*
|* Zeichenfunktionen; alle Objekte sind geclippt
|* Einzelner Punkt
|*
\************************************************************************/

#define POLYGONOFFSET_VALUE         (120)

void Base3DDefault::Clipped3DPoint(UINT32 nInd)
{
    B3dEntity& rEntity = aBuffers[nInd];

    // Geometrie holen
    rEntity.ToDeviceCoor(GetTransformationSet());
    Point aOutPoint = GetPixelCoor(rEntity);
    UINT32 nDepth = (UINT32)rEntity.Point().Z();

    // PolygonOffset beachten
    if(GetPolygonOffset(Base3DPolygonOffsetPoint))
    {
        if(nDepth >= POLYGONOFFSET_VALUE)
            nDepth -= POLYGONOFFSET_VALUE;
        else
            nDepth = 0;
    }

    // Zeichnen
    if(IsVisibleAndScissor(aOutPoint.X(), aOutPoint.Y(), nDepth))
        WritePixel(aOutPoint.X(), aOutPoint.Y(), rEntity.Color(), nDepth);
}

/*************************************************************************
|*
|* Zeichenfunktionen; alle Objekte sind geclippt
|* Linie
|*
\************************************************************************/

void Base3DDefault::Clipped3DLine(UINT32 nInd1, UINT32 nInd2)
{
    B3dEntity& rEntity1 = aBuffers[nInd1];
    B3dEntity& rEntity2 = aBuffers[nInd2];
    bNormalsUsed = rEntity1.IsNormalUsed() && rEntity2.IsNormalUsed();
    bTextureUsed = IsTextureActive() && rEntity1.IsTexCoorUsed() && rEntity2.IsTexCoorUsed();

    // ColorModel fuer diese Punkte anwenden, falls Normale vorhanden
    // Danach Normale als ungueltig markieren, da nur noch die berechnete
    // Farbe bei Aufteilungen weiter interpoliert wird
    if(bNormalsUsed)
    {
        // Vektoren normalisieren
        rEntity1.Normal().Normalize();
        rEntity2.Normal().Normalize();

        if(GetShadeModel() != Base3DPhong)
        {
            // Farben auswerten
            rEntity1.Color() = SolveColorModel(GetMaterialObject(),
                rEntity1.Normal(), rEntity1.Point().GetVector3D());
            rEntity2.Color() = SolveColorModel(GetMaterialObject(),
                rEntity2.Normal(), rEntity2.Point().GetVector3D());

            // Die Normalen NICHT ungueltig machen, da die Entities
            // eventuell noch fuer weitere Primitive benutzt werden.
            // Aber lokal merken, dass die Normalen bereits ausgewertet sind
            bNormalsUsed = FALSE;
        }
    }

    // Geometrie holen
    rEntity1.ToDeviceCoor(GetTransformationSet());
    rEntity2.ToDeviceCoor(GetTransformationSet());
    Rectangle aPrimitiveArea;

    aOutPointTop = GetPixelCoor(rEntity1);
    aOutPointLeft = GetPixelCoor(rEntity2);

    if(IsScissorRegionActive())
    {
        aPrimitiveArea.Union(Rectangle(aOutPointTop, aOutPointTop));
        aPrimitiveArea.Union(Rectangle(aOutPointLeft, aOutPointLeft));
    }

    if(!IsScissorRegionActive()
        || (IsScissorRegionActive()
        && !aDefaultScissorRectangle.GetIntersection(aPrimitiveArea).IsEmpty()))
    {
        if(bTextureUsed)
        {
            fTexWidth = (double)GetActiveTexture()->GetBitmapSize().Width();
            fTexHeight = (double)GetActiveTexture()->GetBitmapSize().Height();
        }

        // Punkt, Farbe und Z-Wert interpolieren und die Linie gererieren
        long nDx = aOutPointLeft.X() - aOutPointTop.X();
        long nDy = aOutPointLeft.Y() - aOutPointTop.Y();
        long nCount;

        // Werte fuer Schleife vorbereiten
        if(abs(nDx) > abs(nDy))
            // ueber X gehen
            nCount = abs(nDx);
        else
            // ueber Y gehen
            nCount = abs(nDy);

        if(nCount)
        {
            // Interpolatoren vorbereiten
            aIntXPosLeft.Load(aOutPointTop.X(), aOutPointLeft.X(), nCount);
            aIntXPosRight.Load(aOutPointTop.Y(), aOutPointLeft.Y(), nCount);
            UINT32 nDepth;

            // PolygonOffset beachten
            if(GetPolygonOffset())
            {
                double fDepthLeft = rEntity1.Point().Z();
                double fDepthRight = rEntity2.Point().Z();

                if(fDepthLeft >= double(POLYGONOFFSET_VALUE))
                    fDepthLeft -= double(POLYGONOFFSET_VALUE);
                else
                    fDepthLeft = 0.0;

                if(fDepthRight >= double(POLYGONOFFSET_VALUE))
                    fDepthRight -= double(POLYGONOFFSET_VALUE);
                else
                    fDepthRight = 0.0;

                aIntDepthLine.Load(fDepthLeft, fDepthRight, nCount);
            }
            else
            {
                aIntDepthLine.Load(rEntity1.Point().Z(), rEntity2.Point().Z(), nCount);
            }

            // Texturkoordinateninterpolation?
            if(bTextureUsed)
            {
                aIntTexSLine.Load(
                    rEntity1.TexCoor().X() * fTexWidth,
                    rEntity2.TexCoor().X() * fTexWidth,
                    nCount);
                aIntTexTLine.Load(
                    rEntity1.TexCoor().Y() * fTexHeight,
                    rEntity2.TexCoor().Y() * fTexHeight,
                    nCount);
            }

            if(bNormalsUsed && GetShadeModel() == Base3DPhong)
            {
                // Normalen und Geometrie interpolieren
                if(GetTransformationSet())
                {
                    Vector3D aInvTrans = GetTransformationSet()->GetTranslate();
                    Vector3D aInvScale = GetTransformationSet()->GetScale();

                    // Tiefe und Normale vorbereiten
                    aIntVectorLine.Load(rEntity1.Normal(), rEntity2.Normal(), nCount);

                    // Linie zeichnen
                    if(bTextureUsed)
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                            {
                                Point aTmpPoint(nDx, nDy);
                                Vector3D aPoint = Get3DCoor(aTmpPoint, nDepth);
                                aPoint -= aInvTrans;
                                aPoint /= aInvScale;
                                Vector3D aNormal;
                                aIntVectorLine.GetVector3DValue(aNormal);
                                aNormal.Normalize();
                                Color aCol = SolveColorModel(GetMaterialObject(), aNormal, aPoint);
                                GetActiveTexture()->ModifyColor(aCol,
                                    aIntTexSLine.GetDoubleValue(),
                                    aIntTexTLine.GetDoubleValue());
                                WritePixel(nDx, nDy, aCol, nDepth);
                            }

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntVectorLine.Increment();
                                aIntTexSLine.Increment();
                                aIntTexTLine.Increment();
                            }
                        }
                    }
                    else
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                            {
                                Point aTmpPoint(nDx, nDy);
                                Vector3D aPoint = Get3DCoor(aTmpPoint, nDepth);
                                aPoint -= aInvTrans;
                                aPoint /= aInvScale;
                                Vector3D aNormal;
                                aIntVectorLine.GetVector3DValue(aNormal);
                                aNormal.Normalize();
                                Color aCol = SolveColorModel(GetMaterialObject(), aNormal, aPoint);
                                WritePixel(nDx, nDy, aCol, nDepth);
                            }

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntVectorLine.Increment();
                            }
                        }
                    }
                }
            }
            else
            {
                if(rEntity1.Color() != rEntity2.Color())
                {
                    // Farbe und Geometrie interpolieren
                    // Tiefe und Farbe vorbereiten
                    aIntColorLine.Load(rEntity1.Color(), rEntity2.Color(), nCount);

                    // Linie zeichnen
                    if(bTextureUsed)
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                            {
                                Color aCol = aIntColorLine.GetColorValue();
                                GetActiveTexture()->ModifyColor(aCol,
                                    aIntTexSLine.GetDoubleValue(),
                                    aIntTexTLine.GetDoubleValue());
                                WritePixel(nDx, nDy, aCol, nDepth);
                            }

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntColorLine.Increment();
                                aIntTexSLine.Increment();
                                aIntTexTLine.Increment();
                            }
                        }
                    }
                    else
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                                WritePixel(nDx, nDy, aIntColorLine.GetColorValue(), nDepth);

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntColorLine.Increment();
                            }
                        }
                    }
                }
                else
                {
                    // Nur die Geometrie interpolieren
                    // Linie zeichnen
                    if(bTextureUsed)
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                            {
                                Color aCol = rEntity1.Color();
                                GetActiveTexture()->ModifyColor(aCol,
                                    aIntTexSLine.GetDoubleValue(),
                                    aIntTexTLine.GetDoubleValue());
                                WritePixel(nDx, nDy, aCol, nDepth);
                            }

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntTexSLine.Increment();
                                aIntTexTLine.Increment();
                            }
                        }
                    }
                    else
                    {
                        while(nCount--)
                        {
                            // weiterer Punkt
                            nDx = aIntXPosLeft.GetLongValue();
                            nDy = aIntXPosRight.GetLongValue();
                            nDepth = aIntDepthLine.GetUINT32Value();

                            if(IsVisibleAndScissor(nDx, nDy, nDepth))
                                WritePixel(nDx, nDy, rEntity1.Color(), nDepth);

                            if(nCount)
                            {
                                // Weiterschalten
                                aIntXPosLeft.Increment();
                                aIntXPosRight.Increment();
                                aIntDepthLine.Increment();
                                aIntTexSLine.Increment();
                                aIntTexTLine.Increment();
                            }
                        }
                    }
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Zeichenfunktionen; alle Objekte sind geclippt
|* Polygon
|*
\************************************************************************/

void Base3DDefault::Clipped3DTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3)
{
    B3dEntity& rEntity1 = aBuffers[nInd1];
    B3dEntity& rEntity2 = aBuffers[nInd2];
    B3dEntity& rEntity3 = aBuffers[nInd3];
    bNormalsUsed = rEntity1.IsNormalUsed() && rEntity2.IsNormalUsed() && rEntity3.IsNormalUsed();
    bTextureUsed = IsTextureActive() && rEntity1.IsTexCoorUsed() && rEntity2.IsTexCoorUsed() && rEntity3.IsTexCoorUsed();
    Base3DMaterialMode eMode = Base3DMaterialFront;

    // ColorModel fuer diese Punkte anwenden, falls Normale vorhanden
    // Danach Normale als ungueltig markieren, da nur noch die berechnete
    // Farbe bei Aufteilungen weiter interpoliert wird
    if(bNormalsUsed)
    {
        // Vektoren normalisieren
        rEntity1.Normal().Normalize();
        rEntity2.Normal().Normalize();
        rEntity3.Normal().Normalize();

        if(GetShadeModel() != Base3DPhong)
        {
            // Normale berechnen, Farben auswerten
            if(rEntity1.PlaneNormal().Z() < 0.0 && (GetLightGroup() && GetLightGroup()->GetModelTwoSide()))
                eMode = Base3DMaterialBack;

            rEntity1.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity1.Normal(), rEntity1.Point().GetVector3D());
            rEntity2.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity2.Normal(), rEntity2.Point().GetVector3D());
            rEntity3.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity3.Normal(), rEntity3.Point().GetVector3D());

            // Die Normalen NICHT ungueltig machen, da die Entities
            // eventuell noch fuer weitere Primitive benutzt werden.
            // Aber lokal merken, dass die Normalen bereits ausgewertet sind
            bNormalsUsed = FALSE;
        }
    }

    // Geometrie holen
    rEntity1.ToDeviceCoor(GetTransformationSet());
    rEntity2.ToDeviceCoor(GetTransformationSet());
    rEntity3.ToDeviceCoor(GetTransformationSet());

    // Punkte ordnen. Oberster nach pEntTop
    if(rEntity1.Point().Y() < rEntity2.Point().Y()
        && rEntity1.Point().Y() < rEntity3.Point().Y())
    {
        // rEntity1 ist der oberste
        pEntTop = &rEntity1;

        // Left, Right erst mal zuweisen
        pEntRight = &rEntity3;
        pEntLeft = &rEntity2;
    }
    else
    {
        if(rEntity2.Point().Y() < rEntity3.Point().Y())
        {
            // rEntity2 ist der oberste
            pEntTop = &rEntity2;

            // Left, Right erst mal zuweisen
            pEntRight = &rEntity1;
            pEntLeft = &rEntity3;
        }
        else
        {
            // rEntity3 ist der oberste
            pEntTop = &rEntity3;

            // Left, Right erst mal zuweisen
            pEntRight = &rEntity2;
            pEntLeft = &rEntity1;
        }
    }

    // Werte holen
    Rectangle aPrimitiveArea;

    aOutPointTop = GetPixelCoor(*pEntTop);
    aOutPointLeft = GetPixelCoor(*pEntLeft);
    aOutPointRight = GetPixelCoor(*pEntRight);

    if(IsScissorRegionActive())
    {
        aPrimitiveArea.Union(Rectangle(aOutPointTop, aOutPointTop));
        aPrimitiveArea.Union(Rectangle(aOutPointLeft, aOutPointLeft));
        aPrimitiveArea.Union(Rectangle(aOutPointRight, aOutPointRight));
    }

    if(!IsScissorRegionActive()
        || (IsScissorRegionActive()
        && !aDefaultScissorRectangle.GetIntersection(aPrimitiveArea).IsEmpty()))
    {
        if(bTextureUsed)
        {
            fTexWidth = (double)GetActiveTexture()->GetBitmapSize().Width();
            fTexHeight = (double)GetActiveTexture()->GetBitmapSize().Height();
        }

        // Links und rechts ordnen
        long nDeltaYLeft = aOutPointLeft.Y() - aOutPointTop.Y();
        long nDeltaYRight = aOutPointRight.Y() - aOutPointTop.Y();
        long nYLine;

        if((aOutPointLeft.X() - aOutPointTop.X()) * nDeltaYRight
            - nDeltaYLeft * (aOutPointRight.X() - aOutPointTop.X()) > 0)
        {
            // Links und rechts vertauschen
            // Punkte
            nYLine = aOutPointLeft.X();
            aOutPointLeft.X() = aOutPointRight.X();
            aOutPointRight.X() = nYLine;
            nYLine = aOutPointLeft.Y();
            aOutPointLeft.Y() = aOutPointRight.Y();
            aOutPointRight.Y() = nYLine;

            // Deltas
            nYLine = nDeltaYLeft; nDeltaYLeft = nDeltaYRight; nDeltaYRight = nYLine;

            // Zeiger auf Entities
            B3dEntity* pTmp = pEntLeft; pEntLeft = pEntRight; pEntRight = pTmp;
        }

        // YStart, Links und rechts laden
        nYLine = aOutPointTop.Y();
        aIntXPosLeft.Load(aOutPointTop.X(), aOutPointLeft.X(), nDeltaYLeft);
        aIntDepthLeft.Load(pEntTop->Point().Z(), pEntLeft->Point().Z(), nDeltaYLeft);
        aIntXPosRight.Load(aOutPointTop.X(), aOutPointRight.X(), nDeltaYRight);
        aIntDepthRight.Load(pEntTop->Point().Z(), pEntRight->Point().Z(), nDeltaYRight);
        if(bTextureUsed)
        {
            aIntTexSLeft.Load(
                pEntTop->TexCoor().X() * fTexWidth,
                pEntLeft->TexCoor().X() * fTexWidth, nDeltaYLeft);
            aIntTexTLeft.Load(
                pEntTop->TexCoor().Y() * fTexHeight,
                pEntLeft->TexCoor().Y() * fTexHeight, nDeltaYLeft);
            aIntTexSRight.Load(
                pEntTop->TexCoor().X() * fTexWidth,
                pEntRight->TexCoor().X() * fTexWidth, nDeltaYRight);
            aIntTexTRight.Load(
                pEntTop->TexCoor().Y() * fTexHeight,
                pEntRight->TexCoor().Y() * fTexHeight, nDeltaYRight);
        }

        if(bNormalsUsed && GetShadeModel() == Base3DPhong)
        {
            // Normalen und Geometrie interpolieren
            aIntVectorLeft.Load(pEntTop->Normal(), pEntLeft->Normal(), nDeltaYLeft);
            aIntVectorRight.Load(pEntTop->Normal(), pEntRight->Normal(), nDeltaYRight);
            B3dMaterial& rMat = GetMaterialObject(eMode);

            if(bTextureUsed)
            {
                // Schleife
                while(nDeltaYLeft || nDeltaYRight)
                {
                    // Zeile ausgeben
                    DrawLinePhongTexture(nYLine, rMat);

                    // naechste Zeile vorbereiten rechts
                    if(!nDeltaYRight && nDeltaYLeft)
                    {
                        // Rechts ist zuende, lade neu mit Rest nach links
                        nDeltaYRight = nDeltaYLeft;
                        LoadRightTexture(nDeltaYRight);
                        aIntVectorRight.Load(pEntRight->Normal(), pEntLeft->Normal(), nDeltaYRight);
                    }

                    // naechste Zeile vorbereiten links
                    if(!nDeltaYLeft && nDeltaYRight)
                    {
                        // Links ist zuende, lade neu mit Rest nach rechts
                        nDeltaYLeft = nDeltaYRight;
                        LoadLeftTexture(nDeltaYLeft);
                        aIntVectorLeft.Load(pEntLeft->Normal(), pEntRight->Normal(), nDeltaYLeft);
                    }

                    // naechste Zeile rechts
                    if(nDeltaYRight || nDeltaYLeft)
                    {
                        nDeltaYRight--;
                        NextStepRightTexture();
                        aIntVectorRight.Increment();

                        nDeltaYLeft--;
                        NextStepLeftTexture();
                        aIntVectorLeft.Increment();

                        nYLine++;
                    }
                }
            }
            else
            {
                // Schleife
                while(nDeltaYLeft || nDeltaYRight)
                {
                    // Zeile ausgeben
                    DrawLinePhong(nYLine, rMat);

                    // naechste Zeile vorbereiten rechts
                    if(!nDeltaYRight && nDeltaYLeft)
                    {
                        // Rechts ist zuende, lade neu mit Rest nach links
                        nDeltaYRight = nDeltaYLeft;
                        LoadRight(nDeltaYRight);
                        aIntVectorRight.Load(pEntRight->Normal(), pEntLeft->Normal(), nDeltaYRight);
                    }

                    // naechste Zeile vorbereiten links
                    if(!nDeltaYLeft && nDeltaYRight)
                    {
                        // Links ist zuende, lade neu mit Rest nach rechts
                        nDeltaYLeft = nDeltaYRight;
                        LoadLeft(nDeltaYLeft);
                        aIntVectorLeft.Load(pEntLeft->Normal(), pEntRight->Normal(), nDeltaYLeft);
                    }

                    // naechste Zeile rechts
                    if(nDeltaYRight || nDeltaYLeft)
                    {
                        nDeltaYRight--;
                        NextStepRight();
                        aIntVectorRight.Increment();

                        nDeltaYLeft--;
                        NextStepLeft();
                        aIntVectorLeft.Increment();

                        nYLine++;
                    }
                }
            }
        }
        else
        {
            if(!(rEntity1.Color() == rEntity2.Color() && rEntity1.Color() == rEntity3.Color()))
            {
                // Farbe und Geometrie interpolieren
                aIntColorLeft.Load(pEntTop->Color(), pEntLeft->Color(), nDeltaYLeft);
                aIntColorRight.Load(pEntTop->Color(), pEntRight->Color(), nDeltaYRight);

                if(bTextureUsed)
                {
                    // Schleife
                    while(nDeltaYLeft || nDeltaYRight)
                    {
                        // Zeile ausgeben
                        DrawLineColorTexture(nYLine);

                        // naechste Zeile vorbereiten rechts
                        if(!nDeltaYRight && nDeltaYLeft)
                        {
                            // Rechts ist zuende, lade neu mit Rest nach links
                            nDeltaYRight = nDeltaYLeft;
                            LoadRightTexture(nDeltaYRight);
                            aIntColorRight.Load(pEntRight->Color(), pEntLeft->Color(), nDeltaYRight);
                        }

                        // naechste Zeile vorbereiten links
                        if(!nDeltaYLeft && nDeltaYRight)
                        {
                            // Links ist zuende, lade neu mit Rest nach rechts
                            nDeltaYLeft = nDeltaYRight;
                            LoadLeftTexture(nDeltaYLeft);
                            aIntColorLeft.Load(pEntLeft->Color(), pEntRight->Color(), nDeltaYLeft);
                        }

                        // naechste Zeile rechts
                        if(nDeltaYRight || nDeltaYLeft)
                        {
                            nDeltaYRight--;
                            NextStepRightTexture();
                            aIntColorRight.Increment();

                            nDeltaYLeft--;
                            NextStepLeftTexture();
                            aIntColorLeft.Increment();

                            nYLine++;
                        }
                    }
                }
                else
                {
                    // Schleife
                    while(nDeltaYLeft || nDeltaYRight)
                    {
                        // Zeile ausgeben
                        DrawLineColor(nYLine);

                        // naechste Zeile vorbereiten rechts
                        if(!nDeltaYRight && nDeltaYLeft)
                        {
                            // Rechts ist zuende, lade neu mit Rest nach links
                            nDeltaYRight = nDeltaYLeft;
                            LoadRight(nDeltaYRight);
                            aIntColorRight.Load(pEntRight->Color(), pEntLeft->Color(), nDeltaYRight);
                        }

                        // naechste Zeile vorbereiten links
                        if(!nDeltaYLeft && nDeltaYRight)
                        {
                            // Links ist zuende, lade neu mit Rest nach rechts
                            nDeltaYLeft = nDeltaYRight;
                            LoadLeft(nDeltaYLeft);
                            aIntColorLeft.Load(pEntLeft->Color(), pEntRight->Color(), nDeltaYLeft);
                        }

                        // naechste Zeile rechts
                        if(nDeltaYRight || nDeltaYLeft)
                        {
                            nDeltaYRight--;
                            NextStepRight();
                            aIntColorRight.Increment();

                            nDeltaYLeft--;
                            NextStepLeft();
                            aIntColorLeft.Increment();

                            nYLine++;
                        }
                    }
                }
            }
            else
            {
                // Nur die Geometrie interpolieren
                if(bTextureUsed)
                {
                    // Schleife
                    while(nDeltaYLeft || nDeltaYRight)
                    {
                        // Zeile ausgeben
                        DrawLineTexture(nYLine, pEntTop->Color());

                        // naechste Zeile vorbereiten rechts
                        if(!nDeltaYRight && nDeltaYLeft)
                        {
                            // Rechts ist zuende, lade neu mit Rest nach links
                            nDeltaYRight = nDeltaYLeft;
                            LoadRightTexture(nDeltaYRight);
                        }

                        // naechste Zeile vorbereiten links
                        if(!nDeltaYLeft && nDeltaYRight)
                        {
                            // Links ist zuende, lade neu mit Rest nach rechts
                            nDeltaYLeft = nDeltaYRight;
                            LoadLeftTexture(nDeltaYLeft);
                        }

                        // naechste Zeile rechts
                        if(nDeltaYRight || nDeltaYLeft)
                        {
                            nDeltaYRight--;
                            NextStepRightTexture();

                            nDeltaYLeft--;
                            NextStepLeftTexture();

                            nYLine++;
                        }
                    }
                }
                else
                {
                    // Schleife
                    while(nDeltaYLeft || nDeltaYRight)
                    {
                        // Zeile ausgeben
                        DrawLine(nYLine, pEntTop->Color());

                        // naechste Zeile vorbereiten rechts
                        if(!nDeltaYRight && nDeltaYLeft)
                        {
                            // Rechts ist zuende, lade neu mit Rest nach links
                            nDeltaYRight = nDeltaYLeft;
                            LoadRight(nDeltaYRight);
                        }

                        // naechste Zeile vorbereiten links
                        if(!nDeltaYLeft && nDeltaYRight)
                        {
                            // Links ist zuende, lade neu mit Rest nach rechts
                            nDeltaYLeft = nDeltaYRight;
                            LoadLeft(nDeltaYLeft);
                        }

                        // naechste Zeile rechts
                        if(nDeltaYRight || nDeltaYLeft)
                        {
                            nDeltaYRight--;
                            NextStepRight();

                            nDeltaYLeft--;
                            NextStepLeft();

                            nYLine++;
                        }
                    }
                }
            }
        }
    }
}

void Base3DDefault::DrawLinePhongTexture(long nYPos, B3dMaterial& rMat)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        Vector3D aVectorLeft;
        aIntVectorLeft.GetVector3DValue(aVectorLeft);
        Vector3D aVectorRight;
        aIntVectorRight.GetVector3DValue(aVectorRight);
        aIntVectorLine.Load(aVectorLeft, aVectorRight, nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        // Texturkoordinateninterpolation?
        if(bTextureUsed)
        {
            aIntTexSLine.Load(aIntTexSLeft.GetDoubleValue(), aIntTexSRight.GetDoubleValue(), nXLineDelta);
            aIntTexTLine.Load(aIntTexTLeft.GetDoubleValue(), aIntTexTRight.GetDoubleValue(), nXLineDelta);
        }

        if(GetTransformationSet())
        {
            Vector3D aInvTrans = GetTransformationSet()->GetTranslate();
            Vector3D aInvScale = GetTransformationSet()->GetScale();

            while(nXLineDelta--)
            {
                // Werte vorbereiten
                UINT32 nDepth = aIntDepthLine.GetUINT32Value();

                // Punkt ausgeben
                if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                {
                    Point aTmpPoint(nXLineStart, nYPos);
                    Vector3D aPoint = Get3DCoor(aTmpPoint, nDepth);
                    aPoint -= aInvTrans;
                    aPoint /= aInvScale;
                    Vector3D aNormal;
                    aIntVectorLine.GetVector3DValue(aNormal);
                    aNormal.Normalize();
                    Color aCol = SolveColorModel(rMat, aNormal, aPoint);

                    // Texturkoordinateninterpolation?
                    if(bTextureUsed)
                    {
                        GetActiveTexture()->ModifyColor(aCol,
                            aIntTexSLine.GetDoubleValue(),
                            aIntTexTLine.GetDoubleValue());
                    }
                    WritePixel(nXLineStart, nYPos, aCol, nDepth);
                }

                if(nXLineDelta)
                {
                    // naechste Spalte
                    nXLineStart++;

                    // naechste Tiefe und Farbe
                    aIntDepthLine.Increment();
                    aIntVectorLine.Increment();

                    // Texturkoordinateninterpolation?
                    if(bTextureUsed)
                    {
                        aIntTexSLine.Increment();
                        aIntTexTLine.Increment();
                    }
                }
            }
        }
    }
}

void Base3DDefault::DrawLinePhong(long nYPos, B3dMaterial& rMat)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        Vector3D aVectorLeft;
        aIntVectorLeft.GetVector3DValue(aVectorLeft);
        Vector3D aVectorRight;
        aIntVectorRight.GetVector3DValue(aVectorRight);
        aIntVectorLine.Load(aVectorLeft, aVectorRight, nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        if(GetTransformationSet())
        {
            Vector3D aInvTrans = GetTransformationSet()->GetTranslate();
            Vector3D aInvScale = GetTransformationSet()->GetScale();
            while(nXLineDelta--)
            {
                // Werte vorbereiten
                UINT32 nDepth = aIntDepthLine.GetUINT32Value();

                // Punkt ausgeben
                if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                {
                    Point aTmpPoint(nXLineStart, nYPos);
                    Vector3D aPoint = Get3DCoor(aTmpPoint, nDepth);
                    aPoint -= aInvTrans;
                    aPoint /= aInvScale;
                    Vector3D aNormal;
                    aIntVectorLine.GetVector3DValue(aNormal);
                    aNormal.Normalize();
                    Color aCol = SolveColorModel(rMat, aNormal, aPoint);
                    WritePixel(nXLineStart, nYPos, aCol, nDepth);
                }

                if(nXLineDelta)
                {
                    // naechste Spalte
                    nXLineStart++;

                    // naechste Tiefe und Farbe
                    aIntDepthLine.Increment();
                    aIntVectorLine.Increment();
                }
            }
        }
    }
}

void Base3DDefault::DrawLineColorTexture(long nYPos)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        aIntColorLine.Load(aIntColorLeft.GetColorValue(), aIntColorRight.GetColorValue(), nXLineDelta);
        aIntTexSLine.Load(aIntTexSLeft.GetDoubleValue(), aIntTexSRight.GetDoubleValue(), nXLineDelta);
        aIntTexTLine.Load(aIntTexTLeft.GetDoubleValue(), aIntTexTRight.GetDoubleValue(), nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            UINT32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
            {
                Color aCol = aIntColorLine.GetColorValue();
                GetActiveTexture()->ModifyColor(aCol,
                    aIntTexSLine.GetDoubleValue(),
                    aIntTexTLine.GetDoubleValue());
                WritePixel(nXLineStart, nYPos, aCol, nDepth);
            }

            if(nXLineDelta)
            {
                // naechste Spalte
                nXLineStart++;

                // naechste Tiefe und Farbe
                aIntDepthLine.Increment();
                aIntColorLine.Increment();
                aIntTexSLine.Increment();
                aIntTexTLine.Increment();
            }
        }
    }
}

void Base3DDefault::DrawLineColor(long nYPos)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        aIntColorLine.Load(aIntColorLeft.GetColorValue(), aIntColorRight.GetColorValue(), nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            UINT32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                WritePixel(nXLineStart, nYPos, aIntColorLine.GetColorValue(), nDepth);

            if(nXLineDelta)
            {
                // naechste Spalte
                nXLineStart++;

                // naechste Tiefe und Farbe
                aIntDepthLine.Increment();
                aIntColorLine.Increment();
            }
        }
    }
}

void Base3DDefault::DrawLineTexture(long nYPos, Color& rCol)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        aIntTexSLine.Load(aIntTexSLeft.GetDoubleValue(), aIntTexSRight.GetDoubleValue(), nXLineDelta);
        aIntTexTLine.Load(aIntTexTLeft.GetDoubleValue(), aIntTexTRight.GetDoubleValue(), nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            UINT32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
            {
                // Texturkoordinateninterpolation?
                Color aCol = rCol;
                GetActiveTexture()->ModifyColor(aCol,
                    aIntTexSLine.GetDoubleValue(),
                    aIntTexTLine.GetDoubleValue());
                WritePixel(nXLineStart, nYPos, aCol, nDepth);
            }

            if(nXLineDelta)
            {
                // naechste Spalte
                nXLineStart++;

                // naechste Tiefe und Farbe
                aIntDepthLine.Increment();
                aIntTexSLine.Increment();
                aIntTexTLine.Increment();
            }
        }
    }
}

void Base3DDefault::DrawLine(long nYPos, Color& rCol)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    long nXLineStart = aIntXPosLeft.GetLongValue();
    long nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            UINT32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                WritePixel(nXLineStart, nYPos, rCol, nDepth);

            if(nXLineDelta)
            {
                // naechste Spalte
                nXLineStart++;

                // naechste Tiefe und Farbe
                aIntDepthLine.Increment();
            }
        }
    }
}

void Base3DDefault::LoadLeftTexture(long nSize)
{
    aIntXPosLeft.Load(aOutPointLeft.X(), aOutPointRight.X(), nSize);
    aIntDepthLeft.Load(pEntLeft->Point().Z(), pEntRight->Point().Z(), nSize);
    aIntTexSLeft.Load(
        pEntLeft->TexCoor().X() * fTexWidth,
        pEntRight->TexCoor().X() * fTexWidth, nSize);
    aIntTexTLeft.Load(
        pEntLeft->TexCoor().Y() * fTexHeight,
        pEntRight->TexCoor().Y() * fTexHeight, nSize);
}

void Base3DDefault::LoadLeft(long nSize)
{
    aIntXPosLeft.Load(aOutPointLeft.X(), aOutPointRight.X(), nSize);
    aIntDepthLeft.Load(pEntLeft->Point().Z(), pEntRight->Point().Z(), nSize);
}

void Base3DDefault::LoadRightTexture(long nSize)
{
    aIntXPosRight.Load(aOutPointRight.X(), aOutPointLeft.X(), nSize);
    aIntDepthRight.Load(pEntRight->Point().Z(), pEntLeft->Point().Z(), nSize);
    aIntTexSRight.Load(
        pEntRight->TexCoor().X() * fTexWidth,
        pEntLeft->TexCoor().X() * fTexWidth, nSize);
    aIntTexTRight.Load(
        pEntRight->TexCoor().Y() * fTexHeight,
        pEntLeft->TexCoor().Y() * fTexHeight, nSize);
}

void Base3DDefault::LoadRight(long nSize)
{
    aIntXPosRight.Load(aOutPointRight.X(), aOutPointLeft.X(), nSize);
    aIntDepthRight.Load(pEntRight->Point().Z(), pEntLeft->Point().Z(), nSize);
}

void Base3DDefault::NextStepRightTexture()
{
    aIntXPosRight.Increment();
    aIntDepthRight.Increment();
    aIntTexSRight.Increment();
    aIntTexTRight.Increment();
}

void Base3DDefault::NextStepRight()
{
    aIntXPosRight.Increment();
    aIntDepthRight.Increment();
}

void Base3DDefault::NextStepLeftTexture()
{
    aIntXPosLeft.Increment();
    aIntDepthLeft.Increment();
    aIntTexSLeft.Increment();
    aIntTexTLeft.Increment();
}

void Base3DDefault::NextStepLeft()
{
    aIntXPosLeft.Increment();
    aIntDepthLeft.Increment();
}

