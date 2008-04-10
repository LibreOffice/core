/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3ddeflt.cxx,v $
 * $Revision: 1.11 $
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
#include "b3ddeflt.hxx"
#include "b3dtrans.hxx"
#include <vcl/outdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include "b3dtex.hxx"

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
    fDetailBackup( -1.0 ),
    nMaxPixels(500000),
    bReducedDetail(FALSE),
    bDetailBackedup(FALSE),
    mbPTCorrection(sal_True)

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

void Base3DDefault::SetDisplayQuality(sal_uInt8 nNew)
{
    // Entsprechende PixelGrenze setzen
    SetMaxPixels(((sal_Int32)nNew * 3500) + 3500);

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
    sal_Bool bSizeHasChanged = (aLocalSizePixel.GetSize() != aPicture.GetSizePixel());

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
            sal_Int32 nReducedWidth = (sal_Int32)((double)(aDefaultScissorRectangle.GetWidth() - 1) * fDetail);
            sal_Int32 nReducedHeight = (sal_Int32)((double)(aDefaultScissorRectangle.GetHeight() - 1)* fDetail);
            aDefaultScissorRectangle.SetSize(Size(nReducedWidth + 1, nReducedHeight + 1));
        }
    }

    // #i71618#
    // derive maPolygonOffset from mfPolygonOffset and use instead of old
    // POLYGONOFFSET_VALUE which was much to low with default of 120
    maPolygonOffset = (sal_uInt32)(getPolygonOffset() * ZBUFFER_DEPTH_RANGE);

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
        sal_Bool bWasEnabled = GetOutputDevice()->IsMapModeEnabled();
        GetOutputDevice()->EnableMapMode(FALSE);

#ifdef DBG_UTIL     // draw for testing
        static sal_Bool bDoDrawBitmapForTesting(FALSE);
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

        double fQuadSize = aLocalSizePixel.GetWidth();  // sj: #i40320# solved overrun
        fQuadSize *= aLocalSizePixel.GetHeight();

        if( fQuadSize > GetMaxPixels() )
        {
            // Groesse reduzieren
            double fFactor = sqrt((double)GetMaxPixels() / fQuadSize);

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
            sal_Int32 nReducedWidth = (sal_Int32)((double)(aLocalSizePixel.GetWidth() - 1) * fDetail);
            sal_Int32 nReducedHeight = (sal_Int32)((double)(aLocalSizePixel.GetHeight() - 1)* fDetail);
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
            Point((sal_Int32)(rEntity.Point().getX()),
            (sal_Int32)(rEntity.Point().getY()))) - aSizePixel.TopLeft();
        aRetval.X() = (sal_Int32)((double)aRetval.X() * fDetail);
        aRetval.Y() = (sal_Int32)((double)aRetval.Y() * fDetail);
        return aRetval;
    }
    else
    {
        return GetOutputDevice()->LogicToPixel(
            Point((sal_Int32)(rEntity.Point().getX()),
            (sal_Int32)(rEntity.Point().getY()))) - aSizePixel.TopLeft();
    }
}

/*************************************************************************
|*
|* 3DPunkt aus Pixelkoordinaten und Tiefe rekonstruieren
|*
\************************************************************************/

basegfx::B3DPoint Base3DDefault::Get3DCoor(Point& rPnt, double fDepth)
{
    if(bReducedDetail && fDetail != 0.0)
    {
        Point aPnt(rPnt);
        aPnt.X() = (sal_Int32)((double)aPnt.X() / fDetail);
        aPnt.Y() = (sal_Int32)((double)aPnt.Y() / fDetail);
        aPnt = GetOutputDevice()->PixelToLogic(aPnt + aSizePixel.TopLeft());
        return basegfx::B3DPoint(aPnt.X(), aPnt.Y(), fDepth);
    }
    else
    {
        Point aPnt = GetOutputDevice()->PixelToLogic(rPnt + aSizePixel.TopLeft());
        return basegfx::B3DPoint(aPnt.X(), aPnt.Y(), fDepth);
    }
}

/*************************************************************************
|*
|* ZBuffer Sichtbarkeitstest
|*
\************************************************************************/

sal_Bool Base3DDefault::IsVisibleAndScissor(sal_Int32 nX, sal_Int32 nY, sal_uInt32 nDepth)
{
    // #112303#
    // Do not allow pixels smaller then the bitmap
    if(nX < 0L || nY < 0L)
        return FALSE;

    // #112303#
    // Do not allow pixels bigger then the bitmap
    if(nX > aLocalSizePixel.GetWidth() || nY >aLocalSizePixel.GetHeight())
        return FALSE;

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

sal_Bool Base3DDefault::IsInScissorRegion(sal_Int32 nX, sal_Int32 nY)
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

void Base3DDefault::WritePixel(sal_Int32 nX, sal_Int32 nY, Color aColor, sal_uInt32 nDepth)
{
    // #112303#
    // WritePixel requires the pixel coordinates to be safely on the buffer
    // bitmaps where the paint will take place. Thus, this asserts will
    // ensure that.
    DBG_ASSERT(nX >= 0L, "Base3DDefault::WritePixel: X-Coor negative (!)");
    DBG_ASSERT(nY >= 0L, "Base3DDefault::WritePixel: Y-Coor negative (!)");
    DBG_ASSERT(nX <= aPicture.GetSizePixel().Width(), "Base3DDefault::WritePixel: X-Coor too big (!)");
    DBG_ASSERT(nY <= aPicture.GetSizePixel().Height(), "Base3DDefault::WritePixel: Y-Coor too big (!)");

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

void Base3DDefault::Clipped3DPoint(sal_uInt32 nInd)
{
    B3dEntity& rEntity = aBuffers[nInd];

    // Geometrie holen
    rEntity.ToDeviceCoor(GetTransformationSet());
    Point aOutPoint = GetPixelCoor(rEntity);
    sal_uInt32 nDepth = (sal_uInt32)rEntity.Point().getZ();

    // PolygonOffset beachten
    if(GetPolygonOffset(Base3DPolygonOffsetPoint))
    {
        if(nDepth >= maPolygonOffset)
            nDepth -= maPolygonOffset;
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

void Base3DDefault::Clipped3DLine(sal_uInt32 nInd1, sal_uInt32 nInd2)
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
        rEntity1.Normal().normalize();
        rEntity2.Normal().normalize();

        if(GetShadeModel() != Base3DPhong)
        {
            // Farben auswerten
            rEntity1.Color() = SolveColorModel(GetMaterialObject(),
                rEntity1.Normal(), rEntity1.Point());
            rEntity2.Color() = SolveColorModel(GetMaterialObject(),
                rEntity2.Normal(), rEntity2.Point());

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
        sal_Int32 nDx = aOutPointLeft.X() - aOutPointTop.X();
        sal_Int32 nDy = aOutPointLeft.Y() - aOutPointTop.Y();
        sal_Int32 nCount;

        // Werte fuer Schleife vorbereiten
        if(labs(nDx) > labs(nDy))
            // ueber X gehen
            nCount = labs(nDx);
        else
            // ueber Y gehen
            nCount = labs(nDy);

        if(nCount)
        {
            // Interpolatoren vorbereiten
            aIntXPosLeft.Load(aOutPointTop.X(), aOutPointLeft.X(), nCount);
            aIntXPosRight.Load(aOutPointTop.Y(), aOutPointLeft.Y(), nCount);
            sal_uInt32 nDepth;

            // PolygonOffset beachten
            if(GetPolygonOffset())
            {
                double fDepthLeft = rEntity1.Point().getZ();
                double fDepthRight = rEntity2.Point().getZ();

                if(fDepthLeft >= double(maPolygonOffset))
                    fDepthLeft -= double(maPolygonOffset);
                else
                    fDepthLeft = 0.0;

                if(fDepthRight >= double(maPolygonOffset))
                    fDepthRight -= double(maPolygonOffset);
                else
                    fDepthRight = 0.0;

                aIntDepthLine.Load(fDepthLeft, fDepthRight, nCount);
            }
            else
            {
                aIntDepthLine.Load(rEntity1.Point().getZ(), rEntity2.Point().getZ(), nCount);
            }

            // Texturkoordinateninterpolation?
            if(bTextureUsed)
            {
                aIntTexSLine.Load(
                    rEntity1.TexCoor().getX() * fTexWidth,
                    rEntity2.TexCoor().getX() * fTexWidth,
                    nCount);

                aIntTexTLine.Load(
                    rEntity1.TexCoor().getY() * fTexHeight,
                    rEntity2.TexCoor().getY() * fTexHeight,
                    nCount);
            }

            if(bNormalsUsed && GetShadeModel() == Base3DPhong)
            {
                // Normalen und Geometrie interpolieren
                if(GetTransformationSet())
                {
                    basegfx::B3DVector aInvTrans(GetTransformationSet()->GetTranslate());
                    basegfx::B3DVector aInvScale(GetTransformationSet()->GetScale());

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
                                basegfx::B3DPoint aPoint = Get3DCoor(aTmpPoint, nDepth);
                                aPoint -= aInvTrans;
                                aPoint /= aInvScale;
                                basegfx::B3DVector aNormal;
                                aIntVectorLine.GetVector3DValue(aNormal);
                                aNormal.normalize();
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
                                basegfx::B3DPoint aPoint = Get3DCoor(aTmpPoint, nDepth);
                                aPoint -= aInvTrans;
                                aPoint /= aInvScale;
                                basegfx::B3DVector aNormal;
                                aIntVectorLine.GetVector3DValue(aNormal);
                                aNormal.normalize();
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

void Base3DDefault::Clipped3DTriangle(sal_uInt32 nInd1, sal_uInt32 nInd2, sal_uInt32 nInd3)
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
        rEntity1.Normal().normalize();
        rEntity2.Normal().normalize();
        rEntity3.Normal().normalize();

        if(GetShadeModel() != Base3DPhong)
        {
            // Normale berechnen, Farben auswerten
            if(rEntity1.PlaneNormal().getZ() < 0.0 && (GetLightGroup() && GetLightGroup()->GetModelTwoSide()))
                eMode = Base3DMaterialBack;

            rEntity1.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity1.Normal(),
                rEntity1.Point()
                );

            rEntity2.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity2.Normal(),
                rEntity2.Point()
                );

            rEntity3.Color() = SolveColorModel(
                GetMaterialObject(eMode),
                rEntity3.Normal(),
                rEntity3.Point()
                );

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
    if(rEntity1.Point().getY() < rEntity2.Point().getY() && rEntity1.Point().getY() < rEntity3.Point().getY())
    {
        // rEntity1 ist der oberste
        pEntTop = &rEntity1;

        // Left, Right erst mal zuweisen
        pEntRight = &rEntity3;
        pEntLeft = &rEntity2;
    }
    else
    {
        if(rEntity2.Point().getY() < rEntity3.Point().getY())
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
        sal_Int32 nDeltaYLeft = aOutPointLeft.Y() - aOutPointTop.Y();
        sal_Int32 nDeltaYRight = aOutPointRight.Y() - aOutPointTop.Y();
        sal_Int32 nYLine;

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
        aIntDepthLeft.Load(pEntTop->Point().getZ(), pEntLeft->Point().getZ(), nDeltaYLeft);

        aIntXPosRight.Load(aOutPointTop.X(), aOutPointRight.X(), nDeltaYRight);
        aIntDepthRight.Load(pEntTop->Point().getZ(), pEntRight->Point().getZ(), nDeltaYRight);

        if(bTextureUsed)
        {
            // #96837#
            if(mbPTCorrection)
            {
                // Load real depth interpolators (if needed)
                const double fRealDepthLeft(1.0 / GetTransformationSet()->ViewToEyeCoor(pEntLeft->Point()).getZ());
                const double fRealDepthRight(1.0 / GetTransformationSet()->ViewToEyeCoor(pEntRight->Point()).getZ());
                const double fRealDepthTop(1.0 / GetTransformationSet()->ViewToEyeCoor(pEntTop->Point()).getZ());
                aRealDepthLeft.Load(fRealDepthTop, fRealDepthLeft, nDeltaYLeft);
                aRealDepthRight.Load(fRealDepthTop, fRealDepthRight, nDeltaYRight);

                // #96837#
                aIntTexSLeft.Load(
                    pEntTop->TexCoor().getX() * fTexWidth * fRealDepthTop,
                    pEntLeft->TexCoor().getX() * fTexWidth * fRealDepthLeft,
                    nDeltaYLeft);

                aIntTexTLeft.Load(
                    pEntTop->TexCoor().getY() * fTexHeight * fRealDepthTop,
                    pEntLeft->TexCoor().getY() * fTexHeight * fRealDepthLeft,
                    nDeltaYLeft);

                aIntTexSRight.Load(
                    pEntTop->TexCoor().getX() * fTexWidth * fRealDepthTop,
                    pEntRight->TexCoor().getX() * fTexWidth * fRealDepthRight,
                    nDeltaYRight);

                aIntTexTRight.Load(
                    pEntTop->TexCoor().getY() * fTexHeight * fRealDepthTop,
                    pEntRight->TexCoor().getY() * fTexHeight * fRealDepthRight,
                    nDeltaYRight);
            }
            else
            {
                aIntTexSLeft.Load(
                    pEntTop->TexCoor().getX() * fTexWidth,
                    pEntLeft->TexCoor().getX() * fTexWidth,
                    nDeltaYLeft);

                aIntTexTLeft.Load(
                    pEntTop->TexCoor().getY() * fTexHeight,
                    pEntLeft->TexCoor().getY() * fTexHeight,
                    nDeltaYLeft);

                aIntTexSRight.Load(
                    pEntTop->TexCoor().getX() * fTexWidth,
                    pEntRight->TexCoor().getX() * fTexWidth,
                    nDeltaYRight);

                aIntTexTRight.Load(
                    pEntTop->TexCoor().getY() * fTexHeight,
                    pEntRight->TexCoor().getY() * fTexHeight,
                    nDeltaYRight);
            }
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

void Base3DDefault::DrawLinePhongTexture(sal_Int32 nYPos, B3dMaterial& rMat)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        basegfx::B3DVector aVectorLeft;
        aIntVectorLeft.GetVector3DValue(aVectorLeft);
        basegfx::B3DVector aVectorRight;
        aIntVectorRight.GetVector3DValue(aVectorRight);
        aIntVectorLine.Load(aVectorLeft, aVectorRight, nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        // #96837##
        if(mbPTCorrection)
        {
            aRealDepthLine.Load(aRealDepthLeft.GetDoubleValue(), aRealDepthRight.GetDoubleValue(), nXLineDelta);
        }

        aIntTexSLine.Load(aIntTexSLeft.GetDoubleValue(), aIntTexSRight.GetDoubleValue(), nXLineDelta);
        aIntTexTLine.Load(aIntTexTLeft.GetDoubleValue(), aIntTexTRight.GetDoubleValue(), nXLineDelta);

        if(GetTransformationSet())
        {
            basegfx::B3DVector aInvTrans = GetTransformationSet()->GetTranslate();
            basegfx::B3DVector aInvScale = GetTransformationSet()->GetScale();

            while(nXLineDelta--)
            {
                // Werte vorbereiten
                sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

                // Punkt ausgeben
                if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                {
                    Point aTmpPoint(nXLineStart, nYPos);
                    basegfx::B3DPoint aPoint = Get3DCoor(aTmpPoint, nDepth);
                    aPoint -= aInvTrans;
                    aPoint /= aInvScale;
                    basegfx::B3DVector aNormal;
                    aIntVectorLine.GetVector3DValue(aNormal);
                    aNormal.normalize();
                    Color aCol = SolveColorModel(rMat, aNormal, aPoint);

                    // #96837#
                    if(mbPTCorrection)
                    {
                        GetActiveTexture()->ModifyColor(
                            aCol,
                            aIntTexSLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue(),
                            aIntTexTLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue()
                            );
                    }
                    else
                    {
                        GetActiveTexture()->ModifyColor(
                            aCol,
                            aIntTexSLine.GetDoubleValue(),
                            aIntTexTLine.GetDoubleValue()
                            );
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

                    // #96837#
                    if(mbPTCorrection)
                    {
                        aRealDepthLine.Increment();
                    }

                    aIntTexSLine.Increment();
                    aIntTexTLine.Increment();
                }
            }
        }
    }
}

void Base3DDefault::DrawLinePhong(sal_Int32 nYPos, B3dMaterial& rMat)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

    if(nXLineDelta > 0)
    {
        // Ausserhalb des Clipping-Bereichs?
        if(IsScissorRegionActive()
            && ( nXLineStart+nXLineDelta < aDefaultScissorRectangle.Left()
            || nXLineStart > aDefaultScissorRectangle.Right()))
            return;

        basegfx::B3DVector aVectorLeft;
        aIntVectorLeft.GetVector3DValue(aVectorLeft);
        basegfx::B3DVector aVectorRight;
        aIntVectorRight.GetVector3DValue(aVectorRight);
        aIntVectorLine.Load(aVectorLeft, aVectorRight, nXLineDelta);
        aIntDepthLine.Load(aIntDepthLeft.GetDoubleValue(), aIntDepthRight.GetDoubleValue(), nXLineDelta);

        if(GetTransformationSet())
        {
            basegfx::B3DVector aInvTrans = GetTransformationSet()->GetTranslate();
            basegfx::B3DVector aInvScale = GetTransformationSet()->GetScale();
            while(nXLineDelta--)
            {
                // Werte vorbereiten
                sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

                // Punkt ausgeben
                if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
                {
                    Point aTmpPoint(nXLineStart, nYPos);
                    basegfx::B3DPoint aPoint = Get3DCoor(aTmpPoint, nDepth);
                    aPoint -= aInvTrans;
                    aPoint /= aInvScale;
                    basegfx::B3DVector aNormal;
                    aIntVectorLine.GetVector3DValue(aNormal);
                    aNormal.normalize();
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

void Base3DDefault::DrawLineColorTexture(sal_Int32 nYPos)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

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

        // #96837#
        if(mbPTCorrection)
        {
            aRealDepthLine.Load(aRealDepthLeft.GetDoubleValue(), aRealDepthRight.GetDoubleValue(), nXLineDelta);
        }

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
            {
                Color aCol = aIntColorLine.GetColorValue();

                // #96837#
                if(mbPTCorrection)
                {
                    GetActiveTexture()->ModifyColor(
                        aCol,
                        aIntTexSLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue(),
                        aIntTexTLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue()
                        );
                }
                else
                {
                    GetActiveTexture()->ModifyColor(
                        aCol,
                        aIntTexSLine.GetDoubleValue(),
                        aIntTexTLine.GetDoubleValue()
                        );
                }

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

                // #96837#
                if(mbPTCorrection)
                {
                    aRealDepthLine.Increment();
                }
            }
        }
    }
}

void Base3DDefault::DrawLineColor(sal_Int32 nYPos)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

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
            sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

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

void Base3DDefault::DrawLineTexture(sal_Int32 nYPos, Color& rCol)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

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

        // #96837#
        if(mbPTCorrection)
        {
            aRealDepthLine.Load(aRealDepthLeft.GetDoubleValue(), aRealDepthRight.GetDoubleValue(), nXLineDelta);
        }

        while(nXLineDelta--)
        {
            // Werte vorbereiten
            sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

            // Punkt ausgeben
            if(IsVisibleAndScissor(nXLineStart, nYPos, nDepth))
            {
                // Texturkoordinateninterpolation?
                Color aCol = rCol;

                // #96837#
                if(mbPTCorrection)
                {
                    GetActiveTexture()->ModifyColor(
                        aCol,
                        aIntTexSLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue(),
                        aIntTexTLine.GetDoubleValue() / aRealDepthLine.GetDoubleValue()
                        );
                }
                else
                {
                    GetActiveTexture()->ModifyColor(
                        aCol,
                        aIntTexSLine.GetDoubleValue(),
                        aIntTexTLine.GetDoubleValue()
                        );
                }

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

                // #96837#
                if(mbPTCorrection)
                {
                    aRealDepthLine.Increment();
                }
            }
        }
    }
}

void Base3DDefault::DrawLine(sal_Int32 nYPos, Color& rCol)
{
    // Ausserhalb des Clipping-Bereichs?
    if(IsScissorRegionActive()
        && (nYPos < aDefaultScissorRectangle.Top()
        || nYPos > aDefaultScissorRectangle.Bottom()))
        return;

    // Von links bis rechts zeichnen
    sal_Int32 nXLineStart = aIntXPosLeft.GetLongValue();
    sal_Int32 nXLineDelta = aIntXPosRight.GetLongValue() - nXLineStart;

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
            sal_uInt32 nDepth = aIntDepthLine.GetUINT32Value();

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

void Base3DDefault::LoadLeftTexture(sal_Int32 nSize)
{
    aIntXPosLeft.Load(aOutPointLeft.X(), aOutPointRight.X(), nSize);
    aIntDepthLeft.Load(pEntLeft->Point().getZ(), pEntRight->Point().getZ(), nSize);

    // #96837#
    if(mbPTCorrection)
    {
        const double fRealDepthLeft = 1.0 / GetTransformationSet()->ViewToEyeCoor(pEntLeft->Point()).getZ();
        const double fRealDepthRight = 1.0 / GetTransformationSet()->ViewToEyeCoor(pEntRight->Point()).getZ();
        aRealDepthLeft.Load(fRealDepthLeft, fRealDepthRight, nSize);

        aIntTexSLeft.Load(
            pEntLeft->TexCoor().getX() * fTexWidth * fRealDepthLeft,
            pEntRight->TexCoor().getX() * fTexWidth * fRealDepthRight,
            nSize);

        aIntTexTLeft.Load(
            pEntLeft->TexCoor().getY() * fTexHeight * fRealDepthLeft,
            pEntRight->TexCoor().getY() * fTexHeight * fRealDepthRight,
            nSize);
    }
    else
    {
        aIntTexSLeft.Load(
            pEntLeft->TexCoor().getX() * fTexWidth,
            pEntRight->TexCoor().getX() * fTexWidth,
            nSize);

        aIntTexTLeft.Load(
            pEntLeft->TexCoor().getY() * fTexHeight,
            pEntRight->TexCoor().getY() * fTexHeight,
            nSize);
    }
}

void Base3DDefault::LoadLeft(sal_Int32 nSize)
{
    aIntXPosLeft.Load(aOutPointLeft.X(), aOutPointRight.X(), nSize);
    aIntDepthLeft.Load(pEntLeft->Point().getZ(), pEntRight->Point().getZ(), nSize);
}

void Base3DDefault::LoadRightTexture(sal_Int32 nSize)
{
    aIntXPosRight.Load(aOutPointRight.X(), aOutPointLeft.X(), nSize);
    aIntDepthRight.Load(pEntRight->Point().getZ(), pEntLeft->Point().getZ(), nSize);

    // #96837#
    if(mbPTCorrection)
    {
        const double fRealDepthLeft = 1.0 / GetTransformationSet()->ViewToEyeCoor(pEntLeft->Point()).getZ();
        const double fRealDepthRight = 1.0 / GetTransformationSet()->ViewToEyeCoor(pEntRight->Point()).getZ();
        aRealDepthRight.Load(fRealDepthRight, fRealDepthLeft, nSize);

        // #96837#
        aIntTexSRight.Load(
            pEntRight->TexCoor().getX() * fTexWidth * fRealDepthRight,
            pEntLeft->TexCoor().getX() * fTexWidth * fRealDepthLeft,
            nSize);

        aIntTexTRight.Load(
            pEntRight->TexCoor().getY() * fTexHeight * fRealDepthRight,
            pEntLeft->TexCoor().getY() * fTexHeight * fRealDepthLeft,
            nSize);
    }
    else
    {
        aIntTexSRight.Load(
            pEntRight->TexCoor().getX() * fTexWidth,
            pEntLeft->TexCoor().getX() * fTexWidth,
            nSize);

        aIntTexTRight.Load(
            pEntRight->TexCoor().getY() * fTexHeight,
            pEntLeft->TexCoor().getY() * fTexHeight,
            nSize);
    }
}

void Base3DDefault::LoadRight(sal_Int32 nSize)
{
    aIntXPosRight.Load(aOutPointRight.X(), aOutPointLeft.X(), nSize);
    aIntDepthRight.Load(pEntRight->Point().getZ(), pEntLeft->Point().getZ(), nSize);
}

void Base3DDefault::NextStepRightTexture()
{
    aIntXPosRight.Increment();
    aIntDepthRight.Increment();

    // #96837#
    if(mbPTCorrection)
    {
        aRealDepthRight.Increment();
    }

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

    // #96837#
    if(mbPTCorrection)
    {
        aRealDepthLeft.Increment();
    }

    aIntTexSLeft.Increment();
    aIntTexTLeft.Increment();
}

void Base3DDefault::NextStepLeft()
{
    aIntXPosLeft.Increment();
    aIntDepthLeft.Increment();
}

// eof
