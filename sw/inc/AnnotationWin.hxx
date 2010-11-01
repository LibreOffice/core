/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:  $
 *
 * $Revision:  $
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

#ifndef _ANNOTATIONWIN_HXX
#define _ANNOTATIONWIN_HXX

#include <SidebarWin.hxx>

class PopupMenu;
class OutlinerParaObject;

namespace sw { namespace annotation {

class SwAnnotationWin : public sw::sidebarwindows::SwSidebarWin
{
    public:
        SwAnnotationWin( SwEditWin& rEditWin,
                         WinBits nBits,
                         SwPostItMgr& aMgr,
                         SwPostItBits aBits,
                         SwSidebarItem& rSidebarItem,
                         SwFmtFld* aField );
        virtual ~SwAnnotationWin();

        virtual void    UpdateData();
        virtual void    SetPostItText();
        virtual void    Delete();
        virtual void    GotoPos();

        virtual String  GetAuthor();
        virtual Date    GetDate();
        virtual Time    GetTime();

        virtual sal_uInt32 MoveCaret();

        virtual bool    CalcFollow();
        void            InitAnswer(OutlinerParaObject* pText);

        virtual bool    IsProtected();

    protected:
        virtual MenuButton* CreateMenuButton();

    private:
        virtual SvxLanguageItem GetLanguage(void);
        sal_uInt32 CountFollowing();

        SwFmtFld*       mpFmtFld;
        SwPostItField*  mpFld;
        PopupMenu*      mpButtonPopup;

};

} } // end of namespace sw::annotation
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
