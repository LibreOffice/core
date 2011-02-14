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
#ifndef _TABLEDLG_HXX
#define _TABLEDLG_HXX

#include <sfx2/tabdlg.hxx>


#include "swtypes.hxx"
#include "swtablerep.hxx"

class SwWrtShell;
class SwTabCols;
struct TColumn;

/*-------------------------------------------------------
 Tabellendialog
--------------------------------------------------------- */
class SwTableTabDlg : public SfxTabDialog
{
    SwWrtShell* pShell;
    sal_uInt16      nHtmlMode;

    virtual void        PageCreated(sal_uInt16 nId, SfxTabPage& rPage);
public:
     SwTableTabDlg( Window* pParent, SfxItemPool& Pool,
                    const SfxItemSet* pItemSet, SwWrtShell* pSh );
};


#endif
