/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVXSWFRAMEPOSSTRINGS_HXX
#define _SVXSWFRAMEPOSSTRINGS_HXX

#include <tools/string.hxx>
#include "svx/svxdllapi.h"

/* -----------------04.03.2004 12:58-----------------
    contains strings needed for positioning dialogs
    of frames and drawing in Writer
 --------------------------------------------------*/
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

