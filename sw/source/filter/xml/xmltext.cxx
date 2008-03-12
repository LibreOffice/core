/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmltext.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:43:10 $
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
#include "precompiled_sw.hxx"



#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

// ---------------------------------------------------------------------


class SwXMLBodyContentContext_Impl : public SvXMLImportContext
{
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    SwXMLBodyContentContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                             const OUString& rLName );
    virtual ~SwXMLBodyContentContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    // The body element's text:global attribute can be ignored, because
    // we must have the correct object shell already.
    virtual void EndElement();
};

SwXMLBodyContentContext_Impl::SwXMLBodyContentContext_Impl( SwXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLBodyContentContext_Impl::~SwXMLBodyContentContext_Impl()
{
}

SvXMLImportContext *SwXMLBodyContentContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    pContext = GetSwImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
               XML_TEXT_TYPE_BODY );
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void SwXMLBodyContentContext_Impl::EndElement()
{
    /* #108146# Code moved to SwXMLOmport::endDocument */
    GetImport().GetTextImport()->SetOutlineStyles( sal_False );
}

SvXMLImportContext *SwXMLImport::CreateBodyContentContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !IsStylesOnlyMode() )
         pContext = new SwXMLBodyContentContext_Impl( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );
    else
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                           rLocalName );

    return pContext;
}
