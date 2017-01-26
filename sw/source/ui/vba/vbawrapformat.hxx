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
    /// @throws css::uno::RuntimeException
    void makeWrap();
    /// @throws css::uno::RuntimeException
    float getDistance( const OUString& sName );
    /// @throws css::uno::RuntimeException
    void setDistance( const OUString& sName, float _distance );

public:
    SwVbaWrapFormat(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    virtual ::sal_Int32 SAL_CALL getType() override;
    virtual void SAL_CALL setType( ::sal_Int32 _type ) override;
    virtual ::sal_Int32 SAL_CALL getSide() override;
    virtual void SAL_CALL setSide( ::sal_Int32 _side ) override;
    virtual float SAL_CALL getDistanceTop() override;
    virtual void SAL_CALL setDistanceTop( float _distancetop ) override;
    virtual float SAL_CALL getDistanceBottom() override;
    virtual void SAL_CALL setDistanceBottom( float _distancebottom ) override;
    virtual float SAL_CALL getDistanceLeft() override;
    virtual void SAL_CALL setDistanceLeft( float _distanceleft ) override;
    virtual float SAL_CALL getDistanceRight() override;
    virtual void SAL_CALL setDistanceRight( float _distanceright ) override;

    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAWRAPFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
