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
#ifndef INCLUDED_SW_INC_NDNOTXT_HXX
#define INCLUDED_SW_INC_NDNOTXT_HXX

#include "node.hxx"

namespace tools { class PolyPolygon; }

// SwNoTxtNode

class SW_DLLPUBLIC SwNoTxtNode : public SwCntntNode
{
    friend class SwNodes;
    friend class SwNoTxtFrm;

    tools::PolyPolygon *pContour;
    bool bAutomaticContour : 1; // automatic contour polygon, not manipulated
    bool bContourMapModeValid : 1; // contour map mode is not the graphics's
                                   // preferred map mode, but either
                                      // MM100 or or pixel
    bool bPixelContour : 1;     // contour map mode is invalid and pixel.

    // Creates for all derivations an AttrSet with ranges for frame- and
    // graphics-attributes (only called by SwCntntNode).
    virtual void NewAttrSet( SwAttrPool& ) SAL_OVERRIDE;

    SwNoTxtNode( const SwNoTxtNode& );              // Not allowed.
    SwNoTxtNode &operator=( const SwNoTxtNode& );   // Not allowed.

protected:
    SwNoTxtNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType,
                SwGrfFmtColl *pGrColl, SwAttrSet* pAutoAttr = 0 );

public:
    virtual ~SwNoTxtNode();

    virtual SwCntntFrm *MakeFrm( SwFrm* ) SAL_OVERRIDE;

    inline SwGrfFmtColl *GetGrfColl() const { return (SwGrfFmtColl*)GetRegisteredIn(); }

    virtual Size GetTwipSize() const = 0;

    virtual bool SavePersistentData();
    virtual bool RestorePersistentData();

    OUString GetTitle() const;
    void SetTitle( const OUString& rTitle, bool bBroadcast = false );
    OUString GetDescription() const;
    void SetDescription( const OUString& rDescription, bool bBroadcast = false );

    void               SetContour( const tools::PolyPolygon *pPoly,
                                   bool bAutomatic = false );
    const tools::PolyPolygon *HasContour() const;
    bool               _HasContour() const { return pContour!=0; };
    void               GetContour( tools::PolyPolygon &rPoly ) const;
    void               CreateContour();

    void               SetAutomaticContour( bool bSet ) { bAutomaticContour = bSet; }
    bool               HasAutomaticContour() const { return bAutomaticContour; }

    // set either a MM100 or pixel contour
    void               SetContourAPI( const tools::PolyPolygon *pPoly );

    // get either a MM100 or pixel contour, return sal_False if no contour is set.
    bool               GetContourAPI( tools::PolyPolygon &rPoly ) const;

    void               SetPixelContour( bool bSet ) { bPixelContour = bSet; }
    bool               IsPixelContour() const;

    bool               IsContourMapModeValid() const { return bContourMapModeValid; }

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

#endif // INCLUDED_SW_INC_NDNOTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
