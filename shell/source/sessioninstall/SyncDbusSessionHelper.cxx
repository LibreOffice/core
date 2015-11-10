/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SyncDbusSessionHelper.hxx>

#include <gio/gio.h>
#include <memory>
#include <vector>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace
{
    struct GVariantDeleter { void operator()(GVariant* pV) { g_variant_unref(pV); } };
    struct GVariantBuilderDeleter { void operator()(GVariantBuilder* pVB) { g_variant_builder_unref(pVB); } };
    template <typename T> struct GObjectDeleter { void operator()(T* pO) { g_object_unref(pO); } };
    class GErrorWrapper
    {
        GError* m_pError;
        public:
            explicit GErrorWrapper(GError* pError) : m_pError(pError) {}
            ~GErrorWrapper()
            {
                if(!m_pError)
                    return;
                OUString sMsg = OUString::createFromAscii(m_pError->message);
                g_error_free(m_pError);
                throw RuntimeException(sMsg);
            }
            GError*& getRef() { return m_pError; }
    };
    static inline GDBusProxy* lcl_GetPackageKitProxy(const OUString& sInterface)
    {
        const OString sFullInterface = OUStringToOString("org.freedesktop.PackageKit." + sInterface, RTL_TEXTENCODING_ASCII_US);
        GErrorWrapper error(nullptr);
        GDBusProxy* proxy = nullptr;
        proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                               G_DBUS_PROXY_FLAGS_NONE, nullptr,
                               "org.freedesktop.PackageKit",
                               "/org/freedesktop/PackageKit",
                               reinterpret_cast<const gchar*>(sFullInterface.getStr()),
                               nullptr,
                               &error.getRef());
        if(!proxy)
            throw RuntimeException("couldnt get a proxy!");
        return proxy;
    }

void request(
    char const * method, sal_uInt32 xid,
    css::uno::Sequence<OUString> const & resources,
    OUString const & interaction)
{
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
        lcl_GetPackageKitProxy("Modify"), GObjectDeleter<GDBusProxy>());
    GErrorWrapper error(nullptr);
    g_dbus_proxy_call_sync(
        proxy.get(), method,
        g_variant_new(
            "(uass)", static_cast<guint32>(xid), builder.get(),
            iactUtf8.getStr()),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error.getRef());
}

}

namespace shell { namespace sessioninstall
{
    SyncDbusSessionHelper::SyncDbusSessionHelper(Reference<XComponentContext> const&)
    {
#if !GLIB_CHECK_VERSION(2,36,0)
        g_type_init ();
#endif
    }

void SyncDbusSessionHelper::InstallPackageFiles(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallPackageFiles", xid, files, interaction);
}

void SyncDbusSessionHelper::InstallProvideFiles(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallProvideFiles", xid, files, interaction);
}

void SyncDbusSessionHelper::InstallCatalogs(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallCatalogs", xid, files, interaction);
}

void SyncDbusSessionHelper::InstallPackageNames(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & packages,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallPackageNames", xid, packages, interaction);
}

void SyncDbusSessionHelper::InstallMimeTypes(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & mimeTypes,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallMimeTypes", xid, mimeTypes, interaction);
}

void SyncDbusSessionHelper::InstallFontconfigResources(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & resources,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallFontconfigResources", xid, resources, interaction);
}

void SyncDbusSessionHelper::InstallGStreamerResources(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & resources,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallGStreamerResources", xid, resources, interaction);
}

void SyncDbusSessionHelper::RemovePackageByFiles(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("RemovePackageByFiles", xid, files, interaction);
}

void SyncDbusSessionHelper::InstallPrinterDrivers(
    sal_uInt32 xid, css::uno::Sequence<OUString> const & files,
    OUString const & interaction)
    throw (css::uno::RuntimeException, std::exception)
{
    request("InstallPrinteDrivers", xid, files, interaction);
}

    void SAL_CALL SyncDbusSessionHelper::IsInstalled( const OUString& sPackagename, const OUString& sInteraction, sal_Bool& o_isInstalled ) throw (RuntimeException, std::exception)
    {
        const OString sPackagenameAscii = OUStringToOString(sPackagename, RTL_TEXTENCODING_ASCII_US);
        const OString sInteractionAscii = OUStringToOString(sInteraction, RTL_TEXTENCODING_ASCII_US);
        std::shared_ptr<GDBusProxy> proxy(lcl_GetPackageKitProxy("Query"), GObjectDeleter<GDBusProxy>());
        GErrorWrapper error(nullptr);
        std::shared_ptr<GVariant> result(g_dbus_proxy_call_sync (proxy.get(),
                         "IsInstalled",
                         g_variant_new ("(ss)",
                                sPackagenameAscii.getStr(),
                                sInteractionAscii.getStr()),
                         G_DBUS_CALL_FLAGS_NONE,
                         -1, /* timeout */
                         nullptr, /* cancellable */
                         &error.getRef()),GVariantDeleter());
        if(result.get())
            o_isInstalled = bool(g_variant_get_boolean(g_variant_get_child_value(result.get(),0)));
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
