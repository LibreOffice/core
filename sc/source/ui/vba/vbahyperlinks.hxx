/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_VBA_HYPERLINKS_HXX
#define SC_VBA_HYPERLINKS_HXX

#include <ooo/vba/excel/XHyperlinks.hpp>
#include <rtl/ref.hxx>
#include <vbahelper/vbacollectionimpl.hxx>

class ScRangeList;

// ============================================================================

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

// ============================================================================

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
            throw (css::uno::RuntimeException);

    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);

    // ScVbaCollectionBase
    virtual css::uno::Any createCollectionObject( const css::uno::Any& rSource );

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    ScVbaHyperlinksRef mxSheetHlinks;
};

// ============================================================================

#endif
