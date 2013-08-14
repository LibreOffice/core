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
#ifndef _FLTINI_HXX
#define _FLTINI_HXX

#include <shellio.hxx>

class SwNumRuleTbl;
class SwDoc;
class SwTxtNode;
class SwNumRule;
class SwNodeIndex;

// die speziellen Reader

class HTMLReader: public Reader
{
    // wir wollen die Streams / Storages nicht geoeffnet haben
    virtual int SetStrmStgPtr();
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &);
    virtual OUString GetTemplateName() const;
public:
    HTMLReader();
};

class WW1Reader : public Reader
{
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &);
};

class XMLReader : public Reader
{
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &);
public:
    virtual int GetReaderType();

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual size_t GetSectionList( SfxMedium& rMedium,
                                   std::vector<OUString*>& rStrings ) const;
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
    SwNumRuleTbl* pNumRuleTbl;  // Liste aller benannten NumRules

public:
    SwRelNumRuleSpaces( SwDoc& rDoc, sal_Bool bNewDoc );
    ~SwRelNumRuleSpaces();
};

#define SW_SV_BRUSH_25          0
#define SW_SV_BRUSH_50          1
#define SW_SV_BRUSH_75          2
#define SW_SV_BRUSH_NULL        3
#define SW_SV_BRUSH_SOLID       4
#define SW_SV_BRUSH_INVALID     5

// Get size of fly (if 'automatic' in WW) and check if not too small
SW_DLLPUBLIC void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
    SwTwips nPageWidth);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
