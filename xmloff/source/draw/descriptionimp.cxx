/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: descriptionimp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:32:35 $
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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#include "descriptionimp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLDescriptionContext, SvXMLImportContext );

SdXMLDescriptionContext::SdXMLDescriptionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,   const OUString& rLocalName,
        const Reference< XAttributeList>&, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLDescriptionContext::~SdXMLDescriptionContext()
{
}

void SdXMLDescriptionContext::EndElement()
{
    if( msText.getLength() ) try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY_THROW);
        if(IsXMLToken(GetLocalName(),XML_TITLE))
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Title")), Any(msText));
        }
        else
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Description")), Any(msText));
        }
    }
    catch( uno::Exception& )
    {
    }
}

// This method is called for all characters that are contained in the
// current element. The default is to ignore them.
void SdXMLDescriptionContext::Characters( const ::rtl::OUString& rChars )
{
    msText += rChars;
}

