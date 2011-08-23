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



#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmlimp.hxx"

#include <com/sun/star/style/TabStop.hpp>

#include "xmluconv.hxx"

#include "xmltabi.hxx"

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

// ---

enum SvXMLTokenMapAttrs
{
    XML_TOK_TABSTOP_POSITION,
    XML_TOK_TABSTOP_TYPE,
    XML_TOK_TABSTOP_CHAR,
    XML_TOK_TABSTOP_LEADER,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aTabsAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_POSITION,	 XML_TOK_TABSTOP_POSITION },
    { XML_NAMESPACE_STYLE, XML_TYPE,	  	 XML_TOK_TABSTOP_TYPE },
    { XML_NAMESPACE_STYLE, XML_CHAR,	  	 XML_TOK_TABSTOP_CHAR },
    { XML_NAMESPACE_STYLE, XML_LEADER_CHAR,  XML_TOK_TABSTOP_LEADER },
    XML_TOKEN_MAP_END 
};

// ---

class SvxXMLTabStopContext_Impl : public SvXMLImportContext
{
private:
     style::TabStop	aTabStop;

public:
    TYPEINFO();

    SvxXMLTabStopContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    virtual ~SvxXMLTabStopContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    const style::TabStop& getTabStop() const { return aTabStop; }
};

TYPEINIT1( SvxXMLTabStopContext_Impl, SvXMLImportContext );

SvxXMLTabStopContext_Impl::SvxXMLTabStopContext_Impl(
                               SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference< xml::sax::XAttributeList > & xAttrList )
: SvXMLImportContext( rImport, nPrfx, rLName )
{
    aTabStop.Position = 0;
    aTabStop.Alignment = style::TabAlign_LEFT;
    aTabStop.DecimalChar = sal_Unicode( ',' );
    aTabStop.FillChar = sal_Unicode( ' ' );

    SvXMLTokenMap aTokenMap( aTabsAttributesAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TABSTOP_POSITION:
            if( GetImport().GetMM100UnitConverter().convertMeasure( nVal,
                                                                    rValue ) )
                aTabStop.Position = nVal;
            break;
        case XML_TOK_TABSTOP_TYPE:
            if( IsXMLToken( rValue, XML_LEFT ) )
            {
                aTabStop.Alignment = style::TabAlign_LEFT;
            }
            else if( IsXMLToken( rValue, XML_RIGHT ) )
            {
                aTabStop.Alignment = style::TabAlign_RIGHT;
            }
            else if( IsXMLToken( rValue, XML_CENTER ) )
            {
                aTabStop.Alignment = style::TabAlign_CENTER;
            }
            else if( IsXMLToken( rValue, XML_CHAR ) )
            {
                aTabStop.Alignment = style::TabAlign_DECIMAL;
            }
            else if( IsXMLToken( rValue, XML_DEFAULT ) )
            {
                aTabStop.Alignment = style::TabAlign_DEFAULT;
            }
            break;
        case XML_TOK_TABSTOP_CHAR:
            if( 0 != rValue.getLength() )
                aTabStop.DecimalChar = rValue[0];
            break;
        case XML_TOK_TABSTOP_LEADER:
            if( 0 != rValue.getLength() )
                aTabStop.FillChar = rValue[0];
            break;
        }
    }
}

SvxXMLTabStopContext_Impl::~SvxXMLTabStopContext_Impl()
{
}

SvXMLImportContext *SvxXMLTabStopContext_Impl::CreateChildContext(
                                   sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}




typedef SvxXMLTabStopContext_Impl *SvxXMLTabStopContext_Impl_ImplPtr;
SV_DECL_PTRARR( SvxXMLTabStopArray_Impl, SvxXMLTabStopContext_Impl_ImplPtr,	20, 5 )


// ---

TYPEINIT1( SvxXMLTabStopImportContext, XMLElementPropertyContext );

SvxXMLTabStopImportContext::SvxXMLTabStopImportContext(
                                SvXMLImport& rImport, sal_uInt16 nPrfx,
                                const OUString& rLName,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps )
: XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
  mpTabStops( NULL )
{
}

SvxXMLTabStopImportContext::~SvxXMLTabStopImportContext()
{
    if( mpTabStops )
    {
        sal_uInt16 nCount = mpTabStops->Count();
        while( nCount )
        {
            nCount--;
            SvxXMLTabStopContext_Impl *pTabStop = (*mpTabStops)[nCount];
            mpTabStops->Remove( nCount, 1 );
            pTabStop->ReleaseRef();
        }
    }

    delete mpTabStops;
}

SvXMLImportContext *SvxXMLTabStopImportContext::CreateChildContext(
                                   sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix && IsXMLToken( rLocalName, XML_TAB_STOP ) )
    {
        // create new tabstop import context
        SvxXMLTabStopContext_Impl *pTabStopContext =
            new SvxXMLTabStopContext_Impl( GetImport(), nPrefix, rLocalName,
                                           xAttrList );

        // add new tabstop to array of tabstops
        if( !mpTabStops )
            mpTabStops = new SvxXMLTabStopArray_Impl;

        mpTabStops->Insert( pTabStopContext, mpTabStops->Count() );
        pTabStopContext->AddRef();
        
        pContext = pTabStopContext;
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    
    return pContext;
}

void SvxXMLTabStopImportContext::EndElement( )
{	
    sal_uInt16 nCount = mpTabStops ? mpTabStops->Count() : 0;
    uno::Sequence< style::TabStop> aSeq( nCount );
        
    if( mpTabStops )
    {
        sal_uInt16 nNewCount = 0;

        style::TabStop* pTabStops = aSeq.getArray();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            SvxXMLTabStopContext_Impl *pTabStopContext = (*mpTabStops)[i];
            const style::TabStop& rTabStop = pTabStopContext->getTabStop();
            sal_Bool bDflt = style::TabAlign_DEFAULT == rTabStop.Alignment;
            if( !bDflt || 0==i )
            {
                *pTabStops++ = pTabStopContext->getTabStop();
                nNewCount++;
            }
            if( bDflt && 0==i )
                break;
        }

        if( nCount != nNewCount )
            aSeq.realloc( nNewCount );
    }
    aProp.maValue <<= aSeq;

    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

}




}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
