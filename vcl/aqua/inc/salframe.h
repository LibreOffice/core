/*************************************************************************
 *
 *  $RCSfile: salframe.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 22:12:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifdef __cplusplus

#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

// ----------------
// - SalFrameData -
// ----------------

struct SalFrameData
{
public:
    VCLWINDOW               mhWnd;                  // Window handle
    SalGraphics*            mpGraphics;             // current frame graphics
    SalFrame*               mpNextFrame;            // pointer to next frame
    void*                   mpInst;                 // instance handle for callback
    SALFRAMEPROC            mpProc;                 // callback proc
    SystemEnvData           maSysData;              // system data
    SalFrameState           maState;                // frame state
    int                     mnShowState;            // show state
    long                    mnWidth;                // client width in pixeln
    long                    mnHeight;               // client height in pixeln
    int                     mnFullScreenShowState;  // fullscreen restore show state
    UINT32                  mnInputLang;            // current Input Language
    UINT32                  mnInputCodePage;        // current Input CodePage
    USHORT                  mnStyle;                // style
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbCaption;              // has window a caption
    BOOL                    mbBorder;               // has window a border
    BOOL                    mbSizeBorder;           // has window a sizeable border
    BOOL                    mbFullScreen;           // TRUE: in full screen mode
    BOOL                    mbPresentation;         // TRUE: Presentation Mode running
    BOOL                    mbInShow;               // innerhalb eines Show-Aufrufs
    BOOL                    mbRestoreMaximize;      // Restore-Maximize
    BOOL                    mbInMoveMsg;            // Move-Message wird verarbeitet
    BOOL                    mbInSizeMsg;            // Size-Message wird verarbeitet
    BOOL                    mbFullScreenToolWin;    // WS_EX_TOOLWINDOW reset in FullScreenMode
    BOOL                    mbDefPos;               // default-position
    BOOL                    mbOverwriteState;       // TRUE: WindowState darf umgesetzt werden
    BOOL                    mbIME;                  // TRUE: We are in IME Mode
    BOOL                    mbHandleIME;            // TRUE: Wir handeln die IME-Messages
    BOOL                    mbSpezIME;              // TRUE: Spez IME
    BOOL                    mbAtCursorIME;          // TRUE: Wir behandeln nur einige IME-Messages
    BOOL                    mbCompositionMode;      // TRUE: Wir befinden uns im Composition-Modus
    BOOL                    mbCandidateMode;        // TRUE: Wir befinden uns im Candidate-Modus
};
#endif // __cplusplus

#endif // _SV_SALFRAME_H
