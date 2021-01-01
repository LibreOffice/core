/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/stream.hxx>
#include <graphic/VectorGraphicLoader.hxx>

namespace vcl
{
std::shared_ptr<VectorGraphicData>
loadPdfFromDataContainer(BinaryDataContainer const& rDataContainer)
{
    if (rDataContainer.isEmpty())
        return std::shared_ptr<VectorGraphicData>();

    return std::make_shared<VectorGraphicData>(rDataContainer, VectorGraphicDataType::Pdf);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
