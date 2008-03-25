/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: providerhelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:26:23 $
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

#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#define _UCBHELPER_PROVIDERHELPER_HXX

#ifndef __LIST__
#include <list>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

//=========================================================================

namespace com { namespace sun { namespace star { namespace ucb {
    class XPropertySetRegistry;
    class XPersistentPropertySet;
} } } }

namespace ucbhelper_impl { struct ContentProviderImplHelper_Impl; }

namespace ucbhelper {

//=========================================================================

class ContentImplHelper;
typedef rtl::Reference< ContentImplHelper > ContentImplHelperRef;
typedef std::list< ContentImplHelperRef > ContentRefList;

/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.ContentProvider. It provides contents derived from
  * class ucb::ContentImplHelper.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - maintains a set of ContentImplHelper objects, which were created by
  *   the provider implementation. So there will be exactly one object for
  *   one Content Identifier.
  * - Provides access to the Additional Core PropertySet of a content.
  *   ( These set contains the properties added to a content using its
  *   XPropertyContainer interface )
  */
class UCBHELPER_DLLPUBLIC ContentProviderImplHelper : public cppu::OWeakObject,
                                    public com::sun::star::lang::XTypeProvider,
                                    public com::sun::star::lang::XServiceInfo,
                                    public com::sun::star::ucb::XContentProvider
{
    friend class ContentImplHelper;

    ucbhelper_impl::ContentProviderImplHelper_Impl* m_pImpl;

protected:
    osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >  m_xSMgr;

private:
    UCBHELPER_DLLPRIVATE void removeContent( ContentImplHelper* pContent );

    UCBHELPER_DLLPRIVATE ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XPropertySetRegistry >
    getAdditionalPropertySetRegistry();

    UCBHELPER_DLLPRIVATE void cleanupRegisteredContents();

protected:
    /**
      * This method returns a content with the given id, if it already exists.
      * Use this method in your "queryContent" implementation to ensure unique
      * objects.
      *
      * @param  Identifier is the content identifier, for that an existing
      *         content object is requested.
      * @return the content with the given identifier, if it exists or 0, if it
      *         does not exist.
      */
    rtl::Reference< ContentImplHelper >
    queryExistingContent( const ::com::sun::star::uno::Reference<
                   ::com::sun::star::ucb::XContentIdentifier >& Identifier );

    /**
      * This method returns a content with the given URL, if it already exists.
      *
      * @param  rURL is the URL ( content identifier string ), for that an
      *         existing content object is requested.
      * @return the content with the given URL, if it exists or 0, if it
      *         does not exist.
      */
    rtl::Reference< ContentImplHelper >
    queryExistingContent( const ::rtl::OUString& rURL );

    /**
      * This method registers a newly created content instance with the
      * content provider. It should be called directly after creating a new
      * content instance. The provider can reuse a registered instance upon
      * subsedent requests for content instances with an idententifier
      * of a registered instance.
      * Note that the provider does not hold a hard reference on the
      * registered instance. If last external reference is gone, the provider
      * will remove the instance from its inventory of known instances.
      * Nothing will happen in case an already registered instance shall
      * be registered more than once.
      *
      * @param  the content instance that is to be registered.
     */
    void registerNewContent(
        const com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XContent > & xContent );

public:

    //////////////////////////////////////////////////////////////////////
    // Contsruction/Destruction
    //////////////////////////////////////////////////////////////////////

    ContentProviderImplHelper(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~ContentProviderImplHelper();

    //////////////////////////////////////////////////////////////////////
    // XInterface
    //////////////////////////////////////////////////////////////////////

    XINTERFACE_DECL()

    //////////////////////////////////////////////////////////////////////
    // XTypeProvider
    //////////////////////////////////////////////////////////////////////

    XTYPEPROVIDER_DECL()

    //////////////////////////////////////////////////////////////////////
    // XServiceInfo
    //////////////////////////////////////////////////////////////////////

    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException ) = 0;
    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException ) = 0;

    //////////////////////////////////////////////////////////////////////
    // XContentProvider
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns a content with the requested id.
      *
      * The implementation should:
      *
      * - Check, whether the Identifier is valid ( URL syntax ).
      * - Use queryExistingContent(...) to determine, whether there exists
      *   already a content with the given id.
      * - Return the possibly existing content.Create and return a new
      *   content, otherwise
      */
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException ) = 0;
    virtual sal_Int32 SAL_CALL
    compareContentIds( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ucb::XContentIdentifier >& Id1,
                       const ::com::sun::star::uno::Reference<
                               ::com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns a mutex, which protects the content list of the
      * provider. So you can prevent modifications of that list easyly.
      *
      * @return the mutex.
      */
    osl::Mutex& getContentListMutex() { return m_aMutex; }

    /**
      * This method fills a list with all contents existing at calling time.
      * Note: You may prevent modifications of the content list at any time
      * by acquiring the content list mutex of the provider.
      *
      * @param  rContents is the list to fill with the children.
      */
    void queryExistingContents( ContentRefList& rContents );

    /**
      * This method returns the propertyset containing the Additional Core
      * Properties of a content.
      *
      * @param  rKey is the key for the propertyset.
      * @param  bCreate is a flag indicating whether the propertyset shall
      *         be created in case it does not exist.
      * @return the propertyset containing the Additional Core Properties.
      */
    ::com::sun::star::uno::Reference<
        com::sun::star::ucb::XPersistentPropertySet >
    getAdditionalPropertySet( const ::rtl::OUString& rKey, sal_Bool bCreate );

    /**
      * This method renames the propertyset containing the Additional Core
      * Properties of a content.
      *
      * @param  rOldKey is the old key of the propertyset.
      * @param  rNewKey is the new key for the propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be renamed, too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    sal_Bool renameAdditionalPropertySet( const ::rtl::OUString& rOldKey,
                                          const ::rtl::OUString& rNewKey,
                                          sal_Bool bRecursive );

    /**
      * This method copies the propertyset containing the Additional Core
      * Properties of a content.
      *
      * @param  rSourceKey is the key of the source propertyset.
      * @param  rTargetKey is the key of the target propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rSourceKey shall be copied, too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    sal_Bool copyAdditionalPropertySet( const ::rtl::OUString& rSourceKey,
                                        const ::rtl::OUString& rTargetKey,
                                        sal_Bool bRecursive );

    /**
      * This method removes the propertyset containing the Additional Core
      * Properties of a content.
      *
      * @param  rKey is the key of the propertyset.
      * @param  bRecursive is a flag indicating whether propertysets for
      *         children described by rOldKey shall be removed, too.
      * @return True, if the operation succeeded - False, otherwise.
      */
    sal_Bool removeAdditionalPropertySet( const ::rtl::OUString& rKey,
                                          sal_Bool bRecursive );
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_PROVIDERHELPER_HXX */
