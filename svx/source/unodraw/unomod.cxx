/*************************************************************************
 *
 *  $RCSfile: unomod.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-22 18:14:28 $
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

#define _SVX_USE_UNOGLOBALS_

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NOSUPPORTEXCEPTION_HPP_
#include <com/sun/star/lang/NoSupportException.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#include "globl3d.hxx"
#include "svdtypes.hxx"
#include "unoprov.hxx"
#include "unomod.hxx"
#include "unopage.hxx"
#include "unofield.hxx"

extern UHashMapEntry pSdrShapeIdentifierMap[];

using namespace ::rtl;
using namespace ::com::sun::star;

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstance( const OUString& ServiceSpecifier )
    throw( uno::Exception, uno::RuntimeException )
{
    const OUString aDrawingPrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.") );

    if( ServiceSpecifier.compareTo( aDrawingPrefix, aDrawingPrefix.getLength() ) == 0 )
    {
        OUString aShapeType( ServiceSpecifier.copy( aDrawingPrefix.getLength() ) );

        UINT32 nType = aSdrShapeIdentifierMap.getId( aShapeType );
        if( nType != UHASHMAP_NOTFOUND )
        {
            UINT16 nT = (UINT16)(nType & ~E3D_INVENTOR_FLAG);
            UINT32 nI = (nType & E3D_INVENTOR_FLAG)?E3dInventor:SdrInventor;

            return uno::Reference< uno::XInterface >( (drawing::XShape*) SvxDrawPage::CreateShapeByTypeAndInventor( nT, nI ) );
        }
    }

    const OUString aTextFieldPrexit( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField.") );

    if( ServiceSpecifier.compareTo( aTextFieldPrexit, aTextFieldPrexit.getLength() ) == 0 )
    {
        OUString aFieldType( ServiceSpecifier.copy( aTextFieldPrexit.getLength() ) );

        sal_Int32 nId = ID_NOTFOUND;

        if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("DateTime") ) )
        {
            nId = ID_DATEFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("URL") ) )
        {
            nId = ID_URLFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("PageNumber") ) )
        {
            nId = ID_PAGEFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("PageCount") ) )
        {
            nId = ID_PAGESFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SheetName") ) )
        {
            nId = ID_TABLEFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("FileName") ) )
        {
            nId = ID_EXT_FILEFIELD;
        }
        else if( aFieldType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Author") ) )
        {
            nId = ID_AUTHORFIELD;
        }

        if( nId != ID_NOTFOUND )
            return (::cppu::OWeakObject * )new SvxUnoTextField( nId );
    }

    throw lang::ServiceNotRegisteredException();

    return uno::Reference< uno::XInterface >();
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstanceWithArguments( const OUString& ServiceSpecifier, const uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
    throw( uno::Exception, uno::RuntimeException )
{
    throw lang::NoSupportException();
    return uno::Reference< uno::XInterface >();
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawMSFactory::getAvailableServiceNames()
    throw( uno::RuntimeException )
{
    const OUString aPrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.") );

    UHashMapEntry* pMap = pSdrShapeIdentifierMap;

    UINT32 nCount = 0;
    while (pMap->aIdentifier.getLength())
    {
        pMap++;
        nCount++;
    }

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    pMap = pSdrShapeIdentifierMap;
    UINT32 nIdx = 0;
    while(pMap->aIdentifier.getLength())
    {
        OUString aServiceName( aPrefix );
        aServiceName += pMap->aIdentifier;
        pStrings[nIdx] = aServiceName;
        pMap++;
        nIdx++;
    }

    return aSeq;
}

uno::Sequence< OUString > SvxUnoDrawMSFactory::concatServiceNames( uno::Sequence< OUString >& rServices1, uno::Sequence< OUString >& rServices2 ) throw()
{
    const sal_Int32 nLen1 = rServices1.getLength();
    const sal_Int32 nLen2 = rServices2.getLength();

    uno::Sequence< OUString > aSeq( nLen1+nLen2 );
    OUString* pStrings = aSeq.getArray();

    sal_Int32 nIdx;
    OUString* pStringDst = pStrings;
    const OUString* pStringSrc = rServices1.getArray();

    for( nIdx = 0; nIdx < nLen1; nIdx++ )
        *pStringDst++ = *pStringSrc++;

    pStringSrc = rServices2.getArray();

    for( nIdx = 0; nIdx < nLen2; nIdx++ )
        *pStringDst++ = *pStringSrc++;

    return aSeq;
}


