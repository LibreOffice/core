/*************************************************************************
 *
 *  $RCSfile: formatsh.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
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
    SFX_DECL_INTERFACE(SCID_FORMAT_SHELL);

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
};

#endif
