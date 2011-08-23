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
#include <tools/solar.h>		// UINTXX
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
    BYTE    nRecVer; // may be DFF_PSFLAG_CONTAINER
    UINT16  nRecInstance;
    UINT16  nImpVerInst;
    UINT16  nRecType;
    UINT32  nRecLen;
    ULONG	nFilePos;
public:
    DffRecordHeader() : nRecVer(0), nRecInstance(0), nImpVerInst(0), nRecType(0), nRecLen(0), nFilePos(0) {}
    FASTBOOL IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    ULONG    GetRecBegFilePos() const { return nFilePos; }
    ULONG    GetRecEndFilePos() const { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
    void SeekToEndOfRecord(SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen ); }
    void SeekToContent(    SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE ); }
    void SeekToBegOfRecord(SvStream& rIn) const { rIn.Seek( nFilePos ); }

    MSFILTER_DLLPUBLIC friend SvStream& operator>>(SvStream& rIn, DffRecordHeader& rRec);

};

struct DffPropFlags
{
    BYTE	bSet		: 1;
    BYTE	bComplex	: 1;
    BYTE	bBlip		: 1;
    BYTE	bSoftAttr	: 1;
};

class SvxMSDffManager;

class MSFILTER_DLLPUBLIC DffPropSet : public Table
{
    protected :

        UINT32			mpContents[ 1024 ];
        DffPropFlags	mpFlags[ 1024 ];

    public :

        DffPropSet( BOOL bInitialize = FALSE ){ if ( bInitialize )
                                                memset( mpFlags, 0, 0x400 * sizeof( DffPropFlags ) ); };

        inline BOOL	IsProperty( UINT32 nRecType ) const { return ( mpFlags[ nRecType & 0x3ff ].bSet ); };
        BOOL		IsHardAttribute( UINT32 nId ) const;
        UINT32		GetPropertyValue( UINT32 nId, UINT32 nDefault = 0 ) const;
        /** Returns a boolean property by its real identifier. */
        bool        GetPropertyBool( UINT32 nId, bool bDefault = false ) const;
        /** Returns a string property. */
        ::rtl::OUString GetPropertyString( UINT32 nId, SvStream& rStrm ) const;
        void		SetPropertyValue( UINT32 nId, UINT32 nValue ) const;
        BOOL		SeekToContent( UINT32 nRecType, SvStream& rSt ) const;
        void		Merge( DffPropSet& rMasterPropSet ) const;
        void		InitializePropSet() const;
        friend SvStream& operator>>( SvStream& rIn, DffPropSet& rPropSet );
};

class SfxItemSet;
class SdrObject;
struct DffObjData;

class MSFILTER_DLLPUBLIC DffPropertyReader : public DffPropSet
{
    const SvxMSDffManager&	rManager;
    DffPropSet*				pDefaultPropSet;

    void		ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const;
    void		ApplyCustomShapeAdjustmentAttributes( SfxItemSet& rSet ) const;
    void		ApplyCustomShapeGeometryAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;
    void		ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const; // #i28269#
    void		ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;

public:

    INT32					mnFix16Angle;

    DffPropertyReader( const SvxMSDffManager& rManager );
    ~DffPropertyReader();
    INT32		Fix16ToAngle( INT32 nAngle ) const;

#ifdef DBG_CUSTOMSHAPE
    void		ReadPropSet( SvStream& rIn, void* pClientData, UINT32 nShapeType = 0 ) const;
#else
    void		ReadPropSet( SvStream& rIn, void* pClientData ) const;
#endif

    void		SetDefaultPropSet( SvStream& rIn, UINT32 nOffDgg ) const;
    void		ApplyAttributes( SvStream& rIn, SfxItemSet& rSet ) const;
    void		ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const;
};


#define COL_DEFAULT RGB_COLORDATA( 0xFA, 0xFB, 0xFC )

typedef SvxMSDffBLIPInfo*  SvxMSDffBLIPInfo_Ptr;

typedef SvxMSDffShapeInfo* SvxMSDffShapeInfo_Ptr;

typedef SvxMSDffShapeOrder* SvxMSDffShapeOrder_Ptr;

typedef ::std::map< sal_Int32, SdrObject* > SvxMSDffShapeIdContainer;

#define SVEXT_PERSIST_STREAM "\002OlePres000"

// nach der Reihenfolge des Auftretens sortiert werden:
//
SV_DECL_PTRARR_DEL(SvxMSDffBLIPInfos,	SvxMSDffBLIPInfo_Ptr,	16,16)

SV_DECL_PTRARR_DEL(SvxMSDffShapeOrders,	SvxMSDffShapeOrder_Ptr,	16,16)

// explizit sortiert werden:
//
SV_DECL_PTRARR_SORT_DEL_VISIBILITY(SvxMSDffShapeInfos,	SvxMSDffShapeInfo_Ptr,	16,16, MSFILTER_DLLPUBLIC)

SV_DECL_PTRARR_SORT_VISIBILITY(SvxMSDffShapeTxBxSort,	SvxMSDffShapeOrder_Ptr,	16,16, MSFILTER_DLLPUBLIC)

#define SVXMSDFF_SETTINGS_CROP_BITMAPS		1
#define SVXMSDFF_SETTINGS_IMPORT_PPT		2
#define SVXMSDFF_SETTINGS_IMPORT_EXCEL		4

#define SP_FGROUP		0x001	// This shape is a group shape
#define SP_FCHILD		0x002	// Not a top-level shape
#define SP_FPATRIARCH	0x004	// This is the topmost group shape.
                                // Exactly one of these per drawing.
#define SP_FDELETED		0x008	// The shape has been deleted
#define SP_FOLESHAPE	0x010	// The shape is an OLE object
#define SP_FHAVEMASTER	0x020	// Shape has a hspMaster property
#define SP_FFLIPH		0x040	// Shape is flipped horizontally
#define SP_FFLIPV		0x080	// Shape is flipped vertically
#define SP_FCONNECTOR	0x100	// Connector type of shape
#define SP_FHAVEANCHOR	0x200	// Shape has an anchor of some kind
#define SP_FBACKGROUND	0x400	// Background shape
#define SP_FHAVESPT		0x800	// Shape has a shape type property

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_MATHTYPE_2_STARMATH				0x0001
#define OLE_WINWORD_2_STARWRITER			0x0002
#define OLE_EXCEL_2_STARCALC				0x0004
#define OLE_POWERPOINT_2_STARIMPRESS		0x0008

struct SvxMSDffConnectorRule
{
    sal_uInt32	nRuleId;
    sal_uInt32  nShapeA;		// SPID of shape A

    sal_uInt32  nShapeB;		// SPID of shape B
    sal_uInt32  nShapeC;		// SPID of connector shape
    sal_uInt32  ncptiA;			// Connection site Index of shape A
    sal_uInt32  ncptiB;			// Connection site Index of shape B
    sal_uInt32  nSpFlagsA;      // SpFlags of shape A ( the original mirror flags must be known when solving the Solver Container )
    sal_uInt32  nSpFlagsB;      // SpFlags of shape A

    SdrObject* pAObj;		// pPtr of object ( corresponding to shape A )
    SdrObject* pBObj;		//	 "
    SdrObject* pCObj;		//	 "  of connector object

    SvxMSDffConnectorRule() : nSpFlagsA( 0 ), nSpFlagsB( 0 ), pAObj( NULL ), pBObj( NULL ), pCObj( NULL ) {};

    friend SvStream& operator>>( SvStream& rIn, SvxMSDffConnectorRule& rAtom );
};
struct MSFILTER_DLLPUBLIC SvxMSDffSolverContainer
{
    List	aCList;

            SvxMSDffSolverContainer();
            ~SvxMSDffSolverContainer();

    MSFILTER_DLLPUBLIC friend SvStream& operator>>( SvStream& rIn, SvxMSDffSolverContainer& rAtom );
};

struct FIDCL
{
    UINT32	dgid;		// DG owning the SPIDs in this cluster
    UINT32	cspidCur;	// number of SPIDs used so far
};

//---------------------------------------------------------------------------
//  von SvxMSDffManager fuer jedes in der Gruppe enthaltene Shape geliefert
//---------------------------------------------------------------------------
struct MSDffTxId
{
    USHORT nTxBxS;
    USHORT nSequence;
    MSDffTxId(USHORT nTxBxS_, USHORT nSequence_ )
            : nTxBxS(			  nTxBxS_		),
              nSequence(		  nSequence_	){}
    MSDffTxId(const MSDffTxId& rCopy)
            : nTxBxS(			  rCopy.nTxBxS    ),
              nSequence(		  rCopy.nSequence ){}
};

struct MSFILTER_DLLPUBLIC SvxMSDffImportRec
{
    SdrObject*	pObj;
    Polygon*	pWrapPolygon;
    char*		pClientAnchorBuffer;
    UINT32		nClientAnchorLen;
    char*		pClientDataBuffer;
    UINT32		nClientDataLen;
    UINT32		nXAlign;
    UINT32		nXRelTo;
    UINT32		nYAlign;
    UINT32		nYRelTo;
    UINT32		nLayoutInTableCell;
    UINT32      nFlags;
    long		nTextRotationAngle;
    long		nDxTextLeft;	// Abstand der Textbox vom umgebenden Shape
    long		nDyTextTop;
    long		nDxTextRight;
    long		nDyTextBottom;
    long		nDxWrapDistLeft;
    long		nDyWrapDistTop;
    long		nDxWrapDistRight;
    long		nDyWrapDistBottom;
    long		nCropFromTop;
    long		nCropFromBottom;
    long		nCropFromLeft;
    long		nCropFromRight;
    MSDffTxId	aTextId;		// Kennungen fuer Textboxen
    ULONG		nNextShapeId;	// fuer verlinkte Textboxen
    ULONG		nShapeId;
    MSO_SPT		eShapeType;
    MSO_LineStyle eLineStyle;	// Umrandungs-Arten
    BOOL		bDrawHell		:1;
    BOOL		bHidden			:1;
    BOOL		bReplaceByFly	:1;
    BOOL		bLastBoxInChain	:1;
    BOOL		bHasUDefProp	:1;
    BOOL 		bVFlip :1;
    BOOL 		bHFlip :1;
    BOOL		bAutoWidth      :1;

    SvxMSDffImportRec();
    SvxMSDffImportRec(const SvxMSDffImportRec& rCopy);
    ~SvxMSDffImportRec();
    BOOL operator==( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId == rEntry.nShapeId; }
    BOOL operator<( const SvxMSDffImportRec& rEntry ) const
    {   return nShapeId < rEntry.nShapeId;  }
};
typedef SvxMSDffImportRec* MSDffImportRec_Ptr;

// Liste aller SvxMSDffImportRec fuer eine Gruppe
SV_DECL_PTRARR_SORT_DEL_VISIBILITY(MSDffImportRecords, MSDffImportRec_Ptr, 16,16, MSFILTER_DLLPUBLIC)

//---------------------------------------------------------------------------
//   Import-/Export-Parameterblock fuer 1 x ImportObjAtCurrentStreamPos()
//---------------------------------------------------------------------------
struct SvxMSDffImportData
{
    MSDffImportRecords	aRecords;	// Shape-Pointer, -Ids und private Daten
    Rectangle			aParentRect;// Rectangle der umgebenden Gruppe
                                    // bzw. von aussen reingegebenes Rect
    Rectangle 			aNewRect;	// mit diesem Shape definiertes Rectangle

    SvxMSDffImportData()
        {}
    SvxMSDffImportData(const Rectangle& rParentRect)
        :aParentRect( rParentRect )
        {}
    void SetNewRect(INT32 l, INT32 o,
                    INT32 r, INT32 u ){	aNewRect = Rectangle(l,o, r,u); }
    BOOL HasParRect() const { return aParentRect.IsEmpty(); }
    BOOL HasNewRect() const { return aNewRect.IsEmpty()   ; }
    BOOL HasRecords() const { return 0 != aRecords.Count(); }
    USHORT				GetRecCount() const { return aRecords.Count();	}
    SvxMSDffImportRec*	GetRecord(USHORT iRecord) const
                            {  return aRecords.GetObject( iRecord ); }
};

struct DffObjData
{
    const DffRecordHeader&	rSpHd;

    Rectangle	aBoundRect;
    Rectangle	aChildAnchor;

    UINT32		nShapeId;
    UINT32		nSpFlags;
    MSO_SPT		eShapeType;

    BOOL bShapeType		: 1;
    BOOL bClientAnchor	: 1;
    BOOL bClientData	: 1;
    BOOL bChildAnchor	: 1;
    BOOL bOpt			: 1;
    BOOL bIsAutoText	: 1;

    int nCalledByGroup;

    DffObjData( const DffRecordHeader& rObjHd,
                const Rectangle& rBoundRect,
                int 			 nClByGroup ) :
        rSpHd( rObjHd ),
        aBoundRect(	rBoundRect ),
        nShapeId( 0 ),
        nSpFlags( 0 ),
        eShapeType( mso_sptNil ),
        bShapeType( FALSE ),
        bClientAnchor( FALSE ),
        bClientData( FALSE ),
        bChildAnchor( FALSE ),
        bOpt( FALSE ),
        bIsAutoText( FALSE ),
        nCalledByGroup(	nClByGroup ){}
};

#define DFF_RECORD_MANAGER_BUF_SIZE			64

struct DffRecordList
{
        UINT32				nCount;
        UINT32				nCurrent;
        DffRecordList*		pPrev;
        DffRecordList*		pNext;

        DffRecordHeader		mHd[ DFF_RECORD_MANAGER_BUF_SIZE ];

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

        DffRecordList*		pCList;

        void				Clear();
        void				Consume( SvStream& rIn, BOOL bAppend = FALSE, UINT32 nStOfs = 0 );

        BOOL				SeekToContent( SvStream& rIn, UINT16 nRecType, DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );
        DffRecordHeader*	GetRecordHeader( UINT16 nRecType,  DffSeekToContentMode eMode = SEEK_FROM_BEGINNING );

                            DffRecordManager();
                            DffRecordManager( SvStream& rIn );
                            ~DffRecordManager();

        DffRecordHeader*	Current();
        DffRecordHeader*	First();
        DffRecordHeader*	Next();
        DffRecordHeader*	Prev();
        DffRecordHeader*	Last();
};

/*
    SvxMSDffManager - abstrakte Basis-Klasse fuer Escher-Import
    ===============
    Zweck:	Zugriff auf Objekte im Drawing File Format
    Stand:	Zugriff nur auf BLIPs (wird spaeter erweitert)

    Beachte: in der zwecks PowerPoint-, ODER Word- ODER Excel-Import
    ======== abgeleiteten Klasse
             MUSS jeweils die Methode ProcessUserDefinedRecord()
             implementiert werden!
*/
class MSFILTER_DLLPUBLIC SvxMSDffManager : public DffPropertyReader
{
    FmFormModel*			pFormModel;
    SvxMSDffBLIPInfos*		pBLIPInfos;
    SvxMSDffShapeInfos*		pShapeInfos;
    SvxMSDffShapeOrders*	pShapeOrders;
    ULONG					nDefaultFontHeight;
    long					nOffsDgg;
    USHORT					nBLIPCount;
    USHORT					nShapeCount;
    sal_uInt32				nGroupShapeFlags;

    void CheckTxBxStoryChain();
    void GetFidclData( long nOffsDgg );

protected :

    String                  maBaseURL;
    UINT32                  mnCurMaxShapeId;    // we need this information to
    UINT32					mnDrawingsSaved;	// access the right drawing
    UINT32					mnIdClusters;		// while only knowing the shapeid
    FIDCL*					mpFidcls;
    Table					maDgOffsetTable;	// array of fileoffsets

    friend class DffPropertyReader;

    SvStream&		rStCtrl;
    SvStream*		pStData;
    SvStream*		pStData2;
    SdrModel*		pSdrModel;

    long			nMapMul;
    long			nMapDiv;
    long			nMapXOfs;
    long			nMapYOfs;
    long			nEmuMul;
    long			nEmuDiv;
    long			nPntMul;
    long			nPntDiv;
    FASTBOOL		bNeedMap;
    UINT32			nSvxMSDffSettings;
    UINT32			nSvxMSDffOLEConvFlags;

    /** stores a reference to an imported SdrObject with its shape id if
        it has one
    */
    SvxMSDffShapeIdContainer	maShapeIdContainer;

    void GetCtrlData( long nOffsDgg );
    void GetDrawingGroupContainerData( SvStream& rSt,
                                       ULONG nLenDgg );
    // --> OD 2008-08-01 #156763#
    // Add internal drawing container id as parameter to the sub methods of
    // reading the control information about the drawing objects.
    // The drawing container id is used to distinguish the text ids of drawing
    // objects in different drawing containers.
    void GetDrawingContainerData( SvStream& rSt,
                                  ULONG nLenDg,
                                  const unsigned long nDrawingContainerId );
    BOOL GetShapeGroupContainerData( SvStream& rSt,
                                     ULONG nLenShapeGroupCont,
                                     BOOL bPatriarch,
                                     const unsigned long nDrawingContainerId );
    BOOL GetShapeContainerData( SvStream& rSt,
                                ULONG nLenShapeCont,
                                ULONG nPosGroup,
                                const unsigned long nDrawingContainerId );
    // <--

    FASTBOOL ReadGraphic( SvStream& rSt, ULONG nIndex, Graphic& rGraphic ) const;
    SdrObject* ImportFontWork( SvStream&, SfxItemSet&, Rectangle& rBoundRect ) const;
    SdrObject* ImportGraphic( SvStream&, SfxItemSet&, const DffObjData& ) const;
    // --> OD 2004-12-14 #i32596# - pass <nCalledByGroup> to method
    // Needed in the Writer Microsoft Word import to avoid import of OLE objects
    // inside groups. Instead a graphic object is created.
    virtual SdrObject* ImportOLE( long nOLEId,
                                  const Graphic& rGraf,
                                  const Rectangle& rBoundRect,
                                  const Rectangle& rVisArea,
                                  const int _nCalledByGroup,
                                  sal_Int64 nAspect ) const;
    // <--
    SdrObject* GetAutoForm( MSO_SPT eTyp ) const;
    static const GDIMetaFile* lcl_GetMetaFileFromGrf_Impl( const Graphic& rGrf, GDIMetaFile& rMtf );
#ifndef SVX_LIGHT
    static com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > CheckForConvertToSOObj(
                UINT32 nConvertFlags, SotStorage& rSrcStg,
                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg,
                const Graphic& rGrf,
                const Rectangle& rVisArea );
#endif

/*
        folgende Methoden sind zum Excel-Import zu ueberschreiben:
*/
    virtual BOOL ProcessClientAnchor(SvStream& rStData, ULONG nDatLen, char*& rpBuff, UINT32& rBuffLen ) const;
    virtual void ProcessClientAnchor2( SvStream& rStData, DffRecordHeader& rHd, void* pData, DffObjData& );
    virtual BOOL ProcessClientData(  SvStream& rStData, ULONG nDatLen, char*& rpBuff, UINT32& rBuffLen ) const;
    virtual SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj = NULL);
    virtual ULONG Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const;
    virtual FASTBOOL GetColorFromPalette(USHORT nNum, Color& rColor) const;

    // -----------------------------------------------------------------------

    FASTBOOL ReadDffString(SvStream& rSt, String& rTxt) const;
    FASTBOOL ReadObjText(SvStream& rSt, SdrObject* pObj) const;

    // SJ: New implementation of ReadObjText is used by Fontwork objects, because
    // the old one does not properly import multiple paragraphs
    void ReadObjText( const String& rText, SdrObject* pObj ) const;
    // -----------------------------------------------------------------------

    /*
        folgende Methode ist von allen zu ueberschreiben, die OLE-Objecte
        importieren moechten:
    */
    virtual BOOL GetOLEStorageName( long nOLEId, String& rStorageName,
                                    SotStorageRef& rSrcStorage,
                com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg
                                    ) const;

    /*
        folgende Methode ist von allen zu ueberschreiben, die verhindern
        moechten, dass (abgerundete) Rechtecke mit umgebrochenem Text
        immer in SdrRectObj( OBJ_TEXT ) umgewandelt werden:
        TRUE bedeutet umwandeln.
    */
    virtual BOOL ShapeHasText(ULONG nShapeId, ULONG nFilePos) const;

public:

    void*				pSvxMSDffDummy1;
    void*				pSvxMSDffDummy2;
    void*				pSvxMSDffDummy3;
    List*				pEscherBlipCache;

    DffRecordManager	maShapeRecords;
    ColorData			mnDefaultColor;

    MSFilterTracer*		mpTracer;
    sal_Bool			mbTracing;

    Color MSO_TEXT_CLR_ToColor( sal_uInt32 nColorCode ) const;
    Color MSO_CLR_ToColor( sal_uInt32 nColorCode, sal_uInt16 nContextProperty = DFF_Prop_lineColor ) const;
    virtual BOOL SeekToShape( SvStream& rSt, void* pClientData, UINT32 nId ) const;
    FASTBOOL SeekToRec( SvStream& rSt, USHORT nRecId, ULONG nMaxFilePos, DffRecordHeader* pRecHd = NULL, ULONG nSkipCount = 0 ) const;
    FASTBOOL SeekToRec2( USHORT nRecId1, USHORT nRecId2, ULONG nMaxFilePos, DffRecordHeader* pRecHd = NULL, ULONG nSkipCount = 0 ) const;

    // -----------------------------------------------------------------------
    static void MSDFFReadZString( SvStream& rIn, String& rStr, ULONG nMaxLen, FASTBOOL bUniCode = FALSE );

    static BOOL ReadCommonRecordHeader( DffRecordHeader& rRec, SvStream& rIn );
    static BOOL ReadCommonRecordHeader( SvStream& rSt,
                                        BYTE&     rVer,
                                        USHORT&   rInst,
                                        USHORT&   rFbt,
                                        UINT32&    rLength );
/*
    Konstruktor
    ===========
    Input:	rStCtrl	  - Verwaltungsstream mit Containern,
                        FBSE Objekten und Shapes
                        ( muss immer uebergeben werden;
                          Stream muss bereits offen sein )

            nOffsDgg  -	Offset im rStCtrl: Beginn des Drawing Group Containers

            pStData	  - Datenstream, in dem die BLIPs gespeichert sind
                        ( falls Null, wird angenommen, dass die
                          BLIPs ebenfalls im rStCtrl gespeichert sind;
                          dieser Stream muss ebenfalls bereits offen sein )
            pSdrModel_  kann beim Ctor-Aufruf leer bleiben, muss dann aber
                        spaeter ueber SetModel() gesetzt werden!

    setzt nBLIPCount
*/
    SvxMSDffManager( SvStream& rStCtrl,
                     const String& rBaseURL,
                     long      nOffsDgg,
                     SvStream* pStData,
                     SdrModel* pSdrModel_			=  0,
                     long      nApplicationScale	=  0,
                     ColorData mnDefaultColor_		=  COL_DEFAULT,
                     ULONG     nDefaultFontHeight_	= 24,
                     SvStream* pStData2_			=  0,
                     MSFilterTracer* pTracer		= NULL );

    // in PPT werden die Parameter DGGContainerOffset und PicStream
    // mit Hilfe einer Init Routine Uebergeben.
    SvxMSDffManager( SvStream& rStCtrl, const String& rBaseURL, MSFilterTracer* pTracer );
    void InitSvxMSDffManager( long nOffsDgg_, SvStream* pStData_, sal_uInt32 nSvxMSDffOLEConvFlags );
    void SetDgContainer( SvStream& rSt );

    virtual ~SvxMSDffManager();

    UINT32	GetSvxMSDffSettings() const { return nSvxMSDffSettings; };
    void	SetSvxMSDffSettings( UINT32 nSettings ) { nSvxMSDffSettings = nSettings; };

    static BOOL     MakeContentStream( SotStorage * pStor, const GDIMetaFile & );
    static BOOL     ConvertToOle2( SvStream& rStm, UINT32 nLen, const GDIMetaFile*,
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
    UINT32 ScalePt( UINT32 nPt ) const;
    INT32 ScalePoint( INT32 nVal ) const;

/*
    GetBLIP()			- Anforderung eines bestimmten BLIP
    =========
    Input:	nIdx		- Nummer des angeforderten BLIP
                          ( muss immer uebergeben werden )

    Output:	rData		- bereits fertig konvertierte Daten
                          ( direkt als Grafik in unsere Dokumente einzusetzen )

    Rueckgabewert: TRUE, im Erfolgsfalls, FALSE bei Fehler
*/
    BOOL GetBLIP( ULONG nIdx, Graphic& rData, Rectangle* pVisArea = NULL ) const;

/*
    GetBLIPDirect()		-Einlesen eines BLIP aus schon positioniertem Stream
    ===============
    Input:	rBLIPStream	-bereits korrekt positionierter Stream
                          ( muss immer uebergeben werden )

    Output:	rData		-bereits fertig konvertierte Daten
                          ( direkt als Grafik in unsere Dokumente einzusetzen )

    Rueckgabewert: TRUE, im Erfolgsfalls, FALSE bei Fehler
*/
    BOOL GetBLIPDirect(SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea = NULL ) const;

    BOOL GetShape(ULONG nId,
                  SdrObject*& rpData, SvxMSDffImportData& rData);

/*
    GetBLIPCount()	- Abfrage der verwalteten BLIP Anzahl
    ==============
    Input:	./.
    Output:	./.
    Rueckgabewert: nBLIPCount	- Anzahl der im pStData (bzw. rStCtrl) enthaltenen BLIPs
                                  ( sprich: Anzahl der FBSEs im Drawing Group Container )

                        Werte:	0	- Struktur Ok, jedoch keine BLIPs vorhanden
                                1..	- Anzahl der BLIPs
                        USHRT_MAX	- Fehler: kein korrektes Drawing File Format
*/
    USHORT GetBLIPCount() const{ return nBLIPCount; }

/*
    ZCodecDecompressed()  - Dekomprimierung eines komp. WMF oder Enhanced WMF
    ====================
    Input:	rIn		-bereits korrekt positionierter Stream,
                     der das komprimierte Bild enthaelt
            rOut	-bereits korrekt positionierter Ausgabe-Stream,

        bLookForEnd -Flag, ob das komp. Bild bis zum Stream-Ende reicht.
                     Falls TRUE, wird jeweils geprueft, ob das gelesene	noch
                                                        zum Bild gehoert.
                     Falls FALSE, wird bis zum Stream-Ende gelesen.

    Output:	rIn     -Der Stream steht hinter dem Ende des komp. Bildes.
                     (es kann aber noch eine Ende-Kennung und CRC-Sum folgen)
            rOut	-Der Stream enthaelt das dekomprimierte Bild.
                     Der Stream wird auf den Anfang des Bildes positioniert.
                     (also dorthin, wo der Stream vor der Verarbeitung stand)

    Rueckgabewert:	TRUE, im Erfolgsfall
                    FALSE bei Fehler oder Null Bytes geschrieben
*/
//	static BOOL ZCodecDecompressed(	SvStream& rIn,
//									SvStream& rOut,
//									BOOL bLookForEnd );
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

    void StoreShapeOrder(ULONG			nId,
                         ULONG			nTxBx,
                         SdrObject*		pObject,
                         SwFlyFrmFmt*	pFly = 0,
                         short			nHdFtSection = 0) const;

    void ExchangeInShapeOrder(SdrObject* pOldObject,
                              ULONG nTxBx,
                              SwFlyFrmFmt* pFly,
                              SdrObject*   pObject) const;

    void RemoveFromShapeOrder( SdrObject* pObject ) const;

    UINT32	GetConvertFlags() const { return nSvxMSDffOLEConvFlags; }

    static SdrOle2Obj* CreateSdrOLEFromStorage( const String& rStorageName,
                                                SotStorageRef& rSrcStorage,
                                                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xDestStg,
                                                const Graphic& rGraf,
                                                const Rectangle& rBoundRect,
                                                const Rectangle& rVisArea,
                                                SvStream* pDataStrrm,
                                                ErrCode& rError,
                                                UINT32 nConvertFlags,
                                                sal_Int64 nAspect );

    /* the method SolveSolver will create connections between shapes, it should be called after a page is imported.
    The SvxMSDffSolverContainer is containing necessary data data that is collected during the import of each shape
    */
    void SolveSolver( const SvxMSDffSolverContainer& rSolver );

    static sal_Bool SetPropValue(
        const ::com::sun::star::uno::Any& rAny,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                const String& rPropertyName,
                    sal_Bool bTestPropertyAvailability = sal_False );

    void insertShapeId( sal_Int32 nShapeId, SdrObject* pShape );
    void removeShapeId( SdrObject* pShape );
    SdrObject* getShapeForId( sal_Int32 nShapeId );
};

struct SvxMSDffBLIPInfo
{
    USHORT nBLIPType;	// Art des BLIP: z.B. 6 fuer PNG
    ULONG  nFilePos;	// Offset des BLIP im Daten-Stream
    ULONG  nBLIPSize;	// Anzahl Bytes, die der BLIP im Stream einnimmt
    SvxMSDffBLIPInfo(USHORT nBType, ULONG nFPos, ULONG nBSize):
        nBLIPType( nBType ), nFilePos( nFPos ), nBLIPSize( nBSize ){}
};

struct SvxMSDffShapeInfo
{
    sal_uInt32 nShapeId;	 // Shape Id, verwendet im PLCF SPA und im mso_fbtSp (FSP)
    ULONG nFilePos;	 // Offset des Shape im Kontroll-Stream fuer eventuelle
                     // erneute Zugriffe auf dieses Shape
    sal_uInt32 nTxBxComp;

    BOOL bReplaceByFly  :1; // Shape darf im Writer durch Rahmen ersetzt werden.
    BOOL bSortByShapeId :1;
    BOOL bLastBoxInChain:1;

    SvxMSDffShapeInfo(ULONG nFPos, sal_uInt32 nId=0, // ULONG nBIdx=0,
                      USHORT nSeqId=0, USHORT nBoxId=0):
        nShapeId( nId ),
        nFilePos( nFPos ),
        nTxBxComp( (nSeqId << 16) + nBoxId )
        {
            bReplaceByFly	= FALSE;
            bSortByShapeId	= FALSE;
            bLastBoxInChain	= TRUE;
        }
    SvxMSDffShapeInfo(SvxMSDffShapeInfo& rInfo):
        nShapeId( rInfo.nShapeId ),
        nFilePos( rInfo.nFilePos ),
        nTxBxComp( rInfo.nTxBxComp )
        {
            bReplaceByFly	= rInfo.bReplaceByFly;
            bSortByShapeId	= rInfo.bSortByShapeId;
            bLastBoxInChain	= rInfo.bLastBoxInChain;
        }
    BOOL operator==( const SvxMSDffShapeInfo& rEntry ) const
    {
        return bSortByShapeId ? (nShapeId  == rEntry.nShapeId)
                              : (nTxBxComp == rEntry.nTxBxComp && this == &rEntry);
    }
    BOOL operator<( const SvxMSDffShapeInfo& rEntry ) const
    {
        return bSortByShapeId ? (nShapeId  < rEntry.nShapeId)
                              : (nTxBxComp < rEntry.nTxBxComp);
    }
};

struct SvxMSDffShapeOrder
{
    ULONG nShapeId;		// Shape Id, verwendet im PLCF SPA und im mso_fbtSp (FSP)

    ULONG nTxBxComp;	// Ketten- und Boxnummer in der Text-Box-Story (bzw. Null)

    SwFlyFrmFmt* pFly;	// Frame-Format eines statt des Sdr-Text-Objektes im
                        // Writer eingefuegten Rahmens: zur Verkettung benoetigt!

    short nHdFtSection; // used by Writer to find out if linked frames are in the
                        // same header or footer of the same section

    SdrObject*	pObj;	// Zeiger auf das Draw-Objekt (bzw. Null, falls nicht verwendet)

    // Vorgehensweise:  im Ctor des SvxMSDffManager werden im der Shape-Order-Array
    //					nur die Shape-Ids vermerkt,
    //					Text-Box-Nummer und der Objekt-Pointer werden nur dann
    //					gespeichert, wenn das Shape tatsaechlich importiert wird!
    SvxMSDffShapeOrder( ULONG nId ):
        nShapeId( nId ), nTxBxComp( 0 ), pFly( 0 ), nHdFtSection( 0 ), pObj( 0 ){}

    BOOL operator==( const SvxMSDffShapeOrder& rEntry ) const
    {
        return (nTxBxComp == rEntry.nTxBxComp);
    }
    BOOL operator<( const SvxMSDffShapeOrder& rEntry ) const
    {
        return (nTxBxComp < rEntry.nTxBxComp);
    }
};

#endif

