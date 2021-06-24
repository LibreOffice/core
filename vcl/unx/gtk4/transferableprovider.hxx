/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/link.hxx>

#include <gtk/gtk.h>

struct VclToGtkHelper;

namespace com::sun::star::datatransfer
{
class XTransferable;
}

G_BEGIN_DECLS

/*
   Provide a mechanism to provide data from a LibreOffice XTransferable via a
   GdkContentProvider for gtk clipboard or dnd
*/

G_DECLARE_FINAL_TYPE(TransferableContent, transerable_content, TRANSFERABLE, CONTENT,
                     GdkContentProvider)

GdkContentProvider* transerable_content_new(VclToGtkHelper* pConversionHelper,
                                            css::datatransfer::XTransferable* pTransferable);

/*
   Change to a new XTransferable
 */
void transerable_content_set_transferable(TransferableContent* pContent,
                                          css::datatransfer::XTransferable* pTransferable);

/*
   If the GdkContentProvider is used by a clipboard call rDetachClipboardLink on losing
   ownership of the clipboard
*/
void transerable_content_set_detach_clipboard_link(TransferableContent* pContent,
                                                   const Link<void*, void>& rDetachClipboardLink);

G_END_DECLS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
