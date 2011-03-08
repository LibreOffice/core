/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// SMARTTAGS

#ifndef _STMENU_HXX
#define _STMENU_HXX

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
   menu entry are invoked in the excute-method.
*/

class SwSmartTagPopup : public PopupMenu
{
    SwView*  mpSwView;
    com::sun::star::uno::Reference< com::sun::star::text::XTextRange > mxTextRange;

    struct InvokeAction
    {
        com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > mxAction;
        com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > mxSmartTagProperties;
        sal_uInt32 mnActionID;
        InvokeAction( com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > xAction,
                      com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xSmartTagProperties,
                      sal_uInt32 nActionID ) : mxAction( xAction ), mxSmartTagProperties( xSmartTagProperties ), mnActionID( nActionID ) {}
    };

    std::vector< InvokeAction > maInvokeActions;

    using PopupMenu::Execute;

public:
    SwSmartTagPopup( SwView* _pSwView,
                     ::com::sun::star::uno::Sequence< rtl::OUString >& rSmartTagTypes,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     ::com::sun::star::uno::Reference< com::sun::star::text::XTextRange > xTextRange );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
