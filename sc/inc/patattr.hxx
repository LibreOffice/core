/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: patattr.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:57:13 $
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

#ifndef SC_SCPATATR_HXX
#define SC_SCPATATR_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

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


class SC_DLLPUBLIC ScPatternAttr: public SfxSetItem
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
    virtual SfxPoolItem*    Create(SvStream& rStream, USHORT nVersion) const;
    virtual SvStream&       Store(SvStream& rStream, USHORT nItemVersion) const;

    virtual int             operator==(const SfxPoolItem& rCmp) const;

    const SfxPoolItem&      GetItem( USHORT nWhichP ) const
                                        { return GetItemSet().Get(nWhichP); }

    static const SfxPoolItem& GetItem( USHORT nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet );
    const SfxPoolItem&      GetItem( USHORT nWhich, const SfxItemSet* pCondSet ) const;

                            // pWhich sind keine Ranges, sondern einzelne IDs, 0-terminiert
    BOOL                    HasItemsSet( const USHORT* pWhich ) const;
    void                    ClearItems( const USHORT* pWhich );

    void                    DeleteUnchanged( const ScPatternAttr* pOldAttrs );

    static SvxCellOrientation GetCellOrientation( const SfxItemSet& rItemSet, const SfxItemSet* pCondSet = 0 );
    SvxCellOrientation      GetCellOrientation( const SfxItemSet* pCondSet = 0 ) const;

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

    void                    FillEditParaItems( SfxItemSet* pSet ) const;

    ScPatternAttr*          PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const;

    void                    SetStyleSheet(ScStyleSheet* pNewStyle);
    const ScStyleSheet*     GetStyleSheet() const  { return pStyle; }
    const String*           GetStyleName() const;
    void                    UpdateStyleSheet();
    void                    StyleToName();

    BOOL                    IsVisible() const;
    BOOL                    IsVisibleEqual( const ScPatternAttr& rOther ) const;

                            /** If font is an old symbol font StarBats/StarMath
                                with text encoding RTL_TEXTENC_SYMBOL */
    BOOL                    IsSymbolFont() const;

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

    ULONG                   GetNumberFormat( SvNumberFormatter* ) const;
    ULONG                   GetNumberFormat( SvNumberFormatter* pFormatter,
                                                const SfxItemSet* pCondSet ) const;

    long                    GetRotateVal( const SfxItemSet* pCondSet ) const;
    BYTE                    GetRotateDir( const SfxItemSet* pCondSet ) const;
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
