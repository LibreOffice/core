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

#ifndef _MSDFFIMP_HXX
#define _MSDFFIMP_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/solar.h>
#include <svl/svarray.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <tools/table.hxx>
#include <svx/msdffdef.hxx>
#include <filter/msfilter/msfiltertracer.hxx>
#include <vcl/graph.hxx>
#include <string.h>
#include <map>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <filter/msfilter/msfilterdllapi.h>
#include <sot/storage.hxx>
#include <vector>

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
public:
    DffRecordHeader() : nRecVer(0), nRecInstance(0), nImpVerInst(0), nRecType(0), nRecLen(0), nFilePos(0) {}
    bool        IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    sal_uLong   GetRecBegFilePos() const { return nFilePos; }
    sal_uLong   GetRecEndFilePos() const { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
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

class MSFILTER_DLLPUBLIC DffPropSet : public Table
{
protected:

    sal_uInt32      mpContents[ 1024 ];
    DffPropFlags    mpFlags[ 1024 ];

public:

    DffPropSet( sal_Bool bInitialize = sal_False )
    {
        if ( bInitialize )
            memset( mpFlags, 0, 0x400 * sizeof( DffPropFlags ) );
    };

    inline sal_Bool IsProperty( sal_uInt32 nRecType ) const { return ( mpFlags[ nRecType & 0x3ff ].bSet ); };
    sal_Bool        IsHardAttribute( sal_uInt32 nId ) const;
    sal_uInt32      GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault = 0 ) const;
    /** Returns a boolean property by its real identifier. */
    bool            GetPropertyBool( sal_uInt32 nId, bool bDefault = false ) const;
    /** Returns a string property. */
    ::rtl::OUString GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const;
    void            SetPropertyValue( sal_uInt32 nId, sal_uInt32 nValue ) const;
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

    void        ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const;
    void        ApplyCustomShapeAdjustmentAttributes( SfxItemSet& rSet ) const;
    void        ApplyCustomShapeGeometryAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;
    void        ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const; // #i28269#
    void        ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;

public:

    sal_Int32                   mnFix16Angle;

    DffPropertyReader( const SvxMSDffManager& rManager );
    ~DffPropertyReader();
    sal_Int32       Fix16ToAngle( sal_Int32 nAngle ) const;

#ifdef DBG_CUSTOMSHAPE
    void        ReadPropSet( SvStream& rIn, void* pClientData, sal_uInt32 nShapeType = 0 ) const;
#else
    void        ReadPropSet( SvStream& rIn, void* pClientData ) const;
#endif

    void        SetDefaultPropSet( SvStream& rIn, sal_uInt32 nOffDgg ) const;
    void        ApplyAttributes( SvStream& rIn, SfxItemSet& rSet ) const;
    void        ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;
};


#define COL_DEFAULT RGB_COLORDATA( 0xFA, 0xFB, 0xFC )

typedef SvxMSDffBLIPInfo*  SvxMSDffBLIPInfo_Ptr;

typedef SvxMSDffShapeInfo* SvxMSDffShapeInfo_Ptr;

typedef SvxMSDffShapeOrder* SvxMSDffShapeOrder_Ptr;

typedef ::std::map< sal_Int32, SdrObject* > SvxMSDffShapeIdContainer;

#define SVEXT_PERSIST_STREAM "\002OlePres000"

// nach der Reihenfolge des Auftretens sortiert werden:
SV_DECL_PTRARR_DEL(SvxMSDffBLIPInfos,   SvxMSDffBLIPInfo_Ptr,   16,16)

SV_DECL_PTRARR_DEL(SvxMSDffShapeOrders, SvxMSDffShapeOrder_Ptr, 16,16)

// explizit sortiert werden:
SV_DECL_PTRARR_SORT_DEL_VISIBILITY(SvxMSDffShapeInfos,  SvxMSDffShapeInfo_Ptr,  16,16, MSFILTER_DLLPUBLIC)

SV_DECL_PTRARR_SORT_VISIBILITY(SvxMSDffShapeTxBxSort,   SvxMSDffShapeOrder_Ptr, 16,16, MSFILTER_DLLPUBLIC)

#define SVXMSDFF_SETTINGS_CROP_BITMAPS      1
#define SVXMSDFF_SETTINGS_IMPORT_PPT        2
#define SVXMSDFF_SETTINGS_IMPORT_EXCEL      4

#define SP_FGROUP       0x001   // This shape is a group shape
#define SP_FCHILD       0x002   // Not a top-level shape
#define SP_FPATRIARCH   0x004   // This is the topmost group shape.
                                // Exactly one of these per drawing.
#define SP_FDELETED     0x008   // The shape has been deleted
#define SP_FOLESHAPE    0x010   // The shape is an OLE object
#define SP_FHAVEMASTER  0x020   // Shape has a hspMaster property
#define SP_FFLIPH       0x040   // Shape is flipped horizontally
#define SP_FFLIPV       0x080   // Shape is flipped vertically
#define SP_FCONNECTOR   0x100   // Connector type of shape
#define SP_FHAVEANCHOR  0x200   // Shape has an anchor of some kind
#define SP_FBACKGROUND  0x400   // Background shape
#define SP_FHAVESPT     0x800   // Shape has a shape type property

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_MATHTYPE_2_STARMATH             0x0001
#define OLE_WINWORD_2_STARWRITER            0x0002
#define OLE_EXCEL_2_STARCALC                0x0004
#define OLE_POWERPOINT_2_STARIMPRESS        0x0008

struct SvxMSDffConnectorRule
{
    sal_uInt32  nRuleId;
    sal_uInt32  nShapeA;        // SPID of shape A

    sal_uInt32  nShapeB;        // SPID of shape B
    sal_uInt32  nShapeC;        // SPID of connector shape
    sal_uInt32  ncptiA;         // Connection site Index of shape A
    sal_uInt32  ncptiB;         // Connection site Index of shape B
    sal_uInt32  nSpFlagsA;      // SpFlags of shape A ( the original mirror flags must be known when solving the Solver Container )
    sal_uInt32  nSpFlagsB;      // SpFlags of shape A

    SdrObject*  pAObj;       // pPtr of object ( corresponding to shape A )
    SdrObject*  pBObj;       //   "
    SdrObject*  pCObj;       //   "  of connector object

    SvxMSDffConnectorRule() : nSpFlagsA( 0 ), nSpFlagsB( 0 ), pAObj( NULL ), pBObj( NULL ), pCObj( NULL ) {};

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
    sal_uInt32  dgid;       // DG owning the SPIDs in this cluster
    sal_uInt32  cspidCur;   // number of SPIDs used so far
};

//---------------------------------------------------------------------------
//  von SvxMSDffManager fuer jedes in der Gruppe enthaltene Shape geliefert
//---------------------------------------------------------------------------
struct MSDffTxId
{
    sal_uInt16 nTxBxS;
    sal_uInt16 nSequence;
    MSDffTxId(sal_uInt16 nTxBxS_, sal_uInt16 nSequence_ )
            : nTxBxS(             nTxBxS_       ),
              nSequence(          nSequence_    ){}
    MSDffTxId(const MSDffTxId& rCopy)
            : nTxBxS(             rCopy.nTxBxS    ),
              nSequence(          rCopy.nSequence ){}
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
    long            nDxTextLeft;    // Abstand der Textbox vom umgebenden Shape
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
    MSDffTxId       aTextId;        // Kennungen fuer Textboxen
    sal_uLong       nNextShapeId;   // fuer verlinkte Textboxen
    sal_uLong       nShapeId;
    MSO_SPT         eShapeType;
    MSO_LineStyle   eLineStyle;   // Umrandungs-Arten
    MSO_LineDashing eLineDashing;
    sal_Bool        bDrawHell       :1;
    sal_Bool        bHidden         :1;
    sal_Bool        bReplaceByFly   :1;
    sal_Bool        bLastBoxInChain :1;
    sal_Bool        bHasUDefProp    :1;
    sal_Bool        bVFlip :1;
    sal_Bool        bHFlip :1;
    sal_Bool        bAutoWidth      :1;

    SvxMSDffImportRec();
    SvxMSDffImportRec(const SvxMSDffImportRec& rCopy);
    ~SvxMSDffImportRec();
    sal_Bool operator==( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId == rEntry.nShapeId; }

    sal_Bool operator<( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId < rEntry.nShapeId;  }
private:
    SvxMSDffImportRec &operator=(const SvxMSDffImportRec&);
};

typedef SvxMSDffImportRec* MSDffImportRec_Ptr;

// Liste aller SvxMSDffImportRec fuer eine Gruppe
SV_DECL_PTRARR_SORT_DEL_VISIBILITY(MSDffImportRecords, MSDffImportRec_Ptr, 16,16, MSFILTER_DLLPUBLIC)

//---------------------------------------------------------------------------
//   Import-/Export-Parameterblock fuer 1 x ImportObjAtCurrentStreamPos()
//---------------------------------------------------------------------------
struct SvxMSDffImportData
{
    MSDffImportRecords  aRecords;   // Shape-Pointer, -Ids und private Daten
    Rectangle           aParentRect;// Rectangle der umgebenden Gruppe
                                    // bzw. von aussen reingegebenes Rect
    Rectangle           aNewRect;   // mit diesem Shape definiertes Rectangle

    SvxMSDffImportData()
        {}
    SvxMSDffImportData(const Rectangle& rParentRect)
        :aParentRect( rParentRect )
        {}
    void SetNewRect(sal_Int32 l, sal_Int32 o,
                    sal_Int32 r, sal_Int32 u ){ aNewRect = Rectangle(l,o, r,u); }
    sal_Bool HasParRect() const { return aParentRect.IsEmpty(); }
    sal_Bool HasNewRect() const { return aNewRect.IsEmpty()   ; }
    sal_Bool HasRecords() const { return 0 != aRecords.Count(); }
    sal_uInt16              GetRecCount() const { return aRecords.Count();  }
    SvxMSDffImportRec*  GetRecord(sal_uInt16 iRecord) const
                            {  return aRecords.GetObject( iRecord ); }
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
                int              nClByGroup ) :
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

                        DffRecordList( DffRecordList* pList );
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

        DffRecordList*      pCList;

        void                Clear();
        void                Consume( SvStream& rIn, sal_Bool bAppend = sal_False, sal_uInt32 nStOfs = 0 );

        sal_Bool            SeekToContent( SvStream& rIn, sal_uInt16 nRecType, DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );
        DffRecordHeader*    GetRecordHeader( sal_uInt16 nRecType,  DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );

                            DffRecordManager();
                            DffRecordManager( SvStream& rIn );
                            ~DffRecordManager();

        DffRecordHeader*    Current();
        DffRecordHeader*    First();
        DffRecordHeader*    Next();
        DffRecordHeader*    Prev();
        DffRecordHeader*    Last();
};

/*
    SvxMSDffManager - abstrakte Basis-Klasse fuer Escher-Import
    ===============
    Zweck:  Zugriff auf Objekte im Drawing File Format
    Stand:  Zugriff nur auf BLIPs (wird spaeter erweitert)

    Beachte: in der zwecks PowerPoint-, ODER Word- ODER Excel-Import
    ======== abgeleiteten Klasse
             MUSS jeweils die Methode ProcessUserDefinedRecord()
             implementiert werden!
*/
class MSFILTER_DLLPUBLIC SvxMSDffManager : public DffPropertyReader
{
    FmFormModel*            pFormModel;
    SvxMSDffBLIPInfos*      pBLIPInfos;
    SvxMSDffShapeInfos*     pShapeInfos;
    SvxMSDffShapeOrders*    pShapeOrders;
    sal_uLong               nDefaultFontHeight;
    sal_uInt32              nOffsDgg;
    sal_uInt16              nBLIPCount;
    sal_uInt16              nShapeCount;
    sal_uInt32              nGroupShapeFlags;

    void CheckTxBxStoryChain();
    void GetFidclData(sal_uInt32 nOffsDgg);

protected :

    String          maBaseURL;
    sal_uInt32      mnCurMaxShapeId;    // we need this information to
    sal_uInt32      mnDrawingsSaved;    // access the right drawing
    sal_uInt32      mnIdClusters;       // while only knowing the shapeid
    FIDCL*          mpFidcls;
    Table           maDgOffsetTable;    // array of fileoffsets

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

    /** stores a reference to an imported SdrObject with its shape id if
        it has one
    */
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
    SdrObject* ImportFontWork( SvStream&, SfxItemSet&, Rectangle& rBoundRect ) const;
    SdrObject* ImportGraphic( SvStream&, SfxItemSet&, const DffObjData& );
    // #i32596# - pass <nCalledByGroup> to method
    // Needed in the Writer Microsoft Word import to avoid import of OLE objects
    // inside groups. Instead a graphic object is created.
    virtual SdrObject* ImportOLE( long nOLEId,
                                  const Graphic& rGraf,
                                  const Rectangle& rBoundRect,
                                  const Rectangle& rVisArea,
                                  const int _nCalledByGroup,
                                  sal_Int64 nAspect ) const;
    SdrObject* GetAutoForm( MSO_SPT eTyp ) const;
    static const GDIMetaFile* lcl_GetMetaFileFromGrf_Impl( const Graphic& rGrf, GDIMetaFile& rMtf );
    static com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > CheckForConvertToSOObj(
                sal_uInt32 nConvertFlags, SotStorage& rSrcStg,
                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg,
                const Graphic& rGrf,
                const Rectangle& rVisArea );

/*
        folgende Methoden sind zum Excel-Import zu ueberschreiben:
*/
    virtual sal_Bool ProcessClientAnchor(SvStream& rStData, sal_uLong nDatLen, char*& rpBuff, sal_uInt32& rBuffLen ) const;
    virtual void ProcessClientAnchor2( SvStream& rStData, DffRecordHeader& rHd, void* pData, DffObjData& );
    virtual sal_Bool ProcessClientData(  SvStream& rStData, sal_uLong nDatLen, char*& rpBuff, sal_uInt32& rBuffLen ) const;
    virtual SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj = NULL);
    virtual sal_uLong Calc_nBLIPPos( sal_uLong nOrgVal, sal_uLong nStreamPos ) const;
    virtual bool GetColorFromPalette(sal_uInt16 nNum, Color& rColor) const;

    bool ReadDffString(SvStream& rSt, String& rTxt) const;
    bool ReadObjText(SvStream& rSt, SdrObject* pObj) const;

    // SJ: New implementation of ReadObjText is used by Fontwork objects, because
    // the old one does not properly import multiple paragraphs
    void ReadObjText( const String& rText, SdrObject* pObj ) const;

    /*
        folgende Methode ist von allen zu ueberschreiben, die OLE-Objecte
        importieren moechten:
    */
    virtual sal_Bool GetOLEStorageName( long nOLEId, String& rStorageName,
                                    SotStorageRef& rSrcStorage,
                com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg
                                    ) const;

    /*
        folgende Methode ist von allen zu ueberschreiben, die verhindern
        moechten, dass (abgerundete) Rechtecke mit umgebrochenem Text
        immer in SdrRectObj( OBJ_TEXT ) umgewandelt werden:
        sal_True bedeutet umwandeln.
    */
    virtual sal_Bool ShapeHasText(sal_uLong nShapeId, sal_uLong nFilePos) const;

public:

    void*               pSvxMSDffDummy1;
    void*               pSvxMSDffDummy2;
    void*               pSvxMSDffDummy3;
    std::map<sal_uInt32,ByteString> aEscherBlipCache;

    DffRecordManager    maShapeRecords;
    ColorData           mnDefaultColor;

    MSFilterTracer*     mpTracer;
    sal_Bool            mbTracing;

    Color MSO_TEXT_CLR_ToColor( sal_uInt32 nColorCode ) const;
    Color MSO_CLR_ToColor( sal_uInt32 nColorCode, sal_uInt16 nContextProperty = DFF_Prop_lineColor ) const;
    virtual sal_Bool SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const;
    bool SeekToRec( SvStream& rSt, sal_uInt16 nRecId, sal_uLong nMaxFilePos, DffRecordHeader* pRecHd = NULL, sal_uLong nSkipCount = 0 ) const;
    bool SeekToRec2( sal_uInt16 nRecId1, sal_uInt16 nRecId2, sal_uLong nMaxFilePos, DffRecordHeader* pRecHd = NULL, sal_uLong nSkipCount = 0 ) const;

    static void MSDFFReadZString( SvStream& rIn, String& rStr, sal_uLong nMaxLen, bool bUniCode = sal_False );

    static sal_Bool ReadCommonRecordHeader( DffRecordHeader& rRec, SvStream& rIn );
    static sal_Bool ReadCommonRecordHeader( SvStream& rSt,
                                        sal_uInt8&     rVer,
                                        sal_uInt16&   rInst,
                                        sal_uInt16&   rFbt,
                                        sal_uInt32&    rLength );
/*
    Konstruktor
    ===========
    Input:  rStCtrl   - Verwaltungsstream mit Containern,
                        FBSE Objekten und Shapes
                        ( muss immer uebergeben werden;
                          Stream muss bereits offen sein )

            nOffsDgg  - Offset im rStCtrl: Beginn des Drawing Group Containers

            pStData   - Datenstream, in dem die BLIPs gespeichert sind
                        ( falls Null, wird angenommen, dass die
                          BLIPs ebenfalls im rStCtrl gespeichert sind;
                          dieser Stream muss ebenfalls bereits offen sein )
            pSdrModel_  kann beim Ctor-Aufruf leer bleiben, muss dann aber
                        spaeter ueber SetModel() gesetzt werden!

    setzt nBLIPCount
*/
    SvxMSDffManager( SvStream& rStCtrl,
                     const String& rBaseURL,
                     sal_uInt32 nOffsDgg,
                     SvStream* pStData,
                     SdrModel* pSdrModel_           =  0,
                     long      nApplicationScale    =  0,
                     ColorData mnDefaultColor_      =  COL_DEFAULT,
                     sal_uLong     nDefaultFontHeight_  = 24,
                     SvStream* pStData2_            =  0,
                     MSFilterTracer* pTracer        = NULL );

    // in PPT werden die Parameter DGGContainerOffset und PicStream
    // mit Hilfe einer Init Routine Uebergeben.
    SvxMSDffManager( SvStream& rStCtrl, const String& rBaseURL, MSFilterTracer* pTracer );
    void InitSvxMSDffManager(sal_uInt32 nOffsDgg_, SvStream* pStData_, sal_uInt32 nSvxMSDffOLEConvFlags);
    void SetDgContainer( SvStream& rSt );

    virtual ~SvxMSDffManager();

    sal_uInt32  GetSvxMSDffSettings() const { return nSvxMSDffSettings; };
    void    SetSvxMSDffSettings( sal_uInt32 nSettings ) { nSvxMSDffSettings = nSettings; };

    static sal_Bool     MakeContentStream( SotStorage * pStor, const GDIMetaFile & );
    static sal_Bool     ConvertToOle2( SvStream& rStm, sal_uInt32 nLen, const GDIMetaFile*,
                                const SotStorageRef & rDest );

    void SetModel(SdrModel* pModel, long nApplicationScale);
    SdrModel*  GetModel() const { return pSdrModel; }
    void Scale(sal_Int32& rVal) const;
    void Scale(Point& rPos) const;
    void Scale(Size& rSiz) const;
    void Scale(Rectangle& rRect) const;
    void Scale(Polygon& rPoly) const;
    void Scale(PolyPolygon& rPoly) const;
    void ScaleEmu(sal_Int32& rVal) const;
    sal_uInt32 ScalePt( sal_uInt32 nPt ) const;
    sal_Int32 ScalePoint( sal_Int32 nVal ) const;

/*
    GetBLIP()           - Anforderung eines bestimmten BLIP
    =========
    Input:  nIdx        - Nummer des angeforderten BLIP
                          ( muss immer uebergeben werden )

    Output: rData       - bereits fertig konvertierte Daten
                          ( direkt als Grafik in unsere Dokumente einzusetzen )

    Rueckgabewert: sal_True, im Erfolgsfalls, sal_False bei Fehler
*/
    sal_Bool GetBLIP( sal_uLong nIdx, Graphic& rData, Rectangle* pVisArea = NULL );

/*
    GetBLIPDirect()     -Einlesen eines BLIP aus schon positioniertem Stream
    ===============
    Input:  rBLIPStream -bereits korrekt positionierter Stream
                          ( muss immer uebergeben werden )

    Output: rData       -bereits fertig konvertierte Daten
                          ( direkt als Grafik in unsere Dokumente einzusetzen )

    Rueckgabewert: sal_True, im Erfolgsfalls, sal_False bei Fehler
*/
    sal_Bool GetBLIPDirect(SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea = NULL ) const;

    sal_Bool GetShape(sal_uLong nId,
                  SdrObject*& rpData, SvxMSDffImportData& rData);

/*
    GetBLIPCount()  - Abfrage der verwalteten BLIP Anzahl
    ==============
    Input:  ./.
    Output: ./.
    Rueckgabewert: nBLIPCount   - Anzahl der im pStData (bzw. rStCtrl) enthaltenen BLIPs
                                  ( sprich: Anzahl der FBSEs im Drawing Group Container )

                        Werte:  0   - Struktur Ok, jedoch keine BLIPs vorhanden
                                1.. - Anzahl der BLIPs
                        USHRT_MAX   - Fehler: kein korrektes Drawing File Format
*/
    sal_uInt16 GetBLIPCount() const{ return nBLIPCount; }

/*
    ZCodecDecompressed()  - Dekomprimierung eines komp. WMF oder Enhanced WMF
    ====================
    Input:  rIn     -bereits korrekt positionierter Stream,
                     der das komprimierte Bild enthaelt
            rOut    -bereits korrekt positionierter Ausgabe-Stream,

        bLookForEnd -Flag, ob das komp. Bild bis zum Stream-Ende reicht.
                     Falls sal_True, wird jeweils geprueft, ob das gelesene noch
                                                        zum Bild gehoert.
                     Falls sal_False, wird bis zum Stream-Ende gelesen.

    Output: rIn     -Der Stream steht hinter dem Ende des komp. Bildes.
                     (es kann aber noch eine Ende-Kennung und CRC-Sum folgen)
            rOut    -Der Stream enthaelt das dekomprimierte Bild.
                     Der Stream wird auf den Anfang des Bildes positioniert.
                     (also dorthin, wo der Stream vor der Verarbeitung stand)

    Rueckgabewert:  sal_True, im Erfolgsfall
                    sal_False bei Fehler oder Null Bytes geschrieben
*/
//  static sal_Bool ZCodecDecompressed( SvStream& rIn,
//                                  SvStream& rOut,
//                                  sal_Bool bLookForEnd );
//
    SdrObject* ImportObj(SvStream& rSt, void* pData,
        Rectangle& rClientRect, const Rectangle& rGlobalChildRect, int nCalledByGroup = 0, sal_Int32* pShapeId = NULL);

    SdrObject* ImportGroup( const DffRecordHeader& rHd, SvStream& rSt, void* pData,
        Rectangle& rClientRect, const Rectangle& rGlobalChildRect, int nCalledByGroup = 0, sal_Int32* pShapeId = NULL );

    SdrObject* ImportShape( const DffRecordHeader& rHd, SvStream& rSt, void* pData,
        Rectangle& rClientRect, const Rectangle& rGlobalChildRect, int nCalledByGroup = 0, sal_Int32* pShapeId = NULL);

    Rectangle GetGlobalChildAnchor( const DffRecordHeader& rHd, SvStream& rSt, Rectangle& aClientRect );
    void GetGroupAnchors( const DffRecordHeader& rHd, SvStream& rSt,
                                Rectangle& rGroupClientAnchor, Rectangle& rGroupChildAnchor,
                                    const Rectangle& rClientRect, const Rectangle& rGlobalChildRect );

    inline const SvxMSDffShapeInfos* GetShapeInfos( void ) const
        {
            return pShapeInfos;
        }

    inline const SvxMSDffShapeOrders* GetShapeOrders( void ) const
        {
            return pShapeOrders;
        }

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

    /* the method SolveSolver will create connections between shapes, it should be called after a page is imported.
    The SvxMSDffSolverContainer is containing necessary data data that is collected during the import of each shape
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
    sal_uInt16 nBLIPType;   // Art des BLIP: z.B. 6 fuer PNG
    sal_uLong  nFilePos;    // Offset des BLIP im Daten-Stream
    sal_uLong  nBLIPSize;   // Anzahl Bytes, die der BLIP im Stream einnimmt
    SvxMSDffBLIPInfo(sal_uInt16 nBType, sal_uLong nFPos, sal_uLong nBSize):
        nBLIPType( nBType ), nFilePos( nFPos ), nBLIPSize( nBSize ){}
};

struct SvxMSDffShapeInfo
{
    sal_uInt32 nShapeId;     // Shape Id, verwendet im PLCF SPA und im mso_fbtSp (FSP)
    sal_uLong nFilePos;  // Offset des Shape im Kontroll-Stream fuer eventuelle
                     // erneute Zugriffe auf dieses Shape
    sal_uInt32 nTxBxComp;

    sal_Bool bReplaceByFly  :1; // Shape darf im Writer durch Rahmen ersetzt werden.
    sal_Bool bSortByShapeId :1;
    sal_Bool bLastBoxInChain:1;

    SvxMSDffShapeInfo(sal_uLong nFPos, sal_uInt32 nId=0, // sal_uLong nBIdx=0,
                      sal_uInt16 nSeqId=0, sal_uInt16 nBoxId=0):
        nShapeId( nId ),
        nFilePos( nFPos ),
        nTxBxComp( (nSeqId << 16) + nBoxId )
        {
            bReplaceByFly   = sal_False;
            bSortByShapeId  = sal_False;
            bLastBoxInChain = sal_True;
        }
    SvxMSDffShapeInfo(SvxMSDffShapeInfo& rInfo):
        nShapeId( rInfo.nShapeId ),
        nFilePos( rInfo.nFilePos ),
        nTxBxComp( rInfo.nTxBxComp )
        {
            bReplaceByFly   = rInfo.bReplaceByFly;
            bSortByShapeId  = rInfo.bSortByShapeId;
            bLastBoxInChain = rInfo.bLastBoxInChain;
        }
    sal_Bool operator==( const SvxMSDffShapeInfo& rEntry ) const
    {
        return bSortByShapeId ? (nShapeId  == rEntry.nShapeId)
                              : (nTxBxComp == rEntry.nTxBxComp && this == &rEntry);
    }
    sal_Bool operator<( const SvxMSDffShapeInfo& rEntry ) const
    {
        return bSortByShapeId ? (nShapeId  < rEntry.nShapeId)
                              : (nTxBxComp < rEntry.nTxBxComp);
    }
};

struct SvxMSDffShapeOrder
{
    sal_uLong nShapeId;     // Shape Id, verwendet im PLCF SPA und im mso_fbtSp (FSP)

    sal_uLong nTxBxComp;    // Ketten- und Boxnummer in der Text-Box-Story (bzw. Null)

    SwFlyFrmFmt* pFly;  // Frame-Format eines statt des Sdr-Text-Objektes im
                        // Writer eingefuegten Rahmens: zur Verkettung benoetigt!

    short nHdFtSection; // used by Writer to find out if linked frames are in the
                        // same header or footer of the same section

    SdrObject*  pObj;   // Zeiger auf das Draw-Objekt (bzw. Null, falls nicht verwendet)

    // Vorgehensweise:  im Ctor des SvxMSDffManager werden im der Shape-Order-Array
    //                  nur die Shape-Ids vermerkt,
    //                  Text-Box-Nummer und der Objekt-Pointer werden nur dann
    //                  gespeichert, wenn das Shape tatsaechlich importiert wird!
    SvxMSDffShapeOrder( sal_uLong nId ):
        nShapeId( nId ), nTxBxComp( 0 ), pFly( 0 ), nHdFtSection( 0 ), pObj( 0 ){}

    sal_Bool operator==( const SvxMSDffShapeOrder& rEntry ) const
    {
        return (nTxBxComp == rEntry.nTxBxComp);
    }
    sal_Bool operator<( const SvxMSDffShapeOrder& rEntry ) const
    {
        return (nTxBxComp < rEntry.nTxBxComp);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
