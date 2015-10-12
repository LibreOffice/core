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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBASHEETOBJECTS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBASHEETOBJECTS_HXX

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XGraphicObjects.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XEnumeration; }
    namespace frame { class XModel; }
    namespace sheet { class XSpreadsheet; }
} } }

class ScVbaObjectContainer;
typedef ::rtl::Reference< ScVbaObjectContainer > ScVbaObjectContainerRef;

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
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;

    // ScVbaCollectionBase
    virtual css::uno::Any createCollectionObject( const css::uno::Any& rSource ) override;
    virtual css::uno::Any getItemByStringIndex( const OUString& rIndex ) throw (css::uno::RuntimeException) override;

protected:
    ScVbaObjectContainerRef mxContainer;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaSheetObjectsBase, ov::excel::XGraphicObjects > ScVbaGraphicObjects_BASE;

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
        const css::uno::Any& rHeight ) throw (css::uno::RuntimeException, std::exception) override;
};

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
