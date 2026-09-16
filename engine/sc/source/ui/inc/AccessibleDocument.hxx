/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

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

class ScAccessibleDocument final
    : public cppu::ImplInheritanceHelper<
          ScAccessibleDocumentBase, css::accessibility::XAccessibleSelection,
          css::accessibility::XAccessibleExtendedAttributes, css::view::XSelectionChangeListener>,
      public accessibility::IAccessibleViewForwarder
{
public:
    ScAccessibleDocument(
        const cpo::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScSplitPos eSplitPos);

    void PreInit();

    virtual void Init() override;

    DECL_LINK( WindowChildEventListener, VclWindowEvent&, void );
protected:
    virtual ~ScAccessibleDocument() override;

    using ScAccessibleDocumentBase::IsDefunc;

public:

     virtual void disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleComponent  ============================================

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
        getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual void grabFocus(  ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int64
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild(sal_Int64 nIndex) override;

    /// Return the set of current states.
    virtual sal_Int64
        getAccessibleStateSet() override;

    virtual OUString
        getAccessibleName() override;

    virtual OUString getExtendedAttributes() override ;
    ///=====  XAccessibleSelection  ===========================================

    virtual void
        selectAccessibleChild( sal_Int64 nChildIndex ) override;

    virtual bool
        isAccessibleChildSelected( sal_Int64 nChildIndex ) override;

    virtual void
        clearAccessibleSelection(  ) override;

    virtual void
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int64
        getSelectedAccessibleChildCount(  ) override;

    virtual cpo::uno::Reference<
        css::accessibility::XAccessible >
        getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    virtual void
        deselectAccessibleChild( sal_Int64 nChildIndex ) override;

    ///=====  XSelectionListener  =============================================

    virtual void selectionChanged( const css::lang::EventObject& aEvent ) override;

    virtual void disposing( const css::lang::EventObject& Source ) override;

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

    cpo::uno::Reference< css::accessibility::XAccessible >
        GetAccessibleSpreadsheet();

protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

    /// Return the object's current bounding box relative to the desktop.
    virtual AbsoluteScreenPixelRectangle GetBoundingBoxOnScreen() override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() override;

private:
    ScTabViewShell* mpViewShell;
    ScSplitPos      meSplitPos;
    rtl::Reference<ScAccessibleSpreadsheet> mpAccessibleSpreadsheet;
    std::unique_ptr<ScChildrenShapes> mpChildrenShapes;
    rtl::Reference<ScAccessibleEditObject> mpTempAccEdit;
    cpo::uno::Reference<css::accessibility::XAccessible> mxTempAcc;
    tools::Rectangle maVisArea;
    bool mbCompleteSheetSelected;

public:
    SCTAB getVisibleTable() const; // use it in ScChildrenShapes

private:
    void FreeAccessibleSpreadsheet();

    bool IsTableSelected() const;

    bool IsDefunc(sal_Int64 nParentStates);

    void AddChild(const cpo::uno::Reference<css::accessibility::XAccessible>& xAcc, bool bFireEvent);
    void RemoveChild(const cpo::uno::Reference<css::accessibility::XAccessible>& xAcc, bool bFireEvent);

    OUString GetCurrentCellName() const;
    static const OUString & GetCurrentCellDescription();

    tools::Rectangle GetVisibleArea_Impl();
public:
    ScDocument *GetDocument() const ;
    ScAddress   GetCurCellAddress() const;

    virtual sal_Int32 getForeground(  ) override;

    virtual sal_Int32 getBackground(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
