/*************************************************************************
 *
 *  $RCSfile: VCollection.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:46:38 $
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

#ifndef _CPPUHELPER_IMPLBASE10_HXX_
#include <cppuhelper/implbase10.hxx>
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
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#include "connectivity/StdTypeDefs.hxx"
#endif


namespace connectivity
{
    namespace sdbcx
    {

        // the class OCollection is base class for collections :-)
        // all elements <strong> must </strong> support the XNamed interface

        typedef ::cppu::WeakImplHelper10< ::com::sun::star::container::XNameAccess,
                                         ::com::sun::star::container::XIndexAccess,
                                         ::com::sun::star::container::XEnumerationAccess,
                                         ::com::sun::star::container::XContainer,
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
        class SAL_NO_VTABLE OCollection : public OCollectionBase
        {
        protected:
            typedef ::std::multimap< ::rtl::OUString, Object_BASE, ::comphelper::UStringMixLess> ObjectMap;
            typedef ObjectMap::iterator ObjectIter;

        //  private:
            // this combination of map and vector is used to have a fast name and index access
            ::std::vector< ObjectIter >             m_aElements;        // hold the iterators which point to map
            ObjectMap                               m_aNameMap;         // hold the elements and a name

            ::cppu::OInterfaceContainerHelper       m_aContainerListeners;
            ::cppu::OInterfaceContainerHelper       m_aRefreshListeners;

        protected:
            ::cppu::OWeakObject&                    m_rParent;          // parent of the collection
            ::osl::Mutex&                           m_rMutex;           // mutex of the parent
            sal_Bool                                m_bUseIndexOnly;    // is only TRUE when only an indexaccess is needed

            // the implementing class should refresh their elements
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) = 0;

            // will be called when a object was requested by one of the accessing methods like getByIndex
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createObject(const ::rtl::OUString& _rName) = 0;

            // will be called when a new object should be generated by a call of createDataDescriptor
            // the returned object is empty will be filled outside and added to the collection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject();

            // return an object which is the copy of the argument
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > cloneObject(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);

            virtual void appendObject( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            // called when XDrop was called
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

            OCollection(::cppu::OWeakObject& _rParent,
                        sal_Bool _bCase,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector,
                        sal_Bool _bUseIndexOnly = sal_False);

            /** clear the name map
                <p>Does <em>not</em> dispose the objects hold by the collection.</p>
            */
            void clear_NoDispose();

            /**  insert a new element into the collection
            */
            void insertElement(const ::rtl::OUString& _sElementName,const Object_BASE& _xElement);

            /** return the name of element at index _nIndex
            */
            const ::rtl::OUString& getElementName(sal_Int32 _nIndex)
            {
                return m_aElements[_nIndex]->first;
            }

        public:
            virtual ~OCollection();
            DECLARE_SERVICE_INFO();


            void reFill(const TStringVector &_rVector);
            sal_Bool isCaseSensitive() const { return m_aNameMap.key_comp().isCaseSensitive(); }
            void renameObject(const ::rtl::OUString _sOldName,const ::rtl::OUString _sNewName);

            // only the name is identical to ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // dispatch the refcounting to the parent
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

            // ::com::sun::star::container::XElementAccess
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
            // ::com::sun::star::container::XIndexAccess
            virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

                // ::com::sun::star::container::XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XEnumerationAccess
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);
                        // ::com::sun::star::util::XRefreshable
            virtual void SAL_CALL refresh(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
            // XAppend
            virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
            // XDrop
            virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // ::com::sun::star::container::XContainer
            virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        private:
            void notifyElementRemoved(const ::rtl::OUString& _sName);
            void disposeElements();
            void dropImpl(sal_Int32 _nIndex,sal_Bool _bReallyDrop = sal_True);
            void dropImpl(const ObjectIter& _rCurrentObject,sal_Bool _bReallyDrop = sal_True);
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > getObject(ObjectIter& _rCurrentObject);
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_COLLECTION_HXX_


