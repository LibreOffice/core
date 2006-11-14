/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lightdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:40:03 $
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


#ifndef _SVX_LIGHT3D_HXX_
#define _SVX_LIGHT3D_HXX_

#include "dlgctl3d.hxx"

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SVX_DLG_CTRL_HXX //autogen
#include "dlgctrl.hxx"
#endif

/*************************************************************************
|*
|* 3D-Tab-Dialog
|*
\************************************************************************/

class Svx3DTabDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
            Svx3DTabDialog( Window* pParent, const SfxItemSet* pAttr );
            ~Svx3DTabDialog();
};

/*************************************************************************
|*
|* 3D-SingleTab-Dialog
|*
\************************************************************************/
class Svx3DSingleTabDialog : public SfxSingleTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
        Svx3DSingleTabDialog( Window* pParent, const SfxItemSet* pAttr );
        ~Svx3DSingleTabDialog();
};

#endif // _SVX_LIGHT3D_HXX_

