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
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::std;
using namespace ::rtl;

namespace
{
    struct GVariantDeleter { void operator()(GVariant* pV) { g_variant_unref(pV); } };
    struct GVariantBuilderDeleter { void operator()(GVariantBuilder* pVB) { g_variant_builder_unref(pVB); } };
    template <typename T> struct GObjectDeleter { void operator()(T* pO) { g_object_unref(pO); } };
    class GErrorWrapper
    {
        GError* m_pError;
        public:
            GErrorWrapper(GError* pError) : m_pError(pError) {}
            ~GErrorWrapper()
            {
                if(!m_pError)
                    return;
                OUString sMsg = OUString::createFromAscii(m_pError->message);
                g_error_free(m_pError);
                throw RuntimeException(sMsg, NULL);
            }
            GError** getRef() { return &m_pError; }
    };
    static inline GDBusProxy* lcl_GetPackageKitProxy(const OUString sInterface)
    {
        const OString sFullInterface = OUStringToOString("org.freedesktop.PackageKit." + sInterface, RTL_TEXTENCODING_ASCII_US);
        GErrorWrapper error(NULL);
        GDBusProxy* proxy = NULL;
        proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                               G_DBUS_PROXY_FLAGS_NONE, NULL,
                               "org.freedesktop.PackageKit",
                               "/org/freedesktop/PackageKit",
                               reinterpret_cast<const gchar*>(sFullInterface.getStr()),
                               NULL,
                               error.getRef());
        if(!proxy)
            throw RuntimeException("couldnt get a proxy!",NULL);
        return proxy;
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
    void SAL_CALL SyncDbusSessionHelper::InstallPackageNames( const ::sal_uInt32 nXid, const Sequence< OUString >& vPackages, const OUString& sInteraction ) throw (RuntimeException, std::exception)
    {
        vector< OString > vPackagesOString;
        vPackagesOString.reserve(vPackages.getLength());
        boost::shared_ptr<GVariantBuilder> pBuilder(g_variant_builder_new(G_VARIANT_TYPE ("as")), GVariantBuilderDeleter());
        for( const OUString* pPackage = vPackages.begin(); pPackage != vPackages.end(); ++pPackage)
        {
            vPackagesOString.push_back(OUStringToOString(*pPackage, RTL_TEXTENCODING_ASCII_US));
            g_variant_builder_add(pBuilder.get(), "s", vPackagesOString.back().getStr());
        }

        const OString sInteractionAscii = OUStringToOString(sInteraction, RTL_TEXTENCODING_ASCII_US);
        boost::shared_ptr<GDBusProxy> proxy(lcl_GetPackageKitProxy("Modify"), GObjectDeleter<GDBusProxy>());
        GErrorWrapper error(NULL);
        g_dbus_proxy_call_sync (proxy.get(),
                         "InstallPackageNames",
                         g_variant_new ("(uass)",
                                sal::static_int_cast<guint32>(nXid),
                                pBuilder.get(),
                                sInteractionAscii.getStr()),
                         G_DBUS_CALL_FLAGS_NONE,
                         -1, /* timeout */
                         NULL, /* cancellable */
                         error.getRef());
    }

    void SAL_CALL SyncDbusSessionHelper::IsInstalled( const OUString& sPackagename, const OUString& sInteraction, ::sal_Bool& o_isInstalled ) throw (RuntimeException, std::exception)
    {
        const OString sPackagenameAscii = OUStringToOString(sPackagename, RTL_TEXTENCODING_ASCII_US);
        const OString sInteractionAscii = OUStringToOString(sInteraction, RTL_TEXTENCODING_ASCII_US);
        boost::shared_ptr<GDBusProxy> proxy(lcl_GetPackageKitProxy("Query"), GObjectDeleter<GDBusProxy>());
        GErrorWrapper error(NULL);
        boost::shared_ptr<GVariant> result(g_dbus_proxy_call_sync (proxy.get(),
                         "IsInstalled",
                         g_variant_new ("(ss)",
                                sPackagenameAscii.getStr(),
                                sInteractionAscii.getStr()),
                         G_DBUS_CALL_FLAGS_NONE,
                         -1, /* timeout */
                         NULL, /* cancellable */
                         error.getRef()),GVariantDeleter());
        if(result.get())
            o_isInstalled = g_variant_get_boolean(g_variant_get_child_value(result.get(),0)) ? sal_True : sal_False;
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
