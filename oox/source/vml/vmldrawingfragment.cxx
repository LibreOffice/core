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

#include "oox/vml/vmldrawingfragment.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlinputstream.hxx"
#include "oox/vml/vmlshapecontext.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::io::XInputStream;
using ::oox::core::ContextHandlerRef;
using ::oox::core::FragmentHandler2;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace vml {

// ============================================================================

DrawingFragment::DrawingFragment( XmlFilterBase& rFilter, const OUString& rFragmentPath, Drawing& rDrawing ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mrDrawing( rDrawing )
{
}

Reference< XInputStream > DrawingFragment::openFragmentStream() const
{
    // #i104719# create an input stream that preprocesses the VML data
    return new InputStream( FragmentHandler2::openFragmentStream() );
}

ContextHandlerRef DrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( mrDrawing.getType() )
    {
        // DOCX filter handles plain shape elements with this fragment handler
        case VMLDRAWING_WORD:
            if ( getNamespace( nElement ) == NMSP_VML )
                return ShapeContextBase::createShapeContext( *this, nElement, rAttribs, mrDrawing.getShapes() );
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
                    return ShapeContextBase::createShapeContext( *this, nElement, rAttribs, mrDrawing.getShapes() );
            }
        break;
    }
    return 0;
}

void DrawingFragment::finalizeImport()
{
    // resolve shape template references for all shapes
    mrDrawing.finalizeFragmentImport();
}

// ============================================================================

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
