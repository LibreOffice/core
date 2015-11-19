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

// SwNoTextNode

class SW_DLLPUBLIC SwNoTextNode : public SwContentNode
{
    friend class SwNodes;
    friend class SwNoTextFrame;

    tools::PolyPolygon *pContour;
    bool bAutomaticContour : 1; // automatic contour polygon, not manipulated
    bool bContourMapModeValid : 1; // contour map mode is not the graphics's
                                   // preferred map mode, but either
                                      // MM100 or pixel
    bool bPixelContour : 1;     // contour map mode is invalid and pixel.

    // Creates for all derivations an AttrSet with ranges for frame- and
    // graphics-attributes (only called by SwContentNode).
    virtual void NewAttrSet( SwAttrPool& ) override;

    SwNoTextNode( const SwNoTextNode& ) = delete;
    SwNoTextNode &operator=( const SwNoTextNode& ) = delete;

protected:
    SwNoTextNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType,
                SwGrfFormatColl *pGrColl, SwAttrSet* pAutoAttr = nullptr );

public:
    virtual ~SwNoTextNode();

    virtual SwContentFrame *MakeFrame( SwFrame* ) override;

    inline SwGrfFormatColl *GetGrfColl() const { return const_cast<SwGrfFormatColl*>(static_cast<const SwGrfFormatColl*>(GetRegisteredIn())); }

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
    bool               _HasContour() const { return pContour!=nullptr; };
    void               GetContour( tools::PolyPolygon &rPoly ) const;
    void               CreateContour();

    void               SetAutomaticContour( bool bSet ) { bAutomaticContour = bSet; }
    bool               HasAutomaticContour() const { return bAutomaticContour; }

    // set either a MM100 or pixel contour
    void               SetContourAPI( const tools::PolyPolygon *pPoly );

    // get either a MM100 or pixel contour, return false if no contour is set.
    bool               GetContourAPI( tools::PolyPolygon &rPoly ) const;

    void               SetPixelContour( bool bSet ) { bPixelContour = bSet; }
    bool               IsPixelContour() const;

    bool               IsContourMapModeValid() const { return bContourMapModeValid; }

    // Obtains the graphic with SwapIn for GrfNode via GetData for OLE.
    Graphic GetGraphic() const;
};

// Inline methods from Node.hxx - we know TextNode only here!!
inline SwNoTextNode *SwNode::GetNoTextNode()
{
    return ND_NOTXTNODE & m_nNodeType ? static_cast<SwNoTextNode*>(this) : nullptr;
}
inline const SwNoTextNode *SwNode::GetNoTextNode() const
{
    return ND_NOTXTNODE & m_nNodeType ? static_cast<const SwNoTextNode*>(this) : nullptr;
}

#endif // INCLUDED_SW_INC_NDNOTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
