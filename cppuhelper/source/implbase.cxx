/*************************************************************************
 *
 *  $RCSfile: implbase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:28 $
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

#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include <com/sun/star/lang/XComponent.hpp>

using namespace osl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{
//==================================================================================================
Mutex & SAL_CALL getImplHelperInitMutex(void) SAL_THROW( () )
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMutex)
        {
            static Mutex s_aMutex;
            s_pMutex = & s_aMutex;
        }
    }
    return * s_pMutex;
}

// ClassDataBase
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase() SAL_THROW( () )
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( 0 )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase( sal_Int32 nClassCode_ ) SAL_THROW( () )
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( nClassCode_ )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::~ClassDataBase() SAL_THROW( () )
{
    delete pTypes;
    delete pId;

    for ( sal_Int32 nPos = nType2Offset; nPos--; )
    {
        typelib_typedescription_release(
            (typelib_TypeDescription *)((ClassData *)this)->arType2Offset[nPos].pTD );
    }
}

// ClassData
//__________________________________________________________________________________________________
void ClassData::writeTypeOffset( const Type & rType, sal_Int32 nOffset ) SAL_THROW( () )
{
    arType2Offset[nType2Offset].nOffset = nOffset;

    arType2Offset[nType2Offset].pTD = 0;
    typelib_typedescriptionreference_getDescription(
        (typelib_TypeDescription **)&arType2Offset[nType2Offset].pTD, rType.getTypeLibType() );

    if (arType2Offset[nType2Offset].pTD)
        ++nType2Offset;
#ifdef DEBUG
    else
    {
        OString msg( "### cannot get type description for " );
        msg += OUStringToOString( rType.getTypeName(), RTL_TEXTENCODING_ASCII_US );
        OSL_ENSHURE( sal_False, msg.getStr() );
    }
#endif
}
//__________________________________________________________________________________________________
void ClassData::initTypeProvider() SAL_THROW( () )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if (! pTypes)
    {
        // create id
        pId = new Sequence< sal_Int8 >( 16 );
        rtl_createUuid( (sal_uInt8 *)pId->getArray(), 0, sal_True );

        // collect types
        Sequence< Type > * types = new Sequence< Type >(
            nType2Offset + 1 + (nClassCode == 4 ? 2 : nClassCode) );
        Type * pTypeAr = types->getArray();

        // given types
        sal_Int32 nPos = nType2Offset;
        while (nPos--)
            pTypeAr[nPos] = ((typelib_TypeDescription *)arType2Offset[nPos].pTD)->pWeakRef;

        // XTypeProvider
        pTypeAr[nType2Offset] = ::getCppuType( (const Reference< XTypeProvider > *)0 );

        // class code extra types: [[XComponent,] XWeak[, XAggregation]]
        switch (nClassCode)
        {
        case 4:
            pTypeAr[nType2Offset +2] = ::getCppuType( (const Reference< XComponent > *)0 );
            pTypeAr[nType2Offset +1] = ::getCppuType( (const Reference< XWeak > *)0 );
            break;
        case 3:
            pTypeAr[nType2Offset +3] = ::getCppuType( (const Reference< XComponent > *)0 );
        case 2:
            pTypeAr[nType2Offset +2] = ::getCppuType( (const Reference< XAggregation > *)0 );
        case 1:
            pTypeAr[nType2Offset +1] = ::getCppuType( (const Reference< XWeak > *)0 );
        }

        pTypes = types;
    }
}
//__________________________________________________________________________________________________
Sequence< Type > ClassData::getTypes() SAL_THROW( () )
{
    if (! pTypes)
        initTypeProvider();
    return *pTypes;
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > ClassData::getImplementationId() SAL_THROW( () )
{
    if (! pTypes)
        initTypeProvider();
    return *pId;
}

//--------------------------------------------------------------------------------------------------
static inline sal_Bool td_equals(
    typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}
//__________________________________________________________________________________________________
Any ClassData::query( const Type & rType, XTypeProvider * pBase ) SAL_THROW( () )
{
    if (rType == ::getCppuType( (const Reference< XInterface > *)0 ))
        return Any( &pBase, ::getCppuType( (const Reference< XInterface > *)0 ) );
    for ( sal_Int32 nPos = 0; nPos < nType2Offset; ++nPos )
    {
        const Type_Offset & rTO = arType2Offset[nPos];
        typelib_InterfaceTypeDescription * pTD = rTO.pTD;
        while (pTD)
        {
            if (td_equals( (typelib_TypeDescription *)pTD,
                           *(typelib_TypeDescriptionReference **)&rType ))
            {
                void * pInterface = (char *)pBase + rTO.nOffset;
                return Any( &pInterface, (typelib_TypeDescription *)pTD );
            }
            pTD = pTD->pBaseTypeDescription;
        }
    }
    if (rType == ::getCppuType( (const Reference< XTypeProvider > *)0 ))
        return Any( &pBase, ::getCppuType( (const Reference< XTypeProvider > *)0 ) );

    return Any();
}

}

