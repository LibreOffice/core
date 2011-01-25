/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <rtl/ref.hxx>

#include "updatecheckconfiglistener.hxx"
#include "updateinfo.hxx"

/* Interface to acess configuration data read-only */
struct IByNameAccess
{
    virtual ::com::sun::star::uno::Any getValue(const sal_Char * pName) = 0;
};

/* This helper class provides by name access to a sequence of named values */
class NamedValueByNameAccess : public IByNameAccess
{
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& m_rValues;

public:
    NamedValueByNameAccess(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rValues) :
        m_rValues(rValues) {} ;

    virtual ~NamedValueByNameAccess();

    virtual ::com::sun::star::uno::Any getValue(const sal_Char * pName);
};


/* This class encapsulates the configuration item actually used for storing the state
 * the update check is actually in.
 */
class UpdateCheckROModel
{
public:
    UpdateCheckROModel(IByNameAccess& aNameAccess) : m_aNameAccess(aNameAccess) {};

    bool isAutoCheckEnabled() const;
    bool isDownloadPaused() const;
    rtl::OUString getLocalFileName() const;
    sal_Int64 getDownloadSize() const;

    rtl::OUString getUpdateEntryVersion() const;
    void getUpdateEntry(UpdateInfo& rInfo) const;

private:

    rtl::OUString getStringValue(const sal_Char *) const;

    IByNameAccess& m_aNameAccess;
};



/* This class implements the non published UNO service com.sun.star.setup.UpdateCheckConfig,
 * which primary use is to be able to track changes done in the Toos -> Options page of this
 * component, as this is not supported by the OOo configuration for extendable groups.
 */

class UpdateCheckConfig : public ::cppu::WeakImplHelper3<
    ::com::sun::star::container::XNameReplace,
    ::com::sun::star::util::XChangesBatch,
    ::com::sun::star::lang::XServiceInfo >
{
    UpdateCheckConfig( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xContainer,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xAvailableUpdates,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xIgnoredUpdates,
                       const ::rtl::Reference< UpdateCheckConfigListener >& rListener );

    virtual ~UpdateCheckConfig();

public:

    static ::com::sun::star::uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    static ::rtl::Reference< UpdateCheckConfig > get(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const ::rtl::Reference< UpdateCheckConfigListener >& rListener = ::rtl::Reference< UpdateCheckConfigListener >());

    // Should really implement ROModel ..
    bool isAutoCheckEnabled() const;
    bool isAutoDownloadEnabled() const;
    rtl::OUString getUpdateEntryVersion() const;

    /* Updates the timestamp of last check, but does not commit the change
     * as either clearUpdateFound() or setUpdateFound() are expected to get
     * called next.
     */
    void updateLastChecked();

    /* Returns the date of the last successful check in seconds since 1970 */
    sal_Int64 getLastChecked() const;

    /* Returns configured check interval in seconds */
    sal_Int64 getCheckInterval() const;

    /* Reset values of previously remembered update
     */
    void clearUpdateFound();

    /* Stores the specified data of an available update
     */
    void storeUpdateFound(const UpdateInfo& rInfo, const rtl::OUString& aCurrentBuild);

    // Returns the local file name of a started download
    rtl::OUString getLocalFileName() const;

    // Returns the local file name of a started download
    rtl::OUString getDownloadDestination() const;

    // stores the local file name of a just started download
    void storeLocalFileName(const rtl::OUString& rFileName, sal_Int64 nFileSize);

    // Removes the local file name of a download
    void clearLocalFileName();

    // Stores the bool value for manually paused downloads
    void storeDownloadPaused(bool paused);

    // Returns the directory that acts as the user's desktop
    static rtl::OUString getDesktopDirectory();

    // Returns a directory accessible for all users
    static rtl::OUString getAllUsersDirectory();

    // store and retrieve information about extensions
    bool storeExtensionVersion( const rtl::OUString& rExtensionName,
                                const rtl::OUString& rVersion );
    bool checkExtensionVersion( const rtl::OUString& rExtensionName,
                                const rtl::OUString& rVersion );

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw (::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPendingChanges(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

private:

    static rtl::OUString getSubVersion( const rtl::OUString& rVersion, sal_Int32 *nIndex );
    static bool isVersionGreater( const rtl::OUString& rVersion1, const rtl::OUString& rVersion2 );

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xContainer;
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xAvailableUpdates;
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xIgnoredUpdates;
    const ::rtl::Reference< UpdateCheckConfigListener > m_rListener;
};

//------------------------------------------------------------------------------


template <typename T>
T getValue( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rNamedValues, const sal_Char * pszName )
    throw (::com::sun::star::uno::RuntimeException)
{
    for( sal_Int32 n=0; n < rNamedValues.getLength(); n++ )
    {
        // Unfortunatly gcc-3.3 does not like Any.get<T>();
        if( rNamedValues[n].Name.equalsAscii( pszName ) )
        {
            T value = T();
            if( ! (rNamedValues[n].Value >>= value) )
                throw ::com::sun::star::uno::RuntimeException(
                    ::rtl::OUString(
                        cppu_Any_extraction_failure_msg(
                            &rNamedValues[n].Value,
                            ::cppu::getTypeFavourUnsigned(&value).getTypeLibType() ),
                            SAL_NO_ACQUIRE ),
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

            return value;
        }
    }

    return T();
}
