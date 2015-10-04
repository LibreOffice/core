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
#ifndef INCLUDED_SW_INC_TEXTCURSORHELPER_HXX
#define INCLUDED_SW_INC_TEXTCURSORHELPER_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>

class SwDoc;
class SwPaM;

class OTextCursorHelper : public ::cppu::ImplHelper < css::lang::XUnoTunnel >
{
public:

    OTextCursorHelper(){}
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    virtual const SwPaM*        GetPaM() const  = 0;
    virtual SwPaM*              GetPaM()        = 0;
    virtual const SwDoc*        GetDoc() const  = 0;
    virtual SwDoc*              GetDoc()        = 0;

protected:
    ~OTextCursorHelper() {}
};

#endif // INCLUDED_SW_INC_TEXTCURSORHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
