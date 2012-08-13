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

#ifndef _EPPT_HXX_
#define _EPPT_HXX_
#include <vector>
#include "escherex.hxx"
#include <tools/solar.h>
#include <sot/storage.hxx>
#include <tools/gen.hxx>
#include <vcl/graph.hxx>
#include <unotools/fontcvt.hxx>
#include <tools/string.hxx>
#include "pptexanimations.hxx"
#include <pptexsoundcollection.hxx>

#include "text.hxx"

// ------------------------------------------------------------------------

#include <vcl/mapmod.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/DashStyle.hpp>
#include <com/sun/star/drawing/HatchStyle.hpp>
#include <com/sun/star/drawing/LineEndType.hpp>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/drawing/TextAdjust.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolygonFlags.hpp>
#include <com/sun/star/drawing/XUniversalShapeDescriptor.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/drawing/XConnectorShape.hpp>
#include <com/sun/star/drawing/BezierPoint.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/PresentationRange.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <epptbase.hxx>

#define EPP_MAINMASTER_PERSIST_KEY      0x80010000
#define EPP_MAINNOTESMASTER_PERSIST_KEY 0x80020000
#define EPP_MAINSLIDE_PERSIST_KEY       0x80030000
#define EPP_MAINNOTES_PERSIST_KEY       0x80040000

#define EPP_Persist_Document            0x80080000
#define EPP_Persist_MainMaster          0x80100000
#define EPP_Persist_MainNotes           0x80200000
#define EPP_Persist_Slide               0x80400000
#define EPP_Persist_Notes               0x80800000
#define EPP_Persist_CurrentPos          0x81000000
#define EPP_Persist_VBAInfoAtom         0x84000000
#define EPP_Persist_ExObj               0x88000000

#define EPP_TEXTSTYLE_NORMAL            0x00000001
#define EPP_TEXTSTYLE_TITLE             0x00000010
#define EPP_TEXTSTYLE_BODY              0x00000100
#define EPP_TEXTSTYLE_TEXT              0x00001000

struct EPPTHyperlink
{
    String      aURL;
    sal_uInt32  nType;      // bit 0-7 : type       ( 1: click action to a slide )
                            //                      ( 2: hyperlink url )
                            // bit 8-23: index
                            // bit 31  : hyperlink is attached to a shape

    EPPTHyperlink( const String rURL, sal_uInt32 nT ) :
        aURL        ( rURL ),
        nType       ( nT ){};
};

enum PPTExOleObjEntryType
{
    NORMAL_OLE_OBJECT, OCX_CONTROL
};

struct PPTExOleObjEntry
{
    PPTExOleObjEntryType    eType;
    sal_uInt32              nOfsA; ///< offset to the EPP_ExOleObjAtom in mpExEmbed (set at creation)
    sal_uInt32              nOfsB; ///< offset to the EPP_ExOleObjStg

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >    xControlModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       xShape;

    PPTExOleObjEntry( PPTExOleObjEntryType eT, sal_uInt32 nOfs ) :
        eType   ( eT ),
        nOfsA   ( nOfs ) {};
};

struct TextRuleEntry
{
    int                 nPageNumber;
    SvMemoryStream*     pOut;

    TextRuleEntry( int nPg ) :
        nPageNumber( nPg ),
        pOut ( NULL ){};

    ~TextRuleEntry() { delete pOut; };
};

class TextObjBinary : public TextObj
{
public:
    TextObjBinary( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > &
                   rXText, int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rBuProv ) : TextObj( rXText, nInstance, rFontCollection, rBuProv ) {}
    void            Write( SvStream* pStrm );
    void            WriteTextSpecInfo( SvStream* pStrm );
};

// ------------------------------------------------------------------------

struct CellBorder;
class PPTWriter : public PPTWriterBase, public PPTExBulletProvider
{
        sal_uInt32                      mnCnvrtFlags;
        sal_Bool                        mbStatus;
        sal_Bool                        mbUseNewAnimations;
        sal_uInt32                      mnStatMaxValue;
        sal_uInt32                      mnLatestStatValue;

        std::vector<rtl::OUString>      maSlideNameList;
        rtl::OUString                   maBaseURI;

        ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText >             mXText;             // TextRef des globalen Text
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >             mXCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >              mXCursorText;       // TextRef des Teilstuecks des Cursors
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           mXCursorPropSet;    // die Properties des Teilstueckes
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >              mXTextField;
        sal_uInt32          mnTextStyle;

        sal_Bool            mbFontIndependentLineSpacing;
        sal_uInt32          mnTextSize;

        SvStorageRef        mrStg;
        SvStream*           mpCurUserStrm;
        SvStream*           mpStrm;
        SvStream*           mpPicStrm;
        PptEscherEx*        mpPptEscherEx;

        std::vector<PPTExOleObjEntry*> maExOleObj;
        sal_uInt32          mnVBAOleOfs;
        SvMemoryStream*     mpVBA;
        sal_uInt32          mnExEmbed;
        SvMemoryStream*     mpExEmbed;

        sal_uInt32          mnDrawings;         // anzahl Slides +  masterpages + notes +  handout
        sal_uInt32          mnPagesWritten;
        sal_uInt32          mnUniqueSlideIdentifier;
        sal_uInt32          mnTxId;             // Identifier determined by the HOST (PP) ????
        sal_uInt32          mnDiaMode;          // 0 -> manuell
                                                // 1 -> halbautomatisch
                                                // 2 -> automatisch

        sal_uInt32          mnShapeMasterTitle;
        sal_uInt32          mnShapeMasterBody;

        std::vector<EPPTHyperlink>  maHyperlink;

        ppt::ExSoundCollection  maSoundCollection;

        void                ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId );

        sal_uInt32          ImplProgBinaryTag( SvStream* pOutStrm = NULL );
        sal_uInt32          ImplProgBinaryTagContainer( SvStream* pOutStrm = NULL, SvMemoryStream* pBinTag = NULL );
        sal_uInt32          ImplProgTagContainer( SvStream* pOutStrm = NULL, SvMemoryStream* pBinTag = NULL );
        sal_uInt32          ImplOutlineViewInfoContainer( SvStream* pOutStrm = NULL );
        sal_uInt32          ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pOutStrm = NULL );
        sal_uInt32          ImplVBAInfoContainer( SvStream* pOutStrm = NULL );
        sal_uInt32          ImplDocumentListContainer( SvStream* pOutStrm = NULL );
        sal_uInt32          ImplMasterSlideListContainer( SvStream* pOutStrm = NULL );

    public:
        static void         WriteCString( SvStream&, const String&, sal_uInt32 nInstance = 0 );

    protected:

        sal_Bool            ImplCreateDocumentSummaryInformation();
        sal_Bool            ImplCreateCurrentUserStream();
        void                ImplCreateHeaderFooterStrings( SvStream& rOut,
                                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet );
        void                ImplCreateHeaderFooters( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet );
        virtual sal_Bool        ImplCreateDocument();
        sal_Bool            ImplCreateHyperBlob( SvMemoryStream& rStream );
        sal_uInt32          ImplInsertBookmarkURL( const String& rBookmark, const sal_uInt32 nType,
            const String& rStringVer0, const String& rStringVer1, const String& rStringVer2, const String& rStringVer3 );
        virtual sal_Bool        ImplCreateMainNotes();
        sal_Bool            ImplCreateNotes( sal_uInt32 nPageNum );
        void                ImplWriteBackground( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXBackgroundPropSet );
  void              ImplWriteVBA();
        void                ImplWriteOLE();
        sal_Bool            ImplWriteAtomEnding();

        void                ImplFlipBoundingBox( EscherPropertyContainer& rPropOpt );
        sal_Bool            ImplGetText();
        sal_Bool            ImplCreatePresentationPlaceholder( const sal_Bool bMaster, const PageType PageType,
                                const sal_uInt32 StyleInstance, const sal_uInt8 PlaceHolderId );
        sal_Bool            ImplGetEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                                ::com::sun::star::presentation::AnimationEffect& eEffect,
                                ::com::sun::star::presentation::AnimationEffect& eTextEffect,
                                sal_Bool& bHasSound );
        void                ImplWriteObjectEffect( SvStream& rSt,
                                ::com::sun::star::presentation::AnimationEffect eEffect,
                                ::com::sun::star::presentation::AnimationEffect eTextEffect,
                                sal_uInt16 nOrder );
        void                ImplWriteClickAction( SvStream& rSt, ::com::sun::star::presentation::ClickAction eAction, sal_Bool bMediaClickAction );
        void                ImplWriteParagraphs( SvStream& rOutStrm, TextObj& rTextObj );
        void                ImplWritePortions( SvStream& rOutStrm, TextObj& rTextObj );
        void                ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance, sal_uInt32 nAtomInstance,
                                TextRuleEntry* pTextRule, SvStream& rExtBu, EscherPropertyContainer* );
        void                ImplAdjustFirstLineLineSpacing( TextObj& rTextObj, EscherPropertyContainer& rPropOpt );
        void                ImplCreateShape( sal_uInt32 nType, sal_uInt32 nFlags, EscherSolverContainer& );
        void                ImplCreateTextShape( EscherPropertyContainer&, EscherSolverContainer&, sal_Bool bFill );

        void                ImplWritePage( const PHLayout& rLayout,
                                                EscherSolverContainer& rSolver,
                                                    PageType ePageType,
                                                        sal_Bool bMaster,
                                                            int nPageNumber = 0 );
        void                ImplCreateCellBorder( const CellBorder* pCellBorder, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2 );
        void                ImplCreateTable( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rXShape, EscherSolverContainer& aSolverContainer,
                                EscherPropertyContainer& aPropOpt );

        sal_Bool                            ImplCloseDocument();        // die font-, hyper-, Soundliste wird geschrieben ..

        virtual void        ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterID, sal_uInt16 nMode,
                                            sal_Bool bHasBackground, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
        virtual void        ImplWriteNotes( sal_uInt32 nPageNum );
        virtual void        ImplWriteSlideMaster( sal_uInt32 nPageNum, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );

    public:
                                PPTWriter( SvStorageRef& rSvStorage,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rStatInd,
                                                SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags );

                                ~PPTWriter();

        sal_Bool                IsValid() const { return mbStatus; };

        virtual void        exportPPTPre( const std::vector< com::sun::star::beans::PropertyValue >& );
        virtual void        exportPPTPost( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
