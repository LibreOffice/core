/*************************************************************************
 *
 *  $RCSfile: VCollection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 10:48:21 $
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

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#define _CONNECTIVITY_SDBCX_COLLECTION_HXX_

#ifndef _CPPUHELPER_IMPLBASE9_HXX_
#include <cppuhelper/implbase9.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif

namespace connectivity
{
    namespace sdbcx
    {

        // the class OCollection is base class for collections :-)
        // all elements <strong> must </strong> support the XNamed interface

        typedef ::cppu::WeakImplHelper9< ::com::sun::star::container::XNameAccess,
                                         ::com::sun::star::container::XIndexAccess,
                                         ::com::sun::star::container::XEnumerationAccess,
                                         ::com::sun::star::sdbc::XColumnLocate,
                                         ::com::sun::star::util::XRefreshable,
                                         ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                         ::com::sun::star::sdbcx::XAppend,
                                         ::com::sun::star::sdbcx::XDrop,
                                         ::com::sun::star::lang::XServiceInfo> OCollectionBase;

        typedef ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > Object_BASE;
        //************************************************************
        //  OCollection
        //************************************************************
        class OCollection : public OCollectionBase
        {
        protected:

            typedef ::std::map< ::rtl::OUString, Object_BASE, ::comphelper::UStringMixLess> ObjectMap;
            typedef ObjectMap::iterator ObjectIter;

            // this combination of map and vector is used to have a fast name and index access
            ::std::vector< ObjectIter >             m_aElements;        // hold the iterators which point to map
            ObjectMap                               m_aNameMap;         // hold the elements and a name

            ::cppu::OInterfaceContainerHelper       m_aRefreshListeners;
            ::cppu::OWeakObject&                    m_rParent;          // parent of the collection
            ::osl::Mutex&                           m_rMutex;           // mutex of the parent

            // the implementing class should refresh their elements
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) = 0;

            // will be called when a object was requested by one of the accessing methods like getByIndex
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createObject(const ::rtl::OUString& _rName) = 0;

            // will be called when a new object should be generated by a call of createDataDescriptor
            // the returned object is empty will be filled outside and added to the collection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject() = 0;

            OCollection(::cppu::OWeakObject& _rParent,sal_Bool _bCase, ::osl::Mutex& _rMutex,const ::std::vector< ::rtl::OUString> &_rVector)
                     : m_rParent(_rParent)
                     ,m_rMutex(_rMutex)
                     ,m_aRefreshListeners(_rMutex)
                     ,m_aNameMap(_bCase)
            {
                for(::std::vector< ::rtl::OUString>::const_iterator i=_rVector.begin(); i != _rVector.end();++i)
                    m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(*i,::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNamed >())));
            }

        public:
            virtual ~OCollection()
            {
            }

            DECLARE_SERVICE_INFO();

            sal_Bool isCaseSensitive() const { return m_aNameMap.key_comp().isCaseSensitive(); }

            // only the name is identical to ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // dispatch the refcounting to the parent
            virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)
            {
                m_rParent.acquire();
            }
            virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)
            {
                m_rParent.release();
            }

                // ::com::sun::star::container::XElementAccess
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                return::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*>(NULL));
            }

            virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                return getCount() > 0;
            }

                // ::com::sun::star::container::XIndexAccess
            virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                return m_aElements.size();
            }

            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

                // ::com::sun::star::container::XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException)
            {
                ::osl::MutexGuard aGuard(m_rMutex);
                return m_aNameMap.find(aName) != m_aNameMap.end();
            }
            // XEnumerationAccess
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);
                        // ::com::sun::star::util::XRefreshable
            virtual void SAL_CALL refresh(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException)
            {
                m_aRefreshListeners.addInterface(l);
            }
            virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException)
            {
                m_aRefreshListeners.removeInterface(l);
            }
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
            // XAppend
            virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
            // XDrop
            virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_COLLECTION_HXX_


