/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ndgrf.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:30:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _NDGRF_HXX
#define _NDGRF_HXX
#ifndef _LNKBASE_HXX //autogen
#include <sfx2/lnkbase.hxx>
#endif
#ifndef _GRFMGR_HXX //autogen
#include <goodies/grfmgr.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
// --> OD, MAV 2005-08-17 #i53025#
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
// <--
// --> OD 2007-03-28 #i73788#
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
class SwAsyncRetrieveInputStreamThreadConsumer;
// <--

class SwGrfFmtColl;
class SwDoc;
class GraphicAttr;
class SvStorage;
// --------------------
// SwGrfNode
// --------------------
class SwGrfNode: public SwNoTxtNode
{
    friend class SwNodes;

    GraphicObject aGrfObj;
    ::sfx2::SvBaseLinkRef refLink;       // falls Grafik nur als Link, dann Pointer gesetzt
    Size nGrfSize;
//  String aStrmName;           // SW3: Name des Storage-Streams fuer Embedded
    String aNewStrmName;        // SW3/XML: new stream name (either SW3 stream
                                // name or package url)
    String aLowResGrf;          // HTML: LowRes Grafik (Ersatzdarstellung bis
                                //      die normale (HighRes) geladen ist.
    BOOL bTransparentFlagValid  :1;
    BOOL bInSwapIn              :1;

    BOOL bGrafikArrived         :1;
    BOOL bChgTwipSize           :1;
    BOOL bChgTwipSizeFromPixel  :1;
    BOOL bLoadLowResGrf         :1;
    BOOL bFrameInPaint          :1; //Um Start-/EndActions im Paint (ueber
                                    //SwapIn zu verhindern.
    BOOL bScaleImageMap         :1; //Image-Map in SetTwipSize skalieren

    // --> OD 2007-01-19 #i73788#
    boost::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
    bool mbLinkedInputStreamReady :1;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> mxInputStream;
    sal_Bool mbIsStreamReadOnly;
    // <--
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
               const GraphicObject& rGrfObj,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );

    void InsertLink( const String& rGrfName, const String& rFltName );
    BOOL ImportGraphic( SvStream& rStrm );
    BOOL HasStreamName() const { return aGrfObj.HasUserData(); }
    // --> OD 2005-05-04 #i48434# - adjust return type and rename method to
    // indicate that its an private one.
    // --> OD 2005-08-17 #i53025#
    // embedded graphic stream couldn't be inside a 3.1 - 5.2 storage any more.
    // Thus, return value isn't needed any more.
    void _GetStreamStorageNames( String& rStrmName, String& rStgName ) const;
    // <--
    void DelStreamName();
    DECL_LINK( SwapGraphic, GraphicObject* );

    /** helper method to determine stream for the embedded graphic.

        OD 2005-05-04 #i48434#
        Important note: caller of this method has to handle the thrown exceptions
        OD, MAV 2005-08-17 #i53025#
        Storage, which should contain the stream of the embedded graphic, is
        provided via parameter. Otherwise the returned stream will be closed
        after the the method returns, because its parent stream is closed and deleted.
        Proposed name of embedded graphic stream is also provided by parameter.

        @author OD

        @param _refPics
        input parameter - reference to storage, which should contain the
        embedded graphic stream.

        @param _aStrmName
        input parameter - proposed name of the embedded graphic stream.

        @return SvStream*
        new created stream of the embedded graphic, which has to be destroyed
        after its usage. Could be NULL, if the stream isn't found.
    */
    SvStream* _GetStreamForEmbedGrf(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _refPics,
            String& _aStrmName ) const;

    /** helper method to get a substorage of the document storage for readonly access.

        OD, MAV 2005-08-17 #i53025#
        A substorage with the specified name will be opened readonly. If the provided
        name is empty the root storage will be returned.

        @param _aStgName
        input parameter - name of substorage. Can be empty.

        @return XStorage
        reference to substorage or the root storage
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > _GetDocSubstorageOrRoot(
                                                const String& aStgName ) const;

public:
    virtual ~SwGrfNode();
    const Graphic&          GetGrf() const      { return aGrfObj.GetGraphic(); }
    const GraphicObject&    GetGrfObj() const   { return aGrfObj; }
          GraphicObject&    GetGrfObj()         { return aGrfObj; }

    virtual SwCntntNode *SplitCntntNode( const SwPosition & );

    virtual Size GetTwipSize() const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    void SetTwipSize( const Size& rSz );

    BOOL IsTransparent() const;

    inline BOOL IsAnimated() const              { return aGrfObj.IsAnimated(); }

    inline BOOL IsChgTwipSize() const           { return bChgTwipSize; }
    inline BOOL IsChgTwipSizeFromPixel() const  { return bChgTwipSizeFromPixel; }
    inline void SetChgTwipSize( BOOL b, BOOL bFromPx=FALSE )        { bChgTwipSize = b; bChgTwipSizeFromPixel = bFromPx; }

    inline BOOL IsGrafikArrived() const         { return bGrafikArrived; }
    inline void SetGrafikArrived( BOOL b )      { bGrafikArrived = b; }

    inline BOOL IsFrameInPaint() const          { return bFrameInPaint; }
    inline void SetFrameInPaint( BOOL b )       { bFrameInPaint = b; }

    inline BOOL IsScaleImageMap() const         { return bScaleImageMap; }
    inline void SetScaleImageMap( BOOL b )      { bScaleImageMap = b; }
#endif
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED

    // erneutes Einlesen, falls Graphic nicht Ok ist. Die
    // aktuelle wird durch die neue ersetzt.
    BOOL ReRead( const String& rGrfName, const String& rFltName,
                 const Graphic* pGraphic = 0,
                 const GraphicObject* pGrfObj = 0,
                 BOOL bModify = TRUE );
    // Laden der Grafik unmittelbar vor der Anzeige
    short SwapIn( BOOL bWaitForData = FALSE );
        // Entfernen der Grafik, um Speicher freizugeben
    short SwapOut();
        // Zugriff auf den Storage-Streamnamen
    void SetStreamName( const String& r ) { aGrfObj.SetUserData( r ); }
    void SetNewStreamName( const String& r ) { aNewStrmName = r; }
    // is this node selected by any shell?
    BOOL IsSelected() const;
#endif

        // Der Grafik sagen, dass sich der Node im Undobereich befindet
    virtual BOOL SavePersistentData();
    virtual BOOL RestorePersistentData();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
        // Abfrage der Link-Daten
    BOOL IsGrfLink() const                  { return refLink.Is(); }
    inline BOOL IsLinkedFile() const;
    inline BOOL IsLinkedDDE() const;
    ::sfx2::SvBaseLinkRef GetLink() const    { return refLink; }
    BOOL GetFileFilterNms( String* pFileNm, String* pFilterNm ) const;
    void ReleaseLink();

        // Prioritaet beim Laden der Grafik setzen. Geht nur, wenn der Link
        // ein FileObject gesetzt hat
    void SetTransferPriority( USHORT nPrio );

    // Skalieren einer Image-Map: Die Image-Map wird um den Faktor
    // zwischen Grafik-Groesse und Rahmen-Groesse vergroessert/verkleinert
    void ScaleImageMap();

    // returns the with our graphic attributes filled Graphic-Attr-Structure
    GraphicAttr& GetGraphicAttr( GraphicAttr&, const SwFrm* pFrm ) const;

#endif
    // --> OD 2007-01-18 #i73788#
    boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > GetThreadConsumer();
    bool IsLinkedInputStreamReady() const;
    void TriggerAsyncRetrieveInputStream();
    void ApplyInputStream(
        com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
        const sal_Bool bIsStreamReadOnly );
    void UpdateLinkWithInputStream();
    // <--
};


// ----------------------------------------------------------------------
// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
inline       SwGrfNode   *SwNode::GetGrfNode()
{
     return ND_GRFNODE == nNodeType ? (SwGrfNode*)this : 0;
}
inline const SwGrfNode   *SwNode::GetGrfNode() const
{
     return ND_GRFNODE == nNodeType ? (const SwGrfNode*)this : 0;
}

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


#endif
