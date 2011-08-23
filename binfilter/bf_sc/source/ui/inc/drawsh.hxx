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

#ifndef SC_DRAWSH_HXX
#define SC_DRAWSH_HXX

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

#ifndef _LINK_HXX 
#include <tools/link.hxx>
#endif
namespace binfilter {

class SvxNameDialog;
class ScViewData;


class ScDrawShell : public SfxShell
{



public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_DRAW_SHELL);

    ScDrawShell(ScViewData* pData){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ScDrawShell(ScViewData* pData); 

    void 	StateDisableItems( SfxItemSet &rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void 	StateDisableItems( SfxItemSet &rSet );

    void	ExecDrawAttr(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecDrawAttr(SfxRequest& rReq);
    void	GetDrawAttrState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetDrawAttrState(SfxItemSet &rSet);
    void	GetAttrFuncState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetAttrFuncState(SfxItemSet &rSet);

    void	ExecDrawFunc(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecDrawFunc(SfxRequest& rReq);
    void	GetDrawFuncState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetDrawFuncState(SfxItemSet &rSet);
    void	GetState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetState(SfxItemSet &rSet);

    void	ExecFormText(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecFormText(SfxRequest& rReq);		// StarFontWork
    void	GetFormTextState(SfxItemSet& rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetFormTextState(SfxItemSet& rSet);

    void	ExecuteHLink(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecuteHLink(SfxRequest& rReq);		// Hyperlink
    void	GetHLinkState(SfxItemSet& rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetHLinkState(SfxItemSet& rSet);


};



} //namespace binfilter
#endif


