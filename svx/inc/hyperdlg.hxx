/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperdlg.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:48:10 $
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

#ifndef _SVX_TAB_HYPERLINK_HXX
#define _SVX_TAB_HYPERLINK_HXX

// include ---------------------------------------------------------------

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class AbstractSvxHpLinkDlg;

class SVX_DLLPUBLIC SvxHlinkDlgWrapper : public SfxChildWindow
{
private:
    AbstractSvxHpLinkDlg*   mpDlg;

public:
    SvxHlinkDlgWrapper( Window*pParent, USHORT nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SvxHlinkDlgWrapper);
    virtual sal_Bool    QueryClose();
};

#endif // _SVX_TAB_HYPERLINK_HXX

