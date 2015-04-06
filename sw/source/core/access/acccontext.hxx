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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCCONTEXT_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCCONTEXT_HXX

#include <accframe.hxx>
#include <accmap.hxx>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

namespace vcl { class Window; }
class SwAccessibleMap;
class SwCrsrShell;
class SdrObject;
class SwPaM;
namespace utl {
    class AccessibleStateSetHelper;
}
namespace accessibility {
    class AccessibleShape;
}

const sal_Char sAccessibleServiceName[] = "com.sun.star.accessibility.Accessible";

class SwAccessibleContext :
    public ::cppu::WeakImplHelper<
                ::com::sun::star::accessibility::XAccessible,
                ::com::sun::star::accessibility::XAccessibleContext,
                ::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::com::sun::star::lang::XServiceInfo
                >,
    public SwAccessibleFrame
{
    // The implements for the XAccessibleSelection interface has been
    // 'externalized' and wants access to the protected members like
    // GetMap, GetChild, GetParent, and GetFrm.
    friend class SwAccessibleSelectionHelper;

protected:
    mutable ::osl::Mutex aListenerMutex;
    mutable ::osl::Mutex aMutex;

private:
    OUString sName;  // immutable outside constructor

    // The parent if it has been retrieved. This is always an
    // SwAccessibleContext. (protected by Mutex)
    ::com::sun::star::uno::WeakReference <
        ::com::sun::star::accessibility::XAccessible > xWeakParent;

    SwAccessibleMap *pMap;  // must be protected by solar mutex

    sal_uInt32 nClientId;   // client id in the AccessibleEventNotifier queue
    sal_Int16 nRole;        // immutable outside constructor

    // The current states (protected by mutex)
    bool bIsShowingState : 1;
    bool bIsEditableState : 1;
    bool bIsOpaqueState : 1;
    bool bIsDefuncState : 1;

    // Are we currently disposing that object (protected by solar mutex)?
    bool bDisposing : 1;

    // #i85634# - boolean, indicating if the accessible context is
    // in general registered at the accessible map.
    bool bRegisteredAtAccessibleMap;

    void InitStates();
    //Add a member to identify the first time that document load
    bool bBeginDocumentLoad;

protected:
    void SetName( const OUString& rName ) { sName = rName; }
    inline sal_Int16 GetRole() const
    {
        return nRole;
    }
    //Add a member to identify if the document is Asyn load.
    bool isIfAsynLoad;
    //This flag is used to mark the object's selected state.
    bool   bIsSeletedInDoc;
    void SetParent( SwAccessibleContext *pParent );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> GetWeakParent() const;

    bool IsDisposing() const { return bDisposing; }

    vcl::Window *GetWindow();
    SwAccessibleMap *GetMap() { return pMap; }
    const SwAccessibleMap *GetMap() const { return pMap; }

    /** convenience method to get the SwViewShell through accessibility map */
    inline SwViewShell* GetShell()
    {
        return GetMap()->GetShell();
    }
    inline const SwViewShell* GetShell() const
    {
        return GetMap()->GetShell();
    }

    /** convenience method to get SwCrsrShell through accessibility map
     * @returns SwCrsrShell, or NULL if none is found */
    SwCrsrShell* GetCrsrShell();
    const SwCrsrShell* GetCrsrShell() const;

    // Notify all children that the vis area has changed.
    // The SwFrm might belong to the current object or to any other child or
    // grandchild.
    void ChildrenScrolled( const SwFrm *pFrm, const SwRect& rOldVisArea );

    // The context's showing state changed. May only be called for context that
    // exist even if they aren't visible.
    void Scrolled( const SwRect& rOldVisArea );

    // A child has been moved while setting the vis area
    void ScrolledWithin( const SwRect& rOldVisArea );

    // The has been added while setting the vis area
    void ScrolledIn();

    // The context has to be removed while setting the vis area
    void ScrolledOut( const SwRect& rOldVisArea );

    // Invalidate the states of all children of the specified SwFrm. The
    // SwFrm might belong the current object or to any child or grandchild!
    // #i27301# - use new type definition for <_nStates>
    void InvalidateChildrenStates( const SwFrm* _pFrm,
                                   AccessibleStates _nStates );

    // Dispose children of the specified SwFrm. The SwFrm might belong to
    // the current object or to any other child or grandchild.
    void DisposeChildren( const SwFrm *pFrm,
                          bool bRecursive );

    void DisposeShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl );
    void ScrolledInShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl );

    virtual void _InvalidateContent( bool bVisibleDataFired );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

public:
    void SetMap(SwAccessibleMap *pM){pMap = pM;}
    void FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventObject& rEvent );

protected:
    // broadcast visual data event
    void FireVisibleDataEvent();

    // broadcast state change event
    void FireStateChangedEvent( sal_Int16 nState, bool bNewState );

    // Set states for getAccessibleStateSet.
    // This base class sets DEFUNC(0/1), EDITABLE(0/1), ENABLED(1),
    // SHOWING(0/1), OPAQUE(0/1) and VISIBLE(1).
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

     bool IsEditableState();

    ::com::sun::star::awt::Rectangle SAL_CALL
        getBoundsImpl(bool bRelative)
        throw (css::uno::RuntimeException, std::exception);

    // #i85634#
    inline void NotRegisteredAtAccessibleMap()
    {
        bRegisteredAtAccessibleMap = false;
    }
    void RemoveFrmFromAccessibleMap();

    virtual ~SwAccessibleContext();

public:
    SwAccessibleContext( SwAccessibleMap *pMap, sal_Int16 nRole,
                         const SwFrm *pFrm );

    // XAccessible

    // Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleContext

    // Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) SAL_OVERRIDE;

    // Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore. */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleEventBroadcaster

    virtual void SAL_CALL addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(
            const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(
                const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Return whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service. */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
             throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // thread safe C++ interface

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( bool bRecursive = false );

    // The child object is not visible an longer and should be destroyed
    virtual void DisposeChild( const sw::access::SwAccessibleChild& rFrmOrObj, bool bRecursive );

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rFrm );

    // The vhild object has been moved by the layout
    virtual void InvalidateChildPosOrSize( const sw::access::SwAccessibleChild& rFrmOrObj,
                                           const SwRect& rFrm );

    // The content may have changed (but it hasn't tohave changed)
    void InvalidateContent();

    // The caretPos has changed
    void InvalidateCursorPos();

    // The Focus state has changed
    void InvalidateFocus();

    // Check states
    // #i27301# - use new type definition for <_nStates>
    void InvalidateStates( AccessibleStates _nStates );

    // the XAccessibleRelationSet may have changed
    void InvalidateRelation( sal_uInt16 nType );

    void InvalidateTextSelection(); // #i27301# - text selection has changed
    void InvalidateAttr(); // #i88069# - attributes has changed

    bool HasAdditionalAccessibleChildren();

    // #i88070# - get additional child by index
    vcl::Window* GetAdditionalAccessibleChild( const sal_Int32 nIndex );

    // #i88070# - get all additional accessible children
    void GetAdditionalAccessibleChildren( std::vector< vcl::Window* >* pChildren );

    const OUString& GetName() const { return sName; }

    virtual bool HasCursor();   // required by map to remember that object

    bool Select( SwPaM *pPaM, SdrObject *pObj, bool bAdd );
    inline bool Select( SwPaM& rPaM )
    {
        return Select( &rPaM, 0, false );
    }
    inline bool Select( SdrObject *pObj, bool bAdd )
    {
        return Select( 0, pObj, bAdd );
    }

    //This method is used to updated the selected state and fire the selected state changed event.
    virtual bool SetSelectedState(bool bSeleted);
    bool  IsSeletedInDoc(){  return bIsSeletedInDoc; }

    static OUString GetResource( sal_uInt16 nResId,
                                        const OUString *pArg1 = 0,
                                        const OUString *pArg2 = 0 );
};

// some heavily used exception support
#define THROW_RUNTIME_EXCEPTION( ifc, msg )                                 \
    ::com::sun::star::uno::Reference < ifc > xThis( this );                 \
    ::com::sun::star::uno::RuntimeException aExcept(                        \
        OUString( msg ), xThis );       \
    throw aExcept;

#define CHECK_FOR_DEFUNC_THIS( ifc, ths )                                   \
    if( !(GetFrm() && GetMap()) )                                           \
    {                                                                       \
        ::com::sun::star::uno::Reference < ifc > xThis( ths );              \
        ::com::sun::star::lang::DisposedException aExcept(                  \
            OUString( "object is defunctional" ),        \
            xThis );                                                        \
        throw aExcept;                                                      \
    }

#define CHECK_FOR_DEFUNC( ifc )                                             \
    CHECK_FOR_DEFUNC_THIS( ifc, this )

#define CHECK_FOR_WINDOW( i, w )                                            \
    if( !(w) )                                                              \
    {                                                                       \
        THROW_RUNTIME_EXCEPTION( i, "window is missing" );                  \
    }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
