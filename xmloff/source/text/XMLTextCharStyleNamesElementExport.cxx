/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextCharStyleNamesElementExport.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:28:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLTEXTCHARSTYLENAMESELEMENTEXPORT_HXX
#include "XMLTextCharStyleNamesElementExport.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::XPropertySet;
using ::rtl::OUString;
using namespace ::xmloff::token;

XMLTextCharStyleNamesElementExport::XMLTextCharStyleNamesElementExport(
    SvXMLExport& rExp,
    sal_Bool bDoSth,
    sal_Bool bAllStyles,
    const Reference < XPropertySet > & rPropSet,
    const OUString& rPropName ) :
    rExport( rExp ),
    nCount( 0 )
{
    if( bDoSth )
    {
        Any aAny = rPropSet->getPropertyValue( rPropName );
        Sequence < OUString > aNames;
        if( aAny >>= aNames )
        {
            nCount = aNames.getLength();
            OSL_ENSURE( nCount > 0, "no char style found" );
            if ( bAllStyles ) ++nCount;
            if( nCount > 1 )
            {
                aName = rExport.GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT, GetXMLToken(XML_SPAN) );
                sal_Int32 i = nCount;
                const OUString *pName = aNames.getConstArray();
                while( --i )
                {
                    rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          rExport.EncodeStyleName( *pName ) );
                    rExport.StartElement( aName, sal_False );
                    ++pName;
                }
            }
        }
    }
}

XMLTextCharStyleNamesElementExport::~XMLTextCharStyleNamesElementExport()
{
    if( nCount > 1 )
    {
        sal_Int32 i = nCount;
        while( --i )
            rExport.EndElement( aName, sal_False );
    }
}

