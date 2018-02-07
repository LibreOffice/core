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

class SwNumRuleTable;
class SwDoc;
class SwTextNode;
class SwNumRule;
class SwNodeIndex;

// the special readers

class HTMLReader: public Reader
{
    // we don't want to have the streams/storages open
    virtual bool SetStrmStgPtr() override;
    virtual ErrCode Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
    virtual OUString GetTemplateName(SwDoc& rDoc) const override;
public:
    HTMLReader();
};

class XMLReader : public Reader
{
    virtual ErrCode Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
public:
    virtual int GetReaderType() override;

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual size_t GetSectionList( SfxMedium& rMedium,
                                   std::vector<OUString>& rStrings) const override;
};

// the special writers

void GetWW8Writer( const OUString&, const OUString&, WriterRef& );

// Mapping of the LRSpaces in the currently imported document.
// The foreign filters always provide absolute values for the levels of
// a NumRule. We are now processing relative values related to the LR-Space-Item
// though. The consequence of this is that, for all paragraphs, the indentations
// of the NumRule must be subtracted from the paragraph indentation.
class SW_DLLPUBLIC SwRelNumRuleSpaces
{
    SwNumRuleTable* pNumRuleTable;  // list of all named NumRules

public:
    SwRelNumRuleSpaces( SwDoc const & rDoc, bool bNewDoc );
    ~SwRelNumRuleSpaces();
};


// Get size of fly (if 'automatic' in WW) and check if not too small
SW_DLLPUBLIC void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
    SwTwips nPageWidth);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
