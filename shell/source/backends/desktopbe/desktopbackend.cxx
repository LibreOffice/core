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

#include "sal/config.h"

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implbase.hxx>
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/security.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/current_context.hxx"

namespace {

OUString SAL_CALL getDefaultImplementationName() {
    return OUString(
            "com.sun.star.comp.configuration.backend.DesktopBackend");
}

css::uno::Sequence< OUString > SAL_CALL getDefaultSupportedServiceNames() {
    OUString name(
            "com.sun.star.configuration.backend.DesktopBackend");
    return css::uno::Sequence< OUString >(&name, 1);
}

class Default:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::beans::XPropertySet >
{
public:
    Default() {}
    Default(const Default&) = delete;
    Default& operator=(const Default&) = delete;

private:
    virtual ~Default() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return getDefaultImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return ServiceName == getSupportedServiceNames()[0]; }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override
    { return getDefaultSupportedServiceNames(); }

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) override
    { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

    virtual void SAL_CALL setPropertyValue(
        OUString const &, css::uno::Any const &)
        throw (
            css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}
};

void Default::setPropertyValue(OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    throw css::lang::IllegalArgumentException(
        OUString("setPropertyValue not supported"),
        static_cast< cppu::OWeakObject * >(this), -1);
}

static OUString xdg_user_dir_lookup (const char *type)
{
    char *config_home;
    char *p;
    bool bError = false;

    osl::Security aSecurity;
    oslFileHandle handle;
    OUString aHomeDirURL;
    OUString aDocumentsDirURL;
    OUString aConfigFileURL;
    OUStringBuffer aUserDirBuf;

    if (!aSecurity.getHomeDir( aHomeDirURL ) )
    {
        osl::FileBase::getFileURLFromSystemPath(OUString("/tmp"), aDocumentsDirURL);
        return aDocumentsDirURL;
    }

    config_home = getenv ("XDG_CONFIG_HOME");
    if (config_home == NULL || config_home[0] == 0)
    {
        aConfigFileURL = aHomeDirURL + "/.config/user-dirs.dirs";
    }
    else
    {
        aConfigFileURL = OUString::createFromAscii(config_home) + "/user-dirs.dirs";
    }

    if(osl_File_E_None == osl_openFile(aConfigFileURL.pData, &handle, osl_File_OpenFlag_Read))
    {
        rtl::ByteSequence seq;
        while (osl_File_E_None == osl_readLine(handle , reinterpret_cast<sal_Sequence **>(&seq)))
        {
            /* Remove newline at end */
            int relative = 0;
            int len = seq.getLength();
            if(len>0 && seq[len-1] == '\n')
                seq[len-1] = 0;

            p = reinterpret_cast<char *>(seq.getArray());
            while (*p == ' ' || *p == '\t')
                p++;
            if (strncmp (p, "XDG_", 4) != 0)
                continue;
            p += 4;
            if (strncmp (p, type, strlen (type)) != 0)
                continue;
            p += strlen (type);
            if (strncmp (p, "_DIR", 4) != 0)
                continue;
            p += 4;
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p != '=')
                continue;
            p++;
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p != '"')
                continue;
            p++;
            if (strncmp (p, "$HOME/", 6) == 0)
            {
                p += 6;
                relative = 1;
            }
            else if (*p != '/')
                continue;
            if (relative)
            {
                aUserDirBuf = OUStringBuffer(aHomeDirURL + "/");
            }
            else
            {
                aUserDirBuf = OUStringBuffer();
            }
            while (*p && *p != '"')
            {
                if ((*p == '\\') && (*(p+1) != 0))
                    p++;
                aUserDirBuf.append((sal_Unicode)*p++);
            }
        }//end of while
        osl_closeFile(handle);
    }
    else
        bError = true;
    if (aUserDirBuf.getLength()>0 && !bError)
    {
        aDocumentsDirURL = aUserDirBuf.makeStringAndClear();
        osl::Directory aDocumentsDir( aDocumentsDirURL );
        if( osl::FileBase::E_None == aDocumentsDir.open() )
            return aDocumentsDirURL;
    }
    /* Use fallbacks historical compatibility if nothing else exists */
    return aHomeDirURL + "/" + OUString::createFromAscii(type);
}

css::uno::Any Default::getPropertyValue(OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    if (PropertyName == "TemplatePathVariable")
    {
        OUString aDirURL = xdg_user_dir_lookup("Templates");
        css::uno::Any aValue(aDirURL);
        return css::uno::makeAny(css::beans::Optional<css::uno::Any>(true, aValue));
    }

    if (PropertyName == "WorkPathVariable")
    {
        OUString aDirURL = xdg_user_dir_lookup("Documents");
        css::uno::Any aValue(aDirURL);
        return css::uno::makeAny(css::beans::Optional<css::uno::Any>(true, aValue));
    }

    if ( PropertyName == "EnableATToolSupport" ||
         PropertyName == "ExternalMailer" ||
         PropertyName == "SourceViewFontHeight" ||
         PropertyName == "SourceViewFontName" ||
         PropertyName == "ooInetFTPProxyName" ||
         PropertyName == "ooInetFTPProxyPort" ||
         PropertyName == "ooInetHTTPProxyName" ||
         PropertyName == "ooInetHTTPProxyPort" ||
         PropertyName == "ooInetHTTPSProxyName" ||
         PropertyName == "ooInetHTTPSProxyPort" ||
         PropertyName == "ooInetNoProxy" ||
         PropertyName == "ooInetProxyType" ||
         PropertyName == "givenname" ||
         PropertyName == "sn" )
    {
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    }

    throw css::beans::UnknownPropertyException(
        PropertyName, static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::uno::XInterface > createBackend(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    OUString const & name)
{
    try {
        return css::uno::Reference< css::lang::XMultiComponentFactory >(
            context->getServiceManager(), css::uno::UNO_SET_THROW)->
            createInstanceWithContext(name, context);
    } catch (css::uno::RuntimeException &) {
        // Assuming these exceptions are real errors:
        throw;
    } catch (const css::uno::Exception & e) {
        // Assuming these exceptions indicate that the service is not installed:
        OSL_TRACE(
            "createInstance(%s) failed with %s",
            OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return css::uno::Reference< css::uno::XInterface >();
    }
}

css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    OUString desktop;
    css::uno::Reference< css::uno::XCurrentContext > current(
        css::uno::getCurrentContext());
    if (current.is()) {
        current->getValueByName("system.desktop-environment") >>= desktop;
    }

    // Fall back to the default if the specific backend is not available:
    css::uno::Reference< css::uno::XInterface > backend;
    if ( desktop == "KDE" ) {
        backend = createBackend(
            context,
            "com.sun.star.configuration.backend.KDEBackend");
    } else if ( desktop == "KDE4" ) {
        backend = createBackend(
            context,
            "com.sun.star.configuration.backend.KDE4Backend");
    }
    return backend.is()
        ? backend : static_cast< cppu::OWeakObject * >(new Default);
}

static cppu::ImplementationEntry const services[] = {
    { &createInstance, &getDefaultImplementationName,
      &getDefaultSupportedServiceNames, &cppu::createSingleComponentFactory, nullptr,
      0 },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL desktopbe1_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
