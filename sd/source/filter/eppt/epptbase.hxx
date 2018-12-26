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

#ifndef INCLUDED_SD_SOURCE_FILTER_EPPT_EPPTBASE_HXX
#define INCLUDED_SD_SOURCE_FILTER_EPPT_EPPTBASE_HXX

#include <memory>
#include <vector>

#include <vcl/mapmod.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <vcl/vclptr.hxx>
#include <vcl/graph.hxx>

#include "grouptable.hxx"

namespace com { namespace sun { namespace star { namespace task { class XStatusIndicator; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace awt { struct Rectangle; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XMasterPagesSupplier; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPages; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPagesSupplier; } } } }
namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShapes; } } } }

class VirtualDevice;

// PLACEMENT_ID
enum class EppLayout
{
    TITLESLIDE            =  0, /* The slide is a title slide                                                             */
    TITLEANDBODYSLIDE     =  1, /* Title and body slide                                                                   */
    TITLEMASTERSLIDE      =  2, /* Title master slide                                                                     */
    MASTERSLIDE           =  3, /* Master slide layout                                                                    */
    MASTERNOTES           =  4, /* Master notes layout                                                                    */
    NOTESTITLEBODY        =  5, /* Notes title/body layout                                                                */
    HANDOUTLAYOUT         =  6, /* Handout layout, therefore it doesn't have placeholders except header, footer, and date */
    ONLYTITLE             =  7, /* Only title placeholder                                                                 */
    TWOCOLUMNSANDTITLE    =  8, /* Body of the slide has 2 columns and a title                                            */
    TWOROWSANDTITLE       =  9, /* Slide's body has 2 rows and a title                                                    */
    RIGHTCOLUMN2ROWS      = 10, /* Body contains 2 columns, right column has 2 rows                                       */
    LEFTCOLUMN2ROWS       = 11, /* Body contains 2 columns, left column has 2 rows                                        */
    BOTTOMROW2COLUMNS     = 12, /* Body contains 2 rows, bottom row has 2 columns                                         */
    TOPROW2COLUMN         = 13, /* Body contains 2 rows, top row has 2 columns                                            */
    FOUROBJECTS           = 14, /* 4 objects                                                                              */
    BIGOBJECT             = 15, /* Big object                                                                             */
    BLANCSLIDE            = 16, /* Blank slide                                                                            */
    TITLERIGHTBODYLEFT    = 17, /* Vertical title on the right, body on the left                                          */
    TITLERIGHT2BODIESLEFT = 18  /* Vertical title on the right, body on the left split into 2 rows                        */
};

#define EPP_LAYOUT_SIZE 25

struct PHLayout
{
    EppLayout   nLayout;
    sal_uInt8 nPlaceHolder[ 8 ];

    sal_uInt8   nUsedObjectPlaceHolder;
    sal_uInt8   nTypeOfTitle;
    sal_uInt8   nTypeOfOutliner;

    bool    bTitlePossible;
    bool    bOutlinerPossible;
    bool    bSecOutlinerPossible;
};

enum PageType { NORMAL = 0, MASTER = 1, NOTICE = 2, UNDEFINED = 3, LAYOUT = 4 };

class PropValue
{
    protected:

        css::uno::Any                                    mAny;
        css::uno::Reference< css::beans::XPropertySet >  mXPropSet;

        bool    ImplGetPropertyValue( const OUString& rString );
        bool    ImplGetPropertyValue( const css::uno::Reference
                        < css::beans::XPropertySet > &, const OUString& );

    public:

        PropValue() {}

        static bool GetPropertyValue(
            css::uno::Any& rAny,
            const css::uno::Reference< css::beans::XPropertySet > &,
            const OUString& rPropertyName,
            bool bTestPropertyAvailability = false );

        static css::beans::PropertyState GetPropertyState(
            const css::uno::Reference < css::beans::XPropertySet > &,
            const OUString& rPropertyName );
};

class EscherGraphicProvider;
class PPTExBulletProvider
{
    friend struct PPTExParaSheet;

    protected:

        SvMemoryStream          aBuExPictureStream;
        SvMemoryStream          aBuExOutlineStream;
        SvMemoryStream          aBuExMasterStream;

        std::unique_ptr<EscherGraphicProvider>
                                pGraphicProv;

    public:

        sal_uInt16 GetId(Graphic const & rGraphic, Size& rGraphicSize);

        PPTExBulletProvider();
        ~PPTExBulletProvider();
};

struct FontCollectionEntry
{
        OUString                Name;
        double                  Scaling;
        sal_Int16               Family;
        sal_Int16               Pitch;
        sal_Int16               CharSet;

        OUString const          Original;

        FontCollectionEntry( const OUString& rName, sal_Int16 nFamily, sal_Int16 nPitch, sal_Int16 nCharSet ) :
                            Scaling ( 1.0 ),
                            Family  ( nFamily ),
                            Pitch   ( nPitch ),
                            CharSet ( nCharSet ),
                            Original( rName )
                            {
                                ImplInit( rName );
                            };

        explicit FontCollectionEntry( const OUString& rName ) :
                            Scaling ( 1.0 ),
                            Family  ( 0 ),
                            Pitch   ( 0 ),
                            CharSet ( 0 ),
                            Original( rName )
                            {
                                ImplInit( rName );
                            };
        ~FontCollectionEntry();

    private:

        void ImplInit( const OUString& rName );
};

class FontCollection
{
public:

    FontCollection();

    ~FontCollection();

    static short GetScriptDirection( const OUString& rText );

    sal_uInt32  GetId( FontCollectionEntry& rFontDescriptor );

    sal_uInt32  GetCount() const { return maFonts.size(); };

    const FontCollectionEntry* GetById( sal_uInt32 nId );

    FontCollectionEntry& GetLast() { return *(maFonts.rbegin()); };

private:

    VclPtr<VirtualDevice> pVDev;
    std::vector<FontCollectionEntry> maFonts;
};

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
    Color           mnFontColor;
};

struct PPTExCharSheet
{
                PPTExCharLevel  maCharLevel[ 5 ];

                explicit PPTExCharSheet( int nInstance );

                void    SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > &,
                                        FontCollection& rFontCollection, int nLevel );
                void    Write( SvStream& rSt, sal_uInt16 nLev, bool bSimpleText,
                            const css::uno::Reference< css::beans::XPropertySet > & rPagePropSet );

};

struct PPTExParaLevel
{
    bool            mbIsBullet;
    sal_uInt16      mnBulletChar;
    sal_uInt16      mnBulletFont;
    sal_uInt16      mnBulletHeight;
    sal_uInt32      mnBulletColor;

    sal_uInt16      mnAdjust;
    sal_uInt16      mnLineFeed;
    sal_uInt16      mnUpperDist;
    sal_uInt16      mnLowerDist;
    sal_uInt16      mnTextOfs;
    sal_uInt16      mnBulletOfs;
    sal_uInt16      mnDefaultTab;

    bool            mbExtendedBulletsUsed;
    sal_uInt16      mnBulletId;
    sal_uInt16      mnBulletStart;
    sal_uInt32      mnMappedNumType;
    sal_uInt32      mnNumberingType;
    sal_uInt16      mnAsianSettings;
    sal_uInt16      mnBiDi;
};

struct PPTExParaSheet
{
                PPTExBulletProvider* pBuProv;

                sal_uInt32  mnInstance;

                PPTExParaLevel  maParaLevel[ 5 ];
                PPTExParaSheet( int nInstance, sal_uInt16 nDefaultTab, PPTExBulletProvider* pProv );

                void    SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > &,
                                        FontCollection& rFontCollection, int nLevel, const PPTExCharLevel& rCharLevel );
                void    Write( SvStream& rSt, sal_uInt16 nLev, bool bSimpleText,
                    const css::uno::Reference< css::beans::XPropertySet > & rPagePropSet );
};

class PPTExStyleSheet
{

    public:

                std::unique_ptr<PPTExCharSheet>  mpCharSheet[ PPTEX_STYLESHEETENTRYS ];
                std::unique_ptr<PPTExParaSheet>  mpParaSheet[ PPTEX_STYLESHEETENTRYS ];

                PPTExStyleSheet( sal_uInt16 nDefaultTab, PPTExBulletProvider* pBuProv );
                ~PPTExStyleSheet();

                PPTExParaSheet& GetParaSheet( int nInstance ) { return *mpParaSheet[ nInstance ]; };

                void            SetStyleSheet( const css::uno::Reference< css::beans::XPropertySet > &,
                                                FontCollection& rFontCollection, int nInstance, int nLevel );
                bool        IsHardAttribute( sal_uInt32 nInstance, sal_uInt32 nLevel, PPTExTextAttr eAttr, sal_uInt32 nValue );

                static sal_uInt32 SizeOfTxCFStyleAtom() { return 24; }
                void            WriteTxCFStyleAtom( SvStream& rSt );
};

class PPTWriterBase : public PropValue, public GroupTable
{
protected:
    css::uno::Reference< css::frame::XModel >                 mXModel;
    css::uno::Reference< css::task::XStatusIndicator >        mXStatusIndicator;

    bool            mbStatusIndicator;

    css::uno::Reference< css::drawing::XDrawPagesSupplier >   mXDrawPagesSupplier;
    css::uno::Reference< css::drawing::XMasterPagesSupplier > mXMasterPagesSupplier;
    css::uno::Reference< css::drawing::XDrawPages >           mXDrawPages;
    css::uno::Reference< css::drawing::XDrawPage >            mXDrawPage;
    css::uno::Reference< css::beans::XPropertySet >           mXPagePropSet;
    css::uno::Reference< css::beans::XPropertySet >           mXBackgroundPropSet;
    css::uno::Reference< css::drawing::XShapes >              mXShapes;
    css::uno::Reference< css::drawing::XShape >               mXShape;
    css::awt::Size         maSize;
    css::awt::Point        maPosition;
    ::tools::Rectangle           maRect;
    OString        mType;
    bool            mbPresObj;
    bool            mbEmptyPresObj;
    bool            mbIsBackgroundDark;
    sal_Int32           mnAngle;

    sal_uInt32          mnPages;            ///< number of Slides ( w/o master pages & notes & handout )
    sal_uInt32          mnMasterPages;

    Fraction const     maFraction;
    MapMode const      maMapModeSrc;
    MapMode const      maMapModeDest;
    css::awt::Size     maDestPageSize;
    css::awt::Size     maPageSize; // #i121183# Keep size in logic coordinates (100th mm)
    css::awt::Size     maNotesPageSize;

    PageType                        meLatestPageType;
    std::vector< std::unique_ptr<PPTExStyleSheet> > maStyleSheetList;
    PPTExStyleSheet*                mpStyleSheet;

    FontCollection      maFontCollection;

    virtual void ImplWriteSlide( sal_uInt32 /* nPageNum */, sal_uInt32 /* nMasterNum */, sal_uInt16 /* nMode */,
                                 bool /* bHasBackground */, css::uno::Reference< css::beans::XPropertySet > const & /* aXBackgroundPropSet */ ) {}
    virtual void ImplWriteNotes( sal_uInt32 nPageNum ) = 0;
    virtual void ImplWriteSlideMaster( sal_uInt32 /* nPageNum */, css::uno::Reference< css::beans::XPropertySet > const & /* aXBackgroundPropSet */ ) {}

    virtual void exportPPTPre( const std::vector< css::beans::PropertyValue >& ) {}
    virtual void exportPPTPost() {}

    virtual bool ImplCreateDocument()=0;
    virtual bool ImplCreateMainNotes()=0;

    bool GetStyleSheets();
    bool GetShapeByIndex( sal_uInt32 nIndex, bool bGroup );

    bool CreateMainNotes();

    css::awt::Size   MapSize( const css::awt::Size& );
    css::awt::Point  MapPoint( const css::awt::Point& );
    ::tools::Rectangle        MapRectangle( const css::awt::Rectangle& );

    bool ContainsOtherShapeThanPlaceholders();

public:
    PPTWriterBase();
    PPTWriterBase( const css::uno::Reference< css::frame::XModel > & rModel,
                   const css::uno::Reference< css::task::XStatusIndicator > & rStatInd );

    virtual ~PPTWriterBase();

    void exportPPT(const std::vector< css::beans::PropertyValue >&);

    bool InitSOIface();
    bool GetPageByIndex( sal_uInt32 nIndex, PageType );
    sal_uInt32 GetMasterIndex( PageType ePageType );
    void SetCurrentStyleSheet( sal_uInt32 nPageNum );

    bool GetPresObj() { return mbPresObj; }

    static PHLayout const & GetLayout( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    static PHLayout const & GetLayout( sal_Int32 nOffset );
    static sal_Int32 GetLayoutOffset( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    static sal_Int32 GetLayoutOffsetFixed( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );

    bool CreateSlide( sal_uInt32 nPageNum );
    bool CreateSlideMaster( sal_uInt32 nPageNum );
    bool CreateNotes( sal_uInt32 nPageNum );

    static sal_Int8 GetTransition( sal_Int16 nTransitionType, sal_Int16 nTransitionSubtype, css::presentation::FadeEffect eEffect, sal_uInt8& nDirection );
    static sal_Int8 GetTransition( css::presentation::FadeEffect eEffect, sal_uInt8& nDirection );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
