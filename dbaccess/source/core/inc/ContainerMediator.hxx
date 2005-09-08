/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContainerMediator.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:32:43 $
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
#ifndef DBA_CONTAINERMEDIATOR_HXX
#define DBA_CONTAINERMEDIATOR_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef DBA_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif


//........................................................................
namespace dbaccess
{
//........................................................................

    class OContainerMediator :   public ::comphelper::OBaseMutex
                                ,public ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener >
    {
        typedef ::std::pair<OPropertyForward*,::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> > TPropertyForward;
        DECLARE_STL_USTRINGACCESS_MAP(TPropertyForward,PropertyForwardList);
        PropertyForwardList                                                             m_aForwardList;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xSettings;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >     m_xContainer;
        sal_Bool                                                                        m_bTables;
    protected:
        virtual ~OContainerMediator();
    public:
        OContainerMediator(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xSettings
                        ,sal_Bool _bTables = sal_True);

        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void notifyElementCreated(const ::rtl::OUString& _sElementName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xElement);
    };
//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // DBA_CONTAINERMEDIATOR_HXX
