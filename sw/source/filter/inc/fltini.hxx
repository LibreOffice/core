/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltini.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 16:01:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FLTINI_HXX
#define _FLTINI_HXX

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

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
    virtual ULONG Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
    virtual String GetTemplateName() const;
public:
    HTMLReader();
};

class WW1Reader : public Reader
{
    virtual ULONG Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
};

class WW8Reader : public StgReader
{
    virtual ULONG Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    virtual int GetReaderType();

    virtual BOOL HasGlossaries() const;
    virtual BOOL ReadGlossaries( SwTextBlocks&, BOOL bSaveRelFiles ) const;
};

class XMLReader : public Reader
{
    virtual ULONG Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    virtual int GetReaderType();

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual USHORT GetSectionList( SfxMedium& rMedium,
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
class SwRelNumRuleSpaces
{
    SwNumRuleTbl* pNumRuleTbl;  // Liste aller benannten NumRules
    BOOL bNewDoc;

    void SetNumLSpace( SwTxtNode& rNd, const SwNumRule& rRule );

public:
    SwRelNumRuleSpaces( SwDoc& rDoc, BOOL bNewDoc );
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
void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
    SwTwips nPageWidth);

#endif
