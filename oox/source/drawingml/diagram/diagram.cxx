/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <functional>
#include <boost/bind.hpp>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::com::sun::star;

namespace oox { namespace drawingml {

namespace dgm {


void Connection::dump()
{
    OSL_TRACE("dgm: cnx modelId %s, srcId %s, dstId %s",
              OUSTRING_TO_CSTR( msModelId ),
              OUSTRING_TO_CSTR( msSourceId ),
              OUSTRING_TO_CSTR( msDestId ) );
}

Point::Point()
    : mpShape( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) )
    , mnType( 0 )
{
}

void Point::dump()
{
    OSL_TRACE( "dgm: pt cnxId %s, modelId %s",
               OUSTRING_TO_CSTR( msCnxId ),
               OUSTRING_TO_CSTR( msModelId ) );
}

void Point::setModelId( const ::rtl::OUString & sModelId )
{
    msModelId = sModelId;
    mpShape->setName( msModelId );
}


bool PointsTree::addChild( const PointsTreePtr & pChild )
{
    bool added = false;

    OSL_ENSURE( pChild->mpParent.expired(), "can't add, has already a parent" );
    OSL_ENSURE( mpNode, "has no node" );
    if( mpNode && pChild->mpParent.expired() )
    {
        pChild->mpParent = shared_from_this();
        maChildrens.push_back( pChild );
        added = true;
    }

    return added;
}

PointsTreePtr PointsTree::getParent() const
{
    if( !mpParent.expired() )
    {
        return mpParent.lock() ;
    }
    return PointsTreePtr();
}


} // dgm namespace

DiagramData::DiagramData()
    : mpFillProperties( new FillProperties )
{
}

void DiagramData::dump()
{
    OSL_TRACE("Dgm: DiagramData # of cnx: %d", maConnections.size() );
    std::for_each( maConnections.begin(), maConnections.end(),
                  boost::bind( &dgm::Connection::dump, _1 ) );
    OSL_TRACE("Dgm: DiagramData # of pt: %d", maPoints.size() );
    std::for_each( maPoints.begin(), maPoints.end(),
                  boost::bind( &dgm::Point::dump, _1 ) );
}

static void setPosition( const dgm::PointPtr & pPoint, const awt::Point & pt )
{
    ShapePtr pShape = pPoint->getShape();
    awt::Size sz;
    sz.Width = 50;
    sz.Height = 50;
    pShape->setPosition( pt );
    pShape->setSize( sz );
}

void DiagramLayout::layout( const dgm::PointsTreePtr & pTree, const awt::Point & pt )
{
    setPosition( pTree->getPoint(), pt );
    awt::Point nextPt = pt;
    nextPt.Y += 50;
    dgm::PointsTree::Childrens::const_iterator iter;
    for( iter = pTree->beginChild(); iter != pTree->endChild(); iter++ )
    {
        layout( *iter, nextPt );
        nextPt.X += 50;
    }
}

void Diagram::setData( const DiagramDataPtr & pData)
{
    mpData = pData;
}


void Diagram::setLayout( const DiagramLayoutPtr & pLayout)
{
    mpLayout = pLayout;
}

void Diagram::setQStyles( const DiagramQStylesPtr & pStyles)
{
    mpQStyles = pStyles;
}


void Diagram::setColors( const DiagramColorsPtr & pColors)
{
    mpColors = pColors;
}

void Diagram::build(  )
{
    OSL_TRACE( "building diagram" );
    typedef std::map< OUString, dgm::PointPtr > PointsMap;
    PointsMap aPointsMap;
    dgm::Points::iterator aPointsIter( mpData->getPoints( ).begin() );
    for( ; aPointsIter != mpData->getPoints( ).end() ; aPointsIter++ )
    {
        const OUString & sName((*aPointsIter)->getModelId());
        if( sName.getLength() > 0 )
        {
            aPointsMap[ sName ] = *aPointsIter;
        }
    }

    typedef std::map< OUString, dgm::PointsTreePtr > PointsTreeMap;
    PointsTreeMap aTreeMap;
    PointsTreeMap aRoots;

    dgm::Connections & aConnections(mpData->getConnections( ) );
    dgm::Connections::iterator aCnxIter;
    for( aCnxIter = aConnections.begin(); aCnxIter != aConnections.end(); ++aCnxIter )
    {
        OSL_ENSURE( *aCnxIter, "NULL connection found" );
        if( (*aCnxIter)->mnType != XML_parOf )
        {
//			OSL_TRACE( "ignoring relation %s", OUSTRING_TO_CSTR( (*aCnxIter)->msModelId ) );
            continue;
        }
        dgm::PointPtr pDest;
        dgm::PointsTreePtr pSource;
        PointsMap::iterator iterP;
        OUString & srcId( (*aCnxIter)->msSourceId );
        OUString & dstId( (*aCnxIter)->msDestId );
        OSL_TRACE( "connexion %s -> %s", OUSTRING_TO_CSTR( srcId ),
                   OUSTRING_TO_CSTR( dstId ) );

        PointsTreeMap::iterator iterT = aTreeMap.find( srcId );
        if( iterT != aTreeMap.end() )
        {
            pSource = iterT->second;
        }
        else
        {
            // this tree node is not found. create it with the source
            // and make it the root node.
            iterP = aPointsMap.find( srcId );
            if( iterP != aPointsMap.end() )
            {
                pSource.reset( new dgm::PointsTree( iterP->second ) );
                aRoots[ srcId ] = pSource;
                aTreeMap[ srcId ] = pSource;
            }
            else
            {
                OSL_TRACE("parent node not found !");
            }
        }
        iterP = aPointsMap.find( dstId );
        if( iterP != aPointsMap.end() )
        {
            pDest = iterP->second;
        }
        OSL_ENSURE( pDest, "destination not found" );
        OSL_ENSURE( pSource, "source not found" );
        if(pDest && pSource)
        {
            dgm::PointsTreePtr pNode( new dgm::PointsTree( pDest ) );
            bool added = pSource->addChild( pNode );
            (void)added;
            aRoots.erase( dstId );
            OSL_ENSURE( added, "add child failed" );
            aTreeMap[ dstId ] = pNode;
        }
    }
    // check bounds
    OSL_ENSURE( aRoots.size() == 1, "more than one root" );
    // #i92239# roots may be empty
    if( !aRoots.empty() )
    {
        mpRoot = aRoots.begin()->second;
        OSL_TRACE( "root is %s", OUSTRING_TO_CSTR( mpRoot->getPoint()->getModelId() ) );
        for( PointsTreeMap::iterator iter = aTreeMap.begin();
             iter != aTreeMap.end(); iter++ )
        {
            if(! iter->second->getParent() )
            {
                OSL_TRACE("node without parent %s", OUSTRING_TO_CSTR( iter->first ) );
            }
        }
    }
}


void Diagram::addTo( const ShapePtr & pParentShape )
{
    dgm::Points & aPoints( mpData->getPoints( ) );
    dgm::Points::iterator aPointsIter;
    build( );
    if( mpRoot.get() )
        mpLayout->layout( mpRoot, awt::Point( 0, 0 ) );

    for( aPointsIter = aPoints.begin(); aPointsIter != aPoints.end(); ++aPointsIter )
    {
        if( ( *aPointsIter )->getType() != XML_node )
        {
            continue;
        }
        ShapePtr pShape = ( *aPointsIter )->getShape( );
        if( pShape->getName( ).getLength() > 0 )
        {
            maShapeMap[ pShape->getName( ) ] = pShape;
            OSL_TRACE( "Dgm: added shape %s to map", OUSTRING_TO_CSTR( pShape->getName() ) );
        }
        pParentShape->addChild( pShape );
    }

    OSL_TRACE( "Dgm: addTo() # of childs %d", pParentShape->getChildren().size() );
    for( std::vector< ShapePtr >::iterator iter = pParentShape->getChildren().begin();
         iter != pParentShape->getChildren().end(); ++iter)
    {
        OSL_TRACE( "Dgm: shape name %s", OUSTRING_TO_CSTR( (*iter)->getName() ) );
    }
}

OUString Diagram::getLayoutId() const
{
    OUString sLayoutId;
    if( mpLayout )
    {
        sLayoutId = mpLayout->getUniqueId();
    }
    return sLayoutId;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
