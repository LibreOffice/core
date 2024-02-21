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

#include <optional>
#include <tools/poly.hxx>
#include "node.hxx"

class Size;

/// Layout frame for SwNoTextNode, i.e. graphics and OLE nodes (including charts).
class SAL_DLLPUBLIC_RTTI SwNoTextNode : public SwContentNode
{
    friend class SwNodes;
    friend class SwNoTextFrame;

    mutable std::optional<tools::PolyPolygon> m_pContour;
    bool m_bAutomaticContour : 1; // automatic contour polygon, not manipulated
    mutable bool m_bContourMapModeValid : 1; // contour map mode is not the graphics's
                                   // preferred map mode, but either
                                      // MM100 or pixel
    mutable bool m_bPixelContour : 1;     // contour map mode is invalid and pixel.

    // Creates for all derivations an AttrSet with ranges for frame- and
    // graphics-attributes (only called by SwContentNode).
    virtual void NewAttrSet( SwAttrPool& ) override;

    SwNoTextNode( const SwNoTextNode& ) = delete;
    SwNoTextNode &operator=( const SwNoTextNode& ) = delete;

protected:
    SwNoTextNode( SwNode& rWhere, const SwNodeType nNdType,
                SwGrfFormatColl *pGrColl, SwAttrSet const * pAutoAttr );

public:
    virtual ~SwNoTextNode() override;

    virtual SwContentFrame *MakeFrame( SwFrame* ) override;

    SwGrfFormatColl *GetGrfColl() const { return const_cast<SwGrfFormatColl*>(static_cast<const SwGrfFormatColl*>(GetRegisteredIn())); }

    virtual Size GetTwipSize() const = 0;

    virtual bool SavePersistentData();
    virtual bool RestorePersistentData();

    SW_DLLPUBLIC OUString GetTitle() const;
    void SetTitle( const OUString& rTitle );
    SW_DLLPUBLIC OUString GetDescription() const;
    void SetDescription( const OUString& rDescription );

    SW_DLLPUBLIC void  SetContour( const tools::PolyPolygon *pPoly,
                                   bool bAutomatic = false );
    SW_DLLPUBLIC const tools::PolyPolygon *HasContour() const;
    bool               HasContour_() const { return bool(m_pContour); };
    void               GetContour( tools::PolyPolygon &rPoly ) const;
    void               CreateContour();

    void               SetAutomaticContour( bool bSet ) { m_bAutomaticContour = bSet; }
    bool               HasAutomaticContour() const { return m_bAutomaticContour; }

    // set either a MM100 or pixel contour
    void               SetContourAPI( const tools::PolyPolygon *pPoly );

    // get either a MM100 or pixel contour, return false if no contour is set.
    bool               GetContourAPI( tools::PolyPolygon &rPoly ) const;

    void               SetPixelContour( bool bSet ) { m_bPixelContour = bSet; }
    bool               IsPixelContour() const;

    bool               IsContourMapModeValid() const { return m_bContourMapModeValid; }

    // Obtains the graphic with SwapIn for GrfNode via GetData for OLE.
    SW_DLLPUBLIC Graphic GetGraphic() const;
};

// Inline methods from Node.hxx - we know TextNode only here!!
inline SwNoTextNode *SwNode::GetNoTextNode()
{
    return SwNodeType::NoTextMask & m_nNodeType ? static_cast<SwNoTextNode*>(this) : nullptr;
}
inline const SwNoTextNode *SwNode::GetNoTextNode() const
{
    return SwNodeType::NoTextMask & m_nNodeType ? static_cast<const SwNoTextNode*>(this) : nullptr;
}

#endif // INCLUDED_SW_INC_NDNOTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
