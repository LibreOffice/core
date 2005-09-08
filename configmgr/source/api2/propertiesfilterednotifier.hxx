/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertiesfilterednotifier.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:16:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_PROPERTIESFILTEREDNOTIFIER_HXX_
#define CONFIGMGR_PROPERTIESFILTEREDNOTIFIER_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif

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

