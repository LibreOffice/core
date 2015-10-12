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

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <ooo/vba/msforms/XShapes.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::msforms::XShapes > ScVbaShapes_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShapes : public ScVbaShapes_BASE
{
private:
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    sal_Int32 m_nNewShapeCount;
    void initBaseCollection();
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    css::uno::Reference< css::container::XIndexAccess > getShapesByArrayIndices( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
    css::uno::Reference< css::drawing::XShape > createShape( const OUString& service ) throw (css::uno::RuntimeException);
    css::uno::Any AddRectangle( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, const css::uno::Any& rRange ) throw (css::uno::RuntimeException);
    css::uno::Any AddEllipse( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, const css::uno::Any& rRange ) throw (css::uno::RuntimeException);
    css::uno::Any AddTextboxInWriter( sal_Int32 _nOrientation, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException);
    OUString createName( const OUString& sName );
    //TODO helperapi using a writer document
    //css::awt::Point calculateTopLeftMargin( css::uno::Reference< ov::XHelperInterface > xDocument );

public:
    ScVbaShapes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const css::uno::Reference< css::frame::XModel >& xModel );
    static void setDefaultShapeProperties( css::uno::Reference< css::drawing::XShape > xShape ) throw (css::uno::RuntimeException);
    static void setShape_NameProperty( css::uno::Reference< css::drawing::XShape > xShape, const OUString& sName );
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    virtual void SAL_CALL SelectAll() throw (css::uno::RuntimeException, std::exception) override;
    //helper::calc
    virtual css::uno::Any SAL_CALL AddLine( sal_Int32 StartX, sal_Int32 StartY, sal_Int32 endX, sal_Int32 endY ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL AddShape( sal_Int32 _nType, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL AddTextbox( sal_Int32 _nOrientation, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XShapeRange > SAL_CALL Range( const css::uno::Any& shapes ) throw (css::uno::RuntimeException, std::exception) override;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) throw (css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& Index2)
         throw (css::lang::IndexOutOfBoundsException, css::script::BasicErrorException, css::uno::RuntimeException) override;
};

#endif // INCLUDED_VBAHELPER_VBASHAPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
