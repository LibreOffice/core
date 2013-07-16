/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#ifndef _CONNECTIVITY_FSUBCOMPONENT_HXX_
#define _CONNECTIVITY_FSUBCOMPONENT_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/propshlp.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>

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

    namespace firebird
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

        template<class TYPE>
        sal_Int32                       OPropertyArrayUsageHelper< TYPE >::s_nRefCount  = 0;

        template<class TYPE>
        ::cppu::IPropertyArrayHelper*   OPropertyArrayUsageHelper< TYPE >::s_pProps = NULL;

        template<class TYPE>
        ::osl::Mutex                    OPropertyArrayUsageHelper< TYPE >::s_aMutex;

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



    }
}
#endif // _CONNECTIVITY_FSUBCOMPONENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
