/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _NDNOTXT_HXX
#define _NDNOTXT_HXX

#include "node.hxx"

class PolyPolygon;

// --------------------
// SwNoTxtNode
// --------------------

class SW_DLLPUBLIC SwNoTxtNode : public SwCntntNode
{
    friend class SwNodes;
    friend class SwNoTxtFrm;

//    String aAlternateText;      // alternativer Text  (HTML)

    PolyPolygon *pContour;      // Polygon fuer Konturumlauf
    sal_Bool bAutomaticContour : 1; // automatic contour polygon, not manipulated
    sal_Bool bContourMapModeValid : 1; // contour map mode is not the graphics's
                                   // preferred map mode, but either
                                      // MM100 or or pixel
    sal_Bool bPixelContour : 1;     // contour map mode is invalid and pixel.

    // erzeugt fuer alle Ableitungen einen AttrSet mit Bereichen
    // fuer Frame- und Grafik-Attributen (wird nur vom SwCntntNode gerufen)
    virtual void NewAttrSet( SwAttrPool& );

    SwNoTxtNode( const SwNoTxtNode& );              //nicht erlaubt
    SwNoTxtNode &operator=( const SwNoTxtNode& );   //nicht erlaubt

protected:
    SwNoTxtNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType,
                SwGrfFmtColl *pGrColl, SwAttrSet* pAutoAttr = 0 );

public:
    ~SwNoTxtNode();

    virtual SwCntntFrm *MakeFrm( SwFrm* );

    inline SwGrfFmtColl *GetGrfColl() const { return (SwGrfFmtColl*)GetRegisteredIn(); }

    virtual Size GetTwipSize() const = 0;

    virtual sal_Bool SavePersistentData();
    virtual sal_Bool RestorePersistentData();

    const String GetTitle() const;
    void SetTitle( const String& rTitle,
                   bool bBroadcast = false );
    const String GetDescription() const;
    void SetDescription( const String& rDescription,
                         bool bBroadcast = false );

//    const String GetAlternateText() const;
//    void SetAlternateText( const String& rTxt,
//                           sal_Bool bBroadcast=sal_False );

    void               SetContour( const PolyPolygon *pPoly,
                                   sal_Bool bAutomatic = sal_False );
    const PolyPolygon *HasContour() const;
    sal_Bool               _HasContour() const { return pContour!=0; };
    void               GetContour( PolyPolygon &rPoly ) const;
    void               CreateContour();

    void               SetAutomaticContour( sal_Bool bSet ) { bAutomaticContour = bSet; }
    sal_Bool               HasAutomaticContour() const { return bAutomaticContour; }

    // set either a MM100 or pixel contour
    void               SetContourAPI( const PolyPolygon *pPoly );

    // get either a MM100 or pixel contour, return sal_False if no contour is set.
    sal_Bool               GetContourAPI( PolyPolygon &rPoly ) const;

    void               SetPixelContour( sal_Bool bSet ) { bPixelContour = bSet; }
    sal_Bool               IsPixelContour() const;

    sal_Bool               IsContourMapModeValid() const { return bContourMapModeValid; }

    //Besorgt die Graphic, mit SwapIn fuer GrfNode, per GetData fuer OLE.
    Graphic GetGraphic() const;
};

// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
inline SwNoTxtNode *SwNode::GetNoTxtNode()
{
    return ND_NOTXTNODE & nNodeType ? (SwNoTxtNode*)this : 0;
}
inline const SwNoTxtNode *SwNode::GetNoTxtNode() const
{
    return ND_NOTXTNODE & nNodeType ? (const SwNoTxtNode*)this : 0;
}

#endif  // _NDNOTXT_HXX
