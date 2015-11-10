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

#include "oox/vml/vmldrawingfragment.hxx"

#include "oox/core/xmlfilterbase.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlinputstream.hxx"
#include "oox/vml/vmlshapecontext.hxx"

namespace oox {
namespace vml {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

DrawingFragment::DrawingFragment( XmlFilterBase& rFilter, const OUString& rFragmentPath, Drawing& rDrawing ) :
    FragmentHandler2( rFilter, rFragmentPath, false ),  // do not trim whitespace, has been preprocessed by the input stream
    mrDrawing( rDrawing )
{
}

Reference< XInputStream > DrawingFragment::openFragmentStream() const
{
    // #i104719# create an input stream that preprocesses the VML data
    return new InputStream( getFilter().getComponentContext(), FragmentHandler2::openFragmentStream() );
}

ContextHandlerRef DrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( mrDrawing.getType() )
    {
        // DOCX filter handles plain shape elements with this fragment handler
        case VMLDRAWING_WORD:
            if ( getNamespace( nElement ) == NMSP_vml )
                return ShapeContextBase::createShapeContext( *this, mrDrawing.getShapes(), nElement, rAttribs );
        break;

        // XLSX and PPTX filters load the entire VML fragment
        case VMLDRAWING_EXCEL:
        case VMLDRAWING_POWERPOINT:
            switch( getCurrentElement() )
            {
                case XML_ROOT_CONTEXT:
                    if( nElement == XML_xml ) return this;
                break;
                case XML_xml:
                    return ShapeContextBase::createShapeContext( *this, mrDrawing.getShapes(), nElement, rAttribs );
            }
        break;
    }
    return nullptr;
}

void DrawingFragment::finalizeImport()
{
    // resolve shape template references for all shapes
    mrDrawing.finalizeFragmentImport();
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
