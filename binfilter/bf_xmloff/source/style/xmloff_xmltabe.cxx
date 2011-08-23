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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif


#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTABE_HXX
#include "xmltabe.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

SvXMLEnumMapEntry pXML_tabstop_style[] =
{
    { XML_LEFT,     style::TabAlign_LEFT	},
    { XML_CENTER,   style::TabAlign_CENTER	},
    { XML_RIGHT,    style::TabAlign_RIGHT	},
    { XML_CHAR,     style::TabAlign_DECIMAL	},
    { XML_DEFAULT,  style::TabAlign_DEFAULT  },	// ?????????????????????????????????????
    { XML_TOKEN_INVALID,        0 }
};

void SvxXMLTabStopExport::exportTabStop( const ::com::sun::star::style::TabStop* pTabStop )
{
    SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

    // text:level
    OUStringBuffer sBuffer;

    // position attribute
    rUnitConv.convertMeasure( sBuffer, pTabStop->Position );
    rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_POSITION,
                           sBuffer.makeStringAndClear() );

    // type attribute
    if( style::TabAlign_LEFT != pTabStop->Alignment )
    {
        rUnitConv.convertEnum( sBuffer, pTabStop->Alignment,
                                   pXML_tabstop_style );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_TYPE,
                               sBuffer.makeStringAndClear() );
    }

    // char
    if( style::TabAlign_DECIMAL == pTabStop->Alignment &&
        pTabStop->DecimalChar != 0 )
    {
        sBuffer.append( pTabStop->DecimalChar );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_CHAR,
                               sBuffer.makeStringAndClear() );
    }

    // leader-char
    if( ' ' != pTabStop->FillChar && 0 != pTabStop->FillChar )
    {
        sBuffer.append( pTabStop->FillChar );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEADER_CHAR,
                               sBuffer.makeStringAndClear() );
    }

    SvXMLElementExport rElem( rExport, XML_NAMESPACE_STYLE, XML_TAB_STOP, 
                              sal_True, sal_True );
}


SvxXMLTabStopExport::SvxXMLTabStopExport( 
    SvXMLExport& rExp)
    : rExport( rExp )
{
}

SvxXMLTabStopExport::~SvxXMLTabStopExport()
{
}

void SvxXMLTabStopExport::Export( const uno::Any& rAny )
{
    uno::Sequence< ::com::sun::star::style::TabStop> aSeq;
    if(!(rAny >>= aSeq))
    {
        DBG_ERROR( "SvxXMLTabStopExport needs a Sequence ::com::sun::star::style::TabStop>" );
    }
    else
    {
        const ::com::sun::star::style::TabStop* pTabs = aSeq.getConstArray();
        const sal_Int32 nTabs   = aSeq.getLength();
        
        // ignore default tab stop here
        //if( 1 == nTabs && style::TabAlign_DEFAULT == pTabs[0].Alignment )
        //	return;

        SvXMLElementExport rElem( rExport, XML_NAMESPACE_STYLE, XML_TAB_STOPS,
                                  sal_True, sal_True );

        for( sal_Int32 nIndex = 0; nIndex < nTabs; nIndex++ )
        {
            if( style::TabAlign_DEFAULT != pTabs[nIndex].Alignment )
                exportTabStop( &(pTabs[nIndex]) );			
        }
    }
}


}//end of namespace binfilter
