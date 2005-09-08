/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swframeposstrings.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:03:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVXSWFRAMEPOSSTRINGS_HXX
#define _SVXSWFRAMEPOSSTRINGS_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

