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

#ifndef SD_OUTLINE_BULLET_DLG_HXX
#define SD_OUTLINE_BULLET_DLG_HXX

#include <sfx2/tabdlg.hxx>

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
    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

private:
    using SfxTabDialog::GetOutputItemSet;

    SfxItemSet  aInputSet;
    SfxItemSet  *pOutputSet;
    sal_Bool        bTitle;
    ::sd::View      *pSdView;
};

} // end of namespace sd

#endif

