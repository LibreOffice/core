/*************************************************************************
 *
 *  $RCSfile: drawing.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:34:48 $
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
 *  Copyright 2000 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2000 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DRAWING_H_
#define _DRAWING_H_

/* NAME $Id: drawing.h,v 1.1 2003-10-15 14:34:48 dvo Exp $
 *
 * NOTES
 *
 * HISTORY
 *   frog - Jan 11, 1999: Created.
 *   frog - Nov 20, 2000: remove display function, only needed loading object from binary
 */
#include "precompile.h"

#include <math.h>

#include "hwplib.h"
#include "hwpfile.h"
#include "hiodev.h"
#include "hbox.h"
#include "drawdef.h"

enum
{
    OBJFUNC_LOAD,
    OBJFUNC_FREE,
    OBJFUNC_DISPLAY,
    OBJFUNC_NITEM
};

enum
{
    BEGIN_GRADATION = 0, LINEAR, RADIAL, CONICAL, SQUARE,
    END_GRADATION, BITMAP_PATTERN
};

#define OBJRET_FILE_OK           0
#define OBJRET_FILE_ERROR       (-1)
#define OBJRET_FILE_NO_PRIVATE_BLOCK    (-2)
#define OBJRET_FILE_NO_PRIVATE_BLOCK_2  (-3)

typedef int (*HWPDOFuncType) (int, HWPDrawingObject *, int, void *, int);

#define HWPDOFunc(hdo, cmd, argp, argv) \
    (HWPDOFuncTbl[(hdo)->type]((hdo)->type, (hdo), (cmd), (argp), (argv)))
#define HWPDOFunc2(type, cmd, argp, argv) \
    (HWPDOFuncTbl[(type)]((type), NULL, (cmd), (argp), (argv)))

static int HWPDOLineFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDORectFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDOEllipseFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDOArcFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDOFreeFormFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDOTextBoxFunc(int, HWPDrawingObject *, int, void *, int);
static int HWPDOEllipse2Func(int, HWPDrawingObject *, int, void *, int);
static int HWPDOArc2Func(int, HWPDrawingObject *, int, void *, int);
static int HWPDOContainerFunc(int, HWPDrawingObject *, int, void *, int);
static HWPPara *LoadParaList();

HWPDOFuncType HWPDOFuncTbl[] =
{
    HWPDOContainerFunc,
    HWPDOLineFunc,
    HWPDORectFunc,
    HWPDOEllipseFunc,
    HWPDOArcFunc,
    HWPDOFreeFormFunc,
    HWPDOTextBoxFunc,
    HWPDOFreeFormFunc,
    HWPDOEllipse2Func,
    HWPDOArc2Func,
    HWPDOFreeFormFunc,
};

static HMemIODev *hmem = 0;

static int count = 0;

inline bool HAVE_FCOLOR(HWPDrawingObject * hdo)
{
    return hdo->property.fill_color != HWPDO_COLOR_NONE;
}


inline bool HAVE_PATTERN(HWPDrawingObject * hdo)
{
    return (hdo->property.pattern_type & HWPDO_PAT_TYPE_BITS)
        != HWPDO_PAT_SOLID && hdo->property.pattern_color != HWPDO_COLOR_NONE;
}


inline bool HAVE_GRADATION(HWPDrawingObject * hdo)
{
    return hdo->property.gstyle > BEGIN_GRADATION &&
        hdo->property.gstyle < END_GRADATION &&
        hdo->property.fromcolor != HWPDO_COLOR_NONE &&
        hdo->property.tocolor != HWPDO_COLOR_NONE;
}


inline bool HAVE_BITMAP_PATTERN(HWPDrawingObject * hdo)
{
    return hdo->property.gstyle == BITMAP_PATTERN &&
        hdo->property.szPatternFile[0];
}


inline bool HAS_PAT(HWPDrawingObject * hdo)
{
    return HAVE_FCOLOR(hdo) || HAVE_PATTERN(hdo) ||
        HAVE_GRADATION(hdo) || HAVE_BITMAP_PATTERN(hdo);
}


static void SetHdoParallRgn(HWPDrawingObject * hdo, int width, int height)
{
    hdo->property.parall.pt[0].x = 0;
    hdo->property.parall.pt[0].y = 0;
    hdo->property.parall.pt[1].x = width;
    hdo->property.parall.pt[1].y = 0;
    hdo->property.parall.pt[2].x = width;
    hdo->property.parall.pt[2].y = height;
}


static bool SkipPrivateBlock(int type)
{
    int n;

    if (type == OBJRET_FILE_NO_PRIVATE_BLOCK)
    {
        n = hmem->read4b();
        if (hmem->state() || hmem->skipBlock(n) != n)
            return false;
    }
    n = hmem->read4b();
    if (hmem->state())
        return false;
    return hmem->skipBlock(n) == n;
}


static int SizeExpected;
static int SizeRead;

static int ReadSizeField(int size)
{
    SizeExpected = size;
    SizeRead = hmem->read4b();
    if (hmem->state())
        return -1;
    return SizeRead;
}


static bool SkipUnusedField(void)
{
    return (SizeExpected < SizeRead) ?
        hmem->skipBlock(SizeRead - SizeExpected) != 0 : true;
}


#define HDOFILE_HEADER_SIZE (2*4+16)              // 16=sizeof(ZZRect)
#define HDOFILE_COMMON_SIZE (7*4+16+44)

#define HDOFILE_HAS_NEXT    0x01
#define HDOFILE_HAS_CHILD   0x02

/**
 * 공통 헤더를 읽어드린다.
 * 개체종류/연결정보/상대위치/개체크기/절대위치/차지영역/기본속성/회전속성/그라데이션/비트맵패턴
 */
static bool LoadCommonHeader(HWPDrawingObject * hdo, WORD * link_info)
{
    uint size, property_size, common_size;

     if( !hmem )
         return FALSE;
    size = hmem->read4b();
    if (hmem->state())
    {
        return FALSE;
    }
    if (size < HDOFILE_COMMON_SIZE)
    {
        return FALSE;
    }

    common_size = HDOFILE_COMMON_SIZE;
    property_size = 44;
    hdo->type = hmem->read2b();
    *link_info = hmem->read2b();
    hdo->offset.x = hmem->read4b();
    hdo->offset.y = hmem->read4b();
    hdo->extent.w = hmem->read4b();
    hdo->extent.h = hmem->read4b();
    hdo->offset2.x = hmem->read4b();
    hdo->offset2.y = hmem->read4b();

    if (hmem->state())
        return FALSE;

    hdo->vrect.x = hmem->read4b();
    hdo->vrect.y = hmem->read4b();
    hdo->vrect.w = hmem->read4b();
    hdo->vrect.h = hmem->read4b();

// read bare property 44 bytes
    hdo->property.line_pstyle = hmem->read4b();
    hdo->property.line_hstyle = hmem->read4b();
    hdo->property.line_tstyle = hmem->read4b();
    hdo->property.line_color = hmem->read4b();
    hdo->property.line_width = (hunit) hmem->read4b();
    hdo->property.fill_color = hmem->read4b();
    hdo->property.pattern_type = hmem->read4b();
    hdo->property.pattern_color = hmem->read4b();
    hdo->property.hmargin = (hunit) hmem->read4b();
    hdo->property.vmargin = (hunit) hmem->read4b();
    hdo->property.flag = hmem->read4b();
// read ratation property 32 bytes
    if ((size >= common_size + 32)
        && (hdo->property.flag & HWPDO_FLAG_ROTATION))
    {
        hdo->property.rot_originx = hmem->read4b();
        hdo->property.rot_originy = hmem->read4b();
        for (int ii = 0; ii < 3; ii++)
        {
            hdo->property.parall.pt[ii].x = hmem->read4b();
            hdo->property.parall.pt[ii].y = hmem->read4b();
        }
        common_size += 32;
    }
    else
        SetHdoParallRgn(hdo, hdo->extent.w, hdo->extent.h);

// read gradient property 28 bytes
    if ((size >= common_size + 28) &&
        (hdo->property.flag & HWPDO_FLAG_GRADATION))
    {
        hdo->property.fromcolor = hmem->read4b();
        hdo->property.tocolor = hmem->read4b();
        hdo->property.gstyle = hmem->read4b();
        hdo->property.angle = hmem->read4b();
        hdo->property.center_x = hmem->read4b();
        hdo->property.center_y = hmem->read4b();
        hdo->property.nstep = hmem->read4b();
        common_size += 28;
    }

// read bitmap property 278 bytes
    if ((size >= common_size + 278) && \
        (hdo->property.flag & HWPDO_FLAG_BITMAP))
    {
        hdo->property.offset1.x = hmem->read4b();
        hdo->property.offset1.y = hmem->read4b();
        hdo->property.offset2.x = hmem->read4b();
        hdo->property.offset2.y = hmem->read4b();
        if (!hmem->readBlock(hdo->property.szPatternFile, 261))
            return FALSE;
        hdo->property.pictype = hmem->read1b();
        common_size += 278;
    }
     if( ( size >= common_size + 3 ) && ( hdo->property.flag & HWPDO_FLAG_WATERMARK ) )
     //if( ( size >= common_size ) && ( hdo->property.flag >> 20 & 0x01 ) )
     {
          if( size - common_size >= 5 )
              hmem->skipBlock( 2 );
         hdo->property.luminance = hmem->read1b();
         hdo->property.contrast = hmem->read1b();
         hdo->property.greyscale = hmem->read1b();
         common_size += 5;
     }
    else{
         hdo->property.luminance = 0;
         hdo->property.contrast = 0;
         hdo->property.greyscale = 0;
    }
     hdo->property.pPara = 0L;

     if( ( size > common_size ) && (hdo->property.flag & HWPDO_FLAG_AS_TEXTBOX) )
     {
          hmem->skipBlock(8);
          hdo->property.pPara = LoadParaList();
          if( hdo->property.pPara )
                return TRUE;
          else
                return FALSE;
     }

     if( size <= common_size )
          return TRUE;
     return hmem->skipBlock(size - common_size ) != 0;
}


static HWPDrawingObject *LoadDrawingObject(void)
{
    HWPDrawingObject *hdo, *head, *prev;
    int res;

    WORD link_info;

    head = prev = NULL;
    do
    {
        if ((hdo = new HWPDrawingObject) == NULL)
        {
            goto error;
        }
        if (!LoadCommonHeader(hdo, &link_info))
        {
            goto error;
        }
        if (hdo->type < 0 || hdo->type >= HWPDO_NITEMS)
        {
            hdo->type = HWPDO_RECT;
            if (!SkipPrivateBlock(OBJRET_FILE_NO_PRIVATE_BLOCK))
            {
                goto error;
            }
        }
        else
        {
            switch (res = HWPDOFunc(hdo, OBJFUNC_LOAD, NULL, 0))
            {
                case OBJRET_FILE_ERROR:
                    goto error;
                case OBJRET_FILE_OK:
                    break;
                case OBJRET_FILE_NO_PRIVATE_BLOCK:
                case OBJRET_FILE_NO_PRIVATE_BLOCK_2:
                    if (!SkipPrivateBlock(res))
                        goto error;
                    break;
            }
        }
        if (link_info & HDOFILE_HAS_CHILD)
        {
            hdo->child = LoadDrawingObject();
            if (hdo->child == NULL)
            {
            }
        }
        if (prev == NULL)
            head = hdo;
        else
            prev->next = hdo;
        prev = hdo;
    }
    while (link_info & HDOFILE_HAS_NEXT);

    return head;
    error:
// drawing object can be list.
// hdo = current item, head = list;

    if (hdo != NULL)
    {
        HWPDOFunc(hdo, OBJFUNC_FREE, NULL, 0);
        delete hdo;
    }
    if( prev )
    {
        prev->next = NULL;
        return head;
    }
    else
        return 0;
}


static bool LoadDrawingObjectBlock(Picture * pic)
{
    int size = hmem->read4b();

    if (hmem->state() || size < HDOFILE_HEADER_SIZE)
        return false;

    pic->picinfo.picdraw.zorder = hmem->read4b();
    pic->picinfo.picdraw.mbrcnt = hmem->read4b();
    pic->picinfo.picdraw.vrect.x = hmem->read4b();
    pic->picinfo.picdraw.vrect.y = hmem->read4b();
    pic->picinfo.picdraw.vrect.w = hmem->read4b();
    pic->picinfo.picdraw.vrect.h = hmem->read4b();

    if (size > HDOFILE_HEADER_SIZE &&
        !hmem->skipBlock(size - HDOFILE_HEADER_SIZE))
        return false;

    pic->picinfo.picdraw.hdo = LoadDrawingObject();
    if (pic->picinfo.picdraw.hdo == 0)
        return false;
    return true;
}


// object manipulation function

static int
HWPDODefaultFunc(int type, HWPDrawingObject * hdo, int cmd, void *, int)
{
    if (cmd == OBJFUNC_LOAD)
        return OBJRET_FILE_NO_PRIVATE_BLOCK;
    return true;
}


// arrow polygon

static void
calcArrowPolygonPts(long lWidth, ZZPoint * arrowPt,
ZZPoint * boxPt, int x1, int y1, int x2, int y2)
{
    long lLength = lWidth;
    int dx, dy;

#if 0
    if (gc->lineWidth > ONE_MILI)
        lWidth = lLength = DRPX2(gc->lineWidth) * 2;
    else
        lWidth = lLength = DRPX2(ARROW_WIDTH);
#endif

    dx = x1 - x2;
    dy = y1 - y2;

    if (dx == 0)
    {
        arrowPt[0].x = x1;
        boxPt[3].x = boxPt[0].x = arrowPt[1].x = x1 + lWidth;
        boxPt[2].x = boxPt[1].x = arrowPt[2].x = x1 - lWidth;
        if (y1 > y2)
        {
            boxPt[2].y = boxPt[3].y = y1 + lLength * 2 / 3;
            arrowPt[0].y = y1 + lLength * 3 / 2;
            boxPt[0].y = boxPt[1].y = arrowPt[1].y = arrowPt[2].y = y1 - lLength;
        }
        else
        {
            boxPt[0].y = boxPt[1].y = y1 - lLength * 2 / 3;
            arrowPt[0].y = y1 - lLength * 3 / 2;
            boxPt[2].y = boxPt[3].y = arrowPt[1].y = arrowPt[2].y = y1 + lLength;
        }
        return;
    }

    double rSlope, rRadians;
    long DX1, DY1, DX2, DY2;

    rSlope = (double) dy / (double) dx;
    rRadians = atan(rSlope);
    DX1 = (long) (lLength * cos(rRadians) + 0.5);
    DY1 = (long) (lLength * sin(rRadians) + 0.5);
    DX2 = (long) (lWidth * sin(rRadians) + 0.5);
    DY2 = (long) (lWidth * cos(rRadians) + 0.5);

    if (dx > 0)
    {
        arrowPt[0].x = (int) (x1 + cos(rRadians) * lLength * 3 / 2);
        arrowPt[0].y = (int) (y1 + sin(rRadians) * lLength * 3 / 2);
        boxPt[0].x = arrowPt[1].x = x1 - DX1 - DX2;
        boxPt[0].y = arrowPt[1].y = y1 - DY1 + DY2;
        boxPt[1].x = arrowPt[2].x = x1 - DX1 + DX2;
        boxPt[1].y = arrowPt[2].y = y1 - DY1 - DY2;
        boxPt[2].x = arrowPt[0].x - DX1 + DX2;
        boxPt[2].y = arrowPt[0].y - DY1 - DY2;
        boxPt[3].x = arrowPt[0].x - DX1 - DX2;
        boxPt[3].y = arrowPt[0].y - DY1 + DY2;
    }
    else
    {
        arrowPt[0].x = (int) (x1 - cos(rRadians) * lLength * 3 / 2);
        arrowPt[0].y = (int) (y1 - sin(rRadians) * lLength * 3 / 2);
        boxPt[0].x = arrowPt[1].x = x1 + DX1 - DX2;
        boxPt[0].y = arrowPt[1].y = y1 + DY1 + DY2;
        boxPt[1].x = arrowPt[2].x = x1 + DX1 + DX2;
        boxPt[1].y = arrowPt[2].y = y1 + DY1 - DY2;
        boxPt[3].x = arrowPt[0].x + DX1 - DX2;
        boxPt[3].y = arrowPt[0].y + DY1 + DY2;
        boxPt[2].x = arrowPt[0].x + DX1 + DX2;
        boxPt[2].y = arrowPt[0].y + DY1 - DY2;
    }
}


static int
HWPDOLineFunc(int type, HWPDrawingObject * hdo, int cmd, void *argp, int argv)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            hdo->u.line_arc.flip = hmem->read4b();
            if (hmem->state())
                return OBJRET_FILE_ERROR;
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;
            return OBJRET_FILE_NO_PRIVATE_BLOCK_2;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


// rectangle

static int
HWPDORectFunc(int type, HWPDrawingObject * hdo, int cmd, void *argp, int argv)
{
    return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
}


// ellipse

static int
HWPDOEllipseFunc(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
    return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
}

#define WTMM(x)     ((double)(x) / 1800. * 25.4)
static int
HWPDOEllipse2Func(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(16) < 16)
                return OBJRET_FILE_ERROR;
            hdo->u.arc.radial[0].x = hmem->read4b();
            hdo->u.arc.radial[0].y = hmem->read4b();
            hdo->u.arc.radial[1].x = hmem->read4b();
            hdo->u.arc.radial[1].y = hmem->read4b();

            if (ReadSizeField(0) < 0)
                return OBJRET_FILE_ERROR;
            break;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


// arc

static int
HWPDOArcFunc(int type, HWPDrawingObject * hdo, int cmd, void *argp, int argv)
{
// TRACE("arc");
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            hdo->u.line_arc.flip = hmem->read4b();
            if (hmem->state())
                return OBJRET_FILE_ERROR;
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;
            break;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


static int
HWPDOArc2Func(int type, HWPDrawingObject * hdo, int cmd, void *argp, int argv)
{
// TRACE("arc2");
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            return OBJRET_FILE_NO_PRIVATE_BLOCK;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


// freeform

#define SPLINE_NSTEP    100
#define SPLINE_UNIT 20
#define SPLINE_UNIT2    40
#define SPLINE_UNIT3    60

static int getBlend(int alpha)
{
    static bool first = true;
    static char isCached[SPLINE_NSTEP];
    static int blend[SPLINE_NSTEP];
    double ntheta;

    if (first)
    {
        memset(isCached, 0, sizeof(char) * SPLINE_NSTEP);

        first = FALSE;
    }
    if ((alpha < -SPLINE_UNIT2) || (alpha > SPLINE_UNIT2))
        return 0;

    if (!isCached[alpha + SPLINE_UNIT2])
    {
        isCached[alpha + SPLINE_UNIT2] = TRUE;
        ntheta = (double) alpha / SPLINE_UNIT;

        if ((alpha < -SPLINE_UNIT) || (alpha > SPLINE_UNIT))
        {
            ntheta = (ntheta > 1) ? (2 - ntheta) : (2 + ntheta);
            blend[alpha + SPLINE_UNIT2] =
                (int) (1000 * ntheta * ntheta * ntheta / 6.);
        }
        else if (alpha <= 0)
            blend[alpha + SPLINE_UNIT2] =
                    (int) (1000 *
                    (4 - 6 * ntheta * ntheta -
                    3 * ntheta * ntheta * ntheta) / 6);
        else
            blend[alpha + SPLINE_UNIT2] =
                (int) (1000 *
                (4 - 6 * ntheta * ntheta +
                3 * ntheta * ntheta * ntheta) / 6);
    }
    return blend[alpha + SPLINE_UNIT2];
}


static int
HWPDOFreeFormFunc(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
        {
            hdo->u.freeform.pt = 0;
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            hdo->u.freeform.npt = hmem->read4b();
            if (hmem->state())
                return OBJRET_FILE_ERROR;
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;

            int size = hdo->u.freeform.npt * sizeof(ZZPoint);

            if (ReadSizeField(size) < size)
                return OBJRET_FILE_ERROR;
            if (hdo->u.freeform.npt)
            {
                hdo->u.freeform.pt = new ZZPoint[hdo->u.freeform.npt];
                if (hdo->u.freeform.pt == NULL)
                {
                    hdo->u.freeform.npt = 0;
                    return OBJRET_FILE_ERROR;
                }
                for (int ii = 0; ii < hdo->u.freeform.npt; ii++)
                {
                    hdo->u.freeform.pt[ii].x = hmem->read4b();
                    hdo->u.freeform.pt[ii].y = hmem->read4b();
                    if (hmem->state())
                    {
                        delete[]hdo->u.freeform.pt;
                        hdo->u.freeform.npt = 0;
                        return OBJRET_FILE_ERROR;
                    }
                }
            }
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;
            return OBJRET_FILE_OK;
        }
        case OBJFUNC_FREE:
            if (hdo->u.freeform.pt)
                delete[]hdo->u.freeform.pt;
            break;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


// text box

static void FreeParaList(HWPPara * para)
{
    if (para->Next())
        FreeParaList(para->Next());
    delete para;
}


static HWPPara *LoadParaList()
{
    if (!hmem)
        return 0;

    HWPFile *hwpf = GetCurrentDoc();
    HIODev *hio = hwpf->SetIODevice(hmem);

    LinkedList < HWPPara > plist;

    hwpf->ReadParaList(plist);
    hwpf->SetIODevice(hio);

    return plist.count()? plist.first() : 0;
}


static int
HWPDOTextBoxFunc(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
// TRACE("textbox");
    hunit sx, sy, xs, ys;

    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(0) < 0 || !SkipUnusedField())
                return OBJRET_FILE_ERROR;
            if (ReadSizeField(0) < 0)
                return OBJRET_FILE_ERROR;
            hdo->u.textbox.h = LoadParaList();
            return hdo->u.textbox.h ? OBJRET_FILE_OK : OBJRET_FILE_ERROR;
        case OBJFUNC_FREE:
            if (hdo->u.textbox.h)
            {
                FreeParaList(hdo->u.textbox.h);
                hdo->u.textbox.h = NULL;
            }
            break;
        default:
            return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
    }
    return true;
}


static int
HWPDOContainerFunc(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
    return HWPDODefaultFunc(type, hdo, cmd, argp, argv);
}


// HWPDrawObject 멤버 함수

HWPDrawingObject::HWPDrawingObject()
{
    memset(this, 0, sizeof(HWPDrawingObject));
    index = ++count;
}


HWPDrawingObject::~HWPDrawingObject()
{
#if 0
    if (hdo->property.pictype == PICTYP_EMBED)
        RemoveEmbeddedPic(hdo->property.szPatternFile);
    hdo->property.szPatternFile[0] = 0;
#endif
    if (child)
        delete child;

    if (next)
        delete next;

    HWPDOFunc(this, OBJFUNC_FREE, NULL, 0);
}
#endif
