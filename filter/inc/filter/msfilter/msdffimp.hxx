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

#ifndef FLT_MSDFFIMP_HXX
#define FLT_MSDFFIMP_HXX

#include <string.h>

#include <map>
#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_set.hpp>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/solar.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>

#include <sot/storage.hxx>

#include <vcl/graph.hxx>

#include <svx/msdffdef.hxx>

#include <filter/msfilter/msfilterdllapi.h>


class Graphic;
class SvStream;
class SdrObject;
class SdrOle2Obj;
class SotStorageRef;
class Polygon;
class PolyPolygon;
class FmFormModel;
class SdrModel;
class DffRecordHeader;
class SwFlyFrmFmt;

struct SvxMSDffBLIPInfo;
struct SvxMSDffShapeInfo;
struct SvxMSDffShapeOrder;

class MSFILTER_DLLPUBLIC DffRecordHeader
{
public:
    sal_uInt8   nRecVer; // may be DFF_PSFLAG_CONTAINER
    sal_uInt16  nRecInstance;
    sal_uInt16  nImpVerInst;
    sal_uInt16  nRecType;
    sal_uInt32  nRecLen;
    sal_uLong   nFilePos;

    DffRecordHeader() : nRecVer(0), nRecInstance(0), nImpVerInst(0),
                        nRecType(0), nRecLen(0), nFilePos(0) {}
    bool        IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    sal_uLong   GetRecBegFilePos() const { return nFilePos; }
    sal_uLong   GetRecEndFilePos() const
        { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
    bool SeekToEndOfRecord(SvStream& rIn) const
    {
        sal_Size nPos = nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen;
        return nPos == rIn.Seek(nPos);
    }
    bool SeekToContent(SvStream& rIn) const
    {
        sal_Size nPos = nFilePos + DFF_COMMON_RECORD_HEADER_SIZE;
        return nPos == rIn.Seek(nPos);
    }
    bool SeekToBegOfRecord(SvStream& rIn) const
    {
        return nFilePos == rIn.Seek(nFilePos);
    }

    MSFILTER_DLLPUBLIC friend SvStream& operator>>(SvStream& rIn, DffRecordHeader& rRec);
};

struct DffPropFlags
{
    sal_uInt8   bSet        : 1;
    sal_uInt8   bComplex    : 1;
    sal_uInt8   bBlip       : 1;
    sal_uInt8   bSoftAttr   : 1;
};

class SvxMSDffManager;

class MSFILTER_DLLPUBLIC DffPropSet
{
private:
    void InitializeProp(sal_uInt32 nKey, sal_uInt32 nContent,
            DffPropFlags& rFlags, sal_uInt32 nRecordType) const;

protected:
    typedef std::map<sal_uInt32, sal_uInt32> RecordTypesMap;

    RecordTypesMap  maRecordTypes;
    sal_uInt32      mpContents[ 1024 ];
    DffPropFlags    mpFlags[ 1024 ];

public:
    explicit DffPropSet( sal_Bool bInitialize = sal_False )
    {
        if ( bInitialize )
            memset( mpFlags, 0, 0x400 * sizeof( DffPropFlags ) );
    };

    inline sal_Bool IsProperty( sal_uInt32 nRecType ) const
        { return ( mpFlags[ nRecType & 0x3ff ].bSet ); };
    sal_Bool        IsHardAttribute( sal_uInt32 nId ) const;
    sal_uInt32      GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault = 0 ) const;
    /** Returns a boolean property by its real identifier. */
    bool            GetPropertyBool( sal_uInt32 nId, bool bDefault = false ) const;
    /** Returns a string property. */
    ::rtl::OUString GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const;
    sal_Bool        SeekToContent( sal_uInt32 nRecType, SvStream& rSt ) const;
    void            Merge( DffPropSet& rMasterPropSet ) const;
    void            InitializePropSet() const;
    friend SvStream& operator>>( SvStream& rIn, DffPropSet& rPropSet );
};

class SfxItemSet;
class SdrObject;
struct DffObjData;

class MSFILTER_DLLPUBLIC DffPropertyReader : public DffPropSet
{
    const SvxMSDffManager&  rManager;
    DffPropSet*             pDefaultPropSet;

    void ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const;
    void ApplyCustomShapeAdjustmentAttributes( SfxItemSet& rSet ) const;
    void ApplyCustomShapeGeometryAttributes( SvStream& rIn,
                                             SfxItemSet& rSet,
                                             const DffObjData& rObjData ) const;
    void ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const; // #i28269#
    void ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;

public:
    sal_Int32 mnFix16Angle;
    explicit DffPropertyReader( const SvxMSDffManager& rManager );
    ~DffPropertyReader();
    sal_Int32 Fix16ToAngle( sal_Int32 nAngle ) const;

#ifdef DBG_CUSTOMSHAPE
    void ReadPropSet( SvStream& rIn, void* pClientData, sal_uInt32 nShapeType = 0 ) const;
#else
    void ReadPropSet( SvStream& rIn, void* pClientData ) const;
#endif

    void SetDefaultPropSet( SvStream& rIn, sal_uInt32 nOffDgg ) const;
    void ApplyAttributes( SvStream& rIn, SfxItemSet& rSet ) const;
    void ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;
};

#define COL_DEFAULT RGB_COLORDATA( 0xFA, 0xFB, 0xFC )

typedef ::std::map< sal_Int32, SdrObject* > SvxMSDffShapeIdContainer;

#define SVEXT_PERSIST_STREAM "\002OlePres000"

// the following two will be sorted by the order of their appearance:
typedef boost::ptr_vector<SvxMSDffBLIPInfo> SvxMSDffBLIPInfos;
class SvxMSDffShapeOrders : public std::vector<SvxMSDffShapeOrder*>
{
public:
    ~SvxMSDffShapeOrders();
};

struct MSFILTER_DLLPUBLIC CompareSvxMSDffShapeInfoById
{
    bool operator()(::boost::shared_ptr<SvxMSDffShapeInfo> const& lhs,
                    ::boost::shared_ptr<SvxMSDffShapeInfo> const& rhs) const;
};
struct MSFILTER_DLLPUBLIC CompareSvxMSDffShapeInfoByTxBxComp
{
    bool operator()(::boost::shared_ptr<SvxMSDffShapeInfo> const& lhs,
                    ::boost::shared_ptr<SvxMSDffShapeInfo> const& rhs) const;
};

// the following will be sorted explicitly:
typedef ::std::set< ::boost::shared_ptr<SvxMSDffShapeInfo>,
            CompareSvxMSDffShapeInfoById > SvxMSDffShapeInfos_ById;
typedef ::std::multiset< ::boost::shared_ptr<SvxMSDffShapeInfo>,
            CompareSvxMSDffShapeInfoByTxBxComp> SvxMSDffShapeInfos_ByTxBxComp;

#define SVXMSDFF_SETTINGS_CROP_BITMAPS      1
#define SVXMSDFF_SETTINGS_IMPORT_PPT        2
#define SVXMSDFF_SETTINGS_IMPORT_EXCEL      4

#define SP_FGROUP       0x001   ///< This shape is a group shape
#define SP_FCHILD       0x002   ///< Not a top-level shape
#define SP_FPATRIARCH   0x004   ///< This is the topmost group shape.
                                ///< Exactly one of these per drawing.
#define SP_FDELETED     0x008   ///< The shape has been deleted
#define SP_FOLESHAPE    0x010   ///< The shape is an OLE object
#define SP_FHAVEMASTER  0x020   ///< Shape has a hspMaster property
#define SP_FFLIPH       0x040   ///< Shape is flipped horizontally
#define SP_FFLIPV       0x080   ///< Shape is flipped vertically
#define SP_FCONNECTOR   0x100   ///< Connector type of shape
#define SP_FHAVEANCHOR  0x200   ///< Shape has an anchor of some kind
#define SP_FBACKGROUND  0x400   ///< Background shape
#define SP_FHAVESPT     0x800   ///< Shape has a shape type property

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_MATHTYPE_2_STARMATH             0x0001
#define OLE_WINWORD_2_STARWRITER            0x0002
#define OLE_EXCEL_2_STARCALC                0x0004
#define OLE_POWERPOINT_2_STARIMPRESS        0x0008

struct SvxMSDffConnectorRule
{
    sal_uInt32  nRuleId;
    sal_uInt32  nShapeA;   ///< SPID of shape A
    sal_uInt32  nShapeB;   ///< SPID of shape B
    sal_uInt32  nShapeC;   ///< SPID of connector shape
    sal_uInt32  ncptiA;    ///< Connection site Index of shape A
    sal_uInt32  ncptiB;    ///< Connection site Index of shape B
    sal_uInt32  nSpFlagsA; ///< SpFlags of shape A (the original mirror flags
                           ///< must be known when solving the Solver Container)
    sal_uInt32  nSpFlagsB; ///< SpFlags of shape B

    SdrObject*  pAObj;     ///< pPtr of object (corresponding to shape A)
    SdrObject*  pBObj;     ///< pPtr of object (corresponding to shape B)
    SdrObject*  pCObj;     ///< pPtr of connector object

    SvxMSDffConnectorRule() : nSpFlagsA( 0 ), nSpFlagsB( 0 ), pAObj( NULL ),
                              pBObj( NULL ), pCObj( NULL ) {};

    friend SvStream& operator>>( SvStream& rIn, SvxMSDffConnectorRule& rAtom );
};

typedef ::std::vector< SvxMSDffConnectorRule* > SvxMSDffConnectorRuleList;

struct MSFILTER_DLLPUBLIC SvxMSDffSolverContainer
{
    SvxMSDffConnectorRuleList   aCList;

    SvxMSDffSolverContainer();
    ~SvxMSDffSolverContainer();

    MSFILTER_DLLPUBLIC friend SvStream& operator>>( SvStream& rIn, SvxMSDffSolverContainer& rAtom );
};

struct FIDCL
{
    sal_uInt32  dgid;       ///< DG owning the SPIDs in this cluster
    sal_uInt32  cspidCur;   ///< number of SPIDs used so far
};

/// provided by SvxMSDffManager for each shape in a group
struct MSDffTxId
{
    sal_uInt16 nTxBxS;
    sal_uInt16 nSequence;
    MSDffTxId( sal_uInt16 nTxBxS_, sal_uInt16 nSequence_ )
        : nTxBxS( nTxBxS_ ), nSequence( nSequence_ ){}
    MSDffTxId( const MSDffTxId& rCopy )
        : nTxBxS( rCopy.nTxBxS ), nSequence( rCopy.nSequence ){}
};

struct MSFILTER_DLLPUBLIC SvxMSDffImportRec
{
    static const int RELTO_DEFAULT = 2;

    SdrObject*      pObj;
    Polygon*        pWrapPolygon;
    char*           pClientAnchorBuffer;
    sal_uInt32      nClientAnchorLen;
    char*           pClientDataBuffer;
    sal_uInt32      nClientDataLen;
    sal_uInt32      nXAlign;
    sal_uInt32      *pXRelTo;
    sal_uInt32      nYAlign;
    sal_uInt32      *pYRelTo;
    sal_uInt32      nLayoutInTableCell;
    sal_uInt32      nFlags;
    long            nTextRotationAngle;
    long            nDxTextLeft;    ///< distance of text box from surrounding shape
    long            nDyTextTop;
    long            nDxTextRight;
    long            nDyTextBottom;
    long            nDxWrapDistLeft;
    long            nDyWrapDistTop;
    long            nDxWrapDistRight;
    long            nDyWrapDistBottom;
    long            nCropFromTop;
    long            nCropFromBottom;
    long            nCropFromLeft;
    long            nCropFromRight;
    MSDffTxId       aTextId;        ///< identifier for text boxes
    sal_uLong       nNextShapeId;   ///< for linked text boxes
    sal_uLong       nShapeId;
    MSO_SPT         eShapeType;
    MSO_LineStyle   eLineStyle;     ///< border types
    MSO_LineDashing eLineDashing;
    sal_Bool        bDrawHell       :1;
    sal_Bool        bHidden         :1;
    sal_Bool        bReplaceByFly   :1;
    sal_Bool        bLastBoxInChain :1;
    sal_Bool        bHasUDefProp    :1;
    sal_Bool        bVFlip          :1;
    sal_Bool        bHFlip          :1;
    sal_Bool        bAutoWidth      :1;
    int             relativeHorizontalWidth; ///< in 0.1% or -1 for none
    bool            isHorizontalRule;

    SvxMSDffImportRec();
    SvxMSDffImportRec(const SvxMSDffImportRec& rCopy);
    ~SvxMSDffImportRec();

    bool operator==( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId == rEntry.nShapeId; }
    bool operator<( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId < rEntry.nShapeId;  }

private:
    SvxMSDffImportRec &operator=(const SvxMSDffImportRec&);
};

/** list of all SvxMSDffImportRec instances of/for a group */
class MSDffImportRecords
    : public ::boost::ptr_set<SvxMSDffImportRec>
{};

/** block of parameters for import/export for a single call of
    ImportObjAtCurrentStreamPos() */
struct SvxMSDffImportData
{
    MSDffImportRecords  aRecords;   ///< Shape pointer, Shape ids and private data
    Rectangle           aParentRect;///< Rectangle of the surrounding groups,
                                    ///< which might have been provided externally
    Rectangle           aNewRect;   ///< Rectangle that is defined by this shape

    SvxMSDffImportData()
        {}
    explicit SvxMSDffImportData( const Rectangle& rParentRect ) : aParentRect( rParentRect ) {}
    void SetNewRect(sal_Int32 left, sal_Int32 top, sal_Int32 right, sal_Int32 bottom )
        { aNewRect = Rectangle(left, top, right, bottom); }
    sal_Bool HasParRect() const { return aParentRect.IsEmpty(); }
    sal_Bool HasNewRect() const { return aNewRect.IsEmpty()   ; }
    bool empty() const { return aRecords.empty(); }
    size_t size() const { return aRecords.size(); }
    MSDffImportRecords::const_iterator begin() const { return aRecords.begin();  }
    MSDffImportRecords::const_iterator end() const { return aRecords.end();  }
};

struct DffObjData
{
    const DffRecordHeader&  rSpHd;

    Rectangle   aBoundRect;
    Rectangle   aChildAnchor;

    sal_uInt32  nShapeId;
    sal_uInt32  nSpFlags;
    MSO_SPT     eShapeType;

    sal_Bool bShapeType     : 1;
    sal_Bool bClientAnchor  : 1;
    sal_Bool bClientData    : 1;
    sal_Bool bChildAnchor   : 1;
    sal_Bool bOpt           : 1;
    sal_Bool bIsAutoText    : 1;

    int nCalledByGroup;

    DffObjData( const DffRecordHeader& rObjHd,
                const Rectangle& rBoundRect,
                int   nClByGroup ) :
        rSpHd( rObjHd ),
        aBoundRect( rBoundRect ),
        nShapeId( 0 ),
        nSpFlags( 0 ),
        eShapeType( mso_sptNil ),
        bShapeType( sal_False ),
        bClientAnchor( sal_False ),
        bClientData( sal_False ),
        bChildAnchor( sal_False ),
        bOpt( sal_False ),
        bIsAutoText( sal_False ),
        nCalledByGroup( nClByGroup ){}
};

#define DFF_RECORD_MANAGER_BUF_SIZE         64

struct DffRecordList
{
    sal_uInt32          nCount;
    sal_uInt32          nCurrent;
    DffRecordList*      pPrev;
    DffRecordList*      pNext;

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
public :
        DffRecordList*   pCList;

        void             Clear();
        void             Consume( SvStream& rIn,
                                  sal_Bool bAppend = sal_False,
                                  sal_uInt32 nStOfs = 0 );

        sal_Bool         SeekToContent( SvStream& rIn,
                                        sal_uInt16 nRecType,
                                        DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );
        DffRecordHeader* GetRecordHeader( sal_uInt16 nRecType,
                                          DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );

        DffRecordManager();
        explicit DffRecordManager( SvStream& rIn );
        ~DffRecordManager();

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
    FmFormModel*            pFormModel;
    SvxMSDffBLIPInfos*      pBLIPInfos;
    ::boost::scoped_ptr<SvxMSDffShapeInfos_ByTxBxComp> m_pShapeInfosByTxBxComp;
    ::boost::scoped_ptr<SvxMSDffShapeInfos_ById> m_pShapeInfosById;
    SvxMSDffShapeOrders*    pShapeOrders;
    sal_uLong               nDefaultFontHeight;
    sal_uInt32              nOffsDgg;
    sal_uInt16              nBLIPCount;
    sal_uInt16              nShapeCount;
    sal_uInt32              nGroupShapeFlags;

    void CheckTxBxStoryChain();
    void GetFidclData(sal_uInt32 nOffsDgg);

protected :
    typedef std::map<sal_uInt32, sal_uInt32> OffsetMap;

    String          maBaseURL;
    sal_uInt32      mnCurMaxShapeId;    // we need this information to
    sal_uInt32      mnDrawingsSaved;    // access the right drawing
    sal_uInt32      mnIdClusters;       // while only knowing the shapeid
    std::vector<FIDCL> maFidcls;
    OffsetMap       maDgOffsetTable;    ///< array of fileoffsets

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
    // #156763#
    // Add internal drawing container id as parameter to the sub methods of
    // reading the control information about the drawing objects.
    // The drawing container id is used to distinguish the text ids of drawing
    // objects in different drawing containers.
    void GetDrawingContainerData( SvStream& rSt,
                                  sal_uLong nLenDg,
                                  const unsigned long nDrawingContainerId );
    sal_Bool GetShapeGroupContainerData( SvStream& rSt,
                                     sal_uLong nLenShapeGroupCont,
                                     sal_Bool bPatriarch,
                                     const unsigned long nDrawingContainerId );
    sal_Bool GetShapeContainerData( SvStream& rSt,
                                sal_uLong nLenShapeCont,
                                sal_uLong nPosGroup,
                                const unsigned long nDrawingContainerId );

    bool ReadGraphic( SvStream& rSt, sal_uLong nIndex, Graphic& rGraphic ) const;
    SdrObject* ImportGraphic( SvStream&, SfxItemSet&, const DffObjData& );
    // #i32596# - pass <nCalledByGroup> to method
    // Needed in Writer's Microsoft Word import to avoid import of OLE objects
    // inside groups. Instead a graphic object is created.
    virtual SdrObject* ImportOLE( long nOLEId,
                                  const Graphic& rGraf,
                                  const Rectangle& rBoundRect,
                                  const Rectangle& rVisArea,
                                  const int _nCalledByGroup,
                                  sal_Int64 nAspect ) const;
    static com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > CheckForConvertToSOObj(
                sal_uInt32 nConvertFlags, SotStorage& rSrcStg,
                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg,
                const Graphic& rGrf,
                const Rectangle& rVisArea );

// the following methods need to be overriden for Excel imports
    virtual sal_Bool ProcessClientAnchor( SvStream& rStData,
                                          sal_uInt32 nDatLen,
                                          char*& rpBuff,
                                          sal_uInt32& rBuffLen ) const;
    virtual void ProcessClientAnchor2( SvStream& rStData,
                                       DffRecordHeader& rHd,
                                       void* pData, DffObjData& );
    virtual sal_Bool ProcessClientData( SvStream& rStData,
                                        sal_uInt32 nDatLen,
                                        char*& rpBuff,
                                        sal_uInt32& rBuffLen ) const;
    virtual SdrObject* ProcessObj( SvStream& rSt,
                                   DffObjData& rData,
                                   void* pData,
                                   Rectangle& rTextRect,
                                   SdrObject* pObj = NULL);
    virtual sal_uLong Calc_nBLIPPos( sal_uLong nOrgVal,
                                     sal_uLong nStreamPos ) const;
    virtual bool GetColorFromPalette(sal_uInt16 nNum, Color& rColor) const;

    // Fontwork objects use a new implementation of ReadObjText because the old
    // one does not properly import multiple paragraphs.
    void ReadObjText( const String& rText, SdrObject* pObj ) const;

// the following method needs to be overridden for the import of OLE objects
    virtual sal_Bool GetOLEStorageName( long nOLEId,
                                        String& rStorageName,
                                        SotStorageRef& rSrcStorage,
                      com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg
                                    ) const;

    /** Prevent that (rounded) rectangles with wrapped text will always be
        converted into SdrRectObj( OBJ_TEXT ).

        @return sal_True means "conversion".
    */
    virtual sal_Bool ShapeHasText(sal_uLong nShapeId, sal_uLong nFilePos) const;

public:
    std::map<sal_uInt32,rtl::OString> aEscherBlipCache;

    DffRecordManager    maShapeRecords;
    ColorData           mnDefaultColor;

    sal_Bool            mbTracing;

    Color MSO_TEXT_CLR_ToColor( sal_uInt32 nColorCode ) const;
    Color MSO_CLR_ToColor( sal_uInt32 nColorCode,
                           sal_uInt16 nContextProperty = DFF_Prop_lineColor ) const;
    virtual sal_Bool SeekToShape( SvStream& rSt,
                                  void* pClientData,
                                  sal_uInt32 nId ) const;
    bool SeekToRec( SvStream& rSt,
                    sal_uInt16 nRecId,
                    sal_uLong nMaxFilePos,
                    DffRecordHeader* pRecHd = NULL,
                    sal_uLong nSkipCount = 0 ) const;
    bool SeekToRec2( sal_uInt16 nRecId1,
                     sal_uInt16 nRecId2,
                     sal_uLong nMaxFilePos,
                     DffRecordHeader* pRecHd = NULL,
                     sal_uLong nSkipCount = 0 ) const;

    static rtl::OUString MSDFFReadZString( SvStream& rIn,
                                           sal_uInt32 nMaxLen,
                                           bool bUniCode = sal_False);

    static bool ReadCommonRecordHeader( DffRecordHeader& rRec,
                                        SvStream& rIn) SAL_WARN_UNUSED_RESULT;
    static bool ReadCommonRecordHeader( SvStream& rSt,
                                        sal_uInt8& rVer,
                                        sal_uInt16& rInst,
                                        sal_uInt16& rFbt,
                                        sal_uInt32& rLength) SAL_WARN_UNUSED_RESULT;

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
    */
    SvxMSDffManager( SvStream& rStCtrl,
                     const String& rBaseURL,
                     sal_uInt32 nOffsDgg,
                     SvStream* pStData,
                     SdrModel* pSdrModel_           =  0,
                     long      nApplicationScale    =  0,
                     ColorData mnDefaultColor_      =  COL_DEFAULT,
                     sal_uLong nDefaultFontHeight_  = 24,
                     SvStream* pStData2_            =  0 );

    // in PPT the parameters DGGContainerOffset and PicStream are provided by an
    // init method
    SvxMSDffManager( SvStream& rStCtrl, const String& rBaseURL );
    void InitSvxMSDffManager( sal_uInt32 nOffsDgg_,
                              SvStream* pStData_,
                              sal_uInt32 nSvxMSDffOLEConvFlags);
    void SetDgContainer( SvStream& rSt );

    virtual ~SvxMSDffManager();

    sal_uInt32  GetSvxMSDffSettings() const { return nSvxMSDffSettings; };
    void    SetSvxMSDffSettings( sal_uInt32 nSettings ) { nSvxMSDffSettings = nSettings; };

    static sal_Bool     MakeContentStream( SotStorage * pStor, const GDIMetaFile & );
    static sal_Bool     ConvertToOle2( SvStream& rStm,
                                       sal_uInt32 nLen,
                                       const GDIMetaFile*,
                                       const SotStorageRef & rDest );
    static rtl::OUString ReadDffString( SvStream& rSt,
                                        DffRecordHeader aStrHd = DffRecordHeader());
    static bool ReadObjText(SvStream& rSt, SdrObject* pObj);

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

        @return sal_True if successful, sal_False otherwise
    */
    sal_Bool GetBLIP( sal_uLong nIdx, Graphic& rData, Rectangle* pVisArea = NULL );

// TODO: provide proper documentation here
    /** read a BLIP out of a already positioned stream

        @param[in] rBLIPStream alread positioned stream (mandatory)
        @param[out] rData      already converted data (insert directly as
                               graphics into our documents)
        @param pVisArea        ???

        @return sal_True if successful, sal_False otherwise
    */
    sal_Bool GetBLIPDirect(SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea = NULL ) const;

    sal_Bool GetShape(sal_uLong nId, SdrObject*& rpData, SvxMSDffImportData& rData);

    /** Get count of managed BLIPs

        @return Number of BLIPs in pStData (or rStCtrl), thus number of FBSEs in
                the drawing group container. If 0 is returned this means that
                the structure is ok but there are no BLIPs; if USHRT_MAX is
                returned than there was an error: no correct Drawing File Format
    */
    sal_uInt16 GetBLIPCount() const{ return nBLIPCount; }

    SdrObject* ImportObj( SvStream& rSt,
                          void* pData,
                          Rectangle& rClientRect,
                          const Rectangle& rGlobalChildRect,
                          int nCalledByGroup = 0,
                          sal_Int32* pShapeId = NULL);
    SdrObject* ImportGroup( const DffRecordHeader& rHd,
                            SvStream& rSt,
                            void* pData,
                            Rectangle& rClientRect,
                            const Rectangle& rGlobalChildRect,
                            int nCalledByGroup = 0,
                            sal_Int32* pShapeId = NULL );
    SdrObject* ImportShape( const DffRecordHeader& rHd,
                            SvStream& rSt,
                            void* pData,
                            Rectangle& rClientRect,
                            const Rectangle& rGlobalChildRect,
                            int nCalledByGroup = 0,
                            sal_Int32* pShapeId = NULL);

    Rectangle GetGlobalChildAnchor( const DffRecordHeader& rHd,
                                    SvStream& rSt,
                                    Rectangle& aClientRect );
    void GetGroupAnchors( const DffRecordHeader& rHd,
                          SvStream& rSt,
                          Rectangle& rGroupClientAnchor,
                          Rectangle& rGroupChildAnchor,
                          const Rectangle& rClientRect,
                          const Rectangle& rGlobalChildRect );

    inline const SvxMSDffShapeInfos_ById* GetShapeInfos( void ) const
        { return m_pShapeInfosById.get(); }

    inline const SvxMSDffShapeOrders* GetShapeOrders( void ) const
        { return pShapeOrders; }

    void StoreShapeOrder(sal_uLong      nId,
                         sal_uLong      nTxBx,
                         SdrObject*     pObject,
                         SwFlyFrmFmt*   pFly = 0,
                         short          nHdFtSection = 0) const;

    void ExchangeInShapeOrder(SdrObject*    pOldObject,
                              sal_uLong     nTxBx,
                              SwFlyFrmFmt*  pFly,
                              SdrObject*    pObject) const;

    void RemoveFromShapeOrder( SdrObject* pObject ) const;

    sal_uInt32  GetConvertFlags() const { return nSvxMSDffOLEConvFlags; }

    static SdrOle2Obj* CreateSdrOLEFromStorage( const String& rStorageName,
                                                SotStorageRef& rSrcStorage,
                                                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg,
                                                const Graphic& rGraf,
                                                const Rectangle& rBoundRect,
                                                const Rectangle& rVisArea,
                                                SvStream* pDataStrrm,
                                                ErrCode& rError,
                                                sal_uInt32 nConvertFlags,
                                                sal_Int64 nAspect );

    /** Create connections between shapes.
        This method should be called after a page is imported.

        @param rSolver contains necessary data that is collected during the
                       import of each shape
    */
    void SolveSolver( const SvxMSDffSolverContainer& rSolver );

    static sal_Bool SetPropValue(
        const ::com::sun::star::uno::Any& rAny,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rPropertyName,
        sal_Bool bTestPropertyAvailability = sal_False
    );

    void insertShapeId( sal_Int32 nShapeId, SdrObject* pShape );
    void removeShapeId( SdrObject* pShape );
    SdrObject* getShapeForId( sal_Int32 nShapeId );
};

struct SvxMSDffBLIPInfo
{
    sal_uInt16 nBLIPType;   ///< type of BLIP: e.g. 6 for PNG
    sal_uLong  nFilePos;    ///< offset of the BLIP in data strem
    sal_uLong  nBLIPSize;   ///< number of bytes that the BLIP needs in stream
    SvxMSDffBLIPInfo(sal_uInt16 nBType, sal_uLong nFPos, sal_uLong nBSize):
        nBLIPType( nBType ), nFilePos( nFPos ), nBLIPSize( nBSize ){}
};

struct SvxMSDffShapeInfo
{
    sal_uInt32 nShapeId; ///< shape id, used in PLCF SPA and in mso_fbtSp (FSP)
    sal_uLong nFilePos;  ///< offset of the shape in control stream for
                         ///< potential later access to it
    sal_uInt32 nTxBxComp;

    sal_Bool bReplaceByFly  :1; ///< shape can be replaced by a frame in Writer
    sal_Bool bLastBoxInChain:1;

    explicit SvxMSDffShapeInfo(sal_uLong nFPos, sal_uInt32 nId=0, // sal_uLong nBIdx=0,
                      sal_uInt16 nSeqId=0, sal_uInt16 nBoxId=0):
        nShapeId( nId ),
        nFilePos( nFPos ),
        nTxBxComp( (nSeqId << 16) + nBoxId )
        {
            bReplaceByFly   = sal_False;
            bLastBoxInChain = sal_True;
        }
    SvxMSDffShapeInfo(SvxMSDffShapeInfo& rInfo):
        nShapeId( rInfo.nShapeId ),
        nFilePos( rInfo.nFilePos ),
        nTxBxComp( rInfo.nTxBxComp )
        {
            bReplaceByFly   = rInfo.bReplaceByFly;
            bLastBoxInChain = rInfo.bLastBoxInChain;
        }
};


struct SvxMSDffShapeOrder
{
    sal_uLong nShapeId;  ///< shape id used in PLCF SPA and in mso_fbtSp (FSP)
    sal_uLong nTxBxComp; ///< chain or box number in the Text-Box-Story (or NULL)
    SwFlyFrmFmt* pFly;   ///< format of frame that was inserted as a replacement
                         ///< for a Sdr-Text object in Writer - needed for
                         ///< chaining!
    short nHdFtSection;  ///< used by Writer to find out if linked frames are in
                         ///< the same header or footer of the same section
    SdrObject*  pObj;    ///< pointer to the draw object (or NULL if not used)

    // Approach: In the Ctor of SvxMSDffManager only the shape ids are stored in
    //           the shape order array. The Text-Box number and the object
    //           pointer are only stored if the shape is really imported.
    explicit SvxMSDffShapeOrder( sal_uLong nId ):
        nShapeId( nId ), nTxBxComp( 0 ), pFly( 0 ), nHdFtSection( 0 ), pObj( 0 ){}

    bool operator==( const SvxMSDffShapeOrder& rEntry ) const
    { return (nTxBxComp == rEntry.nTxBxComp); }
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
