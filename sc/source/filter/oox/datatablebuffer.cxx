/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "datatablebuffer.hxx"

namespace oox {
namespace xls {

ScTableData& TableDataBuffer::createDataTable(const OUString& rId)
{
    std::pair<std::map<OUString, ScTableData>::iterator, bool> aData =
        maTableData.insert(std::pair<OUString, ScTableData>(rId, ScTableData()));
    return aData.first->second;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
