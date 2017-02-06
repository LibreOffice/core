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
#ifndef INCLUDED_SVX_SWFRAMEPOSSTRINGS_HXX
#define INCLUDED_SVX_SWFRAMEPOSSTRINGS_HXX

#include <rtl/ustring.hxx>
#include <svx/svxdllapi.h>
#include <memory>

/*
    contains strings needed for positioning dialogs
    of frames and drawing in Writer
 */
class ResStringArray;
class SVX_DLLPUBLIC SvxSwFramePosString
{
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
    OUString GetString(StringId eId) const;

private:
    std::unique_ptr<ResStringArray> pImpl;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
