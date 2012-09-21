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



#ifndef SC_SCPATATR_HXX
#define SC_SCPATATR_HXX

#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svl/brdcst.hxx>
#include <unotools/fontcvt.hxx>
#include <editeng/svxenum.hxx>
#include "scdllapi.h"

class Font;
class OutputDevice;
class Fraction;
class ScStyleSheet;
class SvNumberFormatter;
class ScDocument;


//  how to treat COL_AUTO in GetFont:

enum ScAutoFontColorMode
{
    SC_AUTOCOL_RAW,         // COL_AUTO is returned
    SC_AUTOCOL_BLACK,       // always use black
    SC_AUTOCOL_PRINT,       // black or white, depending on background
    SC_AUTOCOL_DISPLAY,     // from style settings, or black/white if needed
    SC_AUTOCOL_IGNOREFONT,  // like DISPLAY, but ignore stored font color (assume COL_AUTO)
    SC_AUTOCOL_IGNOREBACK,  // like DISPLAY, but ignore stored background color (use configured color)
    SC_AUTOCOL_IGNOREALL    // like DISPLAY, but ignore stored font and background colors
};


class SC_DLLPUBLIC ScPatternAttr: public SfxSetItem, public SfxBroadcaster
{
    String*         pName;
    ScStyleSheet*   pStyle;
public:
    static ScDocument* pDoc;
                            ScPatternAttr(SfxItemSet* pItemSet, const String& rStyleName);
                            ScPatternAttr(SfxItemSet* pItemSet, ScStyleSheet* pStyleSheet = NULL);
                            ScPatternAttr(SfxItemPool* pItemPool);
                            ScPatternAttr(const ScPatternAttr& rPatternAttr);

                            ~ScPatternAttr();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream& rStream, sal_uInt16 nVersion) const;
    virtual SvStream&       Store(SvStream& rStream, sal_uInt16 nItemVersion) const;

    virtual int             operator==(const SfxPoolItem& rCmp) const;

    const SfxPoolItem&      GetItem( sal_uInt16 nWhichP ) const
                                        { return GetItemSet().Get(nWhichP); }

    static const SfxPoolItem& GetItem( sal_uInt16 nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet );
    const SfxPoolItem&      GetItem( sal_uInt16 nWhich, const SfxItemSet* pCondSet ) const;

                            // pWhich sind keine Ranges, sondern einzelne IDs, 0-terminiert
    sal_Bool                    HasItemsSet( const sal_uInt16* pWhich ) const;
    void                    ClearItems( const sal_uInt16* pWhich );

    void                    DeleteUnchanged( const ScPatternAttr* pOldAttrs );

    static SvxCellOrientation GetCellOrientation( const SfxItemSet& rItemSet, const SfxItemSet* pCondSet = 0 );
    SvxCellOrientation      GetCellOrientation( const SfxItemSet* pCondSet = 0 ) const;

    /** Static helper function to fill a font object from the passed item set. */
    static void             GetFont( Font& rFont, const SfxItemSet& rItemSet,
                                        ScAutoFontColorMode eAutoMode,
                                        OutputDevice* pOutDev = NULL,
                                        const Fraction* pScale = NULL,
                                        const SfxItemSet* pCondSet = NULL,
                                        sal_uInt8 nScript = 0, const Color* pBackConfigColor = NULL,
                                        const Color* pTextConfigColor = NULL );
    /** Fills a font object from the own item set. */
    void                    GetFont( Font& rFont, ScAutoFontColorMode eAutoMode,
                                        OutputDevice* pOutDev = NULL,
                                        const Fraction* pScale = NULL,
                                        const SfxItemSet* pCondSet = NULL,
                                        sal_uInt8 nScript = 0, const Color* pBackConfigColor = NULL,
                                        const Color* pTextConfigColor = NULL ) const;

    /** Converts all Calc items contained in rSrcSet to edit engine items and puts them into rEditSet. */
    static void             FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet = NULL );
    /** Converts all Calc items contained in the own item set to edit engine items and puts them into pEditSet. */
    void                    FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet = NULL ) const;

    /** Converts all edit engine items contained in rEditSet to Calc items and puts them into rDestSet. */
    static void             GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet );
    /** Converts all edit engine items contained in pEditSet to Calc items and puts them into the own item set. */
    void                    GetFromEditItemSet( const SfxItemSet* pEditSet );

    void                    FillEditParaItems( SfxItemSet* pSet ) const;

    ScPatternAttr*          PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const;

    void                    SetStyleSheet(ScStyleSheet* pNewStyle);
    const ScStyleSheet*     GetStyleSheet() const  { return pStyle; }
    const String*           GetStyleName() const;
    void                    UpdateStyleSheet();
    void                    StyleToName();

    sal_Bool                    IsVisible() const;
    sal_Bool                    IsVisibleEqual( const ScPatternAttr& rOther ) const;

                            /** If font is an old symbol font StarBats/StarMath
                                with text encoding RTL_TEXTENC_SYMBOL */
    sal_Bool                    IsSymbolFont() const;

//UNUSED2008-05                          /** Create a FontToSubsFontConverter if needed for
//UNUSED2008-05                              this pattern, else return 0.
//UNUSED2008-05
//UNUSED2008-05                              @param nFlags is the bit mask which shall be
//UNUSED2008-05                              used for CreateFontToSubsFontConverter().
//UNUSED2008-05
//UNUSED2008-05                              The converter must be destroyed by the caller
//UNUSED2008-05                              using DestroyFontToSubsFontConverter() which
//UNUSED2008-05                              should be accomplished using the
//UNUSED2008-05                              ScFontToSubsFontConverter_AutoPtr
//UNUSED2008-05                           */
//UNUSED2008-05  FontToSubsFontConverter GetSubsFontConverter( sal_uLong nFlags ) const;

    sal_uLong                   GetNumberFormat( SvNumberFormatter* ) const;
    sal_uLong                   GetNumberFormat( SvNumberFormatter* pFormatter,
                                                const SfxItemSet* pCondSet ) const;

    long                    GetRotateVal( const SfxItemSet* pCondSet ) const;
    sal_uInt8                   GetRotateDir( const SfxItemSet* pCondSet ) const;
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


#endif
