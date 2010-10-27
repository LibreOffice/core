/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _REPORT_RPTUILIST_HXX
#define _REPORT_RPTUILIST_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

namespace rptui
{
class OObjectBase;

//============================================================================
// OObjectListener
//============================================================================

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
