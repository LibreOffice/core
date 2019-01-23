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

#ifndef INCLUDED_FILTER_MSFILTER_MSDFFIMP_HXX
#define INCLUDED_FILTER_MSFILTER_MSDFFIMP_HXX

#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <unordered_map>

#include <boost/optional.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/stl_types.hxx>
#include <filter/msfilter/dffpropset.hxx>
#include <filter/msfilter/dffrecordheader.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/msfilterdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/msdffdef.hxx>
#include <vcl/errcode.hxx>
#include <tools/gen.hxx>
#include <tools/ref.hxx>
#include <tools/solar.h>
#include <vcl/graph.hxx>

class Color;
class GDIMetaFile;
class SotStorage;
class SvStream;
class SdrObject;
class SdrOle2Obj;
namespace tools {
    class Polygon;
}
class SdrModel;
class SwFlyFrameFormat;

struct SvxMSDffBLIPInfos;
struct SvxMSDffShapeInfo;
struct SvxMSDffShapeOrder;

class SvxMSDffManager;
class SfxItemSet;
struct DffObjData;
class SvGlobalName;

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace embed { class XEmbeddedObject; }
    namespace embed { class XStorage; }
} } }

class MSFILTER_DLLPUBLIC SvxMSDffClientData
{
public:
    virtual void NotifyFreeObj(SdrObject* pObj) = 0;
    virtual ~SvxMSDffClientData() {}
};

class MSFILTER_DLLPUBLIC DffPropertyReader : public DffPropSet
{
    const SvxMSDffManager&       rManager;
    std::unique_ptr<DffPropSet>  pDefaultPropSet;

    void ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const;
    void CheckAndCorrectExcelTextRotation( SvStream& rIn, SfxItemSet& rSet, DffObjData const & rObjData ) const;
    void ApplyCustomShapeGeometryAttributes( SvStream& rIn,
                                             SfxItemSet& rSet,
                                             const DffObjData& rObjData ) const;
    void ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const; // #i28269#
    void ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;

public:
    sal_Int32 mnFix16Angle;
    bool      mbRotateGranientFillWithAngle;

    explicit DffPropertyReader( const SvxMSDffManager& rManager );
    ~DffPropertyReader();

    DffPropertyReader& operator=( DffPropertyReader const & ) = delete; // MSVC2015 workaround
    DffPropertyReader( DffPropertyReader const & ) = delete; // MSVC2015 workaround

    static sal_Int32 Fix16ToAngle( sal_Int32 nAngle );

#ifdef DBG_CUSTOMSHAPE
    void ReadPropSet( SvStream& rIn, SvxMSDffClientData* pClientData, sal_uInt32 nShapeType = 0 ) const;
#else
    void ReadPropSet( SvStream& rIn, SvxMSDffClientData* pClientData ) const;
#endif

    void SetDefaultPropSet( SvStream& rIn, sal_uInt32 nOffDgg ) const;
    void ApplyAttributes( SvStream& rIn, SfxItemSet& rSet ) const;
    void ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, DffObjData const & rObjData ) const;
    void ImportGradientColor( SfxItemSet& aSet, MSO_FillType eMSO_FillType, double dTrans, double dBackTrans ) const;
};

#define COL_DEFAULT ::Color( 0xFA, 0xFB, 0xFC )

typedef ::std::map< sal_Int32, SdrObject* > SvxMSDffShapeIdContainer;

#define SVEXT_PERSIST_STREAM "\002OlePres000"

/// the following will be sorted by the order of their appearance:
typedef std::vector<std::unique_ptr<SvxMSDffShapeOrder>> SvxMSDffShapeOrders;

struct MSFILTER_DLLPUBLIC CompareSvxMSDffShapeInfoById
{
    bool operator()(std::shared_ptr<SvxMSDffShapeInfo> const& lhs,
                    std::shared_ptr<SvxMSDffShapeInfo> const& rhs) const;
};
struct MSFILTER_DLLPUBLIC CompareSvxMSDffShapeInfoByTxBxComp
{
    bool operator()(std::shared_ptr<SvxMSDffShapeInfo> const& lhs,
                    std::shared_ptr<SvxMSDffShapeInfo> const& rhs) const;
};

// the following will be sorted explicitly:
typedef ::std::set< std::shared_ptr<SvxMSDffShapeInfo>,
            CompareSvxMSDffShapeInfoById > SvxMSDffShapeInfos_ById;
typedef ::std::multiset< std::shared_ptr<SvxMSDffShapeInfo>,
            CompareSvxMSDffShapeInfoByTxBxComp> SvxMSDffShapeInfos_ByTxBxComp;

#define SVXMSDFF_SETTINGS_CROP_BITMAPS      1
#define SVXMSDFF_SETTINGS_IMPORT_PPT        2
#define SVXMSDFF_SETTINGS_IMPORT_EXCEL      4

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this is stored in
#define OLE_MATHTYPE_2_STARMATH             0x0001
#define OLE_WINWORD_2_STARWRITER            0x0002
#define OLE_EXCEL_2_STARCALC                0x0004
#define OLE_POWERPOINT_2_STARIMPRESS        0x0008

struct SvxMSDffConnectorRule
{
    sal_uInt32  nShapeA;   ///< SPID of shape A
    sal_uInt32  nShapeB;   ///< SPID of shape B
    sal_uInt32  nShapeC;   ///< SPID of connector shape
    sal_uInt32  ncptiA;    ///< Connection site Index of shape A
    sal_uInt32  ncptiB;    ///< Connection site Index of shape B
    ShapeFlag   nSpFlagsA; ///< SpFlags of shape A (the original mirror flags
                           ///< must be known when solving the Solver Container)
    ShapeFlag   nSpFlagsB; ///< SpFlags of shape B

    SdrObject*  pAObj;     ///< pPtr of object (corresponding to shape A)
    SdrObject*  pBObj;     ///< pPtr of object (corresponding to shape B)
    SdrObject*  pCObj;     ///< pPtr of connector object

    SvxMSDffConnectorRule()
        : nShapeA(0)
        , nShapeB(0)
        , nShapeC(0)
        , ncptiA(0)
        , ncptiB(0)
        , nSpFlagsA( ShapeFlag::NONE )
        , nSpFlagsB( ShapeFlag::NONE )
        , pAObj( nullptr )
        , pBObj( nullptr )
        , pCObj( nullptr )
        {};

};

struct MSFILTER_DLLPUBLIC SvxMSDffSolverContainer
{
    ::std::vector< std::unique_ptr<SvxMSDffConnectorRule> > aCList;

    SvxMSDffSolverContainer();
    ~SvxMSDffSolverContainer();

    SvxMSDffSolverContainer& operator=( SvxMSDffSolverContainer const & ) = delete; // MSVC2015 workaround
    SvxMSDffSolverContainer( SvxMSDffSolverContainer const & ) = delete; // MSVC2015 workaround

    MSFILTER_DLLPUBLIC friend SvStream& ReadSvxMSDffSolverContainer( SvStream& rIn, SvxMSDffSolverContainer& rAtom );
};

struct FIDCL
{
    sal_uInt32  dgid;       ///< DG owning the SPIDs in this cluster
};

/// provided by SvxMSDffManager for each shape in a group
struct MSDffTxId
{
    sal_uInt16 nTxBxS;
    sal_uInt16 nSequence;
    MSDffTxId()
        : nTxBxS( 0 ), nSequence( 0 ) {}
    MSDffTxId( const MSDffTxId& rCopy )
        : nTxBxS( rCopy.nTxBxS ), nSequence( rCopy.nSequence ) {}
};

struct MSFILTER_DLLPUBLIC SvxMSDffImportRec
{
    static const int RELTO_DEFAULT = 2;

    SdrObject*      pObj;
    std::unique_ptr<tools::Polygon>
                    pWrapPolygon;
    std::unique_ptr<char[]>
                    pClientAnchorBuffer;
    sal_uInt32      nClientAnchorLen;
    std::unique_ptr<char[]>
                    pClientDataBuffer;
    sal_uInt32      nClientDataLen;
    sal_uInt32      nXAlign;
    boost::optional<sal_uInt32> nXRelTo;
    sal_uInt32      nYAlign;
    boost::optional<sal_uInt32> nYRelTo;
    sal_uInt32      nLayoutInTableCell;
    ShapeFlag       nFlags;
    sal_Int32       nDxTextLeft;    ///< distance of text box from surrounding shape
    sal_Int32       nDyTextTop;
    sal_Int32       nDxTextRight;
    sal_Int32       nDyTextBottom;
    sal_Int32       nDxWrapDistLeft;
    sal_Int32       nDyWrapDistTop;
    sal_Int32       nDxWrapDistRight;
    sal_Int32       nDyWrapDistBottom;
    sal_Int32       nCropFromTop;
    sal_Int32       nCropFromBottom;
    sal_Int32       nCropFromLeft;
    sal_Int32       nCropFromRight;
    MSDffTxId       aTextId;        ///< identifier for text boxes
    sal_uLong       nNextShapeId;   ///< for linked text boxes
    sal_uLong       nShapeId;
    MSO_SPT         eShapeType;
    MSO_LineStyle   eLineStyle;     ///< border types
    MSO_LineDashing eLineDashing;
    bool            bDrawHell       :1;
    bool            bHidden         :1;
    bool            bReplaceByFly   :1;
    bool            bVFlip          :1;
    bool            bHFlip          :1;
    bool            bAutoWidth      :1;
    int             relativeHorizontalWidth; ///< in 0.1% or -1 for none
    bool            isHorizontalRule;

    SvxMSDffImportRec();
    SvxMSDffImportRec(const SvxMSDffImportRec& rCopy);
    ~SvxMSDffImportRec();

    bool operator<( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId < rEntry.nShapeId;  }

private:
    SvxMSDffImportRec &operator=(const SvxMSDffImportRec&) = delete;
};

/** block of parameters for import/export for a single call of
    ImportObjAtCurrentStreamPos() */
class MSFILTER_DLLPUBLIC SvxMSDffImportData : public SvxMSDffClientData
{
private:
    /** list of all SvxMSDffImportRec instances of/for a group */
    typedef std::set<std::unique_ptr<SvxMSDffImportRec>,
            comphelper::UniquePtrValueLess<SvxMSDffImportRec>> MSDffImportRecords;
    MSDffImportRecords  m_Records;  ///< Shape pointer, Shape ids and private data
    std::map<const SdrObject*, SvxMSDffImportRec*> m_ObjToRecMap;
public:
    tools::Rectangle    aParentRect;///< Rectangle of the surrounding groups,
                                    ///< which might have been provided externally

    explicit SvxMSDffImportData(const tools::Rectangle& rParentRect);
    SvxMSDffImportData& operator=( SvxMSDffImportData const & ) = delete; // MSVC2015 workaround
    SvxMSDffImportData( SvxMSDffImportData const & ) = delete; // MSVC2015 workaround
    virtual ~SvxMSDffImportData() override;
    bool empty() const { return m_Records.empty(); }
    void insert(std::unique_ptr<SvxMSDffImportRec> pImpRec);
    void unmap(const SdrObject* pObj) { m_ObjToRecMap.erase(pObj); }
    size_t size() const { return m_Records.size(); }
    SvxMSDffImportRec* find(const SdrObject* pObj);
    MSDffImportRecords::const_iterator begin() const { return m_Records.begin();  }
    MSDffImportRecords::const_iterator end() const { return m_Records.end();  }
    virtual void NotifyFreeObj(SdrObject* pObj) override;
};

struct DffObjData
{
    const DffRecordHeader&  rSpHd;

    tools::Rectangle   aBoundRect;
    tools::Rectangle   aChildAnchor;

    sal_uInt32  nShapeId;
    ShapeFlag   nSpFlags;
    MSO_SPT     eShapeType;

    bool        bShapeType     : 1;
    bool        bClientAnchor  : 1;
    bool        bClientData    : 1;
    bool        bChildAnchor   : 1;
    bool        bOpt           : 1;
    bool        bOpt2          : 1;
    bool        bRotateTextWithShape : 1;
    bool        bPageAnchor;
    int         nCalledByGroup;

    DffObjData( const DffRecordHeader& rObjHd,
                const tools::Rectangle& rBoundRect,
                int   nClByGroup ) :
        rSpHd( rObjHd ),
        aBoundRect( rBoundRect ),
        nShapeId( 0 ),
        nSpFlags( ShapeFlag::NONE ),
        eShapeType( mso_sptNil ),
        bShapeType( false ),
        bClientAnchor( false ),
        bClientData( false ),
        bChildAnchor( false ),
        bOpt( false ),
        bOpt2( false ),
        bRotateTextWithShape( true ),
        bPageAnchor( true ),
        nCalledByGroup( nClByGroup ){}

    // Clone a DffObjData _o_ by replacing its rSpHd with a shared_ptr to another one
    DffObjData( const std::shared_ptr<DffRecordHeader>& rObjHd, const DffObjData& o) :
        rSpHd( *rObjHd ),
        aBoundRect( o.aBoundRect ),
        nShapeId( o.nShapeId ),
        nSpFlags( o.nSpFlags ),
        eShapeType( o.eShapeType ),
        bShapeType( o.bShapeType ),
        bClientAnchor( o.bClientAnchor ),
        bClientData( o.bClientData ),
        bChildAnchor( o.bChildAnchor ),
        bOpt( o.bOpt ),
        bOpt2( o.bOpt2 ),
        bRotateTextWithShape( o.bRotateTextWithShape ),
        bPageAnchor( o.bPageAnchor ),
        nCalledByGroup( o.nCalledByGroup ){}
};

#define DFF_RECORD_MANAGER_BUF_SIZE         64

struct DffRecordList
{
    sal_uInt32          nCount;
    sal_uInt32          nCurrent;
    DffRecordList*      pPrev;
    std::unique_ptr<DffRecordList>
                        pNext;

    DffRecordHeader     mHd[ DFF_RECORD_MANAGER_BUF_SIZE ];

    explicit DffRecordList( DffRecordList* pList );
    ~DffRecordList();
};

enum DffSeekToContentMode
{
    SEEK_FROM_BEGINNING,
    SEEK_FROM_CURRENT,
    SEEK_FROM_CURRENT_AND_RESTART
};

class MSFILTER_DLLPUBLIC DffRecordManager : public DffRecordList
{
public:
        DffRecordList*   pCList;

        void             Clear();
        void             Consume( SvStream& rIn,
                                  sal_uInt32 nStOfs = 0 );

        bool             SeekToContent( SvStream& rIn,
                                        sal_uInt16 nRecType,
                                        DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );
        DffRecordHeader* GetRecordHeader( sal_uInt16 nRecType,
                                          DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );

        DffRecordManager();
        explicit DffRecordManager( SvStream& rIn );

        DffRecordHeader* Current();
        DffRecordHeader* First();
        DffRecordHeader* Next();
        DffRecordHeader* Prev();
        DffRecordHeader* Last();
};

/** abstract base class for Escher imports

    Purpose: access to objects in Drawing File Format

    Note: The method ProcessUserDefinedRecord() _has_ to be implemented in the
          inheriting class(es) that is/are eventually used for PowerPoint, Word,
          or Excel importing.

    Status: Currently only access to BLIPs (will be extended later)
*/
class MSFILTER_DLLPUBLIC SvxMSDffManager : public DffPropertyReader
{
    std::unique_ptr<SvxMSDffBLIPInfos>      m_pBLIPInfos;
    std::unique_ptr<SvxMSDffShapeInfos_ByTxBxComp> m_xShapeInfosByTxBxComp;
    std::unique_ptr<SvxMSDffShapeInfos_ById> m_xShapeInfosById;
    SvxMSDffShapeOrders     m_aShapeOrders;
    sal_uInt32              nOffsDgg;
    sal_uInt16              nBLIPCount;
    ShapeFlag               nGroupShapeFlags;

    void CheckTxBxStoryChain();
    void GetFidclData(sal_uInt32 nOffsDgg);

protected:
    typedef std::map<sal_uInt32, sal_uInt32> OffsetMap;

    OUString        maBaseURL;
    sal_uInt32      mnIdClusters;       // while only knowing the shapeid
    std::vector<FIDCL> maFidcls;
    OffsetMap       maDgOffsetTable;    ///< array of fileoffsets

    /** When importing Excel files, cell anchor computations for non-page-anchored
        groups must be done after all nested groups have been processed; for each open
        group, the pending data is stored here. The storage also holds a shared_ptr to
        the DffObjData ow DffRecordHeader to avoid it going out of scope except when needed
     */
    std::vector< std::pair<DffObjData, std::shared_ptr<DffRecordHeader> > > maPendingGroupData;

    friend class DffPropertyReader;

    SvStream&       rStCtrl;
    SvStream*       pStData;
    SvStream*       pStData2;
    SdrModel*       pSdrModel;

    long            nMapMul;
    long            nMapDiv;
    long            nMapXOfs;
    long            nMapYOfs;
    long            nEmuMul;
    long            nEmuDiv;
    long            nPntMul;
    long            nPntDiv;
    bool            bNeedMap;
    sal_uInt32      nSvxMSDffSettings;
    sal_uInt32      nSvxMSDffOLEConvFlags;

    /** stores a reference to an imported SdrObject
        with its shape id if it has one */
    SvxMSDffShapeIdContainer    maShapeIdContainer;

    void GetCtrlData(sal_uInt32 nOffsDgg);
    void GetDrawingGroupContainerData( SvStream& rSt,
                                       sal_uLong nLenDgg );
    // Add internal drawing container id as parameter to the sub methods of
    // reading the control information about the drawing objects.
    // The drawing container id is used to distinguish the text ids of drawing
    // objects in different drawing containers.
    void GetDrawingContainerData( SvStream& rSt,
                                  sal_uLong nLenDg,
                                  sal_uInt16 nDrawingContainerId );
    bool GetShapeGroupContainerData( SvStream& rSt,
                                     sal_uLong nLenShapeGroupCont,
                                     bool bPatriarch,
                                     sal_uInt16 nDrawingContainerId );
    bool GetShapeContainerData( SvStream& rSt,
                                sal_uLong nLenShapeCont,
                                sal_uLong nPosGroup,
                                sal_uInt16 nDrawingContainerId );

    SdrObject* ImportGraphic( SvStream&, SfxItemSet&, const DffObjData& );
    // #i32596# - pass <nCalledByGroup> to method
    // Needed in Writer's Microsoft Word import to avoid import of OLE objects
    // inside groups. Instead a graphic object is created.
    virtual SdrObject* ImportOLE( sal_uInt32 nOLEId,
                                  const Graphic& rGraf,
                                  const tools::Rectangle& rBoundRect,
                                  const tools::Rectangle& rVisArea,
                                  const int _nCalledByGroup ) const;
    static css::uno::Reference < css::embed::XEmbeddedObject > CheckForConvertToSOObj(
                sal_uInt32 nConvertFlags, SotStorage& rSrcStg,
                const css::uno::Reference < css::embed::XStorage >& xDestStg,
                const Graphic& rGrf,
                const tools::Rectangle& rVisArea,
                OUString const& rBaseURL);

// the following methods need to be overridden for Excel imports
    static void ProcessClientAnchor( SvStream& rStData,
                                          sal_uInt32 nDatLen,
                                          std::unique_ptr<char[]>& rpBuff,
                                          sal_uInt32& rBuffLen );
    virtual void ProcessClientAnchor2( SvStream& rStData,
                                       DffRecordHeader& rHd,
                                       SvxMSDffClientData& rData, DffObjData& );
    static void ProcessClientData( SvStream& rStData,
                                        sal_uInt32 nDatLen,
                                        std::unique_ptr<char[]>& rpBuff,
                                        sal_uInt32& rBuffLen );
    virtual SdrObject* ProcessObj( SvStream& rSt,
                                   DffObjData& rData,
                                   SvxMSDffClientData& rClientData,
                                   tools::Rectangle& rTextRect,
                                   SdrObject* pObj);
    virtual void NotifyFreeObj(SvxMSDffClientData& rData, SdrObject* pObj);
    void FreeObj(SvxMSDffClientData& rData, SdrObject* pObj);


    /** Object finalization, used by the Excel filter to correctly
        compute the object anchoring after nested objects have been imported.
    */
    virtual SdrObject* FinalizeObj(DffObjData& rData,
                                   SdrObject* pObj);

    virtual bool GetColorFromPalette(sal_uInt16 nNum, Color& rColor) const;

    // Fontwork objects use a new implementation of ReadObjText because the old
    // one does not properly import multiple paragraphs.
    static void ReadObjText( const OUString& rText, SdrObject* pObj );

// the following method needs to be overridden for the import of OLE objects
    virtual bool GetOLEStorageName( sal_uInt32 nOLEId,
                                      OUString& rStorageName,
                                      tools::SvRef<SotStorage>& rSrcStorage,
                                      css::uno::Reference < css::embed::XStorage >& xDestStg
                                    ) const;

    /** Prevent that (rounded) rectangles with wrapped text will always be
        converted into SdrRectObj( OBJ_TEXT ).

        @return true means "conversion".
    */
    virtual bool ShapeHasText(sal_uLong nShapeId, sal_uLong nFilePos) const;

public:
    std::unique_ptr<DffPropertyReader> pSecPropSet;
    std::unordered_map<sal_uInt32, Graphic> aEscherBlipCache;

    DffRecordManager    maShapeRecords;
    Color               mnDefaultColor;

    bool                mbSkipImages;

    Color MSO_TEXT_CLR_ToColor( sal_uInt32 nColorCode ) const;
    Color MSO_CLR_ToColor( sal_uInt32 nColorCode,
                           sal_uInt16 nContextProperty = DFF_Prop_lineColor ) const;
    virtual bool SeekToShape( SvStream& rSt,
                                  SvxMSDffClientData* pClientData,
                                  sal_uInt32 nId ) const;
    static bool SeekToRec( SvStream& rSt,
                    sal_uInt16 nRecId,
                    sal_uLong nMaxFilePos,
                    DffRecordHeader* pRecHd = nullptr,
                    sal_uLong nSkipCount = 0 );
    bool SeekToRec2( sal_uInt16 nRecId1,
                     sal_uInt16 nRecId2,
                     sal_uLong nMaxFilePos ) const;

    static OUString MSDFFReadZString( SvStream& rIn,
                                           sal_uInt32 nMaxLen,
                                           bool bUniCode);

    [[nodiscard]] static bool ReadCommonRecordHeader( SvStream& rSt,
                                        sal_uInt8& rVer,
                                        sal_uInt16& rInst,
                                        sal_uInt16& rFbt,
                                        sal_uInt32& rLength);

// TODO: provide proper documentation here
    /** constructor

        sets nBLIPCount

        @param rStCtrl             Management stream with containers, FBSE
                                   objects and shapes (mandatory; stream needs
                                   to be open already)
        @param rBaseURL            ???
        @param nOffsDgg            offset in rStrCtrl; beginning of the drawing
                                   group container
        @param pStData             data stream in that the BLIPs are stored (if
                                   NULL it is assumed that all BLIPs are also in
                                   the rStCtrl; stream needs to be open already)
        @param pSdrModel_          ??? (can be empty during Ctor call but needs
                                   to be set via SetModel() later in that case!)
        @param nApplicationScale   ???
        @param mnDefaultColor_     ???
        @param nDefaultFontHeight_ ???
        @param pStData2_           ???
        @param bSkipImages         skipping images for text extraction/indexing
    */
    SvxMSDffManager( SvStream& rStCtrl,
                     const OUString& rBaseURL,
                     sal_uInt32 nOffsDgg,
                     SvStream* pStData,
                     SdrModel* pSdrModel_,
                     long      nApplicationScale,
                     Color     mnDefaultColor_,
                     SvStream* pStData2_            =  nullptr,
                     bool bSkipImages               =  false );

    // in PPT the parameters DGGContainerOffset and PicStream are provided by an
    // init method
    SvxMSDffManager( SvStream& rStCtrl, const OUString& rBaseURL );
    void InitSvxMSDffManager( sal_uInt32 nOffsDgg_,
                              SvStream* pStData_,
                              sal_uInt32 nSvxMSDffOLEConvFlags);
    void SetDgContainer( SvStream& rSt );

    virtual ~SvxMSDffManager();

    sal_uInt32  GetSvxMSDffSettings() const { return nSvxMSDffSettings; };
    void    SetSvxMSDffSettings( sal_uInt32 nSettings ) { nSvxMSDffSettings = nSettings; };

    static bool         MakeContentStream( SotStorage * pStor, const GDIMetaFile & );
    static void         ReadObjText( SvStream& rStream, SdrObject* pObj );
    static bool         ConvertToOle2( SvStream& rStm,
                                       sal_uInt32 nLen,
                                       const GDIMetaFile*,
                                       const tools::SvRef<SotStorage> & rDest );

    void SetModel(SdrModel* pModel, long nApplicationScale);
    SdrModel*  GetModel() const { return pSdrModel; }
    void Scale(sal_Int32& rVal) const;
    void Scale(Point& rPos) const;
    void Scale(Size& rSiz) const;
    void ScaleEmu(sal_Int32& rVal) const;
    sal_uInt32 ScalePt( sal_uInt32 nPt ) const;
    sal_Int32 ScalePoint( sal_Int32 nVal ) const;

// TODO: provide proper documentation here
    /** Request for a specific BLIP.

        @param[in] nIdx   number of the requested BLIP (mandatory)
        @param[out] rData already converted data (insert directly as graphics
                          into our documents)
        @param pVisArea   ???

        @return true if successful, false otherwise
    */
    bool GetBLIP( sal_uLong nIdx, Graphic& rData, tools::Rectangle* pVisArea = nullptr );

// TODO: provide proper documentation here
    /** read a BLIP out of a already positioned stream

        @param[in] rBLIPStream already positioned stream (mandatory)
        @param[out] rData      already converted data (insert directly as
                               graphics into our documents)
        @param pVisArea        ???

        @return true if successful, false otherwise
    */
    static bool GetBLIPDirect(SvStream& rBLIPStream, Graphic& rData, tools::Rectangle* pVisArea = nullptr );

    bool GetShape(sal_uLong nId, SdrObject*& rpData, SvxMSDffImportData& rData);

    SdrObject* ImportObj( SvStream& rSt,
                          SvxMSDffClientData& rData,
                          tools::Rectangle& rClientRect,
                          const tools::Rectangle& rGlobalChildRect,
                          int nCalledByGroup,
                          sal_Int32* pShapeId);
    SdrObject* ImportGroup( const DffRecordHeader& rHd,
                            SvStream& rSt,
                            SvxMSDffClientData& rData,
                            tools::Rectangle& rClientRect,
                            const tools::Rectangle& rGlobalChildRect,
                            int nCalledByGroup,
                            sal_Int32* pShapeId );
    SdrObject* ImportShape( const DffRecordHeader& rHd,
                            SvStream& rSt,
                            SvxMSDffClientData& rData,
                            tools::Rectangle& rClientRect,
                            const tools::Rectangle& rGlobalChildRect,
                            int nCalledByGroup,
                            sal_Int32* pShapeId);

    tools::Rectangle GetGlobalChildAnchor( const DffRecordHeader& rHd,
                                    SvStream& rSt,
                                    tools::Rectangle& aClientRect );
    void GetGroupAnchors( const DffRecordHeader& rHd,
                          SvStream& rSt,
                          tools::Rectangle& rGroupClientAnchor,
                          tools::Rectangle& rGroupChildAnchor,
                          const tools::Rectangle& rClientRect,
                          const tools::Rectangle& rGlobalChildRect );

    const SvxMSDffShapeInfos_ById* GetShapeInfos() const
        { return m_xShapeInfosById.get(); }

    const SvxMSDffShapeOrders* GetShapeOrders() const
        { return &m_aShapeOrders; }

    void StoreShapeOrder(sal_uLong      nId,
                         sal_uLong      nTxBx,
                         SdrObject*     pObject,
                         SwFlyFrameFormat*   pFly = nullptr) const;

    void ExchangeInShapeOrder(SdrObject const * pOldObject,
                              sal_uLong     nTxBx,
                              SdrObject*    pObject) const;

    void RemoveFromShapeOrder( SdrObject const * pObject ) const;

    static SdrOle2Obj* CreateSdrOLEFromStorage(
        SdrModel& rSdrModel,
        const OUString& rStorageName,
        tools::SvRef<SotStorage> const & rSrcStorage,
        const css::uno::Reference < css::embed::XStorage >& xDestStg,
        const Graphic& rGraf,
        const tools::Rectangle& rBoundRect,
        const tools::Rectangle& rVisArea,
        SvStream* pDataStrrm,
        ErrCode& rError,
        sal_uInt32 nConvertFlags,
        sal_Int64 nAspect,
        OUString const& rBaseURL);

    /** Create connections between shapes.
        This method should be called after a page is imported.

        @param rSolver contains necessary data that is collected during the
                       import of each shape
    */
    static void SolveSolver( const SvxMSDffSolverContainer& rSolver );

    static bool SetPropValue(
        const css::uno::Any& rAny,
        const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
        const OUString& rPropertyName
    );

    /// Determines an ODF filter name (if there is one) for aGlobName.
    static OUString GetFilterNameFromClassID(const SvGlobalName& aGlobName);
    /// Extracts ODF data from rSrcStg.
    static void ExtractOwnStream(SotStorage& rSrcStg, SvMemoryStream& rMemStream);

    void insertShapeId( sal_Int32 nShapeId, SdrObject* pShape );
    void removeShapeId( SdrObject const * pShape );
    SdrObject* getShapeForId( sal_Int32 nShapeId );
};

struct SvxMSDffShapeInfo
{
    sal_uInt32 nShapeId; ///< shape id, used in PLCF SPA and in mso_fbtSp (FSP)
    sal_uLong nFilePos;  ///< offset of the shape in control stream for
                         ///< potential later access to it
    sal_uInt32 nTxBxComp;

    bool bReplaceByFly  :1; ///< shape can be replaced by a frame in Writer

    explicit SvxMSDffShapeInfo(sal_uLong nFPos, sal_uInt32 nId=0, // sal_uLong nBIdx=0,
                      sal_uInt16 nSeqId=0, sal_uInt16 nBoxId=0):
        nShapeId( nId ),
        nFilePos( nFPos ),
        nTxBxComp( (nSeqId << 16) + nBoxId )
        {
            bReplaceByFly   = false;
        }
    SvxMSDffShapeInfo(const SvxMSDffShapeInfo& rInfo):
        nShapeId( rInfo.nShapeId ),
        nFilePos( rInfo.nFilePos ),
        nTxBxComp( rInfo.nTxBxComp ),
        bReplaceByFly( rInfo.bReplaceByFly )
        {
        }
};


struct SvxMSDffShapeOrder
{
    sal_uLong nShapeId;  ///< shape id used in PLCF SPA and in mso_fbtSp (FSP)
    sal_uLong nTxBxComp; ///< chain or box number in the Text-Box-Story (or NULL)
    SwFlyFrameFormat* pFly;   ///< format of frame that was inserted as a replacement
                         ///< for a Sdr-Text object in Writer - needed for
                         ///< chaining!
    SdrObject*  pObj;    ///< pointer to the draw object (or NULL if not used)

    // Approach: In the Ctor of SvxMSDffManager only the shape ids are stored in
    //           the shape order array. The Text-Box number and the object
    //           pointer are only stored if the shape is really imported.
    explicit SvxMSDffShapeOrder( sal_uLong nId ):
        nShapeId( nId ), nTxBxComp( 0 ), pFly( nullptr ), pObj( nullptr ){}

    bool operator<( const SvxMSDffShapeOrder& rEntry ) const
    { return (nTxBxComp < rEntry.nTxBxComp); }
};

// the following will be sorted explicitly:
struct CompareSvxMSDffShapeTxBxSort
{
  bool operator()( SvxMSDffShapeOrder* const& lhs, SvxMSDffShapeOrder* const& rhs ) const { return (*lhs)<(*rhs); }
};
class MSFILTER_DLLPUBLIC SvxMSDffShapeTxBxSort : public std::set<SvxMSDffShapeOrder*,CompareSvxMSDffShapeTxBxSort> {};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
