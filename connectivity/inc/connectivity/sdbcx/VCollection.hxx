/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCollection.hxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:17:23 $
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
#include <memory>


namespace connectivity
{
    namespace sdbcx
    {

        // the class OCollection is base class for collections :-)
        typedef ::cppu::ImplHelper10< ::com::sun::star::container::XNameAccess,
                                         ::com::sun::star::container::XIndexAccess,
                                         ::com::sun::star::container::XEnumerationAccess,
                                         ::com::sun::star::container::XContainer,
                                         ::com::sun::star::sdbc::XColumnLocate,
                                         ::com::sun::star::util::XRefreshable,
                                         ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                         ::com::sun::star::sdbcx::XAppend,
                                         ::com::sun::star::sdbcx::XDrop,
                                         ::com::sun::star::lang::XServiceInfo> OCollectionBase;

        typedef ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > ObjectType;

        class SAL_NO_VTABLE IObjectCollection
        {
        public:
            virtual void reserve(size_t nLength) = 0;
            virtual bool exists(const ::rtl::OUString& _sName ) = 0;
            virtual bool empty() = 0;
            virtual void swapAll() = 0;
            virtual void swap() = 0;
            virtual void clear() = 0;
            virtual void reFill(const TStringVector &_rVector) = 0;
            virtual void insert(const ::rtl::OUString& _sName,const ObjectType& _xObject) = 0;
            virtual bool rename(const ::rtl::OUString _sOldName,const ::rtl::OUString _sNewName) = 0;
            virtual sal_Int32 size() = 0;
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > getElementNames() = 0;
            virtual ::rtl::OUString getName(sal_Int32 _nIndex) = 0;
            virtual void disposeAndErase(sal_Int32 _nIndex) = 0;
            virtual void disposeElements() = 0;
            virtual sal_Int32 findColumn( const ::rtl::OUString& columnName ) = 0;
            virtual ::rtl::OUString findColumnAtIndex(  sal_Int32 _nIndex) = 0;
            virtual ObjectType getObject(sal_Int32 _nIndex) = 0;
            virtual ObjectType getObject(const ::rtl::OUString& columnName) = 0;
            virtual void setObject(sal_Int32 _nIndex,const ObjectType& _xObject) = 0;
            virtual sal_Bool isCaseSensitive() const = 0;
        };
        //************************************************************
        //  OCollection
        //************************************************************
        class SAL_NO_VTABLE OCollection : public OCollectionBase
        {
        protected:
            ::std::auto_ptr<IObjectCollection>      m_pElements;

            ::cppu::OInterfaceContainerHelper       m_aContainerListeners;
            ::cppu::OInterfaceContainerHelper       m_aRefreshListeners;

        protected:
            ::cppu::OWeakObject&                    m_rParent;          // parent of the collection
            ::osl::Mutex&                           m_rMutex;           // mutex of the parent
            sal_Bool                                m_bUseIndexOnly;    // is only TRUE when only an indexaccess is needed

            // the implementing class should refresh their elements
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) = 0;

            // will be called when a object was requested by one of the accessing methods like getByIndex
            virtual ObjectType createObject(const ::rtl::OUString& _rName) = 0;

            // will be called when a new object should be generated by a call of createDataDescriptor
            // the returned object is empty will be filled outside and added to the collection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();

            /** appends an object described by a descriptor, under a given name
                @param _rForName
                    is the name under which the object should be appended. Guaranteed to not be empty.
                    This is passed for convenience only, since it's the result of a call of
                    getNameForObject for the given descriptor
                @param descriptor
                    describes the object to append
                @return
                    the new object which is to be inserted into the collection. This might be the result
                    of a call of <code>createObject( _rForName )</code>, or a clone of the descriptor.
            */
            virtual ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );

            // called when XDrop was called
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

            /** returns the name for the object. The default implementation ask for the property NAME. If this doesn't satisfy, it has to be overloaded.
                @param  _xObject    The object where the name should be extracted.
                @return The name of the object.
            */
            virtual ::rtl::OUString getNameForObject(const ObjectType& _xObject);

            /** clones the given descriptor

                The method calls createDescriptor to create a new, empty descriptor, and then copies all properties from
                _descriptor to the new object, which is returned.

                This method might come handy in derived classes for implementing appendObject, when the object
                is not actually appended to any backend (e.g. for the columns collection of a descriptor object itself,
                where there is not yet a database backend to append the column to).
            */
            ObjectType cloneDescriptor( const ObjectType& _descriptor );

            OCollection(::cppu::OWeakObject& _rParent,
                        sal_Bool _bCase,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector,
                        sal_Bool _bUseIndexOnly = sal_False,
                        sal_Bool _bUseHardRef = sal_True);

            /** clear the name map
                <p>Does <em>not</em> dispose the objects hold by the collection.</p>
            */
            void clear_NoDispose();

            /**  insert a new element into the collection
            */
            void insertElement(const ::rtl::OUString& _sElementName,const ObjectType& _xElement);

            /** return the name of element at index _nIndex
            */
            inline ::rtl::OUString getElementName(sal_Int32 _nIndex)
            {
                return m_pElements->findColumnAtIndex(_nIndex);
            }


            /** return the object, if not existent it creates it.
                @param  _nIndex
                    The index of the object to create.
                @return ObjectType
            */
            ObjectType getObject(sal_Int32 _nIndex);

        public:
            virtual ~OCollection();
            DECLARE_SERVICE_INFO();

            void reFill(const TStringVector &_rVector);
            inline sal_Bool isCaseSensitive() const { return m_pElements->isCaseSensitive(); }
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
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_COLLECTION_HXX_


