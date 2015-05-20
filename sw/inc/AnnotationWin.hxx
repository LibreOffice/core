/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_INC_ANNOTATIONWIN_HXX
#define INCLUDED_SW_INC_ANNOTATIONWIN_HXX

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
                         SwFormatField* aField );
        virtual ~SwAnnotationWin();
        virtual void dispose() SAL_OVERRIDE;

        virtual void    UpdateData() SAL_OVERRIDE;
        virtual void    SetPostItText() SAL_OVERRIDE;
        virtual void    Delete() SAL_OVERRIDE;
        virtual void    GotoPos() SAL_OVERRIDE;

        virtual OUString GetAuthor() SAL_OVERRIDE;
        virtual Date    GetDate() SAL_OVERRIDE;
        virtual tools::Time GetTime() SAL_OVERRIDE;

        virtual sal_uInt32 MoveCaret() SAL_OVERRIDE;

        virtual bool    CalcFollow() SAL_OVERRIDE;
        void            InitAnswer(OutlinerParaObject* pText);

        virtual bool    IsProtected() SAL_OVERRIDE;

    protected:
        virtual VclPtr<MenuButton> CreateMenuButton() SAL_OVERRIDE;

    private:
        virtual SvxLanguageItem GetLanguage() SAL_OVERRIDE;
        sal_uInt32 CountFollowing();

        SwFormatField*       mpFormatField;
        SwPostItField*  mpField;
        PopupMenu*      mpButtonPopup;

};

} } // end of namespace sw::annotation
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
