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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR_HXX

#include <rtl/ustring.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/util.hxx>
#include <editeng/frmdir.hxx>
#include <fltshell.hxx>

#include <svx/svdobj.hxx>

#include <vector>
#include <stack>
#include <string_view>
#include <deque>
#include <map>
#include <utility>
#include <memory>

#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "ww8glsy.hxx"
#include "ww8graf.hxx"
#include "wrtww8.hxx"
#include <msfilter.hxx>
#include <xmloff/odffields.hxx>
#include <IMark.hxx>

#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <swtypes.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ndtxt.hxx>
#include <editeng/lrspitem.hxx>
#include <o3tl/sorted_vector.hxx>
#include <oox/ole/olehelper.hxx>

#define SW_UD_IMAPDATA      2

class SwDoc;
class SwPaM;
class SfxPoolItem;
class SwTextFormatColl;
class SwPageDesc;
class SvxBoxItem;
class SwFormat;
class SwNodeIndex;
class SwFlyFrameFormat;
class SwAttrSet;
class SwNumRule;
class SwFrameFormat;
class Writer;
class SwFormatField;
class WW8Fib;
class WW8PLCFMan;
struct WW8PLCFManResult;
class WW8RStyle;
class WW8PLCF_HdFt;
class WW8ScannerBase;
struct WW8FlyPara;
struct WW8SwFlyPara;
struct WW8_PIC;
class WW8TabDesc;
struct WW8_SHD;
struct WW8_OLST;
class SwNumFormat;
struct WW8_ANLD;
struct WW8_ANLV;
struct WW8_DO;
struct WW8_DPHEAD;
struct WW8_FSPA;
class SdrModel;
class SdrPage;
class SdrObject;
class SdrTextObj;
class SdrUnoObj;
class Size;
class EditEngine;
struct SwPosition;
class WW8ReaderSave;
struct WW8PicDesc;
class Graphic;
class SwFieldType;
class SotStorage;
class SwAttrSet;
class GDIMetaFile;
struct ESelection;
class SfxItemSet;
class OutlinerParaObject;

namespace com::sun::star{
    namespace beans{ class XPropertySet;}
    namespace form { class XFormComponent;}
    namespace drawing{class XShape;}
    namespace lang{class XMultiServiceFactory;}
}

// defines only for the WW8-variable of the INI file
#define WW8FL_NO_STYLES      2
#define WW8FL_NO_GRAF     0x80

#define WW8FL_NO_OUTLINE         0x1000
#define WW8FL_NO_IMPLPASP        0x4000  // no implicit para space
#define WW8FL_NO_GRAFLAYER       0x8000

// Add-on-filter-flags, valid from Winword 8 on
#define WW8FL_NO_FLY_FOR_TXBX         1

//            List-Manager (from Ver8 on)

struct WW8LFOInfo;

class WW8Reader : public StgReader
{
    std::shared_ptr<SvStream> mDecodedStream;
    virtual ErrCode Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
    ErrCode OpenMainStream( tools::SvRef<SotStorageStream>& rRef, sal_uInt16& rBuffSize );
    ErrCode DecryptDRMPackage();
public:
    WW8Reader() {}
    virtual SwReaderType GetReaderType() override;

    virtual bool HasGlossaries() const override;
    virtual bool ReadGlossaries( SwTextBlocks&, bool bSaveRelFiles ) const override;
};

class SwWW8ImplReader;
struct WW8LSTInfo;
class WW8ListManager
{
public:
    WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_);
    //Min and Max possible List Levels in Word
    enum ListLevel {nMinLevel=1, nMaxLevel=9};
    //the rParaSprms returns back the original word paragraph indent
    //sprms which were attached to the original numbering format
    SwNumRule* GetNumRuleForActivation(sal_uInt16 nLFOPosition, const sal_uInt8 nLevel,
        std::vector<sal_uInt8> &rParaSprms, SwTextNode *pNode=nullptr);
    SwNumRule* CreateNextRule(bool bSimple);
    ~WW8ListManager();
    SwNumRule* GetNumRule(size_t i);
    size_t GetWW8LSTInfoNum() const{return maLSTInfos.size();}
    static SvxNumType GetSvxNumTypeFromMSONFC(sal_uInt16 nMSONFC);

private:
    wwSprmParser maSprmParser;
    SwWW8ImplReader& rReader;
    SwDoc&           rDoc;
    const WW8Fib&    rFib;
    SvStream&        rSt;
    std::vector<std::unique_ptr<WW8LSTInfo>> maLSTInfos;
    std::vector<std::unique_ptr<WW8LFOInfo>> m_LFOInfos;// D. from PLF LFO, sorted exactly like in the WW8 Stream
    sal_uInt16       nUniqueList; // current number for creating unique list names
    SprmResult GrpprlHasSprm(sal_uInt16 nId, sal_uInt8& rSprms, sal_uInt8 nLen);
    WW8LSTInfo* GetLSTByListId(    sal_uInt32  nIdLst     ) const;
    //the rParaSprms returns back the original word paragraph indent
    //sprms which are attached to this numbering level
    bool ReadLVL(SwNumFormat& rNumFormat, std::unique_ptr<SfxItemSet>& rpItemSet, sal_uInt16 nLevelStyle,
        bool bSetStartNo, sal_uInt16 nLevel, ww::bytes &rParaSprms);

    // character attributes from GrpprlChpx
    typedef std::unique_ptr<SfxItemSet> WW8aISet[nMaxLevel];
    // character style pointer
    typedef SwCharFormat* WW8aCFormat[nMaxLevel];

    void AdjustLVL(sal_uInt8 nLevel, SwNumRule& rNumRule, WW8aISet const & rListItemSet,
        WW8aCFormat& aCharFormat, bool& bNewCharFormatCreated,
        const OUString& aPrefix = OUString());

    void ImplDestroy();

    WW8ListManager(const WW8ListManager&) = delete;
    WW8ListManager& operator=(const WW8ListManager&) = delete;
    sal_uInt16 nLastLFOPosition;
};

struct WW8FlyPara
{                       // WinWord-attributes
                        // Attention: *DO NOT* reorder, since parts will be
                        // compared using memcmp
    bool bVer67;
    sal_Int16 nSp26, nSp27;         // raw position
    sal_Int16 nSp45, nSp28;         // width / height
    sal_Int16 nLeMgn, nRiMgn, nUpMgn, nLoMgn;           // borders
    sal_uInt8 nSp29;                 // raw binding + alignment
    sal_uInt8 nSp37;                 // Wrap-Mode ( 1 / 2; 0 = no Apo ? )
    WW8_BRCVer9_5 brc;          // borders Top, Left, Bottom, Right, Between
    bool bBorderLines;          // border lines
    bool bGrafApo;              // true: this frame is only used to position
                                // the contained graphics *not* as a character
    bool mbVertSet;             // true if vertical positioning has been set

    WW8FlyPara(bool bIsVer67, const WW8FlyPara* pSrc = nullptr);
    bool operator==(const WW8FlyPara& rSrc) const;
    void Read(sal_uInt8 nSprm29, WW8PLCFx_Cp_FKP* pPap);
    void ReadFull(sal_uInt8 nSprm29, SwWW8ImplReader* pIo);
    void Read(sal_uInt8 nSprm29, WW8RStyle const * pStyle);
    void ApplyTabPos(const WW8_TablePos *pTabPos);
    bool IsEmpty() const;
};

class SwWW8StyInf
{
    OUString        m_sWWStyleName;
    sal_uInt16      m_nWWStyleId;
public:
    rtl_TextEncoding m_eLTRFontSrcCharSet;    // rtl_TextEncoding for the font
    rtl_TextEncoding m_eRTLFontSrcCharSet;    // rtl_TextEncoding for the font
    rtl_TextEncoding m_eCJKFontSrcCharSet;    // rtl_TextEncoding for the font
    SwFormat*      m_pFormat;
    std::shared_ptr<WW8FlyPara> m_xWWFly;
    SwNumRule*  m_pOutlineNumrule;
    tools::Long        m_nFilePos;
    sal_uInt16      m_nBase;
    sal_uInt16      m_nFollow;
    sal_uInt16      m_nLFOIndex;
    sal_uInt8        m_nListLevel;

    // WW8 outline level is zero-based:
    // 0: outline level 1
    // 1: outline level 2
    // ...
    // 8: outline level 9
    // 9: body text
    sal_uInt8 mnWW8OutlineLevel;

    sal_uInt16  m_n81Flags;           // for bold, italic, ...
    sal_uInt16  m_n81BiDiFlags;       // for bold, italic, ...
    std::shared_ptr<SvxLRSpaceItem> maWordLR;
    bool m_bValid;            // empty of valid
    bool m_bImported;         // for recursive imports
    bool m_bColl;             // true-> pFormat is SwTextFormatColl
    bool m_bImportSkipped;    // only true if !bNewDoc && existing style
    bool m_bHasStyNumRule;    // true-> named NumRule in style
    bool m_bHasBrokenWW6List; // true-> WW8+ style has a WW7- list
    bool m_bListRelevantIndentSet; //true if this style's indent has
                                 //been explicitly set, it's set to the value
                                 //of pFormat->GetItemState(RES_LR_SPACE, false)
                                 //if it was possible to get the ItemState
                                 //for L of the LR space independently
    bool m_bParaAutoBefore;   // For Auto spacing before a paragraph
    bool m_bParaAutoAfter;    // For Auto Spacing after a paragraph
    sal_Int16 m_nRelativeJustify;

    SwWW8StyInf() :
        m_sWWStyleName( OUString() ),
        m_nWWStyleId( 0 ),
        m_eLTRFontSrcCharSet(0),
        m_eRTLFontSrcCharSet(0),
        m_eCJKFontSrcCharSet(0),
        m_pFormat( nullptr ),
        m_pOutlineNumrule( nullptr ),
        m_nFilePos( 0 ),
        m_nBase( 0 ),
        m_nFollow( 0 ),
        m_nLFOIndex( USHRT_MAX ),
        m_nListLevel(MAXLEVEL),
        mnWW8OutlineLevel( MAXLEVEL ),
        m_n81Flags( 0 ),
        m_n81BiDiFlags(0),
        maWordLR(std::make_shared<SvxLRSpaceItem>(RES_LR_SPACE)),
        m_bValid(false),
        m_bImported(false),
        m_bColl(false),
        m_bImportSkipped(false),
        m_bHasStyNumRule(false),
        m_bHasBrokenWW6List(false),
        m_bListRelevantIndentSet(false),
        m_bParaAutoBefore(false),
        m_bParaAutoAfter(false),
        m_nRelativeJustify(-1)

    {}

    void SetOrgWWIdent( const OUString& rName, const sal_uInt16 nId )
    {
        m_sWWStyleName = rName;
        m_nWWStyleId = nId;

        // apply default WW8 outline level to WW8 Built-in Heading styles
        if (IsWW8BuiltInHeadingStyle())
        {
            mnWW8OutlineLevel = m_nWWStyleId - 1;
        }
    }

    const OUString& GetOrgWWName() const
    {
        return m_sWWStyleName;
    }

    bool HasWW8OutlineLevel() const
    {
        return (m_pFormat != nullptr && (MAXLEVEL > mnWW8OutlineLevel));
    }

    bool IsOutlineNumbered() const
    {
        return m_pOutlineNumrule && HasWW8OutlineLevel();
    }

    const SwNumRule* GetOutlineNumrule() const
    {
        return m_pOutlineNumrule;
    }
    rtl_TextEncoding GetCharSet() const;
    rtl_TextEncoding GetCJKCharSet() const;

    sal_uInt16 GetWWStyleId() const
    {
        return m_nWWStyleId;
    }

    bool IsWW8BuiltInHeadingStyle() const
    {
        return GetWWStyleId() >= 1 && GetWWStyleId() <= 9;
    }

    bool IsWW8BuiltInDefaultStyle() const
    {
        return GetWWStyleId() == 0;
    }

    static sal_uInt8
    WW8OutlineLevelToOutlinelevel(const sal_uInt8 nWW8OutlineLevel)
    {
        if (nWW8OutlineLevel < MAXLEVEL)
        {
            if (nWW8OutlineLevel == 9)
            {
                return 0; // no outline level --> body text
            }
            else
            {
                return nWW8OutlineLevel + 1; // outline level 1..9
            }
        }

        return 0;
    }
};

//            Stack

class SwWW8FltControlStack : public SwFltControlStack
{
private:
    SwWW8ImplReader& rReader;
    std::unique_ptr<SfxItemSet> m_xScratchSet;
    sal_uInt16 nToggleAttrFlags;
    sal_uInt16 nToggleBiDiAttrFlags;
    SwWW8FltControlStack(const SwWW8FltControlStack&) = delete;
    SwWW8FltControlStack& operator=(const SwWW8FltControlStack&) = delete;
    const SwNumFormat* GetNumFormatFromStack(const SwPosition &rPos,
        const SwTextNode &rTextNode);
protected:
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry& rEntry) override;

public:
    SwWW8FltControlStack(SwDoc& rDo, sal_uLong nFieldFl, SwWW8ImplReader& rReader_ )
        : SwFltControlStack( rDo, nFieldFl ), rReader( rReader_ ),
        nToggleAttrFlags(0), nToggleBiDiAttrFlags(0)
    {}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr);

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId, bool bTstEnd=true, tools::Long nHand=LONG_MAX, bool consumedByField=false) override;

    void SetToggleAttr(sal_uInt8 nId, bool bOn)
    {
        if( bOn )
            nToggleAttrFlags |= (1 << nId);
        else
            nToggleAttrFlags &= ~(1 << nId);
    }

    sal_uInt16 GetToggleAttrFlags() const { return nToggleAttrFlags; }

    void SetToggleBiDiAttr(sal_uInt8 nId, bool bOn)
    {
        if( bOn )
            nToggleBiDiAttrFlags |= (1 << nId);
        else
            nToggleBiDiAttrFlags &= ~(1 << nId);
    }

    sal_uInt16 GetToggleBiDiAttrFlags() const { return nToggleBiDiAttrFlags; }
    void SetToggleAttrFlags(sal_uInt16 nFlags) { nToggleAttrFlags = nFlags; }
    void SetToggleBiDiAttrFlags(sal_uInt16 nFlags) {nToggleBiDiAttrFlags = nFlags;}

    const SfxPoolItem* GetFormatAttr(const SwPosition& rPos, sal_uInt16 nWhich);
    template<class T> const T* GetFormatAttr( const SwPosition& rPos, TypedWhichId<T> nWhich )
    {
        return static_cast<const T*>(GetFormatAttr(rPos, sal_uInt16(nWhich)));
    }
    const SfxPoolItem* GetStackAttr(const SwPosition& rPos, sal_uInt16 nWhich);
};

//The only thing this is for is RES_FLTR_ANCHOR, anything else is an error.
//For graphics whose anchoring position would otherwise be automatically moved
//along by the insertion of text.
class SwWW8FltAnchorStack : public SwFltControlStack
{
public:
    SwWW8FltAnchorStack(SwDoc& rDo, sal_uLong nFieldFl)
        : SwFltControlStack( rDo, nFieldFl ) {}
    void AddAnchor(const SwPosition& rPos,SwFrameFormat *pFormat);
    void Flush();
private:
    SwWW8FltAnchorStack(const SwWW8FltAnchorStack&) = delete;
    SwWW8FltAnchorStack& operator=(const SwWW8FltAnchorStack&) = delete;
};


namespace SwWW8
{
    struct ltstr
    {
        bool operator()(const OUString &r1, std::u16string_view r2) const
        {
            return r1.compareToIgnoreAsciiCase(r2)<0;
        }
    };
};

class SwWW8ReferencedFltEndStack : public SwFltEndStack
{
public:
    SwWW8ReferencedFltEndStack( SwDoc& rDo, sal_uLong nFieldFl )
        : SwFltEndStack( rDo, nFieldFl )
        , aReferencedTOCBookmarks()
    {}

    // Keep track of referenced TOC bookmarks in order to suppress the import
    // of unreferenced ones.
    std::set<OUString, SwWW8::ltstr> aReferencedTOCBookmarks;
protected:
    virtual void SetAttrInDoc( const SwPosition& rTmpPos,
                               SwFltStackEntry& rEntry ) override;
};

class SwWW8FltRefStack final : public SwFltEndStack
{
public:
    SwWW8FltRefStack(SwDoc& rDo, sal_uLong nFieldFl)
        : SwFltEndStack( rDo, nFieldFl )
        , aFieldVarNames()
    {}
    bool IsFootnoteEdnBkmField(const SwFormatField& rFormatField, sal_uInt16& rBkmNo);

    //Keep track of variable names created with fields, and the bookmark
    //mapped to their position, hopefully the same, but very possibly
    //an additional pseudo bookmark
    std::map<OUString, OUString, SwWW8::ltstr> aFieldVarNames;
private:
    SwFltStackEntry *RefToVar(const SwField* pField,SwFltStackEntry& rEntry);
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry& rEntry) override;
    SwWW8FltRefStack(const SwWW8FltRefStack&) = delete;
    SwWW8FltRefStack& operator=(const SwWW8FltRefStack&) = delete;
};

template< typename Type >
inline bool get_flag( Type nBitField, Type nMask )
{ return (nBitField & nMask) != 0; }

template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue, ReturnType nMax )
{ return static_cast< ReturnType >( std::min< Type >( nValue, nMax ) ); }

template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue )
{ return ulimit_cast( nValue, std::numeric_limits< ReturnType >::max() ); }

class SwMacroInfo : public SdrObjUserData
{
public:
    SwMacroInfo();
    virtual ~SwMacroInfo() override;

    SwMacroInfo(SwMacroInfo const &) = default;
    SwMacroInfo(SwMacroInfo &&) = default;
    SwMacroInfo & operator =(SwMacroInfo const &) = delete; // due to SdrObjUserData
    SwMacroInfo & operator =(SwMacroInfo &&) = delete; // due to SdrObjUserData

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* pObj ) const override;

    void SetHlink( const OUString& rHlink ) { maHlink = rHlink; }
    const OUString& GetHlink() const { return maHlink; }
    void SetTarFrame( const OUString& rTarFrame ) { maTarFrame = rTarFrame; }
    const OUString& GetTarFrame() const { return maTarFrame; }
    void SetShapeId( sal_Int32 rShapeId ) { mnShapeId = rShapeId; }
    const sal_Int32& GetShapeId() const { return mnShapeId; }
    void SetName( const OUString& rName ) { maNameStr = rName; }
    const OUString& GetName() const { return maNameStr; }

private:
    sal_Int32 mnShapeId;
    OUString maHlink;
    OUString maNameStr;
    OUString maTarFrame;
};

struct HyperLinksTable
{
    OUString hLinkAddr;
    OUString tarFrame;
};

namespace sw
{
    namespace hack
    {
        class Position
        {
        private:
            SwNodeIndex maPtNode;
            sal_Int32 mnPtContent;
        public:
            explicit Position(const SwPosition &rPos);
            operator SwPosition() const;
            const SwNodeIndex& GetPtNode() const { return maPtNode; };
            sal_Int32 GetPtContent() const { return mnPtContent; };
        };
    }

    auto FilterControlChars(OUString const& rString) -> OUString;
}

class WW8FieldEntry
{
    private:
        OUString msBookmarkName;
        OUString msMarkType;
        OUString msMarkCode;
        ::sw::mark::IFieldmark::parameter_map_t maParams;

    public:
        sw::hack::Position maStartPos;
        sal_uInt16 mnFieldId;
        sal_uLong mnObjLocFc;
        WW8FieldEntry(SwPosition const &rPos, sal_uInt16 nFieldId) noexcept;
        WW8FieldEntry(const WW8FieldEntry &rOther) noexcept;
        WW8FieldEntry &operator=(const WW8FieldEntry &rOther) noexcept;
        void Swap(WW8FieldEntry &rOther) noexcept;

        SwNodeIndex GetPtNode() const { return maStartPos.GetPtNode(); };
        sal_Int32 GetPtContent() const { return maStartPos.GetPtContent(); };

        const OUString& GetBookmarkName() const { return msBookmarkName;}
        const OUString& GetBookmarkCode() const { return msMarkCode;}
        void SetBookmarkName(const OUString& bookmarkName);
        void SetBookmarkType(const OUString& bookmarkType);
        void SetBookmarkCode(const OUString& bookmarkCode);
        ::sw::mark::IFieldmark::parameter_map_t& getParameters() { return maParams;}
};

//    mini marker for some flags

class WW8ReaderSave
{
private:
    WW8PLCFxSaveAll maPLCFxSave;
    std::shared_ptr<SwUnoCursor> mxTmpPos;
    std::deque<bool> maOldApos;
    std::deque<WW8FieldEntry> maOldFieldStack;
    std::unique_ptr<SwWW8FltControlStack> mxOldStck;
    std::unique_ptr<SwWW8FltAnchorStack> mxOldAnchorStck;
    std::unique_ptr<sw::util::RedlineStack> mxOldRedlines;
    std::shared_ptr<WW8PLCFMan> mxOldPlcxMan;
    std::unique_ptr<WW8FlyPara> mpWFlyPara;
    std::unique_ptr<WW8SwFlyPara> mpSFlyPara;
    SwPaM* mpPreviousNumPaM;
    const SwNumRule* mpPrevNumRule;
    std::unique_ptr<WW8TabDesc> mxTableDesc;
    int mnInTable;
    sal_uInt16 mnCurrentColl;
    sal_Unicode mcSymbol;
    bool mbIgnoreText;
    bool mbSymbol;
    bool mbHdFtFootnoteEdn;
    bool mbTxbxFlySection;
    bool mbAnl;
    bool mbInHyperlink;
    bool mbPgSecBreak;
    bool mbWasParaEnd;
    bool mbHasBorder;
    bool mbFirstPara;
public:
    WW8ReaderSave(SwWW8ImplReader* pRdr, WW8_CP nStart=-1);
    void Restore(SwWW8ImplReader* pRdr);
    const SwPosition &GetStartPos() const { return *mxTmpPos->GetPoint(); }
};

enum class eF_ResT { OK, TEXT, TAGIGN, READ_FSPA };

class SwWW8Shade
{
public:
    Color aColor;
    SwWW8Shade(bool bVer67, const WW8_SHD& rSHD);
    SwWW8Shade(Color nFore, Color nBack, sal_uInt16 nIndex)
    {
        SetShade(nFore, nBack, nIndex);
    }
private:
    void SetShade(Color nFore, Color nBack, sal_uInt16 nIndex);
};

//    Formulas

enum SwWw8ControlType
{
    WW8_CT_EDIT,
    WW8_CT_CHECKBOX,
    WW8_CT_DROPDOWN
};

class WW8FormulaControl
{
protected:
    SwWW8ImplReader &mrRdr;

    WW8FormulaControl(WW8FormulaControl const&) = delete;
    WW8FormulaControl& operator=(WW8FormulaControl const&) = delete;

public:
    WW8FormulaControl(const OUString& rN, SwWW8ImplReader &rRdr)
        : mrRdr(rRdr), mfUnknown(0), mfDropdownIndex(0),
        mfToolTip(0), mfNoMark(0), mfType(0),
        mfUnused(0), mhpsCheckBox(20), mnChecked(0), mnMaxLen(0),
        mbHelp(false), msName( rN )
    {
    }
    sal_uInt8 mfUnknown:2;
    sal_uInt8 mfDropdownIndex:6;
    sal_uInt8 mfToolTip:1;
    sal_uInt8 mfNoMark:1;
    sal_uInt8 mfType:3;
    sal_uInt8 mfUnused:3;

    sal_uInt16 mhpsCheckBox;
    sal_uInt16 mnChecked;

    /// FFData.cch in the spec: maximum length, in characters, of the value of the textbox.
    sal_uInt16 mnMaxLen;
    OUString msTitle;
    OUString msDefault;
    OUString msFormatting;
    bool mbHelp;
    OUString msHelp;
    OUString msToolTip;
    OUString msEntryMcr;
    OUString msExitMcr;
    std::vector<OUString> maListEntries;
    virtual ~WW8FormulaControl() {}
    void FormulaRead(SwWw8ControlType nWhich,SvStream *pD);
    virtual bool Import(const css::uno::Reference< css::lang::XMultiServiceFactory> &rServiceFactory,
        css::uno::Reference< css::form::XFormComponent> &rFComp,
        css::awt::Size &rSz) = 0;
    OUString msName;
};

class WW8FormulaCheckBox : public WW8FormulaControl
{
private:
    WW8FormulaCheckBox(const WW8FormulaCheckBox&) = delete;
    WW8FormulaCheckBox& operator=(const WW8FormulaCheckBox&) = delete;

public:
    explicit WW8FormulaCheckBox(SwWW8ImplReader &rR);

    virtual bool Import(const css::uno::Reference< css::lang::XMultiServiceFactory> &rServiceFactory,
        css::uno::Reference< css::form::XFormComponent> &rFComp,
        css::awt::Size &rSz) override;
};

class WW8FormulaListBox : public WW8FormulaControl
{
private:
    WW8FormulaListBox(const WW8FormulaListBox&) = delete;
    WW8FormulaListBox& operator=(const WW8FormulaListBox&) = delete;

public:
    explicit WW8FormulaListBox(SwWW8ImplReader &rR);

    virtual bool Import(const css::uno::Reference< css::lang::XMultiServiceFactory> &rServiceFactory,
        css::uno::Reference< css::form::XFormComponent> &rFComp,
        css::awt::Size &rSz) override;
};

class WW8FormulaEditBox : public WW8FormulaControl
{
private:
    WW8FormulaEditBox(const WW8FormulaEditBox&) = delete;
    WW8FormulaEditBox& operator=(const WW8FormulaEditBox&) = delete;
public:
    explicit WW8FormulaEditBox(SwWW8ImplReader &rR);
    //no real implementation, return false
    virtual bool Import(const css::uno::Reference< css::lang::XMultiServiceFactory> & /* rServiceFactory */,
        css::uno::Reference< css::form::XFormComponent> & /* rFComp */,
        css::awt::Size & /* rSz */) override { return false; }
};

class SwMSConvertControls: public oox::ole::MSConvertOCXControls
{
public:
    SwMSConvertControls( SfxObjectShell const *pDSh, SwPaM *pP );
    void InsertFormula( WW8FormulaControl &rFormula);
    virtual bool InsertControl(const css::uno::Reference< css::form::XFormComponent >& rFComp,
        const css::awt::Size& rSize,
        css::uno::Reference<  css::drawing::XShape > *pShape, bool bFloatingCtrl) override;
    void ExportControl(WW8Export &rWrt, const SdrUnoObj& rFormObj);
    bool ReadOCXStream( tools::SvRef<SotStorage> const & rSrc1,
        css::uno::Reference< css::drawing::XShape > *pShapeRef,
        bool bFloatingCtrl=false );
private:
    SwPaM *pPaM;
    sal_uInt32 mnObjectId;
};

class SwMSDffManager : public SvxMSDffManager
{
private:
    SwWW8ImplReader& rReader;
    SvStream *pFallbackStream;
    std::unordered_map<sal_uInt32, Graphic> aOldEscherBlipCache;

    virtual bool GetOLEStorageName( sal_uInt32 nOLEId, OUString& rStorageName,
        tools::SvRef<SotStorage>& rSrcStorage, css::uno::Reference < css::embed::XStorage >& rDestStorage ) const override;
    virtual bool ShapeHasText( sal_uLong nShapeId, sal_uLong nFilePos ) const override;
    // #i32596# - new parameter <_nCalledByGroup>, which
    // indicates, if the OLE object is imported inside a group object
    virtual SdrObject* ImportOLE( sal_uInt32 nOLEId,
                                  const Graphic& rGrf,
                                  const tools::Rectangle& rBoundRect,
                                  const tools::Rectangle& rVisArea,
                                  const int _nCalledByGroup ) const override;

    SwMSDffManager(const SwMSDffManager&) = delete;
    SwMSDffManager& operator=(const SwMSDffManager&) = delete;
public:
    static sal_uInt32 GetFilterFlags();
    static sal_Int32 GetEscherLineMatch(MSO_LineStyle eStyle, MSO_SPT eShapeType,
        sal_Int32 &rThick);
    SwMSDffManager( SwWW8ImplReader& rRdr, bool bSkipImages );
    void DisableFallbackStream();
    void EnableFallbackStream();
protected:
    virtual SdrObject* ProcessObj( SvStream& rSt, DffObjData& rObjData, SvxMSDffClientData& rData, tools::Rectangle& rTextRect, SdrObject* pObj ) override;
};

class wwSection
{
public:
    explicit wwSection(const SwPosition &rPos);
    SEPr maSep;
    WW8_BRCVer9 brc[4];
    SwNodeIndex maStart;
    SwSection *mpSection;
    SwPageDesc *mpPage;
    SvxFrameDirection meDir;

    sal_uInt32 nPgWidth;
    sal_uInt32 nPgLeft;
    sal_uInt32 nPgRight;
    sal_uInt32 nPgGutter;
    bool m_bRtlGutter = false;

    css::drawing::TextVerticalAdjust mnVerticalAdjustment;
    sal_uInt8 mnBorders;
    bool mbHasFootnote;
    void SetDirection();
    bool IsContinuous() const { return maSep.bkc == 0; }
    bool IsNotProtected() const { return maSep.fUnlocked != 0; }
    bool IsVertical() const;
    sal_Int16 NoCols() const { return maSep.ccolM1 + 1; }
    sal_Int32 StandardColSeparation() const { return maSep.dxaColumns; }
    bool HasTitlePage() const { return maSep.fTitlePage != 0; }
    sal_uInt16 PageStartAt() const { return maSep.pgnStart; }
    bool PageRestartNo() const { return maSep.fPgnRestart != 0; }
    bool IsBiDi() const { return maSep.fBiDi != 0; }
    sal_uInt32 GetPageWidth() const { return nPgWidth; }
    sal_uInt32 GetTextAreaWidth() const
        { return GetPageWidth() - GetPageLeft() - nPgGutter - GetPageRight(); }
    sal_uInt32 GetPageHeight() const { return maSep.yaPage; }
    sal_uInt32 GetPageLeft() const { return nPgLeft; }
    sal_uInt32 GetPageRight() const { return nPgRight; }
    bool IsLandScape() const { return maSep.dmOrientPage != 0; }
    bool IsFixedHeightHeader() const { return maSep.dyaTop < 0; }
    bool IsFixedHeightFooter() const { return maSep.dyaBottom < 0; }
};

class wwSectionManager
{
private:
    /*
    A queue of the ms sections in the document
    */
    SwWW8ImplReader& mrReader;
    std::deque<wwSection> maSegments;
    typedef std::deque<wwSection>::iterator mySegIter;

    //Num of page desc's entered into the document
    sal_uInt16 mnDesc;

    struct wwULSpaceData
    {
        bool bHasHeader, bHasFooter;
        sal_uInt32 nSwHLo, nSwFUp, nSwUp,  nSwLo;
        wwULSpaceData()
            : bHasHeader(false)
            , bHasFooter(false)
            , nSwHLo(0)
            , nSwFUp(0)
            , nSwUp(0)
            , nSwLo(0)
        {}
    };

    void SetSegmentToPageDesc(const wwSection &rSection, bool bIgnoreCols);

    void GetPageULData(const wwSection &rNewSection,
        wwULSpaceData& rData) const;
    static void SetPageULSpaceItems(SwFrameFormat &rFormat, wwULSpaceData const & rData,
        const wwSection &rSection);

    static void SetPage(SwPageDesc &rPageDesc, SwFrameFormat &rFormat,
        const wwSection &rSection, bool bIgnoreCols);

    static void SetNumberingType(const wwSection &rNewSection, SwPageDesc &rPageDesc);

    void SetUseOn(wwSection &rSection);
    void SetHdFt(wwSection const &rSection, int nSect, const wwSection *pPrevious);

    SwSectionFormat *InsertSection(SwPaM const & rMyPaM, wwSection &rSection);
    static bool SetCols(SwFrameFormat &rFormat, const wwSection &rSection,
        sal_uInt32 nNetWidth);
    bool SectionIsProtected(const wwSection &rSection) const;
    static void SetLeftRight(wwSection &rSection);
    /*
     The segment we're inserting, the start of the segments container, and the
     nodeindex of where we want the page break to (normally the segments start
     position
    */
    SwFormatPageDesc SetSwFormatPageDesc(mySegIter const &rIter, mySegIter const &rStart,
        bool bIgnoreCols);

    wwSectionManager(const wwSectionManager&) = delete;
    wwSectionManager& operator=(const wwSectionManager&) = delete;
public:
    explicit wwSectionManager(SwWW8ImplReader &rReader) : mrReader(rReader), mnDesc(0)
        {}
    void SetCurrentSectionHasFootnote();
    void SetCurrentSectionVerticalAdjustment(const css::drawing::TextVerticalAdjust nVA);
    bool CurrentSectionIsVertical() const;
    bool CurrentSectionIsProtected() const;
    void PrependedInlineNode(const SwPosition &rPos, const SwNode &rNode);
    sal_uInt16 CurrentSectionColCount() const;
    bool WillHavePageDescHere(const SwNodeIndex& rIdx) const;
    void CreateSep(const tools::Long nTextPos);
    void InsertSegments();
    void JoinNode(const SwPosition &rPos, const SwNode &rNode);
    sal_uInt32 GetPageLeft() const;
    sal_uInt32 GetPageRight() const;
    sal_uInt32 GetPageWidth() const;
    sal_uInt32 GetWWPageTopMargin() const;
    sal_uInt32 GetTextAreaWidth() const;
};

//Various writer elements like frames start off containing a blank paragraph,
//sometimes this paragraph turns out to be extraneous, e.g. the frame should
//only contain a table with no trailing paragraph.

//We want to remove these extra paragraphs, but removing them during the parse
//is problematic, because we don't want to remove any paragraphs that are still
//addressed by property entries in a SwFltControlStack which have not yet been
//committed to the document.

//Safest thing is to not delete SwTextNodes from a document during import, and
//remove these extraneous paragraphs at the end after all SwFltControlStack are
//destroyed.
class wwExtraneousParas
{
private:
    struct TextNodeListener : public SwClient
    {
        TextNodeListener(SwTextNode* pTextNode, wwExtraneousParas* pOwner)
            : m_pTextNode(pTextNode)
            , m_pOwner(pOwner)
        {
        }
        bool operator<(const TextNodeListener& rOther) const
        {
            return m_pTextNode->GetIndex() < rOther.m_pTextNode->GetIndex();
        }
        virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

        SwTextNode *m_pTextNode;
        wwExtraneousParas* m_pOwner;
    };
    /*
    A vector of SwTextNodes to erase from a document after import is complete
    */
    std::set<TextNodeListener> m_aTextNodes;
    SwDoc& m_rDoc;

    void remove_if_present(SwModify* pModify);

    wwExtraneousParas(wwExtraneousParas const&) = delete;
    wwExtraneousParas& operator=(wwExtraneousParas const&) = delete;
public:
    explicit wwExtraneousParas(SwDoc &rDoc) : m_rDoc(rDoc) {}
    ~wwExtraneousParas() { delete_all_from_doc(); }
    void insert(SwTextNode *pTextNode);
    void remove_if_present(SwTextNode *pTextNode)
    {
        remove_if_present(static_cast<SwModify*>(pTextNode));
    }
    void delete_all_from_doc();
};

class wwFrameNamer
{
private:
    OUString msSeed;
    sal_Int32 mnImportedGraphicsCount;
    bool mbIsDisabled;

    wwFrameNamer(wwFrameNamer const&) = delete;
    wwFrameNamer& operator=(wwFrameNamer const&) = delete;

public:
    void SetUniqueGraphName(SwFrameFormat *pFrameFormat, std::u16string_view rFixedPart);
    wwFrameNamer(bool bIsDisabled, const OUString &rSeed)
        : msSeed(rSeed), mnImportedGraphicsCount(0), mbIsDisabled(bIsDisabled)
    {
    }
};

class wwSectionNamer
{
private:
    const SwDoc &mrDoc;
    OUString msFileLinkSeed;
    int mnFileSectionNo;
    wwSectionNamer(const wwSectionNamer&) = delete;
    wwSectionNamer& operator=(const wwSectionNamer&) = delete;
public:
    OUString UniqueName();
    wwSectionNamer(const SwDoc &rDoc, const OUString &rSeed)
        : mrDoc(rDoc), msFileLinkSeed(rSeed), mnFileSectionNo(0)
        { }
};

class FootnoteDescriptor
{
public:
    ManTypes meType;
    bool mbAutoNum;
    WW8_CP mnStartCp;
    WW8_CP mnLen;
};

struct ApoTestResults
{
    bool mbStartApo;
    bool mbStopApo;
    bool m_bHasSprm37;
    bool m_bHasSprm29;
    sal_uInt8 m_nSprm29;
    WW8FlyPara* mpStyleApo;
    ApoTestResults()
        : mbStartApo(false), mbStopApo(false), m_bHasSprm37(false)
        , m_bHasSprm29(false), m_nSprm29(0), mpStyleApo(nullptr) {}
    bool HasStartStop() const { return (mbStartApo || mbStopApo); }
    bool HasFrame() const { return (m_bHasSprm29 || m_bHasSprm37 || mpStyleApo); }
};

struct ANLDRuleMap
{
    OUString msOutlineNumRule;    // WinWord 6 numbering, variant 1
    OUString msNumberingNumRule;  // WinWord 6 numbering, variant 2
    SwNumRule* GetNumRule(const SwDoc& rDoc, sal_uInt8 nNumType);
    void SetNumRule(const OUString& rNumRule, sal_uInt8 nNumType);
};

struct SprmReadInfo;
class SwDocShell;
struct WW8PostProcessAttrsInfo
{
    bool mbCopy;
    WW8_CP mnCpStart;
    WW8_CP mnCpEnd;
    SwPaM mPaM;
    SfxItemSet mItemSet;

    WW8PostProcessAttrsInfo(WW8_CP nCpStart, WW8_CP nCpEnd, SwPaM & rPaM);
};

#define MAX_COL 64  // WW6-description: 32, WW6-UI: 31 & WW8-UI: 63!

struct WW8TabBandDesc
{
    WW8TabBandDesc* pNextBand;
    short nGapHalf;
    short mnDefaultLeft;
    short mnDefaultTop;
    short mnDefaultRight;
    short mnDefaultBottom;
    bool mbHasSpacing;
    short nLineHeight;
    short nRows;
    sal_uInt16 maDirections[MAX_COL + 1];
    short nCenter[MAX_COL + 1]; // X-edge of all cells of this band
    short nWidth[MAX_COL + 1];  // length of all cells of this band
    short nWwCols;      // sal_uInt8 would be sufficient, alignment -> short
    short nSwCols;      // SW: number of columns for the writer
    bool bLEmptyCol;    // SW: an additional empty column at the left
    bool bREmptyCol;    // SW: same at the right
    bool bCantSplit;
    bool bCantSplit90;
    WW8_TCell* pTCs;
    sal_uInt8 nOverrideSpacing[MAX_COL + 1];
    short nOverrideValues[MAX_COL + 1][4];
    WW8_SHD* pSHDs;
    Color* pNewSHDs;
    WW8_BRCVer9 aDefBrcs[6];

    bool bExist[MAX_COL];           // does this cell exist ?
    sal_uInt8 nTransCell[MAX_COL + 2];  // translation WW-Index -> SW-Index

    sal_uInt8 transCell(sal_uInt8 nWwCol) const
    {
        return nWwCol < SAL_N_ELEMENTS(nTransCell) ? nTransCell[nWwCol] : 0xFF;
    }

    WW8TabBandDesc();
    WW8TabBandDesc(WW8TabBandDesc const & rBand);    // deep copy
    ~WW8TabBandDesc();
    void ReadDef(bool bVer67, const sal_uInt8* pS, short nLen);
    void ProcessDirection(const sal_uInt8* pParams);
    void ProcessSprmTSetBRC(int nBrcVer, const sal_uInt8* pParamsTSetBRC, sal_uInt16 nParamsLen);
    void ProcessSprmTTableBorders(int nBrcVer, const sal_uInt8* pParams, sal_uInt16 nParamsLen);
    void ProcessSprmTDxaCol(const sal_uInt8* pParamsTDxaCol);
    void ProcessSprmTDelete(const sal_uInt8* pParamsTDelete);
    void ProcessSprmTInsert(const sal_uInt8* pParamsTInsert);
    void ProcessSpacing(const sal_uInt8* pParamsTInsert);
    void ProcessSpecificSpacing(const sal_uInt8* pParamsTInsert);
    void ReadShd(const sal_uInt8* pS );
    void ReadNewShd(const sal_uInt8* pS, bool bVer67, sal_uInt8 nStart);

    enum wwDIR {wwTOP = 0, wwLEFT = 1, wwBOTTOM = 2, wwRIGHT = 3};

private:
    WW8TabBandDesc & operator =(WW8TabBandDesc const &) = default; // only for use in copy ctor
};

//            Storage-Reader

typedef std::set<WW8_CP> cp_set;

class SwWW8ImplReader
{
private:
    SwDocShell *m_pDocShell;         // The Real DocShell

friend class WW8RStyle;
friend class WW8TabDesc;
friend class WW8ReaderSave;
friend struct WW8FlyPara;
friend struct WW8SwFlyPara;
friend class WW8FlySet;
friend class SwMSDffManager;
friend class SwWW8FltControlStack;
friend class WW8FormulaControl;
friend class wwSectionManager;

private:

    SotStorage* m_pStg;                // Input-Storage
    SvStream* m_pStrm;                // Input-(Storage)Stream
    SvStream* m_pTableStream;         // Input-(Storage)Stream
    SvStream* m_pDataStream;          // Input-(Storage)Stream

// general stuff
    SwDoc& m_rDoc;
    std::shared_ptr<SwUnoCursor> mpCursor;
    SwPaM* m_pPaM;

    std::unique_ptr<SwWW8FltControlStack> m_xCtrlStck;    // stack for the attributes

    /*
    This stack is for redlines, because their sequence of discovery can
    be out of order of their order of insertion into the document.
    */
    std::unique_ptr<sw::util::RedlineStack> m_xRedlineStack;

    /*
    This stack is for fields that get referenced later, e.g. BookMarks and TOX.
    They get inserted at the end of the document, it is the same stack for
    headers/footers/main text/textboxes/tables etc...
    */
    std::unique_ptr<SwWW8ReferencedFltEndStack> m_xReffedStck;

    /*
    This stack is for fields whose true conversion cannot be determined until
    the end of the document, it is the same stack for headers/footers/main
    text/textboxes/tables etc... They are things that reference other things
    e.g. NoteRef and Ref, they are processed after m_xReffedStck
    */
    std::unique_ptr<SwWW8FltRefStack> m_xReffingStck;

    /*
    For graphics anchors. Determines the graphics whose anchors are in the
    current paragraph, and works around the difficulty in inserting a graphic
    anchored to character before a character to be anchored to has been
    inserted. Is emptied at the end of each paragraph.
    */
    std::unique_ptr<SwWW8FltAnchorStack> m_xAnchorStck;

    /*
    A stack of fields identifiers to keep track of any open fields that need
    to be closed. Generally word fields are inserted as writer fields as soon
    as they are encountered, and so their end point is normally unimportant.
    But hyperlink fields need to be applied as attributes to text and it is
    far easier and safer to set the end point of an attribute when we
    encounter the end marker of the field instead of calculating in advance
    where the end point will fall, to do so fully correctly duplicates the
    main logic of the filter itself.
    */
    std::deque<WW8FieldEntry> m_aFieldStack;

    /*
    A stack of open footnotes. Should only be one in it at any time.
    */
    std::deque<FootnoteDescriptor> m_aFootnoteStack;

    /*
    A queue of the ms sections in the document
    */
    wwSectionManager m_aSectionManager;

    /*
    A vector of surplus-to-requirements paragraph in the final document,
    that exist because of quirks of the SwDoc document model and/or API,
    which need to be removed.
    */
    wwExtraneousParas m_aExtraneousParas;

    /*
    A map of tables to their follow nodes for use in inserting tables into
    already existing document, i.e. insert file
    */
    sw::util::InsertedTablesManager m_aInsertedTables;

    /*
    Creates unique names to give to (file link) sections (WW1/WW2/...)
    */
    wwSectionNamer m_aSectionNameGenerator;

    /*
    Knows how to split a series of bytes into sprms and their arguments
    */
    std::unique_ptr<wwSprmParser> m_xSprmParser;

    /*
    Creates unique names to give to graphics
    */
    wwFrameNamer m_aGrfNameGenerator;

    /*
    Knows which writer style a given word style should be imported as.
    */
    sw::util::ParaStyleMapper m_aParaStyleMapper;
    sw::util::CharStyleMapper m_aCharStyleMapper;

    /*
     Stack of textencoding being used as we progress through the document text
    */
    std::stack<rtl_TextEncoding> m_aFontSrcCharSets;
    std::stack<rtl_TextEncoding> m_aFontSrcCJKCharSets;

    /*
     Progress bar
     */
    std::unique_ptr<ImportProgress> m_xProgress;

    std::unique_ptr<SwMSConvertControls> m_xFormImpl; // implementation of control

    SwFlyFrameFormat* m_pFlyFormatOfJustInsertedGraphic;
    std::unique_ptr<FrameDeleteWatch> m_xFormatOfJustInsertedApo;
    SwPaM* m_pPreviousNumPaM;
    const SwNumRule* m_pPrevNumRule;

    //Keep track of APO environments
    std::deque<bool> m_aApos;
    /*
    Keep track of generated Ruby character formats we can minimize the
    number of character formats created
    */
    std::vector<const SwCharFormat*> m_aRubyCharFormats;

    /*
    For fuzzing keep track of source offset of inserted graphics
    */
    std::set<sal_uLong> m_aGrafPosSet;

    std::unique_ptr<WW8PostProcessAttrsInfo> m_pPostProcessAttrsInfo;

    std::shared_ptr<WW8Fib> m_xWwFib;
    std::unique_ptr<WW8Fonts> m_xFonts;
    std::unique_ptr<WW8Dop> m_xWDop;
    std::unique_ptr<WW8ListManager> m_xLstManager;
    std::unique_ptr<WW8ScannerBase> m_xSBase;
    std::shared_ptr<WW8PLCFMan> m_xPlcxMan;
    std::map<short, OUString> m_aLinkStringMap;

    o3tl::sorted_vector<const SwNode*> m_aTextNodesHavingFirstLineOfstSet; // #i103711#
    o3tl::sorted_vector<const SwNode*> m_aTextNodesHavingLeftIndentSet; // #i105414#

    std::unique_ptr<WW8RStyle> m_xStyles;     // pointer to the style reading class
    SwFormat* m_pCurrentColl;        // collection to be created now
                            // ( always 0 outside of a Style-Def )
    std::unique_ptr<SfxItemSet> m_xCurrentItemSet;// character attributes to be read in now
                            // (always 0 outside of the WW8ListManager Ctor)
    std::vector<SwWW8StyInf> m_vColl;
    const SwTextFormatColl* m_pDfltTextFormatColl;    // Default
    SwFormat* m_pStandardFormatColl;// "Standard"

    std::unique_ptr<WW8PLCF_HdFt> m_xHdFt;        // pointer to Header / Footer - scanner class

    std::unique_ptr<WW8FlyPara> m_xWFlyPara;      // WW-parameter
    std::unique_ptr<WW8SwFlyPara> m_xSFlyPara;    // Sw parameters created from previous

    std::unique_ptr<WW8TabDesc> m_xTableDesc;     // description of table properties
    //Keep track of tables within tables
    std::stack<std::unique_ptr<WW8TabDesc>> m_aTableStack;

    ANLDRuleMap m_aANLDRules;
    std::unique_ptr<WW8_OLST> m_xNumOlst;         // position in text

    SdrModel* m_pDrawModel;
    SdrPage* m_pDrawPg;
    std::unique_ptr<EditEngine> m_pDrawEditEngine;
    std::unique_ptr<wwZOrderer> m_xWWZOrder;

    SwFieldType* m_pNumFieldType;   // for number circle

    std::unique_ptr<SwMSDffManager> m_xMSDffManager;

    std::optional<std::vector<OUString>> m_xAtnNames;

    std::unique_ptr<WW8SmartTagData> m_pSmartTagData;

    /// Redlining Authors, map word author key to writer author value
    std::map<sal_uInt16, std::size_t> m_aAuthorInfos;
    OUString m_sBaseURL;

                                // Ini-Flags:
    sal_uInt32 m_nIniFlags;            // flags from writer.ini
    sal_uInt32 m_nIniFlags1;           // ditto ( additional flags )
    sal_uInt32 m_nFieldFlags;          // ditto for fields
    sal_uInt32 m_nFieldTagAlways[3];   // ditto for tagging of fields
    sal_uInt32 m_nFieldTagBad[3];      // ditto for tagging of fields that can't be imported
    bool m_bRegardHindiDigits;  // import digits in CTL scripts as Hindi numbers

    bool m_bDrawCpOValid;
    WW8_CP m_nDrawCpO;            // start of Txbx-SubDocs

    sal_uLong m_nPicLocFc;            // Picture Location in File (FC)
    sal_uLong m_nObjLocFc;            // Object Location in File (FC)

    sal_Int32 m_nIniFlyDx;            // X-offset of Flys
    sal_Int32 m_nIniFlyDy;            // Y-offset of Flys

    rtl_TextEncoding m_eTextCharSet;    // Default charset for Text
    rtl_TextEncoding m_eStructCharSet;  // rtl_TextEncoding for structures
    rtl_TextEncoding m_eHardCharSet;    // Hard rtl_TextEncoding-Attribute
    sal_uInt16 m_nProgress;           // percentage for Progressbar
    sal_uInt16 m_nCurrentColl;          // per WW-count
    sal_uInt16 m_nFieldNum;             // serial number for that
    sal_uInt16 m_nLFOPosition;

    short m_nCharFormat;             // per WW-count, <0 for none

    short m_nDrawXOfs, m_nDrawYOfs;
    short m_nDrawXOfs2, m_nDrawYOfs2;

    sal_Unicode m_cSymbol;        // symbol to be read now

    sal_uInt8 m_nWantedVersion;        // originally requested WW-Doc version by Writer

    sal_uInt8 m_nSwNumLevel;           // level number for outline / enumeration
    sal_uInt8 m_nWwNumType;            // outline / number / enumeration
    const SwNumRule* m_pChosenWW8OutlineStyle; // copied to the "Outline" Chapter Numbering style
    sal_uInt8 m_nListLevel;

    bool m_bNewDoc;          // new document?
    bool m_bSkipImages;      // skip images for text extraction/indexing
    bool m_bReadNoTable;        // no tables
    bool m_bPgSecBreak;       // Page- or Sectionbreak is still to be added
    bool m_bSpec;             // special char follows in text
    bool m_bObj;              // Obj in Text
    bool m_bTxbxFlySection;   // FlyFrame that was inserted as replacement for Winword Textbox
    bool m_bHasBorder;        // for bundling of the borders
    bool m_bSymbol;           // e.g. Symbol instead of Times
    bool m_bIgnoreText;       // e.g. for FieldVanish
    int  m_nInTable;          // are we currently reading a table?
    bool m_bWasTabRowEnd;     // table : Row End Mark
    bool m_bWasTabCellEnd;    // table: Cell End Mark

    bool m_bAnl;              // enumeration in work
                                // Anl stands for "Autonumber level"

    bool m_bHdFtFootnoteEdn;       // special text: header/footer/and so on
    bool m_bFootnoteEdn;           // footnote or endnote
    bool m_bIsHeader;         // text is read from header ( line height )
    bool m_bIsFooter;         // text is read from footer ( line height )

    bool m_bIsUnicode;            // current piece of text is encoded as 2-byte Unicode
                                // please do NOT handle this as bit field!

    bool m_bCpxStyle;         // style in the complex part
    bool m_bStyNormal;        // style with Id 0 is currently read
    bool m_bWWBugNormal;      // WW-Version with Bug Dya in Style Normal
    bool m_bNoAttrImport;     // ignore attributes for ignoring styles
    bool m_bInHyperlink;      // special case for reading 0x01
                                   // see also: SwWW8ImplReader::Read_F_Hyperlink()
    bool m_bWasParaEnd;

    // useful helper variables
    bool m_bVer67;            // ( (6 == nVersion) || (7 == nVersion) );
    bool m_bVer6;             //   (6 == nVersion);
    bool m_bVer7;             //   (7 == nVersion);
    bool m_bVer8;             //   (8 == nVersion);

    bool m_bEmbeddObj;        // EmbeddField is being read

    bool m_bCurrentAND_fNumberAcross; // current active Annotated List Descriptor - ROW flag

    bool m_bNoLnNumYet;       // no Line Numbering has been activated yet (we import
                            //     the very 1st Line Numbering and ignore the rest)

    bool m_bFirstPara;        // first paragraph?
    bool m_bFirstParaOfPage;
    bool m_bParaAutoBefore;
    bool m_bParaAutoAfter;

    bool m_bDropCap;
    sal_Int32 m_nDropCap;

    bool m_bBidi;
    bool m_bReadTable;
    // Indicate that currently on loading a TOC, managed by Read_F_TOX() and End_Field()
    bool m_bLoadingTOXCache;
    int m_nEmbeddedTOXLevel;
    // Indicate that current on loading a hyperlink, which is inside a TOC; Managed by Read_F_Hyperlink() and End_Field()
    bool m_bLoadingTOXHyperlink;
    // a document position recorded the after-position of TOC section, managed by Read_F_TOX() and End_Field()
    std::unique_ptr<SwPaM> m_pPosAfterTOC;
    // used for some dropcap tweaking
    SwTextNode* m_pPreviousNode;

    std::unique_ptr< SwPosition > m_pLastAnchorPos;

    bool m_bCareFirstParaEndInToc;
    bool m_bCareLastParaEndInToc;
    cp_set m_aTOXEndCps;

    bool m_bNotifyMacroEventRead;
    bool m_bFuzzing;

    const SprmReadInfo& GetSprmReadInfo(sal_uInt16 nId) const;

    bool StyleExists(unsigned int nColl) const { return (nColl < m_vColl.size()); }
    SwWW8StyInf *GetStyle(sal_uInt16 nColl) const;
    void AppendTextNode(SwPosition& rPos);

    void Read_HdFt(int nSect, const SwPageDesc *pPrev,
        const wwSection &rSection);
    void Read_HdFtText(WW8_CP nStartCp, WW8_CP nLen, SwFrameFormat const * pHdFtFormat);
    void Read_HdFtTextAsHackedFrame(WW8_CP nStart, WW8_CP nLen,
        SwFrameFormat const &rHdFtFormat, sal_uInt16 nPageWidth);

    bool isValid_HdFt_CP(WW8_CP nHeaderCP) const;

    bool HasOwnHeaderFooter(sal_uInt8 nWhichItems, sal_uInt8 grpfIhdt, int nSect);

    void HandleLineNumbering(const wwSection &rSection);

    void CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                           SwPageDesc* pNewPageDesc, sal_uInt8 nCode );

    void DeleteStack(std::unique_ptr<SwFltControlStack> prStck);
    void DeleteCtrlStack()
    {
        DeleteStack(std::move(m_xCtrlStck));
    }
    void DeleteRefStacks()
    {
        DeleteStack(std::move(m_xReffedStck));
        DeleteStack(std::move(m_xReffingStck));
    }
    void DeleteAnchorStack()
    {
        DeleteStack(std::move(m_xAnchorStck));
    }
    void emulateMSWordAddTextToParagraph(const OUString& rAddString);
    void simpleAddTextToParagraph(const OUString& rAddString);
    bool HandlePageBreakChar();
    bool ReadChar(tools::Long nPosCp, tools::Long nCpOfs);
    bool ReadPlainChars(WW8_CP& rPos, sal_Int32 nEnd, sal_Int32 nCpOfs);
    bool ReadChars(WW8_CP& rPos, WW8_CP nNextAttr, tools::Long nTextEnd, tools::Long nCpOfs);
    static bool LangUsesHindiNumbers(LanguageType nLang);
    static sal_Unicode TranslateToHindiNumbers(sal_Unicode);

    void SetDocumentGrid(SwFrameFormat &rFormat, const wwSection &rSection);

    void ProcessCurrentCollChange(WW8PLCFManResult& rRes, bool* pStartAttr,
        bool bCallProcessSpecial);
    tools::Long ReadTextAttr(WW8_CP& rTextPos, tools::Long nTextEnd, bool& rbStartLine, int nDepthGuard = 0);
    void ReadAttrs(WW8_CP& rTextPos, WW8_CP& rNext, tools::Long nTextEnd, bool& rbStartLine);
    void CloseAttrEnds();
    bool ReadText(WW8_CP nStartCp, WW8_CP nTextLen, ManTypes nType);

    void ReadRevMarkAuthorStrTabl( SvStream& rStrm, sal_Int32 nTablePos,
        sal_Int32 nTableSiz, SwDoc& rDoc );

    void Read_HdFtFootnoteText(const SwNodeIndex* pSttIdx, WW8_CP nStartCp,
                               WW8_CP nLen, ManTypes nType);

    void ImportTox( int nFieldId, const OUString& aStr );

    void EndSprm( sal_uInt16 nId );
    // #i103711#
    // #i105414#
    void NewAttr( const SfxPoolItem& rAttr,
                  const bool bFirstLineOfStSet = false,
                  const bool bLeftIndentSet = false );

    bool GetFontParams(sal_uInt16, FontFamily&, OUString&, FontPitch&,
        rtl_TextEncoding&);
    bool SetNewFontAttr(sal_uInt16 nFCode, bool bSetEnums, sal_uInt16 nWhich);
    void ResetCharSetVars();
    void ResetCJKCharSetVars();

    const SfxPoolItem* GetFormatAttr( sal_uInt16 nWhich );
    bool JoinNode(SwPaM &rPam, bool bStealAttr = false);

    static bool IsBorder(const WW8_BRCVer9* pbrc, bool bChkBtwn = false);

    //Set closest writer border equivalent into rBox from pbrc, optionally
    //recording true winword dimensions in pSizeArray. nSetBorders to mark a
    //border which has been previously set to a value and for which becoming
    //empty is valid. Set bCheBtwn to work with paragraphs that have a special
    //between paragraphs border

    // Note #i20672# we can't properly support between lines so best to ignore
    // them for now
    static bool SetBorder(SvxBoxItem& rBox, const WW8_BRCVer9* pbrc,
        short *pSizeArray=nullptr, sal_uInt8 nSetBorders=0xFF);
    static void GetBorderDistance(const WW8_BRCVer9* pbrc, tools::Rectangle& rInnerDist);
    static sal_uInt16 GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing);
    static bool SetShadow(SvxShadowItem& rShadow, const short *pSizeArray,
        const WW8_BRCVer9& aRightBrc);
    //returns true is a shadow was set
    static bool SetFlyBordersShadow(SfxItemSet& rFlySet, const WW8_BRCVer9 *pbrc,
        short *SizeArray);
    static void SetPageBorder(SwFrameFormat &rFormat, const wwSection &rSection);

    static sal_Int32 MatchSdrBoxIntoFlyBoxItem( const Color& rLineColor,
        MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType, sal_Int32 &rLineWidth,
        SvxBoxItem& rBox );
    void MatchSdrItemsIntoFlySet( SdrObject const *    pSdrObj, SfxItemSet &aFlySet,
        MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType, tools::Rectangle &rInnerDist );
    static void AdjustLRWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxLRSpaceItem &rLR);
    static void AdjustULWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxULSpaceItem &rUL);
    static void MapWrapIntoFlyFormat(const SvxMSDffImportRec& rRecord, SwFrameFormat& rFlyFormat);

    void SetAttributesAtGrfNode(const SvxMSDffImportRec& rRecord, const SwFrameFormat& rFlyFormat,
                                WW8_FSPA const* pF);

    bool IsDropCap() const;
    bool IsListOrDropcap() const { return (!m_xCurrentItemSet  || m_bDropCap); };

    //Apo == Absolutely Positioned Object, MSWord's old-style frames
    std::unique_ptr<WW8FlyPara> ConstructApo(const ApoTestResults &rApo,
        const WW8_TablePos *pTabPos);
    bool StartApo(const ApoTestResults &rApo, const WW8_TablePos *pTabPos);
    void StopApo();
    bool TestSameApo(const ApoTestResults &rApo, const WW8_TablePos *pTabPos);
    ApoTestResults TestApo(int nCellLevel, bool bTableRowEnd,
        const WW8_TablePos *pTabPos);
    static void StripNegativeAfterIndent(SwFrameFormat const *pFlyFormat);

    void EndSpecial();
    bool ProcessSpecial(bool &rbReSync, WW8_CP nStartCp);
    sal_uInt16 TabRowSprm(int nLevel) const;

    bool ReadGrafFile(OUString& rFileName, std::unique_ptr<Graphic>& rpGraphic,
       const WW8_PIC& rPic, SvStream* pSt, sal_uLong nFilePos, bool* pDelIt);

    static void ReplaceObj(const SdrObject &rReplaceTextObj,
        SdrObject &rSubObj);

    SwFlyFrameFormat* MakeGrafNotInContent(const WW8PicDesc& rPD,
        const Graphic* pGraph, const OUString& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrameFormat* MakeGrafInContent(const WW8_PIC& rPic, const WW8PicDesc& rPD,
        const Graphic* pGraph, const OUString& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrameFormat *AddAutoAnchor(SwFrameFormat *pFormat);
    SwFrameFormat* ImportGraf1(WW8_PIC const & rPic, SvStream* pSt, sal_uLong nFilePos);
    SwFrameFormat* ImportGraf(SdrTextObj const * pTextObj = nullptr, SwFrameFormat const * pFlyFormat = nullptr);

    SdrObject* ImportOleBase( Graphic& rGraph, const Graphic* pGrf=nullptr,
        const SfxItemSet* pFlySet=nullptr, const tools::Rectangle& aVisArea = tools::Rectangle() );

    SwFrameFormat* ImportOle( const Graphic* = nullptr, const SfxItemSet* pFlySet = nullptr,
        const SfxItemSet* pGrfSet = nullptr, const tools::Rectangle& aVisArea = tools::Rectangle() );
    SwFlyFrameFormat* InsertOle(SdrOle2Obj &rObject, const SfxItemSet &rFlySet,
        const SfxItemSet *rGrfSet);

    bool ImportFormulaControl(WW8FormulaControl &rBox,WW8_CP nStart,
        SwWw8ControlType nWhich);

    void ImportDop();

    //This converts MS Asian Typography information into OOo's
    void ImportDopTypography(const WW8DopTypography &rTypo);

    ErrCode LoadThroughDecryption(WW8Glossary *pGloss);
    ErrCode SetSubStreams(tools::SvRef<SotStorageStream> &rTableStream, tools::SvRef<SotStorageStream> &rDataStream);
    ErrCode CoreLoad(WW8Glossary const *pGloss);

    void ReadDocVars();

    bool StartTable(WW8_CP nStartCp);
    bool InEqualApo(int nLvl) const;
    bool InLocalApo() const { return InEqualApo(m_nInTable); }
    bool InEqualOrHigherApo(int nLvl) const;
    void TabCellEnd();
    void StopTable();
    bool IsInvalidOrToBeMergedTabCell() const;

// Enumerations / lists ( Autonumbered List Data Descriptor )
// list:        ANLD ( Autonumbered List Data Descriptor )
//   one level: ANLV ( Autonumber Level Descriptor )

// Chg7-8:
// lists are separate structures in WW8 that are handled via the following three tables:
// rglst, hpllfo and hsttbListNames
// the corresponding structures are: LSTF, LVLF, LFO LFOLVL

    void SetAnlvStrings(SwNumFormat &rNum, int nLevel, WW8_ANLV const &rAV, const sal_uInt8* pText,
        size_t nStart, size_t nElements, bool bOutline);
    void SetAnld(SwNumRule* pNumR, WW8_ANLD const * pAD, sal_uInt8 nSwLevel, bool bOutLine);
    void SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, sal_uInt8 nSwLevel );
    SwNumRule* GetStyRule();

    void StartAnl(const sal_uInt8* pSprm13);
    void NextAnlLine(const sal_uInt8* pSprm13);
    void StopAllAnl(bool bGoBack = true);
    void StopAnlToRestart(sal_uInt8 nType, bool bGoBack = true);

// graphics layer

    bool ReadGrafStart(void* pData, short nDataSiz, WW8_DPHEAD const * pHd,
        SfxAllItemSet &rSet);
    SdrObject *ReadLine(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadRect(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadEllipse(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadArc(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadPolyLine(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    void InsertTxbxStyAttrs( SfxItemSet& rS, sal_uInt16 nColl );
    void InsertAttrsAsDrawingAttrs(WW8_CP nStartCp, WW8_CP nEndCp, ManTypes eType, bool bONLYnPicLocFc=false);

    bool GetTxbxTextSttEndCp(WW8_CP& rStartCp, WW8_CP& rEndCp, sal_uInt16 nTxBxS,
        sal_uInt16 nSequence);
    sal_Int32 GetRangeAsDrawingString(OUString& rString, tools::Long StartCp, tools::Long nEndCp, ManTypes eType);
    std::optional<OutlinerParaObject> ImportAsOutliner(OUString &rString, WW8_CP nStartCp, WW8_CP nEndCp, ManTypes eType);
    void InsertTxbxText(SdrTextObj* pTextObj, Size const * pObjSiz,
        sal_uInt16 nTxBxS, sal_uInt16 nSequence, tools::Long nPosCp, SwFrameFormat const * pFlyFormat,
        bool bMakeSdrGrafObj, bool& rbEraseTextObj,
        bool* pbTestTxbxContainsText = nullptr, tools::Long* pnStartCp = nullptr,
        tools::Long* pnEndCp = nullptr, bool* pbContainsGraphics = nullptr,
        SvxMSDffImportRec const * pRecord = nullptr);
    bool TxbxChainContainsRealText( sal_uInt16 nTxBxS,
                                    tools::Long&  rStartCp,
                                    tools::Long&  rEndCp );
    SdrObject *ReadTextBox(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadCaptionBox(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadGroup(WW8_DPHEAD const * pHd, SfxAllItemSet &rSet);
    SdrObject *ReadGrafPrimitive(short& rLeft, SfxAllItemSet &rSet);
    void ReadGrafLayer1(WW8PLCFspecial& rPF, tools::Long nGrafAnchorCp);
    SdrObject* CreateContactObject(SwFrameFormat* pFlyFormat);
    RndStdIds ProcessEscherAlign(SvxMSDffImportRec& rRecord, WW8_FSPA& rFSPA, SfxItemSet& rFlySet);
    bool MiserableRTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
        sal_Int16 eHoriOri, sal_Int16 eHoriRel);
    SwFrameFormat* Read_GrafLayer( tools::Long nGrafAnchorCp );
    SwFlyFrameFormat* ImportReplaceableDrawables(SdrObject*& rpObject, SdrObject*& rpOurNewObject,
                                                 SvxMSDffImportRec& rRecord, WW8_FSPA& rF,
                                                 SfxItemSet& rFlySet);
    SwFlyFrameFormat* ConvertDrawTextToFly(SdrObject*& rpObject, SdrObject*& rpOurNewObject,
                                           const SvxMSDffImportRec& rRecord, RndStdIds eAnchor,
                                           const WW8_FSPA& rF, SfxItemSet& rFlySet);
    SwFrameFormat* MungeTextIntoDrawBox(SvxMSDffImportRec& rRecord, tools::Long nGrafAnchorCp,
                                        SwFrameFormat* pRetFrameFormat);

    void GraphicCtor();
    void GraphicDtor();

// other stuff
    OUString GetFieldResult( WW8FieldDesc const * pF );
    void MakeTagString( OUString& rStr, const OUString& rOrg );
    void UpdateFields();
    OUString ConvertFFileName(const OUString& rRaw);
    WW8_CP Read_F_Tag(WW8FieldDesc* pF);
    void InsertTagField( const sal_uInt16 nId, const OUString& rTagText );
    tools::Long ImportExtSprm(WW8PLCFManResult* pRes);
    void EndExtSprm(sal_uInt16 nSprmId);
    void ReadDocInfo();

// Ver8 lists

    void RegisterNumFormatOnTextNode(sal_uInt16 nCurrentLFO, sal_uInt8 nCurrentLevel,
                                 const bool bSetAttr = true);

    void RegisterNumFormatOnStyle(sal_uInt16 nStyle);
    void SetStylesList(sal_uInt16 nStyle, sal_uInt16 nCurrentLFO,
        sal_uInt8 nCurrentLevel);
    void RegisterNumFormat(sal_uInt16 nCurrentLFO, sal_uInt8 nCurrentLevel);

// to be replaced by calls in the appropriate extended SvxMSDffManager

    const OUString* GetAnnotationAuthor(sal_uInt16 nIdx);

    void GetSmartTagInfo(SwFltRDFMark& rMark);

    // interfaces for the toggle attributes
    void SetToggleAttr(sal_uInt8 nAttrId, bool bOn);
    void SetToggleBiDiAttr(sal_uInt8 nAttrId, bool bOn);
    void ChkToggleAttr_( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask );

    void ChkToggleAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            m_xCtrlStck->GetToggleAttrFlags() )
            ChkToggleAttr_( nOldStyle81Mask, nNewStyle81Mask );
    }

    void ChkToggleBiDiAttr_( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask );

    void ChkToggleBiDiAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            m_xCtrlStck->GetToggleBiDiAttrFlags() )
            ChkToggleBiDiAttr_( nOldStyle81Mask, nNewStyle81Mask );
    }

    void PopTableDesc();
    void MoveInsideFly(const SwFrameFormat *pFlyFormat);
    SwTwips MoveOutsideFly(SwFrameFormat *pFlyFormat, const SwPosition &rPos,
        bool bTableJoin = true);

    void SetOutlineStyles();

    bool SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper);
    bool SetUpperSpacing(SwPaM &pMyPam, int nSpace);
    bool SetLowerSpacing(SwPaM &rMyPam, int nSpace);

    bool IsInlineEscherHack() const
        { return !m_aFieldStack.empty() && m_aFieldStack.back().mnFieldId == 95; };

    void StoreMacroCmds();

    // #i84783#
    // determine object attribute "Layout in Table Cell"
    bool IsObjectLayoutInTableCell( const sal_uInt32 nLayoutInTableCell ) const;
    void ReadGlobalTemplateSettings( std::u16string_view sCreatedFrom, const css::uno::Reference< css::container::XNameContainer >& xPrjNameMap );
    SwWW8ImplReader(const SwWW8ImplReader &) = delete;
    SwWW8ImplReader& operator=(const SwWW8ImplReader&) = delete;
public:     // really private, but can only be done public
    ~SwWW8ImplReader();
    sal_uInt16 GetToggleAttrFlags() const;
    sal_uInt16 GetToggleBiDiAttrFlags() const;
    void SetToggleAttrFlags(sal_uInt16 nFlags);
    void SetToggleBiDiAttrFlags(sal_uInt16 nFlags);

    tools::Long Read_Footnote(WW8PLCFManResult* pRes);
    sal_uInt16 End_Footnote();
    tools::Long Read_Field(WW8PLCFManResult* pRes);
    sal_uInt16 End_Field();
    tools::Long Read_Book(WW8PLCFManResult*);
    tools::Long Read_And(WW8PLCFManResult* pRes);
    tools::Long Read_AtnBook(WW8PLCFManResult*);
    tools::Long Read_FactoidBook(WW8PLCFManResult*);

    // attributes

    void Read_Special(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_Obj(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_PicLoc(sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_BoldUsw(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_Bidi(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_BoldBiDiUsw(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_AmbiguousSPRM(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_SubSuper(         sal_uInt16, const sal_uInt8*, short nLen );
    bool ConvertSubToGraphicPlacement();
    static SwFrameFormat *ContainsSingleInlineGraphic(const SwPaM &rRegion);
    void Read_SubSuperProp(     sal_uInt16, const sal_uInt8*, short nLen );
    void Read_Underline(        sal_uInt16, const sal_uInt8*, short nLen );
    void Read_TextColor(         sal_uInt16, const sal_uInt8*, short nLen );
    void openFont(sal_uInt16 nFCode, sal_uInt16 nId);
    void closeFont(sal_uInt16 nId);
    void Read_FontCode(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_FontSize(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_CharSet(sal_uInt16 , const sal_uInt8* pData, short nLen);
    void Read_Language(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_CColl(            sal_uInt16, const sal_uInt8*, short nLen );
    void Read_Kern(             sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_FontKern(         sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_Emphasis(         sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ScaleWidth(       sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_Relief(           sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_TextAnim(      sal_uInt16, const sal_uInt8* pData, short nLen);

    void Read_NoLineNumb(       sal_uInt16 nId, const sal_uInt8* pData, short nLen );

    void Read_LR(               sal_uInt16 nId, const sal_uInt8*, short nLen );
    void Read_UL(               sal_uInt16 nId, const sal_uInt8*, short nLen );
    void Read_ParaAutoBefore(sal_uInt16 , const sal_uInt8 *pData, short nLen);
    void Read_ParaAutoAfter(sal_uInt16 , const sal_uInt8 *pData, short nLen);
    void Read_ParaContextualSpacing( sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_LineSpace(        sal_uInt16, const sal_uInt8*, short nLen );

    void SetRelativeJustify( bool bRel );
    bool IsRelativeJustify();
    bool IsRelativeJustify(sal_uInt16 nColl, o3tl::sorted_vector<sal_uInt16>& rVisitedStyles);
    void Read_Justify(sal_uInt16, const sal_uInt8*, short nLen);

    void Read_IdctHint(sal_uInt16, const sal_uInt8*, short nLen);
    bool IsRightToLeft();
    void Read_RTLJustify(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_Hyphenation(      sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_WidowControl(     sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_AlignFont(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_UsePgsuSettings(  sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_KeepLines(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_KeepParas(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_BreakBefore(      sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ApoPPC(sal_uInt16, const sal_uInt8* pData, short);

    void Read_BoolItem(         sal_uInt16 nId, const sal_uInt8*, short nLen );

    void Read_Border(           sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_CharBorder(sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_Tab(              sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_Symbol(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_FieldVanish(        sal_uInt16 nId, const sal_uInt8* pData, short nLen );

    // Revision Marks ( == Redlining )

    // insert or delete content (change char attributes resp.)
    void Read_CRevisionMark(RedlineType eType, const sal_uInt8* pData, short nLen);
    // insert new content
    void Read_CFRMark(sal_uInt16 , const sal_uInt8* pData, short nLen);
    // delete old content
    void Read_CFRMarkDel(sal_uInt16 , const sal_uInt8* pData, short nLen);
    // change properties of content (e.g. char formatting)
    void Read_CPropRMark(sal_uInt16 , const sal_uInt8* pData, short nLen); // complex!

    void Read_TabRowEnd(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_TabCellEnd(        sal_uInt16, const sal_uInt8* pData, short nLen );
    bool ParseTabPos(WW8_TablePos *aTabPos, WW8PLCFx_Cp_FKP* pPap);
    void Read_Shade(            sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ANLevelNo(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ANLevelDesc(      sal_uInt16, const sal_uInt8* pData, short nLen );

    // outline level Ver8
    void Read_POutLvl(sal_uInt16, const sal_uInt8* pData, short nLen);

    void Read_OLST(             sal_uInt16, const sal_uInt8* pData, short nLen );

    void Read_CharShadow(       sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_CharHighlight(    sal_uInt16, const sal_uInt8* pData, short nLen );
                                        // Ver8-Listen

    void Read_ListLevel(        sal_uInt16 nId, const sal_uInt8* pData, short nLen);

    /**
     * read and interpret the sprmPIlfo used to determine which list
     * contains the paragraph.
     * @param     nId   unused (sprm value, 0x460b for sprmPIlfo).
     * @param[in] pData operand.
     * @param[in] nLen  size of the operand (pData) in byte, should be 2.
     *                  -1 to indicate the actual level is finished.
     */
    void Read_LFOPosition(      sal_uInt16 nId, const sal_uInt8* pData, short nLen);
    bool SetTextFormatCollAndListLevel(const SwPaM& rRg, SwWW8StyInf& rStyleInfo);

    void Read_StyleCode(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_Majority(sal_uInt16, const sal_uInt8* , short );
    void Read_DoubleLine_Rotate( sal_uInt16, const sal_uInt8* pDATA, short nLen);

    void Read_TextForeColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_TextBackColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_ParaBackColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_ParaBiDi(sal_uInt16, const sal_uInt8* pData, short nLen);
    static Color ExtractColour(const sal_uInt8* &rpData, bool bVer67);

    void Read_TextVerticalAdjustment(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_UnderlineColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    tools::Long MapBookmarkVariables(const WW8FieldDesc* pF, OUString &rOrigName,
        const OUString &rData);
    OUString GetMappedBookmark(std::u16string_view rOrigName);

    // fields
    eF_ResT Read_F_Input(WW8FieldDesc*, OUString& rStr);
    eF_ResT Read_F_InputVar(WW8FieldDesc*, OUString& rStr);
    eF_ResT Read_F_ANumber( WW8FieldDesc*, OUString& );
    eF_ResT Read_F_DocInfo( WW8FieldDesc* pF, OUString& rStr );
    eF_ResT Read_F_Author( WW8FieldDesc*, OUString& );
    eF_ResT Read_F_TemplName( WW8FieldDesc*, OUString& );
    SvNumFormatType GetTimeDatePara(OUString const & rStr, sal_uInt32& rFormat, LanguageType &rLang,
        int nWhichDefault, bool bHijri = false);
    bool ForceFieldLanguage(SwField &rField, LanguageType nLang);
    eF_ResT Read_F_DateTime( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_FileName( WW8FieldDesc*, OUString& rStr);
    eF_ResT Read_F_Num( WW8FieldDesc* pF, OUString& );
    eF_ResT Read_F_CurPage( WW8FieldDesc*, OUString& );
    eF_ResT Read_F_Ref( WW8FieldDesc* pF, OUString& );

    eF_ResT Read_F_Set( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_PgRef( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_NoteReference( WW8FieldDesc* pF, OUString& rStr );

    eF_ResT Read_F_Tox( WW8FieldDesc* pF, OUString& rStr );
    eF_ResT Read_F_Symbol( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_Embedd( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_FormTextBox( WW8FieldDesc* pF, OUString& rStr);
    eF_ResT Read_F_FormCheckBox( WW8FieldDesc* pF, OUString& rStr );
    eF_ResT Read_F_FormListBox( WW8FieldDesc* pF, OUString& rStr);
    css::awt::Size MiserableDropDownFormHack(const OUString &rString,
        css::uno::Reference<css::beans::XPropertySet> const & rPropSet);

    eF_ResT Read_F_Macro( WW8FieldDesc*, OUString& rStr);
    eF_ResT Read_F_DBField( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_DBNext( WW8FieldDesc*, OUString& );
    eF_ResT Read_F_DBNum( WW8FieldDesc*, OUString& );
    eF_ResT Read_F_Equation( WW8FieldDesc*, OUString& );
    void Read_SubF_Ruby( msfilter::util::WW8ReadFieldParams& rReadParam);
    eF_ResT Read_F_IncludePicture( WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_IncludeText(    WW8FieldDesc*, OUString& rStr );
    eF_ResT Read_F_Seq( WW8FieldDesc*, OUString& rStr );
    /// Reads a STYLEREF field.
    eF_ResT Read_F_Styleref(WW8FieldDesc*, OUString& rStr);

    eF_ResT Read_F_OCX(WW8FieldDesc*, OUString&);
    eF_ResT Read_F_Hyperlink(WW8FieldDesc*, OUString& rStr);
    eF_ResT Read_F_Shape(WW8FieldDesc* pF, OUString& rStr);
    eF_ResT Read_F_HTMLControl( WW8FieldDesc* pF, OUString& rStr);

    short ImportSprm(const sal_uInt8* pPos, sal_Int32 nMemLen, sal_uInt16 nId = 0);

    bool SearchRowEnd(WW8PLCFx_Cp_FKP* pPap,WW8_CP &rStartCp, int nLevel) const;
    /// Seek to the end of the table with pPap, returns true on success.
    bool SearchTableEnd(WW8PLCFx_Cp_FKP* pPap) const;
    bool FloatingTableConversion(WW8PLCFx_Cp_FKP* pPap);

    const WW8Fib& GetFib() const    { return *m_xWwFib; }
    SwDoc& GetDoc() const           { return m_rDoc; }
    sal_uInt16 GetCurrentColl()  const     { return m_nCurrentColl; }
    void SetNCurrentColl( sal_uInt16 nColl ) { m_nCurrentColl = nColl;    }
    std::unique_ptr<SfxItemSet> SetCurrentItemSet(std::unique_ptr<SfxItemSet> pItemSet);
    sal_uInt16 StyleUsingLFO( sal_uInt16 nLFOIndex ) const ;
    const SwFormat* GetStyleWithOrgWWName( std::u16string_view rName ) const ;

    static bool GetPictGrafFromStream(Graphic& rGraphic, SvStream& rSrc);
    SAL_WARN_UNUSED_RESULT static bool PicRead(SvStream *pDataStream, WW8_PIC *pPic, bool bVer67);
    static bool ImportOleWMF(const tools::SvRef<SotStorage>& xSrc1, GDIMetaFile& rWMF, tools::Long& rX,
                             tools::Long& rY);
    static Color GetCol(sal_uInt8 nIco);

    SwWW8ImplReader( sal_uInt8 nVersionPara, SotStorage* pStorage, SvStream* pSt,
        SwDoc& rD, const OUString& rBaseURL, bool bNewDoc, bool bSkipImages, SwPosition const &rPos );

    const OUString& GetBaseURL() const { return m_sBaseURL; }
    // load a complete doc file
    ErrCode LoadDoc(WW8Glossary *pGloss=nullptr);
    rtl_TextEncoding GetCurrentCharSet();
    rtl_TextEncoding GetCurrentCJKCharSet();
    rtl_TextEncoding GetCharSetFromLanguage();
    rtl_TextEncoding GetCJKCharSetFromLanguage();

    void PostProcessAttrs();
    void ReadEmbeddedData(SvStream& rStrm, SwDocShell const * pDocShell, struct HyperLinksTable& hlStr);
    void NotifyMacroEventRead();
};

bool CanUseRemoteLink(const OUString &rGrfName);
void UseListIndent(SwWW8StyInf &rStyle, const SwNumFormat &rFormat);
void SetStyleIndent(SwWW8StyInf &rStyleInfo, const SwNumFormat &rFormat);
// #i103711#
// #i105414#
void SyncIndentWithList( SvxLRSpaceItem &rLR,
                         const SwNumFormat &rFormat,
                         const bool bFirstLineOfStSet,
                         const bool bLeftIndentSet );
tools::Long GetListFirstLineIndent(const SwNumFormat &rFormat);
OUString BookmarkToWriter(std::u16string_view rBookmark);
bool RTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize);
void MatchEscherMirrorIntoFlySet(const SvxMSDffImportRec &rRecord,
    SfxItemSet &rFlySet);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
