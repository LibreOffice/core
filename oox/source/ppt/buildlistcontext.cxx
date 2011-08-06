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

#include "buildlistcontext.hxx"
#include <rtl/ustring.hxx>
#include "oox/helper/attributelist.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace ppt {

    BuildListContext::BuildListContext( FragmentHandler2& rParent,
                const Reference< XFastAttributeList >& /*xAttribs*/,
                TimeNodePtrList & aTimeNodeList)
        : FragmentHandler2( rParent )
        , maTimeNodeList( aTimeNodeList )
        , mbInBldGraphic( false )
        ,   mbBuildAsOne( false )
    {
    }

    BuildListContext::~BuildListContext( )
    {
    }

    void BuildListContext::onEndElement()
    {
        switch( getCurrentElement() )
        {
        case PPT_TOKEN( bldGraphic ):
            mbInBldGraphic = false;
            break;
        default:
            break;
        }
    }

    ::oox::core::ContextHandlerRef BuildListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( bldAsOne ):
            if( mbInBldGraphic )
            {
                mbBuildAsOne = true;
            }
            return this;
        case PPT_TOKEN( bldSub ):
            if( mbInBldGraphic )
            {
            }
            return this;
        case PPT_TOKEN( bldGraphic ):
        {
            mbInBldGraphic = true;
            OUString sShapeId = rAttribs.getString( XML_spid, OUString() );
// TODO
//      bool uiExpand = rAttribs.getBool( XML_uiExpand, true );
                /* this is unsigned */
//      sal_uInt32 nGroupId =  rAttribs.getUnsignedInteger( XML_grpId, 0 );
        return this;
        }
        case A_TOKEN( bldDgm ):
        case A_TOKEN( bldOleChart ):
        case A_TOKEN( bldP ):
            return this;
        default:
            break;
        }

        return this;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
