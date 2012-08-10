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

#ifndef _SVDFPPT_HXX
#define _SVDFPPT_HXX

#include <tools/solar.h>
#include <tools/string.hxx>
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
#include "filter/msfilter/msfilterdllapi.h"
#include <vcl/font.hxx>
#include <vector>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class SdrModel;
class SdPage;
class SdrPage;
class SdrObject;
class SvStream;
class Polygon;
class PolyPolygon;
class SfxItemSet;
class Outliner;
class Graphic;
class SvxMSDffManager;
class PPTTextObj;
class DffRecordHeader;
class SvxBulletItem;

#define PPT_IMPORTFLAGS_NO_TEXT_ASSERT  1

struct MSFILTER_DLLPUBLIC PptCurrentUserAtom
{
    sal_uInt32  nMagic;
    sal_uInt32  nCurrentUserEdit;
    sal_uInt16  nDocFileVersion;
    sal_uInt8   nMajorVersion;
    sal_uInt8   nMinorVersion;
    String      aCurrentUser;

public:

    PptCurrentUserAtom() :  nMagic              ( 0 ),
                            nCurrentUserEdit    ( 0 ),
                            nDocFileVersion     ( 0 ),
                            nMajorVersion       ( 0 ),
                            nMinorVersion       ( 0 ) {}

    MSFILTER_DLLPUBLIC friend SvStream& operator>>( SvStream& rIn, PptCurrentUserAtom& rAtom );
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
    String      aTarget;
    String      aSubAdress;

    sal_Int32   nStartPos;
    sal_Int32   nEndPos;

    String      aConvSubString;
    ESelection  aESelection;
    sal_Bool    bSelection;
};

// Hilfsklasse zum Einlesen der PPT InteractiveInfoAtom
struct MSFILTER_DLLPUBLIC PptInteractiveInfoAtom
{
    sal_uInt32          nSoundRef;
    sal_uInt32          nExHyperlinkId;
    sal_uInt8           nAction;
    sal_uInt8           nOleVerb;
    sal_uInt8           nJump;
    sal_uInt8           nFlags;
    sal_uInt8           nHyperlinkType;

    // unknown, da Gesamtgroesse 16 ist
    sal_uInt8           nUnknown1;
    sal_uInt8           nUnknown2;
    sal_uInt8           nUnknown3;

public:

    MSFILTER_DLLPUBLIC friend SvStream& operator>>( SvStream& rIn, PptInteractiveInfoAtom& rAtom );
};

enum PptPageKind { PPT_MASTERPAGE, PPT_SLIDEPAGE, PPT_NOTEPAGE };

enum PptPageFormat
{   PPTPF_SCREEN,   //
    PPTPF_USLETTER, // 8.5x11"
    PPTPF_A4,       // 210x297mm
    PPTPF_35MMDIA,  // DIA
    PPTPF_OVERHEAD, //
    PPTPF_CUSTOM
};

// Werte fuer den sal_uLong im PPT_PST_TextHeaderAtom
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

// Inventor-Id fuer PPT UserData
const sal_uInt32 PPTInventor = sal_uInt32('P') * 0x00000001
                             + sal_uInt32('P') * 0x00000100
                             + sal_uInt32('T') * 0x00010000
                             + sal_uInt32('0') * 0x01000000;

// Object-Ids fuer StarDraw UserData
#define PPT_OBJECTINFO_ID       (1)

struct MSFILTER_DLLPUBLIC PptDocumentAtom
{
    Size            aSlidesPageSize;                    // page size of the slides in 576DPI
    Size            aNotesPageSize;                     // page size of the notes in 576DPI
    // RatioAtom erstmal weggelassen
    sal_uInt32      nNotesMasterPersist;                // 0=non-existent
    sal_uInt32      nHandoutMasterPersist;              // 0=non-existent
    sal_uInt16      n1stPageNumber;                     // page number of the first slide
    PptPageFormat   eSlidesPageFormat;                  // page format of the slides
    sal_Bool        bEmbeddedTrueType           : 1;    // TrueType direcly within the File?
    sal_Bool        bTitlePlaceholdersOmitted   : 1;
    sal_Bool        bRightToLeft                : 1;
    sal_Bool        bShowComments               : 1;

public:

    Size    GetPageSize( const Size& rSiz ) const;
    Size    GetSlidesPageSize() const { return GetPageSize( aSlidesPageSize ); }
    Size    GetNotesPageSize() const { return GetPageSize( aNotesPageSize ); }

    friend SvStream& operator>>( SvStream& rIn, PptDocumentAtom& rAtom );
};

struct PptSlideLayoutAtom
{
    sal_Int32           eLayout;                // 0..18
    sal_uInt8           aPlaceholderId[ 8 ];
    sal_uLong           aPlacementId[ 8 ];

public:
                        PptSlideLayoutAtom() { Clear(); }
    void                Clear();

    // Das SlideLayoutAtom wird ohne Header eingelesen!
    friend SvStream& operator>>( SvStream& rIn, PptSlideLayoutAtom& rAtom );
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

    friend SvStream& operator>>(SvStream& rIn, PptSlideAtom& rAtom);
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

    friend SvStream& operator>>(SvStream& rIn, PptSlidePersistAtom& rAtom);
};

struct PptNotesAtom
{
    sal_uInt32          nSlideId;
    sal_uInt16          nFlags;

public:
                        PptNotesAtom() { Clear(); }
    void                Clear();

    friend SvStream& operator>>(SvStream& rIn, PptNotesAtom& rAtom);
};

struct PptColorSchemeAtom
{
    sal_uInt8           aData[32];

public:
                        PptColorSchemeAtom () { Clear(); }
    void                Clear();
    Color               GetColor( sal_uInt16 nNum ) const;

    friend SvStream& operator>>(SvStream& rIn, PptColorSchemeAtom& rAtom);
};

struct PptFontEntityAtom
{
    String              aName;
    double              fScaling;
    sal_uInt8           lfClipPrecision;
    sal_uInt8           lfQuality;

    sal_uInt32          nUniqueFontId;          // not used anymore
    CharSet             eCharSet;
    FontFamily          eFamily;
    FontPitch           ePitch;
    sal_Bool            bAvailable;

    friend SvStream& operator>>(SvStream& rIn, PptFontEntityAtom& rAtom);
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
    PptUserEditAtom() : nOffsetPersistDirectory( 0 ) {}

    friend SvStream& operator>>( SvStream& rIn, PptUserEditAtom& rAtom );
};

struct PptOEPlaceholderAtom
{
    sal_uInt32          nPlacementId;
    sal_uInt8           nPlaceholderId;
    sal_uInt8           nPlaceholderSize; // 0=Full size, 1=Half size, 2=Quarter of Slide

public:
                        PptOEPlaceholderAtom() { Clear(); }
    void                Clear();

    friend SvStream& operator>>( SvStream& rIn, PptOEPlaceholderAtom& rAtom );
};

struct ProcessData;
struct PPTStyleSheet;
struct HeaderFooterEntry;
struct PptSlidePersistEntry
{
    PptSlidePersistAtom aPersistAtom;
    PptSlideAtom        aSlideAtom;
    PptNotesAtom        aNotesAtom;
    PptColorSchemeAtom  aColorScheme;           // each slide includes this colorscheme atom
    PPTStyleSheet*      pStyleSheet;            // stylesheet of this page ( only in masterpages ), since XP supports more than one masterpage

    sal_uInt32          HeaderFooterOfs[ 4 ];   // containing the ofs to the placeholder (only masterpage)
    HeaderFooterEntry*  pHeaderFooterEntry;
    SvxMSDffSolverContainer* pSolverContainer;
    sal_uInt32          nSlidePersistStartOffset;// is an array to the end of the SlidePersistAtom of this page, TextHeaderAtom is following
    sal_uInt32          nSlidePersistEndOffset;
    sal_uInt32          nBackgroundOffset;      // fileoffset
    sal_uInt32          nDrawingDgId;           // valid, if not -1
    sal_uInt32*         pPresentationObjects;   // if valid, this is a pointer to an array that includes the offsets to the presentation objects
                                                // on this masterpage for each instance ( 0 - 8 );
    SdrObject*          pBObj;
    sal_Bool            bBObjIsTemporary;

    void*               pDummy2;
    PptPageKind         ePageKind;              //

    sal_Bool            bNotesMaster    : 1;    // for NotesMaster
    sal_Bool            bHandoutMaster  : 1;    // for HandoutMaster
    sal_Bool            bStarDrawFiller : 1;    // special for StarDraw
public:
                        PptSlidePersistEntry();
                        ~PptSlidePersistEntry();
    sal_uInt32          GetSlideId() const { return aPersistAtom.nSlideId; }
};

class _PptSlidePersistList : public std::vector<PptSlidePersistEntry*>
{
public:
    ~_PptSlidePersistList()
    {
        for( const_iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};

#define PPTSLIDEPERSIST_ENTRY_NOTFOUND 0xFFFF

class MSFILTER_DLLPUBLIC PptSlidePersistList: public _PptSlidePersistList
{
public:
    sal_uInt16          FindPage( sal_uInt32 nId ) const;
};

class  SfxObjectShell;
struct PPTOleEntry
{
    sal_uInt32          nId;                        // OleId
    sal_uInt32          nPersistPtr;                // PersistPtr
    sal_uInt32          nRecHdOfs;                  // points to the record header: ExObjListHd
    SfxObjectShell*     pShell;
    sal_uInt16          nType;                      // maybe PPT_PST_ExEmbed or PPT_PST_ExControl
    sal_uInt32          nAspect;                    // the aspect of the OLE object

    PPTOleEntry( sal_uInt32 nid, sal_uInt32 nOfs, SfxObjectShell* pSh, sal_uInt16 nT, sal_uInt32 nAsp ) :
        nId             ( nid ),
        nRecHdOfs       ( nOfs ),
        pShell          ( pSh ),
        nType           ( nT ),
        nAspect         ( nAsp ) {}
};

struct PptExOleObjAtom
{
    sal_uInt32  nAspect;
    sal_uInt32  nDummy1;
    sal_uInt32  nId;
    sal_uInt32  nDummy2;
    sal_uInt32  nPersistPtr;
    sal_uInt32  nDummy4;

public:

    friend SvStream& operator>>( SvStream& rIn, PptExOleObjAtom& rAtom );
};

typedef ::std::vector< PPTOleEntry* > PPTOleEntryList;
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

    PPTOleEntryList     aOleObjectList;             // contains PPTOleEntrys

    PptFontCollection*  pFonts;

    sal_uInt32          nStreamLen;
    sal_uInt16          nTextStylesIndex;

    CharSet             eCharSetSystem;

    sal_Bool            bWingdingsChecked       : 1;
    sal_Bool            bWingdingsAvailable     : 1;
    sal_Bool            bMonotypeSortsChecked   : 1;
    sal_Bool            bMonotypeSortsAvailable : 1;
    sal_Bool            bTimesNewRomanChecked   : 1;
    sal_Bool            bTimesNewRomanAvailable : 1;

    sal_Bool            ReadString( rtl::OUString& rStr ) const;
    // nur fuer PowerPoint-Filter:
    virtual const PptSlideLayoutAtom* GetSlideLayoutAtom() const;

public:
    using SvxMSDffManager::ReadObjText;

    PowerPointImportParam& rImportParam;

    void*               pSdrEscherDummy1;
    void*               pSdrEscherDummy2;
    void*               pSdrEscherDummy3;
    void*               pSdrEscherDummy4;

                        SdrEscherImport( PowerPointImportParam&, const String& rBaseURL );
    virtual             ~SdrEscherImport();
    virtual bool        GetColorFromPalette( sal_uInt16 nNum, Color& rColor ) const;
    virtual sal_Bool    SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const;
    PptFontEntityAtom*  GetFontEnityAtom( sal_uInt32 nNum ) const;
    void                RecolorGraphic( SvStream& rSt, sal_uInt32 nRecLen, Graphic& rGraph );
    virtual SdrObject*  ReadObjText( PPTTextObj* pTextObj, SdrObject* pObj, SdPage* pPage ) const;
    virtual SdrObject*  ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj );
    virtual void        ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* pData, DffObjData& rObj );
    void                ImportHeaderFooterContainer( DffRecordHeader& rHeader, HeaderFooterEntry& rEntry );
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SD_HEADERFOOTER_DATE        1
#define SD_HEADERFOOTER_TODAYDATE   2
#define SD_HEADERFOOTER_USERDATE    4
#define SD_HEADERFOOTER_SLIDENUMBER 8
#define SD_HEADERFOOTER_HEADER      16
#define SD_HEADERFOOTER_FOOTER      32

class SvxFieldItem;
struct MSFILTER_DLLPUBLIC PPTFieldEntry
{
    sal_uInt16          nPos;
    sal_uInt16          nTextRangeEnd;
    SvxFieldItem*       pField1;
    SvxFieldItem*       pField2;
    String*             pString;

    PPTFieldEntry() : nPos( 0 ), nTextRangeEnd( 0 ), pField1( NULL ), pField2( NULL ), pString( NULL ) {};
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
    String              pPlaceholder[ 4 ];
    sal_uInt32          nAtom;

    sal_uInt32          GetMaskForInstance( sal_uInt32 nInstance );
    sal_uInt32          IsToDisplay( sal_uInt32 nInstance );
    sal_uInt32          NeedToImportInstance(
                            const sal_uInt32 nInstance,
                            const PptSlidePersistEntry& rSlidePersist
                        );

                        explicit HeaderFooterEntry( const PptSlidePersistEntry* pMaster = NULL );
                        ~HeaderFooterEntry();
};

struct ProcessData
{
    PptSlidePersistEntry&       rPersistEntry;
    SdPage*                     pPage;
    ::std::vector< SdrObject* > aBackgroundColoredObjects;
    sal_uInt32*                 pTableRowProperties;

    ProcessData( PptSlidePersistEntry& rP, SdPage* pP ) :
        rPersistEntry               ( rP ),
        pPage                       ( pP ),
        pTableRowProperties         ( NULL ) {};
    ~ProcessData() { delete[] pTableRowProperties; };
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef ::std::vector< SdHyperlinkEntry* > SdHyperlinkEntryList;
class SdrTextObj;
class SfxObjectShell;

class MSFILTER_DLLPUBLIC SdrPowerPointImport : public SdrEscherImport
{
protected:

    friend class PPTTextObj;
    friend class PPTExtParaProv;
    friend struct PPTStyleSheet;
    friend class PPTNumberFormatCreator;

    sal_Bool                bOk;
    PptUserEditAtom         aUserEditAtom;
    PptColorSchemeAtom      aPageColors;
    SdHyperlinkEntryList    aHyperList;
    sal_uInt32*             pPersistPtr;
    sal_uLong               nPersistPtrAnz;

    const PPTStyleSheet*    pPPTStyleSheet; // this is the current stylesheet;
    const PPTStyleSheet*    pDefaultSheet;  // this is a sheet we are using if no masterpage can be found, but that should
                                            // never happen just preventing a crash
    PptSlidePersistList*    pMasterPages;
    PptSlidePersistList*    pSlidePages;
    PptSlidePersistList*    pNotePages;
    sal_uInt16              nAktPageNum;
    sal_uLong               nDocStreamPos;
    sal_uInt16              nPageColorsNum;
    PptPageKind             ePageColorsKind;
    PptPageKind             eAktPageKind;

protected:
    using SdrEscherImport::ReadObjText;

    sal_Bool                SeekToAktPage(DffRecordHeader* pRecHd=NULL) const;
    sal_Bool                SeekToDocument(DffRecordHeader* pRecHd=NULL) const;
    sal_Bool                SeekToContentOfProgTag(
                                sal_Int32 nVersion,
                                SvStream& rSt,
                                const DffRecordHeader& rProgTagBinaryDataHd,
                                DffRecordHeader& rContentHd
                            );
    virtual SdrObject*      ApplyTextObj(
                                PPTTextObj* pTextObj,
                                SdrTextObj* pText,
                                SdPage* pPage,
                                SfxStyleSheet*,
                                SfxStyleSheet** )
                             const;
    virtual SdrObject*      ReadObjText( PPTTextObj* pTextObj, SdrObject* pObj, SdPage* pPage ) const;
    // #i32596# - new parameter <_nCalledByGroup>, which
    // indicates, if the OLE object is imported inside a group object.
    virtual SdrObject*      ImportOLE(
                                long nOLEId,
                                const Graphic& rGraf,
                                const Rectangle& rBoundRect,
                                const Rectangle& rVisArea,
                                const int _nCalledByGroup,
                                sal_Int64 nAspect
                            ) const;
    SvMemoryStream*         ImportExOleObjStg( sal_uInt32 nPersistPtr, sal_uInt32& nOleId ) const;
    SdrPage*                MakeBlancPage(sal_Bool bMaster) const;
    sal_Bool                ReadFontCollection();
    sal_Bool                ForceFontCollection() const
                            { return pFonts!=NULL?sal_True:((SdrPowerPointImport*)this)->ReadFontCollection(); }
    PptSlidePersistList*    GetPageList(PptPageKind ePageKind) const;
    sal_uInt32              GetAktPageId();
    sal_uInt32              GetMasterPageId(sal_uInt16 nPageNum, PptPageKind ePageKind) const;
    sal_uInt32              GetNotesPageId(sal_uInt16 nPageNum ) const;
    SdrOutliner*            GetDrawOutliner( SdrTextObj* pSdrText ) const;
    void                    SeekOle( SfxObjectShell* pShell, sal_uInt32 nFilterOptions );

public:
                            SdrPowerPointImport( PowerPointImportParam&, const String& rBaseURL );
    virtual                 ~SdrPowerPointImport();
    sal_uInt16              GetPageCount( PptPageKind eKind = PPT_SLIDEPAGE ) const;
    void                    SetPageNum( sal_uInt16 nPageNum, PptPageKind = PPT_SLIDEPAGE );
    sal_uInt16              GetPageNum() const { return nAktPageNum; }
    PptPageKind             GetPageKind() const { return eAktPageKind; }
    Size                    GetPageSize() const;
    SdrObject*              ImportPageBackgroundObject(
                                const SdrPage& rPage,
                                sal_uInt32& nBgFileOffset,
                                sal_Bool bForce
                            );
    sal_Bool                IsNoteOrHandout( sal_uInt16 nPageNum, PptPageKind ePageKind ) const;
    sal_Bool                HasMasterPage(
                                sal_uInt16 nPageNum,
                                PptPageKind ePageKind = PPT_SLIDEPAGE
                            ) const;
    sal_uInt16              GetMasterPageIndex(
                                sal_uInt16 nPageNum,
                                PptPageKind ePageKind = PPT_SLIDEPAGE
                            ) const;

    void                    ImportPage( SdrPage* pPage, const PptSlidePersistEntry* pMasterPersist = NULL );
    virtual bool            GetColorFromPalette(sal_uInt16 nNum, Color& rColor) const;
    virtual sal_Bool        SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const;
    sal_Unicode             PPTSubstitute(
                                sal_uInt16 nFont,
                                sal_Unicode nChar,
                                sal_uInt32& nMappedFontId,
                                Font& rFont,
                                char nDefault
                            ) const;
    const PptDocumentAtom&  GetDocumentAtom() const { return aDocAtom; }
    virtual const PptSlideLayoutAtom*   GetSlideLayoutAtom() const;
    SdrObject*              CreateTable(
                                SdrObject* pGroupObject,
                                sal_uInt32* pTableArry,
                                SvxMSDffSolverContainer*
                            );
    virtual bool ReadFormControl( SotStorageRef& rSrc1, com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rFormComp ) const = 0;
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

    sal_Bool        Read( SvStream& rIn, const DffRecordHeader& rRecHd );

    sal_uInt32      GetColor( sal_uInt32 nDefault );
};

struct PPTTextParagraphStyleAtomInterpreter
{
    sal_Bool        bValid;
    sal_Bool        bForbiddenRules;
    sal_Bool        bHangingPunctuation;
    sal_Bool        bLatinTextWrap;

                    PPTTextParagraphStyleAtomInterpreter();
                    ~PPTTextParagraphStyleAtomInterpreter();

    sal_Bool        Read( SvStream& rIn, const DffRecordHeader& rRecHd );
};

struct PPTTextSpecInfo
{
    sal_uInt32      nCharIdx;
    sal_uInt16      nLanguage[ 3 ];
    sal_uInt16      nDontKnow;

    explicit PPTTextSpecInfo( sal_uInt32 nCharIdx );
    ~PPTTextSpecInfo();
};

typedef ::std::vector< PPTTextSpecInfo* > PPTTextSpecInfoList;
struct  PPTTextSpecInfoAtomInterpreter
{
    sal_Bool            bValid;
    PPTTextSpecInfoList aList;

                    PPTTextSpecInfoAtomInterpreter();
                    ~PPTTextSpecInfoAtomInterpreter();

    sal_Bool        Read(
                        SvStream& rIn,
                        const DffRecordHeader& rRecHd,
                        sal_uInt16 nRecordType,
                        const PPTTextSpecInfo* pTextSpecDefault = NULL
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
    sal_Bool    mbSet;

    PPTExtParaLevel();
    friend SvStream& operator>>( SvStream& rIn, PPTExtParaLevel& rL );
};

struct PPTExtParaSheet
{
    PPTExtParaLevel aExtParaLevel[ 5 ];
};

struct PPTBuGraEntry
{
    sal_uInt32  nInstance;
    Graphic     aBuGra;

                PPTBuGraEntry( Graphic& rGraphic, sal_uInt32 nInstance );
};

typedef ::std::vector< PPTBuGraEntry* > PPTBuGraEntryList;

class PPTExtParaProv
{
    PPTBuGraEntryList   aBuGraList;

public :
    sal_Bool            bStyles;
    sal_Bool            bGraphics;
    DffRecordManager    aExtendedPresRules;

    PPTExtParaSheet     aExtParaSheet[ PPT_STYLESHEETENTRYS ];

    sal_Bool            GetGraphic( sal_uInt32 nInstance, Graphic& rGraphic ) const;

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
    PPTCharLevel    maCharLevel[ 5 ];

                    explicit PPTCharSheet( sal_uInt32 nInstance );
                    PPTCharSheet( const PPTCharSheet& rCharSheet );

    void            Read( SvStream& rIn, sal_Bool bMasterStyle, sal_uInt32 nLevel, sal_Bool bFirst );
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

    PPTParaLevel    maParaLevel[ 5 ];

                    explicit PPTParaSheet( sal_uInt32 nInstance );
                    PPTParaSheet( const PPTParaSheet& rParaSheet );

    void            Read(
                        SdrPowerPointImport& rMan,
                        SvStream& rIn,
                        sal_Bool bMasterStyle,
                        sal_uInt32 nLevel,
                        sal_Bool bFirst
                    );
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
    sal_Bool    ImplGetExtNumberFormat(
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

    sal_Bool    GetNumberFormat(
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
    void*               mpDummy;

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
                { mnRefCount = 1; mnAttrSet = 0; mnExtParagraphMask = 0; mnDepth = 0; };
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

    ImplPPTCharPropSet(){ mnRefCount = 1; mnAttrSet = 0; };
};

struct PPTCharPropSet
{

    sal_uInt32          mnOriginalTextPos;
    sal_uInt32          mnParagraph;
    String              maString;
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
        sal_uInt16          nTextOfs[ 5 ];
        sal_uInt16          nBulletOfs[ 5 ];
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
                        SdrPowerPointImport&,
                        DffRecordHeader& rHd,
                        SvStream& rIn
                    );
                    ~PPTTextRulerInterpreter();

        sal_uInt16  GetTabOffsetByIndex( sal_uInt16 nIndex ) const
                    { return mpImplRuler->pTab[ nIndex ].nOffset; };

        sal_uInt16  GetTabStyleByIndex( sal_uInt16 nIndex ) const
                    { return mpImplRuler->pTab[ nIndex ].nStyle; };

        sal_uInt16  GetTabCount() const { return mpImplRuler->nTabCount; };
        sal_Bool    GetDefaultTab( sal_uInt32 nLevel, sal_uInt16& nValue ) const;
        sal_Bool    GetTextOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const;
        sal_Bool    GetBulletOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const;

        PPTTextRulerInterpreter& operator=( PPTTextRulerInterpreter& rRuler );
};

#define PPT_SPEC_NEWLINE            0x10000
#define PPT_SPEC_SYMBOL             0x20000
#define PPT_SPEC_USE_STARBATS       0x40000

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
                SdrPowerPointImport&,
                const DffRecordHeader& rClientTextBoxHd,
                PPTTextRulerInterpreter& rInterpreter,
                const DffRecordHeader& rExtParaHd,
                sal_uInt32 nTextInstance
            );
            ~PPTStyleTextPropReader();

    void    Init(
                SvStream& rIn,
                SdrPowerPointImport&,
                const DffRecordHeader& rClientTextBoxHd,
                PPTTextRulerInterpreter& rInterpreter,
                const DffRecordHeader& rExtParaHd,
                sal_uInt32 nTextInstance
            );
    void    ReadParaProps(
                SvStream& rIn,
                SdrPowerPointImport& rMan,
                const DffRecordHeader& rTextHeader,
                const String& aString,
                PPTTextRulerInterpreter& rRuler,
                sal_uInt32& nCharCount,
                sal_Bool& bTextPropAtom
            );
    void    ReadCharProps(
                SvStream& rIn,
                PPTCharPropSet& aCharPropSet,
                const String& aString,
                sal_uInt32& nCharCount,
                sal_uInt32 nCharAnzRead,
                sal_Bool& bTextPropAtom,
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

    sal_Bool        GetAttrib( sal_uInt32 nAttr, sal_uInt32& nVal, sal_uInt32 nInstanceInSheet ) const;
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
    sal_uInt32      Count() const { return ( mpFieldItem ) ? 1 : maString.Len(); };
    sal_Bool        HasTabulator();
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

protected:

    void                    ImplClear();

public:

    sal_Bool                mbTab;          // if true, this paragraph has tabulators in text

    sal_uInt32              mnCurrentObject;
    ::boost::ptr_vector<PPTPortionObj> m_PortionList;

    void                    UpdateBulletRelSize( sal_uInt32& nBulletRelSize ) const;
    sal_Bool                GetAttrib( sal_uInt32 nAttr, sal_uInt32& nVal, sal_uInt32 nInstanceInSheet );

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

    explicit ImplPPTTextObj( PptSlidePersistEntry& rPersistEntry ) : mrPersistEntry ( rPersistEntry ) {};
};

class PPTTextObj
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
    void                    SetVertical( sal_Bool bVertical )
                            {
                                if ( bVertical )
                                    mpImplTextObj->mnTextFlags |= PPT_TEXTOBJ_FLAGS_VERTICAL;
                                else
                                    mpImplTextObj->mnTextFlags &= ~PPT_TEXTOBJ_FLAGS_VERTICAL;
                            }
    sal_Bool                GetVertical() const
                            { return ( mpImplTextObj->mnTextFlags & PPT_TEXTOBJ_FLAGS_VERTICAL ) != 0; }

    const SfxItemSet*       GetBackground() const;

    PPTTextObj&             operator=( PPTTextObj& rTextObj );
};

class PPTConvertOCXControls : public SvxMSConvertOCXControls
{
    virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > & GetDrawPage();
    PptPageKind     ePageKind;
    const SdrPowerPointImport* mpPPTImporter;
    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > mxInStrm;
public :

    PPTConvertOCXControls( const SdrPowerPointImport* pPPTImporter, com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& rxInStrm, const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel, PptPageKind ePKind ) :
        SvxMSConvertOCXControls ( rxModel ),
        ePageKind               ( ePKind ),
        mpPPTImporter           ( pPPTImporter ),
        mxInStrm                ( rxInStrm )
    {};
    virtual sal_Bool ReadOCXStream( SotStorageRef& rSrc1,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShapeRef=0,
        sal_Bool bFloatingCtrl=sal_False );
    virtual sal_Bool InsertControl(
        const com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > &rFComp,
        const com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape > *pShape,
        sal_Bool bFloatingCtrl
    );
};

// Powerpoint Recordtypen
#define PPT_PST_Unknown                         0
#define PPT_PST_SubContainerCompleted           1
#define PPT_PST_IRRAtom                         2
#define PPT_PST_PSS                             3
#define PPT_PST_SubContainerException           4
#define PPT_PST_ClientSignal1                   6
#define PPT_PST_ClientSignal2                   7
#define PPT_PST_PowerPointStateInfoAtom         10
#define PPT_PST_Document                        1000
#define PPT_PST_DocumentAtom                    1001
#define PPT_PST_EndDocument                     1002
#define PPT_PST_SlidePersist                    1003
#define PPT_PST_SlideBase                       1004
#define PPT_PST_SlideBaseAtom                   1005
#define PPT_PST_Slide                           1006
#define PPT_PST_SlideAtom                       1007
#define PPT_PST_Notes                           1008
#define PPT_PST_NotesAtom                       1009
#define PPT_PST_Environment                     1010
#define PPT_PST_SlidePersistAtom                1011
#define PPT_PST_Scheme                          1012
#define PPT_PST_SchemeAtom                      1013
#define PPT_PST_DocViewInfo                     1014
#define PPT_PST_SslideLayoutAtom                1015
#define PPT_PST_MainMaster                      1016
#define PPT_PST_SSSlideInfoAtom                 1017
#define PPT_PST_SlideViewInfo                   1018
#define PPT_PST_GuideAtom                       1019
#define PPT_PST_ViewInfo                        1020
#define PPT_PST_ViewInfoAtom                    1021
#define PPT_PST_SlideViewInfoAtom               1022
#define PPT_PST_VBAInfo                         1023
#define PPT_PST_VBAInfoAtom                     1024
#define PPT_PST_SSDocInfoAtom                   1025
#define PPT_PST_Summary                         1026
#define PPT_PST_Texture                         1027
#define PPT_PST_VBASlideInfo                    1028
#define PPT_PST_VBASlideInfoAtom                1029
#define PPT_PST_DocRoutingSlip                  1030
#define PPT_PST_OutlineViewInfo                 1031
#define PPT_PST_SorterViewInfo                  1032
#define PPT_PST_ExObjList                       1033
#define PPT_PST_ExObjListAtom                   1034
#define PPT_PST_PPDrawingGroup                  1035
#define PPT_PST_PPDrawing                       1036
#define PPT_PST_NewlyAddedAtomByXP1037          1037
#define PPT_PST_NamedShows                      1040
#define PPT_PST_NamedShow                       1041
#define PPT_PST_NamedShowSlides                 1042
#define PPT_PST_List                            2000
#define PPT_PST_FontCollection                  2005
#define PPT_PST_ListPlaceholder                 2017
#define PPT_PST_BookmarkCollection              2019
#define PPT_PST_SoundCollection                 2020
#define PPT_PST_SoundCollAtom                   2021
#define PPT_PST_Sound                           2022
#define PPT_PST_SoundData                       2023
#define PPT_PST_BookmarkSeedAtom                2025
#define PPT_PST_GuideList                       2026
#define PPT_PST_RunArray                        2028
#define PPT_PST_RunArrayAtom                    2029
#define PPT_PST_ArrayElementAtom                2030
#define PPT_PST_Int4ArrayAtom                   2031
#define PPT_PST_ColorSchemeAtom                 2032

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_ExtendedBuGraContainer          2040    // consist of 4041
#define PPT_PST_ExtendedBuGraAtom               2041    // the instance of this atom indices the current graphic

#define PPT_PST_OEShape                         3008
#define PPT_PST_ExObjRefAtom                    3009
#define PPT_PST_OEPlaceholderAtom               3011
#define PPT_PST_GrColor                         3020
#define PPT_PST_GrectAtom                       3025
#define PPT_PST_GratioAtom                      3031
#define PPT_PST_Gscaling                        3032
#define PPT_PST_GpointAtom                      3034
#define PPT_PST_OEShapeAtom                     3035
#define PPT_PST_OutlineTextRefAtom              3998
#define PPT_PST_TextHeaderAtom                  3999
#define PPT_PST_TextCharsAtom                   4000
#define PPT_PST_StyleTextPropAtom               4001
#define PPT_PST_BaseTextPropAtom                4002
#define PPT_PST_TxMasterStyleAtom               4003
#define PPT_PST_TxCFStyleAtom                   4004
#define PPT_PST_TxPFStyleAtom                   4005
#define PPT_PST_TextRulerAtom                   4006
#define PPT_PST_TextBookmarkAtom                4007
#define PPT_PST_TextBytesAtom                   4008
#define PPT_PST_TxSIStyleAtom                   4009
#define PPT_PST_TextSpecInfoAtom                4010
#define PPT_PST_DefaultRulerAtom                4011

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_ExtendedParagraphAtom           4012
#define PPT_PST_ExtendedParagraphMasterAtom     4013
#define PPT_PST_ExtendedPresRuleContainer       4014    // consist of 4012, 4015,
#define PPT_PST_ExtendedParagraphHeaderAtom     4015    // the instance of this atom indices the current presobj
                                                        // the first sal_uInt32 in this atom indices the current slideId
#define PPT_PST_NewlyAddedAtom4016              4016

#define PPT_PST_FontEntityAtom                  4023
#define PPT_PST_FontEmbedData                   4024
#define PPT_PST_TypeFace                        4025
#define PPT_PST_CString                         4026
#define PPT_PST_ExternalObject                  4027
#define PPT_PST_MetaFile                        4033
#define PPT_PST_ExOleObj                        4034
#define PPT_PST_ExOleObjAtom                    4035
#define PPT_PST_ExPlainLinkAtom                 4036
#define PPT_PST_CorePict                        4037
#define PPT_PST_CorePictAtom                    4038
#define PPT_PST_ExPlainAtom                     4039
#define PPT_PST_SrKinsoku                       4040
#define PPT_PST_Handout                         4041
#define PPT_PST_ExEmbed                         4044
#define PPT_PST_ExEmbedAtom                     4045
#define PPT_PST_ExLink                          4046
#define PPT_PST_ExLinkAtom_old                  4047
#define PPT_PST_BookmarkEntityAtom              4048
#define PPT_PST_ExLinkAtom                      4049
#define PPT_PST_SrKinsokuAtom                   4050
#define PPT_PST_ExHyperlinkAtom                 4051
#define PPT_PST_ExPlain                         4053
#define PPT_PST_ExPlainLink                     4054
#define PPT_PST_ExHyperlink                     4055
#define PPT_PST_SlideNumberMCAtom               4056
#define PPT_PST_HeadersFooters                  4057
#define PPT_PST_HeadersFootersAtom              4058
#define PPT_PST_RecolorEntryAtom                4062
#define PPT_PST_TxInteractiveInfoAtom           4063
#define PPT_PST_EmFormatAtom                    4065
#define PPT_PST_CharFormatAtom                  4066
#define PPT_PST_ParaFormatAtom                  4067
#define PPT_PST_MasterText                      4068
#define PPT_PST_RecolorInfoAtom                 4071
#define PPT_PST_ExQuickTime                     4073
#define PPT_PST_ExQuickTimeMovie                4074
#define PPT_PST_ExQuickTimeMovieData            4075
#define PPT_PST_ExSubscription                  4076
#define PPT_PST_ExSubscriptionSection           4077
#define PPT_PST_ExControl                       4078
#define PPT_PST_ExControlAtom                   4091
#define PPT_PST_SlideListWithText               4080
#define PPT_PST_AnimationInfoAtom               4081
#define PPT_PST_InteractiveInfo                 4082
#define PPT_PST_InteractiveInfoAtom             4083
#define PPT_PST_SlideList                       4084
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
#define PPT_PST_ExMIDIAudio                     4109
#define PPT_PST_ExCDAudio                       4110
#define PPT_PST_ExWAVAudioEmbedded              4111
#define PPT_PST_ExWAVAudioLink                  4112
#define PPT_PST_ExOleObjStg                     4113
#define PPT_PST_ExCDAudioAtom                   4114
#define PPT_PST_ExWAVAudioEmbeddedAtom          4115
#define PPT_PST_AnimationInfo                   4116
#define PPT_PST_RTFDateTimeMCAtom               4117
#define PPT_PST_ProgTags                        5000
#define PPT_PST_ProgStringTag                   5001
#define PPT_PST_ProgBinaryTag                   5002
#define PPT_PST_BinaryTagData                   5003
#define PPT_PST_PrintOptions                    6000
#define PPT_PST_PersistPtrFullBlock             6001
#define PPT_PST_PersistPtrIncrementalBlock      6002

// these atoms first was seen in ppt2000 in a private Tag atom
#define PPT_PST_NewlyAddedAtomByPPT2000_6010    6010
#define PPT_PST_NewlyAddedAtomByPPT2000_6011    6011

#define PPT_PST_RulerIndentAtom                 10000
#define PPT_PST_GscalingAtom                    10001
#define PPT_PST_GrColorAtom                     10002
#define PPT_PST_GLPointAtom                     10003
#define PPT_PST_GlineAtom                       10004

#define PPT_PST_NewlyAddedAtomByXP11008         11008
#define PPT_PST_NewlyAddedAtomByXP11010         11010
#define PPT_PST_Comment10                       12000
#define PPT_PST_CommentAtom10                   12001
#define PPT_PST_NewlyAddedAtomByXP12004         12004
#define PPT_PST_NewlyAddedAtomByXP12010         12010
#define PPT_PST_NewlyAddedAtomByXP12011         12011
#define PPT_PST_NewlyAddedAtomByXP14001         14001

// Attribute fuer PptTextStyleSheet
#define PPT_ParaAttr_BulletOn       0   //00000001
#define PPT_ParaAttr_BuHardFont     1   //00000002
#define PPT_ParaAttr_BuHardColor    2   //00000004
#define PPT_ParaAttr_BuHardHeight   3   //00000008
#define PPT_ParaAttr_BulletFont     4   //00000010
#define PPT_ParaAttr_BulletColor    5   //00000020
#define PPT_ParaAttr_BulletHeight   6   //00000040
#define PPT_ParaAttr_BulletChar     7   //00000080
#define PPT_ParaAttr_DontKnow1      8   //00000100
#define PPT_ParaAttr_DontKnow2      9   //00000200
#define PPT_ParaAttr_DontKnow3      10  //00000400
#define PPT_ParaAttr_Adjust         11  //00000800 0000=Left, 0001=Center, 0002=Right, 0003=Block
#define PPT_ParaAttr_LineFeed       12  //00001000
#define PPT_ParaAttr_UpperDist      13  //00002000 ist bei Textframes default immer auf 0032 gesetzt
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
#define PPT_CharAttr_ResetNumbering     10  //00000400
#define PPT_CharAttr_EnableNumbering1   11  //00000800
#define PPT_CharAttr_EnableNumbering2   12  //00001000
#define PPT_CharAttr_Font               16  //00010000
#define PPT_CharAttr_AsianOrComplexFont 21  //00200000
#define PPT_CharAttr_ANSITypeface       22  //00400000
#define PPT_CharAttr_Symbol             23  //00800000
#define PPT_CharAttr_FontHeight         17  //00020000
#define PPT_CharAttr_FontColor          18  //00040000
#define PPT_CharAttr_Escapement         19  //00080000

// Werte fuer PptSlideLayoutAtom.eLayout
#define PPT_LAYOUT_TITLESLIDE               0   // The slide is a title slide
#define PPT_LAYOUT_TITLEANDBODYSLIDE        1   // Title and body slide
#define PPT_LAYOUT_TITLEMASTERSLIDE         2   // Title master slide
#define PPT_LAYOUT_MASTERSLIDE              3   // Master slide layout
#define PPT_LAYOUT_MASTERNOTES              4   // Master notes layout
#define PPT_LAYOUT_NOTESTITLEBODY           5   // Notes title/body layout
#define PPT_LAYOUT_HANDOUTLAYOUT            6   // Handout layout, therefore it doesn't have placeholders except header, footer, and date
#define PPT_LAYOUT_ONLYTITLE                7   // Only title placeholder
#define PPT_LAYOUT_2COLUMNSANDTITLE         8   // Body of the slide has 2 columns and a title
#define PPT_LAYOUT_2ROWSANDTITLE            9   // Slide's body has 2 rows and a title
#define PPT_LAYOUT_RIGHTCOLUMN2ROWS         10  // Body contains 2 columns, right column has 2 rows
#define PPT_LAYOUT_LEFTCOLUMN2ROWS          11  // Body contains 2 columns, left column has 2 rows
#define PPT_LAYOUT_BOTTOMROW2COLUMNS        12  // Body contains 2 rows, bottom row has 2 columns
#define PPT_LAYOUT_TOPROW2COLUMN            13  // Body contains 2 rows, top row has 2 columns
#define PPT_LAYOUT_4OBJECTS                 14  // 4 objects
#define PPT_LAYOUT_BIGOBJECT                15  // Big object
#define PPT_LAYOUT_BLANCSLIDE               16  // Blank slide
#define PPT_LAYOUT_TITLERIGHTBODYLEFT       17  // Vertical title on the right, body on the left
#define PPT_LAYOUT_TITLERIGHT2BODIESLEFT    18  // Vertical title on the right, body on the left split into 2 rows

// the following table describes the placeholder id's (values from reality followed by values taken from the documentation)
#define PPT_PLACEHOLDER_NONE                    0   //  0 None
#define PPT_PLACEHOLDER_MASTERTITLE             1   //  1 Master title
#define PPT_PLACEHOLDER_MASTERBODY              2   //  2 Master body
#define PPT_PLACEHOLDER_MASTERCENTEREDTITLE     3   //  3 Master centered title
#define PPT_PLACEHOLDER_MASTERSUBTITLE          4   // 10 Master subtitle
#define PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE   5   //  4 Master notes slide image
#define PPT_PLACEHOLDER_MASTERNOTESBODYIMAGE    6   //  5 Master notes body image
#define PPT_PLACEHOLDER_MASTERDATE              7   //  6 Master date
#define PPT_PLACEHOLDER_MASTERSLIDENUMBER       8   //  7 Master slide number
#define PPT_PLACEHOLDER_MASTERFOOTER            9   //  8 Master footer
#define PPT_PLACEHOLDER_MASTERHEADER            10  //  9 Master header
#define PPT_PLACEHOLDER_GENERICTEXTOBJECT           // 11 Generic text object
#define PPT_PLACEHOLDER_TITLE                   13  // 12 Title
#define PPT_PLACEHOLDER_BODY                    14  // 13 Body
#define PPT_PLACEHOLDER_NOTESBODY               12  // 14 Notes body
#define PPT_PLACEHOLDER_CENTEREDTITLE           15  // 15 Centered title
#define PPT_PLACEHOLDER_SUBTITLE                16  // 16 Subtitle
#define PPT_PLACEHOLDER_VERTICALTEXTTITLE       17  // 17 Vertical text title
#define PPT_PLACEHOLDER_VERTICALTEXTBODY        18  // 18 Vertical text body
#define PPT_PLACEHOLDER_NOTESSLIDEIMAGE         11  // 19 Notes slide image
#define PPT_PLACEHOLDER_OBJECT                  19  // 20 Object (no matter the size)
#define PPT_PLACEHOLDER_GRAPH                   20  // 21 Graph
#define PPT_PLACEHOLDER_TABLE                   21  // 22 Table
#define PPT_PLACEHOLDER_CLIPART                 22  // 23 Clip Art
#define PPT_PLACEHOLDER_ORGANISZATIONCHART      23  // 24 Organization Chart
#define PPT_PLACEHOLDER_MEDIACLIP               24  // 25 Media Clip

#endif //_SVDFPPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
