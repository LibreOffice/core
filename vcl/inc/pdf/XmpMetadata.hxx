/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/dllapi.h>
#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <memory>

namespace vcl::pdf
{
class XmpMetadata
{
private:
    bool mbWritten;
    std::unique_ptr<SvMemoryStream> mpMemoryStream;

public:
    OString msTitle;
    OString msAuthor;
    OString msSubject;
    OString msProducer;
    OString msKeywords;
    OString m_sCreatorTool;
    OString m_sCreateDate;

    sal_Int32 mnPDF_A;
    bool mbPDF_UA;

public:
    XmpMetadata();
    sal_uInt64 getSize();
    const void* getData();

private:
    void write();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
