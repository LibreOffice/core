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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAWRAPFORMAT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAWRAPFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <ooo/vba/word/XWrapFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XWrapFormat > SwVbaWrapFormat_BASE;

class SwVbaWrapFormat : public SwVbaWrapFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 mnWrapFormatType;
    sal_Int32 mnSide;

private:
    void makeWrap() throw (css::uno::RuntimeException);
    float getDistance( const OUString& sName ) throw (css::uno::RuntimeException);
    void setDistance( const OUString& sName, float _distance ) throw (css::uno::RuntimeException);

public:
    SwVbaWrapFormat(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    virtual ::sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setType( ::sal_Int32 _type ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getSide() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSide( ::sal_Int32 _side ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual float SAL_CALL getDistanceTop() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDistanceTop( float _distancetop ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual float SAL_CALL getDistanceBottom() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDistanceBottom( float _distancebottom ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual float SAL_CALL getDistanceLeft() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDistanceLeft( float _distanceleft ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual float SAL_CALL getDistanceRight() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDistanceRight( float _distanceright ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAWRAPFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
