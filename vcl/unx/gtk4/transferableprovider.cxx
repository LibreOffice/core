/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <unx/gtk/gtkinst.hxx>
#include "transferableprovider.hxx"

#define TRANSFERABLE_CONTENT(obj)                                                                  \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), transerable_content_get_type(), TransferableContent))

struct _TransferableContent
{
    GdkContentProvider parent;
    VclToGtkHelper* m_pConversionHelper;
    css::datatransfer::XTransferable* m_pContents;
    Link<void*, void> m_aDetachClipboardLink;
};

namespace
{
struct _TransferableContentClass : public GdkContentProviderClass
{
};
}

G_DEFINE_TYPE(TransferableContent, transerable_content, GDK_TYPE_CONTENT_PROVIDER)

static void transerable_content_write_mime_type_async(GdkContentProvider* provider,
                                                      const char* mime_type, GOutputStream* stream,
                                                      int io_priority, GCancellable* cancellable,
                                                      GAsyncReadyCallback callback,
                                                      gpointer user_data)
{
    TransferableContent* self = TRANSFERABLE_CONTENT(provider);
    if (!self->m_pContents)
        return;
    // tdf#129809 take a reference in case m_aContents is replaced during this
    // call
    css::uno::Reference<css::datatransfer::XTransferable> xCurrentContents(self->m_pContents);
    self->m_pConversionHelper->setSelectionData(xCurrentContents, provider, mime_type, stream,
                                                io_priority, cancellable, callback, user_data);
}

static gboolean transerable_content_write_mime_type_finish(GdkContentProvider*,
                                                           GAsyncResult* result, GError** error)
{
    return g_task_propagate_boolean(G_TASK(result), error);
}

static GdkContentFormats* transerable_content_ref_formats(GdkContentProvider* provider)
{
    TransferableContent* self = TRANSFERABLE_CONTENT(provider);
    css::uno::Reference<css::datatransfer::XTransferable> xCurrentContents(self->m_pContents);
    if (!xCurrentContents)
        return nullptr;

    auto aFormats = xCurrentContents->getTransferDataFlavors();
    std::vector<OString> aGtkTargets(self->m_pConversionHelper->FormatsToGtk(aFormats));

    GdkContentFormatsBuilder* pBuilder = gdk_content_formats_builder_new();
    for (const auto& rFormat : aGtkTargets)
        gdk_content_formats_builder_add_mime_type(pBuilder, rFormat.getStr());
    return gdk_content_formats_builder_free_to_formats(pBuilder);
}

static void transerable_content_detach_clipboard(GdkContentProvider* provider, GdkClipboard*)
{
    TransferableContent* self = TRANSFERABLE_CONTENT(provider);
    self->m_aDetachClipboardLink.Call(nullptr);
}

static void transerable_content_class_init(TransferableContentClass* klass)
{
    GdkContentProviderClass* provider_class = GDK_CONTENT_PROVIDER_CLASS(klass);

    provider_class->ref_formats = transerable_content_ref_formats;
    provider_class->detach_clipboard = transerable_content_detach_clipboard;
    provider_class->write_mime_type_async = transerable_content_write_mime_type_async;
    provider_class->write_mime_type_finish = transerable_content_write_mime_type_finish;
}

static void transerable_content_init(TransferableContent* self)
{
    self->m_pConversionHelper = nullptr;
    self->m_pContents = nullptr;
    // prevent loplugin:unreffun firing on macro generated function
    (void)transerable_content_get_instance_private(self);
}

void transerable_content_set_transferable(TransferableContent* pContent,
                                          css::datatransfer::XTransferable* pTransferable)
{
    pContent->m_pContents = pTransferable;
}

void transerable_content_set_detach_clipboard_link(TransferableContent* pContent,
                                                   const Link<void*, void>& rDetachClipboardLink)
{
    pContent->m_aDetachClipboardLink = rDetachClipboardLink;
}

GdkContentProvider* transerable_content_new(VclToGtkHelper* pConversionHelper,
                                            css::datatransfer::XTransferable* pTransferable)
{
    TransferableContent* content
        = TRANSFERABLE_CONTENT(g_object_new(transerable_content_get_type(), nullptr));
    content->m_pConversionHelper = pConversionHelper;
    content->m_pContents = pTransferable;
    return GDK_CONTENT_PROVIDER(content);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
