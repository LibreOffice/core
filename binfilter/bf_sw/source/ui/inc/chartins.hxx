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
#ifndef _CHARTINS_HXX
#define _CHARTINS_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <bf_sfx2/childwin.hxx>
#endif

#ifdef _CHARTINS_CXX

#ifndef _BASEDLGS_HXX //autogen
#include <bf_sfx2/basedlgs.hxx>
#endif

#ifndef _STDCTRL_HXX 
#include <bf_svtools/stdctrl.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#include "autoedit.hxx"
namespace binfilter {



//========================================================================

}//namespace binfilter
#endif
namespace binfilter {//STRIP009
class SwInsertChartChild : public SfxChildWindow
{
public:
    SwInsertChartChild( Window* ,
                        USHORT nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    SFX_DECL_CHILDWINDOW( SwInsertChartChild );
};


} //namespace binfilter
#endif

