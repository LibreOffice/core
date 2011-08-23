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
#ifndef _NDGRF_HXX
#define _NDGRF_HXX


#ifndef _LNKBASE_HXX //autogen
#include <bf_so3/lnkbase.hxx>
#endif
#ifndef _BF_GOODIES_GRAPHICOBJECT_HXX //autogen
#include <bf_goodies/graphicobject.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
namespace binfilter {

class SvStorage; 
class SwGrfFmtColl;
class SwDoc;
class BfGraphicAttr;


// --------------------
// SwGrfNode
// --------------------
class SwGrfNode: public SwNoTxtNode
{
    friend class SwNodes;
    friend class SwGrfFrm;

    BfGraphicObject aGrfObj;
    ::binfilter::SvBaseLinkRef refLink;		// falls Grafik nur als Link, dann Pointer gesetzt
    Size nGrfSize;
//	String aStrmName;			// SW3: Name des Storage-Streams fuer Embedded
    String aNewStrmName;		// SW3/XML: new stream name (either SW3 stream
                                // name or package url)
    String aLowResGrf;			// HTML: LowRes Grafik (Ersatzdarstellung bis
                                // 		die normale (HighRes) geladen ist.

    BOOL bTransparentFlagValid	:1;
    BOOL bInSwapIn				:1;
    BOOL bGrafikArrived			:1;
    BOOL bChgTwipSize			:1;
    BOOL bChgTwipSizeFromPixel	:1;
    BOOL bLoadLowResGrf			:1;
    BOOL bFrameInPaint			:1;	//Um Start-/EndActions im Paint (ueber
                                    //SwapIn zu verhindern.
    BOOL bScaleImageMap	 		:1; //Image-Map in SetTwipSize skalieren

    SwGrfNode( const SwNodeIndex& rWhere,
               const String& rGrfName, const String& rFltName,
               const Graphic* pGraphic,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    // Ctor fuer Einlesen (SW/G) ohne Grafik
    SwGrfNode( const SwNodeIndex& rWhere,
               const String& rGrfName, const String& rFltName,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    SwGrfNode( const SwNodeIndex& rWhere,
               const BfGraphicObject& rGrfObj,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );

    void InsertLink( const String& rGrfName, const String& rFltName );
    BOOL ImportGraphic( SvStream& rStrm ){DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 	BOOL ImportGraphic( SvStream& rStrm );
    BOOL HasStreamName() const { return aGrfObj.HasUserData(); }
    BOOL GetStreamStorageNames( String& rStrmName, String& rStgName ) const;

    DECL_LINK( SwapGraphic, BfGraphicObject* );

public:
    virtual ~SwGrfNode();

    const Graphic& 			GetGrf() const  	{ return aGrfObj.GetGraphic(); }
    const BfGraphicObject&	GetGrfObj() const  	{ return aGrfObj; }
          BfGraphicObject&	GetGrfObj() 		{ return aGrfObj; }

    virtual SwCntntNode *SplitNode( const SwPosition & );

    virtual Size GetTwipSize() const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    void SetTwipSize( const Size& rSz );

    inline BOOL IsAnimated() const				{ return aGrfObj.IsAnimated(); }

    inline BOOL IsChgTwipSize() const			{ return bChgTwipSize; }
    inline BOOL IsChgTwipSizeFromPixel() const	{ return bChgTwipSizeFromPixel; }
    inline void SetChgTwipSize( BOOL b, BOOL bFromPx=FALSE ) 		{ bChgTwipSize = b; bChgTwipSizeFromPixel = bFromPx; }

    inline BOOL IsGrafikArrived() const			{ return bGrafikArrived; }
    inline void SetGrafikArrived( BOOL b ) 		{ bGrafikArrived = b; }

    inline BOOL IsFrameInPaint() const			{ return bFrameInPaint; }
    inline void SetFrameInPaint( BOOL b )		{ bFrameInPaint = b; }

    inline BOOL IsScaleImageMap() const			{ return bScaleImageMap; }
    inline void SetScaleImageMap( BOOL b )		{ bScaleImageMap = b; }

    // alles fuers Laden der LowRes-Grafiken
    inline BOOL IsLoadLowResGrf() const			{ return bLoadLowResGrf; }
    inline void SetLoadLowResGrf( BOOL b ) 		{ bLoadLowResGrf = b; }
    const String& GetLowResGrfName() const		{ return aLowResGrf; }
    void SetLowResGrfName( const String& r ) 	{ aLowResGrf = r; }
#endif
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
        // erneutes Einlesen, falls Graphic nicht Ok ist. Die
        // aktuelle wird durch die neue ersetzt.
    BOOL ReRead( const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic = 0,
                  const BfGraphicObject* pGrfObj = 0,
                  BOOL bModify = TRUE );
        // Laden der Grafik unmittelbar vor der Anzeige
    short SwapIn( BOOL bWaitForData = FALSE );
        // Entfernen der Grafik, um Speicher freizugeben
        // Schreiben der Grafik
    BOOL StoreGraphics( SvStorage* pDocStg = NULL );
        // Zugriff auf den Storage-Streamnamen
    String GetStreamName() const;
    void SetStreamName( const String& r ) { aGrfObj.SetUserData( r ); }
    void SetNewStreamName( const String& r ) { aNewStrmName = r; }
    void SaveCompleted( BOOL bClear );
    // is this node selected by any shell?
    BOOL IsSelected() const{DBG_BF_ASSERT(0, "STRIP");return FALSE;} ;//STRIP001 	BOOL IsSelected() const;
#endif

        // Der Grafik sagen, dass sich der Node im Undobereich befindet

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
        // Abfrage der Link-Daten
    BOOL IsGrfLink() const 					{ return refLink.Is(); }
    inline BOOL IsLinkedFile() const;
    inline BOOL IsLinkedDDE() const;
    ::binfilter::SvBaseLinkRef GetLink() const 	{ return refLink; }
    BOOL GetFileFilterNms( String* pFileNm, String* pFilterNm ) const;

        // Prioritaet beim Laden der Grafik setzen. Geht nur, wenn der Link
        // ein FileObject gesetzt hat

    // Skalieren einer Image-Map: Die Image-Map wird um den Faktor
    // zwischen Grafik-Groesse und Rahmen-Groesse vergroessert/verkleinert

    // returns the with our graphic attributes filled Graphic-Attr-Structure

#endif
};


// ----------------------------------------------------------------------
// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline       SwGrfNode	 *SwNode::GetGrfNode()
{
     return ND_GRFNODE == nNodeType ? (SwGrfNode*)this : 0;
}
inline const SwGrfNode   *SwNode::GetGrfNode() const
{
     return ND_GRFNODE == nNodeType ? (const SwGrfNode*)this : 0;
}
#endif

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
inline BOOL SwGrfNode::IsLinkedFile() const
{
    return refLink.Is() && OBJECT_CLIENT_GRF == refLink->GetObjType();
}
inline BOOL SwGrfNode::IsLinkedDDE() const
{
    return refLink.Is() && OBJECT_CLIENT_DDE == refLink->GetObjType();
}
#endif


} //namespace binfilter
#endif
