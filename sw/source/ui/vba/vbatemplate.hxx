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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATEMPLATE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATEMPLATE_HXX

#include <ooo/vba/word/XTemplate.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XTemplate > SwVbaTemplate_BASE;

class SwVbaTemplate : public SwVbaTemplate_BASE
{
private:
    OUString msFullUrl;
public:
    SwVbaTemplate( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const OUString& );
    virtual ~SwVbaTemplate() override;

   // XTemplate
    virtual OUString SAL_CALL getName() override;
    virtual OUString SAL_CALL getPath() override;
    virtual css::uno::Any SAL_CALL AutoTextEntries( const css::uno::Any& index ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBATEMPLATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
