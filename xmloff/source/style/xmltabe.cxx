/*************************************************************************
 *
 *  $RCSfile: xmltabe.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_TABALIGN_HPP_
#include <com/sun/star/style/TabAlign.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLEXPIT_HXX
#include "xmlexpit.hxx"
#endif

#ifndef _XMLOFF_XMLTABE_HXX
#include "xmltabe.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

SvXMLEnumMapEntry psXML_tabstop_style[] =
{
    { "left", style::TabAlign_LEFT  },
    { "center", style::TabAlign_CENTER  },
    { "right", style::TabAlign_RIGHT    },
    { "char", style::TabAlign_DECIMAL   },
    { "????", style::TabAlign_DEFAULT  },   // ?????????????????????????????????????
    { 0,        0 }
};

void SvxXMLTabStopExport::exportTabStop( const ::com::sun::star::style::TabStop* pTabStop )
{
    CheckAttrList();

    // text:level
    OUStringBuffer sBuffer;

    // position attribute
    mrUnitConv.convertMeasure( sBuffer, pTabStop->Position );
    AddAttribute( XML_NAMESPACE_STYLE, sXML_tabstop_position,
                  sBuffer.makeStringAndClear() );

    // type attribute
    if( style::TabAlign_LEFT != pTabStop->Alignment )
    {
        mrUnitConv.convertEnum( sBuffer, pTabStop->Alignment,
                                   psXML_tabstop_style );
        AddAttribute( XML_NAMESPACE_STYLE, sXML_tabstop_type,
                      sBuffer.makeStringAndClear() );
    }

    // char
    if( style::TabAlign_DECIMAL == pTabStop->Alignment )
    {
        sBuffer.append( pTabStop->DecimalChar );
        AddAttribute( XML_NAMESPACE_STYLE, sXML_tabstop_char,
                      sBuffer.makeStringAndClear() );
    }

    // leader-char
    if( ' ' != pTabStop->FillChar )
    {
        sBuffer.append( pTabStop->FillChar );
        AddAttribute( XML_NAMESPACE_STYLE, sXML_tabstop_leader,
                      sBuffer.makeStringAndClear() );
    }

    OUString sElem = GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_tabstop_tab_stop) );
    mxHandler->ignorableWhitespace( msWS );
    mxHandler->startElement( sElem, mxAttrList );
    ClearAttrList();
    mxHandler->ignorableWhitespace( msWS );
    mxHandler->endElement( sElem );
}

OUString SvxXMLTabStopExport::GetQNameByKey( sal_uInt16 nKey,
                                               const OUString& rLocalName ) const
{
    DBG_ASSERT( mpNamespaceMap,
                "SvxXMLTabStopExport::GetQNameByKey: namespace map is missing" );
    if( mpNamespaceMap )
        return mpNamespaceMap->GetQNameByKey( nKey, rLocalName );
    else
        return rLocalName;
}

void SvxXMLTabStopExport::ClearAttrList()
{
    mpAttrList->Clear();
}

#ifndef PRODUCT
void SvxXMLTabStopExport::CheckAttrList()
{
    DBG_ASSERT( !mpAttrList->getLength(),
                "SvxXMLTabStopExport::CheckAttrList: list is not empty" );
}
#endif

void SvxXMLTabStopExport::AddAttribute( sal_uInt16 nPrefixKey, const char *pName,
                                       const OUString& rValue )
{
    OUString sName( OUString::createFromAscii( pName ) );

    mpAttrList->AddAttribute( GetQNameByKey( nPrefixKey, sName ),
                             msCDATA, rValue );
}

SvxXMLTabStopExport::SvxXMLTabStopExport( const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                                      const SvXMLUnitConverter& rUnitConverter )
:   msCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    msWS( OUString::createFromAscii( sXML_WS ) ),
    mpNamespaceMap( 0 ),
    mrUnitConv( rUnitConverter ),
    mpAttrList( new SvXMLAttributeList )
{
    mxHandler = rHandler;
    mxAttrList = mpAttrList;
}

SvxXMLTabStopExport::~SvxXMLTabStopExport()
{
}

void SvxXMLTabStopExport::Export( const uno::Any& rAny,
                                  const SvXMLNamespaceMap& rNamespaceMap )
{
    mpNamespaceMap = &rNamespaceMap;

    CheckAttrList();


    uno::Sequence< ::com::sun::star::style::TabStop> aSeq;
    if(!(rAny >>= aSeq))
    {
        DBG_ERROR( "SvxXMLTabStopExport needs a Sequence ::com::sun::star::style::TabStop>" );
    }
    else
    {
        const ::com::sun::star::style::TabStop* pTabs = aSeq.getConstArray();
        const sal_uInt16 nTabs   = aSeq.getLength();

        // ignore default tab stop here
        if( 1 == nTabs && style::TabAlign_DEFAULT == pTabs[0].Alignment )
            return;

        OUString sElem = GetQNameByKey( XML_NAMESPACE_STYLE,
                                       OUString::createFromAscii(sXML_tabstop_tab_stops) );
        mxHandler->ignorableWhitespace( msWS );
        mxHandler->startElement( sElem, mxAttrList );
        ClearAttrList();

        for( sal_uInt16 nIndex = 0; nIndex < nTabs; nIndex++ )
        {
            DBG_ASSERT( style::TabAlign_DEFAULT == pTabs[nIndex].Alignment ||
                        nIndex == 0 ||
                        style::TabAlign_DEFAULT != pTabs[nIndex-1].Alignment,
                        "tab default distance unexpected" );
            if( style::TabAlign_DEFAULT != pTabs[nIndex].Alignment )
                exportTabStop( &(pTabs[nIndex]) );
        }

        mxHandler->ignorableWhitespace( msWS );
        mxHandler->endElement( sElem );
    }

    mpNamespaceMap = 0;
}


