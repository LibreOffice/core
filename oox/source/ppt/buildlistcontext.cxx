/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: buildlistcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:44:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        ,   mbBuildAsOne( false )
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
//      bool uiExpand = attribs.getBool( XML_uiExpand, true );
                /* this is unsigned */
//      sal_uInt32 nGroupId =  attribs.getUnsignedInteger( XML_grpId, 0 );
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
