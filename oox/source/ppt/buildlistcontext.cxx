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
#include "oox/core/namespaces.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace ppt {

    BuildListContext::BuildListContext( ContextHandler& rParent,
                const Reference< XFastAttributeList >& /*xAttribs*/,
                TimeNodePtrList & aTimeNodeList)
        : ContextHandler( rParent )
        , maTimeNodeList( aTimeNodeList )
        , mbInBldGraphic( false )
        ,	mbBuildAsOne( false )
    {
    }

    BuildListContext::~BuildListContext( )
    {
    }

    void SAL_CALL BuildListContext::endFastElement( sal_Int32 aElement ) throw ( SAXException, RuntimeException)
    {
        switch( aElement )
        {
        case NMSP_PPT|XML_bldGraphic:
            mbInBldGraphic = false;
            break;
        default:
            break;
        }
    }

    Reference< XFastContextHandler > SAL_CALL BuildListContext::createFastChildContext( ::sal_Int32 aElementToken,
                                                                                                                                                                         const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;

        switch( aElementToken )
        {
        case NMSP_PPT|XML_bldAsOne:
            if( mbInBldGraphic )
            {
                mbBuildAsOne = true;
            }
            break;
        case NMSP_PPT|XML_bldSub:
            if( mbInBldGraphic )
            {
            }
            break;
        case NMSP_PPT|XML_bldGraphic:
        {
            mbInBldGraphic = true;
            AttributeList attribs( xAttribs );
            OUString sShapeId = xAttribs->getOptionalValue( XML_spid );
// TODO
//		bool uiExpand = attribs.getBool( XML_uiExpand, true );
                /* this is unsigned */
//		sal_uInt32 nGroupId =  attribs.getUnsignedInteger( XML_grpId, 0 );
            break;
        }
        case NMSP_DRAWINGML|XML_bldDgm:
        case NMSP_DRAWINGML|XML_bldOleChart:
        case NMSP_DRAWINGML|XML_bldP:

            break;
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set(this);

        return xRet;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
