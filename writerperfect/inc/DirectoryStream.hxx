/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <librevenge-stream/librevenge-stream.h>
#include <com/sun/star/uno/Reference.h>
#include "writerperfectdllapi.h"
#include <tools/long.hxx>
#include <memory>

namespace com::sun::star::ucb
{
class XContent;
}

namespace writerperfect
{
class WRITERPERFECT_DLLPUBLIC DirectoryStream final : public librevenge::RVNGInputStream
{
    struct Impl;

public:
    explicit DirectoryStream(const css::uno::Reference<css::ucb::XContent>& xContent);
    virtual ~DirectoryStream() override;

    static bool isDirectory(const css::uno::Reference<css::ucb::XContent>& xContent);
    static std::unique_ptr<DirectoryStream>
    createForParent(const css::uno::Reference<css::ucb::XContent>& xContent);

    css::uno::Reference<css::ucb::XContent> getContent() const;

    virtual bool isStructured() override;
    virtual unsigned subStreamCount() override;
    virtual const char* subStreamName(unsigned id) override;
    virtual bool existsSubStream(const char* name) override;
    virtual librevenge::RVNGInputStream* getSubStreamByName(const char* name) override;
    virtual librevenge::RVNGInputStream* getSubStreamById(unsigned id) override;

    virtual const unsigned char* read(unsigned long numBytes, unsigned long& numBytesRead) override;
    virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) override;
    virtual long tell() override;
    virtual bool isEnd() override;

private:
    std::unique_ptr<Impl> m_pImpl;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
