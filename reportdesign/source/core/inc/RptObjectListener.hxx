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

#ifndef _REPORT_RPTUILIST_HXX
#define _REPORT_RPTUILIST_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

namespace rptui
{
class OObjectBase;


// OObjectListener


typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener > TPropertyChangeListenerHelper;

class OObjectListener: public TPropertyChangeListenerHelper
{
private:
    OObjectBase*        m_pObject;
    OObjectListener(const OObjectListener&);
    void operator =(const OObjectListener&);
protected:
    virtual ~OObjectListener();
public:
    OObjectListener(OObjectBase* _pObject);

    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);
};
}
#endif // _REPORT_RPTUILIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
