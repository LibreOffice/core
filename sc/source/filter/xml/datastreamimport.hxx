/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "importcontext.hxx"

#include <importfilterdata.hxx>
#include <address.hxx>

namespace sax_fastparser { class FastAttributeList; }

class ScXMLDataStreamContext : public ScXMLImportContext
{
    OUString maURL;
    ScRange maRange;
    bool mbRefreshOnEmpty;
    sc::ImportPostProcessData::DataStream::InsertPos meInsertPos;

public:
    ScXMLDataStreamContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLDataStreamContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
