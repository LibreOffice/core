/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: themefragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:32:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/objectdefaultcontext.hxx"
#include "oox/drawingml/themeelementscontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;

namespace oox {
namespace drawingml {

// ============================================================================

ThemeFragmentHandler::ThemeFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, Theme& rTheme ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mrTheme( rTheme )
{
}

ThemeFragmentHandler::~ThemeFragmentHandler()
{
}

ContextWrapper ThemeFragmentHandler::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    // CT_OfficeStyleSheet
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return nElement == (NMSP_DRAWINGML|XML_theme);

        case NMSP_DRAWINGML|XML_theme:
            switch( nElement )
            {
                case NMSP_DRAWINGML|XML_themeElements:              // CT_BaseStyles
                    return new themeElementsContext( *this, mrTheme );
                case NMSP_DRAWINGML|XML_objectDefaults:             // CT_ObjectStyleDefaults
                    return new objectDefaultContext( *this, mrTheme );
                case NMSP_DRAWINGML|XML_extraClrSchemeLst:          // CT_ColorSchemeList
                    return false;
                case NMSP_DRAWINGML|XML_custClrLst:                 // CustomColorList
                    return false;
                case NMSP_DRAWINGML|XML_ext:                        // CT_OfficeArtExtension
                    return false;
            }
        break;
    }
    return false;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

