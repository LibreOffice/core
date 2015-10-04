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
#ifndef INCLUDED_SVX_INC_UNOMLSTR_HXX
#define INCLUDED_SVX_INC_UNOMLSTR_HXX

#include <com/sun/star/util/XModifyListener.hpp>

#include <cppuhelper/implbase.hxx>

class SdrObject;

class SvxUnoShapeModifyListener : public ::cppu::WeakAggImplHelper< css::util::XModifyListener >
{
    SdrObject*  mpObj;

public:
    SvxUnoShapeModifyListener( SdrObject* pObj ) throw();
    virtual ~SvxUnoShapeModifyListener() throw() override;

    // css::util::XModifyListener
    virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) throw(  css::uno::RuntimeException, std::exception) override;

    // css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(  css::uno::RuntimeException, std::exception) override;

    // internal
    void invalidate() throw();
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
