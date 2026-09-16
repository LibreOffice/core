/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/XAssistant.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef ::cppu::WeakImplHelper< ov::XAssistant > Assistant;
typedef InheritedHelperInterfaceImpl< Assistant > ScVbaAssistantImpl_BASE;

class ScVbaAssistant : public ScVbaAssistantImpl_BASE
{
private:
    bool            m_bIsVisible;
    sal_Int32       m_nPointsLeft;
    sal_Int32       m_nPointsTop;
    sal_Int32       m_nAnimation;
public:
    ScVbaAssistant( const cpo::uno::Reference< ov::XHelperInterface >& rParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& rContext );
    virtual ~ScVbaAssistant() override;
    // XAssistant
    virtual bool getOn() override;
    virtual void setOn( bool _on ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;
    virtual ::sal_Int32 getTop() override;
    virtual void setTop( ::sal_Int32 _top ) override;
    virtual ::sal_Int32 getLeft() override;
    virtual void setLeft( ::sal_Int32 _left ) override;
    virtual ::sal_Int32 getAnimation() override;
    virtual void setAnimation( ::sal_Int32 _animation ) override;

    virtual OUString Name(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
