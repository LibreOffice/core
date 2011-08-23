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

#ifndef GRAPHSH_HXX
#define GRAPHSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <bf_sfx2/shell.hxx>
#endif
#include "shellids.hxx"
#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <bf_svx/svdmark.hxx>
#endif

class ScViewData;

#include "drawsh.hxx"
namespace binfilter {

class ScGraphicShell: public ScDrawShell
{
public:

    SFX_DECL_INTERFACE(SCID_GRAPHIC_SHELL);

    ScGraphicShell(ScViewData* pData):ScDrawShell(pData){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ScGraphicShell(ScViewData* pData);
    void	Execute(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	Execute(SfxRequest& rReq);
    void	GetAttrState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetAttrState(SfxItemSet &rSet);
 
    void	ExecuteFilter(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecuteFilter(SfxRequest& rReq);
    void	GetFilterState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetFilterState(SfxItemSet &rSet);
};

} //namespace binfilter
#endif
