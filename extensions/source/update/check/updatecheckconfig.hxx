/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATECHECKCONFIG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATECHECKCONFIG_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <rtl/ref.hxx>

#include "updatecheckconfiglistener.hxx"
#include "updateinfo.hxx"

/* This helper class provides by name access to a sequence of named values */
class NamedValueByNameAccess
{
    const css::uno::Sequence< css::beans::NamedValue >& m_rValues;

public:
    explicit NamedValueByNameAccess(
        const css::uno::Sequence< css::beans::NamedValue >& rValues) :
        m_rValues(rValues) {} ;

    ~NamedValueByNameAccess();

    css::uno::Any getValue(const sal_Char * pName);
};


/* This class encapsulates the configuration item actually used for storing the state
 * the update check is actually in.
 */
class UpdateCheckROModel
{
public:
    explicit UpdateCheckROModel(NamedValueByNameAccess& aNameAccess) : m_aNameAccess(aNameAccess) {};

    bool isAutoCheckEnabled() const;
    bool isDownloadPaused() const;
    OUString getLocalFileName() const;
    sal_Int64 getDownloadSize() const;

    OUString getUpdateEntryVersion() const;
    void getUpdateEntry(UpdateInfo& rInfo) const;

private:

    OUString getStringValue(const sal_Char *) const;

    NamedValueByNameAccess& m_aNameAccess;
};


/* This class implements the non published UNO service com.sun.star.setup.UpdateCheckConfig,
 * which primary use is to be able to track changes done in the Tools -> Options page of this
 * component, as this is not supported by the OOo configuration for extendable groups.
 */

class UpdateCheckConfig : public ::cppu::WeakImplHelper<
    css::container::XNameReplace,
    css::util::XChangesBatch,
    css::lang::XServiceInfo >
{
    UpdateCheckConfig( const css::uno::Reference< css::container::XNameContainer >& xContainer,
                       const css::uno::Reference< css::container::XNameContainer >& xAvailableUpdates,
                       const css::uno::Reference< css::container::XNameContainer >& xIgnoredUpdates,
                       const ::rtl::Reference< UpdateCheckConfigListener >& rListener );

    virtual ~UpdateCheckConfig();

public:

    static css::uno::Sequence< OUString > getServiceNames();
    static OUString getImplName();

    static ::rtl::Reference< UpdateCheckConfig > get(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ::rtl::Reference< UpdateCheckConfigListener >& rListener = ::rtl::Reference< UpdateCheckConfigListener >());

    // Should really implement ROModel...
    bool isAutoCheckEnabled() const;
    bool isAutoDownloadEnabled() const;
    OUString getUpdateEntryVersion() const;

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
    void storeUpdateFound(const UpdateInfo& rInfo, const OUString& aCurrentBuild);

    // Returns the local file name of a started download
    OUString getLocalFileName() const;

    // Returns the local file name of a started download
    OUString getDownloadDestination() const;

    // stores the local file name of a just started download
    void storeLocalFileName(const OUString& rFileName, sal_Int64 nFileSize);

    // Removes the local file name of a download
    void clearLocalFileName();

    // Stores the bool value for manually paused downloads
    void storeDownloadPaused(bool paused);

    // Returns the directory that acts as the user's desktop
    static OUString getDesktopDirectory();

    // Returns a directory accessible for all users
    static OUString getAllUsersDirectory();

    // store and retrieve information about extensions
    bool storeExtensionVersion( const OUString& rExtensionName,
                                const OUString& rVersion );
    bool checkExtensionVersion( const OUString& rExtensionName,
                                const OUString& rVersion );

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw (css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasPendingChanges(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

private:

    static OUString getSubVersion( const OUString& rVersion, sal_Int32 *nIndex );
    static bool isVersionGreater( const OUString& rVersion1, const OUString& rVersion2 );

    const css::uno::Reference< css::container::XNameContainer > m_xContainer;
    const css::uno::Reference< css::container::XNameContainer > m_xAvailableUpdates;
    const css::uno::Reference< css::container::XNameContainer > m_xIgnoredUpdates;
    const ::rtl::Reference< UpdateCheckConfigListener > m_rListener;
};


template <typename T>
T getValue( const css::uno::Sequence< css::beans::NamedValue >& rNamedValues, const sal_Char * pszName )
    throw (css::uno::RuntimeException)
{
    for( sal_Int32 n=0; n < rNamedValues.getLength(); n++ )
    {
        // Unfortunately gcc-3.3 does not like Any.get<T>();
        if( rNamedValues[n].Name.equalsAscii( pszName ) )
        {
            T value = T();
            if( ! (rNamedValues[n].Value >>= value) )
                throw css::uno::RuntimeException(
                    OUString(
                        cppu_Any_extraction_failure_msg(
                            &rNamedValues[n].Value,
                            ::cppu::getTypeFavourUnsigned(&value).getTypeLibType() ),
                            SAL_NO_ACQUIRE ),
                    css::uno::Reference< css::uno::XInterface >() );

            return value;
        }
    }

    return T();
}

#endif // INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATECHECKCONFIG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
