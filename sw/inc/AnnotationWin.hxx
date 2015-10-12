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
        virtual void dispose() override;

        virtual void    UpdateData() override;
        virtual void    SetPostItText() override;
        virtual void    Delete() override;
        virtual void    GotoPos() override;

        virtual OUString GetAuthor() override;
        virtual Date    GetDate() override;
        virtual tools::Time GetTime() override;

        virtual sal_uInt32 MoveCaret() override;

        virtual bool    CalcFollow() override;
        void            InitAnswer(OutlinerParaObject* pText);

        virtual bool    IsProtected() override;

    protected:
        virtual VclPtr<MenuButton> CreateMenuButton() override;

    private:
        virtual SvxLanguageItem GetLanguage() override;
        sal_uInt32 CountFollowing();

        SwFormatField*       mpFormatField;
        SwPostItField*  mpField;
        PopupMenu*      mpButtonPopup;

};

} } // end of namespace sw::annotation
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
