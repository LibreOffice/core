/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTableFilterPattern.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:13:26 $
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
#ifndef DBA_XMLTABLEFILTERPATTERN_HXX
#include "xmlTableFilterPattern.hxx"
#endif
#ifndef DBA_XMLTABLEFILTERLIST_HXX
#include "xmlTableFilterList.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLTableFilterPattern)

OXMLTableFilterPattern::OXMLTableFilterPattern( SvXMLImport& rImport,
                sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,sal_Bool _bNameFilter
                ,OXMLTableFilterList& _rParent) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
        ,m_bNameFilter(_bNameFilter)
        ,m_rParent(_rParent)
{
    DBG_CTOR(OXMLTableFilterPattern,NULL);

}
// -----------------------------------------------------------------------------

OXMLTableFilterPattern::~OXMLTableFilterPattern()
{

    DBG_DTOR(OXMLTableFilterPattern,NULL);
}
// -----------------------------------------------------------------------------
void OXMLTableFilterPattern::Characters( const ::rtl::OUString& rChars )
{
    if ( m_bNameFilter )
        m_rParent.pushTableFilterPattern(rChars);
    else
        m_rParent.pushTableTypeFilter(rChars);
}

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
