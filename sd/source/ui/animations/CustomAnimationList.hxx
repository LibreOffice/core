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

#ifndef _SD_CUSTOMANIMATIONLIST_HXX
#define _SD_CUSTOMANIMATIONLIST_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <svtools/treelistbox.hxx>
#include <CustomAnimationEffect.hxx>
#include "CustomAnimationPreset.hxx"
#include "CustomAnimation.hrc"

namespace sd {

class CustomAnimationEffect;
typedef boost::shared_ptr< CustomAnimationEffect > CustomAnimationEffectPtr;

class ICustomAnimationListController
{
public:
    virtual void onSelect() = 0;
    virtual void onDoubleClick() = 0;
    virtual void onContextMenu( sal_uInt16 nSelectedPopupEntry ) = 0;
    virtual ~ICustomAnimationListController() {}
};

class CustomAnimationList : public SvTreeListBox, public ISequenceListener
{
    friend class CustomAnimationListEntryItem;
    friend struct stl_append_effect_func;

public:
    CustomAnimationList( ::Window* pParent, const ResId& rResId, ICustomAnimationListController* pController );
    virtual ~CustomAnimationList();

    // methods

    /** selects or deselects the given effect.
        Selections of other effects are not changed */
    void select( CustomAnimationEffectPtr pEffect, bool bSelect = true );

    /** populates the list with all effects from the given MainSequence */
    void update( MainSequencePtr pMainSequence );

    void update();

    EffectSequence getSelection() const;

    // events
    void onSelectionChanged( ::com::sun::star::uno::Any aSelection );

    // overrides
    virtual void    SelectHdl();
    virtual sal_Bool    DoubleClickHdl();

    virtual void    Paint( const Rectangle& rRect );

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

    virtual void KeyInput( const KeyEvent& rKEvt );

    virtual void    SetTabs();

    virtual void notify_change();

    const Image& getImage( sal_uInt16 nId );

    bool isExpanded( const CustomAnimationEffectPtr& pEffect ) const;

    /// clears all entries from the listbox
    void clear();

private:
    bool    mbIgnorePaint;

    /** appends the given effect to the list*/
    void append( CustomAnimationEffectPtr pEffect );

    ICustomAnimationListController* mpController;

    MainSequencePtr mpMainSequence;

    Image maImgEmpty;

    Image maImages[ IMG_CUSTOMANIMATION_MEDIA_STOP - IMG_CUSTOMANIMATION_ON_CLICK + 1];

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxLastTargetShape;
    sal_Int32 mnLastGroupId;
    SvLBoxEntry* mpLastParentEntry;

};

}

#endif // _SD_CUSTOMANIMATIONLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
