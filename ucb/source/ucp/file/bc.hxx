/*************************************************************************
 *
 *  $RCSfile: bc.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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

#ifndef _BC_HXX_
#define _BC_HXX_

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFOCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFOCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertySetInfoChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_SHELL_HXX_
#include "shell.hxx"
#endif


namespace fileaccess {

    class PropertyListeners;
    class shell;
    class FileProvider;

    class BaseContent:
        public cppu::OWeakObject,
        public com::sun::star::lang::XComponent,
        public com::sun::star::ucb::XCommandProcessor,
        public com::sun::star::beans::XPropertiesChangeNotifier,
        public com::sun::star::beans::XPropertyContainer,
        public com::sun::star::beans::XPropertySetInfoChangeNotifier,
        public com::sun::star::ucb::XContentCreator,
        public com::sun::star::container::XChild,
        public com::sun::star::ucb::XContent,
        public fileaccess::Notifier    // implementation class
    {
    private:

        // A special creator for inserted contents; Creates an ugly object
        BaseContent( shell* pMyShell,
                     const rtl::OUString& parentName,
                     sal_Bool bFolder );

    public:
        BaseContent(
            shell* pMyShell,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xContentIdentifier,
            const rtl::OUString& aUnqPath );

        virtual ~BaseContent();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        release(
            void )
            throw( com::sun::star::uno::RuntimeException);


        // XComponent
        virtual void SAL_CALL
        dispose(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException );


        // XCommandProcessor
        virtual sal_Int32 SAL_CALL
        createCommandIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Any SAL_CALL
        execute(
            const com::sun::star::ucb::Command& aCommand,
            sal_Int32 CommandId,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::ucb::CommandAbortedException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        abort(
            sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );


        // XContent
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        getIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL
        getContentType(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addContentEventListener(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeContentEventListener(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        // XPropertiesChangeNotifier

        virtual void SAL_CALL
        addPropertiesChangeListener(
            const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removePropertiesChangeListener( const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
                                        const com::sun::star::uno::Reference<
                                        com::sun::star::beans::XPropertiesChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        // XPropertyContainer

        virtual void SAL_CALL
        addProperty(
            const rtl::OUString& Name,
            sal_Int16 Attributes,
            const com::sun::star::uno::Any& DefaultValue )
            throw( com::sun::star::beans::PropertyExistException,
                   com::sun::star::beans::IllegalTypeException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        removeProperty(
            const rtl::OUString& Name )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::NotRemoveableException,
                   com::sun::star::uno::RuntimeException );

        // XPropertySetInfoChangeNotifier

        virtual void SAL_CALL
        addPropertySetInfoChangeListener(
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removePropertySetInfoChangeListener(
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );


        // XContentCreator

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        createNewContent(
            const com::sun::star::ucb::ContentInfo& Info )
            throw( com::sun::star::uno::RuntimeException );


        // XChild
        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        getParent(
            void ) throw( com::sun::star::uno::RuntimeException );

        // Not supported
        virtual void SAL_CALL
        setParent( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& Parent )
            throw( com::sun::star::lang::NoSupportException,
                   com::sun::star::uno::RuntimeException);


        // Notifier

        ContentEventNotifier*          cDEL( void );
        ContentEventNotifier*          cEXC( const rtl::OUString aNewName );
        ContentEventNotifier*          cCEL( void );
        PropertySetInfoChangeNotifier* cPSL( void );
        PropertyChangeNotifier*        cPCL( void );
        rtl::OUString                  getKey( void );

    private:
        // Data members
        shell*                                                                      m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >   m_xContentIdentifier;
        rtl::OUString                                                               m_aUncPath;

        enum state { NameForInsertionSet = 1,
                     JustInserted = 2,
                     Deleted = 4,
                     FullFeatured = 8,
                     Connected = 16 };
        sal_Bool                                                                    m_bFolder;
        sal_uInt16                                                                  m_nState;

        vos::OMutex                         m_aMutex;

        osl::Mutex                          m_aEventListenerMutex;
        cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
        cppu::OInterfaceContainerHelper*    m_pContentEventListeners;
        cppu::OInterfaceContainerHelper*    m_pPropertySetInfoChangeListeners;
        PropertyListeners*                  m_pPropertyListener;


        // Private Methods
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo > SAL_CALL
        getCommandInfo(
            sal_Int32 nMyCommandIdentifier )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(
            sal_Int32 nMyCommandIdentifier )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& PropertySet )
            throw( com::sun::star::uno::RuntimeException );

        void SAL_CALL
        setPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Values )
            throw( com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet > SAL_CALL
        open(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::ucb::OpenCommandArgument2& aCommandArgument )
            throw( com::sun::star::ucb::CommandAbortedException );

        void SAL_CALL
        deleteContent( sal_Int32 nMyCommandIdentifier,
                       sal_Bool bDeleteArgument );

        void SAL_CALL
        transfer( sal_Int32 nMyCommandIdentifier,
                  const com::sun::star::ucb::TransferInfo& aTransferInfo )
            throw( com::sun::star::ucb::CommandAbortedException );

        void SAL_CALL
        insert( sal_Int32 nMyCommandIdentifier,
                const com::sun::star::ucb::InsertCommandArgument& aInsertArgument )
            throw( com::sun::star::ucb::CommandAbortedException );

        void SAL_CALL
        write( sal_Int32 nMyCommandIdentifier,
               sal_Bool OverWrite,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream )
            throw( com::sun::star::ucb::CommandAbortedException );

        friend class ContentEventNotifier;
    };

}             // end namespace fileaccess

#endif

