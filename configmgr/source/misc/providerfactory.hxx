/*************************************************************************
 *
 *  $RCSfile: providerfactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-03 16:33:58 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _CONFIGMGR_PROVIDER_FACTORY_HXX_
#define _CONFIGMGR_PROVIDER_FACTORY_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include "confapifactory.hxx"

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

//........................................................................
namespace configmgr
{
//........................................................................

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        createProviderFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const ::rtl::OUString& _rComponentName,
            ::configmgr::ProviderInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames
        );

    class ConnectionSettings;

    //====================================================================
    //= OProviderFactory
    //====================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XSingleServiceFactory > OProviderFactory_Base;
    /** a special factory for the configuration provider, which implements some kind of
        "shared multiple instances" factory.
    */

    class OProviderFactory : public OProviderFactory_Base
    {
        friend class ODisposingListener;
    protected:
        ::osl::Mutex                        m_aMutex;
        ProviderInstantiation               m_pObjectCreator;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > m_xEventListener; // must be the first uno::object

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                            m_xDefaultProvider;
        ConnectionSettings*                 m_pPureSettings;
                                                // the pure settings, not overwritten by any runtime arguments

        typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >   ProviderReference;
        DECLARE_STL_USTRINGACCESS_MAP(ProviderReference, UserSpecificProviders);
        DECLARE_STL_USTRINGACCESS_MAP(UserSpecificProviders, SessionSpecificProviders);
        SessionSpecificProviders    m_aProviders;

    public:
        OProviderFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ProviderInstantiation _pObjectCreator);
        ~OProviderFactory();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createProvider();
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createProviderWithArguments(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments);

    protected:
        void    ensureDefaultProvider();

        // from the given map, extract a provider for the given user. (if necessary, create one and insert it into the map)
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > implGetProvider(
                    const ConnectionSettings& _rSettings,
                    sal_Bool _bCreateWithPassword
                );

        // to be called with m:aMutex locked
        void ensureSettings();
        void disposing(com::sun::star::lang::EventObject const& rEvt) throw();
    };

//........................................................................
}   // namespace configmgr
//........................................................................

#endif // _CONFIGMGR_PROVIDER_FACTORY_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/01/29 08:51:11  dg
 *  #82336# invalid syntax for template
 *
 *  Revision 1.2  2001/01/26 07:54:21  lla
 *  #82734# disposing with lasy writing necessary
 *
 *  Revision 1.1  2000/12/01 13:53:17  fs
 *  initial checkin - afctory for configuration provider(s)
 *
 *
 *  Revision 1.0 30.11.00 19:03:57  fs
 ************************************************************************/

