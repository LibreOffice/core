/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframe.h,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:33:56 $
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

#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifndef _SV_VCLWINDOW_H
#include <VCLWindow.h>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

// ----------------
// - SalFrameData -
// ----------------

struct SalFrameData
{
    VCLWINDOW               mhWnd;                  // Window handle
    SalGraphics*            mpGraphics;             // current frame graphics
    SalFrame*               mpNextFrame;            // pointer to next frame
    SalFrame*               mpParent;               // pointer to parent frame
    void*                   mpInst;                 // instance handle for callback
    SALFRAMEPROC            mpProc;                 // callback proc
    long                    mnWidth;                // client width in pixels
    long                    mnHeight;               // client height in pixels
    BOOL                    mbGraphics;             // is Graphics used?
};

#endif // _SV_SALFRAME_H
