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

#include <vos/mutex.hxx>
#include <tools/list.hxx>
#include <tools/color.hxx>
#include <tools/string.hxx>
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#include <rtl/uuid.h>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/compbase6.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <memory>
#include <vector>

// -----------
// - Defines -
// -----------

#define ITEM_OFFSET                 4
#define ITEM_OFFSET_DOUBLE          6
#define NAME_LINE_OFF_X             2
#define NAME_LINE_OFF_Y             2
#define NAME_LINE_HEIGHT            2
#define NAME_OFFSET                 2
#define SCRBAR_OFFSET               1
#define VALUESET_ITEM_NONEITEM      0xFFFE
#define VALUESET_SCROLL_OFFSET      4

// --------------------
// - ValueSetItemType -
// --------------------

enum ValueSetItemType
{
    VALUESETITEM_NONE,
    VALUESETITEM_IMAGE,
    VALUESETITEM_COLOR,
    VALUESETITEM_USERDRAW,
    VALUESETITEM_SPACE
};

// ----------------
// - ValueSetItem -
// ----------------

class ValueSet;

struct ValueSetItem
{
    ValueSet&           mrParent;
    sal_uInt16              mnId;
    sal_uInt16              mnBits;
    ValueSetItemType    meType;
    Image               maImage;
    Color               maColor;
    XubString           maText;
    void*               mpData;
    Rectangle           maRect;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >* mpxAcc;

    ValueSetItem( ValueSet& rParent );
    ~ValueSetItem();

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetAccessible( bool bIsTransientChildrenDisabled );
     void               ClearAccessible();
};

// -----------------------------------------------------------------------------

DECLARE_LIST( ValueItemList, ValueSetItem* )

// -----------------------------------------------------------------------------

struct ValueSet_Impl
{
    ::std::auto_ptr< ValueItemList >    mpItemList;
    bool                                mbIsTransientChildrenDisabled;
    Link                                maHighlightHdl;

    ValueSet_Impl() :   mpItemList( ::std::auto_ptr< ValueItemList >( new ValueItemList() ) ),
                        mbIsTransientChildrenDisabled( false )
    {
    }
};

// ---------------
// - ValueSetAcc -
// ---------------

typedef ::cppu::WeakComponentImplHelper6<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
    ::com::sun::star::accessibility::XAccessibleContext,
    ::com::sun::star::accessibility::XAccessibleComponent,
    ::com::sun::star::accessibility::XAccessibleSelection,
    ::com::sun::star::lang::XUnoTunnel >
    ValueSetAccComponentBase;

class ValueSetAcc :
    public ::comphelper::OBaseMutex,
    public ValueSetAccComponentBase
{
public:

    ValueSetAcc( ValueSet* pParent, bool bIsTransientChildrenDisabled );
    ~ValueSetAcc();

    void                FireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue );
    sal_Bool                HasAccessibleListeners() const { return( mxEventListeners.size() > 0 ); }

    static ValueSetAcc* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();

public:

    /** Called by the corresponding ValueSet when it gets the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void GetFocus (void);

    /** Called by the corresponding ValueSet when it loses the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void LoseFocus (void);


    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventBroadcaster
    using cppu::WeakComponentImplHelper6<com::sun::star::accessibility::XAccessible, com::sun::star::accessibility::XAccessibleEventBroadcaster, com::sun::star::accessibility::XAccessibleContext, com::sun::star::accessibility::XAccessibleComponent, com::sun::star::accessibility::XAccessibleSelection, com::sun::star::lang::XUnoTunnel>::addEventListener;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    using cppu::WeakComponentImplHelper6<com::sun::star::accessibility::XAccessible, com::sun::star::accessibility::XAccessibleEventBroadcaster, com::sun::star::accessibility::XAccessibleContext, com::sun::star::accessibility::XAccessibleComponent, com::sun::star::accessibility::XAccessibleSelection, com::sun::star::lang::XUnoTunnel>::removeEventListener;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );

private:
    //    ::vos::OMutex                                                                                                           maMutex;
    ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleEventListener > >   mxEventListeners;
    ValueSet*                                                           mpParent;
    bool                                                                mbIsTransientChildrenDisabled;
    /// The current FOCUSED state.
    bool mbIsFocused;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    /** Tell all listeners that the object is dying.  This callback is
        usually called from the WeakComponentImplHelper class.
    */
    virtual void SAL_CALL disposing (void);

    /** Return the number of items.  This takes the None-Item into account.
    */
    sal_uInt16 getItemCount (void) const;

    /** Return the item associated with the given index.  The None-Item is
        taken into account which, when present, is taken to be the first
        (with index 0) item.
        @param nIndex
            Index of the item to return.  The index 0 denotes the None-Item
            when present.
        @return
            Returns NULL when the given index is out of range.
    */
    ValueSetItem* getItem (sal_uInt16 nIndex) const;

    /** Check whether or not the object has been disposed (or is in the
        state of beeing disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    /** Check whether or not the object has been disposed (or is in the
        state of beeing disposed).

        @return sal_True, if the object is disposed or in the course
        of being disposed. Otherwise, sal_False is returned.
    */
    sal_Bool IsDisposed (void);

    /** Check whether the value set has a 'none' field, i.e. a field (button)
        that deselects any items (selects none of them).
        @return
            Returns <true/> if there is a 'none' field and <false/> it it is
            missing.
    */
    bool HasNoneField (void) const;
};

// ----------------
// - ValueItemAcc -
// ----------------

class ValueItemAcc : public ::cppu::WeakImplHelper5< ::com::sun::star::accessibility::XAccessible,
                                                     ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                                                     ::com::sun::star::accessibility::XAccessibleContext,
                                                     ::com::sun::star::accessibility::XAccessibleComponent,
                                                     ::com::sun::star::lang::XUnoTunnel >
{
private:

    ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleEventListener > >   mxEventListeners;
    ::vos::OMutex                                                       maMutex;
    ValueSetItem*                                                       mpParent;
    bool                                                                mbIsTransientChildrenDisabled;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

public:

    ValueItemAcc( ValueSetItem* pParent, bool bIsTransientChildrenDisabled );
    ~ValueItemAcc();

    void    ParentDestroyed();

    void    FireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue );
    sal_Bool    HasAccessibleListeners() const { return( mxEventListeners.size() > 0 ); }

    static ValueItemAcc* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();

public:

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
};
