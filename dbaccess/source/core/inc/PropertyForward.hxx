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
#ifndef DBA_PROPERTYSETFORWARD_HXX
#define DBA_PROPERTYSETFORWARD_HXX

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <vector>

namespace dbaccess
{

    // OPropertyForward
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;
    class OPropertyForward  :public ::comphelper::OBaseMutex
                            ,public OPropertyForward_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDestInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xDestContainer;
        OUString     m_sName;
        sal_Bool            m_bInInsert;

    protected:
        virtual ~OPropertyForward();

    public:
        OPropertyForward( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSource,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xDestContainer,
                          const OUString& _sName,
                          const ::std::vector< OUString >& _aPropertyList
                         );

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        inline void setName( const OUString& _sName ) { m_sName = _sName; }
        void setDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDest);
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDefinition() const { return m_xDest; }
    };

}   // namespace dbaccess

#endif // DBA_PROPERTYSETFORWARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
