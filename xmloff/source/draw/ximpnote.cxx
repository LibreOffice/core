/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ximpnote.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:38:07 $
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

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::SdXMLNotesContext( SdXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
    OUString sStyleName;

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetMasterPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
            {
                msPageMasterName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_STYLE_NAME:
            {
                sStyleName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_HEADER_NAME:
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_FOOTER_NAME:
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME:
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }

        }
    }

    SetStyle( sStyleName );

    // now delete all up-to-now contained shapes from this notes page
    uno::Reference< drawing::XShape > xShape;
    while(rShapes->getCount())
    {
        rShapes->getByIndex(0L) >>= xShape;
        if(xShape.is())
            rShapes->remove(xShape);
    }

    // set page-master?
    if(msPageMasterName.getLength())
    {
        SetPageMaster( msPageMasterName );
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::~SdXMLNotesContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLNotesContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    // OK, notes page is set on base class, objects can be imported on notes page
    SvXMLImportContext *pContext = 0L;

    // some special objects inside presentation:notes context
    // ...







    // call parent when no own context was created
    if(!pContext)
        pContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLNotesContext::EndElement()
{
    SdXMLGenericPageContext::EndElement();
}
