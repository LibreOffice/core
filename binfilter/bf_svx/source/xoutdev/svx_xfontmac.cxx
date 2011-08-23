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

#ifndef _SOLAR_H
#include <Solar.h>
#endif

#if defined (MAC)
#include <mac_start.h>

#include <GestaltEqu.h>
#include <ToolUtils.h>

#ifndef __RESOURCES__
  #include <Resources.h>
#endif

#ifndef __FONTS__
  #include <Fonts.h>
#endif

#include <math.h>
#include <GXIncludes.h>

#include <XFontMac.hxx>

#include <mac_end.h>
#endif

#ifndef _SV_HXX
#include <sv.hxx>
#endif

#include "xpoly.hxx"
#include "xattr.hxx"
#include "xoutx.hxx"

#define GLOBALOVERFLOW
namespace binfilter {


/*************************************************************************
|*
|*    HasGX()
|*
|*    Prueft, ob GX installiert ist
|*    Ersterstellung    05.02.95 DV
|*    Letzte Aenderung  05.02.95 DV
|*
*************************************************************************/
BOOL HasGX()
{
    long			 nVersion;
    gxGraphicsClient aClient;

    if ( Gestalt( gestaltGraphicsVersion, &nVersion ) != noErr )
        return FALSE;

    aClient = GXNewGraphicsClient( nil, 500L * 1024L, 0L );
    if ( aClient == 0 )
        return FALSE;

    GXEnterGraphics();
    if ( GXGetGraphicsError( nil ) == out_of_memory )
        return FALSE;

    return TRUE;
}


/*************************************************************************
|*
|*    SetUpFontData()
|*
|*    Uebertraegt die Fontattribute uas dem OutputDevice
|*	  in das Text Objekt
|*
|*    Ersterstellung    05.02.95 DV
|*    Letzte Aenderung  05.02.95 DV
|*
*************************************************************************/

void SetUpFontData( OutputDevice &rOut, gxShape aTextShape )
{
    gxFont		aFont = 0;

    String& rName = rOut.GetFont().GetName();

    GXFindFonts( nil, gxFamilyFontName, gxMacintoshPlatform,
                 gxRomanScript, gxEnglishLanguage, rName.Len(),
                 (const unsigned char*) rName.GetStr(),
                 1, 1, &aFont );

    if ( aFont )
    {
        gxFont aTmpFont;
        rName = rOut.GetFont().GetStyleName();

        if ( rName.Len() )
        {
            if ( GXFindFonts( aFont, gxStyleFontName, gxMacintoshPlatform,
                              gxRomanScript, gxEnglishLanguage,
                              rName.Len(),
                              (const unsigned char*) rName.GetStr(),
                              1, 1, &aTmpFont ) )
            {
                aFont = aTmpFont;
            }
        }
        else
        {
            if ( GXFindFonts( aFont, gxStyleFontName, gxMacintoshPlatform,
                              gxRomanScript, gxEnglishLanguage,
                              7, (const unsigned char*) "Regular",
                              1, 1, &aTmpFont ) )
            {
                aFont = aTmpFont;
            }
        }

        Size aSize( rOut.GetFont().GetSize() );
        aSize = rOut.LogicToPixel( aSize );

        GXSetShapeFont( aTextShape, aFont );
        GXSetShapeTextSize( aTextShape, aSize.Height() * fixed1 );
    }
}


/*************************************************************************
|*
|*    XOutGetCharOutlineGX()
|*
|*    Ein Zeichen eines Outlinefonts in ein Bezier-PolyPolygon umwandeln
|*    Wenn keine Umwandlung moeglich ist, wird ein leeres PolyPolygon
|*    zurueckgegeben
|*    Ersterstellung    03.02.95 DV
|*    Letzte Aenderung  27.06.95 DV
|*
*************************************************************************/

XPolyPolygon XOutGetCharOutlineGX(USHORT nChar, OutputDevice& rOut,
                                BOOL bOptimizeSize)
{
    gxPoint			aPoint, aDummy;
    gxShape			aOutlineShape;
    long			nSize, nContours, i, j;
    gxPaths			*pPathData;
    long			*pCurPath;
    long			nFontHeight = 1,
                    nOrgHeight = 1;
    unsigned char	cChar = (unsigned char) nChar;

    Font aFont = rOut.GetFont();

    if ( bOptimizeSize )
    {
        /* Font moeglichst gross machen, um eine hohe Aufloesung zu
         * erreichen, da sonst eventuell mit Hints gearbeitet wird
         */
        aFont.SetSize(rOut.PixelToLogic(Size(0, 500)));
        nFontHeight = aFont.GetSize().Height();
        rOut.SetFont(aFont);
        nOrgHeight = aFont.GetSize().Height();
    }

    /* Textshape mit einem Buchstaben anlegen, die Fontattribute holen
     * und dann in ein Path Objekt umwandeln.
     */
    aOutlineShape = GXNewShape( gxTextType );
    SetUpFontData( rOut, aOutlineShape );
    GXGetShapeFontMetrics( aOutlineShape, &aPoint, &aDummy, &aDummy, &aDummy );
    aPoint.y = -aPoint.y;

    GXSetText( aOutlineShape, 1, &cChar, &aPoint );

    GXSetShapeType( aOutlineShape, gxPathType );

    /* Die Groesse das Path Objektes ermitteln, genug Speicher anfordern,
     * und die Path Daten aus dem Path Objekt rausholen
     */
    nSize = GXGetPaths( aOutlineShape, nil );
    pPathData = (gxPaths*) new char[ nSize ];
    nSize = GXGetPaths( aOutlineShape, pPathData );

    nContours = pPathData->contours;

    /* Der Path laesst sich vortrefflich auf ein XPolyPolygon abbilden.
     * Da wir schon wissen, wieviele Polygone wir habe, koennen wir auch
     * gleich die richtige Groesse anfordern. Danach wird es leider etwas
     * trickreich, da die GXPaths Struktur dynamisch ist. Deshalb auch
     * die vielen Pointer.
     */
    XPolyPolygon aXPolyPoly( nContours );	// Unser XPolyPolygon

    pCurPath = ((long*)pPathData) + 1;		// pCurPath zeigt jetzt auf das erste
                                            // Polygon ( kommt gleich hinter der
                                            // Anzahl der Path Objekte )
    for ( i=0; i< nContours; i++ )
    {
        /* fuer jeden einzelnen Path brauchen wir die Anzahl der darin
         * enthaltenen Punkte, einen Zeiger auf das Bitfeld, das festlegt,
         * ob ein Punkt auf der Kurve liegt oder nicht, und einen Zeiger
         * auf die einzelnen Punkte. Da wir hier auch wieder eine dynamische
         * Datenstruktur haben, wirds wieder undurchsichtig.
         * pPoint wird zunaechst auf den ersten Punkt gesetzt. ( Wir haben
         * ein long fuer die Anzahl und ( Anzahl +31 )/ 32 longs fuer das
         * Bitfeld.
         */
        long		nPointAnz	= ( (gxPath*) pCurPath )->vectors;
        char		*pBits		= (char*) ( (gxPath*) pCurPath )->controlBits;
        USHORT		nPnt;
        XPolygon	aXPoly( 100, 50 );
        BOOL		bCloseIt = TRUE;

        gxPoint 	*pPoint;

        pPoint	= (gxPoint*) ( pCurPath + ( 1 + (nPointAnz+31) / 32 ) );
        nPnt	= 0;

        for ( j=0; j<nPointAnz; j++ )
        {
            /* Die einzelnen Punkte werden von gxPoints in StarView Points
             * umgewandelt und in das XPolygon eingetragen. Anschliessend
             * wird geprueft, ob es sich um einen Off Curve point handlete.
             * (gluecklicherweise bietet Apple eine Funktion an, mit der man
             * die Bits in einem dynamischen Bitfeld testen kann (BitTst) ).
             * Zum Schluss muss jetzt nur noch der Punktpointer eins weiter
             * gesetzt werden.
             */
            aXPoly[nPnt].X() = (long) FixedToInt( pPoint->x );
            aXPoly[nPnt].Y() = (long) FixedToInt( pPoint->y );
            nPnt++;

            /* Wenn Off Curve Points ins Spiel kommen, wirds leider etwas
             * komplizierter
             */
            if ( BitTst( pBits, j ) )
            {
                /* Es gibt wohl wirklich polygone, die mit einem Off
                 * Curve Point anfangen.
                 */
                if ( j==0 )
                {
                    gxPoint *pTmpPt = pPoint + ( nPointAnz - 1);
                    aXPoly[nPnt] = aXPoly[nPnt-1];
                    nPnt++;
                    aXPoly[0].X() = (long) FixedToInt( pTmpPt->x );
                    aXPoly[0].Y() = (long) FixedToInt( pTmpPt->y );
                    bCloseIt = FALSE;
                }

                // Punkt verdoppeln fuer Bezier-Wandlung
                aXPoly[nPnt] = aXPoly[nPnt-1];
                nPnt++;

                /* Wenn Polygone mit Off Curve Points anfangen, dann hoeren
                 * sie sicherlich auch mal damit auf
                 */
                if ( j == nPointAnz - 1 )
                {
                    aXPoly[nPnt] = aXPoly[0];
                    nPnt++;
                    bCloseIt = FALSE;
                }
                else if ( ! BitTst( pBits, j+1 ) )
                {
                    /* Wenn der naechste Punkt auf der Kurve liegt, dann
                     * braucht er nur in unser Polygon eingetragen zu werden
                     */
                    pPoint += 1;
                    aXPoly[nPnt].X() = (long) FixedToInt( pPoint->x );
                    aXPoly[nPnt].Y() = (long) FixedToInt( pPoint->y );
                    nPnt++;
                    j++;
                }
                else
                {
                    /* ansonsten muessen wir noch einen weiteren Punkt
                     * mit aufnehmen. ( Und zwar den Mittelpunkt zwischen
                     * den beiden Kontrollpunkten. )
                     */
                    aXPoly[nPnt].X() = (long) FixedToInt( pPoint->x / 2 +
                                                          (pPoint+1)->x / 2 );
                    aXPoly[nPnt].Y() = (long) FixedToInt( pPoint->y / 2 +
                                                          (pPoint+1)->y / 2 );
                    nPnt++;
                }
                /* Umrechnung in Bezier (PQ=TrueType-Controlpunkt):
                 * P1 = 1/3 * (P0 + 2 * PQ)
                 * P2 = 1/3 * (P3 + 2 * PQ)
                 */
                aXPoly[nPnt-3] = (aXPoly[nPnt-4] + aXPoly[nPnt-3] * 2) / 3;
                aXPoly[nPnt-2] = (aXPoly[nPnt-1] + aXPoly[nPnt-2] * 2) / 3;
                aXPoly.SetFlags(nPnt-3, XPOLY_CONTROL);
                aXPoly.SetFlags(nPnt-2, XPOLY_CONTROL);
            }
            pPoint += 1;
        }
        for (j = 0; j < nPnt; j++)
        {
            // GX gibt Pixelkoordinaten zurueck, deshalb
            // in logische Koordinaten umrechnen; Size verwenden,
            // damit der MapMode-Origin nicht beachtet wird
            Size aSize(aXPoly[j].X(), aXPoly[j].Y());
            aSize = rOut.PixelToLogic(aSize);
            // zuletzt auf Originalgroesse des Fonts skalieren
            aXPoly[j].X() = aSize.Width()  * nOrgHeight / nFontHeight;
            aXPoly[j].Y() = aSize.Height() * nOrgHeight / nFontHeight;
        }

        // Polygon schliessen
        if ( bCloseIt )
            aXPoly[nPnt] = aXPoly[0];


        /* Nachdem die einzelnen Punkte in das XPolygon eingetragen wurden,
         * wird jetzt das Xpolygon in das XPolyPolygon gestopft und danach
         * geloescht.
         * Unser pPoint Pointer zeigt jetzt nicht mehr auf einen Punkt sondern
         * auf den Anfang des naechsten Path. Das nutzen wir natuerlich aus, um
         * den Path Pointer umzusetzen.
         */
        aXPolyPoly.Insert( aXPoly );
        aXPoly.SetPointCount(0);

        pCurPath = (long*) pPoint;
    }

    /* was wir angefordert haben, muessen wir auch wieder freigeben!
    */
    GXDisposeShape( aOutlineShape );
    delete[] pPathData;

    rOut.SetFont( aFont );

    return aXPolyPoly;
}

/*************************************************************************
|*
|*    Hilfsfunktionen fuer XOutGetCharOutlineQD gefunden in einem
|*	  Artikel zu Develop 8, 1995
|*
|*    Ersterstellung    28.11.95 KH
|*    Letzte Aenderung  28.11.95 KH
|*
*************************************************************************/

inline long PostFontError(long error)
{
#ifdef DBG_UTIL
    if (error)
        DBG_ERROR("FontError");
#endif
    return error;
}

/*************************************************************************/

Handle GetNamedSfntHandle(const String& rName, short styleWord)
{
    Handle hFond = GetNamedResource('FOND', rName.GetPascalStr());
    if (hFond && !ResError())
    {	FamRec* pFrec = (FamRec*)*hFond;
        register short* assoc = (short*)(pFrec + 1);
        register short sfntID = 0;
        register count = *assoc++;

        while (count-- >= 0 && !sfntID)
        {	if (*assoc++ == 0)		/* size == 0 means sfnt */
                if (*assoc++ == styleWord)
                    sfntID = *assoc;
                else
                    assoc++;
            else
                assoc += 2;
        }
        if (sfntID)
        {	Handle hSfnt = GetResource('sfnt', sfntID);
            if (hSfnt && !ResError())
                return hSfnt;
        }
    }
    DBG_ERROR("GetNamedSfntHandle : Font NotFound");
    return 0;
}

/*************************************************************************/

void InitGlyphOutline(GlyphOutline* out)
{
    out->contourCount = 0;
    out->pointCount = 0;
    out->endPoints = (short**)NewHandle(0);
    out->onCurve = (BYTE**)NewHandle(0);
    out->x = (Fixed**)NewHandle(0);
    out->y = (Fixed**)NewHandle(0);
}

/*************************************************************************/

void KillGlyphOutline(GlyphOutline* out)
{
    DisposeHandle((Handle)out->endPoints);
    DisposeHandle((Handle)out->onCurve);
    DisposeHandle((Handle)out->x);
    DisposeHandle((Handle)out->y);
}

/*************************************************************************/

void LockGlyphOutline(GlyphOutline* out)
{
    HLock((Handle)out->endPoints);
    HLock((Handle)out->onCurve);
    HLock((Handle)out->x);
    HLock((Handle)out->y);
}

/*************************************************************************/

void UnlockGlyphOutline(GlyphOutline* out)
{
    HUnlock((Handle)out->endPoints);
    HUnlock((Handle)out->onCurve);
    HUnlock((Handle)out->x);
    HUnlock((Handle)out->y);
}

/*************************************************************************/

static short GetFontState(Handle sfnt)
{
    short state;

    LoadResource(sfnt);
    state = HGetState(sfnt);
    HNoPurge(sfnt);
    return state;
}


/*************************************************************************/

static void* GetSfntTablePtr(Handle sfnt, fontTableTag tag)
{
    FontTableInfo info;

    if (GetSfntTableInfo(sfnt, tag, &info))
        return 0;
    return *sfnt + info.offset;
}

/*************************************************************************/

inline void SetFontState(Handle sfnt, short state)
{
    HSetState(sfnt, state);
}

/*************************************************************************/

FontError GetSfntTableInfo(Handle sfnt, fontTableTag tag, FontTableInfo* fTable)
{
    short state = GetFontState(sfnt);
    register sfnt_OffsetTable* dir = (sfnt_OffsetTable*)*sfnt;
    register sfnt_DirectoryEntry* table = dir->table;
    register short count = dir->numOffsets;
    FontError error = fNoError;

    for (; --count >= 0; table++)
        if (table->tableTag == tag)
        {	fTable->offset = table->offset;
            fTable->length = table->length;
            fTable->checkSum = table->checkSum;
            break;
        }
    if (count < 0)
        error = PostFontError(fTableNotFound);
    SetFontState(sfnt, state);
    return error;
}

/*************************************************************************/

/*	Glue routine and Macro to make handle growing easier.*/

inline short NiceSetHandleSize(Handle h, long size)
{
    SetHandleSize(h, size);
    return MemError();
}

#define SHS(h, s)	NiceSetHandleSize((Handle)h, s)

/*************************************************************************/

/*	Returns the glyph index for the given character code, or
 *	an error code.
 */
long GetCharGlyphIndex(Handle sfnt, unsigned short charCode)
{
    sfnt_char2IndexDirectory* table;
    short i;
    long mapOffset = 0;
    long glyphIndex = 0;		/* missing character glyph */
    short state = GetFontState(sfnt);

    if (!( 0 != (table = (sfnt_char2IndexDirectory*) GetSfntTablePtr(sfnt, tag_CharToIndexMap))))
    {	SetFontState(sfnt, state);
        return PostFontError(fTableNotFound);
    }

    {	register sfnt_platformEntry* plat = table->platform;

        /* You can change this section to look for other scripts
        */
        for ( i = table->numTables - 1; i >= 0; --i )
        {	if ( plat->platformID == plat_Macintosh && plat->specificID == smRoman)
            {	mapOffset = plat->offset;
                break;
            }
            ++plat;
        }
    }

    if (mapOffset)
    {	sfnt_mappingTable* mapping = (sfnt_mappingTable*)((char*)table + mapOffset);

        switch ( mapping->format ) {
        case 0:
            {	BYTE* glyphs = (BYTE*)(mapping + 1);
                glyphIndex = glyphs[charCode];
            }
            break;
        case 6:
            {	short* glyphs = (short*)(mapping + 1);
                short first = *glyphs++;
                short count = *glyphs++;
                charCode -= first;
                if (charCode < count)
                    glyphIndex = glyphs[charCode];
            }
            break;
        default:
            glyphIndex = PostFontError(fUnimplemented);
        }
    }
    else
        glyphIndex = PostFontError(fCMapNotFound);

    SetFontState(sfnt, state);
    return glyphIndex;
}

/*************************************************************************/

static void* GetSfntGlyphPtr(Handle sfnt, long glyphIndex, long* length)
{
    void* loc;
    char* glyphStart;
    sfnt_FontHeader* head;

    if (!((0 != (head = (sfnt_FontHeader*)GetSfntTablePtr(sfnt, tag_FontHeader))) &&
          (0 != (loc =  				  GetSfntTablePtr(sfnt, tag_IndexToLoc))) &&
          (0 != (glyphStart = (char*)     GetSfntTablePtr(sfnt, tag_GlyphData)))))
        return 0;

    if (head->indexToLocFormat == SHORT_INDEX_TO_LOC_FORMAT)
    {	unsigned short* offset = (unsigned short*)loc + glyphIndex;
        *length = (long)(offset[1] - *offset) << 1;
        return glyphStart + ((long)*offset << 1);
    }
    else
    {	long* offset = (long*)loc + glyphIndex;
        *length = offset[1] - *offset;
        return glyphStart + *offset;
    }
}

/*************************************************************************/

void MoveGlyphOutline(GlyphOutline* out, Fixed xDelta, Fixed yDelta)
{
    Fixed* x = *out->x;
    Fixed* y = *out->y;
    short count = out->pointCount;

    for (--count; count >= 0; --count)
    {	*x++ += xDelta;
        *y++ += yDelta;
    }
    out->origin.x += xDelta;
    out->origin.y += yDelta;
}

/*************************************************************************/

void AppendHandle(Handle dst, Handle extra)
{
    long dstSize = GetHandleSize(dst);
    long extraSize = GetHandleSize(extra);

    SetHandleSize(dst, dstSize + extraSize);
    if (MemError())
        Debugger();
    else
        BlockMove(*extra, *dst + dstSize, extraSize);
}

#define APPENDHANDLE(a,b)	AppendHandle((Handle)a, (Handle)b)

/*************************************************************************/

void AppendGlyphOutline(GlyphOutline* a, GlyphOutline* b)
{
    APPENDHANDLE(a->endPoints, b->endPoints);
    {	short* p = *a->endPoints + a->contourCount;
        short* endp = p + b->contourCount;
        short newFirstPoint = a->contourCount ? p[-1] + 1 : 0;
        for (; p < endp; p++)
            *p = *p + newFirstPoint;
    }
    a->contourCount += b->contourCount;
    a->pointCount += b->pointCount;
    APPENDHANDLE(a->onCurve, b->onCurve);
    APPENDHANDLE(a->x, b->x);
    APPENDHANDLE(a->y, b->y);
}

/*************************************************************************/

#define GetUnsignedByte( p ) ((BYTE)(*p++))

/*************************************************************************/

FontError GetGlyphOutline(Handle hSfnt, long glyphIndex, GlyphOutline* pOutline, Matrix xform)
{
    short upem, state, sideBearing, adjustToLsb;
    short* pGlyph;
    sfnt_FontHeader* pHead;
    sfnt_HorizontalHeader* pHHea;
    sfnt_HorizontalMetrics* pHori;
    long length;
    FontError error = fNoError;

    state = GetFontState( hSfnt );
    HLock( hSfnt );

    if (!((0 != (pHead = (sfnt_FontHeader*) GetSfntTablePtr(hSfnt, tag_FontHeader))) &&
          (0 != (pHHea = (sfnt_HorizontalHeader*) GetSfntTablePtr(hSfnt, tag_HoriHeader))) &&
          (0 != (pHori = (sfnt_HorizontalMetrics*) GetSfntTablePtr(hSfnt, tag_HorizontalMetrics)))))
    {
        error = fTableNotFound;
        goto EXIT;
    }

    upem = pHead->unitsPerEm;

    {	long longMetrics = pHHea->numberLongMetrics;
        if ( glyphIndex < longMetrics )
        {	pOutline->advance.x = FixRatio( pHori[glyphIndex].advance, upem );
            sideBearing = pHori[glyphIndex].sideBearing;
        }
        else
        {	short *lsb = (short *)&pHori[longMetrics]; /* first entry after[AW,LSB] array */

            pOutline->advance.x = FixRatio( pHori[longMetrics-1].advance, upem );
            sideBearing = pHori[glyphIndex - longMetrics].sideBearing;
        }
        pOutline->advance.y = 0;
    }

    pOutline->origin.x = pOutline->origin.y = 0;

    if (!(pGlyph = (short*) GetSfntGlyphPtr(hSfnt, glyphIndex, &length)))
    {	error = fGlyphNotFound;
        goto EXIT;
    }

    if (length == 0)
    {	pOutline->contourCount = pOutline->pointCount = 0;
        goto EXIT;
    }

    pOutline->contourCount = *pGlyph++;
    adjustToLsb = *pGlyph - sideBearing;		/* xmin - lsb */
    pGlyph += 4;						/* skip bounds rect */

    if (pOutline->contourCount == 0)
        pOutline->pointCount = 0;
    else if (pOutline->contourCount == -1)
    {	short flags, index, newMatrix;

        pOutline->contourCount = pOutline->pointCount = 0;
        SHS(pOutline->endPoints, 0);
        SHS(pOutline->onCurve, 0);
        SHS(pOutline->x, 0);
        SHS(pOutline->y, 0);
        do
        {	Matrix compXform;
            short arg1, arg2;

            flags = *pGlyph++;
            index = *pGlyph++;
            newMatrix = false;

            if ( flags & ARG_1_AND_2_ARE_WORDS )
            {	arg1 = *pGlyph++;
                arg2 = *pGlyph++;
            }
            else
            {	char* byteP = (char*)pGlyph;
                if ( flags & ARGS_ARE_XY_VALUES )
                {	/* offsets are signed */
                    arg1 = *byteP++;
                    arg2 = *byteP;
                }
                else
                {	/* anchor points are unsigned */
                    arg1 = (unsigned char)*byteP++;
                    arg2 = (unsigned char)*byteP;
                }
                ++pGlyph;
            }
            {	GlyphOutline out;
                InitGlyphOutline(&out);
                GetGlyphOutline(hSfnt, index, &out, newMatrix ? compXform : xform);
                {	Fixed dx, dy;
                    if (flags & ARGS_ARE_XY_VALUES)
                    {	dx = FixRatio(arg1, upem);
                        dy = -FixRatio(arg2, upem);
                    }
                    else
                    {	dx = (*pOutline->x)[arg1] - (*out.x)[arg2];
                        dy = (*pOutline->y)[arg1] - (*out.y)[arg2];
                    }
                    MoveGlyphOutline(&out, dx, dy);
                }
                AppendGlyphOutline(pOutline, &out);
                KillGlyphOutline(&out);
            }
        } while (flags & MORE_COMPONENTS);
    }
    else if (pOutline->contourCount > 0)
    {	/*	Load in the end points.
         */
        {	long size = pOutline->contourCount * sizeof(short);

            if (SHS(pOutline->endPoints, size))
            {	error = fMemoryError;
                goto EXIT;
            }
            BlockMove( (Ptr)pGlyph, (Ptr)*pOutline->endPoints, size );
            pGlyph += pOutline->contourCount;
        }

        pOutline->pointCount = (*pOutline->endPoints)[pOutline->contourCount - 1] + 1;
        if (SHS(pOutline->onCurve, pOutline->pointCount * sizeof(char)))
        {	error = fMemoryError;
            goto EXIT;
        }
        if (SHS(pOutline->x, pOutline->pointCount * sizeof(Fixed)))
        {	error = fMemoryError;
            goto EXIT;
        }
        if (SHS(pOutline->y, pOutline->pointCount * sizeof(Fixed)))
        {	error = fMemoryError;
            goto EXIT;
        }

        /*	Skip the word for instruction count + the instructions.
         *	Then load in the onCurve bytes.
         */
        {	BYTE* p = (BYTE*)pGlyph + sizeof(short) + *pGlyph;
            BYTE* onCurve = *pOutline->onCurve;
            BYTE* stop = onCurve + pOutline->pointCount;
            BYTE flag;

            while (onCurve < stop)
            {	*onCurve++ = flag = GetUnsignedByte( p );
                if ( flag & REPEAT_FLAGS ) {
                    short count = GetUnsignedByte( p );
                    for (--count; count >= 0; --count)
                        *onCurve++ = flag;
                }
            }
            /*	Lets do X
            */
            {	short coord = adjustToLsb;
                Fixed* x = *pOutline->x;

                onCurve = *pOutline->onCurve;
                while (onCurve < stop)
                {	if ( (flag = *onCurve++) & XSHORT ) {
                        if ( flag & SHORT_X_IS_POS )
                            coord += GetUnsignedByte( p );
                        else
                            coord -= GetUnsignedByte( p );
                    }
                    else if ( !(flag & NEXT_X_IS_ZERO) )
                    {	coord += (short)(*p++) << 8;
                        coord += (BYTE)*p++;
                    }
                    *x++ = FixRatio( coord, upem );
                }
            }
            /*	Lets do Y
            */
            {	short coord = 0;
                Fixed* y = *pOutline->y;

                onCurve = *pOutline->onCurve;
                while (onCurve < stop)
                {	if ( (flag = *onCurve) & YSHORT ) {
                        if ( flag & SHORT_Y_IS_POS )
                            coord += GetUnsignedByte( p );
                        else
                            coord -= GetUnsignedByte( p );
                    }
                    else if ( !(flag & NEXT_Y_IS_ZERO) )
                    {	coord += (short)(*p++) << 8;
                        coord += (BYTE)*p++;
                    }
                    *y++ = -FixRatio( coord, upem );

                    /*	Filter off the extra bits
                    */
                    *onCurve++ = flag & ONCURVE;
                }
            }
        }
    }
    else
        error = fUnimplemented;
EXIT:
    SetFontState( hSfnt, state );

    return PostFontError(error);
}

/*************************************************************************/

static long* PackControlBits(long* p, BYTE * onCurve, long count)
{
    unsigned long mask = 0x80000000;

    *p = 0;
    while (count--)
    {	if (!mask)
        {	mask = 0x80000000;
            *++p = 0;
        }
        if (!*onCurve++)
            *p |= mask;
        mask >>= 1;
    }
    return p + 1;
}

/*************************************************************************/

gxPaths* OutlineToPaths(GlyphOutline* out)
{
    long size, *p, *origP;

    size = sizeof(long);		/* paths.contours */

    {	long i, sp = 0;
        for (i = 0; i < out->contourCount; i++)
        {	long pts = (*out->endPoints)[i] - sp + 1;
            size += sizeof(long);			/* path.vectors */
            size += (pts + 31 >> 5) << 2;	/* path.controlBits */
            size += pts << 3;			/* path.vector[] */
            sp = (*out->endPoints)[i] + 1;
        }
    }

    origP = p = (long*) SvMemAlloc( size );
    if (!p || MemError())	Debugger();

    *p++ = out->contourCount;
    {	long i, sp = 0;
        Fixed* x = *out->x;
        Fixed* y = *out->y;
        short* ep = *out->endPoints;
        BYTE* onCurve = *out->onCurve;
        for (i = 0; i < out->contourCount; i++)
        {	long pts = *ep - sp + 1;
            *p++ = pts;
            p = PackControlBits(p, onCurve, pts);
            onCurve += pts;
            while (pts--)
            {	*p++ = *x++;
                *p++ = *y++;
            }
            sp = *ep++ + 1;
        }
    }
    return (gxPaths*)origP;
}

/*************************************************************************/

void ScaleGlyphOutline(GlyphOutline* out, Fixed xScale, Fixed yScale)
{
    Fixed* x = *out->x;
    Fixed* y = *out->y;
    short count = out->pointCount;

    for (--count; count >= 0; --count)
    {	*x = FixMul( *x, xScale );
        x++;
        *y = FixMul( *y, yScale );
        y++;
    }
    out->origin.x = FixMul( out->origin.x, xScale );
    out->origin.y = FixMul( out->origin.y, yScale );
    out->advance.x = FixMul( out->advance.x, xScale );
    out->advance.y = FixMul( out->advance.y, yScale );
}

/*************************************************************************
|*
|*    XOutGetCharOutlineQD()
|*
|*    Ein Zeichen eines Outlinefonts in ein Bezier-PolyPolygon umwandeln
|*    Wenn keine Umwandlung moeglich ist, wird ein leeres PolyPolygon
|*    zurueckgegeben
|*    Ersterstellung    28.11.95 KH
|*    Letzte Aenderung  28.11.95 KH
|*
*************************************************************************/

XPolyPolygon XOutGetCharOutlineQD(USHORT nChar, OutputDevice& rOut,
                                    BOOL bOptimizeSize)
{
    Font aFont = rOut.GetFont();
    short style = 0;

    if (aFont.GetWeight() > WEIGHT_MEDIUM)
        style |= bold;
    if (aFont.GetItalic() > ITALIC_OBLIQUE)
        style |= italic;
    if (aFont.GetUnderline() > UNDERLINE_NONE)
        style |= underline;
    if (aFont.IsOutline())
        style |= outline;
    if (aFont.IsShadow())
        style |= shadow;

    Handle hSfnt = GetNamedSfntHandle( aFont.GetName(), style);

    if (!hSfnt && style) // Font liegt vielleicht nur in der Grundform vor ?
        hSfnt = GetNamedSfntHandle( aFont.GetName(), 0);

    if (!hSfnt)
        return XPolyPolygon();

    FontMetric aMetric = rOut.GetFontMetric();

    long nBaseLineOffset = aMetric.GetAscent();
    long nOrgHeight = aFont.GetSize().Height();

    unsigned nState = HGetState( hSfnt );
    HNoPurge( hSfnt );

    GlyphOutline out;

    int i,j;

    InitGlyphOutline( &out );

    // Index des Glyphes besorgen
    long glyphIndex = GetCharGlyphIndex( hSfnt, nChar );

    // Glyph in Outline umwandlen
    GetGlyphOutline( hSfnt, glyphIndex, &out, 0 );
    ScaleGlyphOutline( &out, ff(nOrgHeight), ff(nOrgHeight) );

    gxPaths* pPathData = OutlineToPaths( &out );

    // Im Outline steht jetzt wieviele contouren wir haben
    // so groß muuss alos unser Ergebnis sein
    XPolyPolygon aXPolyPoly( out.contourCount );	// Unser XPolyPolygon

    long* pCurPath = ((long*)pPathData) + 1;	// pCurPath zeigt jetzt auf das erste
                                                // Polygon ( kommt gleich hinter der
                                                // Anzahl der Path Objekte )

    for ( i=0; i < out.contourCount; i++ )
    {
        /* fuer jeden einzelnen Path brauchen wir die Anzahl der darin
         * enthaltenen Punkte, einen Zeiger auf das Bitfeld, das festlegt,
         * ob ein Punkt auf der Kurve liegt oder nicht, und einen Zeiger
         * auf die einzelnen Punkte. Da wir hier auch wieder eine dynamische
         * Datenstruktur haben, wirds wieder undurchsichtig.
         * pPoint wird zunaechst auf den ersten Punkt gesetzt. ( Wir haben
         * ein long fuer die Anzahl und ( Anzahl +31 )/ 32 longs fuer das
         * Bitfeld.
         */
        long		nPointAnz	= ( (gxPath*) pCurPath )->vectors;
        char		*pBits		= (char*) ( (gxPath*) pCurPath )->controlBits;
        USHORT		nPnt;
        XPolygon	aXPoly( 100, 50 );
        BOOL		bCloseIt = TRUE;

        gxPoint	 	*pPoint;

        pPoint	= (gxPoint*) ( pCurPath + ( 1 + (nPointAnz+31) / 32 ) );
        nPnt	= 0;

        for ( j=0; j< nPointAnz; j++ )
        {
            /* Die einzelnen Punkte werden von gxPoints in StarView Points
             * umgewandelt und in das XPolygon eingetragen. Anschliessend
             * wird geprueft, ob es sich um einen Off Curve point handlete.
             * (gluecklicherweise bietet Apple eine Funktion an, mit der man
             * die Bits in einem dynamischen Bitfeld testen kann (BitTst) ).
             * Zum Schluss muss jetzt nur noch der Punktpointer eins weiter
             * gesetzt werden.
             */

            Point aPoint((long) FixedToInt( pPoint->x ),(long) FixedToInt( pPoint->y ));
            aPoint.Y() += nBaseLineOffset;

            aXPoly[nPnt] = aPoint;
            nPnt++;

            /* Wenn Off Curve Points ins Spiel kommen, wirds leider etwas
             * komplizierter
             */
            if ( BitTst( pBits, j ) )
            {
                /* Es gibt wohl wirklich polygone, die mit einem Off
                 * Curve Point anfangen.
                 */
                if ( j==0 )
                {
                    gxPoint *pTmpPt = pPoint + ( nPointAnz - 1);
                    aXPoly[nPnt] = aXPoly[nPnt-1];
                    nPnt++;

                    Point aPoint((long) FixedToInt( pTmpPt->x ),(long) FixedToInt( pTmpPt->y ));
                    aPoint.Y() += nBaseLineOffset;

                    aXPoly[0] = aPoint;
                    bCloseIt = FALSE;
                }

                // Punkt verdoppeln fuer Bezier-Wandlung
                aXPoly[nPnt] = aXPoly[nPnt-1];
                nPnt++;

                /* Wenn Polygone mit Off Curve Points anfangen, dann hoeren
                 * sie sicherlich auch mal damit auf
                 */
                if ( j == nPointAnz - 1 )
                {
                    aXPoly[nPnt] = aXPoly[0];
                    nPnt++;
                    bCloseIt = FALSE;
                }
                else if ( ! BitTst( pBits, j+1 ) )
                {
                    /* Wenn der naechste Punkt auf der Kurve liegt, dann
                     * braucht er nur in unser Polygon eingetragen zu werden
                     */
                    pPoint += 1;
                    Point aPoint((long) FixedToInt( pPoint->x ),(long) FixedToInt( pPoint->y ));
                    aPoint.Y() += nBaseLineOffset;

                    aXPoly[nPnt] = aPoint;
                    nPnt++;
                    j++;
                }
                else
                {
                    /* ansonsten muessen wir noch einen weiteren Punkt
                     * mit aufnehmen. ( Und zwar den Mittelpunkt zwischen
                     * den beiden Kontrollpunkten. )
                     */
                    Point aPoint((long) FixedToInt( pPoint->x / 2 + (pPoint+1)->x / 2 ),
                                 (long) FixedToInt( pPoint->y / 2 + (pPoint+1)->y / 2 ));

                    aPoint.Y() += nBaseLineOffset;
                    aXPoly[nPnt] = aPoint;
                    nPnt++;
                }
                /* Umrechnung in Bezier (PQ=TrueType-Controlpunkt):
                 * P1 = 1/3 * (P0 + 2 * PQ)
                 * P2 = 1/3 * (P3 + 2 * PQ)
                 */
                aXPoly[nPnt-3] = (aXPoly[nPnt-4] + aXPoly[nPnt-3] * 2) / 3;
                aXPoly[nPnt-2] = (aXPoly[nPnt-1] + aXPoly[nPnt-2] * 2) / 3;
                aXPoly.SetFlags(nPnt-3, XPOLY_CONTROL);
                aXPoly.SetFlags(nPnt-2, XPOLY_CONTROL);
            }
            pPoint += 1;
        }

        // Polygon schliessen
        if ( bCloseIt )
            aXPoly[nPnt] = aXPoly[0];


        /* Nachdem die einzelnen Punkte in das XPolygon eingetragen wurden,
         * wird jetzt das Xpolygon in das XPolyPolygon gestopft und danach
         * geloescht.
         * Unser pPoint Pointer zeigt jetzt nicht mehr auf einen Punkt sondern
         * auf den Anfang des naechsten Path. Das nutzen wir natuerlich aus, um
         * den Path Pointer umzusetzen.
         */
        aXPolyPoly.Insert( aXPoly );
        aXPoly.SetPointCount(0);

        pCurPath = (long*) pPoint;
    }

    SvMemFree(pPathData);
    KillGlyphOutline( &out );

    HSetState( hSfnt, nState );

    return aXPolyPoly;
}

/*************************************************************************
|*
|*    XOutGetCharOutline()
|*
|*    Ein Zeichen eines Outlinefonts in ein Bezier-PolyPolygon umwandeln
|*    Wenn keine Umwandlung moeglich ist, wird ein leeres PolyPolygon
|*    zurueckgegeben
|*    Ersterstellung    03.02.95 DV
|*    Letzte Aenderung  28.11.95 KH
|*
*************************************************************************/

XPolyPolygon XOutGetCharOutline(USHORT nChar, OutputDevice& rOut,
                                BOOL bOptimizeSize)
{
    static BOOL		bInit = FALSE;
    static BOOL		bHasGX;

    if ( !bInit )
    {
        bHasGX = HasGX();
        bInit = TRUE;
    }

    if ( bHasGX )
        return XOutGetCharOutlineGX(nChar,rOut,bOptimizeSize);
    else
        return XOutGetCharOutlineQD(nChar,rOut,bOptimizeSize);
}

/*************************************************************************
|*
|*    XOutputDevice::DrawFormText(String, Polygon, Font, nAbsStart)
|*
|*    Einen String entlang eines Polygons ausgeben; nAbsStart
|*    ueberschreibt den XFormTextStartItem-Wert und wird fuer die
|*    Ausgabe mehrerer Strings entlang des gleichen Polygons benoetigt.
|*    Rueckgabewert ist die Endposition des ausgegebenen Textes in
|*    Bezug auf den Linienanfang.
|*
|*    nAbsStart <  0: Die Gesamtlaenge aller Strings, notwendig fuer
|*                    alle Formatierungen ausser Linksbuendig
|*              <= 0: Item-Startwert verwenden
|*              >  0: Text an dieser absoluten Position ausgeben; ist
|*                    normalerweise der Rueckgabewert eines vorigen
|*                    DrawFormText-Aufrufs
|*
|*    bToLastPoint: alle Linien einschliesslich der letzten Zeichnen,
|*                  sonst die letzte Linie auslassen
|*
|*    bDraw: wenn FALSE, wird nichts ausgegeben, sondern nur das BoundRect
|*           berechnet
|*
|*	  pDXArray: wenn vorhanden, enthaelt dieses Array die horizontalen
|*              Positionen der einzelnen Zeichen, beginnend beim ersten
|*              und endend hinter dem letzten Zeichen; es muﬂ also
|*              rText.Len()-1 long-Werte enthalten
|*
|*
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  11.10.95 ESO
|*
*************************************************************************/

long XOutputDevice::ImpDrawFormText(const String& rText, const Polygon& rPoly,
                                    Font aFont, long nAbsStart, BOOL bIsShadow,
                                    BOOL bToLastPoint, BOOL bDraw,
                                    const long* pDXArray)
{
    long	nXMin = LONG_MAX, nYMin = LONG_MAX;
    long	nXMax = LONG_MIN, nYMax = LONG_MIN;
    long	nMaxCharExtent;
    long	nPolyLen = 0;
    long	nTextLen = 0;
    long	nTotal = 0;
    long	nTextWidth;
    long	nStart;
    USHORT	nPntCnt = rPoly.GetSize();
    USHORT	nLastPnt;
    USHORT	nPnt;
    USHORT	nChar = 0;
    USHORT	nCharCnt = rText.Len();
    short	nDirection;

    if ( nPntCnt < 2 || nCharCnt == 0 )
        return 0;

    BOOL bIsSlantShadow = ( bIsShadow && eFormTextShadow == XFTSHADOW_SLANT );
    BOOL bDrawAsPoly = ( bFormTextOutline || bIsSlantShadow );

    if ( nAbsStart > 0 )	nStart = nAbsStart;
    else					nStart = nFormTextStart;

    aFont.SetTransparent(TRUE);

    // Attribute sichern
    XLineStyle eOldLineStyle = eLineStyle;
    XFillStyle eOldFillStyle = eFillStyle;
    Color aOldLineColor( pOut->GetLineColor() );

    pOut->SetFillColor( aFont.GetColor() );

    if ( !(bFormTextOutline && bHair) || bIsShadow )
        pOut->SetLineColor();

    if ( !bFormTextOutline || bIsShadow )
        eLineStyle = XLINE_NONE;

    eFillStyle = XFILL_SOLID;

    if ( eFormTextAdjust == XFT_AUTOSIZE && nAbsStart > 0 )
        aFont.SetSize(Size(0, nFtAutoHeightSave));

    Font aOldFont	= pOut->GetFont();
    long nAscent	= pOut->GetFontMetric().GetAscent();

    pOut->SetFont(aFont);

    if ( pDXArray )	nTextWidth = pDXArray[nCharCnt - 1];
    else			nTextWidth = pOut->GetTextWidth(rText);

    if ( eFormTextAdjust != XFT_LEFT && nAbsStart <= 0 )
    {
        // Gesamtlaenge des Polygons berechnen
        USHORT nMax = nPntCnt;
        nPnt = 1;

        if ( !bToLastPoint )
            nMax--;

        for ( ; nPnt < nMax; nPnt++)
        {
            double	fDx = rPoly[nPnt].X() - rPoly[nPnt-1].X();
            double	fDy = rPoly[nPnt].Y() - rPoly[nPnt-1].Y();
            nPolyLen += (long) (sqrt(fDx * fDx + fDy * fDy) + 0.5);
        }

        if ( nAbsStart == 0 )
            nAbsStart = - nTextWidth;

        if ( eFormTextAdjust == XFT_AUTOSIZE )
        {
            aFont.SetSize(Size(0, nPolyLen * aFont.GetSize().Height() /
                                - nAbsStart));
            nAbsStart = - nPolyLen;
            pOut->SetFont(aFont);
            nTextWidth = pOut->GetTextWidth(rText);
            nAscent = pOut->GetFontMetric().GetAscent();
            nFtAutoHeightSave = aFont.GetSize().Height();
            // Nach Aenderung der Fontsize ist der Arrayinhalt ungueltig
            pDXArray = NULL;
        }
        // Bei rechtsbuendiger Ausgabe auch Startposition beruecksichtigen
        if ( eFormTextAdjust == XFT_RIGHT )
            nAbsStart -= nStart;
        nStart = nPolyLen + nAbsStart;

        if ( eFormTextAdjust != XFT_RIGHT )
            nStart /= 2;
    }
    if ( nStart < 0 )
        nStart = 0;

    if ( bIsShadow && eFormTextShadow != XFTSHADOW_SLANT )
        nStart += nFormTextShdwXVal;

    nTotal = nStart + nTextWidth;

    if ( bIsShadow && eFormTextShadow == XFTSHADOW_SLANT &&
         nFormTextShdwYVal && nFormTextShdwYVal != 100 )
        nAscent = nAscent * nFormTextShdwYVal / 100;

    // Maximaler Zeichenbereich ca. 1,4 (sqrt(2)) * Ascent fuer BoundRect
    nMaxCharExtent = nAscent * 7 / 5;

    // Laufrichtung des Polygons ggf. spiegeln
    if ( bFormTextMirror )
    {
        nDirection = -1;
        nLastPnt = (USHORT) -1;
        nPnt = nPntCnt - 2;
        if ( !bToLastPoint )
            nPnt--;
    }
    else
    {
        nDirection = 1;
        nLastPnt = nPntCnt - 1;
        nPnt = 1;
        if ( bToLastPoint )
            nLastPnt++;
    }

    while ( nChar < nCharCnt && nPnt != nLastPnt )
    {
        Point	aPos = rPoly[nPnt];
        double	fDx, fDy, fLen;
        long	nLen, nChar1Len;

        fDx = aPos.X() - rPoly[nPnt-nDirection].X();
        fDy = aPos.Y() - rPoly[nPnt-nDirection].Y();
        fLen = sqrt(fDx * fDx + fDy * fDy);
        nLen = (long) (fLen + 0.5);

        if ( pDXArray )
        {
            nChar1Len = pDXArray[nChar];
            if ( nChar > 0 )
                nChar1Len -= pDXArray[nChar-1];
        }
        else
            nChar1Len = pOut->GetTextWidth(rText, nChar, 1);

        double fXDist, fYDist;
        double fCos = fDx;
        double fSin = fDy;
        double fPartLen = fLen;
        double fLenSum = fLen - nTextLen;
        USHORT nSumPnt = nPnt + nDirection;

        // Laenge halbieren, um Zeichenmitte als Referenzpunkt zu verwenden
        nChar1Len /= 2;

        while ( nSumPnt != nLastPnt && fLenSum < nChar1Len )
        {
            fCos = rPoly[nSumPnt].X() - rPoly[nSumPnt-nDirection].X();
            fSin = rPoly[nSumPnt].Y() - rPoly[nSumPnt-nDirection].Y();
            fPartLen = sqrt(fSin * fSin + fCos * fCos);
            fLenSum += fPartLen;
            nSumPnt += nDirection;
        }
        fXDist = - fSin * nFormTextDistance / fPartLen;
        fYDist =   fCos * nFormTextDistance / fPartLen;

        fSin /= - fPartLen;
        fCos /=   fPartLen;

        if ( nStart > 0 )
            nStart -= nLen;
        else
        {
            nLen -= nTextLen;
            nTextLen = - nLen;
        }

        if ( nLen > 0 && nStart <= 0 )
        {
            USHORT nCnt = 0;

            if ( nStart < 0 )
            {
                nLen = - nStart;
                nStart = 0;
            }
            do
            {
                if ( pDXArray )
                {
                    nTextLen = pDXArray[nChar + nCnt];
                    if ( nChar > 0 )
                        nTextLen -= pDXArray[nChar-1];
                }
                else
                    nTextLen = pOut->GetTextWidth(rText, nChar, nCnt+1);
                nCnt++;
            }
            while ( nChar + nCnt < nCharCnt && nTextLen < nLen );

            aPos.X() -= (long) (fDx * nLen / fLen + fXDist);
            aPos.Y() -= (long) (fDy * nLen / fLen + fYDist);
//			aPos.X() -= (long) ((fDx * nLen - fDy * nFormTextDistance) / fLen);
//			aPos.Y() -= (long) ((fDy * nLen + fDx * nFormTextDistance) / fLen);

            // BoundRect-Approximation
            nXMin = Min(nXMin, aPos.X() - nMaxCharExtent);
            nYMin = Min(nYMin, aPos.Y() - nMaxCharExtent);
            nXMax = Max(nXMax, aPos.X() + nMaxCharExtent);
            nYMax = Max(nYMax, aPos.Y() + nMaxCharExtent);

            if ( eFormTextStyle == XFT_ROTATE )
            {
                if ( bDrawAsPoly )
                {
                    for (USHORT i = 0; i < nCnt; i++)
                    {
                        XPolyPolygon aChar = XOutGetCharOutline(
                                    (BYTE) rText[USHORT(nChar+i)], *pOut);
                        Point aPolyPos = aPos;

                        if ( i > 0 )
                        {
                            long nW;

                            if ( pDXArray )
                            {
                                nW = pDXArray[nChar + i - 1];
                                if ( nChar > 0 )
                                    nW -= pDXArray[nChar-1];
                            }
                            else
                                nW = pOut->GetTextWidth(rText, nChar, i);

                            aPolyPos.X() += (long) (fDx * nW / fLen);
                            aPolyPos.Y() += (long) (fDy * nW / fLen);
                        }
                        Point aCenter = aPolyPos;

                        if ( bIsSlantShadow )
                        {
                            if ( nFormTextShdwYVal && nFormTextShdwYVal != 100 )
                                aChar.Scale(1.0, (double)nFormTextShdwYVal/100);

                            aChar.SlantX(nAscent,
                                         sin(- F_PI * nFormTextShdwXVal / 1800),
                                         cos(- F_PI * nFormTextShdwXVal / 1800));
                        }
                        aPolyPos.Y() -= nAscent;
                        aChar.Translate(aPolyPos);
                        aChar.Rotate(aCenter, fSin, fCos);
                        if ( bDraw )
                            DrawXPolyPolygon(aChar);
                    }
                }
                else
                {
                    short nAngle = (short) (acos(fCos) * 1800 / F_PI + 0.5);
                    if ( fSin < 0 )
                        nAngle = 3600 - nAngle;
                    aFont.SetOrientation(nAngle);
                    pOut->SetFont(aFont);
                    if ( bDraw )
                        pOut->DrawTextArray(aPos, rText, pDXArray, nChar, nCnt);
//						pOut->DrawText(aPos, rText, nChar, nCnt);
                }
            }
            else
            {
                bDrawAsPoly = ( bDrawAsPoly || eFormTextStyle != XFT_UPRIGHT);

                for (USHORT i = 0; i < nCnt; i++)
                {
                    XPolyPolygon aChar(0);
                    Point aPolyPos = aPos;

                    if ( i > 0 )
                    {
                        long nW;

                        if ( pDXArray )
                        {
                            nW = pDXArray[nChar + i - 1];
                            if ( nChar > 0 )
                                nW -= pDXArray[nChar-1];
                        }
                        else
                            nW = pOut->GetTextWidth(rText, nChar, i);

                        aPolyPos.X() += (long) (fDx * nW / fLen);
                        aPolyPos.Y() += (long) (fDy * nW / fLen);
                    }
                    if ( bDrawAsPoly )
                    {
                        aChar = XOutGetCharOutline(
                                        (BYTE)rText[USHORT(nChar+i)], *pOut);

                        if ( bIsSlantShadow )
                        {
                            if ( nFormTextShdwYVal && nFormTextShdwYVal != 100 )
                                aChar.Scale(1.0, (double)nFormTextShdwYVal/100);

                            aChar.SlantX(nAscent,
                                         sin(- F_PI * nFormTextShdwXVal / 1800),
                                         cos(- F_PI * nFormTextShdwXVal / 1800));
                        }
                    }

                    if ( eFormTextStyle == XFT_SLANTY )
                    {
                        aPolyPos.Y() -= nAscent;
                        aChar.SlantY(0, fSin, fCos);
                    }
                    else
                    {
                        long nW;

                        if ( pDXArray )
                        {
                            nW = pDXArray[nChar + i];
                            if ( nChar > 0 || i > 0 )
                                nW -= pDXArray[nChar + i - 1];
                        }
                        else
                            nW = pOut->GetTextWidth(rText, nChar+i, 1);

                        aPolyPos.X() -= nW / 2;

                        if ( eFormTextStyle == XFT_SLANTX )
                        {
                            aPolyPos.X() += (long) (fDy * nAscent / fLen);
                            aPolyPos.Y() -= (long) (fDx * nAscent / fLen);
                            aChar.SlantX(0, fSin, fCos);
                        }
                        else if ( bDrawAsPoly )
                            aPolyPos.Y() -= nAscent;
                    }
                    aChar.Translate(aPolyPos);

                    if ( bDraw )
                    {
                        if ( bDrawAsPoly )
                            DrawXPolyPolygon(aChar);
                        else
                            pOut->DrawTextArray(aPolyPos, rText, pDXArray,
                                                nChar+i, 1);
//							pOut->DrawText(aPolyPos, rText, nChar+i, 1);
                    }
                }
            }
            nChar += nCnt;
            nTextLen -= nLen;
        }
        nPnt += nDirection;
    }
    // auch letzten Punkt fuer BoundRect-Approximation beruecksichtigen
    Point aPos = rPoly[nPnt - nDirection];
    nXMin = Min(nXMin, aPos.X() - nMaxCharExtent);
    nYMin = Min(nYMin, aPos.Y() - nMaxCharExtent);
    nXMax = Max(nXMax, aPos.X() + nMaxCharExtent);
    nYMax = Max(nYMax, aPos.Y() + nMaxCharExtent);

    aFormTextBoundRect.Union(Rectangle(nXMin, nYMin, nXMax, nYMax));

    pOut->SetFont(aOldFont);
    pOut->SetLineColor( aOldLineColor );
    eLineStyle = eOldLineStyle;
    eFillStyle = eOldFillStyle;

    return nTotal;
}

/*************************************************************************
|*
|*    XOutputDevice::DrawFormTextShadow(String, Polygon, Font, nAbsStart)
|*
|*    Schatten fuer FormText ausgeben
|*
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  11.10.95 ESO
|*
*************************************************************************/

void XOutputDevice::DrawFormTextShadow(const String& rText,
                                       const Polygon& rPoly,
                                       const Font& rFont, long nAbsStart,
                                       BOOL bToLastPoint, BOOL bDraw,
                                       const long* pDXArray)
{
    if ( eFormTextShadow != XFTSHADOW_NONE )
    {
        BOOL bOutline = bFormTextOutline;

        if ( eFormTextShadow != XFTSHADOW_SLANT )
            nFormTextDistance += nFormTextShdwYVal;

        Font aShadowFont(rFont);
        aShadowFont.SetColor(aFormTextShdwColor);

        ImpDrawFormText(rText, rPoly, aShadowFont, nAbsStart, TRUE,
                        bToLastPoint, bDraw, pDXArray);

        if ( eFormTextShadow != XFTSHADOW_SLANT )
            nFormTextDistance -= nFormTextShdwYVal;
        bFormTextOutline = bOutline;
    }
}

/*************************************************************************
|*
|*    XOutputDevice::DrawFormText(String, Polygon, Font, nAbsStart)
|*
|*    Einen String entlang eines Polygons ausgeben
|*
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  11.10.95 ESO
|*
*************************************************************************/

long XOutputDevice::DrawFormText(const String& rText, const Polygon& rPoly,
                                 Font aFont, long nAbsStart,
                                 BOOL bToLastPoint, BOOL bDraw,
                                 const long* pDXArray)
{
    // Beim erstn Aufruf BoundRect leeren
    if ( nAbsStart <= 0 )
        aFormTextBoundRect = Rectangle();

    DrawFormTextShadow(rText, rPoly, aFont, nAbsStart, bToLastPoint, bDraw,
                        pDXArray);
    return ImpDrawFormText(rText, rPoly, aFont, nAbsStart, FALSE,
                           bToLastPoint, bDraw, pDXArray);
}

/*************************************************************************
|*
|*    XOutputDevice::DrawFormText(String, XPolygon, Font, nAbsStart)
|*
|*    Einen String entlang eines Bezier-Polygons ausgeben
|*
|*    Ersterstellung    03.02.95 ESO
|*    Letzte Aenderung  11.10.95 ESO
|*
*************************************************************************/

long XOutputDevice::DrawFormText(const String& rText, const XPolygon& rXPoly,
                                 Font aFont, long nAbsStart,
                                 BOOL bToLastPoint, BOOL bDraw,
                                 const long* pDXArray)
{
    // Beim ersten Aufruf BoundRect leeren
    if ( nAbsStart <= 0 )
        aFormTextBoundRect = Rectangle();

    Polygon aPoly = XOutCreatePolygon(rXPoly, pOut);

    DrawFormTextShadow(rText, aPoly, aFont, nAbsStart, bToLastPoint, bDraw,
                        pDXArray);
    return ImpDrawFormText(rText, aPoly, aFont, nAbsStart, FALSE,
                           bToLastPoint, bDraw, pDXArray);
}

}
