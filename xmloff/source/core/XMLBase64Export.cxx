/*************************************************************************
 *
 *  $RCSfile: XMLBase64Export.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2001-06-19 14:51:24 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLBASE64EXPORT_HXX
#include "XMLBase64Export.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::rtl;

#define INPUT_BUFFER_SIZE 54
#define OUTPUT_BUFFER_SIZE 72

XMLBase64Export::XMLBase64Export( SvXMLExport& rExp ) :
    rExport( rExp ){
}

sal_Bool XMLBase64Export::exportXML( const Reference < XInputStream> & rIn )
{
    sal_Bool bRet = sal_True;
    try
    {
        Sequence < sal_Int8 > aInBuff( INPUT_BUFFER_SIZE );
        OUStringBuffer aOutBuff( OUTPUT_BUFFER_SIZE );
        sal_Int32 nRead;
        do
        {
            nRead = rIn->readBytes( aInBuff, INPUT_BUFFER_SIZE );
            if( nRead > 0 )
            {
                GetExport().GetMM100UnitConverter().encodeBase64( aOutBuff,
                                                                  aInBuff );
                GetExport().GetDocHandler()->characters(
                                            aOutBuff.makeStringAndClear() );
                if( nRead == INPUT_BUFFER_SIZE )
                    GetExport().GetDocHandler()->ignorableWhitespace(
                                            GetExport().sWS );
            }
        }
        while( nRead == INPUT_BUFFER_SIZE );
    }
    catch( ... )
    {
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool XMLBase64Export::exportElement(
            const Reference < XInputStream > & rIn,
            sal_uInt16 nNamespace,
            enum ::xmloff::token::XMLTokenEnum eName )
{
    SvXMLElementExport aElem( GetExport(), nNamespace, eName, sal_True,
                              sal_True );
    return exportXML( rIn );
}

sal_Bool XMLBase64Export::exportOfficeBinaryDataElement(
            const Reference < XInputStream > & rIn )
{
    return exportElement( rIn, XML_NAMESPACE_OFFICE,
                             ::xmloff::token::XML_BINARY_DATA );
}

