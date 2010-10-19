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

#include "layoutnodecontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "diagramdefinitioncontext.hxx"
#include "constraintlistcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml {

class IfContext
    : public LayoutNodeContext
{
public:
    IfContext( ContextHandler& rParent,
               const Reference< XFastAttributeList >& xAttribs,
               const ConditionAtomPtr& pAtom )
        : LayoutNodeContext( rParent, xAttribs, pAtom )
    {}
};



class AlgorithmContext
    : public ContextHandler
{
public:
    AlgorithmContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, const AlgAtomPtr & pNode )
        : ContextHandler( rParent )
        , mnRevision( 0 )
        , mpNode( pNode )
        {
            AttributeList aAttribs( xAttribs );
            mnRevision = aAttribs.getInteger( XML_rev, 0 );
            pNode->setType(xAttribs->getOptionalValueToken(XML_type, 0));
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( ::sal_Int32 aElement,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( getToken(aElement) )
            {
                case XML_param:
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
    : public ContextHandler
{
public:
    ChooseContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, const LayoutAtomPtr & pNode )
        : ContextHandler( rParent )
        , mpNode( pNode )
        {
            msName = xAttribs->getOptionalValue( XML_name );
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( ::sal_Int32 aElement,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( getToken(aElement) )
            {
            case XML_if:
            {
                // CT_When
                mpConditionNode.reset( new ConditionAtom(xAttribs) );
                mpNode->addChild( mpConditionNode );
                xRet.set( new IfContext( *this, xAttribs, mpConditionNode ) );
                break;
            }
            case XML_else:
                // CT_Otherwise
                if( mpConditionNode )
                {
                    mpConditionNode->readElseBranch();
                    xRet.set( new IfContext( *this, xAttribs, mpConditionNode ) );
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
    ForEachContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, const ForEachAtomPtr& pAtom )
        : LayoutNodeContext( rParent, xAttribs, pAtom )
        {
            xAttribs->getOptionalValue( XML_ref );
            pAtom->iterator().loadFromXAttr( xAttribs );
        }
};


// CT_LayoutVariablePropertySet
class LayoutVariablePropertySetContext
    : public ContextHandler
{
public:
    LayoutVariablePropertySetContext( ContextHandler& rParent, LayoutNode::VarMap & aVar )
        : ContextHandler( rParent )
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

            sal_Int32 nIdx =  LayoutNodeContext::tagToVarIdx( getToken( aElement ) );
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
LayoutNodeContext::LayoutNodeContext( ContextHandler& rParent,
                                      const Reference< XFastAttributeList >& xAttribs,
                                      const LayoutAtomPtr& pAtom )
    : ContextHandler( rParent )
    , mpNode( pAtom )
{
    OSL_ENSURE( pAtom, "Node must NOT be NULL" );
    mpNode->setName( xAttribs->getOptionalValue( XML_name ) );
}


LayoutNodeContext::~LayoutNodeContext()
{
}

void SAL_CALL LayoutNodeContext::endFastElement( ::sal_Int32 )
    throw (SAXException, RuntimeException)
{

}

/** convert the XML tag to a variable index in the array
 * @param aTag the tag, wihout namespace
 * @return the variable index. -1 is an error
 */
sal_Int32 LayoutNodeContext::tagToVarIdx( sal_Int32 aTag )
{
    sal_Int32 nIdx = -1;
    switch( aTag & ~NMSP_DIAGRAM )
    {
    case XML_animLvl:
        nIdx = LayoutNode::VAR_animLvl;
        break;
    case XML_animOne:
        nIdx = LayoutNode::VAR_animOne;
        break;
    case XML_bulletEnabled:
        nIdx = LayoutNode::VAR_bulletEnabled;
        break;
    case XML_chMax:
        nIdx = LayoutNode::VAR_chMax;
        break;
    case XML_chPref:
        nIdx = LayoutNode::VAR_chPref;
        break;
    case XML_dir:
        nIdx = LayoutNode::VAR_dir;
        break;
    case XML_hierBranch:
        nIdx = LayoutNode::VAR_hierBranch;
        break;
    case XML_orgChart:
        nIdx = LayoutNode::VAR_orgChart;
        break;
    case XML_resizeHandles:
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
    case NMSP_DIAGRAM|XML_layoutNode:
    {
        LayoutNodePtr pNode( new LayoutNode() );
        mpNode->addChild( pNode );
        pNode->setChildOrder( xAttribs->getOptionalValueToken( XML_chOrder, XML_b ) );
        pNode->setMoveWith( xAttribs->getOptionalValue( XML_moveWith ) );
        pNode->setStyleLabel( xAttribs->getOptionalValue( XML_styleLbl ) );
        xRet.set( new LayoutNodeContext( *this, xAttribs, pNode ) );
        break;
    }
    case NMSP_DIAGRAM|XML_shape:
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
                pShape->getCustomShapeProperties()->setShapePresetType(
                    GetShapePresetType( nType ) );
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
    case NMSP_DIAGRAM|XML_extLst:
        return xRet;
    case NMSP_DIAGRAM|XML_alg:
    {
        // CT_Algorithm
        AlgAtomPtr pAtom( new AlgAtom );
        mpNode->addChild( pAtom );
        xRet.set( new AlgorithmContext( *this, xAttribs, pAtom ) );
        break;
    }
    case NMSP_DIAGRAM|XML_choose:
    {
        // CT_Choose
        LayoutAtomPtr pAtom( new ChooseAtom );
        mpNode->addChild( pAtom );
        xRet.set( new ChooseContext( *this, xAttribs, pAtom ) );
         break;
    }
    case NMSP_DIAGRAM|XML_forEach:
    {
        // CT_ForEach
        ForEachAtomPtr pAtom( new ForEachAtom(xAttribs) );
        mpNode->addChild( pAtom );
        xRet.set( new ForEachContext( *this, xAttribs, pAtom ) );
        break;
    }
    case NMSP_DIAGRAM|XML_constrLst:
        // CT_Constraints
        xRet.set( new ConstraintListContext( *this, xAttribs, mpNode ) );
        break;
    case NMSP_DIAGRAM|XML_presOf:
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
    case NMSP_DIAGRAM|XML_ruleLst:
        // CT_Rules
        // TODO
        break;
    case NMSP_DIAGRAM|XML_varLst:
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
