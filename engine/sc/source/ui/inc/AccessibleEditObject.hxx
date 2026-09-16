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
    : public cppu::ImplInheritanceHelper<ScAccessibleContextBase,
                                         css::accessibility::XAccessibleSelection>
{
public:
    enum EditObjectType
    {
        CellInEditMode,
        EditLine,
        EditControl
    };

    ScAccessibleEditObject(
        const rtl::Reference<comphelper::OAccessible>& rpParent,
        EditView* pEditView, vcl::Window* pWin, const OUString& rName,
        const OUString& rDescription, EditObjectType eObjectType);

    void InitAcc(
        const rtl::Reference<comphelper::OAccessible>& rpParent,
        EditView* pEditView, const OUString& rName, const OUString& rDescription);

protected:
    virtual ~ScAccessibleEditObject() override;

    ScAccessibleEditObject(EditObjectType eObjectType);

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;

    virtual void disposing() override;

    void LostFocus();

    void GotFocus();

    ///=====  XAccessibleComponent  ============================================

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
        getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual OutputDevice* GetOutputDeviceForView();

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual AbsoluteScreenPixelRectangle GetBoundingBoxOnScreen() override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() override;

public:
    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int64
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild(sal_Int64 nIndex) override;

    /// Return the set of current states.
    virtual sal_Int64 getAccessibleStateSet() override;

    //=====  XAccessibleSelection  ============================================

        virtual void selectAccessibleChild(
            sal_Int64 nChildIndex ) override;
        virtual bool isAccessibleChildSelected(
            sal_Int64 nChildIndex ) override;
        virtual void clearAccessibleSelection(  ) override;
        virtual void selectAllAccessibleChildren(  ) override;
        virtual sal_Int64 getSelectedAccessibleChildCount(  ) override;
        virtual cpo::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild(
            sal_Int64 nSelectedChildIndex ) override;
        virtual void deselectAccessibleChild(
            sal_Int64 nSelectedChildIndex ) override;
    virtual cpo::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
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
    virtual void
        addAccessibleEventListener(
            const cpo::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener) override;

    //  Remove an existing event listener.
    virtual void
        removeAccessibleEventListener(
            const cpo::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener) override;

private:
    std::unique_ptr<accessibility::AccessibleTextHelper> mpTextHelper;
    EditView*      mpEditView;
    VclPtr<vcl::Window> mpWindow;
protected:
    ScTextWnd* mpTextWnd;
private:
    EditObjectType meObjectType;
    bool           mbHasFocus;

    bool IsDefunc(sal_Int64 nParentStates);

    void CreateTextHelper();
    ScDocument *m_pScDoc;
    ScAddress m_curCellAddress;

    ///=====  XAccessibleComponent  ============================================
    virtual sal_Int32 getForeground(  ) override;

    virtual sal_Int32 getBackground(  ) override;

    sal_Int32 GetFgBgColor(  const OUString &strPropColor) ;
};

class ScAccessibleEditControlObject : public ScAccessibleEditObject
{
private:
    weld::CustomWidgetController* m_pController;

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual AbsoluteScreenPixelRectangle GetBoundingBoxOnScreen() override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() override;

public:
    ScAccessibleEditControlObject(weld::CustomWidgetController* pController, EditObjectType eObjectType)
        : ScAccessibleEditObject(eObjectType)
        , m_pController(pController)
    {
    }

    virtual cpo::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;

    // for mapping positions/sizes within the TextView to a11y
    virtual OutputDevice* GetOutputDeviceForView() override;

    using ScAccessibleContextBase::disposing;
    virtual void disposing() override;
};

class ScAccessibleEditLineObject final : public ScAccessibleEditControlObject
{
public:
    ScAccessibleEditLineObject(ScTextWnd* pTextWnd);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
