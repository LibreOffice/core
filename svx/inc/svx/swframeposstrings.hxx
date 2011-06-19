/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVXSWFRAMEPOSSTRINGS_HXX
#define _SVXSWFRAMEPOSSTRINGS_HXX

#include <tools/string.hxx>
#include "svx/svxdllapi.h"

/*
    contains strings needed for positioning dialogs
    of frames and drawing in Writer
 */
class SvxSwFramePosString_Impl;
class SVX_DLLPUBLIC SvxSwFramePosString
{
    SvxSwFramePosString_Impl* pImpl;
public:
    SvxSwFramePosString();
    ~SvxSwFramePosString();
    enum StringId
    {
        LEFT                       ,
        RIGHT                      ,
        FROMLEFT                   ,
        MIR_LEFT                   ,
        MIR_RIGHT                  ,
        MIR_FROMLEFT               ,
        FRAME                      ,
        PRTAREA                    ,
        REL_PG_LEFT                ,
        REL_PG_RIGHT               ,
        REL_FRM_LEFT               ,
        REL_FRM_RIGHT              ,
        MIR_REL_PG_LEFT            ,
        MIR_REL_PG_RIGHT           ,
        MIR_REL_FRM_LEFT           ,
        MIR_REL_FRM_RIGHT          ,
        REL_PG_FRAME               ,
        REL_PG_PRTAREA             ,
        REL_BASE                   ,
        REL_CHAR                   ,
        REL_ROW                    ,
        REL_BORDER                 ,
        REL_PRTAREA                ,
        FLY_REL_PG_LEFT            ,
        FLY_REL_PG_RIGHT           ,
        FLY_REL_PG_FRAME           ,
        FLY_REL_PG_PRTAREA         ,
        FLY_MIR_REL_PG_LEFT        ,
        FLY_MIR_REL_PG_RIGHT       ,
        TOP,
        BOTTOM,
        CENTER_HORI,
        CENTER_VERT,
        FROMTOP,
        FROMBOTTOM,
        BELOW,
        FROMRIGHT,
        REL_PG_TOP,
        REL_PG_BOTTOM,
        REL_FRM_TOP,
        REL_FRM_BOTTOM,
        REL_LINE,

        STR_MAX
    };
    const String& GetString(StringId eId);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
