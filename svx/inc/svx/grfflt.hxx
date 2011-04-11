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

#ifndef _SVX_GRFFLT_HXX
#define _SVX_GRFFLT_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/timer.hxx>
#include <vcl/dialog.hxx>
#include <vcl/group.hxx>
#include <vcl/salbtype.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/graphctl.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/rectenum.hxx>
#include "svx/svxdllapi.h"

// ---------------
// - Error codes -
// ---------------

#define SVX_GRAPHICFILTER_ERRCODE_NONE              0x00000000
#define SVX_GRAPHICFILTER_UNSUPPORTED_GRAPHICTYPE   0x00000001
#define SVX_GRAPHICFILTER_UNSUPPORTED_SLOT          0x00000002

// --------------------
// - SvxGraphicFilter -
// --------------------

class SfxRequest;
class SfxItemSet;

class SVX_DLLPUBLIC SvxGraphicFilter
{
public:

    static sal_uLong    ExecuteGrfFilterSlot( SfxRequest& rReq, GraphicObject& rFilterObject );
    static void     DisableGraphicFilterSlots( SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
