/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/virdev.hxx>
#include <ImageViewerDialog.hxx>

ImageViewerDialog::ImageViewerDialog(weld::Dialog* pParent, BitmapEx aBitmap, OUString atitle)
    : GenericDialogController(pParent, u"cui/ui/imageviewer.ui"_ustr, u"ImageViewerDialog"_ustr)
    , m_xDisplayImage(m_xBuilder->weld_image(u"ImgVW_mainImage"_ustr))
{
    m_xDialog->set_title(atitle);
    aBitmap.Scale(Size(300, 300), BmpScaleFlag::Fast);
    ScopedVclPtr<VirtualDevice> m_pVirDev = m_xDisplayImage->create_virtual_device();
    m_pVirDev->SetOutputSizePixel(aBitmap.GetSizePixel());
    m_pVirDev->DrawBitmapEx(Point(0, 0), aBitmap);
    m_xDisplayImage->set_image(m_pVirDev.get());
    m_pVirDev.disposeAndClear();
}
