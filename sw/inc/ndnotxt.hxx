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
#ifndef _NDNOTXT_HXX
#define _NDNOTXT_HXX

#include "node.hxx"

class PolyPolygon;

// SwNoTxtNode

class SW_DLLPUBLIC SwNoTxtNode : public SwCntntNode
{
    friend class SwNodes;
    friend class SwNoTxtFrm;

    PolyPolygon *pContour;
    sal_Bool bAutomaticContour : 1; // automatic contour polygon, not manipulated
    sal_Bool bContourMapModeValid : 1; // contour map mode is not the graphics's
                                   // prefered map mode, but either
                                      // MM100 or or pixel
    sal_Bool bPixelContour : 1;     // contour map mode is invalid and pixel.

    // Creates for all derivations an AttrSet with ranges for frame- and
    // graphics-attributes (only called by SwCntntNode).
    virtual void NewAttrSet( SwAttrPool& );

    SwNoTxtNode( const SwNoTxtNode& );              // Not allowed.
    SwNoTxtNode &operator=( const SwNoTxtNode& );   // Not allowed.

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

    OUString GetTitle() const;
    void SetTitle( const OUString& rTitle, bool bBroadcast = false );
    OUString GetDescription() const;
    void SetDescription( const OUString& rDescription, bool bBroadcast = false );

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

    // Obtains the graphic with SwapIn for GrfNode via GetData for OLE.
    Graphic GetGraphic() const;
};

// Inline methods from Node.hxx - we know TxtNode only here!!
inline SwNoTxtNode *SwNode::GetNoTxtNode()
{
    return ND_NOTXTNODE & nNodeType ? (SwNoTxtNode*)this : 0;
}
inline const SwNoTxtNode *SwNode::GetNoTxtNode() const
{
    return ND_NOTXTNODE & nNodeType ? (const SwNoTxtNode*)this : 0;
}

#endif  // _NDNOTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
