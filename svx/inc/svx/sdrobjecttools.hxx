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

#ifndef _SVDOBJ_TOOLS_HXX
#define _SVDOBJ_TOOLS_HXX

#include "svx/svxdllapi.h"
#include <svx/svdobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SdrPathObj;

//////////////////////////////////////////////////////////////////////////////
// defines

enum DefaultSdrPathObjType
{
    DefaultSdrPathObjType_Line,         // SID_DRAW_LINE, SID_DRAW_XLINE, SID_LINE_ARROW_START, SID_LINE_ARROW_END, SID_LINE_ARROWS, SID_LINE_ARROW_CIRCLE, SID_LINE_CIRCLE_ARROW, SID_LINE_ARROW_SQUARE, SID_LINE_SQUARE_ARROW
    DefaultSdrPathObjType_BezierFill,   // SID_DRAW_BEZIER_FILL
    DefaultSdrPathObjType_Bezier,       // SID_DRAW_BEZIER_NOFILL
    DefaultSdrPathObjType_Freeline,     // SID_DRAW_FREELINE, SID_DRAW_FREELINE_NOFILL
    DefaultSdrPathObjType_Polygon,      // SID_DRAW_POLYGON, SID_DRAW_POLYGON_NOFILL
    DefaultSdrPathObjType_XPolygon,     // SID_DRAW_XPOLYGON, SID_DRAW_XPOLYGON_NOFILL
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper for constructing views to create default geometries for diverse kinds of
// SdrPathObjs, as used e.g. in pressing CTRL-Tab in keyboard toolbar navigation

void SVX_DLLPUBLIC initializeDefaultSdrPathObjByObjectType(SdrPathObj& rObj, DefaultSdrPathObjType eType, const basegfx::B2DRange& rRange, bool bClose);

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDOBJ_TOOLS_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
