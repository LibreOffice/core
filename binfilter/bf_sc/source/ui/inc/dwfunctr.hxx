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
#ifndef _SC_DWFUNCTR_HXX
#define _SC_DWFUNCTR_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <bf_sfx2/childwin.hxx>
#endif

#ifndef _SFXDOCKWIN_HXX //autogen
#include <bf_sfx2/dockwin.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <bf_svtools/lstner.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <bf_svtools/stdctrl.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"		// ScAddress
#endif

namespace binfilter {

#ifndef	LRU_MAX
#define LRU_MAX 10
#endif
/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Controller
|*
\************************************************************************/

class ScFunctionChildWindow : public SfxChildWindow
{
 public:
     ScFunctionChildWindow( Window* pParent, USHORT nId, SfxBindings* pBindings,SfxChildWinInfo* pInfo):SfxChildWindow( pParent, nId ){DBG_ASSERT(0, "STRIP")}; //STRIP001 ScFunctionChildWindow( Window*, USHORT, SfxBindings*,

    SFX_DECL_CHILDWINDOW(ScFunctionChildWindow);
};

} //namespace binfilter
#endif


