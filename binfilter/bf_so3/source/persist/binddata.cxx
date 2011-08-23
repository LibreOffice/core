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

#define _BINDDATA_CXX "$Revision: 1.4 $"

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _WILDCARD_HXX
#include <tools/wldcrd.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _BINDING_HXX
#include <binddata.hxx>
#endif
#ifndef _TRANSPRT_HXX
#include <bf_so3/transprt.hxx>
#endif
#ifndef _TRANSUNO_HXX
#include <transuno.hxx>
#endif
#ifndef _SO2DLL_HXX
#include <bf_so3/so2dll.hxx>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;

using rtl::OUString;

namespace binfilter {

typedef com::sun::star::uno::XInterface ifc_type;

#define SOAPP() SoDll::GetOrCreate()

/*========================================================================
 *
 * SvBindingData_Impl interface.
 *
 *======================================================================*/
class SvBindingData_Impl :
    public OWeakObject,
    public XPropertyChangeListener
{
public:
    SvBindingData_Impl (
        const Reference<XMultiServiceFactory> &rxFactory);

    void dispose (void);

    sal_Bool hasHttpCache (void);
    sal_Bool hasFtpProxy (void);
    sal_Bool shouldUseFtpProxy (const String &rUrl);

    /** XInterface.
     */
    virtual Any SAL_CALL queryInterface (
        const Type &rType) throw(RuntimeException);

    virtual void SAL_CALL acquire (void) throw();

    virtual void SAL_CALL release (void) throw();

    /** XEventEventListener.
     */
    void SAL_CALL disposing (
        const EventObject &rEvent) throw(RuntimeException);

    /** XPropertyChangeListener.
     */
    virtual void SAL_CALL propertyChange (
        const PropertyChangeEvent &rEvent) throw(RuntimeException);

protected:
    virtual ~SvBindingData_Impl (void);

    Reference<ifc_type> getConfigManager_Impl (void);

private:
    /** Representation.
     */
    Reference<XMultiServiceFactory> m_xFactory;
    Reference<ifc_type>             m_xManager;
    Reference<XContent>             m_xCache;

    String     m_aNoProxyList;
    String     m_aFtpProxyName;
    sal_uInt16 m_nFtpProxyPort;
    sal_uInt16 m_nProxyType;

    /** Initialization.
     */
    String readConfigKey_Impl (
        const Reference<XRegistryKey> &rxRootKey,
        const String                  &rKeyName);

    void readConfigManager_Impl (void);
    void initConfigManager_Impl (void);

    /** Not implemented.
     */
    SvBindingData_Impl (const SvBindingData_Impl&);
    SvBindingData_Impl& operator= (const SvBindingData_Impl&);
};

/*========================================================================
 *
 * SvBindingData implementation.
 *
 *======================================================================*/
/*
 * SvBindingData.
 */
SvBindingData::SvBindingData (void)
    : m_pImpl (NULL)
{
    Reference<XMultiServiceFactory> xFactory (
        SvBindingTransport_Impl::getProcessServiceFactory(), UNO_QUERY);
    if (xFactory.is())
    {
        m_pImpl = new SvBindingData_Impl (xFactory);
        m_pImpl->acquire();
    }
}

/*
 * ~SvBindingData.
 */
SvBindingData::~SvBindingData (void)
{
    if (m_pImpl)
    {
        m_pImpl->dispose();
        m_pImpl->release();
    }
}

/*
 * Get.
 */
SvBindingData* SvBindingData::Get (void)
{
    SvBindingData *&rpData = SOAPP()->pBindingData;
    if (rpData == NULL)
    {
        // Allocate data.
        rpData = new SvBindingData();

        // Register transport factories.
        new SvLockBytesTransportFactory;
        new CntTransportFactory;
    }
    return rpData;
}

/*
 * Delete.
 */
void SvBindingData::Delete (void)
{
    SvBindingData *&rpData = SOAPP()->pBindingData;
    DELETEZ (rpData);
}

/*
 * HasHttpCache.
 */
BOOL SvBindingData::HasHttpCache (void)
{
    if (m_pImpl)
        return m_pImpl->hasHttpCache();
    else
        return FALSE;
}

/*
 * ShouldUseFtpProxy.
 */
BOOL SvBindingData::ShouldUseFtpProxy (const String &rUrl)
{
    if (m_pImpl)
        return m_pImpl->shouldUseFtpProxy (rUrl);
    else
        return FALSE;
}

/*========================================================================
 *
 * SvBindingData_Impl internals.
 *
 *======================================================================*/
#define U2S(u) OUStringToString((u), CHARSET_SYSTEM)

#define STR_KEY_INET_PROXYTYPE     "INet/ProxyType"
#define STR_KEY_INET_NOPROXY       "INet/NoProxy"
#define STR_KEY_INET_FTPPROXYNAME  "INet/FTPProxyName"
#define STR_KEY_INET_FTPPROXYPORT  "INet/FTPProxyPort"

#define STRL_KEY_INET_PROXYTYPE    "inet/proxytype"
#define STRL_KEY_INET_NOPROXY      "inet/noproxy"
#define STRL_KEY_INET_FTPPROXYNAME "inet/ftpproxyname"
#define STRL_KEY_INET_FTPPROXYPORT "inet/ftpproxyport"

#define INIMANAGER_SERVICE_NAME \
    "com.sun.star.config.SpecialConfigManager"

/*========================================================================
 *
 * SvBindingData_Impl implementation.
 *
 *======================================================================*/
/*
 * SvBindingData_Impl.
 */
SvBindingData_Impl::SvBindingData_Impl (
    const Reference<XMultiServiceFactory> &rxFactory)
    : m_xFactory      (rxFactory),
      m_xCache        (NULL),
      m_nFtpProxyPort (0),
      m_nProxyType    (0)
{
}

/*
 * ~SvBindingData_Impl.
 */
SvBindingData_Impl::~SvBindingData_Impl (void)
{
}

/*
 * hasHttpCache.
 */
sal_Bool SvBindingData_Impl::hasHttpCache (void)
{
    if (!m_xCache.is())
    {
        m_xCache = SvBindingTransport_Impl::createContent (
            OUString::createFromAscii ("private:httpcache"));

        Reference<XCommandProcessor> xProcessor (m_xCache, UNO_QUERY);
        if (xProcessor.is())
        {
            Sequence<Property> aProps(3);
            aProps[0].Handle = aProps[1].Handle = aProps[2].Handle = -1;

            aProps[0].Name = OUString::createFromAscii ("ConnectionLimit");
            aProps[1].Name = OUString::createFromAscii ("SizeLimit");
            aProps[2].Name = OUString::createFromAscii ("Size");

            SvBindingTransport_Impl::getProperties (xProcessor, aProps);
        }
    }
    return m_xCache.is();
}

/*
 * hasFtpProxy.
 */
sal_Bool SvBindingData_Impl::hasFtpProxy (void)
{
    Reference<XConfigManager> xManager (getConfigManager_Impl(), UNO_QUERY);
    if (!xManager.is())
        return sal_False;

    if (!((m_aFtpProxyName.Len() > 0) && (m_nFtpProxyPort > 0)))
        return sal_False;

    return (m_nProxyType > 0);
}

/*
 * shouldUseFtpProxy.
 */
sal_Bool SvBindingData_Impl::shouldUseFtpProxy (const String &rUrl)
{
    // Check URL.
    INetURLObject aURL (rUrl);
    if (!(aURL.GetProtocol() == INET_PROT_FTP))
        return sal_False;

    if (!hasFtpProxy())
        return sal_False;

    if (m_aNoProxyList.Len())
    {
        // Setup Endpoint.
        String aEndpoint (aURL.GetHost());
        if (!aEndpoint.Len())
            return sal_False;

        aEndpoint += ':';
        if (aURL.HasPort())
            aEndpoint += String::CreateFromInt64(aURL.GetPort());
        else
            aEndpoint.AppendAscii( "21" );

        // Match NoProxyList.
        USHORT i, n = m_aNoProxyList.GetTokenCount (';');
        for (i = 0; i < n; i++)
        {
            String aWildToken (m_aNoProxyList.GetToken (i, ';'));
            if (aWildToken.Search(':') == STRING_NOTFOUND)
                aWildToken.AppendAscii( ":*" );

            WildCard aWildCard (aWildToken);
            if (aWildCard.Matches (aEndpoint))
                return sal_False;
        }
    }
    return sal_True;
}

/*
 * XInterface: queryInterface.
 */
Any SAL_CALL SvBindingData_Impl::queryInterface (
    const Type &rType) throw(RuntimeException)
{
    Any aRet (cppu::queryInterface (
        rType,
        SAL_STATIC_CAST(XEventListener*, this),
        SAL_STATIC_CAST(XPropertyChangeListener*, this)));
       return aRet.hasValue() ? aRet : OWeakObject::queryInterface (rType);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL SvBindingData_Impl::acquire (void) throw()
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL SvBindingData_Impl::release (void) throw()
{
    OWeakObject::release();
}

/*
 * XEventEventListener: disposing.
 */
void SAL_CALL SvBindingData_Impl::disposing (
    const EventObject & ) throw(RuntimeException)
{
    m_xManager = NULL;
}

/*
 * XPropertyChangeListener: propertyChange.
 */
void SAL_CALL SvBindingData_Impl::propertyChange (
    const PropertyChangeEvent &rEvent) throw(RuntimeException)
{
    OUString aKey (rEvent.PropertyName.toAsciiLowerCase());
    if (aKey.compareToAscii (STRL_KEY_INET_PROXYTYPE) == 0)
    {
        OUString aNewValue;
        if (rEvent.NewValue >>= aNewValue)
            m_nProxyType = (USHORT)aNewValue.toInt32();
        return;
    }
    if (aKey.compareToAscii (STRL_KEY_INET_NOPROXY) == 0)
    {
        OUString aNewValue;
        if (rEvent.NewValue >>= aNewValue)
            m_aNoProxyList = String( aNewValue );
        return;
    }
    if (aKey.compareToAscii (STRL_KEY_INET_FTPPROXYNAME) == 0)
    {
        OUString aNewValue;
        if (rEvent.NewValue >>= aNewValue)
            m_aFtpProxyName = String( aNewValue );
        return;
    }
    if (aKey.compareToAscii (STRL_KEY_INET_FTPPROXYPORT) == 0)
    {
        OUString aNewValue;
        if (rEvent.NewValue >>= aNewValue)
            m_nFtpProxyPort = (USHORT)aNewValue.toInt32();
        return;
    }
}

/*
 * getConfigManager_Impl.
 */
Reference<ifc_type> SvBindingData_Impl::getConfigManager_Impl (void)
{
    if (!m_xManager.is() && m_xFactory.is())
    {
        try
        {
            m_xManager = m_xFactory->createInstance (
                OUString::createFromAscii (INIMANAGER_SERVICE_NAME));
            if (m_xManager.is())
            {
                readConfigManager_Impl();
                initConfigManager_Impl();
            }
        }
        catch (Exception)
        {
        }
    }
    return m_xManager;
}

/*
 * readConfigKey_Impl.
 */
String SvBindingData_Impl::readConfigKey_Impl (
    const Reference<XRegistryKey> &rxRootKey,
    const String                  &rKeyName)
{
    if (rxRootKey.is())
    {
        try
        {
            Reference<XRegistryKey> xKey (
                rxRootKey->openKey (rKeyName));
            if (xKey.is())
                return xKey->getStringValue();
        }
        catch (InvalidValueException)
        {
        }
    }
    return String();
}

/*
 * readConfigManager_Impl.
 */
void SvBindingData_Impl::readConfigManager_Impl (void)
{
    Reference<XSimpleRegistry> xRegistry (m_xManager, UNO_QUERY);
    if (xRegistry.is())
    {
        try
        {
            Reference<XRegistryKey> xRootKey (xRegistry->getRootKey());
            if (xRootKey.is())
            {
                m_aNoProxyList = readConfigKey_Impl (
                    xRootKey, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STR_KEY_INET_NOPROXY ) ) );
                m_aFtpProxyName = readConfigKey_Impl (
                    xRootKey, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STR_KEY_INET_FTPPROXYNAME ) ) );
                m_nFtpProxyPort = (USHORT)readConfigKey_Impl (
                    xRootKey, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STR_KEY_INET_FTPPROXYPORT ) ) ).ToInt32();
                m_nProxyType = (USHORT)readConfigKey_Impl (
                    xRootKey, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STR_KEY_INET_PROXYTYPE ) ) ).ToInt32();
            }
        }
        catch (InvalidRegistryException)
        {
        }
    }
}

/*
 * initConfigManager_Impl.
 */
void SvBindingData_Impl::initConfigManager_Impl (void)
{
    Reference<XConfigManager> xConfig (m_xManager, UNO_QUERY);
    if (xConfig.is())
    {
        try
        {
            xConfig->addPropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_PROXYTYPE), this);
            xConfig->addPropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_NOPROXY), this);
            xConfig->addPropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_FTPPROXYNAME), this);
            xConfig->addPropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_FTPPROXYPORT), this);
        }
        catch (Exception)
        {
        }
    }
}

/*
 * dispose.
 */
void SvBindingData_Impl::dispose (void)
{
    Reference<XConfigManager> xConfig (m_xManager, UNO_QUERY);
    if (xConfig.is())
    {
        try
        {
            xConfig->removePropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_PROXYTYPE), this);
            xConfig->removePropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_NOPROXY), this);
            xConfig->removePropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_FTPPROXYNAME), this);
            xConfig->removePropertyChangeListener (
                OUString::createFromAscii (STR_KEY_INET_FTPPROXYPORT), this);
        }
        catch (Exception)
        {
        }
    }
}

}
