/*************************************************************************
 *
 *  $RCSfile: xmltabe.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:29:00 $
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
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry pXML_tabstop_style[] =
{
    { XML_LEFT,     style::TabAlign_LEFT    },
    { XML_CENTER,   style::TabAlign_CENTER  },
    { XML_RIGHT,    style::TabAlign_RIGHT   },
    { XML_CHAR,     style::TabAlign_DECIMAL },
    { XML_DEFAULT,  style::TabAlign_DEFAULT  }, // ?????????????????????????????????????
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
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEADER_STYLE,
                      GetXMLToken('.' == pTabStop->FillChar ? XML_DOTTED
                                                               : XML_SOLID) );

        sBuffer.append( pTabStop->FillChar );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEADER_TEXT,
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
        //  return;

        SvXMLElementExport rElem( rExport, XML_NAMESPACE_STYLE, XML_TAB_STOPS,
                                  sal_True, sal_True );

        for( sal_Int32 nIndex = 0; nIndex < nTabs; nIndex++ )
        {
            if( style::TabAlign_DEFAULT != pTabs[nIndex].Alignment )
                exportTabStop( &(pTabs[nIndex]) );
        }
    }
}


