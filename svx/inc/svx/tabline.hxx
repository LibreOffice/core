/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabline.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:41:51 $
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
#ifndef _SVX_TAB_LINE_HXX
#define _SVX_TAB_LINE_HXX

// include ---------------------------------------------------------------

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_XLINIIT_HXX //autogen
#include <svx/xlineit.hxx>
#endif
#ifndef _SVX_XSETITEM_HXX //autogen
#include <svx/xsetit.hxx>
#endif

#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif
#ifndef _XOUTX_HXX //autogen
#include <svx/xoutx.hxx>
#endif

#ifndef _SV_MENUBTN_HXX
#include <vcl/menubtn.hxx>
#endif
class SvxBrushItem;
// define ----------------------------------------------------------------

typedef USHORT ChangeType; // auch in tab_area.hxx (mitpflegen !!!)

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

