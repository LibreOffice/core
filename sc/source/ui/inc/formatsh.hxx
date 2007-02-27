/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatsh.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:23:15 $
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

#ifndef SC_FORMATSH_HXX
#define SC_FORMATSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#include "shellids.hxx"
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

class ScViewData;

class ScFormatShell: public SfxShell
{
    ScViewData* pViewData;

protected:

    ScViewData*         GetViewData(){return pViewData;}

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_FORMAT_SHELL)

                ScFormatShell(ScViewData* pData);
    virtual     ~ScFormatShell();

    void        ExecuteNumFormat( SfxRequest& rReq );
    void        GetNumFormatState( SfxItemSet& rSet );

    void        ExecuteAttr( SfxRequest& rReq );
    void        GetAttrState( SfxItemSet& rSet );

    void        ExecuteAlignment( SfxRequest& rReq );

    void        ExecuteTextAttr( SfxRequest& rReq );
    void        GetTextAttrState( SfxItemSet& rSet );

    void        GetAlignState( SfxItemSet& rSet );
    void        GetBorderState( SfxItemSet& rSet );

    void        ExecuteStyle( SfxRequest& rReq );
    void        GetStyleState( SfxItemSet& rSet );

    void        ExecBckCol( SfxRequest& rReq );
    void        GetBckColState( SfxItemSet& rSet );

    void        ExecuteTextDirection( SfxRequest& rReq );
    void        GetTextDirectionState( SfxItemSet& rSet );

    void        ExecFormatPaintbrush( SfxRequest& rReq );
    void        StateFormatPaintbrush( SfxItemSet& rSet );
};

#endif
