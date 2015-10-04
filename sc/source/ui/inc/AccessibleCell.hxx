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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECELL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECELL_HXX

#include "AccessibleCellBase.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <rtl/ref.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <editeng/AccessibleStaticTextBase.hxx>
#include <comphelper/uno3.hxx>

class ScTabViewShell;
class ScAccessibleDocument;

typedef cppu::ImplHelper< css::accessibility::XAccessibleExtendedAttributes >
                    ScAccessibleCellAttributeImpl;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleCell</code> service.
*/
class ScAccessibleCell
    :   public  ScAccessibleCellBase,
        public  accessibility::AccessibleStaticTextBase,
        public  ScAccessibleCellAttributeImpl
{
public:
    static rtl::Reference<ScAccessibleCell> create(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc);

private:
    ScAccessibleCell(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc);

    virtual void Init() override;

    using ScAccessibleCellBase::disposing;
    virtual void SAL_CALL disposing() override;

protected:
    virtual ~ScAccessibleCell() override;

    using ScAccessibleCellBase::IsDefunc;

public:
    ///=====  XInterface  =====================================================

    DECLARE_XINTERFACE()

    ///=====  XTypeProvider  ===================================================

    DECLARE_XTYPEPROVIDER()

    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus(  )
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox() const
        throw (css::uno::RuntimeException, std::exception) override;

public:
    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
                css::lang::IndexOutOfBoundsException, std::exception) override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::accessibility::XAccessibleRelationSet> SAL_CALL
           getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ====================================================

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

    virtual css::uno::Any SAL_CALL getExtendedAttributes()
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    // Override this method to handle cell's ParaIndent attribute specially.
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
private:
    ScTabViewShell* mpViewShell;
    ScAccessibleDocument* mpAccDoc;

    ScSplitPos meSplitPos;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);
    virtual bool IsEditable(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates) override;
    bool IsOpaque(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);
    bool IsSelected();

    static ScDocument* GetDocument(ScTabViewShell* mpViewShell);

    ::std::unique_ptr< SvxEditSource > CreateEditSource(ScTabViewShell* pViewShell, ScAddress aCell, ScSplitPos eSplitPos);

    void FillDependends(utl::AccessibleRelationSetHelper* pRelationSet);
    void FillPrecedents(utl::AccessibleRelationSetHelper* pRelationSet);
    void AddRelation(const ScAddress& rCell,
        const sal_uInt16 aRelationType,
        ::utl::AccessibleRelationSetHelper* pRelationSet);
    void AddRelation(const ScRange& rRange,
        const sal_uInt16 aRelationType,
        ::utl::AccessibleRelationSetHelper* pRelationSet);
    bool IsFormulaMode();
    bool IsDropdown();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
