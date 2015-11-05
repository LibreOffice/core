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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_STMENU_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_STMENU_HXX

#include <vcl/menu.hxx>

#include <vector>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>

#include <com/sun/star/container/XStringKeyMap.hpp>
#include <com/sun/star/text/XTextRange.hpp>

class SwView;

/** Class: SwSmartTagPopup

   This class contains the implementation of the smarttag popup
   menu that is opened if a user clicks on an underlined word.

   The menu is built in the constructor and the actions for each
   menu entry are invoked in the execute-method.
*/

class SwSmartTagPopup : public PopupMenu
{
    SwView*  mpSwView;
    css::uno::Reference< css::text::XTextRange > mxTextRange;

    struct InvokeAction
    {
        css::uno::Reference< css::smarttags::XSmartTagAction > mxAction;
        css::uno::Reference< css::container::XStringKeyMap > mxSmartTagProperties;
        sal_uInt32 mnActionID;
        InvokeAction( css::uno::Reference< css::smarttags::XSmartTagAction > xAction,
                      css::uno::Reference< css::container::XStringKeyMap > xSmartTagProperties,
                      sal_uInt32 nActionID ) : mxAction( xAction ), mxSmartTagProperties( xSmartTagProperties ), mnActionID( nActionID ) {}
    };

    std::vector< InvokeAction > maInvokeActions;

    using PopupMenu::Execute;

public:
    SwSmartTagPopup( SwView* _pSwView,
                     css::uno::Sequence< OUString >& rSmartTagTypes,
                     css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                     css::uno::Reference< css::text::XTextRange > xTextRange );

    sal_uInt16  Execute( const Rectangle& rPopupPos, vcl::Window* pWin );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
