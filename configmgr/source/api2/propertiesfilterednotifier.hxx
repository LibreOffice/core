/*************************************************************************
 *
 *  $RCSfile: propertiesfilterednotifier.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

