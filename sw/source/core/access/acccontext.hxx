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
class SwCursorShell;
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
                css::accessibility::XAccessible,
                css::accessibility::XAccessibleContext,
                css::accessibility::XAccessibleComponent,
                css::accessibility::XAccessibleEventBroadcaster,
                css::lang::XServiceInfo
                >,
    public SwAccessibleFrame
{
    // The implements for the XAccessibleSelection interface has been
    // 'externalized' and wants access to the protected members like
    // GetMap, GetChild, GetParent, and GetFrame.
    friend class SwAccessibleSelectionHelper;

protected:
    mutable ::osl::Mutex m_Mutex;

private:
    OUString m_sName;  // immutable outside constructor

    // The parent if it has been retrieved. This is always an
    // SwAccessibleContext. (protected by Mutex)
    css::uno::WeakReference <
        css::accessibility::XAccessible > m_xWeakParent;

    SwAccessibleMap *m_pMap; // must be protected by solar mutex

    sal_uInt32 m_nClientId;  // client id in the AccessibleEventNotifier queue
    sal_Int16 m_nRole;        // immutable outside constructor

    // The current states (protected by mutex)
    bool m_isShowingState : 1;
    bool m_isEditableState : 1;
    bool m_isOpaqueState : 1;
    bool m_isDefuncState : 1;

    // Are we currently disposing that object (protected by solar mutex)?
    bool m_isDisposing : 1;

    // #i85634# - boolean, indicating if the accessible context is
    // in general registered at the accessible map.
    bool m_isRegisteredAtAccessibleMap;

    void InitStates();

protected:
    void SetName( const OUString& rName ) { m_sName = rName; }
    inline sal_Int16 GetRole() const
    {
        return m_nRole;
    }
    //This flag is used to mark the object's selected state.
    bool   m_isSelectedInDoc;
    void SetParent( SwAccessibleContext *pParent );
    css::uno::Reference< css::accessibility::XAccessible> GetWeakParent() const;

    bool IsDisposing() const { return m_isDisposing; }

    vcl::Window *GetWindow();
    SwAccessibleMap *GetMap() { return m_pMap; }
    const SwAccessibleMap *GetMap() const { return m_pMap; }

    /** convenience method to get the SwViewShell through accessibility map */
    inline SwViewShell* GetShell()
    {
        return GetMap()->GetShell();
    }
    inline const SwViewShell* GetShell() const
    {
        return GetMap()->GetShell();
    }

    /** convenience method to get SwCursorShell through accessibility map
     * @returns SwCursorShell, or NULL if none is found */
    SwCursorShell* GetCursorShell();
    const SwCursorShell* GetCursorShell() const;

    // Notify all children that the vis area has changed.
    // The SwFrame might belong to the current object or to any other child or
    // grandchild.
    void ChildrenScrolled( const SwFrame *pFrame, const SwRect& rOldVisArea );

    // The context's showing state changed. May only be called for context that
    // exist even if they aren't visible.
    void Scrolled( const SwRect& rOldVisArea );

    // A child has been moved while setting the vis area
    void ScrolledWithin( const SwRect& rOldVisArea );

    // The has been added while setting the vis area
    void ScrolledIn();

    // The context has to be removed while setting the vis area
    void ScrolledOut( const SwRect& rOldVisArea );

    // Invalidate the states of all children of the specified SwFrame. The
    // SwFrame might belong the current object or to any child or grandchild!
    // #i27301# - use new type definition for <_nStates>
    void InvalidateChildrenStates( const SwFrame* _pFrame,
                                   AccessibleStates _nStates );

    // Dispose children of the specified SwFrame. The SwFrame might belong to
    // the current object or to any other child or grandchild.
    void DisposeChildren( const SwFrame *pFrame,
                          bool bRecursive );

    void DisposeShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl );
    void ScrolledInShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl );

    virtual void _InvalidateContent( bool bVisibleDataFired );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

public:
    void SetMap(SwAccessibleMap *const pMap) { m_pMap = pMap; }
    void FireAccessibleEvent( css::accessibility::AccessibleEventObject& rEvent );

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

    css::awt::Rectangle SAL_CALL
        getBoundsImpl(bool bRelative)
        throw (css::uno::RuntimeException, std::exception);

    // #i85634#
    inline void NotRegisteredAtAccessibleMap()
    {
        m_isRegisteredAtAccessibleMap = false;
    }
    void RemoveFrameFromAccessibleMap();

    virtual ~SwAccessibleContext();

public:
    SwAccessibleContext( SwAccessibleMap *m_pMap, sal_Int16 nRole,
                         const SwFrame *pFrame );

    // XAccessible

    // Return the XAccessibleContext.
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext

    // Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
                css::lang::IndexOutOfBoundsException, std::exception) override;

    // Return a reference to the parent.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return NULL to indicate that an empty relation set.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore. */
    virtual css::lang::Locale SAL_CALL
        getLocale()
        throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster

    virtual void SAL_CALL addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(
            const css::awt::Point& aPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(
                const css::awt::Point& aPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getBounds()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocation()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service. */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
             throw (css::uno::RuntimeException, std::exception) override;

    // thread safe C++ interface

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( bool bRecursive = false );

    // The child object is not visible an longer and should be destroyed
    virtual void DisposeChild( const sw::access::SwAccessibleChild& rFrameOrObj, bool bRecursive );

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rFrame );

    // The child object has been moved by the layout
    virtual void InvalidateChildPosOrSize( const sw::access::SwAccessibleChild& rFrameOrObj,
                                           const SwRect& rFrame );

    // The content may have changed (but it hasn't to have changed)
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

    const OUString& GetName() const { return m_sName; }

    virtual bool HasCursor();   // required by map to remember that object

    bool Select( SwPaM *pPaM, SdrObject *pObj, bool bAdd );
    inline bool Select( SwPaM& rPaM )
    {
        return Select( &rPaM, nullptr, false );
    }
    inline bool Select( SdrObject *pObj, bool bAdd )
    {
        return Select( nullptr, pObj, bAdd );
    }

    //This method is used to updated the selected state and fire the selected state changed event.
    virtual bool SetSelectedState(bool bSeleted);
    bool  IsSeletedInDoc() { return m_isSelectedInDoc; }

    static OUString GetResource( sal_uInt16 nResId,
                                        const OUString *pArg1 = nullptr,
                                        const OUString *pArg2 = nullptr );
};

// some heavily used exception support
#define THROW_RUNTIME_EXCEPTION( ifc, msg )         \
    css::uno::Reference < ifc > xThis( this );      \
    css::uno::RuntimeException aExcept(             \
        OUString( msg ), xThis );       \
    throw aExcept;

#define CHECK_FOR_DEFUNC_THIS( ifc, ths )                \
    if( !(GetFrame() && GetMap()) )                        \
    {                                                    \
        css::uno::Reference < ifc > xThis( ths );        \
        css::lang::DisposedException aExcept(            \
            OUString( "object is defunctional" ),        \
            xThis );                                     \
        throw aExcept;                                   \
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
