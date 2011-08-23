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

#ifndef SC_FORMATSH_HXX
#define SC_FORMATSH_HXX

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
namespace binfilter {

class ScViewData;

class ScFormatShell: public SfxShell
{
    ScViewData* pViewData;

protected:

    ScViewData*			GetViewData(){return pViewData;}

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_FORMAT_SHELL);

                ScFormatShell(ScViewData* pData);
    virtual		~ScFormatShell();

    void		ExecuteNumFormat( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		ExecuteNumFormat( SfxRequest& rReq );
    void		GetNumFormatState( SfxItemSet& rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		GetNumFormatState( SfxItemSet& rSet );

    void		ExecuteAttr( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		ExecuteAttr( SfxRequest& rReq );
    void		GetAttrState( SfxItemSet& rSet );

    void		ExecuteAlignment( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		ExecuteAlignment( SfxRequest& rReq );

    void		ExecuteTextAttr( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		ExecuteTextAttr( SfxRequest& rReq );
    void		GetTextAttrState( SfxItemSet& rSet );

    void		GetAlignState( SfxItemSet& rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		GetAlignState( SfxItemSet& rSet );
    void		GetBorderState( SfxItemSet& rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		GetBorderState( SfxItemSet& rSet );

    void		ExecuteStyle( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void		ExecuteStyle( SfxRequest& rReq );
    void		GetStyleState( SfxItemSet& rSet );


    void        ExecuteTextDirection( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001     void        ExecuteTextDirection( SfxRequest& rReq );
    void        GetTextDirectionState( SfxItemSet& rSet );
};

} //namespace binfilter
#endif
