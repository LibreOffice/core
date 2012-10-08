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

#ifndef _DRAWING_H_
#define _DRAWING_H_

#include "precompile.h"

#include <list>
#include <math.h>

#include <osl/diagnose.h>

#include <comphelper/newarray.hxx>

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

static bool LoadCommonHeader(HWPDrawingObject * hdo, WORD * link_info)
{
    uint size, common_size;

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
    hdo->type = hmem->read2b();
    *link_info = sal::static_int_cast<WORD>(hmem->read2b());
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
        hdo->property.pictype = sal::static_int_cast<char>(hmem->read1b());
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
                goto error;
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
HWPDODefaultFunc(int , HWPDrawingObject * , int cmd, void *, int)
{
    if (cmd == OBJFUNC_LOAD)
        return OBJRET_FILE_NO_PRIVATE_BLOCK;
    return true;
}


static int
HWPDOLineFunc(int type, HWPDrawingObject * hdo, int cmd, void *argp, int argv)
{
    int ret = true;
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
            ret = OBJRET_FILE_NO_PRIVATE_BLOCK_2;
            break;
        default:
            ret = HWPDODefaultFunc(type, hdo, cmd, argp, argv);
            break;
    }
    return ret;
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
    int ret = true;
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            ret = OBJRET_FILE_NO_PRIVATE_BLOCK;
            break;
        default:
            ret = HWPDODefaultFunc(type, hdo, cmd, argp, argv);
            break;
    }
    return ret;
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
                hdo->u.freeform.pt =
                    ::comphelper::newArray_null<ZZPoint>(hdo->u.freeform.npt);
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

    std::list < HWPPara* > plist;

    hwpf->ReadParaList(plist);
    hwpf->SetIODevice(hio);

    return plist.size()? plist.front() : 0;
}


static int
HWPDOTextBoxFunc(int type, HWPDrawingObject * hdo,
int cmd, void *argp, int argv)
{
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


HWPDrawingObject::HWPDrawingObject()
{
    memset(this, 0, sizeof(HWPDrawingObject));
    index = ++count;
}


HWPDrawingObject::~HWPDrawingObject()
{
    if (child)
        delete child;

    if (next)
        delete next;

    HWPDOFunc(this, OBJFUNC_FREE, NULL, 0);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
