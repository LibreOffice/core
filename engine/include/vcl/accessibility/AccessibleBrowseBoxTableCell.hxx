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

#include <config_options.h>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxCell.hxx>

// implementation of a table cell of BrowseBox
class UNLESS_MERGELIBS(VCL_DLLPUBLIC) AccessibleBrowseBoxTableCell final
    : public cppu::ImplInheritanceHelper<AccessibleBrowseBoxCell,
                                         css::accessibility::XAccessibleText>,
      public ::comphelper::OCommonAccessibleText
{
private:
    // OCommonAccessibleText
    virtual OUString                        implGetText() final override;
    virtual css::lang::Locale               implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

public:
    AccessibleBrowseBoxTableCell( const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                                ::vcl::IAccessibleTableProvider& _rBrowseBox,
                                sal_Int32 _nRowId,
                                sal_uInt16 _nColId);

    // XEventListener
    using AccessibleBrowseBoxBase::disposing;
    virtual void disposing( const css::lang::EventObject& Source ) override;

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int64 getAccessibleIndexInParent() override;

    /** @return
            The name of this class.
    */
    virtual OUString getImplementationName() override;

    /** @return
            The count of visible children.
    */
    virtual sal_Int64 getAccessibleChildCount() override;

    /** @return
            The XAccessible interface of the specified child.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getAccessibleChild( sal_Int64 nChildIndex ) override;

    /** Return a bitset of states of the current object.
    */
    sal_Int64 implCreateStateSet() override;

    // XAccessibleText
    virtual sal_Int32 getCaretPosition() override;
    virtual bool setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode getCharacter( sal_Int32 nIndex ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCharacterAttributes( sal_Int32 nIndex, const cpo::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 getCharacterCount() override;
    virtual sal_Int32 getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString getSelectedText() override;
    virtual sal_Int32 getSelectionStart() override;
    virtual sal_Int32 getSelectionEnd() override;
    virtual bool setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString getText() final override;
    virtual OUString getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual bool copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

private:
    AccessibleBrowseBoxTableCell(const AccessibleBrowseBoxTableCell&) = delete;
    AccessibleBrowseBoxTableCell& operator=(const AccessibleBrowseBoxTableCell&) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
