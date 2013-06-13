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

#include "layoutnodecontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "diagramdefinitioncontext.hxx"
#include "constraintlistcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

class IfContext
    : public LayoutNodeContext
{
public:
    IfContext( ContextHandler2Helper& rParent,
               const AttributeList& rAttribs,
               const ConditionAtomPtr& pAtom )
        : LayoutNodeContext( rParent, rAttribs, pAtom )
    {}
};



class AlgorithmContext
    : public ContextHandler2
{
public:
    AlgorithmContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, const AlgAtomPtr & pNode )
        : ContextHandler2( rParent )
        , mnRevision( 0 )
        , mpNode( pNode )
        {
            mnRevision = rAttribs.getInteger( XML_rev, 0 );
            pNode->setType(rAttribs.getToken(XML_type, 0));
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( ::sal_Int32 aElement,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElement )
            {
                case DGM_TOKEN( param ):
                {
                    AttributeList aAttribs( xAttribs );
                    const sal_Int32 nValTok=aAttribs.getToken( XML_val, 0 );
                    mpNode->addParam(
                        aAttribs.getToken( XML_type, 0 ),
                        nValTok>0 ? nValTok : aAttribs.getInteger( XML_val, 0 ) );
                    break;
                }
                default:
                    break;
            }

            if( !xRet.is() )
                xRet.set(this);

            return xRet;
        }

private:
    sal_Int32  mnRevision;
    AlgAtomPtr mpNode;
};


class ChooseContext
    : public ContextHandler2
{
public:
    ChooseContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, const LayoutAtomPtr & pNode )
        : ContextHandler2( rParent )
        , mpNode( pNode )
        {
            msName = rAttribs.getString( XML_name ).get();
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( ::sal_Int32 aElement,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElement )
            {
            case DGM_TOKEN( if ):
            {
                // CT_When
                mpConditionNode.reset( new ConditionAtom(xAttribs) );
                mpNode->addChild( mpConditionNode );
                xRet.set( new IfContext( *this, AttributeList( xAttribs ), mpConditionNode ) );
                break;
            }
            case DGM_TOKEN( else ):
                // CT_Otherwise
                if( mpConditionNode )
                {
                    mpConditionNode->readElseBranch();
                    xRet.set( new IfContext( *this, AttributeList( xAttribs ), mpConditionNode ) );
                    mpConditionNode.reset();
                }
                else
                {
                    OSL_TRACE( "ignoring second else clause" );
                }
                break;
            default:
                break;
            }

            if( !xRet.is() )
                xRet.set(this);

            return xRet;
        }
private:
    OUString msName;
    LayoutAtomPtr mpNode;
    ConditionAtomPtr mpConditionNode;
};




class ForEachContext
    : public LayoutNodeContext
{
public:
    ForEachContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, const ForEachAtomPtr& pAtom )
        : LayoutNodeContext( rParent, rAttribs, pAtom )
        {
            rAttribs.getString( XML_ref );
            pAtom->iterator().loadFromXAttr( rAttribs.getFastAttributeList() );
        }
};


// CT_LayoutVariablePropertySet
class LayoutVariablePropertySetContext
    : public ContextHandler2
{
public:
    LayoutVariablePropertySetContext( ContextHandler2Helper& rParent, LayoutNode::VarMap & aVar )
        : ContextHandler2( rParent )
        , mVariables( aVar )
        {
        }

    virtual ~LayoutVariablePropertySetContext()
        {
        }

    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            sal_Int32 nIdx =  LayoutNodeContext::tagToVarIdx( getBaseToken( aElement ) );
            if( nIdx != -1 )
            {
                mVariables[ nIdx ] = makeAny( xAttribs->getOptionalValue( XML_val ) );
            }
            if( !xRet.is() )
                xRet.set(this);

            return xRet;
        }
private:
    LayoutNode::VarMap & mVariables;
};


// CT_LayoutNode
LayoutNodeContext::LayoutNodeContext( ContextHandler2Helper& rParent,
                                      const AttributeList& rAttribs,
                                      const LayoutAtomPtr& pAtom )
    : ContextHandler2( rParent )
    , mpNode( pAtom )
{
    OSL_ENSURE( pAtom, "Node must NOT be NULL" );
    mpNode->setName( rAttribs.getString( XML_name ).get() );
}


LayoutNodeContext::~LayoutNodeContext()
{
}

void SAL_CALL LayoutNodeContext::endFastElement( ::sal_Int32 )
    throw (SAXException, RuntimeException)
{

}

/** convert the XML tag to a variable index in the array
 * @param aTag the tag, without namespace
 * @return the variable index. -1 is an error
 */
sal_Int32 LayoutNodeContext::tagToVarIdx( sal_Int32 aTag )
{
    sal_Int32 nIdx = -1;
    switch( aTag )
    {
    case DGM_TOKEN( animLvl ):
        nIdx = LayoutNode::VAR_animLvl;
        break;
    case DGM_TOKEN( animOne ):
        nIdx = LayoutNode::VAR_animOne;
        break;
    case DGM_TOKEN( bulletEnabled ):
        nIdx = LayoutNode::VAR_bulletEnabled;
        break;
    case DGM_TOKEN( chMax ):
        nIdx = LayoutNode::VAR_chMax;
        break;
    case DGM_TOKEN( chPref ):
        nIdx = LayoutNode::VAR_chPref;
        break;
    case DGM_TOKEN( dir ):
        nIdx = LayoutNode::VAR_dir;
        break;
    case DGM_TOKEN( hierBranch ):
        nIdx = LayoutNode::VAR_hierBranch;
        break;
    case DGM_TOKEN( orgChart ):
        nIdx = LayoutNode::VAR_orgChart;
        break;
    case DGM_TOKEN( resizeHandles ):
        nIdx = LayoutNode::VAR_resizeHandles;
        break;
    default:
        break;
    }
    return nIdx;
}


Reference< XFastContextHandler > SAL_CALL
LayoutNodeContext::createFastChildContext( ::sal_Int32 aElement,
                                                  const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( layoutNode ):
    {
        LayoutNodePtr pNode( new LayoutNode() );
        mpNode->addChild( pNode );
        pNode->setChildOrder( xAttribs->getOptionalValueToken( XML_chOrder, XML_b ) );
        pNode->setMoveWith( xAttribs->getOptionalValue( XML_moveWith ) );
        pNode->setStyleLabel( xAttribs->getOptionalValue( XML_styleLbl ) );
        xRet.set( new LayoutNodeContext( *this, AttributeList( xAttribs ), pNode ) );
        break;
    }
    case DGM_TOKEN( shape ):
    {
        LayoutNodePtr pNode( boost::dynamic_pointer_cast< LayoutNode >( mpNode ) );
        if( pNode )
        {
            ShapePtr pShape;

            if( xAttribs->hasAttribute( XML_type ) )
            {
                pShape.reset( new Shape("com.sun.star.drawing.CustomShape") );
                const sal_Int32 nType(xAttribs->getOptionalValueToken( XML_type, XML_obj ));
                pShape->setSubType( nType );
                pShape->getCustomShapeProperties()->setShapePresetType( nType );
            }
            else
            {
                pShape.reset( new Shape("com.sun.star.drawing.GroupShape") );
            }

            pNode->setShape( pShape );
            xRet.set( new ShapeContext( *this, ShapePtr(), pShape ) );
        }
        else
        {
            OSL_TRACE( "OOX: encountered a shape in a non layoutNode context" );
        }
        break;
    }
    case DGM_TOKEN( extLst ):
        return xRet;
    case DGM_TOKEN( alg ):
    {
        // CT_Algorithm
        AlgAtomPtr pAtom( new AlgAtom );
        mpNode->addChild( pAtom );
        xRet.set( new AlgorithmContext( *this, AttributeList( xAttribs ), pAtom ) );
        break;
    }
    case DGM_TOKEN( choose ):
    {
        // CT_Choose
        LayoutAtomPtr pAtom( new ChooseAtom );
        mpNode->addChild( pAtom );
        xRet.set( new ChooseContext( *this, AttributeList( xAttribs ), pAtom ) );
         break;
    }
    case DGM_TOKEN( forEach ):
    {
        // CT_ForEach
        ForEachAtomPtr pAtom( new ForEachAtom(xAttribs) );
        mpNode->addChild( pAtom );
        xRet.set( new ForEachContext( *this, AttributeList( xAttribs ), pAtom ) );
        break;
    }
    case DGM_TOKEN( constrLst ):
        // CT_Constraints
        xRet.set( new ConstraintListContext( *this, AttributeList( xAttribs ), mpNode ) );
        break;
    case DGM_TOKEN( presOf ):
    {
        // CT_PresentationOf
        // TODO
        xAttribs->getOptionalValue( XML_axis );
        xAttribs->getOptionalValue( XML_cnt );
        xAttribs->getOptionalValue( XML_hideLastTrans );
        xAttribs->getOptionalValue( XML_ptType );
        xAttribs->getOptionalValue( XML_st );
        xAttribs->getOptionalValue( XML_step );
        break;
    }
    case DGM_TOKEN( ruleLst ):
        // CT_Rules
        // TODO
        break;
    case DGM_TOKEN( varLst ):
    {
        LayoutNodePtr pNode( boost::dynamic_pointer_cast< LayoutNode >( mpNode ) );
        if( pNode )
        {
            xRet.set( new LayoutVariablePropertySetContext( *this, pNode->variables() ) );
        }
        else
        {
            OSL_TRACE( "OOX: encountered a varLst in a non layoutNode context" );
        }
        break;
    }
    default:
        break;
    }
    if( !xRet.is() )
        xRet.set(this);

    return xRet;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
