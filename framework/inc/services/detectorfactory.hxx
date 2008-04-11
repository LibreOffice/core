/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: detectorfactory.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_DETECTORFACTORY_HXX_
#define __FRAMEWORK_SERVICES_DETECTORFACTORY_HXX_

//_______________________________________________
// includes of own project

#include <classes/filtercache.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <general.h>

//_______________________________________________
// includes of interfaces
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XFlushable.hpp>

//_______________________________________________
// includes of other projects
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// exported const

//_______________________________________________
// exported definitions

/// @HTML
/** @short      factory to create detect service objects and initialize it in the right way.

    @descr      This class can be used to create new detect services for specified contents.
                It uses cached values of the configuration to lay down, which detector match
                a given name. Further this class provides full access to the configuration data
                of such detect services and following implementations will support some special
                query modes.

    @author     as96863

    @docdate    07.03.2003 by as96863

    @todo       <ul>
                    <li>implementation of query mode</li>
                    <li>simple restore mechanism of last consistent cache state,
                        if flush failed</li>
                </ul>
 */
/// @NOHTML

class DetectorFactory : // interfaces
                        public  css::lang::XTypeProvider            ,
                        public  css::lang::XServiceInfo             ,
                        public  css::lang::XMultiServiceFactory     ,
                        public  css::container::XNameContainer      ,       // => XNameReplace => XNameAccess => XElementAccess
                        public  css::util::XFlushable               ,
                        // base classes
                        // Order is neccessary for right initialization of it!
                        private ThreadHelpBase                      ,
                        public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        /** reference to the global uno service manager.
            It will be used to create own needed services on demand. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** singleton, which contains all needed configuration data and provides
            read/write access on it. */
        FilterCache m_aCache;

        /** contains all registered listener. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_aListenerContainer;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        // XInterface, XTypeProvider, XServiceInfo

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        #ifdef ENABLE_AUTODOC_FIX
        ;
        #endif

        //---------------------------------------

        /** @short  initialize new instance of this class.

            @param  xSMGR
                    reference to the global uno service manager, which created this new factory instance.
                    It must be used during runtime to create own needed services.
         */

        DetectorFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

        //---------------------------------------

        /** @short  release internal structures.
         */

        virtual ~DetectorFactory();

        //---------------------------------------
        // XMultiServiceFactory

        /** @short  create a new detect service and initialize it with it's own configuration data.

            @param  sName
                    means the uno implementation name of a detect service.

            @exception  com::sun::star::uno::Exception
                        if the requested service could not be created or initialized.
         */

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& sName )
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //---------------------------------------
        // XMultiServiceFactory

        /** @short  does the same as createInstance() method, but initialize created detect service with some
                    additional data.

            @param  sName
                    means the uno implementation name of a detect service.

            @param  lArguments
                    the optional arguments, which are passed to the created detect service against
                    it's own configuration data.

            @exception  com::sun::star::uno::Exception
                        if the requested service could not be created or initialized.
         */

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString&                     sName      ,
                                                                                                  const css::uno::Sequence< css::uno::Any >& lArguments )
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //---------------------------------------
        // XMultiServiceFactory

        /** @short  return list of all well know detect services, which can be created by this factory.

            @attention  Because this service implements read/write access to the configuration too,
                        this list is dynamic. Means: in multithreaded environments some items of this
                        return list could be invalid next time!

            @return A list of all registered detect services.
         */

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XNameContainer

        /** @short  add new detect service entry to this container.

            @descr  This will change all internal structures only!
                    Updating of the configuration layer and notify of all registered
                    listener will be done inside API call XFlushable::flush() on this
                    container.

            @param  sName
                    means the uno implementation name of this new detect service entry.

            @param  aPropertySet [sequence< com::sun::star::beans::PropertyValue >!]
                    describe this new entry. For a list of all supported properties
                    have a look on method >>getByName()<<.
                    Note: Missing values will be created with defaults!

            @exception  com::sun::star::lang::IllegalArgumentException
                        if one of the incoming parameters seams to be invalid.
                        That doesn't include the check, if this item already exist!

            @exception  com::sun::star::container::ElementExistException
                        if this item already exist inside this container.

            @exception  com::sun::star::lang::WrappedTargetException
                        f creation of the internal structures failed.
         */

        virtual void SAL_CALL insertByName( const ::rtl::OUString& sName        ,
                                            const css::uno::Any&   aPropertySet )
            throw(css::lang::IllegalArgumentException  ,
                  css::container::ElementExistException,
                  css::lang::WrappedTargetException    ,
                  css::uno::RuntimeException           );

        //---------------------------------------
        // XNameContainer

        /** @short  remove a detect service entry from this container.

            @descr  This will change all internal structures only!
                    Updating of the configuration layer and notify of all registered
                    listener will be done inside API call XFlushable::flush() on this
                    container.

            @param  sName
                    means the uno implementation name of a detect service entry.

            @exception  com::sun::star::container::NoSuchElementException
                        if the requested item does not exist inside this container.

            @exception  com::sun::star::lang::WrappedTargetException
                        if creation of the internal structures failed.
         */

        virtual void SAL_CALL removeByName( const ::rtl::OUString& sName )
            throw(css::container::NoSuchElementException,
                  css::lang::WrappedTargetException     ,
                  css::uno::RuntimeException            );

        //---------------------------------------
        // XNameReplace

        /** @short  change a detect service entry inside this container.

            @descr  This will change all internal structures only!
                    Updating of the configuration layer and notify of all registered
                    listener will be done inside API call XFlushable::flush() on this
                    container.

            @param  sName
                    means the uno implementation name of a detect service entry.

            @param  aPropertySet [sequence< com::sun::star::beans::PropertyValue >!]
                    describe the changes on this entry. For a list of all supported properties
                    have a look on method getByName().
                    Note: Missing properties will be untouched.

            @exception  com::sun::star::lang::IllegalArgumentException
                        if one of the incoming parameters seams to be invalid.
                        That doesn't include the check, if this item exist!

            @exception  com::sun::star::container::NoSuchElementException
                        if the requested item does not exist inside this container.

            @exception  com::sun::star::lang::WrappedTargetException
                        if updating of the internal structures failed.
         */

        virtual void SAL_CALL replaceByName( const ::rtl::OUString& sName        ,
                                             const css::uno::Any&   aPropertySet )
            throw(css::lang::IllegalArgumentException   ,
                  css::container::NoSuchElementException,
                  css::lang::WrappedTargetException     ,
                  css::uno::RuntimeException            );

        //---------------------------------------
        // XNameAccess

        /// @HTML
        /** @short  return properties of queried detector item.

            @descr  Use this method to get all informations about a detector item of this cache.
                    A sequence< com::sun::star::beans::PropertyValue > packed inside an Any will be returned.
                    Following properties are defined:
                    <table border=1>
                    <tr>
                        <td><b>Property</b></td>
                        <td><b>Value<b></td>
                        <td><b>Description<b></td>
                    </tr>
                    <tr>
                        <td>Name</td>
                        <td>[string]</td>
                            <td>the uno implementation name for this detect service</td>
                    </tr>
                    <tr>
                        <td>Types</td>
                        <td>[sequence< string >]</td>
                        <td>a list of all internbal type names, which this detect service is registered for</td>
                    </tr>
                    </table>

            @param  sName
                    the uno implementation name the requested detector.

            @return A property set, which describe this detect service.
                    It uses a sequence< com::sun::star::beans::PropertyValue > internaly.

            @exception  com::sun::star::container::NoSuchElementException
                        if the requested entry does not exist inside this container.
         */
        /// @NOHTML

        virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& sName )
            throw(css::container::NoSuchElementException,
                  css::lang::WrappedTargetException     ,
                  css::uno::RuntimeException            );

        //---------------------------------------
        // XNameAccess

        /** @short  return list of all well know container entries available on this container.

            @attention  Because this service implements read/write access to the configuration too,
                        this list is dynamic. Means: in multithreaded environments some items of this
                        return list could be invalid next time!

            @return A list of all well known container items.
         */

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XNameAccess

        /** @short  check if searched entry exist inside this container.

            @attention  In multithreaded environments it's not guaranteed, that a
                        queried item exist next time realy! It can be deleted by
                        another thread ...

            @param  sName
                    the name of the queried container entry.

            @return TRUE if the requested item exist; FALSE otherwise.
         */

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& sName )
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XElementAccess

        /** @short  return the uno type, which is used for all container items.

            @return Type of sequence< com::sun::star::beans::PropertyValue > everytime - because it's fix.
         */

        virtual css::uno::Type SAL_CALL getElementType()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XElementAccess

        /** @short  return fill state of this cache.

            @return TRUE if any item exist inside this conatiner; FALSE otherwhise.
         */

        virtual sal_Bool SAL_CALL hasElements()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XFlushable

        /** @short  update the configuration layer and notify all registered listener.

            @descr  All container interface methods update the internal structures of
                    this container only. But the underlying configuration layer and
                    may some possible other caches was not updated.
                    Calling of flush() will do that.
                    At the same time all currently registered flush listener will be informed,
                    so they can update her structures too.
                    Note: Before all these operations are started realy, all changes will be
                    verified and if neccessary some corrections will be done.
                    In case the cache will be invalid and could not be repaired an exception is thrown!
                    Normaly this container will not work correctly afterwards ...

            @exception  com::sun::star::uno::RuntimeException
                        if the changes on this container was invald and could not be repaired.
         */

        virtual void SAL_CALL flush()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XFlushable

        /** @short  register listener for container updates.

            @param  xListener
                    reference to listener, which wish to be registered.

            @exception  com::sun::star::uno::RuntimeException
                        if the given listener is an invalid reference.
                        Note: multiple calls of this method for the same listener won't be checked!
         */

        virtual void SAL_CALL addFlushListener( const css::uno::Reference< css::util::XFlushListener >& xListener )
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XFlushable

        /** @short  deregister listener for container updates.

            @param  xListener
                    reference to listener, which wish to be deregistered.

            @exception  com::sun::star::uno::RuntimeException
                        if the given listener is an invalid reference.
                        Note: multiple calls of this method for the same listener won't be checked!
         */

        virtual void SAL_CALL removeFlushListener( const css::uno::Reference< css::util::XFlushListener >& xListener )
            throw(css::uno::RuntimeException);

}; // class DetectorFactory

} // namespace framework

#endif // #ifndef __FRAMEWORK_SERVICES_DETECTORFACTORY_HXX_
