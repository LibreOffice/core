/*************************************************************************
 *
 *  $RCSfile: fltini.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    virtual void SetFltName( const String& rFltNm );
};

class HTMLReader: public Reader
{
    // wir wollen die Streams / Storages nicht geoeffnet haben
    virtual int SetStrmStgPtr();
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
    HTMLReader();
};

class WW1Reader : public Reader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
};

class WW8Reader : public StgReader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
    virtual int GetReaderType();

    virtual BOOL HasGlossaries() const;
    virtual BOOL ReadGlossaries( SwTextBlocks&, BOOL bSaveRelFiles ) const;
};

class XMLReader : public Reader
{
    virtual ULONG Read(SwDoc &,SwPaM &,const String &);
public:
    XMLReader();
};

// die speziellen Writer

#if 0
WriterRef GetW4WWriter( const String& );
WriterRef GetStgWriter( const String& );
WriterRef GetWWWriter( const String& );
WriterRef GetWW8Writer( const String& );
#if !( defined(PRODUCT) || defined(MAC) || defined(PM2) )
WriterRef GetDebugWriter( const String& );
WriterRef GetUndoWriter( const String& );
#endif
#else
void GetW4WWriter( const String&, WriterRef& );
void GetStgWriter( const String&, WriterRef& );
void GetWWWriter( const String&, WriterRef& );
void GetWW8Writer( const String&, WriterRef& );
#if !( defined(PRODUCT) || defined(MAC) || defined(PM2) )
void GetDebugWriter( const String&, WriterRef& );
void GetUndoWriter( const String&, WriterRef& );
#endif
#endif

// Zugriff auf die Ini-Datei
ULONG ReadFilterFlags( const sal_Char* pName, const sal_Char* pAltName = 0 );

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

Color   ConvertBrushStyle(const Color& rCol, const Color& rFillCol, BYTE nStyle);

// Get size of fly (if 'automatic' in WW) and check if not too small
void    CalculateFlySize( SfxItemSet& rFlySet, SwNodeIndex& rAnchor,
                          SwTwips nPageWidth );

#endif _FLTINI_HXX
