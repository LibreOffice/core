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

#include <oox/helper/attributelist.hxx>
#include <drawingml/diagram/diagram.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <drawingml/customshapeproperties.hxx>
#include "diagramdefinitioncontext.hxx"
#include "constraintlistcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

class IfContext
    : public LayoutNodeContext
{
public:
    IfContext( ContextHandler2Helper const & rParent,
               const AttributeList& rAttribs,
               const ConditionAtomPtr& pAtom )
        : LayoutNodeContext( rParent, rAttribs, pAtom )
    {}
};

class AlgorithmContext
    : public ContextHandler2
{
public:
    AlgorithmContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, const AlgAtomPtr & pNode )
        : ContextHandler2( rParent )
        , mnRevision( 0 )
        , mpNode( pNode )
        {
            mnRevision = rAttribs.getInteger( XML_rev, 0 );
            pNode->setType(rAttribs.getToken(XML_type, 0));
        }

    virtual ContextHandlerRef
    onCreateContext( ::sal_Int32 aElement,
                     const AttributeList& rAttribs ) override
        {
            switch( aElement )
            {
                case DGM_TOKEN( param ):
                {
                    sal_Int32 nType = rAttribs.getToken(XML_type, 0);
                    switch (nType)
                    {
                        case XML_ar:
                            mpNode->setAspectRatio(rAttribs.getDouble(XML_val, 0));
                            break;
                        default:
                            const sal_Int32 nValTok = rAttribs.getToken(XML_val, 0);
                            mpNode->addParam(nType, nValTok > 0 ? nValTok
                                                                : rAttribs.getInteger(XML_val, 0));
                            break;
                    }
                    break;
                }
                default:
                    break;
            }

            return this;
        }

private:
    sal_Int32  mnRevision;
    AlgAtomPtr mpNode;
};

class ChooseContext
    : public ContextHandler2
{
public:
    ChooseContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, const LayoutAtomPtr & pNode )
        : ContextHandler2( rParent )
        , mpNode( pNode )
        {
            msName = rAttribs.getString( XML_name ).get();
        }

    virtual ContextHandlerRef
    onCreateContext( ::sal_Int32 aElement,
                     const AttributeList& rAttribs ) override
        {
            switch( aElement )
            {
            case DGM_TOKEN( if ):
            {
                // CT_When
                ConditionAtomPtr pNode( new ConditionAtom(mpNode->getLayoutNode(), false, rAttribs.getFastAttributeList()) );
                LayoutAtom::connect(mpNode, pNode);
                return new IfContext( *this, rAttribs, pNode );
            }
            case DGM_TOKEN( else ):
            {
                // CT_Otherwise
                ConditionAtomPtr pNode( new ConditionAtom(mpNode->getLayoutNode(), true, rAttribs.getFastAttributeList()) );
                LayoutAtom::connect(mpNode, pNode);
                return new IfContext( *this, rAttribs, pNode );
            }
            default:
                break;
            }

            return this;
        }
private:
    OUString msName;
    LayoutAtomPtr mpNode;
};

class ForEachContext
    : public LayoutNodeContext
{
public:
    ForEachContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, const ForEachAtomPtr& pAtom )
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
    LayoutVariablePropertySetContext( ContextHandler2Helper const & rParent, LayoutNode::VarMap & aVar )
        : ContextHandler2( rParent )
        , mVariables( aVar )
        {
        }

    virtual ContextHandlerRef onCreateContext( ::sal_Int32 aElement, const AttributeList& rAttribs ) override
        {
            mVariables[ getBaseToken(aElement) ] = rAttribs.getString( XML_val ).get();
            return this;
        }
private:
    LayoutNode::VarMap & mVariables;
};

// CT_LayoutNode
LayoutNodeContext::LayoutNodeContext( ContextHandler2Helper const & rParent,
                                      const AttributeList& rAttribs,
                                      const LayoutAtomPtr& pAtom )
    : ContextHandler2( rParent )
    , mpNode( pAtom )
{
    assert( pAtom && "Node must NOT be NULL" );
    mpNode->setName( rAttribs.getString( XML_name ).get() );
}

LayoutNodeContext::~LayoutNodeContext()
{
}

ContextHandlerRef
LayoutNodeContext::onCreateContext( ::sal_Int32 aElement,
                                    const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( layoutNode ):
    {
        LayoutNodePtr pNode( new LayoutNode(mpNode->getLayoutNode().getDiagram()) );
        LayoutAtom::connect(mpNode, pNode);
        pNode->setChildOrder( rAttribs.getToken( XML_chOrder, XML_b ) );
        pNode->setMoveWith( rAttribs.getString( XML_moveWith ).get() );
        pNode->setStyleLabel( rAttribs.getString( XML_styleLbl ).get() );
        return new LayoutNodeContext( *this, rAttribs, pNode );
    }
    case DGM_TOKEN( shape ):
    {
        ShapePtr pShape;

        if( rAttribs.hasAttribute( XML_type ) )
        {
            pShape.reset( new Shape("com.sun.star.drawing.CustomShape") );
            if (!rAttribs.getBool(XML_hideGeom, false))
            {
                const sal_Int32 nType(rAttribs.getToken( XML_type, XML_obj ));
                pShape->setSubType( nType );
                pShape->getCustomShapeProperties()->setShapePresetType( nType );
            }
        }
        else
        {
            pShape.reset( new Shape("com.sun.star.drawing.GroupShape") );
        }

        pShape->setDiagramRotation(rAttribs.getInteger(XML_rot, 0) * PER_DEGREE);

        pShape->setZOrderOff(rAttribs.getInteger(XML_zOrderOff, 0));

        ShapeAtomPtr pAtom( new ShapeAtom(mpNode->getLayoutNode(), pShape) );
        LayoutAtom::connect(mpNode, pAtom);
        return new ShapeContext( *this, ShapePtr(), pShape );
    }
    case DGM_TOKEN( extLst ):
        return nullptr;
    case DGM_TOKEN( alg ):
    {
        // CT_Algorithm
        AlgAtomPtr pAtom( new AlgAtom(mpNode->getLayoutNode()) );
        LayoutAtom::connect(mpNode, pAtom);
        mpNode->getLayoutNode().setAlgAtom(pAtom);
        return new AlgorithmContext( *this, rAttribs, pAtom );
    }
    case DGM_TOKEN( choose ):
    {
        // CT_Choose
        LayoutAtomPtr pAtom( new ChooseAtom(mpNode->getLayoutNode()) );
        LayoutAtom::connect(mpNode, pAtom);
        return new ChooseContext( *this, rAttribs, pAtom );
    }
    case DGM_TOKEN( forEach ):
    {
        // CT_ForEach
        ForEachAtomPtr pAtom( new ForEachAtom(mpNode->getLayoutNode(), rAttribs.getFastAttributeList()) );
        LayoutAtom::connect(mpNode, pAtom);
        return new ForEachContext( *this, rAttribs, pAtom );
    }
    case DGM_TOKEN( constrLst ):
        // CT_Constraints
        return new ConstraintListContext( *this, mpNode );
    case DGM_TOKEN( presOf ):
    {
        // CT_PresentationOf
        // TODO
        IteratorAttr aIterator;
        aIterator.loadFromXAttr(rAttribs.getFastAttributeList());
        break;
    }
    case DGM_TOKEN( ruleLst ):
        // CT_Rules
        // TODO
        break;
    case DGM_TOKEN( varLst ):
    {
        LayoutNodePtr pNode(std::dynamic_pointer_cast<LayoutNode>(mpNode));
        if( pNode )
        {
            return new LayoutVariablePropertySetContext( *this, pNode->variables() );
        }
        else
        {
            SAL_WARN("oox",  "OOX: encountered a varLst in a non layoutNode context" );
        }
        break;
    }
    default:
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
