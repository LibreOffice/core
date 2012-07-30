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

#ifndef EPPT_EPPTBASE_HXX
#define EPPT_EPPTBASE_HXX

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vcl/mapmod.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>

#include "grouptable.hxx"

// PLACEMENT_ID
#define EPP_LAYOUT_TITLESLIDE             0 /* The slide is a title slide                                                             */
#define EPP_LAYOUT_TITLEANDBODYSLIDE      1 /* Title and body slide                                                                   */
#define EPP_LAYOUT_TITLEMASTERSLIDE       2 /* Title master slide                                                                     */
#define EPP_LAYOUT_MASTERSLIDE            3 /* Master slide layout                                                                    */
#define EPP_LAYOUT_MASTERNOTES            4 /* Master notes layout                                                                    */
#define EPP_LAYOUT_NOTESTITLEBODY         5 /* Notes title/body layout                                                                */
#define EPP_LAYOUT_HANDOUTLAYOUT          6 /* Handout layout, therefore it doesn't have placeholders except header, footer, and date */
#define EPP_LAYOUT_ONLYTITLE              7 /* Only title placeholder                                                                 */
#define EPP_LAYOUT_2COLUMNSANDTITLE       8 /* Body of the slide has 2 columns and a title                                            */
#define EPP_LAYOUT_2ROWSANDTITLE          9 /* Slide's body has 2 rows and a title                                                    */
#define EPP_LAYOUT_RIGHTCOLUMN2ROWS      10 /* Body contains 2 columns, right column has 2 rows                                       */
#define EPP_LAYOUT_LEFTCOLUMN2ROWS       11 /* Body contains 2 columns, left column has 2 rows                                        */
#define EPP_LAYOUT_BOTTOMROW2COLUMNS     12 /* Body contains 2 rows, bottom row has 2 columns                                         */
#define EPP_LAYOUT_TOPROW2COLUMN         13 /* Body contains 2 rows, top row has 2 columns                                            */
#define EPP_LAYOUT_4OBJECTS              14 /* 4 objects                                                                              */
#define EPP_LAYOUT_BIGOBJECT             15 /* Big object                                                                             */
#define EPP_LAYOUT_BLANCSLIDE            16 /* Blank slide                                                                            */
#define EPP_LAYOUT_TITLERIGHTBODYLEFT    17 /* Vertical title on the right, body on the left                                          */
#define EPP_LAYOUT_TITLERIGHT2BODIESLEFT 18 /* Vertical title on the right, body on the left split into 2 rows                        */

#define EPP_LAYOUT_SIZE 25

class PptEscherEx;

struct PHLayout
{
    sal_Int32   nLayout;
    sal_uInt8   nPlaceHolder[ 8 ];

    sal_uInt8   nUsedObjectPlaceHolder;
    sal_uInt8   nTypeOfTitle;
    sal_uInt8   nTypeOfOutliner;

    sal_Bool    bTitlePossible;
    sal_Bool    bOutlinerPossible;
    sal_Bool    bSecOutlinerPossible;
};

enum PageType { NORMAL = 0, MASTER = 1, NOTICE = 2, UNDEFINED = 3, LAYOUT = 4 };

class PropValue
{
    protected :

        ::com::sun::star::uno::Any                              mAny;

        ::com::sun::star::uno::Reference
            < ::com::sun::star::beans::XPropertySet >           mXPropSet;

        sal_Bool    ImplGetPropertyValue( const String& rString );
        sal_Bool    ImplGetPropertyValue( const ::com::sun::star::uno::Reference
                        < ::com::sun::star::beans::XPropertySet > &, const String& );

    public :

        PropValue() {}

        PropValue( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet )
            : mXPropSet( rXPropSet )
        {}

        ::com::sun::star::uno::Any GetAny() { return mAny; }

        static sal_Bool GetPropertyValue(
                ::com::sun::star::uno::Any& rAny,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                        const String& rPropertyName,
                            sal_Bool bTestPropertyAvailability = sal_False );

        static ::com::sun::star::beans::PropertyState GetPropertyState(
                    const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > &,
                        const String& rPropertyName );
};

class EscherGraphicProvider;
class PPTExBulletProvider
{
    friend struct PPTExParaSheet;

    protected :

        SvMemoryStream          aBuExPictureStream;
        SvMemoryStream          aBuExOutlineStream;
        SvMemoryStream          aBuExMasterStream;

        EscherGraphicProvider*  pGraphicProv;

    public :

        sal_uInt16              GetId( const rtl::OString& rUniqueId, Size& rGraphicSize );

                                PPTExBulletProvider();
                                ~PPTExBulletProvider();
};

struct FontCollectionEntry
{
        String                  Name;
        double                  Scaling;
        sal_Int16               Family;
        sal_Int16               Pitch;
        sal_Int16               CharSet;

        String                  Original;
        sal_Bool                bIsConverted;

        FontCollectionEntry( const String& rName, sal_Int16 nFamily, sal_Int16 nPitch, sal_Int16 nCharSet ) :
                            Scaling ( 1.0 ),
                            Family  ( nFamily ),
                            Pitch   ( nPitch ),
                            CharSet ( nCharSet ),
                            Original( rName )
                            {
                                ImplInit( rName );
                            };

        FontCollectionEntry( const String& rName ) :
                            Scaling ( 1.0 ),
                            Original( rName )
                            {
                                ImplInit( rName );
                            };
        ~FontCollectionEntry();

    private :

        FontCollectionEntry() {};

        void ImplInit( const String& rName );
};

class VirtualDevice;
class FontCollection
{
public :

    FontCollection();

    ~FontCollection();

    short GetScriptDirection( const rtl::OUString& rText ) const;

    sal_uInt32  GetId( FontCollectionEntry& rFontDescriptor );

    inline sal_uInt32  GetCount() const { return maFonts.size(); };

    const FontCollectionEntry* GetById( sal_uInt32 nId );

    FontCollectionEntry& GetLast() { return *(maFonts.rbegin()); };

private:

    VirtualDevice* pVDev;
    boost::ptr_vector<FontCollectionEntry> maFonts;
};

// ------------------------------------------------------------------------

#define PPTEX_STYLESHEETENTRYS  9

enum PPTExTextAttr
{
    ParaAttr_BulletOn,
    ParaAttr_BuHardFont,
    ParaAttr_BuHardColor,
    ParaAttr_BuHardHeight,
    ParaAttr_BulletChar,
    ParaAttr_BulletFont,
    ParaAttr_BulletHeight,
    ParaAttr_BulletColor,
    ParaAttr_Adjust,
    ParaAttr_LineFeed,
    ParaAttr_UpperDist,
    ParaAttr_LowerDist,
    ParaAttr_TextOfs,
    ParaAttr_BulletOfs,
    ParaAttr_DefaultTab,
    ParaAttr_AsianLB_1,
    ParaAttr_AsianLB_2,
    ParaAttr_AsianLB_3,
    ParaAttr_BiDi,
    CharAttr_Bold,
    CharAttr_Italic,
    CharAttr_Underline,
    CharAttr_Shadow,
    CharAttr_Strikeout,
    CharAttr_Embossed,
    CharAttr_Font,
    CharAttr_AsianOrComplexFont,
    CharAttr_Symbol,
    CharAttr_FontHeight,
    CharAttr_FontColor,
    CharAttr_Escapement
};

struct PPTExCharLevel
{
    sal_uInt16      mnFlags;
    sal_uInt16      mnFont;
    sal_uInt16      mnAsianOrComplexFont;
    sal_uInt16      mnFontHeight;
    sal_uInt16      mnEscapement;
    sal_uInt32      mnFontColor;
};

struct PPTExCharSheet
{
                PPTExCharLevel  maCharLevel[ 5 ];

                PPTExCharSheet( int nInstance );

                void    SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                                        FontCollection& rFontCollection, int nLevel );
                void    Write( SvStream& rSt, PptEscherEx* pEx, sal_uInt16 nLev, sal_Bool bFirst, sal_Bool bSimpleText,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPagePropSet );

};

struct PPTExParaLevel
{
    sal_Bool        mbIsBullet;
    sal_uInt16      mnBulletChar;
    sal_uInt16      mnBulletFont;
    sal_uInt16      mnBulletHeight;
    sal_uInt32      mnBulletColor;

    sal_uInt16      mnAdjust;
    sal_Int16       mnOOAdjust;
    sal_uInt16      mnLineFeed;
    sal_uInt16      mnUpperDist;
    sal_uInt16      mnLowerDist;
    sal_uInt16      mnTextOfs;
    sal_uInt16      mnBulletOfs;
    sal_uInt16      mnDefaultTab;

    sal_Bool        mbExtendedBulletsUsed;
    sal_uInt16      mnBulletId;
    sal_uInt16      mnBulletStart;
    sal_uInt32      mnMappedNumType;
    sal_uInt32      mnNumberingType;
    sal_uInt16      mnAsianSettings;
    sal_uInt16      mnBiDi;
};

struct PPTExParaSheet
{
                PPTExBulletProvider& rBuProv;

                sal_uInt32  mnInstance;

                PPTExParaLevel  maParaLevel[ 5 ];
                PPTExParaSheet( int nInstance, sal_uInt16 nDefaultTab, PPTExBulletProvider& rProv );

                void    SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                                        FontCollection& rFontCollection, int nLevel, const PPTExCharLevel& rCharLevel );
                void    Write( SvStream& rSt, PptEscherEx* pEx, sal_uInt16 nLev, sal_Bool bFirst, sal_Bool bSimpleText,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPagePropSet );
};

class PPTExStyleSheet
{

    public :

                PPTExCharSheet*     mpCharSheet[ PPTEX_STYLESHEETENTRYS ];
                PPTExParaSheet*     mpParaSheet[ PPTEX_STYLESHEETENTRYS ];

                PPTExStyleSheet( sal_uInt16 nDefaultTab, PPTExBulletProvider& rBuProv );
                ~PPTExStyleSheet();

                PPTExParaSheet& GetParaSheet( int nInstance ) { return *mpParaSheet[ nInstance ]; };
                PPTExCharSheet& GetCharSheet( int nInstance ) { return *mpCharSheet[ nInstance ]; };

                void            SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                                                FontCollection& rFontCollection, int nInstance, int nLevel );
                sal_Bool        IsHardAttribute( sal_uInt32 nInstance, sal_uInt32 nLevel, PPTExTextAttr eAttr, sal_uInt32 nValue );

                sal_uInt32      SizeOfTxCFStyleAtom() const;
                void            WriteTxCFStyleAtom( SvStream& rSt );
};

// ------------------------------------------------------------------------

class PPTWriterBase : public PropValue, public GroupTable
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 mXModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >        mXStatusIndicator;

    sal_Bool            mbStatusIndicator;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPagesSupplier >   mXDrawPagesSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPagesSupplier > mXMasterPagesSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages >           mXDrawPages;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >            mXDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           mXPagePropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           mXBackgroundPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >              mXShapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >               mXShape;
    ::com::sun::star::awt::Size         maSize;
    ::com::sun::star::awt::Point        maPosition;
    Rectangle           maRect;
    rtl::OString        mType;
    sal_Bool            mbPresObj;
    sal_Bool            mbEmptyPresObj;
    sal_Int32           mnAngle;

    sal_uInt32          mnPages;            ///< number of Slides ( w/o master pages & notes & handout )
    sal_uInt32          mnMasterPages;

    Fraction                        maFraction;
    MapMode                         maMapModeSrc;
    MapMode                         maMapModeDest;
    ::com::sun::star::awt::Size     maDestPageSize;
    ::com::sun::star::awt::Size     maNotesPageSize;

    PageType                        meLatestPageType;
    std::vector< PPTExStyleSheet* > maStyleSheetList;
    PPTExStyleSheet*                mpStyleSheet;

    FontCollection      maFontCollection;

    virtual void ImplWriteSlide( sal_uInt32 /* nPageNum */, sal_uInt32 /* nMasterNum */, sal_uInt16 /* nMode */,
                                 sal_Bool /* bHasBackground */, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > /* aXBackgroundPropSet */ ) {}
    virtual void ImplWriteNotes( sal_uInt32 nPageNum ) = 0;
    virtual void ImplWriteSlideMaster( sal_uInt32 /* nPageNum */, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > /* aXBackgroundPropSet */ ) {}
    virtual void ImplWriteLayout( sal_Int32 /* nOffset */, sal_uInt32 /* nMasterNum */ ) {}

    virtual void exportPPTPre( const std::vector< com::sun::star::beans::PropertyValue >& ) {}
    virtual void exportPPTPost() {}

    virtual sal_Bool ImplCreateDocument()=0;
    virtual sal_Bool ImplCreateMainNotes()=0;

    sal_Bool GetStyleSheets();
    sal_Bool GetShapeByIndex( sal_uInt32 nIndex, sal_Bool bGroup = sal_False );

    sal_Bool CreateMainNotes();

    ::com::sun::star::awt::Size   MapSize( const ::com::sun::star::awt::Size& );
    ::com::sun::star::awt::Point  MapPoint( const ::com::sun::star::awt::Point& );
    Rectangle                     MapRectangle( const ::com::sun::star::awt::Rectangle& );

    sal_Bool ContainsOtherShapeThanPlaceholders( sal_Bool bForOOMLX );

public:
    PPTWriterBase();
    PPTWriterBase( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rStatInd );

    virtual ~PPTWriterBase();

    void exportPPT(const std::vector< com::sun::star::beans::PropertyValue >&);

    sal_Bool InitSOIface();
    sal_Bool GetPageByIndex( sal_uInt32 nIndex, PageType );
    sal_uInt32 GetMasterIndex( PageType ePageType );
    sal_Bool SetCurrentStyleSheet( sal_uInt32 nPageNum );

    sal_Bool GetPresObj() { return mbPresObj; }

    PHLayout& GetLayout( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet ) const;
    PHLayout& GetLayout( sal_Int32 nOffset ) const;
    sal_Int32 GetLayoutOffset( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet ) const;
    sal_Int32 GetLayoutOffsetFixed( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet ) const;

    sal_Bool CreateSlide( sal_uInt32 nPageNum );
    sal_Bool CreateSlideMaster( sal_uInt32 nPageNum );
    sal_Bool CreateNotes( sal_uInt32 nPageNum );

    static sal_Int8 GetTransition( sal_Int16 nTransitionType, sal_Int16 nTransitionSubtype, ::com::sun::star::presentation::FadeEffect eEffect, sal_uInt8& nDirection );
    static sal_Int8 GetTransition( ::com::sun::star::presentation::FadeEffect eEffect, sal_uInt8& nDirection );
};

#define PPT_TRANSITION_TYPE_NONE            0
#define PPT_TRANSITION_TYPE_RANDOM          1
#define PPT_TRANSITION_TYPE_BLINDS          2
#define PPT_TRANSITION_TYPE_CHECKER         3
#define PPT_TRANSITION_TYPE_COVER           4
#define PPT_TRANSITION_TYPE_DISSOLVE        5
#define PPT_TRANSITION_TYPE_FADE            6
#define PPT_TRANSITION_TYPE_PULL            7
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11
#define PPT_TRANSITION_TYPE_SPLIT          13

// effects, new in xp
#define PPT_TRANSITION_TYPE_DIAMOND         17
#define PPT_TRANSITION_TYPE_PLUS            18
#define PPT_TRANSITION_TYPE_WEDGE           19
#define PPT_TRANSITION_TYPE_PUSH            20
#define PPT_TRANSITION_TYPE_COMB            21
#define PPT_TRANSITION_TYPE_NEWSFLASH       22
#define PPT_TRANSITION_TYPE_SMOOTHFADE      23
#define PPT_TRANSITION_TYPE_WHEEL           26
#define PPT_TRANSITION_TYPE_CIRCLE          27

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
