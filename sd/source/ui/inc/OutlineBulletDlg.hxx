/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlineBulletDlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:35:23 $
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

#ifndef SD_OUTLINE_BULLET_DLG_HXX
#define SD_OUTLINE_BULLET_DLG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#include "dlgolbul.hrc"

namespace sd {

class View;

/*************************************************************************
|*
|* Bullet-Tab-Dialog
|*
\************************************************************************/
class OutlineBulletDlg
    : public SfxTabDialog
{
public:
    OutlineBulletDlg (
        ::Window* pParent,
        const SfxItemSet* pAttr,
        ::sd::View* pView );
    virtual ~OutlineBulletDlg (void);

    const SfxItemSet* GetOutputItemSet() const;

protected:
    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );

private:
    using SfxTabDialog::GetOutputItemSet;

    SfxItemSet  aInputSet;
    SfxItemSet  *pOutputSet;
    BOOL        bTitle;
    ::sd::View      *pSdView;
};

} // end of namespace sd

#endif

