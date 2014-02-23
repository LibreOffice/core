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

#ifndef INCLUDED_SVX_GRFFLT_HXX
#define INCLUDED_SVX_GRFFLT_HXX

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
#include <svx/svxdllapi.h>


// - Error codes -


#define SVX_GRAPHICFILTER_ERRCODE_NONE              0x00000000
#define SVX_GRAPHICFILTER_UNSUPPORTED_GRAPHICTYPE   0x00000001
#define SVX_GRAPHICFILTER_UNSUPPORTED_SLOT          0x00000002


// - SvxGraphicFilter -


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
