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

#pragma once

#include "AccessibleContextBase.hxx"

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <address.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/customweld.hxx>

#include <memory>

namespace accessibility
{
    class AccessibleTextHelper;
}
class EditView;
class ScTextWnd;
namespace vcl { class Window; }

/** @descr
        This base class provides an implementation of the
        <code>AccessibleCell</code> service.
*/
class ScAccessibleEditObject
    :   public  ScAccessibleContextBase,
        public css::accessibility::XAccessibleSelection
{
public:
    enum EditObjectType
    {
        CellInEditMode,
        EditLine,
        EditControl
    };

    ScAccessibleEditObject(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        EditView* pEditView, vcl::Window* pWin, const OUString& rName,
        const OUString& rDescription, EditObjectType eObjectType);

    void InitAcc(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        EditView* pEditView, vcl::Window* pWin, ScTextWnd* pTextWnd,
        const OUString& rName, const OUString& rDescription);

protected:
    virtual ~ScAccessibleEditObject() override;

    ScAccessibleEditObject(EditObjectType eObjectType);

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;

    virtual void SAL_CALL disposing() override;

    void LostFocus();

    void GotFocus();
///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;
    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual OutputDevice* GetOutputDeviceForView();

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() const override;

public:
    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex) override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    //=====  XAccessibleSelection  ============================================

        virtual void SAL_CALL selectAccessibleChild(
            sal_Int32 nChildIndex ) override;
        virtual sal_Bool SAL_CALL isAccessibleChildSelected(
            sal_Int32 nChildIndex ) override;
        virtual void SAL_CALL clearAccessibleSelection(  ) override;
        virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
        virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
            sal_Int32 nSelectedChildIndex ) override;
        virtual void SAL_CALL deselectAccessibleChild(
            sal_Int32 nSelectedChildIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

public:
    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener) override;

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    ///=====  XTypeProvider  ===================================================

    /** Returns an implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

private:
    std::unique_ptr<accessibility::AccessibleTextHelper> mpTextHelper;
    EditView*      mpEditView;
    VclPtr<vcl::Window> mpWindow;
    ScTextWnd* mpTextWnd;
    EditObjectType meObjectType;
    bool           mbHasFocus;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();
    ScDocument *m_pScDoc;
    ScAddress m_curCellAddress;

    ///=====  XAccessibleComponent  ============================================
    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    sal_Int32 GetFgBgColor(  const OUString &strPropColor) ;
};

class ScAccessibleEditControlObject : public ScAccessibleEditObject
{
private:
    weld::CustomWidgetController* m_pController;

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() const override;

public:
    ScAccessibleEditControlObject(weld::CustomWidgetController* pController, EditObjectType eObjectType)
        : ScAccessibleEditObject(eObjectType)
        , m_pController(pController)
    {
    }

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;

    // for mapping positions/sizes within the TextView to a11y
    virtual OutputDevice* GetOutputDeviceForView() override;

    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL disposing() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
