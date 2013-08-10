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
    CustomAnimationList( ::Window* pParent );
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

    void setController( ICustomAnimationListController* pController )
    {
        mpController = pController;
    };

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
    SvTreeListEntry* mpLastParentEntry;

};

}

#endif // _SD_CUSTOMANIMATIONLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
