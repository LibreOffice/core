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


#ifndef _SVX_TAB_AREA_HXX
#define _SVX_TAB_AREA_HXX

// include ---------------------------------------------------------------

#include <svtools/valueset.hxx>
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svx/dlgctrl.hxx>
#include <svx/xsetit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfillit.hxx>

// enum ------------------------------------------------------------------

enum ColorModel
{
    CM_RGB,
    CM_CMYK // kann um weitere Modelle, z.B. HSV erweitert werden
};

enum PageType
{
    PT_AREA,
    PT_GRADIENT,
    PT_HATCH,
    PT_BITMAP,
    PT_COLOR,
    PT_SHADOW,
    PT_TRANSPARENCE
};

// define ----------------------------------------------------------------

typedef sal_uInt16 ChangeType; // auch in tab_line.hxx (mitpflegen !!!)

#define CT_NONE                 ( (ChangeType) 0x0000 )
#define CT_MODIFIED             ( (ChangeType) 0x0001 )
#define CT_CHANGED              ( (ChangeType) 0x0002 )
#define CT_SAVED                ( (ChangeType) 0x0004 )

class SdrModel;
class SdrView;

#endif // _SVX_TAB_AREA_HXX

