/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmartTagCtl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:36:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_SMARTTAGSCONTROL_HXX
#define _SVX_SMARTTAGSCONTROL_HXX

// include ---------------------------------------------------------------

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SFXMNUITEM_HXX //autogen
#include <sfx2/mnuitem.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

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
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );

public:
    SvxSmartTagsControl( USHORT nId, Menu&, SfxBindings& );
    ~SvxSmartTagsControl();

    virtual PopupMenu*  GetPopup() const;
    SFX_DECL_MENU_CONTROL();
};

#endif
