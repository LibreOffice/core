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
#include <svx/IAccessibleViewForwarder.hxx>

class ScTabViewShell;
class ScAccessibleSpreadsheet;
class ScChildrenShapes;
class ScAccessibleEditObject;
class VclWindowEvent;

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

    DECL_LINK( WindowChildEventListener, VclWindowEvent&, void );
protected:
    virtual ~ScAccessibleDocument() override;

    using ScAccessibleDocumentBase::IsDefunc;

public:

     virtual void SAL_CALL disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual void SAL_CALL grabFocus(  ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex) override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Any SAL_CALL getExtendedAttributes() override ;
    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex ) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  ) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  ) override;

    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nChildIndex ) override;

    ///=====  XSelectionListener  =============================================

    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    ///=====  XServiceInfo  ===================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    /** Returns a list of all supported services.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    ///=====  XTypeProvider  ===================================================

    /// returns the possible types
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes() override;

    /** Returns an implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

    ///=====  IAccessibleViewForwarder  ========================================

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

        @return
            The rectangle of the visible part of the document. The values
            are, contrary to the base class, in internal coordinates.
     */
    virtual tools::Rectangle GetVisibleArea() const override;

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

    rtl::Reference<utl::AccessibleRelationSetHelper> GetRelationSet(const ScAddress* pAddress) const;

    css::uno::Reference< css::accessibility::XAccessible >
        GetAccessibleSpreadsheet();

protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

    /// Return the object's current bounding box relative to the desktop.
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    ScTabViewShell* mpViewShell;
    ScSplitPos      meSplitPos;
    rtl::Reference<ScAccessibleSpreadsheet> mpAccessibleSpreadsheet;
    std::unique_ptr<ScChildrenShapes> mpChildrenShapes;
    rtl::Reference<ScAccessibleEditObject> mpTempAccEdit;
    css::uno::Reference<css::accessibility::XAccessible> mxTempAcc;
    tools::Rectangle maVisArea;
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

    tools::Rectangle GetVisibleArea_Impl() const;
public:
    ScDocument *GetDocument() const ;
    ScAddress   GetCurCellAddress() const;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
