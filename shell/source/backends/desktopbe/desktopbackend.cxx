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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <rtl/byteseq.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/diagnose_ex.hxx>
#include <uno/current_context.hxx>

namespace {

class Default:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::beans::XPropertySet >
{
public:
    Default() {}
    Default(const Default&) = delete;
    Default& operator=(const Default&) = delete;

private:
    virtual ~Default() override {}

    virtual OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.comp.configuration.backend.DesktopBackend"_ustr; }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return ServiceName == getSupportedServiceNames()[0]; }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override
    { return { u"com.sun.star.configuration.backend.DesktopBackend"_ustr }; }

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() override
    { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

    virtual void SAL_CALL setPropertyValue(
        OUString const &, css::uno::Any const &) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &) override
    {}

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &) override
    {}

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &) override
    {}

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &) override
    {}
};

void Default::setPropertyValue(OUString const &, css::uno::Any const &)
{
    throw css::lang::IllegalArgumentException(
        u"setPropertyValue not supported"_ustr,
        getXWeak(), -1);
}

OUString xdg_user_dir_lookup (const char *type, bool bAllowHomeDir)
{
    size_t nLenType = strlen(type);
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
        osl::FileBase::getFileURLFromSystemPath(u"/tmp"_ustr, aDocumentsDirURL);
        return aDocumentsDirURL;
    }

    config_home = getenv ("XDG_CONFIG_HOME");
    if (config_home == nullptr || config_home[0] == 0)
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
            int relative = 0;
            int len = seq.getLength();
            seq.realloc(len + 1);
            seq[len] = 0;

            p = reinterpret_cast<char *>(seq.getArray());
            while (*p == ' ' || *p == '\t')
                p++;
            if (strncmp (p, "XDG_", 4) != 0)
                continue;
            p += 4;
            if (strncmp (p, OString(type, nLenType).toAsciiUpperCase().getStr(), nLenType) != 0)
                continue;
            p += nLenType;
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
                aUserDirBuf = aHomeDirURL + "/";
            }
            else
            {
                aUserDirBuf.truncate();
            }
            while (*p && *p != '"')
            {
                if ((*p == '\\') && (*(p+1) != 0))
                    p++;
                aUserDirBuf.append(static_cast<sal_Unicode>(*p++));
            }
        }//end of while
        osl_closeFile(handle);
    }
    else
        bError = true;
    if (aUserDirBuf.getLength()>0 && !bError)
    {
        aDocumentsDirURL = aUserDirBuf.makeStringAndClear();
        if ( bAllowHomeDir ||
             (aDocumentsDirURL != aHomeDirURL && aDocumentsDirURL != Concat2View(aHomeDirURL + "/")) )
        {
            osl::Directory aDocumentsDir( aDocumentsDirURL );
            if( osl::FileBase::E_None == aDocumentsDir.open() )
                return aDocumentsDirURL;
        }
    }
    /* Use fallbacks historical compatibility if nothing else exists */
    return aHomeDirURL + "/" + OUString::createFromAscii(type);
}

css::uno::Any xdgDirectoryIfExists(char const * type, bool bAllowHomeDir) {
    auto url = xdg_user_dir_lookup(type, bAllowHomeDir);
    return css::uno::Any(
        osl::Directory(url).open() == osl::FileBase::E_None
        ? css::beans::Optional<css::uno::Any>(true, css::uno::Any(url))
        : css::beans::Optional<css::uno::Any>(false, css::uno::Any()));
}

css::uno::Any Default::getPropertyValue(OUString const & PropertyName)
{
    if (PropertyName == "TemplatePathVariable")
    {
        // Never pick up the HOME directory as the default location of user's templates
        return xdgDirectoryIfExists("Templates", false);
    }

    if (PropertyName == "WorkPathVariable")
    {
        return xdgDirectoryIfExists("Documents", true);
    }

    if ( PropertyName == "EnableATToolSupport" ||
         PropertyName == "ExternalMailer" ||
         PropertyName == "SourceViewFontHeight" ||
         PropertyName == "SourceViewFontName" ||
         PropertyName == "ooInetHTTPProxyName" ||
         PropertyName == "ooInetHTTPProxyPort" ||
         PropertyName == "ooInetHTTPSProxyName" ||
         PropertyName == "ooInetHTTPSProxyPort" ||
         PropertyName == "ooInetNoProxy" ||
         PropertyName == "ooInetProxyType" ||
         PropertyName == "givenname" ||
         PropertyName == "sn" )
    {
        return css::uno::Any(css::beans::Optional< css::uno::Any >());
    }

    throw css::beans::UnknownPropertyException(
        PropertyName, getXWeak());
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
    } catch (const css::uno::Exception &) {
        // Assuming these exceptions indicate that the service is not installed:
        TOOLS_WARN_EXCEPTION("shell", "createInstance(" << name << ") failed");
        return css::uno::Reference< css::uno::XInterface >();
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_DesktopBackend_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    OUString desktop;
    css::uno::Reference< css::uno::XCurrentContext > current(
        css::uno::getCurrentContext());
    if (current.is()) {
        current->getValueByName(u"system.desktop-environment"_ustr) >>= desktop;
    }

    // Fall back to the default if the specific backend is not available:
    css::uno::Reference< css::uno::XInterface > backend;
    if (desktop == "PLASMA5")
        backend = createBackend(context,
            u"com.sun.star.configuration.backend.KF5Backend"_ustr);
    if (!backend)
        backend = getXWeak(new Default);
    backend->acquire();
    return backend.get();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
