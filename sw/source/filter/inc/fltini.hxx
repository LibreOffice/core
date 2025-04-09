/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_FILTER_INC_FLTINI_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_FLTINI_HXX

#include <shellio.hxx>

// the special readers

class HTMLReader final : public Reader
{
    // we don't want to have the streams/storages open
    virtual bool SetStrmStgPtr() override;
    virtual OUString GetTemplateName(SwDoc& rDoc) const override;

    /// Parse FilterOptions passed to the importer.
    void SetupFilterOptions();

    OUString m_aNamespace;

public:
    HTMLReader();
    virtual ErrCodeMsg Read(SwDoc&, const OUString& rBaseURL, SwPaM&, const OUString&) override;
};

class XMLReader final : public Reader
{
    virtual ErrCodeMsg Read(SwDoc&, const OUString& rBaseURL, SwPaM&, const OUString&) override;

public:
    virtual SwReaderType GetReaderType() override;

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual size_t GetSectionList(SfxMedium& rMedium,
                                  std::vector<OUString>& rStrings) const override;
};

// the special writers

void GetWW8Writer(std::u16string_view, const OUString&, WriterRef&);

// Get size of fly (if 'automatic' in WW) and check if not too small
SW_DLLPUBLIC void CalculateFlySize(SfxItemSet& rFlySet, const SwNode& rAnchor, SwTwips nPageWidth);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
