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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_RPTOBJECTLISTENER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_RPTOBJECTLISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

namespace rptui
{
class OObjectBase;

class OObjectListener: public ::cppu::WeakImplHelper< css::beans::XPropertyChangeListener >
{
private:
    OObjectBase*        m_pObject;
    OObjectListener(const OObjectListener&) = delete;
    void operator =(const OObjectListener&) = delete;
protected:
    virtual ~OObjectListener();
public:
    OObjectListener(OObjectBase* _pObject);

    // XEventListener
    virtual void SAL_CALL disposing( const  css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException, std::exception) override;
};
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_RPTOBJECTLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
