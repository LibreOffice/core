/*************************************************************************
 *
 *  $RCSfile: OSubComponent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:17:31 $
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
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#define _CONNECTIVITY_OSUBCOMPONENT_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace cppu {
    class IPropertyArrayHelper;
}

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XComponent;
            }
        }
    }
}
namespace connectivity
{

    namespace skeleton
    {
        void release(oslInterlockedCount& _refCount,
                     ::cppu::OBroadcastHelper& rBHelper,
                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                     ::com::sun::star::lang::XComponent* _pObject);

        void checkDisposed(sal_Bool _bThrow) throw ( ::com::sun::star::lang::DisposedException );
        //************************************************************
        // OSubComponent
        //************************************************************
        template <class SELF, class WEAK> class OSubComponent
        {
        protected:
            // the parent must support the tunnel implementation
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;
            SELF*   m_pDerivedImplementation;

        public:
            OSubComponent(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent,
                    SELF* _pDerivedImplementation)
                :m_xParent(_xParent)
                ,m_pDerivedImplementation(_pDerivedImplementation)
            {
            }

        protected:
            void dispose_ChildImpl()
            {
                ::osl::MutexGuard aGuard( m_pDerivedImplementation->rBHelper.rMutex );
                m_xParent = NULL;
            }
            void relase_ChildImpl()
            {
                release(m_pDerivedImplementation->m_refCount,
                                        m_pDerivedImplementation->rBHelper,
                                        m_xParent,
                                        m_pDerivedImplementation);

                m_pDerivedImplementation->WEAK::release();
            }
        };


        template <class TYPE>
        class OPropertyArrayUsageHelper
        {
        protected:
            static sal_Int32                        s_nRefCount;
            static ::cppu::IPropertyArrayHelper*    s_pProps;
            static ::osl::Mutex                     s_aMutex;

        public:
            OPropertyArrayUsageHelper();
            virtual ~OPropertyArrayUsageHelper()
            {   // ARGHHHHHHH ..... would like to implement this in proparrhlp_impl.hxx (as we do with all other methods)
                // but SUNPRO 5 compiler (linker) doesn't like this
                ::osl::MutexGuard aGuard(s_aMutex);
                OSL_ENSURE(s_nRefCount > 0, "OPropertyArrayUsageHelper::~OPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
                if (!--s_nRefCount)
                {
                    delete s_pProps;
                    s_pProps = NULL;
                }
            }

            /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
                class, which is created if neccessary.
            */
            ::cppu::IPropertyArrayHelper*   getArrayHelper();

        protected:
            /** used to implement the creation of the array helper which is shared amongst all instances of the class.
                This method needs to be implemented in derived classes.
                <BR>
                The method gets called with s_aMutex acquired.
                <BR>
                as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
                assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
                @return                         an pointer to the newly created array helper. Must not be NULL.
            */
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const = 0;
        };

#ifndef MACOSX

        template<class TYPE>
        sal_Int32                       OPropertyArrayUsageHelper< TYPE >::s_nRefCount  = 0;

        template<class TYPE>
        ::cppu::IPropertyArrayHelper*   OPropertyArrayUsageHelper< TYPE >::s_pProps = NULL;

        template<class TYPE>
        ::osl::Mutex                    OPropertyArrayUsageHelper< TYPE >::s_aMutex;

#endif /* MACOSX */

        //------------------------------------------------------------------
        template <class TYPE>
        OPropertyArrayUsageHelper<TYPE>::OPropertyArrayUsageHelper()
        {
            ::osl::MutexGuard aGuard(s_aMutex);
            ++s_nRefCount;
        }

        //------------------------------------------------------------------
        template <class TYPE>
        ::cppu::IPropertyArrayHelper* OPropertyArrayUsageHelper<TYPE>::getArrayHelper()
        {
            OSL_ENSURE(s_nRefCount, "OPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
            if (!s_pProps)
            {
                ::osl::MutexGuard aGuard(s_aMutex);
                if (!s_pProps)
                {
                    s_pProps = createArrayHelper();
                    OSL_ENSURE(s_pProps, "OPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
                }
            }
            return s_pProps;
        }



        class OBase_Mutex
        {
        public:
            ::osl::Mutex m_aMutex;
        };

        namespace internal
        {
            template <class T>
            void implCopySequence(const T* _pSource, T*& _pDest, sal_Int32 _nSourceLen)
            {
                for (sal_Int32 i=0; i<_nSourceLen; ++i, ++_pSource, ++_pDest)
                    *_pDest = *_pSource;
            }
        }
        //-------------------------------------------------------------------------
        /// concat two sequences
        template <class T>
        ::com::sun::star::uno::Sequence<T> concatSequences(const ::com::sun::star::uno::Sequence<T>& _rLeft, const ::com::sun::star::uno::Sequence<T>& _rRight)
        {
            sal_Int32 nLeft(_rLeft.getLength()), nRight(_rRight.getLength());
            const T* pLeft = _rLeft.getConstArray();
            const T* pRight = _rRight.getConstArray();

            sal_Int32 nReturnLen(nLeft + nRight);
            ::com::sun::star::uno::Sequence<T> aReturn(nReturnLen);
            T* pReturn = aReturn.getArray();

            internal::implCopySequence(pLeft, pReturn, nLeft);
            internal::implCopySequence(pRight, pReturn, nRight);

            return aReturn;
        }


#define DECLARE_SERVICE_INFO()  \
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException); \
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) \

#define IMPLEMENT_SERVICE_INFO(classname, implasciiname, serviceasciiname)  \
    ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)   \
    {   \
        return ::rtl::OUString::createFromAscii(implasciiname); \
    }   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname); \
        return aSupported;  \
    }   \
    sal_Bool SAL_CALL classname::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
    {   \
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());             \
        const ::rtl::OUString* pSupported = aSupported.getConstArray();                 \
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();              \
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)   \
            ;                                                                           \
                                                                                        \
        return pSupported != pEnd;                                                      \
    }   \


    }
}
#endif // _CONNECTIVITY_OSUBCOMPONENT_HXX_

