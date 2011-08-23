/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FLTINI_HXX
#define _FLTINI_HXX


#include <shellio.hxx>
#include <tools/color.hxx>
namespace binfilter {

class SwNumRuleTbl;
class SwDoc;
class SwTxtNode;
class SwNodeNum;
class SwNumRule;
class SwNodeIndex;


// die speziellen Reader

class Sw6Reader: public Reader
{
    virtual ULONG Read( SwDoc &,SwPaM &,const String &);
};

 class W4WReader: public StgReader
 {
    String sVersion;
    USHORT nFilter;
    BOOL bStorageFlag;
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
    // wir wollen die Streams / Storages nicht geoeffnet haben
    virtual int SetStrmStgPtr();
 public:
    W4WReader() : StgReader(), nFilter(0), bStorageFlag(FALSE) {}
    virtual int GetReaderType();
    virtual void SetFltName( const String& rFltName );

    USHORT GetFilter() const { return nFilter; }
    const String& GetVersion() const { return sVersion; }
 };

#ifdef DEBUG_SH

class Internal_W4WReader: public W4WReader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
};

#endif

class ExcelReader : public StgReader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
    virtual int GetReaderType();
};

class LotusReader : public Reader
{
    CharSet eCodeSet;
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
};




class XMLReader : public Reader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
    virtual int GetReaderType();

    XMLReader();

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual USHORT GetSectionList( SfxMedium& rMedium,
                                SvStrings& rStrings ) const;
};

// die speziellen Writer

#if 0
/*?*/ WriterRef GetW4WWriter( const String& );
WriterRef GetStgWriter( const String& );
WriterRef GetWWWriter( const String& );
/*?*/ WriterRef GetWW8Writer( const String& );
#if !( !defined(DBG_UTIL) || defined(MAC) || defined(PM2) )
/*?*/ WriterRef GetDebugWriter( const String& );
/*?*/ WriterRef GetUndoWriter( const String& );
#endif
#else
/*?*/ void GetW4WWriter( const String&, WriterRef& );
void GetStgWriter( const String&, WriterRef& );
void GetWWWriter( const String&, WriterRef& );
#if !( !defined(DBG_UTIL) || defined(MAC) || defined(PM2) )
#endif
#endif


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

#define SW_SV_BRUSH_25     		0
#define SW_SV_BRUSH_50          1
#define SW_SV_BRUSH_75          2
#define SW_SV_BRUSH_NULL        3
#define SW_SV_BRUSH_SOLID       4
#define SW_SV_BRUSH_INVALID     5

Color	ConvertBrushStyle(const Color& rCol, const Color& rFillCol, BYTE nStyle);

// Get size of fly (if 'automatic' in WW) and check if not too small
void	CalculateFlySize( SfxItemSet& rFlySet, SwNodeIndex& rAnchor,
                          SwTwips nPageWidth );

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
