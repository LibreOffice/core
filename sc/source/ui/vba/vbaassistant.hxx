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
#ifndef SC_VBA_ASSISTANT_HXX
#define SC_VBA_ASSISTANT_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XAssistant.hpp>

#include <sfx2/sfxhelp.hxx>

#include "excelvbahelper.hxx"
#include <vbahelper/vbahelperinterface.hxx>

typedef ::cppu::WeakImplHelper1< ov::XAssistant > Assistant;
typedef InheritedHelperInterfaceImpl< Assistant > ScVbaAssistantImpl_BASE;

class ScVbaAssistant : public ScVbaAssistantImpl_BASE
{
private:
    sal_Bool        m_bIsVisible;
    sal_Int32       m_nPointsLeft;
    sal_Int32       m_nPointsTop;
    OUString   m_sName;
    sal_Int32       m_nAnimation;
public:
    ScVbaAssistant( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext );
    virtual ~ScVbaAssistant();
    // XAssistant
    virtual sal_Bool SAL_CALL getOn() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOn( sal_Bool _on ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( ::sal_Int32 _top ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( ::sal_Int32 _left ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getAnimation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAnimation( ::sal_Int32 _animation ) throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL Name(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};

#endif//SC_VBA_ASSISTANT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
