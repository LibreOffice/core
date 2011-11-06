/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CONNECTIVITY_ADO_COLLECTION_HXX_
#define _CONNECTIVITY_ADO_COLLECTION_HXX_

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "ado/Awrapadox.hxx"
#include "ado/Aolevariant.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace ado
    {
        namespace starcontainer = ::com::sun::star::container;
        namespace starlang      = ::com::sun::star::lang;
        namespace staruno       = ::com::sun::star::uno;
        namespace starbeans     = ::com::sun::star::beans;

        typedef ::cppu::WeakImplHelper3< starcontainer::XNameAccess,
                                         starcontainer::XIndexAccess,
                                         starlang::XServiceInfo> OCollectionBase;

        //************************************************************
        //  OCollection
        //************************************************************
        template <class T,class SimT,class OCl> class OCollection : public OCollectionBase
        {
        private:
            OCollection( const OCollection& );              // never implemented
            OCollection& operator=( const OCollection& );   // never implemented

        protected:
            vector<OCl*>                            m_aElements;
            ::cppu::OWeakObject&                    m_rParent;
            ::osl::Mutex&                           m_rMutex;       // mutex of the parent
            T*                                      m_pCollection;


        public:
            OCollection(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,T* _pCollection)
                     : m_rParent(_rParent)
                     ,m_rMutex(_rMutex)
                     ,m_pCollection(_pCollection)
            {
                m_pCollection->AddRef();
            }

            ~OCollection()
            {
                m_pCollection->Release();
            }

            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (staruno::RuntimeException)
            {
                return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ACollection");
            }
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& _rServiceName ) throw(staruno::RuntimeException)
            {
                staruno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
                const ::rtl::OUString* pSupported = aSupported.getConstArray();
                for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
                    if (pSupported->equals(_rServiceName))
                        return sal_True;

                return sal_False;
            }
            virtual staruno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(staruno::RuntimeException)
            {
                staruno::Sequence< ::rtl::OUString > aSupported(1);
                aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Container");
                return aSupported;
            }

            // dispatch the refcounting to the parent
            virtual void SAL_CALL acquire() throw()
            {
                m_rParent.acquire();
            }
            virtual void SAL_CALL release() throw()
            {
                m_rParent.release();
            }

        // ::com::sun::star::container::XElementAccess
            virtual staruno::Type SAL_CALL getElementType(  ) throw(staruno::RuntimeException)
            {
                return::getCppuType(static_cast< staruno::Reference< starbeans::XPropertySet>*>(NULL));
            }

            virtual sal_Bool SAL_CALL hasElements(  ) throw(staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                return getCount() > 0;
            }

        // starcontainer::XIndexAccess
            virtual sal_Int32 SAL_CALL getCount(  ) throw(staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                sal_Int32 nCnt = 0;
                m_pCollection->get_Count(&nCnt);
                return nCnt;
            }

            virtual staruno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                if (Index < 0 || Index >= getCount())
                    throw starlang::IndexOutOfBoundsException();
                SimT* pCol = NULL;
                m_pCollection->get_Item(OLEVariant(Index),&pCol);
                if(!pCol)
                    throw starlang::IndexOutOfBoundsException();

                OCl* pIndex = new OCl(pCol);

                m_aElements.push_back(pIndex);

                return staruno::makeAny( staruno::Reference< starbeans::XPropertySet >(pIndex));
            }


        // starcontainer::XNameAccess
            virtual staruno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);

                SimT* pCol = NULL;
                m_pCollection->get_Item(OLEVariant(aName),&pCol);
                if(!pCol)
                    throw starlang::IndexOutOfBoundsException();

                OCl* pIndex = new OCl(pCol);

                m_aElements.push_back(pIndex);

                return staruno::makeAny( staruno::Reference< starbeans::XPropertySet >(pIndex));
            }
            virtual staruno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                sal_Int32 nLen = getCount();
                staruno::Sequence< ::rtl::OUString > aNameList(nLen);

                ::rtl::OUString* pStringArray = aNameList.getArray();
                OLEVariant aVar;
                for (sal_Int32 i=0;i<nLen;++i)
                {
                    aVar.setInt32(i);
                    SimT* pIdx = NULL;
                    m_pCollection->get_Item(aVar,&pIdx);
                    pIdx->AddRef();
                    _bstr_t sBSTR;
                    pIdx->get_Name(&sBSTR);
                    (*pStringArray) = (sal_Unicode*)sBSTR;
                    pIdx->Release();
                    ++pStringArray;
                }
                return aNameList;
            }
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(staruno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                SimT* pCol = NULL;
                m_pCollection->get_Item(OLEVariant(aName),&pCol);
                return pCol != NULL;
            }

            void SAL_CALL disposing()
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                for (::std::vector<OCl*>::const_iterator i = m_aElements.begin(); i != m_aElements.end(); ++i)
                {
                    (*i)->disposing();
                    (*i)->release();
                }
                m_aElements.clear();
            }

        };

        class OIndex;
        class OKey;
        class OColumn;
        class OTable;
        class OView;
        class OGroup;
        class OUser;

        typedef OCollection< ADOIndexes,ADOIndex,OIndex>    OIndexes;
        typedef OCollection< ADOKeys,ADOKey,OKey>           OKeys;
        typedef OCollection< ADOColumns,ADOColumn,OColumn>  OColumns;
        typedef OCollection< ADOTables,ADOTable,OTable>     OTables;
        typedef OCollection< ADOViews,ADOView,OView>        OViews;
        typedef OCollection< ADOGroups,ADOGroup,OGroup>     OGroups;
        typedef OCollection< ADOUsers,ADOUser,OUser>        OUsers;

    }
}
#endif // _CONNECTIVITY_ADO_COLLECTION_HXX_



