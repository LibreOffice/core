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

#ifndef SC_SCPATATR_HXX
#define SC_SCPATATR_HXX


#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SV_FONTCVT_HXX
#include <unotools/fontcvt.hxx>
#endif
class Font;
class OutputDevice;
class Fraction;
namespace binfilter {

class SvNumberFormatter;

class ScStyleSheet;
class ScDocument;


//	how to treat COL_AUTO in GetFont:

enum ScAutoFontColorMode
{
    SC_AUTOCOL_RAW,			// COL_AUTO is returned
    SC_AUTOCOL_BLACK,		// always use black
    SC_AUTOCOL_PRINT,		// black or white, depending on background
    SC_AUTOCOL_DISPLAY,		// from style settings, or black/white if needed
    SC_AUTOCOL_IGNOREFONT,	// like DISPLAY, but ignore stored font color (assume COL_AUTO)
    SC_AUTOCOL_IGNOREBACK,	// like DISPLAY, but ignore stored background color (use configured color)
    SC_AUTOCOL_IGNOREALL	// like DISPLAY, but ignore stored font and background colors
};


class ScPatternAttr: public SfxSetItem
{
    String*			pName;
    ScStyleSheet*	pStyle;
public:
    static ScDocument* pDoc;
                            ScPatternAttr(SfxItemSet* pItemSet, const String& rStyleName);
                            ScPatternAttr(SfxItemSet* pItemSet, ScStyleSheet* pStyleSheet = NULL);
                            ScPatternAttr(SfxItemPool* pItemPool);
                            ScPatternAttr(const ScPatternAttr& rPatternAttr);

                            ~ScPatternAttr();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream& rStream, USHORT nVersion) const;
    virtual SvStream&       Store(SvStream& rStream, USHORT nItemVersion) const;

    virtual int 			operator==(const SfxPoolItem& rCmp) const;

    const SfxPoolItem&      GetItem( USHORT nWhich ) const
                                        { return GetItemSet().Get(nWhich); }

    const SfxPoolItem&		GetItem( USHORT nWhich, const SfxItemSet* pCondSet ) const;

                            // pWhich sind keine Ranges, sondern einzelne IDs, 0-terminiert
/*N*/ 	BOOL					HasItemsSet( const USHORT* pWhich ) const;
/*N*/ 	void					ClearItems( const USHORT* pWhich );

    void                    DeleteUnchanged( const ScPatternAttr* pOldAttrs );

    /** Static helper function to fill a font object from the passed item set. */
    static void             GetFont( Font& rFont, const SfxItemSet& rItemSet,
                                        ScAutoFontColorMode eAutoMode,
                                        OutputDevice* pOutDev = NULL,
                                        const Fraction* pScale = NULL,
                                        const SfxItemSet* pCondSet = NULL,
                                        BYTE nScript = 0, const Color* pBackConfigColor = NULL,
                                        const Color* pTextConfigColor = NULL );
    /** Fills a font object from the own item set. */
    void                    GetFont( Font& rFont, ScAutoFontColorMode eAutoMode,
                                        OutputDevice* pOutDev = NULL,
                                        const Fraction* pScale = NULL,
                                        const SfxItemSet* pCondSet = NULL,
                                        BYTE nScript = 0, const Color* pBackConfigColor = NULL,
                                        const Color* pTextConfigColor = NULL ) const;

    /** Converts all Calc items contained in rSrcSet to edit engine items and puts them into rEditSet. */
    static void             FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet = NULL );
    /** Converts all Calc items contained in the own item set to edit engine items and puts them into pEditSet. */
    void                    FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet = NULL ) const;

    /** Converts all edit engine items contained in rEditSet to Calc items and puts them into rDestSet. */
    static void             GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet );
    /** Converts all edit engine items contained in pEditSet to Calc items and puts them into the own item set. */
    void                    GetFromEditItemSet( const SfxItemSet* pEditSet );

    void					FillEditParaItems( SfxItemSet* pSet ) const;

    ScPatternAttr*          PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const;

    void					SetStyleSheet(ScStyleSheet* pNewStyle);
    const ScStyleSheet*		GetStyleSheet() const  { return pStyle; }
    const String*			GetStyleName() const;
    void					UpdateStyleSheet();

    BOOL					IsVisible() const;
    BOOL					IsVisibleEqual( const ScPatternAttr& rOther ) const;

                            /** If font is an old symbol font StarBats/StarMath
                                with text encoding RTL_TEXTENC_SYMBOL */
    BOOL					IsSymbolFont() const;

                            /** Create a FontToSubsFontConverter if needed for
                                this pattern, else return 0.

                                @param nFlags is the bit mask which shall be
                                used for CreateFontToSubsFontConverter().

                                The converter must be destroyed by the caller
                                using DestroyFontToSubsFontConverter() which
                                should be accomplished using the
                                ScFontToSubsFontConverter_AutoPtr
                             */
    FontToSubsFontConverter GetSubsFontConverter( ULONG nFlags ) const;

    ULONG					GetNumberFormat( SvNumberFormatter* ) const;
    ULONG					GetNumberFormat( SvNumberFormatter* pFormatter,
                                                const SfxItemSet* pCondSet ) const;
};


class ScFontToSubsFontConverter_AutoPtr
{
            FontToSubsFontConverter h;

            void                    release()
                                    {
                                        if ( h )
                                            DestroyFontToSubsFontConverter( h );
                                    }

                                // prevent usage
                                ScFontToSubsFontConverter_AutoPtr( const ScFontToSubsFontConverter_AutoPtr& );
    ScFontToSubsFontConverter_AutoPtr& operator=( const ScFontToSubsFontConverter_AutoPtr& );

public:
                                ScFontToSubsFontConverter_AutoPtr()
                                    : h(0)
                                    {}
                                ~ScFontToSubsFontConverter_AutoPtr()
                                    {
                                        release();
                                    }

    ScFontToSubsFontConverter_AutoPtr& operator=( FontToSubsFontConverter hN )
                                    {
                                        release();
                                        h = hN;
                                        return *this;
                                    }

            operator FontToSubsFontConverter() const
                                    { return h; }
};


} //namespace binfilter
#endif
