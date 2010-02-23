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

#ifndef CONFIGMGR_PROPERTIESFILTEREDNOTIFIER_HXX_
#define CONFIGMGR_PROPERTIESFILTEREDNOTIFIER_HXX_

#include <com/sun/star/beans/XPropertiesChangeListener.hpp>

#include <vos/refernce.hxx>

namespace configmgr
{
    class PropertiesFilteredNotifier
    : public ::com::sun::star::beans::XPropertiesChangeListener
    {
    public:
        PropertiesFilteredNotifier(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener > const& xTarget,
            ::com::sun::star::uno::Sequence< ::rtl::OUString > const& aFilterNames
        );
        virtual ~PropertiesFilteredNotifier();

    public:
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        virtual ::com::sun::star::uno::Any SAL_CALL
            queryInterface( const ::com::sun::star::uno::Type& aType )
                throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
            disposing( const ::com::sun::star::lang::EventObject& Source )
                throw(::com::sun::star::uno::RuntimeException) ;

        virtual void SAL_CALL
            propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt )
                throw(::com::sun::star::uno::RuntimeException);
    private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >
            implFilter(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt)
                const;

        bool implAccept(const ::com::sun::star::beans::PropertyChangeEvent& evt) const;

        ::vos::ORefCount m_aRefCount;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener > m_xTarget;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aFilterNames;
    };
}

#endif // CONFIGMGR_PROPERTIESFILTEREDNOTIFIER_HXX_

