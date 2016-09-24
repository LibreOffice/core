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

// die speziellen Reader

class HTMLReader: public Reader
{
    // wir wollen die Streams / Storages nicht geoeffnet haben
    virtual bool SetStrmStgPtr() override;
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
    virtual OUString GetTemplateName() const override;
public:
    HTMLReader();
};

class XMLReader : public Reader
{
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
public:
    virtual int GetReaderType() override;

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual size_t GetSectionList( SfxMedium& rMedium,
                                   std::vector<OUString*>& rStrings ) const override;
};

// die speziellen Writer

void GetWW8Writer( const OUString&, const OUString&, WriterRef& );

// Umsetzen der LRSpaces im aktuell importierten Doc. Die Fremd-Filter
// liefern immer absolute Werte fuer die Ebenen einer NumRule. Wir
// verarbeiten jetzt aber relative Werte bezogen auf das LR-Space-Item.
// Das hat zur Folge, das bei allen Absaetzen die EInzuege der NumRule vom
// Absatz-Einzug abgezogen werden muss.
class SW_DLLPUBLIC SwRelNumRuleSpaces
{
    SwNumRuleTable* pNumRuleTable;  // Liste aller benannten NumRules

public:
    SwRelNumRuleSpaces( SwDoc& rDoc, bool bNewDoc );
    ~SwRelNumRuleSpaces();
};


// Get size of fly (if 'automatic' in WW) and check if not too small
SW_DLLPUBLIC void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
    SwTwips nPageWidth);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
