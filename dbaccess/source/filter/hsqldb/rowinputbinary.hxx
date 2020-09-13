/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <tools/stream.hxx>

#include <com/sun/star/io/XInputStream.hpp>

#include "columndef.hxx"

namespace dbahsql
{
class HsqlRowInputStream
{
private:
    std::unique_ptr<SvStream> m_pStream;

protected:
    OUString readString();
    bool checkNull();

    OUString readUTF(sal_Int32 nLen);

public:
    HsqlRowInputStream();

    /**
     * Reads one row from the actual position.
     * @param colTypes Field types of the row, in a strict order.
     */
    std::vector<css::uno::Any> readOneRow(const std::vector<ColumnDefinition>& colTypes);

    /**
     * Sets the file-pointer offset, measured from the beginning of the file
     */
    void seek(sal_Int32 nPos);

    void setInputStream(css::uno::Reference<css::io::XInputStream> const& rStream);
    SvStream* getInputStream() const;
};

} // namespace dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
