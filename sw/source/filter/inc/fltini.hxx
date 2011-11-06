/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _FLTINI_HXX
#define _FLTINI_HXX

#include <shellio.hxx>
#include <tools/color.hxx>

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
    virtual sal_uLong Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
    virtual String GetTemplateName() const;
public:
    HTMLReader();
};

class WW1Reader : public Reader
{
    virtual sal_uLong Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
};

class XMLReader : public Reader
{
    virtual sal_uLong Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    virtual int GetReaderType();

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual sal_uInt16 GetSectionList( SfxMedium& rMedium,
                                SvStrings& rStrings ) const;
};

// die speziellen Writer

void GetWW8Writer( const String&, const String&, WriterRef& );


// JP 17.03.99 - 63049
// Umsetzen der LRSpaces im aktuell importierten Doc. Die Fremd-Filter
// liefern immer absolute Werte fuer die Ebenen einer NumRule. Wir
// verarbeiten jetzt aber relative Werte bezogen auf das LR-Space-Item.
// Das hat zur Folge, das bei allen Absaetzen die EInzuege der NumRule vom
// Absatz-Einzug abgezogen werden muss.
class SW_DLLPUBLIC SwRelNumRuleSpaces
{
    SwNumRuleTbl* pNumRuleTbl;  // Liste aller benannten NumRules
    sal_Bool bNewDoc;

    void SetNumLSpace( SwTxtNode& rNd, const SwNumRule& rRule );

public:
    SwRelNumRuleSpaces( SwDoc& rDoc, sal_Bool bNewDoc );
    ~SwRelNumRuleSpaces();

    void SetNumRelSpaces( SwDoc& rDoc );
    void SetOultineRelSpaces( const SwNodeIndex& rStt,
                                const SwNodeIndex& rEnd );
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
