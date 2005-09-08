/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: provider.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:19:16 $
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

#ifndef CONFIGMGR_API_PROVIDER_HXX_
#define CONFIGMGR_API_PROVIDER_HXX_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HDL_
#include <com/sun/star/lang/IllegalArgumentException.hdl>
#endif
#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#include "confsvccomponent.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;
    namespace beans = css::beans;
    namespace util = css::util;
    using ::rtl::OUString;
    using ::vos::ORef;

    class Module;
    class ContextReader;
    class OProviderImpl;

    typedef ::cppu::ImplHelper4 <    lang::XMultiServiceFactory
                                    ,lang::XLocalizable
                                    ,util::XRefreshable
                                    ,util::XFlushable
                                >   OProvider_Base;

    //==========================================================================
    //= OProvider
    //==========================================================================
    /** Base class to receive access to the configuration data. A provider for configuration is
        a factory for service which allow a readonly or update access to the configuration trees.
    */
    class OProvider : protected ServiceComponentImpl
                    , protected ::comphelper::OPropertyContainer
            // don't want to allow our derivees to access the mutex of ServiceComponentImpl
            // (this helps to prevent deadlocks: The mutex of the base class is used by the OComponentHelper to
            // protect addEvenetListener calls. Unfortunately these calls are made from within API object we create,
            // too, which can lead do deadlocks ....)
                    , public OProvider_Base
    {
//      friend class OProviderFactory;
        friend class OProviderDisposingListener;

    protected:
        uno::Reference< uno::XComponentContext >            m_xContext;
        uno::Reference< lang::XEventListener >              m_xDisposeListener;

    public:
        // make ServiceComponentImpl allocation functions public
        static void * SAL_CALL operator new( size_t nSize ) throw()
        { return ServiceComponentImpl::operator new( nSize ); }
        static void SAL_CALL operator delete( void * pMem ) throw()
            { ServiceComponentImpl::operator delete( pMem ); }

        typedef uno::Reference< uno::XComponentContext > CreationContext;

        OProvider(CreationContext const & xContext, ServiceImplementationInfo const* pInfo);
        virtual ~OProvider();

        /// XTypeOProvider
        virtual uno::Sequence< uno::Type > SAL_CALL getTypes(  ) throw(uno::RuntimeException);

        /// XInterface
        virtual void SAL_CALL acquire() throw() { ServiceComponentImpl::acquire(); }
        virtual void SAL_CALL release() throw() { ServiceComponentImpl::release(); }
        virtual uno::Any SAL_CALL queryInterface(uno::Type const& rType) throw (uno::RuntimeException);

        // XPropertySet
        virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(uno::RuntimeException);

        // OPropertSetHelper
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const uno::Any& rValue
                                                 )
                                                 throw (uno::Exception)
        {
            OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, rValue);
        }

        static sal_Int32 countServices(ServiceRegistrationInfo const* aInfo) { return ServiceRegistrationHelper(aInfo).countServices(); }

    protected:
        // creates a new session
        void implConnect(OProviderImpl& rFreshProviderImpl, const ContextReader& _rSettings) throw (uno::Exception);

    protected:
        // disambuiguated access
        cppu::OBroadcastHelper & getBroadcastHelper()
        { return ServiceComponentImpl::rBHelper; }

        /// Component Helper override
        virtual void SAL_CALL disposing();
        virtual void SAL_CALL disposing(lang::EventObject const& rEvt) throw();


        // OPropertyContainer
        void    registerProperty(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
                                 void* _pPointerToMember, const uno::Type& _rMemberType)
            { OPropertyContainer::registerProperty(_rName, _nHandle, _nAttributes, _pPointerToMember, _rMemberType);}

        void    describeProperties(uno::Sequence< beans::Property >& /* [out] */ _rProps) const
            { OPropertyContainer::describeProperties(_rProps);}


        uno::Any SAL_CALL queryPropertyInterface(uno::Type const& rType) throw (uno::RuntimeException)
        { return OPropertyContainer::queryInterface(rType);}

    private:
        void attachToContext();
        uno::Reference< lang::XComponent > releaseContext();
        void discardContext(uno::Reference< lang::XComponent > const & xContext);
    };

} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDER_HXX_


