/*************************************************************************
 *
 *  $RCSfile: ndgrf.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-31 15:39:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _NDGRF_HXX
#define _NDGRF_HXX


#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#ifndef _GRFMGR_HXX //autogen
#include <goodies/grfmgr.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif

class SwGrfFmtColl;
class SwDoc;
class GraphicAttr;

// --------------------
// SwGrfNode
// --------------------
class SwGrfNode: public SwNoTxtNode
{
    friend class SwNodes;
    friend class SwGrfFrm;

    GraphicObject aGrfObj;
    SvBaseLinkRef refLink;      // falls Grafik nur als Link, dann Pointer gesetzt
    Size nGrfSize;
    String aStrmName;           // SW3: Name des Storage-Streams fuer Embedded
    String aNewStrmName;        // SW3: name des Storage-Streams bei SaveAs
    String aLowResGrf;          // HTML: LowRes Grafik (Ersatzdarstellung bis
                                //      die normale (HighRes) geladen ist.

    BOOL bTransparentFlagValid  :1;
    BOOL bInSwapIn              :1;
    BOOL bGrafikArrived         :1;
    BOOL bChgTwipSize           :1;
    BOOL bLoadLowResGrf         :1;
    BOOL bFrameInPaint          :1; //Um Start-/EndActions im Paint (ueber
                                    //SwapIn zu verhindern.
    BOOL bScaleImageMap         :1; //Image-Map in SetTwipSize skalieren

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

    void InsertLink( const String& rGrfName, const String& rFltName );
    void DelStreamName();

    DECL_LINK( SwapGraphic, GraphicObject* );

public:
    virtual ~SwGrfNode();

    const Graphic&          GetGrf() const      { return aGrfObj.GetGraphic(); }
    const GraphicObject&    GetGrfObj() const   { return aGrfObj; }
          GraphicObject&    GetGrfObj()         { return aGrfObj; }

    virtual SwCntntNode *SplitNode( const SwPosition & );

    virtual Size GetTwipSize() const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    void SetTwipSize( const Size& rSz );

    BOOL IsTransparent() const;

    inline BOOL IsAnimated() const              { return aGrfObj.IsAnimated(); }

    inline BOOL IsChgTwipSize() const           { return bChgTwipSize; }
    inline void SetChgTwipSize( BOOL b )        { bChgTwipSize = b; }

    inline BOOL IsGrafikArrived() const         { return bGrafikArrived; }
    inline void SetGrafikArrived( BOOL b )      { bGrafikArrived = b; }

    inline BOOL IsFrameInPaint() const          { return bFrameInPaint; }
    inline void SetFrameInPaint( BOOL b )       { bFrameInPaint = b; }

    inline BOOL IsScaleImageMap() const         { return bScaleImageMap; }
    inline void SetScaleImageMap( BOOL b )      { bScaleImageMap = b; }

    // alles fuers Laden der LowRes-Grafiken
    inline BOOL IsLoadLowResGrf() const         { return bLoadLowResGrf; }
    inline void SetLoadLowResGrf( BOOL b )      { bLoadLowResGrf = b; }
    const String& GetLowResGrfName() const      { return aLowResGrf; }
    void SetLowResGrfName( const String& r )    { aLowResGrf = r; }
#endif
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
        // erneutes Einlesen, falls Graphic nicht Ok ist. Die
        // aktuelle wird durch die neue ersetzt.
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    BOOL ReRead( const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic = 0, BOOL bModify = TRUE );
        // Laden der Grafik unmittelbar vor der Anzeige
    short SwapIn( BOOL bWaitForData = FALSE );
        // Entfernen der Grafik, um Speicher freizugeben
    short SwapOut();
        // Schreiben der Grafik
    BOOL StoreGraphics( SvStorage* pDocStg = NULL );
        // Zugriff auf den Storage-Streamnamen
    const String& GetStreamName() const;
    void SetStreamName( const String& r ) { aStrmName = r; }
    void SaveCompleted( BOOL bClear );
#endif

        // Der Grafik sagen, dass sich der Node im Undobereich befindet
    virtual BOOL SavePersistentData();
    virtual BOOL RestorePersistentData();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
        // Abfrage der Link-Daten
    BOOL IsGrfLink() const                  { return refLink.Is(); }
    inline BOOL IsLinkedFile() const;
    inline BOOL IsLinkedDDE() const;
    SvBaseLinkRef GetLink() const               { return refLink; }
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
