/*************************************************************************
 *
 *  $RCSfile: XMLClipPropertyHandler.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:04 $
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

#ifndef _XMLOFF_XMLCLIPPROPERTYHANDLER_HXX
#include "XMLClipPropertyHandler.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmlkywd.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePropHdl
//

XMLClipPropertyHandler::~XMLClipPropertyHandler()
{
    // nothing to do
}

sal_Bool XMLClipPropertyHandler::equals(
        const Any& r1,
        const Any& r2 ) const
{
    GraphicCrop aCrop1, aCrop2;
    r1 >>= aCrop1;
    r2 >>= aCrop2;

    return aCrop1.Top == aCrop2.Top &&
           aCrop1.Bottom == aCrop2.Bottom &&
           aCrop1.Left == aCrop2.Left &&
           aCrop1.Right == aCrop2.Right;
}

sal_Bool XMLClipPropertyHandler::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nLen = rStrImpValue.getLength();
    if( nLen > 6 &&
        0 == rStrImpValue.compareToAscii( sXML_rect, 4 ) &&
        rStrImpValue[4] == '(' &&
        rStrImpValue[nLen-1] == ')' )
    {
        GraphicCrop aCrop;
        OUString sTmp( rStrImpValue.copy( 5, nLen-6 ) );
        SvXMLTokenEnumerator aTokenEnum( sTmp );

        sal_uInt16 nPos = 0;
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            sal_Int32 nVal = 0;
            if( aToken.compareToAscii( sXML_auto ) != 0 &&
                 !rUnitConverter.convertMeasure( nVal, aToken ) )
                break;

            switch( nPos )
            {
            case 0: aCrop.Top = nVal;   break;
            case 1: aCrop.Right = nVal; break;
            case 2: aCrop.Bottom = nVal;    break;
            case 3: aCrop.Left = nVal;  break;
            }
            nPos++;
        }

        bRet = (4 == nPos );
        if( bRet )
            rValue <<= aCrop;
    }

    return bRet;
}

sal_Bool XMLClipPropertyHandler::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut(30);
    GraphicCrop aCrop;

    if( rValue >>= aCrop )
    {
        aOut.appendAscii( sXML_rect );
        aOut.append( (sal_Unicode)'(' );
        rUnitConverter.convertMeasure( aOut, aCrop.Top );
        aOut.append( (sal_Unicode)' ' );
        rUnitConverter.convertMeasure( aOut, aCrop.Right );
        aOut.append( (sal_Unicode)' ' );
        rUnitConverter.convertMeasure( aOut, aCrop.Bottom );
        aOut.append( (sal_Unicode)' ' );
        rUnitConverter.convertMeasure( aOut, aCrop.Left );
        aOut.append( (sal_Unicode)')' );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}
