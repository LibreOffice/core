/*************************************************************************
 *
 *  $RCSfile: contenthelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#define _UCBHELPER_CONTENTHELPER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFOCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFOCHANGENOTIFIER_HPP_
#include <com/sun/star/ucb/XCommandInfoChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandInfo;
    class XPersistentPropertySet;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySetInfo;
} } } }

namespace ucb_impl { struct ContentImplHelper_Impl; }

namespace ucb
{

//=========================================================================

struct PropertyInfoTableEntry
{
    const sal_Char*                     pName;
    sal_Int32                           nHandle;
    const ::com::sun::star::uno::Type*  pType;
    sal_Int16                           nAttributes;
};

//=========================================================================

struct CommandInfoTableEntry
{
    const sal_Char*                     pName;
    sal_Int32                           nHandle;
    const ::com::sun::star::uno::Type*  pArgType;
};

//=========================================================================

class ContentProviderImplHelper;

/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.Content. Implementations derived from this class are
  * objects provided by implementations derived from
  * class ucb::ContentProviderImplHelper.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - all required interfaces for service com::sun::star::ucb::Content
  * - all required listener containers
  *   ( XComponent, XPropertiesChangeNotifier, XPropertySetInfoChangeNotifier,
  *     XCommandInfoChangeNotifier )
  * - XPropertyContainer implementation ( persistence is implemented using
  *   service com.sun.star.ucb.Store )
  * - complete XPropertySetInfo implementation ( including Additioanl Core
  *   Properties supplied via XPropertyContainer interface )
  *   -> protected method: getPropertySetInfo
  * - complete XCommandInfo implementation
  *    -> protected method: getCommandInfo
  */
class ContentImplHelper :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XContent,
                public com::sun::star::ucb::XCommandProcessor,
                public com::sun::star::beans::XPropertiesChangeNotifier,
                public com::sun::star::beans::XPropertyContainer,
                public com::sun::star::beans::XPropertySetInfoChangeNotifier,
                public com::sun::star::ucb::XCommandInfoChangeNotifier,
                public com::sun::star::container::XChild
{
    friend class PropertySetInfo;
    friend class CommandProcessorInfo;

    ucb_impl::ContentImplHelper_Impl* m_pImpl;

protected:
    osl::Mutex                       m_aMutex;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                     m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >
                                     m_xIdentifier;
    vos::ORef< ContentProviderImplHelper >
                                     m_xProvider;
    sal_uInt32                       m_nCommandId;

private:
    /**
      * Your implementation of this method must return a table containing
      * the meta data of the properties supported by the content.
      * Note: If you wish to provide your own implementation of the interface
      * XPropertyContainer ( completely override addContent and removeContent
      * implementation of this base class in this case ), you can supply the
      * meta data for your Additional Core Properties here to get a fully
      * featured getPropertySetInfo method ( see below ).
      *
      * @return a table containing the property meta data.
      */
    virtual const ::ucb::PropertyInfoTableEntry& getPropertyInfoTable() = 0;

    /**
      * Your implementation of this method must return a table containing
      * the meta data of the commands supported by the content.
      *
      * @return a table containing the command meta data.
      */
    virtual const ::ucb::CommandInfoTableEntry&  getCommandInfoTable() = 0;

    /**
      * The implementation of this method shall return the URL of the parent
      * of your content.
      *
      * @return the URL of the parent content or an empty string.
      *         Note that not all contents must have one parent. There may
      *         be contents with no parent. In that case an empty string must
      *         be returned. If your content has more than one parent you may
      *         return the URL of one "preferred" parent or an empty string.
      */
    virtual ::rtl::OUString getParentURL() = 0;

protected:
    /**
      * This method returns complete meta data for the properties ( including
      * Additional Core Properties supplied via XPropertyContainer interface )
      * supported by the content. To implement the required command
      * "getPropertySetInfo" simply return the return value of this method.
      *
      * @return an XPropertySetInfo implementation object containing meta data
      *         for the properties supported by this content.
      */
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
    getPropertySetInfo();

    /**
      * This method returns complete meta data for the commands supported by
      * the content. To implement the required command "getCommandInfo" simply
      * return the return value of this method.
      *
      * @return an XCommandInfo implementation object containing meta data
      *         for the commands supported by this content.
      */
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo >
    getCommandInfo();

    /**
      * This method can be used to propagate changes of property values.
      *
      * @param evt is a sequence of property change events.
      */
    void notifyPropertiesChange(
        const com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyChangeEvent >& evt ) const;

    /**
      * This method can be used to propagate changes of the propertyset
      * info of your content (i.e. this happens if a new property is added
      * to your content via its XPropertyContainer interface). This base class
      * automatically generates events when the propertyset info changes. If
      * you provide your own implementations of addproperty and removeProperty,
      * then you must call "notifyPropertySetInfoChange" by yourself.
      *
      * @param evt is a sequence of property change events.
      */
    void notifyPropertySetInfoChange(
        const com::sun::star::beans::PropertySetInfoChangeEvent& evt ) const;

    /**
      * This method can be used to propagate changes of the command info of
      * your content. This can happen at any time if there shall be a new
      * command available at a content or a currently present command shall no
      * longer be present. (i.e. only if the content count of a trash can
      * object is greater then zero, there will be available a  command
      * "emptyTrash". If there are no objects in the trash can, this command
      * won't be available.
      *
      * @param evt is a sequence of command info change events.
      */
    void notifyCommandInfoChange(
            const com::sun::star::ucb::CommandInfoChangeEvent& evt ) const;

    /**
      * This method can be used to propagate content events.
      *
      * @param evt is a sequence of content events.
      */
    void notifyContentEvent(
            const com::sun::star::ucb::ContentEvent& evt ) const;

    /**
      * Use this method to announce the insertion of this content at
      * the end of your implementation of the command "insert". The
      * implementation of is method propagates a ContentEvent( INSERTED ).
      */
    void inserted();

    /**
      * Use this method to announce the destruction of this content at
      * the end of your implementation of the command "delete". The
      * implementation of is method propagates a ContentEvent( DELETED )
      * and a ContentEvent( REMOVED ) at the parent of the deleted content,
      * if a parent exists.
      */
    void deleted();

    /**
      * Use this method to change the identity of a content. The implementation
      * of this method will replace the content identifier of the content and
      * propagate the appropriate ContentEvent( EXCHANGED ).
      *
      * @param  rNewId is the new content identifier for the contant.
      * @return a success indicator.
      */
    sal_Bool exchange( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& rNewId );

    /**
      * Use this method to get access to the Additional Core Properties of
      * the content ( added using content's XPropertyContainer interface ).
      * If you supply your own XPropertyContainer implementation, this method
      * will always return an empty propertyset.
      *
      * @param  bCreate indicates whether a new propertyset shall be created
      *         if it does not exist.
      * @return the implementation of the service
      *         com.sun.star.ucb.PersistentPropertySet.
      */
    com::sun::star::uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
    getAdditionalPropertySet( sal_Bool bCreate );

    /**
      * This method renames the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  rOldKey is the old key of the propertyset.
      * @param  rNewKey is the new key for the propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be renamed too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    sal_Bool renameAdditionalPropertySet( const ::rtl::OUString& rOldKey,
                                          const ::rtl::OUString& rNewKey,
                                          sal_Bool bRecursive );

    /**
      * This method removes the propertyset containing the Additional Core
      * Properties of the content.
      *
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be removed too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    sal_Bool removeAdditionalPropertySet( sal_Bool bRecursive );

public:
    /**
      * Constructor.
      *
      * Note that the implementation of this ctor registers itself at its
      * content provider. The provider implementation inserts the content
      * in a hash map. So it easyly can be found and reused when the provider
      * is asked for a content.
      *
      * @param rxSMgr is a Service Manager.
      * @param rxProvider is the provider for the content.
      * @param Identifier is the content identifier for the content.
      * @param bRegisterAtProvider can be used to create a content that
      *        will not autmatically register itself at its provider. This
      *        can be usefull if creating a content object from inside a
      *        XContentCreator::createNewContent method. That new content
      *        would register at its provider after it has successfully
      *        processed its "insert" command".
      */
    ContentImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const vos::ORef< ContentProviderImplHelper >& rxProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            sal_Bool bRegisterAtProvider = sal_True );

    /**
      * Destructor.
      *
      * Note that the implementation of this dtor deregisters itself from its
      * content provider. The provider implementation removes the content
      * from a hash map.
      */
    virtual ~ContentImplHelper();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException ) = 0;
    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException ) = 0;

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XContent
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException ) = 0;
    virtual void SAL_CALL
    addContentEventListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeContentEventListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException ) = 0;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException ) = 0;

    // XPropertiesChangeNotifier
    virtual void SAL_CALL
    addPropertiesChangeListener(
        const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
         const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removePropertiesChangeListener(
        const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
        const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XCommandInfoChangeNotifier
    virtual void SAL_CALL
    addCommandInfoChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeCommandInfoChangeListener(
        const com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XPropertyContainer

    /**
      * This method adds a property to the content according to the interface
      * specification. The properties will be stored using the service
      * com.sun.star.ucb.Store.
      *
      * Note: You may provide your own implementation of this method, for
      * instance, if your data source supports adding/removing of properties.
      * Don't forget to return the meta data for these properties in your
      * implementation of getPropertyInfoTable.
      */
    virtual void SAL_CALL
    addProperty( const rtl::OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException );

    /**
      * This method removes a property from the content according to the
      * interface specification. The properties will be stored using the
      * service com.sun.star.ucb.Store.
      *
      * Note: You may provide your own implementation of this method, for
      * instance, if your data source supports adding/removing of properties.
      * Don't forget to return the meta data for these properties in your
      * implementation of getPropertyInfoTable.
      */
    virtual void SAL_CALL
    removeProperty( const rtl::OUString& Name )
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

    // XChild

    /**
      * This method returns the content representing the parent of a content,
      * if such a parent exists. The implementation of this method uses your
      * implementation of getParentURL.
      */
    virtual com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > SAL_CALL
    getParent()
        throw( com::sun::star::uno::RuntimeException );

    /**
      * The implementation of this method always throws a NoSupportException.
      */
    virtual void SAL_CALL
    setParent( const com::sun::star::uno::Reference<
                        com::sun::star::uno::XInterface >& Parent )
        throw( com::sun::star::lang::NoSupportException,
               com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns the provider of the content.
      *
      * @return the provider of the content.
      */
    const vos::ORef< ContentProviderImplHelper >& getProvider() const
    { return m_xProvider; }
};

} // namespace ucb

#endif /* !_UCBHELPER_CONTENTHELPER_HXX */
