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
#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#define __FRAMEWORK_CLASSES_BMKMENU_HXX

#include "framework/addonmenu.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <framework/fwedllapi.h>

#include <vcl/menu.hxx>
#include <vcl/image.hxx>

class ImageList;

#define BMKMENU_ITEMID_START    20000

namespace framework
{

class BmkMenu_Impl;
class FWE_DLLPUBLIC BmkMenu : public AddonMenu
{
    public:
        enum BmkMenuType
        {
            BMK_NEWMENU,
            BMK_WIZARDMENU
        };

        BmkMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                 BmkMenuType nType
        );

        ~BmkMenu();

        void Initialize(); // Synchrones Laden der Eintraege

    protected:
        BmkMenu::BmkMenuType m_nType;
        sal_uInt16          CreateMenuId();

    private:

        BmkMenu_Impl*   _pImp;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
