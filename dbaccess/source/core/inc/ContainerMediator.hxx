/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContainerMediator.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:40:45 $
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <map>

//........................................................................
namespace dbaccess
{
//........................................................................

    class OPropertyForward;

    class OContainerMediator :   public ::comphelper::OBaseMutex
                                ,public ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener >
    {
    public:
        enum ContainerType
        {
            eColumns,
            eTables
        };

    private:
        typedef ::rtl::Reference< OPropertyForward >                TPropertyForward;
        typedef ::std::map< ::rtl::OUString, TPropertyForward >     PropertyForwardList;
        PropertyForwardList                                                             m_aForwardList;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xSettings;    // can not be weak
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >     m_xContainer;   // can not be weak
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_aConnection;
        ContainerType                                                                   m_eType;

    protected:
        virtual ~OContainerMediator();

    public:
        OContainerMediator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xSettings,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            ContainerType _eType
       );

        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void notifyElementCreated(const ::rtl::OUString& _sElementName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xElement);

    private:
        /** cleans up the instance, by deregistering as listener at the containers,
            and resetting them to <NULL/>
        */
        void    impl_cleanup_nothrow();

        /** retrieves the settings object to initialize a container element

            Normally, this object will simply retrieve the object with the given name from our settings
            container. Hiowever, for columns, there's a fallback in case this settings object does
            not yet exist: Then, we check if the given destination object refers to a table column, via its
            TableName and RealName property. If so, this table column is used as initialization object.

            @param _rName
                the name of the destination object in its container
            @param _rxDestination
                the destination object to initialize
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                impl_getSettingsForInitialization_nothrow(
                    const ::rtl::OUString& _rName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDestination
                ) const;
    };
//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // DBA_CONTAINERMEDIATOR_HXX
