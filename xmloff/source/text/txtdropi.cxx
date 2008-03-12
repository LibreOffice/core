/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtdropi.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:07:42 $
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

#ifndef _COM_SUN_STAR_STYLE_DROPCAPFORMAT_HPP_
#include <com/sun/star/style/DropCapFormat.hpp>
#endif
#ifndef _TXTDROPI_HXX
#include "txtdropi.hxx"
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;


enum SvXMLTokenMapDropAttrs
{
    XML_TOK_DROP_LINES,
    XML_TOK_DROP_LENGTH,
    XML_TOK_DROP_DISTANCE,
    XML_TOK_DROP_STYLE,
    XML_TOK_DROP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aDropAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_LINES,       XML_TOK_DROP_LINES  },
    { XML_NAMESPACE_STYLE, XML_LENGTH,      XML_TOK_DROP_LENGTH },
    { XML_NAMESPACE_STYLE, XML_DISTANCE,    XML_TOK_DROP_DISTANCE   },
    { XML_NAMESPACE_STYLE, XML_STYLE_NAME,  XML_TOK_DROP_STYLE  },
    XML_TOKEN_MAP_END
};

TYPEINIT1( XMLTextDropCapImportContext, XMLElementPropertyContext );
void XMLTextDropCapImportContext::ProcessAttrs(
        const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( aDropAttrTokenMap );

    DropCapFormat aFormat;
    sal_Bool bWholeWord = sal_False;

    sal_Int32 nTmp;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_DROP_LINES:
            if( GetImport().GetMM100UnitConverter().convertNumber( nTmp, rValue, 0, 255 ) )
            {
                aFormat.Lines = nTmp < 2 ? 0 : (sal_Int8)nTmp;
            }
            break;

        case XML_TOK_DROP_LENGTH:
            if( IsXMLToken( rValue, XML_WORD ) )
            {
                bWholeWord = sal_True;
            }
            else if( GetImport().GetMM100UnitConverter().convertNumber( nTmp, rValue, 1, 255 ) )
            {
                bWholeWord = sal_False;
                aFormat.Count = (sal_Int8)nTmp;
            }
            break;

        case XML_TOK_DROP_DISTANCE:
            if( GetImport().GetMM100UnitConverter().convertMeasure( nTmp, rValue, 0 ) )
            {
                aFormat.Distance = (sal_uInt16)nTmp;
            }
            break;

        case XML_TOK_DROP_STYLE:
            sStyleName = rValue;
            break;
        }
    }

    if( aFormat.Lines > 1 && aFormat.Count < 1 )
        aFormat.Count = 1;

    aProp.maValue <<= aFormat;

    aWholeWordProp.maValue.setValue( &bWholeWord, ::getBooleanCppuType() );
}

XMLTextDropCapImportContext::XMLTextDropCapImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nWholeWordIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
    aWholeWordProp( nWholeWordIdx )
{
    ProcessAttrs( xAttrList );
}

XMLTextDropCapImportContext::~XMLTextDropCapImportContext()
{
}

void XMLTextDropCapImportContext::EndElement()
{
    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

    if( -1 != aWholeWordProp.mnIndex )
        rProperties.push_back( aWholeWordProp );
}


