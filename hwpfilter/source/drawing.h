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

#ifndef INCLUDED_HWPFILTER_SOURCE_DRAWING_H
#define INCLUDED_HWPFILTER_SOURCE_DRAWING_H

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

static HIODev *hmem = nullptr;

static int count = 0;

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
        if (!hmem->read4b(n))
            return false;
        if (hmem->state() || hmem->skipBlock(n) != static_cast<size_t>(n))
            return false;
    }
    if (!hmem->read4b(n))
        return false;
    if (hmem->state())
        return false;
    return hmem->skipBlock(n) == static_cast<size_t>(n);
}

static int SizeExpected;
static int SizeRead;

static int ReadSizeField(int size)
{
    SizeExpected = size;
    if (!hmem->read4b(SizeRead))
        return -1;
    if (hmem->state())
        return -1;
    return SizeRead;
}

static bool SkipUnusedField(void)
{
    return (SizeExpected >= SizeRead) &&
        hmem->skipBlock(SizeRead - SizeExpected) != 0;
}


#define HDOFILE_HEADER_SIZE (2*4+16)              // 16=sizeof(ZZRect)
#define HDOFILE_COMMON_SIZE (7*4+16+44)

#define HDOFILE_HAS_NEXT    0x01
#define HDOFILE_HAS_CHILD   0x02

static bool LoadCommonHeader(HWPDrawingObject * hdo, unsigned short * link_info)
{
    uint size, common_size;

    if (!hmem)
        return false;
    if (!hmem->read4b(size))
        return false;
    if (hmem->state())
        return false;
    if (size < HDOFILE_COMMON_SIZE)
        return false;

    common_size = HDOFILE_COMMON_SIZE;
    unsigned short tmp16;
    if (!hmem->read2b(tmp16))
        return false;
    hdo->type = tmp16;
    if (!hmem->read2b(tmp16))
        return false;
    *link_info = tmp16;
    if (!hmem->read4b(hdo->offset.x))
        return false;
    if (!hmem->read4b(hdo->offset.y))
        return false;
    if (!hmem->read4b(hdo->extent.w))
        return false;
    if (!hmem->read4b(hdo->extent.h))
        return false;
    if (!hmem->read4b(hdo->offset2.x))
        return false;
    if (!hmem->read4b(hdo->offset2.y))
        return false;

    if (hmem->state())
        return false;

    if (!hmem->read4b(hdo->vrect.x))
        return false;
    if (!hmem->read4b(hdo->vrect.y))
        return false;
    if (!hmem->read4b(hdo->vrect.w))
        return false;
    if (!hmem->read4b(hdo->vrect.h))
        return false;

// read bare property 44 bytes
    if (!hmem->read4b(hdo->property.line_pstyle))
        return false;
    if (!hmem->read4b(hdo->property.line_hstyle))
        return false;
    if (!hmem->read4b(hdo->property.line_tstyle))
        return false;
    if (!hmem->read4b(hdo->property.line_color))
        return false;
    unsigned int tmp32;
    if (!hmem->read4b(tmp32))
        return false;
    hdo->property.line_width = static_cast<hunit>(tmp32);
    if (!hmem->read4b(hdo->property.fill_color))
        return false;
    if (!hmem->read4b(hdo->property.pattern_type))
        return false;
    if (!hmem->read4b(hdo->property.pattern_color))
        return false;
    if (!hmem->read4b(tmp32))
        return false;
    hdo->property.hmargin = static_cast<hunit>(tmp32);
    if (!hmem->read4b(tmp32))
        return false;
    hdo->property.vmargin = static_cast<hunit>(tmp32);
    if (!hmem->read4b(hdo->property.flag))
        return false;
// read rotation property 32 bytes
    if ((size >= common_size + 32)
        && (hdo->property.flag & HWPDO_FLAG_ROTATION))
    {
        if (!hmem->read4b(hdo->property.rot_originx))
            return false;
        if (!hmem->read4b(hdo->property.rot_originy))
            return false;
        for (int ii = 0; ii < 3; ++ii)
        {
            if (!hmem->read4b(hdo->property.parall.pt[ii].x))
                return false;
            if (!hmem->read4b(hdo->property.parall.pt[ii].y))
                return false;
        }
        common_size += 32;
    }
    else
        SetHdoParallRgn(hdo, hdo->extent.w, hdo->extent.h);

// read gradient property 28 bytes
    if ((size >= common_size + 28) &&
        (hdo->property.flag & HWPDO_FLAG_GRADATION))
    {
        if (!hmem->read4b(hdo->property.fromcolor))
            return false;
        if (!hmem->read4b(hdo->property.tocolor))
            return false;
        if (!hmem->read4b(hdo->property.gstyle))
            return false;
        if (!hmem->read4b(hdo->property.angle))
            return false;
        if (!hmem->read4b(hdo->property.center_x))
            return false;
        if (!hmem->read4b(hdo->property.center_y))
            return false;
        if (!hmem->read4b(hdo->property.nstep))
            return false;
        common_size += 28;
    }

// read bitmap property 278 bytes
    if ((size >= common_size + 278) && \
        (hdo->property.flag & HWPDO_FLAG_BITMAP))
    {
        if (!hmem->read4b(hdo->property.offset1.x))
            return false;
        if (!hmem->read4b(hdo->property.offset1.y))
            return false;
        if (!hmem->read4b(hdo->property.offset2.x))
            return false;
        if (!hmem->read4b(hdo->property.offset2.y))
            return false;
        if (!hmem->readBlock(hdo->property.szPatternFile, 261))
            return false;
        if (!hmem->read1b(hdo->property.pictype))
            return false;
        common_size += 278;
    }
    if( ( size >= common_size + 3 ) && ( hdo->property.flag & HWPDO_FLAG_WATERMARK ) )
     //if( ( size >= common_size ) && ( hdo->property.flag >> 20 & 0x01 ) )
    {
        if (size - common_size >= 5)
            hmem->skipBlock(2);
        unsigned char tmp8;
        if (!hmem->read1b(tmp8))
            return false;
        hdo->property.luminance = tmp8;
        if (!hmem->read1b(tmp8))
            return false;
        hdo->property.contrast = tmp8;
        if (!hmem->read1b(tmp8))
            return false;
        hdo->property.greyscale = tmp8;

        common_size += 5;
    }
    else
    {
        hdo->property.luminance = 0;
        hdo->property.contrast = 0;
        hdo->property.greyscale = 0;
    }
    hdo->property.pPara = nullptr;

    if( ( size > common_size ) && (hdo->property.flag & HWPDO_FLAG_AS_TEXTBOX) )
    {
        hmem->skipBlock(8);
        hdo->property.pPara = LoadParaList();
        if( hdo->property.pPara )
            return true;
        else
            return false;
     }

    if (size <= common_size)
          return true;
    return hmem->skipBlock(size - common_size ) != 0;
}

static std::unique_ptr<HWPDrawingObject> LoadDrawingObject(void)
{
    HWPDrawingObject *prev = nullptr;
    std::unique_ptr<HWPDrawingObject> hdo, head;

    unsigned short link_info;

    do
    {
        hdo.reset(new HWPDrawingObject);
        if (!LoadCommonHeader(hdo.get(), &link_info))
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
            switch (int res = HWPDOFunc(hdo.get(), OBJFUNC_LOAD, nullptr, 0))
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
            if (hdo->child == nullptr)
            {
                goto error;
            }
        }
        if (prev == nullptr)
        {
            head = std::move(hdo);
            prev = head.get();
        }
        else
        {
            prev->next = std::move(hdo);
            prev = prev->next.get();
        }
    }
    while (link_info & HDOFILE_HAS_NEXT);

    return head;

error:
// drawing object can be list.
// hdo = current item, head = list;

    if (hdo->type < 0 || hdo->type >= HWPDO_NITEMS)
    {
        hdo->type = HWPDO_RECT;
    }
    HWPDOFunc(hdo.get(), OBJFUNC_FREE, nullptr, 0);
    hdo.reset();

    if( prev )
    {
        prev->next = nullptr;
        return head;
    }
    else
        return nullptr;
}


static bool LoadDrawingObjectBlock(Picture * pic)
{
    int size;
    if (!hmem->read4b(size))
        return false;

    if (hmem->state() || size < HDOFILE_HEADER_SIZE)
        return false;

    if (!hmem->read4b(pic->picinfo.picdraw.zorder))
        return false;
    if (!hmem->read4b(pic->picinfo.picdraw.mbrcnt))
        return false;
    if (!hmem->read4b(pic->picinfo.picdraw.vrect.x))
        return false;
    if (!hmem->read4b(pic->picinfo.picdraw.vrect.y))
        return false;
    if (!hmem->read4b(pic->picinfo.picdraw.vrect.w))
        return false;
    if (!hmem->read4b(pic->picinfo.picdraw.vrect.h))
        return false;

    if (size > HDOFILE_HEADER_SIZE &&
        !hmem->skipBlock(size - HDOFILE_HEADER_SIZE))
        return false;

    pic->picinfo.picdraw.hdo = LoadDrawingObject().release();
    if (pic->picinfo.picdraw.hdo == nullptr)
        return false;
    return true;
}

// object manipulation function
static int
HWPDODefaultFunc(int cmd)
{
    if (cmd == OBJFUNC_LOAD)
        return OBJRET_FILE_NO_PRIVATE_BLOCK;
    return OBJRET_FILE_OK;
}

static int
HWPDOLineFunc(int /*type*/, HWPDrawingObject * hdo, int cmd, void * /*argp*/, int /*argv*/)
{
    int ret = OBJRET_FILE_OK;
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.line_arc.flip))
                return OBJRET_FILE_ERROR;
            if (hmem->state())
                return OBJRET_FILE_ERROR;
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;
            ret = OBJRET_FILE_NO_PRIVATE_BLOCK_2;
            break;
        default:
            ret = HWPDODefaultFunc(cmd);
            break;
    }
    return ret;
}


// rectangle

static int
HWPDORectFunc(int /*type*/, HWPDrawingObject * /*hdo*/, int cmd, void * /*argp*/, int /*argv*/)
{
    return HWPDODefaultFunc(cmd);
}


// ellipse

static int
HWPDOEllipseFunc(int /*type*/, HWPDrawingObject * /*hdo*/,
int cmd, void * /*argp*/, int /*argv*/)
{
    return HWPDODefaultFunc(cmd);
}

#define WTMM(x)     ((double)(x) / 1800. * 25.4)
static int
HWPDOEllipse2Func(int /*type*/, HWPDrawingObject * hdo,
int cmd, void * /*argp*/, int /*argv*/)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(16) < 16)
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.arc.radial[0].x))
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.arc.radial[0].y))
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.arc.radial[1].x))
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.arc.radial[1].y))
                return OBJRET_FILE_ERROR;
            if (ReadSizeField(0) < 0)
                return OBJRET_FILE_ERROR;
            break;
        default:
            return HWPDODefaultFunc(cmd);
    }
    return OBJRET_FILE_OK;
}


// arc

static int
HWPDOArcFunc(int /*type*/, HWPDrawingObject * hdo, int cmd, void * /*argp*/, int /*argv*/)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.line_arc.flip))
                return OBJRET_FILE_ERROR;
            if (hmem->state())
                return OBJRET_FILE_ERROR;
            if (!SkipUnusedField())
                return OBJRET_FILE_ERROR;
            break;
        default:
            return HWPDODefaultFunc(cmd);
    }
    return OBJRET_FILE_OK;
}


static int
HWPDOArc2Func(int /*type*/, HWPDrawingObject * /*hdo*/, int cmd, void * /*argp*/, int /*argv*/)
{
    int ret = OBJRET_FILE_OK;
    switch (cmd)
    {
        case OBJFUNC_LOAD:
            ret = OBJRET_FILE_NO_PRIVATE_BLOCK;
            break;
        default:
            ret = HWPDODefaultFunc(cmd);
            break;
    }
    return ret;
}


static int
HWPDOFreeFormFunc(int /*type*/, HWPDrawingObject * hdo,
int cmd, void * /*argp*/, int /*argv*/)
{
    switch (cmd)
    {
        case OBJFUNC_LOAD:
        {
            hdo->u.freeform.pt = nullptr;
            if (ReadSizeField(4) < 4)
                return OBJRET_FILE_ERROR;
            if (!hmem->read4b(hdo->u.freeform.npt))
                return OBJRET_FILE_ERROR;
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
                if (hdo->u.freeform.pt == nullptr)
                {
                    hdo->u.freeform.npt = 0;
                    return OBJRET_FILE_ERROR;
                }
                for (int ii = 0; ii < hdo->u.freeform.npt; ++ii)
                {
                    bool bFailure = false;
                    if (!hmem->read4b(hdo->u.freeform.pt[ii].x))
                        bFailure = true;
                    if (!hmem->read4b(hdo->u.freeform.pt[ii].y))
                        bFailure = true;
                    if (hmem->state())
                        bFailure = true;
                    if (bFailure)
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
            return HWPDODefaultFunc(cmd);
    }
    return OBJRET_FILE_OK;
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
        return nullptr;

    HWPFile *hwpf = GetCurrentDoc();
    std::unique_ptr<HIODev> hio = hwpf->SetIODevice(std::unique_ptr<HIODev>(hmem));

    std::vector< HWPPara* > plist;

    hwpf->ReadParaList(plist);
    std::unique_ptr<HIODev> orighmem = hwpf->SetIODevice(std::move(hio));
    hmem = orighmem.release();

    return plist.size()? plist.front() : nullptr;
}


static int
HWPDOTextBoxFunc(int /*type*/, HWPDrawingObject * hdo,
int cmd, void * /*argp*/, int /*argv*/)
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
                hdo->u.textbox.h = nullptr;
            }
            break;
        default:
            return HWPDODefaultFunc(cmd);
    }
    return OBJRET_FILE_OK;
}



static int
HWPDOContainerFunc(int /*type*/, HWPDrawingObject * /*hdo*/,
int cmd, void * /*argp*/, int /*argv*/)
{
    return HWPDODefaultFunc(cmd);
}


HWPDrawingObject::HWPDrawingObject():
    type(0), offset{0, 0}, offset2{0, 0}, extent{0, 0}, vrect{0, 0, 0, 0}
{
    memset(&property, 0, sizeof property);
    memset(&u, 0, sizeof u);
    index = ++count;
}


HWPDrawingObject::~HWPDrawingObject()
{
    if (property.pPara)
        FreeParaList(property.pPara);

    HWPDOFunc(this, OBJFUNC_FREE, nullptr, 0);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
