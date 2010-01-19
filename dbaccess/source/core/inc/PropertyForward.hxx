/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyForward.hxx,v $
 * $Revision: 1.5 $
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
#ifndef DBA_PROPERTYSETFORWARD_HXX
#define DBA_PROPERTYSETFORWARD_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#include <vector>

//........................................................................
namespace dbaccess
{
//........................................................................

    // ===================================================================
    // = OPropertyForward
    // ===================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;
    class OPropertyForward  :public ::comphelper::OBaseMutex
                            ,public OPropertyForward_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDestInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xDestContainer;
        ::rtl::OUString     m_sName;
        sal_Bool            m_bInInsert;

    protected:
        virtual ~OPropertyForward();

    public:
        OPropertyForward( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSource,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xDestContainer,
                          const ::rtl::OUString& _sName,
                          const ::std::vector< ::rtl::OUString >& _aPropertyList
                         );

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        inline void setName( const ::rtl::OUString& _sName ) { m_sName = _sName; }
        void setDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDest);
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDefinition() const { return m_xDest; }
    };

//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // DBA_PROPERTYSETFORWARD_HXX

