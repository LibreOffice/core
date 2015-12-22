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

#ifndef INCLUDED_FILTER_MSFILTER_SVDFPPT_HXX
#define INCLUDED_FILTER_MSFILTER_SVDFPPT_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <svx/svdobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/editdata.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <editeng/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <editeng/flditem.hxx>
#undef ITEMID_FIELD
#include <filter/msfilter/msfilterdllapi.h>
#include <vcl/font.hxx>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

class SdrModel;
class SdPage;
class SdrPage;
class SdrObject;
class SvStream;
namespace tools {
    class Polygon;
    class PolyPolygon;
}
class SfxItemSet;
class Outliner;
class Graphic;
class SvxMSDffManager;
class PPTTextObj;
class DffRecordHeader;
class SvxBulletItem;
enum class PptSlideLayout;
enum class PptPlaceholder;

#define PPT_IMPORTFLAGS_NO_TEXT_ASSERT  1

struct MSFILTER_DLLPUBLIC PptCurrentUserAtom
{
    sal_uInt32  nMagic;
    sal_uInt32  nCurrentUserEdit;
    sal_uInt16  nDocFileVersion;
    sal_uInt8   nMajorVersion;
    sal_uInt8   nMinorVersion;
    OUString    aCurrentUser;

public:

    PptCurrentUserAtom() :  nMagic              ( 0 ),
                            nCurrentUserEdit    ( 0 ),
                            nDocFileVersion     ( 0 ),
                            nMajorVersion       ( 0 ),
                            nMinorVersion       ( 0 ) {}

    MSFILTER_DLLPUBLIC friend SvStream& ReadPptCurrentUserAtom( SvStream& rIn, PptCurrentUserAtom& rAtom );
};

struct MSFILTER_DLLPUBLIC PowerPointImportParam
{
    SvStream&           rDocStream;
    sal_uInt32          nImportFlags;
    PptCurrentUserAtom  aCurrentUserAtom;

    PowerPointImportParam( SvStream& rDocStream, sal_uInt32 nImportFlags );
};

struct SdHyperlinkEntry
{
    sal_uInt32  nIndex;
    sal_Int32   nPrivate1;
    sal_Int32   nPrivate2;
    sal_Int32   nPrivate3;
    sal_Int32   nInfo;
    OUString    aTarget;
    OUString    aSubAdress;

    sal_Int32   nStartPos;
    sal_Int32   nEndPos;

    OUString    aConvSubString;
};

// Helper class for reading the PPT InteractiveInfoAtom
struct MSFILTER_DLLPUBLIC PptInteractiveInfoAtom
{
    sal_uInt32          nSoundRef;
    sal_uInt32          nExHyperlinkId;
    sal_uInt8           nAction;
    sal_uInt8           nOleVerb;
    sal_uInt8           nJump;
    sal_uInt8           nFlags;
    sal_uInt8           nHyperlinkType;

    // unknown, because total size is 16
    sal_uInt8           nUnknown1;
    sal_uInt8           nUnknown2;
    sal_uInt8           nUnknown3;

public:

    MSFILTER_DLLPUBLIC friend SvStream& ReadPptInteractiveInfoAtom( SvStream& rIn, PptInteractiveInfoAtom& rAtom );
};

enum PptPageKind { PPT_MASTERPAGE, PPT_SLIDEPAGE, PPT_NOTEPAGE };

enum PptPageFormat
{   PPTPF_SCREEN,
    PPTPF_USLETTER, // 8.5x11"
    PPTPF_A4,       // 210x297mm
    PPTPF_35MMDIA,  // DIA
    PPTPF_OVERHEAD,
    PPTPF_CUSTOM
};

// values for PPT_PST_TextHeaderAtom's sal_uLong
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       // Text in a Shape
    PPTTH_CENTERBODY,  // Subtitle in Title-Slide
    PPTTH_CENTERTITLE, // Title in Title-Slide
    PPTTH_HALFBODY,    // Body in two-column slide
    PPTTH_QUARTERBODY  // Body in four-body slide
};

#define TSS_TYPE_PAGETITLE      (0)
#define TSS_TYPE_BODY           (1)
#define TSS_TYPE_NOTES          (2)
#define TSS_TYPE_UNUSED         (3)
#define TSS_TYPE_TEXT_IN_SHAPE  (4)
#define TSS_TYPE_SUBTITLE       (5)
#define TSS_TYPE_TITLE          (6)
#define TSS_TYPE_HALFBODY       (7)
#define TSS_TYPE_QUARTERBODY    (8)

// Inventor-Id for PPT UserData
const sal_uInt32 PPTInventor = sal_uInt32('P') * 0x00000001
                             + sal_uInt32('P') * 0x00000100
                             + sal_uInt32('T') * 0x00010000
                             + sal_uInt32('0') * 0x01000000;

const int nMaxPPTLevels = 5;

// Object IDs for StarDraw UserData
#define PPT_OBJECTINFO_ID       (1)

struct MSFILTER_DLLPUBLIC PptDocumentAtom
{
    Size            aSlidesPageSize;                    // page size of the slides in 576DPI
    Size            aNotesPageSize;                     // page size of the notes in 576DPI
    // avoid RatioAtom for the time being
    sal_uInt32      nNotesMasterPersist;                // 0=non-existent
    sal_uInt32      nHandoutMasterPersist;              // 0=non-existent
    sal_uInt16      n1stPageNumber;                     // page number of the first slide
    PptPageFormat   eSlidesPageFormat;                  // page format of the slides
    bool            bEmbeddedTrueType           : 1;    // TrueType direcly within the File?
    bool            bTitlePlaceholdersOmitted   : 1;
    bool            bRightToLeft                : 1;
    bool            bShowComments               : 1;

public:

    static Size GetPageSize( const Size& rSiz );
    Size        GetSlidesPageSize() const { return GetPageSize( aSlidesPageSize ); }
    Size        GetNotesPageSize() const { return GetPageSize( aNotesPageSize ); }

    friend SvStream& ReadPptDocumentAtom( SvStream& rIn, PptDocumentAtom& rAtom );
};

struct PptSlideLayoutAtom
{
    PptSlideLayout      eLayout;                // 0..18
    PptPlaceholder      aPlaceholderId[ 8 ];

public:
                        PptSlideLayoutAtom() { Clear(); }
    void                Clear();

    // SlideLayoutAtom is read without header!
    friend SvStream& ReadPptSlideLayoutAtom( SvStream& rIn, PptSlideLayoutAtom& rAtom );
};

struct PptSlideAtom
{
    PptSlideLayoutAtom  aLayout;
    sal_uInt32          nMasterId;
    sal_uInt32          nNotesId;
    sal_uInt16          nFlags;

public:
                        PptSlideAtom() { Clear(); }
    void                Clear();

    friend SvStream& ReadPptSlideAtom(SvStream& rIn, PptSlideAtom& rAtom);
};

struct PptSlidePersistAtom
{
    sal_uInt32          nPsrReference;
    sal_uInt32          nFlags;
    sal_uInt32          nNumberTexts;
    sal_uInt32          nSlideId;
    sal_uInt32          nReserved;                  // we will use nReserved temporarly to set the offset to SSSlideInfoAtom ( if possible )

public:
                        PptSlidePersistAtom() { Clear(); }
    void                Clear();

    friend SvStream& ReadPptSlidePersistAtom(SvStream& rIn, PptSlidePersistAtom& rAtom);
};

struct PptNotesAtom
{
    sal_uInt32          nSlideId;
    sal_uInt16          nFlags;

public:
                        PptNotesAtom() { Clear(); }
    void                Clear();

    friend SvStream& ReadPptNotesAtom(SvStream& rIn, PptNotesAtom& rAtom);
};

struct PptColorSchemeAtom
{
    sal_uInt8           aData[32];

public:
                        PptColorSchemeAtom () { Clear(); }
    void                Clear();
    Color               GetColor( sal_uInt16 nNum ) const;

    friend SvStream& ReadPptColorSchemeAtom(SvStream& rIn, PptColorSchemeAtom& rAtom);
};

struct PptFontEntityAtom
{
    OUString            aName;
    sal_uInt8           lfClipPrecision;
    sal_uInt8           lfQuality;

    rtl_TextEncoding    eCharSet;
    FontFamily          eFamily;
    FontPitch           ePitch;
    bool                bAvailable;

    friend SvStream& ReadPptFontEntityAtom(SvStream& rIn, PptFontEntityAtom& rAtom);
};

class PptFontCollection;
struct PptUserEditAtom
{
    DffRecordHeader     aHd;
    sal_Int32           nLastSlideID;            // ID of last visible slide
    sal_uInt32          nVersion;                // This is major/minor/build which did the edit
    sal_uInt32          nOffsetLastEdit;         // File offset of prev PptUserEditAtom
    sal_uInt32          nOffsetPersistDirectory; // Offset to PersistPtrs for this file version.
    sal_uInt32          nDocumentRef;
    sal_uInt32          nMaxPersistWritten;      // total number of Persist entries up to this point
    sal_Int16           eLastViewType;           // enum view type

public:
    PptUserEditAtom()
        : nLastSlideID( 0 )
        , nVersion( 0 )
        , nOffsetLastEdit( 0 )
        , nOffsetPersistDirectory( 0 )
        , nDocumentRef( 0 )
        , nMaxPersistWritten( 0 )
        , eLastViewType( 0 )
        {}

    friend SvStream& ReadPptUserEditAtom( SvStream& rIn, PptUserEditAtom& rAtom );
};

struct PptOEPlaceholderAtom
{
    sal_uInt32          nPlacementId;
    PptPlaceholder      nPlaceholderId;
    sal_uInt8           nPlaceholderSize; // 0=Full size, 1=Half size, 2=Quarter of Slide

public:
                        PptOEPlaceholderAtom() { Clear(); }
    void                Clear();

    friend SvStream& ReadPptOEPlaceholderAtom( SvStream& rIn, PptOEPlaceholderAtom& rAtom );
};

struct ProcessData;
struct PPTStyleSheet;
struct HeaderFooterEntry;
struct PptSlidePersistEntry
{
    PptSlidePersistEntry(const PptSlidePersistEntry&) = delete;
    PptSlidePersistEntry& operator=( const PptSlidePersistEntry& ) = delete;

    PptSlidePersistAtom aPersistAtom;
    PptSlideAtom        aSlideAtom;
    PptNotesAtom        aNotesAtom;
    PptColorSchemeAtom  aColorScheme;           // each slide includes this colorscheme atom
    PPTStyleSheet*      pStyleSheet;            // stylesheet of this page (only in masterpages), since XP supports more than one masterpage

    sal_uInt32          HeaderFooterOfs[ 4 ];   // containing the ofs to the placeholder (only masterpage)
    HeaderFooterEntry*  pHeaderFooterEntry;
    SvxMSDffSolverContainer* pSolverContainer;
    sal_uInt32          nSlidePersistStartOffset;// is an array to the end of the SlidePersistAtom of this page, TextHeaderAtom is following
    sal_uInt32          nSlidePersistEndOffset;
    sal_uInt32          nBackgroundOffset;      // fileoffset
    sal_uInt32          nDrawingDgId;           // valid, if not -1
    sal_uInt32*         pPresentationObjects;   // if valid, this is a pointer to an array that includes the offsets to the presentation objects
                                                // on this masterpage for each instance (0 - 8);
    SdrObject*          pBObj;
    bool                bBObjIsTemporary;

    PptPageKind         ePageKind;

    bool                bNotesMaster    : 1;    // for NotesMaster
    bool                bHandoutMaster  : 1;    // for HandoutMaster
    bool                bStarDrawFiller : 1;    // special for StarDraw
public:
                        PptSlidePersistEntry();
                        ~PptSlidePersistEntry();
    sal_uInt32          GetSlideId() const { return aPersistAtom.nSlideId; }
};

#define PPTSLIDEPERSIST_ENTRY_NOTFOUND 0xFFFF

class MSFILTER_DLLPUBLIC PptSlidePersistList
{
private:
    PptSlidePersistList(const PptSlidePersistList&) = delete;
    PptSlidePersistList& operator=( const PptSlidePersistList& ) = delete;

    typedef std::vector<std::unique_ptr<PptSlidePersistEntry>> Entries_t;
    Entries_t mvEntries;

public:
    PptSlidePersistList();
    ~PptSlidePersistList();

    size_t size() const { return mvEntries.size(); }
    bool is_null( size_t nIdx ) const { return mvEntries[ nIdx ] == nullptr; }
    const PptSlidePersistEntry& operator[](size_t nIdx) const { return *mvEntries[ nIdx ]; }
    PptSlidePersistEntry& operator[](size_t nIdx) { return *mvEntries[ nIdx ]; }
    Entries_t::iterator begin() { return mvEntries.begin(); }
    void insert( Entries_t::iterator it,
                 std::unique_ptr<PptSlidePersistEntry> pEntry )
    {
        mvEntries.insert(it, std::move(pEntry));
    }
    void push_back(std::unique_ptr<PptSlidePersistEntry> pEntry)
    {
        mvEntries.push_back(std::move(pEntry));
    }

    sal_uInt16          FindPage( sal_uInt32 nId ) const;
};

class  SfxObjectShell;
struct PPTOleEntry
{
    sal_uInt32          nId;                        // OleId
    sal_uInt32          nRecHdOfs;                  // points to the record header: ExObjListHd
    SfxObjectShell*     pShell;
    sal_uInt16          nType;                      // maybe PPT_PST_ExEmbed or PPT_PST_ExControl
    sal_uInt32          nAspect;                    // the aspect of the OLE object

    PPTOleEntry( sal_uInt32 nid, sal_uInt32 nOfs, SfxObjectShell* pSh, sal_uInt16 nT, sal_uInt32 nAsp )
        : nId(nid)
        , nRecHdOfs(nOfs)
        , pShell(pSh)
        , nType(nT)
        , nAspect(nAsp)
    {}
};

struct PptExOleObjAtom
{
    sal_uInt32  nAspect;
    sal_uInt32  nId;
    sal_uInt32  nPersistPtr;

public:

    friend SvStream& ReadPptExOleObjAtom( SvStream& rIn, PptExOleObjAtom& rAtom );
};

// SdPage derives from SdrPage, is only known inside sd, and needs to be carried
// around as an opaque pointer here:
struct SdPageCapsule {
    explicit SdPageCapsule(SdrPage * thePage): page(thePage) {}

    SdrPage * page;
};

class PPTExtParaProv;
class MSFILTER_DLLPUBLIC SdrEscherImport : public SvxMSDffManager
{
protected:

    friend class PPTTextObj;
    friend class PPTPortionObj;
    friend struct PPTStyleTextPropReader;
    friend class ImplSdPPTImport;

    PptDocumentAtom     aDocAtom;
    DffRecordManager    aDocRecManager;             // contains all first level container and atoms of the document container

    ::std::vector< PPTOleEntry* > aOleObjectList;

    PptFontCollection*  m_pFonts;

    sal_uInt32          nStreamLen;

    bool                 ReadString( OUString& rStr ) const;
    // only for PowerPoint filter:
    virtual const PptSlideLayoutAtom* GetSlideLayoutAtom() const;

public:
    using SvxMSDffManager::ReadObjText;

    PowerPointImportParam& rImportParam;

                        SdrEscherImport( PowerPointImportParam&, const OUString& rBaseURL );
    virtual             ~SdrEscherImport();
    virtual bool        GetColorFromPalette( sal_uInt16 nNum, Color& rColor ) const override;
    virtual bool        SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const override;
    PptFontEntityAtom*  GetFontEnityAtom( sal_uInt32 nNum ) const;
    void                RecolorGraphic( SvStream& rSt, sal_uInt32 nRecLen, Graphic& rGraph );
    virtual SdrObject*  ReadObjText( PPTTextObj* pTextObj, SdrObject* pObj, SdPageCapsule pPage ) const;
    virtual SdrObject*  ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj ) override;
    virtual void        ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* pData, DffObjData& rObj ) override;
    void                ImportHeaderFooterContainer( DffRecordHeader& rHeader, HeaderFooterEntry& rEntry );
};




class SvxFieldItem;
struct MSFILTER_DLLPUBLIC PPTFieldEntry
{
    sal_uInt16          nPos;
    sal_uInt16          nTextRangeEnd;
    SvxFieldItem*       pField1;
    SvxFieldItem*       pField2;
    OUString*           pString;

    PPTFieldEntry() : nPos( 0 ), nTextRangeEnd( 0 ), pField1( nullptr ), pField2( nullptr ), pString( nullptr ) {};
    ~PPTFieldEntry();

    void                SetDateTime( sal_uInt32 nType );

                        // converting PPT date time format:
    static void         GetDateTime(
                            const sal_uInt32 nVal,
                            SvxDateFormat& eDateFormat,
                            SvxTimeFormat& eTimeFormat
                        );
};

struct MSFILTER_DLLPUBLIC HeaderFooterEntry
{
    const PptSlidePersistEntry* pMasterPersist;
    OUString            pPlaceholder[ 4 ];
    sal_uInt32          nAtom;

    sal_uInt32          IsToDisplay( sal_uInt32 nInstance );
    sal_uInt32          NeedToImportInstance(
                            const sal_uInt32 nInstance,
                            const PptSlidePersistEntry& rSlidePersist
                        );

                        explicit HeaderFooterEntry( const PptSlidePersistEntry* pMaster = nullptr );
                        ~HeaderFooterEntry();
};

struct ProcessData
{
    PptSlidePersistEntry&       rPersistEntry;
    SdPageCapsule               pPage;
    ::std::vector< SdrObject* > aBackgroundColoredObjects;
    sal_uInt32*                 pTableRowProperties;

    ProcessData( PptSlidePersistEntry& rP, SdPageCapsule pP ) :
        rPersistEntry               ( rP ),
        pPage                       ( pP ),
        pTableRowProperties         ( nullptr ) {};
    ~ProcessData() { delete[] pTableRowProperties; };
};



class SdrTextObj;
class SfxObjectShell;

class MSFILTER_DLLPUBLIC SdrPowerPointImport : public SdrEscherImport
{
protected:

    friend class PPTTextObj;
    friend class PPTExtParaProv;
    friend struct PPTStyleSheet;
    friend class PPTNumberFormatCreator;

    bool                    bOk;
    PptUserEditAtom         aUserEditAtom;
    PptColorSchemeAtom      aPageColors;
    ::std::vector< SdHyperlinkEntry* > aHyperList;
    sal_uInt32*             pPersistPtr;
    sal_uLong               nPersistPtrAnz;

    const PPTStyleSheet*    pPPTStyleSheet; // this is the current stylesheet;
    const PPTStyleSheet*    pDefaultSheet;  // this is a sheet we are using if no masterpage can be found, but that should
                                            // never happen just preventing a crash
    PptSlidePersistList*    m_pMasterPages;
    PptSlidePersistList*    m_pSlidePages;
    PptSlidePersistList*    m_pNotePages;
    sal_uInt16              nAktPageNum;
    sal_uLong               nDocStreamPos;
    sal_uInt16              nPageColorsNum;
    PptPageKind             ePageColorsKind;
    PptPageKind             eAktPageKind;

protected:
    using SdrEscherImport::ReadObjText;

    bool                    SeekToAktPage(DffRecordHeader* pRecHd=nullptr) const;
    bool                    SeekToDocument(DffRecordHeader* pRecHd=nullptr) const;
    static bool             SeekToContentOfProgTag(
                                sal_Int32 nVersion,
                                SvStream& rSt,
                                const DffRecordHeader& rProgTagBinaryDataHd,
                                DffRecordHeader& rContentHd
                            );
    virtual SdrObject*      ApplyTextObj(
                                PPTTextObj* pTextObj,
                                SdrTextObj* pText,
                                SdPageCapsule pPage,
                                SfxStyleSheet*,
                                SfxStyleSheet** )
                             const;
    virtual SdrObject*      ReadObjText( PPTTextObj* pTextObj, SdrObject* pObj, SdPageCapsule pPage ) const override;
    // #i32596# - new parameter <_nCalledByGroup>, which
    // indicates, if the OLE object is imported inside a group object.
    virtual SdrObject*      ImportOLE(
                                long nOLEId,
                                const Graphic& rGraf,
                                const Rectangle& rBoundRect,
                                const Rectangle& rVisArea,
                                const int _nCalledByGroup,
                                sal_Int64 nAspect
                            ) const override;
    SvMemoryStream*         ImportExOleObjStg( sal_uInt32 nPersistPtr, sal_uInt32& nOleId ) const;
    SdrPage*                MakeBlancPage(bool bMaster) const;
    bool                    ReadFontCollection();
    PptSlidePersistList*    GetPageList(PptPageKind ePageKind) const;
    sal_uInt32              GetAktPageId();
    sal_uInt32              GetMasterPageId(sal_uInt16 nPageNum, PptPageKind ePageKind) const;
    sal_uInt32              GetNotesPageId(sal_uInt16 nPageNum ) const;
    static SdrOutliner*     GetDrawOutliner( SdrTextObj* pSdrText );
    void                    SeekOle( SfxObjectShell* pShell, sal_uInt32 nFilterOptions );

    void                    ApplyTextAnchorAttributes( PPTTextObj& rTextObj, SfxItemSet& rSet ) const;
    bool                    IsVerticalText() const;

public:
                            SdrPowerPointImport( PowerPointImportParam&, const OUString& rBaseURL );
    virtual                 ~SdrPowerPointImport();
    sal_uInt16              GetPageCount( PptPageKind eKind = PPT_SLIDEPAGE ) const;
    void                    SetPageNum( sal_uInt16 nPageNum, PptPageKind = PPT_SLIDEPAGE );
    Size                    GetPageSize() const;
    SdrObject*              ImportPageBackgroundObject(
                                const SdrPage& rPage,
                                sal_uInt32& nBgFileOffset,
                                bool bForce
                            );
    bool                    IsNoteOrHandout( sal_uInt16 nPageNum, PptPageKind ePageKind ) const;
    bool                    HasMasterPage(
                                sal_uInt16 nPageNum,
                                PptPageKind ePageKind = PPT_SLIDEPAGE
                            ) const;
    sal_uInt16              GetMasterPageIndex(
                                sal_uInt16 nPageNum,
                                PptPageKind ePageKind = PPT_SLIDEPAGE
                            ) const;

    void                    ImportPage( SdrPage* pPage, const PptSlidePersistEntry* pMasterPersist = nullptr );
    virtual bool            GetColorFromPalette(sal_uInt16 nNum, Color& rColor) const override;
    virtual bool            SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const override;
    virtual const PptSlideLayoutAtom*   GetSlideLayoutAtom() const override;
    SdrObject*              CreateTable(
                                SdrObject* pGroupObject,
                                sal_uInt32* pTableArry,
                                SvxMSDffSolverContainer*
                            );
    virtual bool ReadFormControl( tools::SvRef<SotStorage>& rSrc1, css::uno::Reference< css::form::XFormComponent > & rFormComp ) const = 0;
};

struct PPTTextCharacterStyleAtomInterpreter
{
    sal_uInt16      nFlags1;
    sal_uInt16      nFlags2;
    sal_uInt16      nFlags3;
    sal_Int32       n1;
    sal_uInt16      nFontHeight;
    sal_Int32       nFontColor;

                    PPTTextCharacterStyleAtomInterpreter();
                    ~PPTTextCharacterStyleAtomInterpreter();
};

struct PPTTextParagraphStyleAtomInterpreter
{
    bool        bValid;
    bool        bForbiddenRules;
    bool        bHangingPunctuation;
    bool        bLatinTextWrap;

                PPTTextParagraphStyleAtomInterpreter();
                ~PPTTextParagraphStyleAtomInterpreter();

    bool        Read( SvStream& rIn, const DffRecordHeader& rRecHd );
};

struct PPTTextSpecInfo
{
    sal_uInt32      nCharIdx;
    sal_uInt16      nLanguage[ 3 ];
    sal_uInt16      nDontKnow;

    explicit PPTTextSpecInfo( sal_uInt32 nCharIdx );
    ~PPTTextSpecInfo();
};

struct  PPTTextSpecInfoAtomInterpreter
{
    bool                bValid;
    ::std::vector< PPTTextSpecInfo* > aList;

                    PPTTextSpecInfoAtomInterpreter();
                    ~PPTTextSpecInfoAtomInterpreter();

    bool            Read(
                        SvStream& rIn,
                        const DffRecordHeader& rRecHd,
                        sal_uInt16 nRecordType,
                        const PPTTextSpecInfo* pTextSpecDefault = nullptr
                    );

};

#define PPT_STYLESHEETENTRYS    9

struct PPTExtParaLevel
{
    sal_uInt32  mnExtParagraphMask;
    sal_uInt16  mnBuBlip;
    sal_uInt16  mnHasAnm;
    sal_uInt32  mnAnmScheme;
    sal_uInt32  mpfPP10Ext;
    sal_uInt32  mnExtCharacterMask;
    sal_uInt32  mcfPP10Ext;
    bool        mbSet;

    PPTExtParaLevel();
    friend SvStream& ReadPPTExtParaLevel( SvStream& rIn, PPTExtParaLevel& rL );
};

struct PPTExtParaSheet
{
    PPTExtParaLevel aExtParaLevel[nMaxPPTLevels];
};

struct PPTBuGraEntry
{
    sal_uInt32  nInstance;
    Graphic     aBuGra;

                PPTBuGraEntry( Graphic& rGraphic, sal_uInt32 nInstance );
};

class PPTExtParaProv
{
    ::std::vector< PPTBuGraEntry* > aBuGraList;

public:
    bool                bStyles;
    bool                bGraphics;
    DffRecordManager    aExtendedPresRules;

    PPTExtParaSheet     aExtParaSheet[ PPT_STYLESHEETENTRYS ];

    bool                GetGraphic( sal_uInt32 nInstance, Graphic& rGraphic ) const;

                        PPTExtParaProv(
                            SdrPowerPointImport& rManager,
                            SvStream& rSt,
                            const DffRecordHeader* pMainMasterHd
                        );
                        ~PPTExtParaProv();
};

struct PPTCharLevel
{
    Color       mnFontColorInStyleSheet;
    sal_uInt32  mnFontColor;
    sal_uInt16  mnFlags;
    sal_uInt16  mnFont;
    sal_uInt16  mnAsianOrComplexFont;
    sal_uInt16  mnFontHeight;
    sal_uInt16  mnEscapement;
};

struct PPTCharSheet
{
    PPTCharLevel    maCharLevel[nMaxPPTLevels];

                    explicit PPTCharSheet( sal_uInt32 nInstance );
                    PPTCharSheet( const PPTCharSheet& rCharSheet );

    void            Read( SvStream& rIn, bool bMasterStyle, sal_uInt32 nLevel, bool bFirst );
};

struct PPTParaLevel
{
    sal_uInt16  mnBuFlags;
    sal_uInt16  mnBulletChar;
    sal_uInt16  mnBulletFont;
    sal_uInt16  mnBulletHeight;
    sal_uInt32  mnBulletColor;

    sal_uInt16  mnAdjust;
    sal_uInt16  mnLineFeed;
    sal_uInt16  mnUpperDist;
    sal_uInt16  mnLowerDist;
    sal_uInt16  mnTextOfs;
    sal_uInt16  mnBulletOfs;
    sal_uInt16  mnDefaultTab;
    sal_uInt16  mnAsianLineBreak;   // bit0:    use asian rules for first and last character
                                    //    1:    do not wrap latin text in the middle of the word
                                    //    2:    allow hanging punctuation
    sal_uInt16  mnBiDi;
};

struct PPTParaSheet
{
public:

    PPTParaLevel    maParaLevel[nMaxPPTLevels];

                    explicit PPTParaSheet( sal_uInt32 nInstance );
                    PPTParaSheet( const PPTParaSheet& rParaSheet );

    bool            Read(
                        SdrPowerPointImport& rMan,
                        SvStream& rIn,
                        bool bMasterStyle,
                        sal_uInt32 nLevel,
                        bool bFirst
                    );
    void        UpdateBulletRelSize(  sal_uInt32 nLevel, sal_uInt16 nFontHeight );
};

class PPTParagraphObj;
class PPTNumberFormatCreator
{
    sal_uInt32 nIsBullet;
    sal_uInt32 nBulletChar;
    sal_uInt32 nBulletFont;
    sal_uInt32 nBulletHeight;
    sal_uInt32 nBulletColor;
    sal_uInt32 nTextOfs;
    sal_uInt32 nBulletOfs;

    void        ImplGetNumberFormat(
                    SdrPowerPointImport& rMan,
                    SvxNumberFormat& rNumberFormat,
                    sal_uInt32 nLevel
                );
    bool       ImplGetExtNumberFormat(
                    SdrPowerPointImport& rMan,
                    SvxNumberFormat& rNumberFormat,
                    sal_uInt32 nLevel,
                    sal_uInt32 nInstance,
                    sal_uInt32 nInstanceInSheet,
                    boost::optional< sal_Int16 >& rStartNumbering,
                    sal_uInt32 nFontHeight,
                    PPTParagraphObj* pPara
                );

protected:

    PPTNumberFormatCreator( PPTExtParaProv* );
    ~PPTNumberFormatCreator();

public:

    PPTExtParaProv*  pExtParaProv;

    void        GetNumberFormat(
                    SdrPowerPointImport& rMan,
                    SvxNumberFormat& rNumberFormat,
                    sal_uInt32 nLevel,
                    const PPTParaLevel& rParaLevel,
                    const PPTCharLevel& rCharLevel,
                    sal_uInt32 nInstance
                );

    bool        GetNumberFormat(
                    SdrPowerPointImport& rMan,
                    SvxNumberFormat& rNumberFormat,
                    PPTParagraphObj* pPara,
                    sal_uInt32 nInstanceInSheet,
                    boost::optional< sal_Int16 >& rStartNumbering
                );
};

class SvxNumBulletItem;
struct PPTStyleSheet : public PPTNumberFormatCreator
{
    PPTTextSpecInfo     maTxSI;
    PPTCharSheet*       mpCharSheet[ PPT_STYLESHEETENTRYS ];
    PPTParaSheet*       mpParaSheet[ PPT_STYLESHEETENTRYS ];
    SvxNumBulletItem*   mpNumBulletItem[ PPT_STYLESHEETENTRYS ];

                        PPTStyleSheet(
                            const DffRecordHeader& rSlideHd,
                            SvStream& rSt, SdrPowerPointImport&,
                            const PPTTextCharacterStyleAtomInterpreter&,
                            const PPTTextParagraphStyleAtomInterpreter&,
                            const PPTTextSpecInfo&
                        );
                        ~PPTStyleSheet();
};

struct ImplPPTParaPropSet
{
    sal_uInt32  mnRefCount;

    sal_uInt16  mnDepth;
    sal_uInt32  mnAttrSet;
    sal_uInt32  mnBulletColor;
    sal_uInt16  mpArry[ 22 ];

    sal_uInt32  mnExtParagraphMask;
    sal_uInt32  mnAnmScheme;
    sal_uInt16  mnHasAnm;
    sal_uInt16  mnBuBlip;

    sal_uInt32  nDontKnow1;
    sal_uInt32  nDontKnow2;
    sal_uInt16  nDontKnow2bit06;

    ImplPPTParaPropSet()
        : mnRefCount( 1 )
        , mnDepth( 0 )
        , mnAttrSet( 0 )
        , mnBulletColor( 0 )
        , mnExtParagraphMask( 0 )
        , mnAnmScheme( 0 )
        , mnHasAnm( 0 )
        , mnBuBlip( 0 )
        , nDontKnow1( 0 )
        , nDontKnow2( 0 )
        , nDontKnow2bit06( 0 )
        { }
};

struct PPTParaPropSet
{
    sal_uInt32          mnOriginalTextPos;
    ImplPPTParaPropSet* pParaSet;

                        PPTParaPropSet();
                        PPTParaPropSet( PPTParaPropSet& rParaPropSet );
                        ~PPTParaPropSet();

    PPTParaPropSet&     operator=( PPTParaPropSet& rParaPropSet );
};

struct ImplPPTCharPropSet
{
    sal_uInt32  mnRefCount;

    sal_uInt32  mnAttrSet;
    sal_uInt16  mnFlags;
    sal_uInt32  mnColor;
    sal_uInt16  mnFont;
    sal_uInt16  mnAsianOrComplexFont;
    sal_uInt16  mnANSITypeface;
    sal_uInt16  mnFontHeight;
    sal_uInt16  mnEscapement;
    sal_uInt16  mnSymbolFont;

    ImplPPTCharPropSet()
        : mnRefCount ( 1 )
        , mnAttrSet( 0 )
        , mnFlags( 0 )
        , mnColor( 0 )
        , mnFont( 0 )
        , mnAsianOrComplexFont( 0 )
        , mnANSITypeface( 0 )
        , mnFontHeight( 0 )
        , mnEscapement( 0 )
        , mnSymbolFont( 0)
        { }
};

struct PPTCharPropSet
{
    //when the bullet text has more than two color,next the text following with bullet has been set hyperlink.
    //now,the bullet color should be set original hyperlink text's color
    //so  "mbHardHylinkOrigColor" hold the original hyperlink text's color.
    sal_uInt32  mnHylinkOrigColor;
    //the bullet text weather has a hyperlink.
    bool        mbIsHyperlink;
    //the hyperlink text weather has a custom color.
    bool        mbHardHylinkOrigColor;

    sal_uInt32          mnOriginalTextPos;
    sal_uInt32          mnParagraph;
    OUString            maString;
    SvxFieldItem*       mpFieldItem;
    sal_uInt16          mnLanguage[ 3 ];

    ImplPPTCharPropSet* pCharSet;

    void                SetFont( sal_uInt16 nFont );
    void                SetColor( sal_uInt32 nColor );

                        explicit PPTCharPropSet( sal_uInt32 nParagraph );
                        PPTCharPropSet( const PPTCharPropSet& rCharPropSet );
                        PPTCharPropSet( const PPTCharPropSet& rCharPropSet, sal_uInt32 nParagraph );
                        ~PPTCharPropSet();

    PPTCharPropSet&     operator=( const PPTCharPropSet& rCharPropSet );

private:
    void                ImplMakeUnique();
};

struct PPTTabEntry
{
    sal_uInt16  nOffset;
    sal_uInt16  nStyle;
};

struct PPTRuler
{
        sal_uInt32          nRefCount;

        sal_Int32           nFlags;
        sal_uInt16          nDefaultTab;
        sal_uInt16          nTextOfs[nMaxPPTLevels];
        sal_uInt16          nBulletOfs[nMaxPPTLevels];
        PPTTabEntry*        pTab;
        sal_uInt16          nTabCount;

        PPTRuler();
        ~PPTRuler();
};

struct PPTTextRulerInterpreter
{
        PPTRuler    *mpImplRuler;

                    PPTTextRulerInterpreter();
                    PPTTextRulerInterpreter( PPTTextRulerInterpreter& rRuler );
                    PPTTextRulerInterpreter(
                        sal_uInt32 nFileOfs,
                        DffRecordHeader& rHd,
                        SvStream& rIn
                    );
                    ~PPTTextRulerInterpreter();

        sal_uInt16  GetTabOffsetByIndex( sal_uInt16 nIndex ) const
                    { return mpImplRuler->pTab[ nIndex ].nOffset; };

        sal_uInt16  GetTabStyleByIndex( sal_uInt16 nIndex ) const
                    { return mpImplRuler->pTab[ nIndex ].nStyle; };

        sal_uInt16  GetTabCount() const { return mpImplRuler->nTabCount; };
        bool        GetDefaultTab( sal_uInt32 nLevel, sal_uInt16& nValue ) const;
        bool        GetTextOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const;
        bool        GetBulletOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const;

        PPTTextRulerInterpreter& operator=( PPTTextRulerInterpreter& rRuler );
};

#define PPT_SPEC_NEWLINE            0x10000
#define PPT_SPEC_SYMBOL             0x20000

struct StyleTextProp9
{
    sal_uInt32  mnExtParagraphMask;
    sal_uInt16  mnBuBlip;
    sal_uInt16  mnHasAnm;
    sal_uInt32  mnAnmScheme;
    sal_uInt32  mpfPP10Ext;
    sal_uInt32  mnExtCharacterMask;
    sal_uInt32  mncfPP10Ext;
    sal_uInt32  mnSpecialInfoMask;
    sal_uInt32  mnPP10Ext;
    sal_uInt16  mfBidi;

    StyleTextProp9()
        : mnExtParagraphMask( 0 )
        , mnBuBlip( 0 )
        , mnHasAnm( 0 )
        , mnAnmScheme( 0 )
        , mpfPP10Ext( 0 )
        , mnExtCharacterMask( 0 )
        , mncfPP10Ext( 0 )
        , mnSpecialInfoMask( 0 )
        , mnPP10Ext( 0 )
        , mfBidi( 0 )
    {
    }
    void Read( SvStream& rSt );
};

typedef std::vector<PPTParaPropSet*> PPTParaPropSetList;
typedef std::vector<PPTCharPropSet*> PPTCharPropSetList;

struct PPTStyleTextPropReader
{
    std::vector< sal_uInt32 >  aSpecMarkerList;    // hiword -> Flags, loword -> Position
    PPTParaPropSetList         aParaPropList;
    PPTCharPropSetList         aCharPropList;

            PPTStyleTextPropReader(
                SvStream& rIn,
                const DffRecordHeader& rClientTextBoxHd,
                PPTTextRulerInterpreter& rInterpreter,
                const DffRecordHeader& rExtParaHd,
                sal_uInt32 nTextInstance
            );
            ~PPTStyleTextPropReader();

    void    Init(
                SvStream& rIn,
                const DffRecordHeader& rClientTextBoxHd,
                PPTTextRulerInterpreter& rInterpreter,
                const DffRecordHeader& rExtParaHd,
                sal_uInt32 nTextInstance
            );
    void    ReadParaProps(
                SvStream& rIn,
                const DffRecordHeader& rTextHeader,
                const OUString& aString,
                PPTTextRulerInterpreter& rRuler,
                sal_uInt32& nCharCount,
                bool& bTextPropAtom
            );
    static void ReadCharProps(
                SvStream& rIn,
                PPTCharPropSet& aCharPropSet,
                const OUString& aString,
                sal_uInt32& nCharCount,
                sal_uInt32 nCharAnzRead,
                bool& bTextPropAtom,
                sal_uInt32 nExtParaPos,
                const std::vector< StyleTextProp9 >& aStyleTextProp9,
                sal_uInt32& nExtParaFlags,
                sal_uInt16& nBuBlip,
                sal_uInt16& nHasAnm,
                sal_uInt32& nAnmScheme
            );
};

class SvxFieldItem;
class MSFILTER_DLLPUBLIC PPTPortionObj : public PPTCharPropSet
{

    friend class PPTParagraphObj;

    const PPTStyleSheet&    mrStyleSheet;
    sal_uInt32              mnInstance;
    sal_uInt32              mnDepth;

public:

    bool            GetAttrib( sal_uInt32 nAttr, sal_uInt32& nVal, sal_uInt32 nInstanceInSheet ) const;
    SvxFieldItem*   GetTextField();

                    PPTPortionObj( const PPTStyleSheet&, sal_uInt32 nInstance, sal_uInt32 nDepth );
                    PPTPortionObj(
                        const PPTCharPropSet&,
                        const PPTStyleSheet&,
                        sal_uInt32 nInstance,
                        sal_uInt32 nDepth
                    );
                    PPTPortionObj( const PPTPortionObj& );
                    ~PPTPortionObj();

    // the following function should be removed during next full update
    void            ApplyTo(
                        SfxItemSet& rSet,
                        SdrPowerPointImport& rManager,
                        sal_uInt32 nInstanceInSheet
                    );
    void            ApplyTo(
                        SfxItemSet& rSet,
                        SdrPowerPointImport& rManager,
                        sal_uInt32 nInstanceInSheet,
                        const PPTTextObj* pTextObj
                    );
    sal_uInt32      Count() const { return ( mpFieldItem ) ? 1 : maString.getLength(); };
    bool            HasTabulator();
};

class MSFILTER_DLLPUBLIC PPTParagraphObj
    :   public PPTParaPropSet,
        public PPTNumberFormatCreator,
        public PPTTextRulerInterpreter
{
    friend class PPTTextObj;
    friend class PPTNumberFormatCreator;

    const PPTStyleSheet&    mrStyleSheet;
    sal_uInt32              mnInstance;

    PPTParagraphObj(PPTParagraphObj const&) = delete;
    void operator=(PPTParagraphObj const&) = delete;

public:

    bool                    mbTab;          // if true, this paragraph has tabulators in text

    sal_uInt32              mnCurrentObject;
    ::std::vector<std::unique_ptr<PPTPortionObj>> m_PortionList;

    void                    UpdateBulletRelSize( sal_uInt32& nBulletRelSize ) const;
    bool                    GetAttrib( sal_uInt32 nAttr, sal_uInt32& nVal, sal_uInt32 nInstanceInSheet );

                            PPTParagraphObj(
                                const PPTStyleSheet&,
                                sal_uInt32 nInstance,
                                sal_uInt16 nDepth
                            );
                            PPTParagraphObj(
                                PPTStyleTextPropReader&,
                                size_t nCurParaPos,
                                size_t& rnCurCharPos,
                                const PPTStyleSheet&,
                                sal_uInt32 nInstance,
                                PPTTextRulerInterpreter& rRuler
                            );
                            ~PPTParagraphObj();

    sal_uInt32              GetTextSize();
    PPTPortionObj*          First();
    PPTPortionObj*          Next();

    void                    AppendPortion( PPTPortionObj& rPortion );
    void                    ApplyTo(
                                SfxItemSet& rSet,
                                boost::optional< sal_Int16 >& rStartNumbering,
                                SdrPowerPointImport& rManager,
                                sal_uInt32 nInstanceInSheet,
                                const PPTParagraphObj* pPrev
                            );
};

#define PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT      1
#define PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER    2
#define PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT     4
#define PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK     8
#define PPT_TEXTOBJ_FLAGS_VERTICAL                      16

struct ImplPPTTextObj
{
    sal_uInt32                  mnRefCount;
    sal_uInt32                  mnShapeId;
    sal_uInt32                  mnShapeMaster;
    PptOEPlaceholderAtom*       mpPlaceHolderAtom;
    sal_uInt16                  mnInstance;
    sal_uInt16                  mnDestinationInstance;
    MSO_SPT                     meShapeType;

    sal_uInt32                  mnCurrentObject;
    sal_uInt32                  mnParagraphCount;
    PPTParagraphObj**           mpParagraphList;
    PptSlidePersistEntry&       mrPersistEntry;

    sal_uInt32                  mnTextFlags;

    explicit ImplPPTTextObj( PptSlidePersistEntry& rPersistEntry )
        : mnRefCount(0)
        , mnShapeId(0)
        , mnShapeMaster(0)
        , mpPlaceHolderAtom(nullptr)
        , mnInstance(0)
        , mnDestinationInstance(0)
        , meShapeType(mso_sptMin)
        , mnCurrentObject(0)
        , mnParagraphCount(0)
        , mpParagraphList(nullptr)
        , mrPersistEntry ( rPersistEntry )
        , mnTextFlags(0) {};
};

class MSFILTER_DLLPUBLIC PPTTextObj
{
    ImplPPTTextObj*         mpImplTextObj;
    void                    ImplClear();

                            PPTTextObj(){};
public:
                            PPTTextObj(
                                SvStream& rSt,
                                SdrPowerPointImport&,
                                PptSlidePersistEntry&,
                                DffObjData*
                            );
                            PPTTextObj( PPTTextObj& rTextObj );
                            ~PPTTextObj();

    sal_uInt32              GetCurrentIndex() const { return mpImplTextObj->mnCurrentObject; };
    sal_uInt32              Count() const { return mpImplTextObj->mnParagraphCount; };
    PPTParagraphObj*        First();
    PPTParagraphObj*        Next();
    MSO_SPT                 GetShapeType() const { return mpImplTextObj->meShapeType; };
    sal_uInt32              GetInstance() const { return mpImplTextObj->mnInstance; };
    void                    SetInstance( sal_uInt16 nInstance )
                            { mpImplTextObj->mnInstance = nInstance; }

    sal_uInt32              GetDestinationInstance() const
                            { return mpImplTextObj->mnDestinationInstance; }

    void                    SetDestinationInstance( sal_uInt16 nInstance )
                            { mpImplTextObj->mnDestinationInstance = nInstance; }

    PptOEPlaceholderAtom*   GetOEPlaceHolderAtom() const { return mpImplTextObj->mpPlaceHolderAtom; }
    sal_uInt32              GetTextFlags() const { return mpImplTextObj->mnTextFlags; }
    void                    SetVertical( bool bVertical )
                            {
                                if ( bVertical )
                                    mpImplTextObj->mnTextFlags |= PPT_TEXTOBJ_FLAGS_VERTICAL;
                                else
                                    mpImplTextObj->mnTextFlags &= ~PPT_TEXTOBJ_FLAGS_VERTICAL;
                            }
    bool                    GetVertical() const
                            { return ( mpImplTextObj->mnTextFlags & PPT_TEXTOBJ_FLAGS_VERTICAL ) != 0; }

    const SfxItemSet*       GetBackground() const;

    PPTTextObj&             operator=( PPTTextObj& rTextObj );
};

class PPTConvertOCXControls : public SvxMSConvertOCXControls
{
    virtual const css::uno::Reference< css::drawing::XDrawPage > & GetDrawPage() override;
    PptPageKind     ePageKind;
    const SdrPowerPointImport* mpPPTImporter;
public:

    PPTConvertOCXControls( const SdrPowerPointImport* pPPTImporter, const css::uno::Reference< css::frame::XModel >& rxModel, PptPageKind ePKind ) :
        SvxMSConvertOCXControls ( rxModel ),
        ePageKind               ( ePKind ),
        mpPPTImporter           ( pPPTImporter )
    {};
    bool ReadOCXStream( tools::SvRef<SotStorage>& rSrc1,
        css::uno::Reference<
        css::drawing::XShape > *pShapeRef=nullptr,
        bool bFloatingCtrl=false );
    virtual bool InsertControl(
        const css::uno::Reference< css::form::XFormComponent > &rFComp,
        const css::awt::Size& rSize,
        css::uno::Reference< css::drawing::XShape > *pShape,
        bool bFloatingCtrl
    ) override;
};

// PowerPoint record types
#define PPT_PST_Document                        1000
#define PPT_PST_DocumentAtom                    1001
#define PPT_PST_SlideAtom                       1007
#define PPT_PST_NotesAtom                       1009
#define PPT_PST_Environment                     1010
#define PPT_PST_SlidePersistAtom                1011
#define PPT_PST_SSSlideInfoAtom                 1017
#define PPT_PST_VBAInfo                         1023
#define PPT_PST_VBAInfoAtom                     1024
#define PPT_PST_SSDocInfoAtom                   1025
#define PPT_PST_ExObjList                       1033
#define PPT_PST_PPDrawingGroup                  1035
#define PPT_PST_PPDrawing                       1036
#define PPT_PST_GridSpacing10Atom               1037
#define PPT_PST_NamedShows                      1040
#define PPT_PST_NamedShow                       1041
#define PPT_PST_NamedShowSlides                 1042
#define PPT_PST_List                            2000
#define PPT_PST_FontCollection                  2005
#define PPT_PST_SoundCollection                 2020
#define PPT_PST_Sound                           2022
#define PPT_PST_SoundData                       2023
#define PPT_PST_ColorSchemeAtom                 2032

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_ExtendedBuGraContainer          2040    // consist of 4041
#define PPT_PST_ExtendedBuGraAtom               2041    // the instance of this atom indices the current graphic

#define PPT_PST_ExObjRefAtom                    3009
#define PPT_PST_OEPlaceholderAtom               3011
#define PPT_PST_OutlineTextRefAtom              3998
#define PPT_PST_TextHeaderAtom                  3999
#define PPT_PST_TextCharsAtom                   4000
#define PPT_PST_StyleTextPropAtom               4001
#define PPT_PST_TxMasterStyleAtom               4003
#define PPT_PST_TxPFStyleAtom                   4005
#define PPT_PST_TextRulerAtom                   4006
#define PPT_PST_TextBytesAtom                   4008
#define PPT_PST_TxSIStyleAtom                   4009
#define PPT_PST_TextSpecInfoAtom                4010

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_ExtendedParagraphAtom           4012
#define PPT_PST_ExtendedParagraphMasterAtom     4013
#define PPT_PST_ExtendedPresRuleContainer       4014    // consist of 4012, 4015,
#define PPT_PST_ExtendedParagraphHeaderAtom     4015    // the instance of this atom indices the current presobj
                                                        // the first sal_uInt32 in this atom indices the current slideId
#define PPT_PST_TextDefaults9Atom               4016

#define PPT_PST_FontEntityAtom                  4023
#define PPT_PST_CString                         4026
#define PPT_PST_ExOleObjAtom                    4035
#define PPT_PST_SrKinsoku                       4040
#define PPT_PST_ExEmbed                         4044
#define PPT_PST_ExHyperlinkAtom                 4051
#define PPT_PST_ExHyperlink                     4055
#define PPT_PST_SlideNumberMCAtom               4056
#define PPT_PST_HeadersFooters                  4057
#define PPT_PST_HeadersFootersAtom              4058
#define PPT_PST_TxInteractiveInfoAtom           4063
#define PPT_PST_MasterText                      4068
#define PPT_PST_RecolorInfoAtom                 4071
#define PPT_PST_ExControl                       4078
#define PPT_PST_SlideListWithText               4080
#define PPT_PST_AnimationInfoAtom               4081
#define PPT_PST_InteractiveInfo                 4082
#define PPT_PST_InteractiveInfoAtom             4083
#define PPT_PST_UserEditAtom                    4085
#define PPT_PST_CurrentUserAtom                 4086
#define PPT_PST_DateTimeMCAtom                  4087
#define PPT_PST_GenericDateMCAtom               4088
#define PPT_PST_HeaderMCAtom                    4089
#define PPT_PST_FooterMCAtom                    4090
#define PPT_PST_ExMediaAtom                     4100
#define PPT_PST_ExVideo                         4101
#define PPT_PST_ExAviMovie                      4102
#define PPT_PST_ExMCIMovie                      4103
#define PPT_PST_AnimationInfo                   4116
#define PPT_PST_RTFDateTimeMCAtom               4117
#define PPT_PST_ProgTags                        5000
#define PPT_PST_ProgBinaryTag                   5002
#define PPT_PST_BinaryTagData                   5003
#define PPT_PST_PersistPtrIncrementalBlock      6002

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_PresentationAdvisorFlags9Atom   6010
#define PPT_PST_HtmlDocInfo9Atom                6011


#define PPT_PST_HashCodeAtom                    11008
#define PPT_PST_BuildList                       11010
#define PPT_PST_Comment10                       12000
#define PPT_PST_CommentAtom10                   12001
#define PPT_PST_CommentIndex10                  12004
#define PPT_PST_SlideFlags10Atom                12010
#define PPT_PST_SlideTime10Atom                 12011
#define PPT_PST_DocToolbarStates10Atom          14001

// attributes for PptTextStyleSheet
#define PPT_ParaAttr_BulletOn       0   //00000001
#define PPT_ParaAttr_BuHardFont     1   //00000002
#define PPT_ParaAttr_BuHardColor    2   //00000004
#define PPT_ParaAttr_BuHardHeight   3   //00000008
#define PPT_ParaAttr_BulletFont     4   //00000010
#define PPT_ParaAttr_BulletColor    5   //00000020
#define PPT_ParaAttr_BulletHeight   6   //00000040
#define PPT_ParaAttr_BulletChar     7   //00000080
#define PPT_ParaAttr_Adjust         11  //00000800 0000=Left, 0001=Center, 0002=Right, 0003=Block
#define PPT_ParaAttr_LineFeed       12  //00001000
#define PPT_ParaAttr_UpperDist      13  //00002000 is set to 0032 for TextFrames by default
#define PPT_ParaAttr_LowerDist      14  //00004000
#define PPT_ParaAttr_TextOfs        15  //00008000
#define PPT_ParaAttr_BulletOfs      16  //00010000
#define PPT_ParaAttr_DefaultTab     17  //00020000
#define PPT_ParaAttr_AsianLB_1      18
#define PPT_ParaAttr_AsianLB_2      19
#define PPT_ParaAttr_AsianLB_3      20
#define PPT_ParaAttr_BiDi           21  //00200000

#define PPT_CharAttr_Bold               0   //00000001
#define PPT_CharAttr_Italic             1   //00000002
#define PPT_CharAttr_Underline          2   //00000004
#define PPT_CharAttr_Shadow             4   //00000010
#define PPT_CharAttr_Strikeout          8   //00000100
#define PPT_CharAttr_Embossed           9   //00000200
#define PPT_CharAttr_Font               16  //00010000
#define PPT_CharAttr_AsianOrComplexFont 21  //00200000
#define PPT_CharAttr_ANSITypeface       22  //00400000
#define PPT_CharAttr_Symbol             23  //00800000
#define PPT_CharAttr_FontHeight         17  //00020000
#define PPT_CharAttr_FontColor          18  //00040000
#define PPT_CharAttr_Escapement         19  //00080000

// values for PptSlideLayoutAtom.eLayout
enum class PptSlideLayout
{
    TITLESLIDE             =  0,   // The slide is a title slide
    TITLEANDBODYSLIDE      =  1,   // Title and body slide
    TITLEMASTERSLIDE       =  2,   // Title master slide
    MASTERSLIDE            =  3,   // Master slide layout
    MASTERNOTES            =  4,   // Master notes layout
    NOTESTITLEBODY         =  5,   // Notes title/body layout
    HANDOUTLAYOUT          =  6,   // Handout layout, therefore it doesn't have placeholders except header, footer, and date
    ONLYTITLE              =  7,   // Only title placeholder
    TWOCOLUMNSANDTITLE     =  8,   // Body of the slide has 2 columns and a title
    TWOROWSANDTITLE        =  9,   // Slide's body has 2 rows and a title
    RIGHTCOLUMN2ROWS       =  10,  // Body contains 2 columns, right column has 2 rows
    LEFTCOLUMN2ROWS        =  11,  // Body contains 2 columns, left column has 2 rows
    BOTTOMROW2COLUMNS      =  12,  // Body contains 2 rows, bottom row has 2 columns
    TOPROW2COLUMN          =  13,  // Body contains 2 rows, top row has 2 columns
    FOUROBJECTS            =  14,  // 4 objects
    BIGOBJECT              =  15,  // Big object
    BLANCSLIDE             =  16,  // Blank slide
    TITLERIGHTBODYLEFT     =  17,  // Vertical title on the right, body on the left
    TITLERIGHT2BODIESLEFT  =  18   // Vertical title on the right, body on the left split into 2 rows
};

// the following table describes the placeholder id's (values from reality followed by values taken from the documentation)
enum class PptPlaceholder
{
    NONE                   = 0,   //  0 None
    MASTERTITLE            = 1,   //  1 Master title
    MASTERBODY             = 2,   //  2 Master body
    MASTERCENTEREDTITLE    = 3,   //  3 Master centered title
    MASTERSUBTITLE         = 4,   // 10 Master subtitle
    MASTERNOTESSLIDEIMAGE  = 5,   //  4 Master notes slide image
    MASTERNOTESBODYIMAGE   = 6,   //  5 Master notes body image
    MASTERDATE             = 7,   //  6 Master date
    MASTERSLIDENUMBER      = 8,   //  7 Master slide number
    MASTERFOOTER           = 9,   //  8 Master footer
    MASTERHEADER           = 10,  //  9 Master header
                                  // 11 Generic text object
    TITLE                  = 13,  // 12 Title
    BODY                   = 14,  // 13 Body
    NOTESBODY              = 12,  // 14 Notes body
    CENTEREDTITLE          = 15,  // 15 Centered title
    SUBTITLE               = 16,  // 16 Subtitle
    VERTICALTEXTTITLE      = 17,  // 17 Vertical text title
    VERTICALTEXTBODY       = 18,  // 18 Vertical text body
    NOTESSLIDEIMAGE        = 11,  // 19 Notes slide image
    OBJECT                 = 19,  // 20 Object (no matter the size)
    GRAPH                  = 20,  // 21 Graph
    TABLE                  = 21,  // 22 Table
    CLIPART                = 22,  // 23 Clip Art
    ORGANISZATIONCHART     = 23,  // 24 Organization Chart
    MEDIACLIP              = 24  // 25 Media Clip
};

#endif // INCLUDED_FILTER_MSFILTER_SVDFPPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
