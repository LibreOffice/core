/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabarea.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:41:40 $
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
#ifndef _SVX_TAB_AREA_HXX
#define _SVX_TAB_AREA_HXX

// include ---------------------------------------------------------------

#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svx/xoutx.hxx>
#include <svx/dlgctrl.hxx>

#ifndef _SVX_XSETITEM_HXX //autogen
#include <svx/xsetit.hxx>
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif

#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif

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

typedef USHORT ChangeType; // auch in tab_line.hxx (mitpflegen !!!)

#define CT_NONE                 ( (ChangeType) 0x0000 )
#define CT_MODIFIED             ( (ChangeType) 0x0001 )
#define CT_CHANGED              ( (ChangeType) 0x0002 )
#define CT_SAVED                ( (ChangeType) 0x0004 )

class SdrModel;
class SdrView;

#endif // _SVX_TAB_AREA_HXX

