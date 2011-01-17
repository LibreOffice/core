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

#ifndef SC_TEXTDLGS_HXX
#define SC_TEXTDLGS_HXX

#include <sfx2/tabdlg.hxx>

class SfxObjectShell;

//
//      Zeichen-Tab-Dialog
//

class ScCharDlg : public SfxTabDialog
{
private:
    const SfxItemSet&       rOutAttrs;
    const SfxObjectShell&   rDocShell;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
            ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                        const SfxObjectShell* pDocShell );
            ~ScCharDlg() {}
};

//
//      Absatz-Tab-Dialog
//

class ScParagraphDlg : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
            ScParagraphDlg( Window* pParent, const SfxItemSet* pAttr );
            ~ScParagraphDlg() {}
};



#endif

