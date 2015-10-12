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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAHYPERLINKS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAHYPERLINKS_HXX

#include <ooo/vba/excel/XHyperlinks.hpp>
#include <rtl/ref.hxx>
#include <vbahelper/vbacollectionimpl.hxx>

class ScRangeList;

namespace detail {

class ScVbaHlinkContainer;
typedef ::rtl::Reference< ScVbaHlinkContainer > ScVbaHlinkContainerRef;

/** Base class for ScVbaHyperlinks to get an initialized ScVbaHlinkContainer
    class member before the ScVbaHyperlinks_BASE base class will be constructed.
 */
struct ScVbaHlinkContainerMember
{
    ScVbaHlinkContainerRef mxContainer;

    explicit ScVbaHlinkContainerMember( ScVbaHlinkContainer* pContainer );
    ~ScVbaHlinkContainerMember();
};

} // namespace detail

class ScVbaHyperlinks;
typedef ::rtl::Reference< ScVbaHyperlinks > ScVbaHyperlinksRef;

typedef CollTestImplHelper< ov::excel::XHyperlinks > ScVbaHyperlinks_BASE;

/** Represents a collection of hyperlinks of a worksheet or of a range.

    When a Hyperlinks collection object has been constructed from a VBA
    Worksheet object, it will always represent the current set of all
    hyperlinks existing in the sheet. Insertion and deletion of hyperlinks will
    be reflected by the instance.

    When a Hyperlinks collection object has been constructed from a VBA Range
    object, it will represent the set of hyperlinks that have existed at its
    construction time, and that are located completely inside the range(s)
    represented by the Range object. Insertion and deletion of hyperlinks will
    *not* be reflected by that instance. The instance will always offer all
    hyperlinks it has been constructed with, even if they no longer exist.
    Furthermore, the instance will not offer hyperlinks inserted later, even if
    the instance itself has been used to insert the new hyperlinks.

    VBA code example:

    With ThisWorkbook.Worksheets(1)

        Set hlinks = .Hyperlinks                ' global Hyperlinks object
        Set myrange = .Range("A1:C3")
        Set rangelinks1 = myrange.Hyperlinks    ' hyperlinks of range A1:C3

        MsgBox hlinks.Count                     ' 0
        MsgBox rangelinks1.Count                ' 0

        hlinks.Add .Range("A1"), "http://example.com"
        ' a new hyperlink has been added in cell A1

        MsgBox hlinks.Count                     ' 1
        MsgBox rangelinks1.Count                ' still 0!
        Set rangelinks2 = myrange.Hyperlinks    ' hyperlinks of range A1:C3
        MsgBox rangelinks2.Count                ' 1 (constructed after Add)

        rangelinks1.Add .Range("A2"), "http://example.com"
        ' a new hyperlink has been constructed via the rangelinks1 object
        ' but this addition has been done by the worksheet Hyperlinks object

        MsgBox hlinks.Count                     ' 2
        MsgBox rangelinks1.Count                ' still 0!!!
        MsgBox rangelinks2.Count                ' still 1!!!
        MsgBox myrange.Hyperlinks.Count         ' 2 (constructed after Add)

    End With
 */
class ScVbaHyperlinks : private detail::ScVbaHlinkContainerMember, public ScVbaHyperlinks_BASE
{
public:
    explicit ScVbaHyperlinks(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext ) throw (css::uno::RuntimeException);

    explicit ScVbaHyperlinks(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const ScVbaHyperlinksRef& rxSheetHlinks, const ScRangeList& rScRanges ) throw (css::uno::RuntimeException);

    virtual ~ScVbaHyperlinks();

    // XHyperlinks
    virtual css::uno::Reference< ov::excel::XHyperlink > SAL_CALL Add(
        const css::uno::Any& rAnchor, const css::uno::Any& rAddress, const css::uno::Any& rSubAddress,
        const css::uno::Any& rScreenTip, const css::uno::Any& rTextToDisplay )
            throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;

    // ScVbaCollectionBase
    virtual css::uno::Any createCollectionObject( const css::uno::Any& rSource ) override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    ScVbaHyperlinksRef mxSheetHlinks;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
