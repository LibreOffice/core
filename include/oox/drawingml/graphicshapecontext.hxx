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

#ifndef INCLUDED_OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX

#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <oox/dllapi.h>

namespace oox { namespace vml { struct OleObjectInfo; } }

namespace oox { namespace drawingml {

class OOX_DLLPUBLIC GraphicShapeContext : public ShapeContext
{
public:
    GraphicShapeContext( ::oox::core::ContextHandler2Helper& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr );

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;
};



class OOX_DLLPUBLIC GraphicalObjectFrameContext : public ShapeContext
{
public:
    GraphicalObjectFrameContext( ::oox::core::ContextHandler2Helper& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr, bool bEmbedShapesInChart );

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;
    virtual void onEndElement() override;

private:
    bool                mbEmbedShapesInChart;
    ::oox::core::ContextHandler2Helper* mpParent;
};



class OleObjectGraphicDataContext : public ShapeContext
{
public:
    OleObjectGraphicDataContext( ::oox::core::ContextHandler2Helper& rParent, ShapePtr pShapePtr );
    virtual ~OleObjectGraphicDataContext();
    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;

private:
    ::oox::vml::OleObjectInfo& mrOleObjectInfo;
};



class Diagram;

class DiagramGraphicDataContext
    : public ShapeContext
{
public:
    DiagramGraphicDataContext( ::oox::core::ContextHandler2Helper& rParent, ShapePtr pShapePtr );
    virtual ~DiagramGraphicDataContext();
    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;

private:
    OUString msDm;
    OUString msLo;
    OUString msQs;
    OUString msCs;
};



class ChartGraphicDataContext : public ShapeContext
{
public:
    explicit            ChartGraphicDataContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const ShapePtr& rxShape, bool bEmbedShapes );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs) override;

private:
    ChartShapeInfo&     mrChartShapeInfo;
};



} }

#endif // INCLUDED_OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
