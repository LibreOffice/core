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
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/textrun.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include "diagram.hxx"
#include "diagramlayoutatoms.hxx"
#include "diagramfragmenthandler.hxx"

#include <iostream>
#include <fstream>

using rtl::OUString;
using namespace ::com::sun::star;

namespace oox { namespace drawingml {

namespace dgm {


void Connection::dump()
{
    OSL_TRACE("dgm: cnx modelId %s, srcId %s, dstId %s, parTransId %s, presId %s, sibTransId %s, srcOrd %d, dstOrd %d",
              OUSTRING_TO_CSTR( msModelId ),
              OUSTRING_TO_CSTR( msSourceId ),
              OUSTRING_TO_CSTR( msDestId ),
              OUSTRING_TO_CSTR( msParTransId ),
              OUSTRING_TO_CSTR( msPresId ),
              OUSTRING_TO_CSTR( msSibTransId ),
              mnSourceOrder,
              mnDestOrder );
}

void Point::dump()
{
    OSL_TRACE( "dgm: pt text %x, cnxId %s, modelId %s, type %d",
               mpShape.get(),
               OUSTRING_TO_CSTR( msCnxId ),
               OUSTRING_TO_CSTR( msModelId ),
               mnType );
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

void DiagramLayout::layout( const dgm::Points & /*pTree*/, const awt::Point & /*pt*/ )
{
    // TODO
#if 0
    setPosition( pTree->getPoint(), pt );
    awt::Point nextPt = pt;
    nextPt.Y += 50;
    dgm::PointsTree::Childrens::const_iterator iter;
    for( iter = pTree->beginChild(); iter != pTree->endChild(); iter++ )
    {
        layout( *iter, nextPt );
        nextPt.X += 50;
    }
#endif
}

void Diagram::setData( const DiagramDataPtr & pData)
{
    mpData = pData;
}


void Diagram::setLayout( const DiagramLayoutPtr & pLayout)
{
    mpLayout = pLayout;
}

#if OSL_DEBUG_LEVEL > 1
rtl::OString normalizeDotName( const rtl::OUString& rStr )
{
    rtl::OUStringBuffer aBuf;
    aBuf.append((sal_Unicode)'N');

    const sal_Int32 nLen(rStr.getLength());
    sal_Int32 nCurrIndex(0);
    while( nCurrIndex < nLen )
    {
        const sal_Int32 aChar=rStr.iterateCodePoints(&nCurrIndex);
        if( aChar != '-' && aChar != '{' && aChar != '}' )
            aBuf.append((sal_Unicode)aChar);
    }

    return rtl::OUStringToOString(aBuf.makeStringAndClear(),
                                  RTL_TEXTENCODING_UTF8);
}
#endif

static sal_Int32 calcDepth( const rtl::OUString& rNodeName,
                            const dgm::Connections& rCnx )
{
    // find length of longest path in 'isChild' graph, ending with rNodeName
    dgm::Connections::const_iterator aCurrCxn( rCnx.begin() );
    const dgm::Connections::const_iterator aEndCxn( rCnx.end() );
    while( aCurrCxn != aEndCxn )
    {
        if( aCurrCxn->msParTransId.getLength() &&
            aCurrCxn->msSibTransId.getLength() &&
            aCurrCxn->msSourceId.getLength() &&
            aCurrCxn->msDestId.getLength() &&
            aCurrCxn->mnType != XML_presOf &&
            aCurrCxn->mnType != XML_presParOf &&
            rNodeName == aCurrCxn->msDestId )
        {
            return calcDepth(aCurrCxn->msSourceId,
                             rCnx) + 1;
        }
        ++aCurrCxn;
    }

    return 0;
}


void Diagram::build(  )
{
    // build name-object maps
    // ======================

#if OSL_DEBUG_LEVEL > 1
    std::ofstream output("/tmp/tree.dot");

    output << "digraph datatree {" << std::endl;
#endif

    dgm::Points::iterator aCurrPoint( getData()->getPoints( ).begin() );
    const dgm::Points::iterator aEndPoint( getData()->getPoints( ).end() );
    while( aCurrPoint != aEndPoint )
    {
#if OSL_DEBUG_LEVEL > 1
        output << "\t"
               << normalizeDotName(aCurrPoint->msModelId).getStr()
               << "[";

        if( aCurrPoint->msPresentationLayoutName.getLength() )
            output << "label=\""
                   << rtl::OUStringToOString(
                       aCurrPoint->msPresentationLayoutName,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";
        else
            output << "label=\""
                   << rtl::OUStringToOString(
                       aCurrPoint->msModelId,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";

        switch( aCurrPoint->mnType )
        {
            case XML_doc: output << "style=filled, color=red"; break;
            case XML_asst: output << "style=filled, color=green"; break;
            default:
            case XML_node: output << "style=filled, color=blue"; break;
            case XML_pres: output << "style=filled, color=yellow"; break;
            case XML_parTrans: output << "color=grey"; break;
            case XML_sibTrans: output << " "; break;
        }

        output << "];" << std::endl;

        // does currpoint have any text set?
        if( aCurrPoint->mpShape &&
            aCurrPoint->mpShape->getTextBody() &&
            !aCurrPoint->mpShape->getTextBody()->getParagraphs().empty() &&
            !aCurrPoint->mpShape->getTextBody()->getParagraphs().front()->getRuns().empty() )
        {
            static sal_Int32 nCount=0;

            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << rtl::OUStringToOString(
                       aCurrPoint->mpShape->getTextBody()->getParagraphs().front()->getRuns().front()->getText(),
                       RTL_TEXTENCODING_UTF8).getStr()
                   << "\"" << "];" << std::endl;
            output << "\t"
                   << normalizeDotName(aCurrPoint->msModelId).getStr()
                   << " -> "
                   << "textNode" << nCount++
                   << ";" << std::endl;
        }


#if 0
        // msPresentationAssociationId does not appear to be
        // valid/used, the relation this imposed for several examples
        // was ~broken
        if( aCurrPoint->msPresentationAssociationId.getLength() )
            output << "\t"
                   << normalizeDotName(aCurrPoint->msModelId).getStr()
                   << " -> "
                   << normalizeDotName(aCurrPoint->msPresentationAssociationId).getStr()
                   << " [style=dotted, color=red, "
                   << "label=\"presAssocID\"];" << std::endl;
#endif

#endif

        const bool bInserted1=getData()->getPointNameMap().insert(
            std::make_pair(aCurrPoint->msModelId,&(*aCurrPoint))).second;
        (void)bInserted1;

        OSL_ENSURE(bInserted1,"Diagram::build(): non-unique point model id");

        if( aCurrPoint->msPresentationLayoutName.getLength() )
        {
            DiagramData::PointsNameMap::value_type::second_type& rVec=
                getData()->getPointsPresNameMap()[aCurrPoint->msPresentationLayoutName];
            rVec.push_back(&(*aCurrPoint));
        }
        ++aCurrPoint;
    }

    dgm::Connections::const_iterator aCurrCxn( getData()->getConnections( ).begin() );
    const dgm::Connections::const_iterator aEndCxn( getData()->getConnections( ).end() );
    while( aCurrCxn != aEndCxn )
    {
#if OSL_DEBUG_LEVEL > 1
        if( aCurrCxn->msParTransId.getLength() ||
            aCurrCxn->msSibTransId.getLength() )
        {
            if( aCurrCxn->msSourceId.getLength() ||
                aCurrCxn->msDestId.getLength() )
            {
                output << "\t"
                       << normalizeDotName(aCurrCxn->msSourceId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msSibTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msDestId).getStr()
                       << " [style=dotted,"
                       << ((aCurrCxn->mnType == XML_presOf) ? " color=red, " : ((aCurrCxn->mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << rtl::OUStringToOString(aCurrCxn->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
            else
            {
                output << "\t"
                       << normalizeDotName(aCurrCxn->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(aCurrCxn->msSibTransId).getStr()
                       << " ["
                       << ((aCurrCxn->mnType == XML_presOf) ? " color=red, " : ((aCurrCxn->mnType == XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << rtl::OUStringToOString(aCurrCxn->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
        }
        else if( aCurrCxn->msSourceId.getLength() ||
                 aCurrCxn->msDestId.getLength() )
            output << "\t"
                   << normalizeDotName(aCurrCxn->msSourceId).getStr()
                   << " -> "
                   << normalizeDotName(aCurrCxn->msDestId).getStr()
                   << " [label=\""
                   << rtl::OUStringToOString(aCurrCxn->msModelId,
                                             RTL_TEXTENCODING_UTF8 ).getStr()
                   << ((aCurrCxn->mnType == XML_presOf) ? "\", color=red]" : ((aCurrCxn->mnType == XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1=getData()->getConnectionNameMap().insert(
            std::make_pair(aCurrCxn->msModelId,&(*aCurrCxn))).second;
        (void)bInserted1;

        OSL_ENSURE(bInserted1,"Diagram::build(): non-unique connection model id");

        if( aCurrCxn->mnType == XML_presOf )
        {
            DiagramData::StringMap::value_type::second_type& rVec=getData()->getPresOfNameMap()[aCurrCxn->msDestId];
            rVec.push_back(
                std::make_pair(
                    aCurrCxn->msSourceId,sal_Int32(0)));
        }

        ++aCurrCxn;
    }

    // assign outline levels
    DiagramData::StringMap::iterator aPresOfIter=getData()->getPresOfNameMap().begin();
    const DiagramData::StringMap::iterator aPresOfEnd=getData()->getPresOfNameMap().end();
    while( aPresOfIter != aPresOfEnd )
    {
        DiagramData::StringMap::value_type::second_type::iterator aPresOfNodeIterCalcLevel=aPresOfIter->second.begin();
        const DiagramData::StringMap::value_type::second_type::iterator aPresOfNodeEnd=aPresOfIter->second.end();
        while(aPresOfNodeIterCalcLevel != aPresOfNodeEnd)
        {
            const sal_Int32 nDepth=calcDepth(aPresOfNodeIterCalcLevel->first,
                                             getData()->getConnections());
            aPresOfNodeIterCalcLevel->second = nDepth != 0 ? nDepth : -1;
            ++aPresOfNodeIterCalcLevel;
        }

        ++aPresOfIter;
    }

#if OSL_DEBUG_LEVEL > 1
    output << "}" << std::endl;
#endif

    // TODO
#if 0
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
//          OSL_TRACE( "ignoring relation %s", OUSTRING_TO_CSTR( (*aCnxIter)->msModelId ) );
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
#endif
}


void Diagram::addTo( const ShapePtr & pParentShape )
{
    // collect data, init maps
    build( );

    // create Shape hierarchy
    ShapeCreationVisitor aCreationVisitor(pParentShape, *this);
    mpLayout->getNode()->accept(aCreationVisitor);

#if 0
    dgm::Points & aPoints( mpData->getPoints( ) );
    dgm::Points::iterator aPointsIter;
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
#endif
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

uno::Reference<xml::dom::XDocument> loadFragment(
    core::XmlFilterBase& rFilter,
    const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    // load diagramming fragments into DOM representation, that later
    // gets serialized back to SAX events and parsed
    return rFilter.importFragment( rxHandler->getFragmentPath() );
}

void importFragment( core::XmlFilterBase& rFilter,
                     const uno::Reference<xml::dom::XDocument>& rXDom,
                     const char* /*pPropName*/,
                     const ShapePtr& /*pShape*/,
                     const rtl::Reference< core::FragmentHandler >& rxHandler )
{
    uno::Reference<xml::sax::XFastSAXSerializable> xSerializer(
        rXDom, uno::UNO_QUERY_THROW);

    // now serialize DOM tree into internal data structures
    rFilter.importFragment( rxHandler, xSerializer );

    // not yet
#if 0
    // tack XDocument onto shape
    pShape->getShapeProperties().setProperty(
        OUString::createFromAscii(pPropName),
        rXDom);
#endif
}

void loadDiagram( const ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const ::rtl::OUString& rDataModelPath,
                  const ::rtl::OUString& rLayoutPath,
                  const ::rtl::OUString& rQStylePath,
                  const ::rtl::OUString& rColorStylePath )
{
    DiagramPtr pDiagram( new Diagram() );

    DiagramDataPtr pData( new DiagramData() );
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout( new DiagramLayout() );
    pDiagram->setLayout( pLayout );

    // data
    if( rDataModelPath.getLength() > 0 )
    {
        rtl::Reference< core::FragmentHandler > xRef(
            new DiagramDataFragmentHandler( rFilter, rDataModelPath, pData ));

        importFragment(rFilter,
                       loadFragment(rFilter,xRef),
                       "DiagramData",
                       pShape,
                       xRef);
    }

    // layout
    if( rLayoutPath.getLength() > 0 )
    {
        rtl::Reference< core::FragmentHandler > xRef(
            new DiagramLayoutFragmentHandler( rFilter, rLayoutPath, pLayout ));
        importFragment(rFilter,
                       loadFragment(rFilter,xRef),
                       "DiagramLayout",
                       pShape,
                       xRef);
    }

    // style
    if( rQStylePath.getLength() > 0 )
    {
        rtl::Reference< core::FragmentHandler > xRef(
            new DiagramQStylesFragmentHandler( rFilter, rQStylePath, pDiagram->getStyles() ));
        importFragment(rFilter,
                       loadFragment(rFilter,xRef),
                       "DiagramQStyle",
                       pShape,
                       xRef);
    }

    // colors
    if( rColorStylePath.getLength() > 0 )
    {
        rtl::Reference< core::FragmentHandler > xRef(
            new ColorFragmentHandler( rFilter, rColorStylePath, pDiagram->getColors() ));
        importFragment(rFilter,
                       loadFragment(rFilter,xRef),
                       "DiagramColorStyle",
                       pShape,
                       xRef);
    }

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
}

void loadDiagram( const ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const uno::Reference<xml::dom::XDocument>& rXDataModelDom,
                  const uno::Reference<xml::dom::XDocument>& rXLayoutDom,
                  const uno::Reference<xml::dom::XDocument>& rXQStyleDom,
                  const uno::Reference<xml::dom::XDocument>& rXColorStyleDom )
{
    DiagramPtr pDiagram( new Diagram() );

    DiagramDataPtr pData( new DiagramData() );
    pDiagram->setData( pData );

    DiagramLayoutPtr pLayout( new DiagramLayout() );
    pDiagram->setLayout( pLayout );

    OUString aEmpty;

    // data
    if( rXDataModelDom.is() )
        importFragment(rFilter,
                       rXDataModelDom,
                       "DiagramData",
                       pShape,
                       new DiagramDataFragmentHandler( rFilter, aEmpty, pData ));

    // layout
    if( rXLayoutDom.is() )
        importFragment(rFilter,
                       rXLayoutDom,
                       "DiagramLayout",
                       pShape,
                       new DiagramLayoutFragmentHandler( rFilter, aEmpty, pLayout ));

    // style
    if( rXQStyleDom.is() )
        importFragment(rFilter,
                       rXQStyleDom,
                       "DiagramQStyle",
                       pShape,
                       new DiagramQStylesFragmentHandler( rFilter, aEmpty, pDiagram->getStyles() ));

    // colors
    if( rXColorStyleDom.is() )
        importFragment(rFilter,
                       rXColorStyleDom,
                       "DiagramColorStyle",
                       pShape,
                       new ColorFragmentHandler( rFilter, aEmpty, pDiagram->getColors() ));

    // diagram loaded. now lump together & attach to shape
    pDiagram->addTo(pShape);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
