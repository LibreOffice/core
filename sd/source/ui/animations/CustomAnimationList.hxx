/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CustomAnimationList.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 15:33:15 $
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

#ifndef _SD_CUSTOMANIMATIONLIST_HXX
#define _SD_CUSTOMANIMATIONLIST_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _SD_CUSTOMANIMATIONEFFECT_HXX
#include <CustomAnimationEffect.hxx>
#endif
#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include "CustomAnimationPreset.hxx"
#endif
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
    virtual void onContextMenu( USHORT nSelectedPopupEntry ) = 0;
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
    virtual BOOL    DoubleClickHdl();

    virtual void    Paint( const Rectangle& rRect );

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

    virtual void KeyInput( const KeyEvent& rKEvt );

//  virtual SvLBoxEntry* CreateEntry() const;
    virtual void    SetTabs();

    virtual void notify_change();

    const Image& getImage( USHORT nId, bool bHighContrast );

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

