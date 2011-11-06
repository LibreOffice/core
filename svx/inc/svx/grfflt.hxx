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



#ifndef _SVX_GRFFLT_HXX
#define _SVX_GRFFLT_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
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
