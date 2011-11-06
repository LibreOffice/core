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


#ifndef _SVX_TAB_LINE_HXX
#define _SVX_TAB_LINE_HXX

// include ---------------------------------------------------------------

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <svx/svdpage.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlineit.hxx>
#include <svx/xsetit.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/menubtn.hxx>
class SvxBrushItem;
// define ----------------------------------------------------------------

typedef sal_uInt16 ChangeType; // auch in tab_area.hxx (mitpflegen !!!)

#define CT_NONE                 ( (ChangeType) 0x0000 )
#define CT_MODIFIED             ( (ChangeType) 0x0001 )
#define CT_CHANGED              ( (ChangeType) 0x0002 )
#define CT_SAVED                ( (ChangeType) 0x0004 )

class SdrModel;
class SdrObject;

//defines fuer die Symbolauswahl auf der Page
//Positive Werte (0,...n) gibt den Indes in pSymbollist an, sonst :
#define SVX_SYMBOLTYPE_NONE       (-3)  //Symbol unsichtbar
#define SVX_SYMBOLTYPE_AUTO       (-2)  //Symbol automatisch erzeugen
#define SVX_SYMBOLTYPE_BRUSHITEM  (-1)  //Symbol ist als Graphic im Item enthalten:
#define SVX_SYMBOLTYPE_UNKNOWN  (-100) //unbekannt/nicht initialisiert

#endif // _SVX_TAB_LINE_HXX

