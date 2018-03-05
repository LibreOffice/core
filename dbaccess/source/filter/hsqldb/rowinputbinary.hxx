/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_ROWINPUTBINARY_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_ROWINPUTBINARY_HXX

#include <vector>
#include <tools/stream.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/io/XInputStream.hpp>

namespace dbahsql
{
class HsqlRowInputStream
{
private:
    std::unique_ptr<SvStream> m_pStream = nullptr;

protected:
    OUString readString();
    bool checkNull();

    // reimplement reading of an UTF string with a given length
    OUString readUTF(sal_Int32 nLen);

public:
    HsqlRowInputStream();
    std::vector<css::uno::Any> readOneRow(const std::vector<sal_Int32>& colTypes);
    void seek(sal_Int32 nPos);
    void setInputStream(css::uno::Reference<css::io::XInputStream>& rStream);
    SvStream* getInputStream() const;
};

} // namespace dbahsql

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_ROWINPUTBINARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
