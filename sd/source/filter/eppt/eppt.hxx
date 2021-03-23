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

#pragma once
#include <memory>
#include <vector>
#include "escherex.hxx"
#include <sal/types.h>
#include <sot/storage.hxx>
#include "pptexsoundcollection.hxx"

#include "text.hxx"

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>

#include "epptbase.hxx"

namespace com::sun::star::awt { class XControlModel; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::task { class XStatusIndicator; }
namespace com::sun::star::text { class XSimpleText; }

class SotStorage;

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
    OUString    aURL;
    sal_uInt32  nType;      // bit 0-7 : type       ( 1: click action to a slide )
                            //                      ( 2: hyperlink url )
                            // bit 8-23: index
                            // bit 31  : hyperlink is attached to a shape

    EPPTHyperlink( const OUString& rURL, sal_uInt32 nT ) :
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

    css::uno::Reference< css::awt::XControlModel >    xControlModel;
    css::uno::Reference< css::drawing::XShape >       xShape;

    PPTExOleObjEntry(PPTExOleObjEntryType eT, sal_uInt32 nOfs)
        : eType(eT)
        , nOfsA(nOfs)
        , nOfsB(0)
    {}
};

struct TextRuleEntry
{
    std::unique_ptr<SvMemoryStream>  pOut;
};

class TextObjBinary : public TextObj
{
public:
    TextObjBinary( css::uno::Reference< css::text::XSimpleText > const & rXText,
                   int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rBuProv ) : TextObj( rXText, nInstance, rFontCollection, rBuProv ) {}
    void            Write( SvStream* pStrm );
    void            WriteTextSpecInfo( SvStream* pStrm );
};

struct CellBorder;
class PPTWriter final : public PPTWriterBase, public PPTExBulletProvider
{
        sal_uInt32                      mnCnvrtFlags;
        bool                        mbStatus;
        sal_uInt32                      mnStatMaxValue;
        sal_uInt32                      mnLatestStatValue;

        std::vector<OUString>      maSlideNameList;
        OUString                   maBaseURI;

        css::uno::Reference< css::text::XSimpleText >             mXText;             // TextRef of the global text
        sal_uInt32          mnTextStyle;

        bool                mbFontIndependentLineSpacing;
        sal_uInt32          mnTextSize;

        tools::SvRef<SotStorage>        mrStg;
        tools::SvRef<SotStorageStream>  mpCurUserStrm;
        tools::SvRef<SotStorageStream>  mpStrm;
        tools::SvRef<SotStorageStream>  mpPicStrm;
        std::unique_ptr<PptEscherEx>    mpPptEscherEx;

        std::vector<std::unique_ptr<PPTExOleObjEntry>> maExOleObj;
        sal_uInt32          mnVBAOleOfs;
        SvMemoryStream*     mpVBA;
        sal_uInt32          mnExEmbed;
        std::unique_ptr<SvMemoryStream> mpExEmbed;

        sal_uInt32          mnPagesWritten;
        sal_uInt32          mnTxId;             // Identifier determined by the HOST (PP) ????
        sal_uInt32          mnDiaMode;          // 0 -> manual
                                                // 1 -> semi-automatic
                                                // 2 -> automatic

        sal_uInt32          mnShapeMasterTitle;
        sal_uInt32          mnShapeMasterBody;

        std::vector<EPPTHyperlink>  maHyperlink;

        ppt::ExSoundCollection  maSoundCollection;

        void                ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId );

        sal_uInt32          ImplProgBinaryTag( SvStream* pOutStrm );
        sal_uInt32          ImplProgBinaryTagContainer( SvStream* pOutStrm, SvMemoryStream* pBinTag );
        sal_uInt32          ImplProgTagContainer( SvStream* pOutStrm, SvMemoryStream* pBinTag = nullptr );
        static sal_uInt32   ImplOutlineViewInfoContainer( SvStream* pOutStrm );
        static sal_uInt32   ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pOutStrm );
        sal_uInt32          ImplVBAInfoContainer( SvStream* pOutStrm );
        sal_uInt32          ImplDocumentListContainer( SvStream* pOutStrm );
        sal_uInt32          ImplMasterSlideListContainer( SvStream* pOutStrm );

    public:
        static void         WriteCString( SvStream&, const OUString&, sal_uInt32 nInstance = 0 );

    private:

        void                ImplCreateDocumentSummaryInformation();
        bool                ImplCreateCurrentUserStream();
        static void         ImplCreateHeaderFooterStrings( SvStream& rOut,
                                css::uno::Reference< css::beans::XPropertySet > const & rXPagePropSet );
        void                ImplCreateHeaderFooters( css::uno::Reference< css::beans::XPropertySet > const & rXPagePropSet );
        virtual bool        ImplCreateDocument() override;
        void                ImplCreateHyperBlob( SvMemoryStream& rStream );
        sal_uInt32          ImplInsertBookmarkURL( const OUString& rBookmark, const sal_uInt32 nType,
                                const OUString& rStringVer0, const OUString& rStringVer1, const OUString& rStringVer2, const OUString& rStringVer3 );
        virtual bool        ImplCreateMainNotes() override;
        void                ImplWriteBackground( css::uno::Reference< css::beans::XPropertySet > const & rXBackgroundPropSet );
        void                ImplWriteVBA();
        void                ImplWriteOLE();
        void                ImplWriteAtomEnding();

        void                ImplFlipBoundingBox( EscherPropertyContainer& rPropOpt );
        bool                ImplGetText();
        bool                ImplCreatePresentationPlaceholder( const bool bMaster,
                                const sal_uInt32 StyleInstance, const sal_uInt8 PlaceHolderId );
        static bool         ImplGetEffect( const css::uno::Reference< css::beans::XPropertySet > &,
                                css::presentation::AnimationEffect& eEffect,
                                css::presentation::AnimationEffect& eTextEffect,
                                bool& bHasSound );
        void                ImplWriteClickAction( SvStream& rSt, css::presentation::ClickAction eAction, bool bMediaClickAction );
        void                ImplWriteParagraphs( SvStream& rOutStrm, TextObj& rTextObj );
        void                ImplWritePortions( SvStream& rOutStrm, TextObj& rTextObj );
        void                ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance, sal_uInt32 nAtomInstance,
                                TextRuleEntry* pTextRule, SvStream& rExtBu, EscherPropertyContainer* );
        void                ImplAdjustFirstLineLineSpacing( TextObj& rTextObj, EscherPropertyContainer& rPropOpt );
        void                ImplCreateShape( sal_uInt32 nType, ShapeFlag nFlags, EscherSolverContainer& );
        void                ImplCreateTextShape( EscherPropertyContainer&, EscherSolverContainer&, bool bFill );

        void                ImplWritePage( const PHLayout& rLayout,
                                           EscherSolverContainer& rSolver,
                                           PageType ePageType,
                                           bool bMaster,
                                           int nPageNumber = 0 );
        bool                ImplCreateCellBorder( const CellBorder* pCellBorder, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2 );
        void                ImplCreateTable( css::uno::Reference< css::drawing::XShape > const & rXShape, EscherSolverContainer& aSolverContainer,
                                EscherPropertyContainer& aPropOpt );

        bool                ImplCloseDocument();        // we write the font, hyper and sound list

        virtual void        ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterID, sal_uInt16 nMode,
                                            bool bHasBackground, css::uno::Reference< css::beans::XPropertySet > const & aXBackgroundPropSet ) override;
        virtual void        ImplWriteNotes( sal_uInt32 nPageNum ) override;
        virtual void        ImplWriteSlideMaster( sal_uInt32 nPageNum, css::uno::Reference< css::beans::XPropertySet > const & aXBackgroundPropSet ) override;

    public:
                                PPTWriter( tools::SvRef<SotStorage> const & rSvStorage,
                                            css::uno::Reference< css::frame::XModel > const & rModel,
                                            css::uno::Reference< css::task::XStatusIndicator > const & rStatInd,
                                            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags );

                                virtual ~PPTWriter() override;

        bool                IsValid() const { return mbStatus; };

        virtual void        exportPPTPre( const std::vector< css::beans::PropertyValue >& ) override;
        virtual void        exportPPTPost( ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
