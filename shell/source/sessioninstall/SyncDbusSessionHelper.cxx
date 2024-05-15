/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SyncDbusSessionHelper.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <gio/gio.h>
#include <cstring>
#include <memory>
#include <string_view>
#include <vector>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace
{
    struct GVariantDeleter { void operator()(GVariant* pV) { if (pV) g_variant_unref(pV); } };
    struct GVariantBuilderDeleter { void operator()(GVariantBuilder* pVB) { g_variant_builder_unref(pVB); } };
    template <typename T> struct GObjectDeleter { void operator()(T* pO) { g_object_unref(pO); } };
    class GErrorWrapper
    {
        GError* m_pError;
        public:
            explicit GErrorWrapper() : m_pError(nullptr) {}
            ~GErrorWrapper() noexcept(false)
            {
                if(!m_pError)
                    return;
                OUString sMsg(
                    m_pError->message, std::strlen(m_pError->message), RTL_TEXTENCODING_UTF8);
                g_error_free(m_pError);
                throw RuntimeException(sMsg);
            }
            GError*& getRef() { return m_pError; }
    };
    GDBusProxy* lcl_GetPackageKitProxy(std::u16string_view sInterface)
    {
        const OString sFullInterface = "org.freedesktop.PackageKit." + OUStringToOString(sInterface, RTL_TEXTENCODING_ASCII_US);
        GDBusProxy* proxy = nullptr;
        {
            GErrorWrapper error;
            proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                               G_DBUS_PROXY_FLAGS_NONE, nullptr,
                               "org.freedesktop.PackageKit",
                               "/org/freedesktop/PackageKit",
                               reinterpret_cast<const gchar*>(sFullInterface.getStr()),
                               nullptr,
                               &error.getRef());
        }
        if(!proxy)
            throw RuntimeException(u"couldn't get a proxy!"_ustr);
        return proxy;
    }

    GVariant* pk_make_platform_data()
    {
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        return g_variant_builder_end(&builder);
    }

void request(
    char const * method,
    css::uno::Sequence<OUString> const & resources,
    std::u16string_view interaction)
{
    // Keep strings alive until after call to g_dbus_proxy_call_sync
    std::vector<OString> resUtf8;
    std::shared_ptr<GVariantBuilder> builder(
        g_variant_builder_new(G_VARIANT_TYPE ("as")), GVariantBuilderDeleter());
    for (auto & i: resources) {
        auto s(OUStringToOString(i, RTL_TEXTENCODING_UTF8));
        resUtf8.push_back(s);
        g_variant_builder_add(builder.get(), "s", s.getStr());
    }
    auto iactUtf8(OUStringToOString(interaction, RTL_TEXTENCODING_UTF8));
    std::shared_ptr<GDBusProxy> proxy(
        lcl_GetPackageKitProxy(u"Modify2"), GObjectDeleter<GDBusProxy>());
    GErrorWrapper error;
    std::shared_ptr<GVariant> result(g_dbus_proxy_call_sync(
        proxy.get(), method,
        g_variant_new(
            "(asss@a{sv})", builder.get(), iactUtf8.getStr(),
            "libreoffice-startcenter.desktop", pk_make_platform_data()),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error.getRef()), GVariantDeleter());
}

}

namespace shell::sessioninstall
{
    SyncDbusSessionHelper::SyncDbusSessionHelper(Reference<XComponentContext> const&)
    {
#if !GLIB_CHECK_VERSION(2,36,0)
        g_type_init ();
#endif
    }

Sequence< OUString > SAL_CALL SyncDbusSessionHelper::getSupportedServiceNames()
{
    return { u"org.freedesktop.PackageKit.SyncDbusSessionHelper"_ustr };
}

OUString SAL_CALL SyncDbusSessionHelper::getImplementationName()
{
    return u"org.libreoffice.comp.shell.sessioninstall.SyncDbusSessionHelper"_ustr;
}

sal_Bool SAL_CALL SyncDbusSessionHelper::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

void SyncDbusSessionHelper::InstallPackageFiles(
    css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
{
    request("InstallPackageFiles", files, interaction);
}

void SyncDbusSessionHelper::InstallProvideFiles(
    css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
{
    request("InstallProvideFiles", files, interaction);
}

void SyncDbusSessionHelper::InstallCatalogs(
    css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
{
    request("InstallCatalogs", files, interaction);
}

void SyncDbusSessionHelper::InstallPackageNames(
    css::uno::Sequence<OUString> const & packages,
    OUString const & interaction)
{
    request("InstallPackageNames", packages, interaction);
}

void SyncDbusSessionHelper::InstallMimeTypes(
    css::uno::Sequence<OUString> const & mimeTypes,
    OUString const & interaction)
{
    request("InstallMimeTypes", mimeTypes, interaction);
}

void SyncDbusSessionHelper::InstallFontconfigResources(
    css::uno::Sequence<OUString> const & resources,
    OUString const & interaction)
{
    request("InstallFontconfigResources", resources, interaction);
}

void SyncDbusSessionHelper::InstallGStreamerResources(
    css::uno::Sequence<OUString> const & resources,
    OUString const & interaction)
{
    request("InstallGStreamerResources", resources, interaction);
}

void SyncDbusSessionHelper::RemovePackageByFiles(
    css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
{
    request("RemovePackageByFiles", files, interaction);
}

void SyncDbusSessionHelper::InstallPrinterDrivers(
    css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
{
    request("InstallPrinterDrivers", files, interaction);
}

void SAL_CALL SyncDbusSessionHelper::IsInstalled( const OUString& sPackagename, const OUString& sInteraction, sal_Bool& o_isInstalled )
{
    const OString sPackagenameAscii = OUStringToOString(sPackagename, RTL_TEXTENCODING_ASCII_US);
    const OString sInteractionAscii = OUStringToOString(sInteraction, RTL_TEXTENCODING_ASCII_US);
    std::shared_ptr<GDBusProxy> proxy(lcl_GetPackageKitProxy(u"Query"), GObjectDeleter<GDBusProxy>());
    GErrorWrapper error;
    std::shared_ptr<GVariant> result(g_dbus_proxy_call_sync (proxy.get(),
                     "IsInstalled",
                     g_variant_new ("(ss)",
                            sPackagenameAscii.getStr(),
                            sInteractionAscii.getStr()),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1, /* timeout */
                     nullptr, /* cancellable */
                     &error.getRef()),GVariantDeleter());
    if(result)
        o_isInstalled = bool(g_variant_get_boolean(g_variant_get_child_value(result.get(),0)));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_sessioninstall_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SyncDbusSessionHelper(context));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
