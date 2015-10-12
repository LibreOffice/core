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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBASYSTEM_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBASYSTEM_HXX

#include <ooo/vba/word/XSystem.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapropvalue.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XSystem > SwVbaSystem_BASE;

class PrivateProfileStringListener : public PropListener
{
private:
    OUString maFileName;
    OString maGroupName;
    OString maKey;
public:
    PrivateProfileStringListener(){};
    virtual ~PrivateProfileStringListener();
    void Initialize( const OUString& rFileName, const OString& rGroupName, const OString& rKey );

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value ) override;
    virtual css::uno::Any getValueEvent() override;
};

class SwVbaSystem : public SwVbaSystem_BASE
{
private:
    PrivateProfileStringListener maPrivateProfileStringListener;

public:
    explicit SwVbaSystem( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaSystem();

    // XSystem
    virtual sal_Int32 SAL_CALL getCursor() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setCursor( sal_Int32 _cursor ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL PrivateProfileString( const OUString& rFilename, const OUString& rSection, const OUString& rKey ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBASYSTEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
