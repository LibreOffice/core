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

#ifndef INCLUDED_VCL_INC_IMPOCTREE_HXX
#define INCLUDED_VCL_INC_IMPOCTREE_HXX

#include "octree.hxx"

class ImpErrorQuad
{
    long nRed;
    long nGreen;
    long nBlue;

public:

    ImpErrorQuad()
        : nRed(0)
        , nGreen(0)
        , nBlue(0)
    {
    }

    ImpErrorQuad( const BitmapColor& rColor )
        : nRed( static_cast<long>(rColor.GetRed()) << 5 )
        , nGreen( static_cast<long>(rColor.GetGreen()) << 5 )
        , nBlue( static_cast<long>(rColor.GetBlue()) << 5 )
    {
    }

    inline void             operator=( const BitmapColor& rColor );
    inline ImpErrorQuad&    operator-=( const BitmapColor& rColor );

    inline void             ImplAddColorError1( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError3( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError5( const ImpErrorQuad& rErrQuad );
    inline void             ImplAddColorError7( const ImpErrorQuad& rErrQuad );

    inline BitmapColor      ImplGetColor();
};

inline void ImpErrorQuad::operator=( const BitmapColor& rColor )
{
    nRed = static_cast<long>(rColor.GetRed()) << 5;
    nGreen = static_cast<long>(rColor.GetGreen()) << 5;
    nBlue = static_cast<long>(rColor.GetBlue()) << 5;
}

inline ImpErrorQuad& ImpErrorQuad::operator-=( const BitmapColor& rColor )
{
    nRed -= ( static_cast<long>(rColor.GetRed()) << 5 );
    nGreen -= ( static_cast<long>(rColor.GetGreen()) << 5 );
    nBlue -= ( static_cast<long>(rColor.GetBlue()) << 5 );

    return *this;
}

inline void ImpErrorQuad::ImplAddColorError1( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed >> 4 );
    nGreen += ( rErrQuad.nGreen >> 4 );
    nBlue += ( rErrQuad.nBlue >> 4 );
}

inline void ImpErrorQuad::ImplAddColorError3( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 3L >> 4 );
    nGreen += ( rErrQuad.nGreen * 3L >> 4 );
    nBlue += ( rErrQuad.nBlue * 3L >> 4 );
}

inline void ImpErrorQuad::ImplAddColorError5( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 5L >> 4 );
    nGreen += ( rErrQuad.nGreen * 5L >> 4 );
    nBlue += ( rErrQuad.nBlue * 5L >> 4 );
}

inline void ImpErrorQuad::ImplAddColorError7( const ImpErrorQuad& rErrQuad )
{
    nRed += ( rErrQuad.nRed * 7L >> 4 );
    nGreen += ( rErrQuad.nGreen * 7L >> 4 );
    nBlue += ( rErrQuad.nBlue *7L >> 4 );
}

inline BitmapColor ImpErrorQuad::ImplGetColor()
{
    return BitmapColor( static_cast<sal_uInt8>( ( nRed < 0 ? 0L : nRed > 8160 ? 8160L : nRed ) >> 5 ),
                        static_cast<sal_uInt8>( ( nGreen < 0 ? 0L : nGreen > 8160 ? 8160L : nGreen ) >> 5 ),
                        static_cast<sal_uInt8>( ( nBlue < 0 ? 0L : nBlue > 8160 ? 8160L : nBlue ) >> 5 ) );
}

class ImpNodeCache
{
    OctreeNode*         pActNode;

public:

                        ImpNodeCache( const sal_uLong nInitSize );
                        ~ImpNodeCache();

    inline OctreeNode*  ImplGetFreeNode();
    inline void         ImplReleaseNode( OctreeNode* pNode );
};

inline OctreeNode* ImpNodeCache::ImplGetFreeNode()
{
    OctreeNode* pNode;

    if ( !pActNode )
    {
        pActNode = new NODE;
        pActNode->pNextInCache = nullptr;
    }

    pNode = pActNode;
    pActNode = pNode->pNextInCache;
    memset( pNode, 0, sizeof( NODE ) );

    return pNode;
}

inline void ImpNodeCache::ImplReleaseNode( OctreeNode* pNode )
{
    pNode->pNextInCache = pActNode;
    pActNode = pNode;
}

#endif // INCLUDED_VCL_INC_IMPOCTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
