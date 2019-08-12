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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVEL_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVEL_HXX

#include <ooo/vba/word/XListLevel.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbalisthelper.hxx"

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XListLevel > SwVbaListLevel_BASE;

class SwVbaListLevel : public SwVbaListLevel_BASE
{
private:
    SwVbaListHelperRef pListHelper;
    sal_Int32 const mnLevel;

public:
    /// @throws css::uno::RuntimeException
    SwVbaListLevel( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, SwVbaListHelperRef const & pHelper, sal_Int32 nLevel );
    virtual ~SwVbaListLevel() override;

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() override;
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) override;
    virtual css::uno::Reference< ::ooo::vba::word::XFont > SAL_CALL getFont() override;
    virtual void SAL_CALL setFont( const css::uno::Reference< ::ooo::vba::word::XFont >& _font ) override;
    virtual ::sal_Int32 SAL_CALL getIndex() override;
    virtual OUString SAL_CALL getLinkedStyle() override;
    virtual void SAL_CALL setLinkedStyle( const OUString& _linkedstyle ) override;
    virtual OUString SAL_CALL getNumberFormat() override;
    virtual void SAL_CALL setNumberFormat( const OUString& _numberformat ) override;
    virtual float SAL_CALL getNumberPosition() override;
    virtual void SAL_CALL setNumberPosition( float _numberposition ) override;
    virtual ::sal_Int32 SAL_CALL getNumberStyle() override;
    virtual void SAL_CALL setNumberStyle( ::sal_Int32 _numberstyle ) override;
    virtual ::sal_Int32 SAL_CALL getResetOnHigher() override;
    virtual void SAL_CALL setResetOnHigher( ::sal_Int32 _resetonhigher ) override;
    virtual ::sal_Int32 SAL_CALL getStartAt() override;
    virtual void SAL_CALL setStartAt( ::sal_Int32 _startat ) override;
    virtual float SAL_CALL getTabPosition() override;
    virtual void SAL_CALL setTabPosition( float _tabposition ) override;
    virtual float SAL_CALL getTextPosition() override;
    virtual void SAL_CALL setTextPosition( float _textposition ) override;
    virtual ::sal_Int32 SAL_CALL getTrailingCharacter() override;
    virtual void SAL_CALL setTrailingCharacter( ::sal_Int32 _trailingcharacter ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
