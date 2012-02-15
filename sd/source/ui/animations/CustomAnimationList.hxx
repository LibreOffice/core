/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SD_CUSTOMANIMATIONLIST_HXX
#define _SD_CUSTOMANIMATIONLIST_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <svtools/svtreebx.hxx>
#include <CustomAnimationEffect.hxx>
#include "CustomAnimationPreset.hxx"
#ifndef _SD_CUSTOMANIMATION_HRC
#include "CustomAnimation.hrc"
#endif

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

    /** updates the given effect in the list */
//  void update( CustomAnimationEffectPtr pEffect );

    void update();

    /** removes the given effect to the list*/
//  void remove( CustomAnimationEffectPtr pEffect );

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

//  virtual SvLBoxEntry* CreateEntry() const;
    virtual void    SetTabs();

    virtual void notify_change();

    const Image& getImage( sal_uInt16 nId, bool bHighContrast );

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

    Image maImages[ IMG_CUSTOMANIMATION_MEDIA_STOP_H - IMG_CUSTOMANIMATION_ON_CLICK + 1];

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxLastTargetShape;
    sal_Int32 mnLastGroupId;
    SvLBoxEntry* mpLastParentEntry;

};

}

#endif // _SD_CUSTOMANIMATIONLIST_HXX

