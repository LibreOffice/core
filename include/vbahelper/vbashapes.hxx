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
#ifndef INCLUDED_VBAHELPER_VBASHAPES_HXX
#define INCLUDED_VBAHELPER_VBASHAPES_HXX

#include <exception>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <ooo/vba/msforms/XShapes.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbacollectionimpl.hxx>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XEnumeration; }
    namespace container { class XIndexAccess; }
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
    namespace msforms { class XShapeRange; }
} }

typedef CollTestImplHelper< ov::msforms::XShapes > ScVbaShapes_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShapes : public ScVbaShapes_BASE
{
private:
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    sal_Int32 m_nNewShapeCount;
    void initBaseCollection();
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::container::XIndexAccess > getShapesByArrayIndices( const css::uno::Any& Index );
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::drawing::XShape > createShape( const OUString& service );
    /// @throws css::uno::RuntimeException
    css::uno::Any AddRectangle( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight );
    /// @throws css::uno::RuntimeException
    css::uno::Any AddEllipse( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight );
    /// @throws css::uno::RuntimeException
    css::uno::Any AddTextboxInWriter( sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight );
    OUString createName( const OUString& sName );
    //TODO helperapi using a writer document
    //css::awt::Point calculateTopLeftMargin( css::uno::Reference< ov::XHelperInterface > xDocument );

public:
    ScVbaShapes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const css::uno::Reference< css::frame::XModel >& xModel );
    /// @throws css::uno::RuntimeException
    static void setDefaultShapeProperties( const css::uno::Reference< css::drawing::XShape >& xShape );
    static void setShape_NameProperty( const css::uno::Reference< css::drawing::XShape >& xShape, const OUString& sName );
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    virtual void SAL_CALL SelectAll() override;
    //helper::calc
    virtual css::uno::Any SAL_CALL AddLine( sal_Int32 StartX, sal_Int32 StartY, sal_Int32 endX, sal_Int32 endY ) override;
    virtual css::uno::Any SAL_CALL AddShape( sal_Int32 _nType, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) override;
    virtual css::uno::Any SAL_CALL AddTextbox( sal_Int32 _nOrientation, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) override;
    virtual css::uno::Reference< ov::msforms::XShapeRange > SAL_CALL Range( const css::uno::Any& shapes ) override;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
};

#endif // INCLUDED_VBAHELPER_VBASHAPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
