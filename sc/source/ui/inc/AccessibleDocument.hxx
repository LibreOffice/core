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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEDOCUMENT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEDOCUMENT_HXX

#include "AccessibleDocumentBase.hxx"
#include "viewdata.hxx"
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleGetAccFlowTo.hpp>
#include <svx/IAccessibleViewForwarder.hxx>

class ScTabViewShell;
class ScAccessibleSpreadsheet;
class ScChildrenShapes;
class ScAccessibleEditObject;

namespace utl
{
    class AccessibleRelationSetHelper;
}

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef cppu::ImplHelper3< css::accessibility::XAccessibleSelection,
                            css::accessibility::XAccessibleExtendedAttributes,
                            css::view::XSelectionChangeListener >
                    ScAccessibleDocumentImpl;

class ScAccessibleDocument
    :   public ScAccessibleDocumentBase,
        public ScAccessibleDocumentImpl,
        public css::accessibility::XAccessibleGetAccFlowTo,
        public accessibility::IAccessibleViewForwarder
{
public:
    //=====  internal  ========================================================
    ScAccessibleDocument(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScSplitPos eSplitPos);

    void PreInit();

    virtual void Init() override;

    DECL_LINK_TYPED( WindowChildEventListener, VclWindowEvent&, void );
protected:
    virtual ~ScAccessibleDocument();

    using ScAccessibleDocumentBase::IsDefunc;

public:

     virtual void SAL_CALL disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus(  )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
                css::lang::IndexOutOfBoundsException, std::exception) override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getExtendedAttributes()
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    ///=====  XSelectionListener  =============================================

    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ===================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /// returns the possible types
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  IAccessibleViewForwarder  ========================================

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

        @return
            The rectangle of the visible part of the document. The values
            are, contrary to the base class, in internal coordinates.
     */
    virtual Rectangle GetVisibleArea() const override;

    /** Transform the specified point from internal coordinates to an
        absolute screen position.

        @param rPoint
            Point in internal coordinates.

        @return
            The same point but in screen coordinates relative to the upper
            left corner of the (current) screen.
     */
    virtual Point LogicToPixel (const Point& rPoint) const override;

    /** Transform the specified size from internal coordinates to a screen
    * oriented pixel size.

        @param rSize
            Size in internal coordinates.

        @return
            The same size but in screen coordinates.
     */
    virtual Size LogicToPixel (const Size& rSize) const override;

    ///======== internal =====================================================

    utl::AccessibleRelationSetHelper* GetRelationSet(const ScAddress* pAddress) const;

    css::uno::Reference< css::accessibility::XAccessible >
        GetAccessibleSpreadsheet();

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox() const
        throw (css::uno::RuntimeException, std::exception) override;

private:
    ScTabViewShell* mpViewShell;
    ScSplitPos      meSplitPos;
    rtl::Reference<ScAccessibleSpreadsheet> mpAccessibleSpreadsheet;
    ScChildrenShapes* mpChildrenShapes;
    ScAccessibleEditObject* mpTempAccEdit;
    css::uno::Reference<css::accessibility::XAccessible> mxTempAcc;
    Rectangle maVisArea;
    bool mbCompleteSheetSelected;

public:
    SCTAB getVisibleTable() const; // use it in ScChildrenShapes

private:
    void FreeAccessibleSpreadsheet();

    bool IsTableSelected() const;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void AddChild(const css::uno::Reference<css::accessibility::XAccessible>& xAcc, bool bFireEvent);
    void RemoveChild(const css::uno::Reference<css::accessibility::XAccessible>& xAcc, bool bFireEvent);

    OUString GetCurrentCellName() const;
    static OUString GetCurrentCellDescription();

    Rectangle GetVisibleArea_Impl() const;
    css::uno::Sequence< css::uno::Any > GetScAccFlowToSequence();
public:
    ScDocument *GetDocument() const ;
    ScAddress   GetCurCellAddress() const;
    //=====  XAccessibleGetAccFromXShape  ============================================
    css::uno::Sequence< css::uno::Any >
        SAL_CALL getAccFlowTo(const css::uno::Any& rAny, sal_Int32 nType)
        throw ( css::uno::RuntimeException, std::exception ) override;

     virtual sal_Int32 SAL_CALL getForeground(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
