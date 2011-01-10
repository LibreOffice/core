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

#ifndef _SVX_SMARTTAGSCONTROL_HXX
#define _SVX_SMARTTAGSCONTROL_HXX

// include ---------------------------------------------------------------

#include <tools/link.hxx>
#include <sfx2/mnuitem.hxx>
#include "svx/svxdllapi.h"
#include <com/sun/star/uno/Reference.hxx>

#include <vector>

class SfxBindings;
class SvxSmartTagItem;
class PopupMenu;

namespace com { namespace sun { namespace star { namespace smarttags {
    class XSmartTagAction;
} } } }

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

// class SvxFontMenuControl ----------------------------------------------

class SVX_DLLPUBLIC SvxSmartTagsControl : public SfxMenuControl
{
private:
    PopupMenu*                  mpMenu;
    Menu&                       mrParent;
    const SvxSmartTagItem*      mpSmartTagItem;

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

    void            FillMenu();
    DECL_LINK( MenuSelect, PopupMenu * );
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );

public:
    SvxSmartTagsControl( sal_uInt16 nId, Menu&, SfxBindings& );
    ~SvxSmartTagsControl();

    virtual PopupMenu*  GetPopup() const;
    SFX_DECL_MENU_CONTROL();
};

#endif
