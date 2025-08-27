/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include <orcus/stream.hpp>

orcus::file_content toFileContent(const OUString& rPath);

/**
 * Stream copied to a temporary file with a filepath.
 */
class CopiedTempStream
{
    utl::TempFileNamed maTemp;

public:
    CopiedTempStream(SvStream& rSrc);
    CopiedTempStream(const css::uno::Reference<css::io::XInputStream>& xSrc);

    OUString getFileName() const { return maTemp.GetFileName(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
