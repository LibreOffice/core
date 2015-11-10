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

#include "diagramdefinitioncontext.hxx"
#include "layoutnodecontext.hxx"
#include "datamodelcontext.hxx"
#include <osl/diagnose.h>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_DiagramDefinition
DiagramDefinitionContext::DiagramDefinitionContext( ContextHandler2Helper& rParent,
                                                    const AttributeList& rAttributes,
                                                    const DiagramLayoutPtr &pLayout )
    : ContextHandler2( rParent )
    , mpLayout( pLayout )
{
    OSL_TRACE( "OOX: DiagramDefinitionContext::DiagramDefinitionContext()" );
    mpLayout->setDefStyle( rAttributes.getString( XML_defStyle ).get() );
    OUString sValue = rAttributes.getString( XML_minVer ).get();
    if( sValue.isEmpty() )
    {
        sValue = "http://schemas.openxmlformats.org/drawingml/2006/diagram";
    }
    mpLayout->setMinVer( sValue );
    mpLayout->setUniqueId( rAttributes.getString( XML_uniqueId ).get() );
}

DiagramDefinitionContext::~DiagramDefinitionContext()
{
    LayoutNodePtr node = mpLayout->getNode();
    if (node)
        node->dump();
}

ContextHandlerRef
DiagramDefinitionContext::onCreateContext( ::sal_Int32 aElement,
                                           const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( title ):
        mpLayout->setTitle( rAttribs.getString( XML_val ).get() );
        break;
    case DGM_TOKEN( desc ):
        mpLayout->setDesc( rAttribs.getString( XML_val ).get() );
        break;
    case DGM_TOKEN( layoutNode ):
    {
        LayoutNodePtr pNode( new LayoutNode() );
        mpLayout->getNode() = pNode;
        pNode->setChildOrder( rAttribs.getToken( XML_chOrder, XML_b ) );
        pNode->setMoveWith( rAttribs.getString( XML_moveWith ).get() );
        pNode->setStyleLabel( rAttribs.getString( XML_styleLbl ).get() );
        return new LayoutNodeContext( *this, rAttribs, pNode );
    }
     case DGM_TOKEN( clrData ):
        // TODO, does not matter for the UI. skip.
        return nullptr;
    case DGM_TOKEN( sampData ):
        mpLayout->getSampData().reset( new DiagramData );
        return new DataModelContext( *this, mpLayout->getSampData() );
    case DGM_TOKEN( styleData ):
        mpLayout->getStyleData().reset( new DiagramData );
        return new DataModelContext( *this, mpLayout->getStyleData() );
    case DGM_TOKEN( cat ):
    case DGM_TOKEN( catLst ):
        // TODO, does not matter for the UI
    default:
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
