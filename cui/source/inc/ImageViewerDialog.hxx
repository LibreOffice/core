/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/bitmapex.hxx>
#include <vcl/weld.hxx>

class ImageViewerDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Image> m_xDisplayImage;

public:
    ImageViewerDialog(weld::Dialog* pParent, BitmapEx aBitmap, const OUString& atitle);
};
