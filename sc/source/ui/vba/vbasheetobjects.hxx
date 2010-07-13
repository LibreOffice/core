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

#ifndef SC_VBA_SHEETOBJECTS_HXX
#define SC_VBA_SHEETOBJECTS_HXX

#include <ooo/vba/excel/XGraphicObjects.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XEnumeration; }
    namespace frame { class XModel; }
    namespace sheet { class XSpreadsheet; }
} } }

// ============================================================================

class ScVbaObjectContainer;
typedef ::rtl::Reference< ScVbaObjectContainer > ScVbaObjectContainerRef;

// ============================================================================

typedef CollTestImplHelper< ov::XCollection > ScVbaSheetObjects_BASE;

/** Base class for collections containing a specific type of drawing object
    embedded in a sheet (worksheet, chart sheet, or dialog sheet).
 */
class ScVbaSheetObjectsBase : public ScVbaSheetObjects_BASE
{
public:
    explicit ScVbaSheetObjectsBase( const ScVbaObjectContainerRef& rxContainer ) throw (css::uno::RuntimeException);
    virtual ~ScVbaSheetObjectsBase();

    /** Updates the collection by fetching all shapes from the draw page. */
    void collectShapes() throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);

    // ScVbaCollectionBase
    virtual css::uno::Any createCollectionObject( const css::uno::Any& rSource );
    virtual css::uno::Any getItemByStringIndex( const ::rtl::OUString& rIndex ) throw (css::uno::RuntimeException);

protected:
    ScVbaObjectContainerRef mxContainer;
};

// ============================================================================

typedef ::cppu::ImplInheritanceHelper1< ScVbaSheetObjectsBase, ov::excel::XGraphicObjects > ScVbaGraphicObjects_BASE;

/** Base class for collections containing a specific type of graphic object
    from a sheet.
 */
class ScVbaGraphicObjectsBase : public ScVbaGraphicObjects_BASE
{
public:
    explicit ScVbaGraphicObjectsBase( const ScVbaObjectContainerRef& rxContainer ) throw (css::uno::RuntimeException);

    // XGraphicObjects
    virtual css::uno::Any SAL_CALL Add(
        const css::uno::Any& rLeft,
        const css::uno::Any& rTop,
        const css::uno::Any& rWidth,
        const css::uno::Any& rHeight ) throw (css::uno::RuntimeException);
};

// ============================================================================

/** Collection containing all button controls from a sheet (not ActiveX controls). */
class ScVbaButtons : public ScVbaGraphicObjectsBase
{
public:
    explicit ScVbaButtons(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::sheet::XSpreadsheet >& rxSheet ) throw (css::uno::RuntimeException);

    VBAHELPER_DECL_XHELPERINTERFACE
};

// ============================================================================

#endif
