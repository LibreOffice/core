/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeprovider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:43:10 $
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
#include "precompiled_cppuhelper.hxx"

#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>

using namespace osl;
using namespace com::sun::star::uno;

namespace cppu
{

//__________________________________________________________________________________________________
OImplementationId::~OImplementationId() SAL_THROW( () )
{
    delete _pSeq;
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > OImplementationId::getImplementationId() const SAL_THROW( () )
{
    if (! _pSeq)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! _pSeq)
        {
            Sequence< sal_Int8 > * pSeq = new Sequence< sal_Int8 >( 16 );
            ::rtl_createUuid( (sal_uInt8 *)pSeq->getArray(), 0, _bUseEthernetAddress );
            _pSeq = pSeq;
        }
    }
    return *_pSeq;
}

//--------------------------------------------------------------------------------------------------
static inline void copy( Sequence< Type > & rDest, const Sequence< Type > & rSource, sal_Int32 nOffset )
    SAL_THROW( () )
{
    Type * pDest = rDest.getArray();
    const Type * pSource = rSource.getConstArray();

    for ( sal_Int32 nPos = rSource.getLength(); nPos--; )
        pDest[nOffset+ nPos] = pSource[nPos];
}

//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 1 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    copy( _aTypes, rAddTypes, 1 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 2 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    copy( _aTypes, rAddTypes, 2 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 3 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    copy( _aTypes, rAddTypes, 3 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 4 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    copy( _aTypes, rAddTypes, 4 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 5 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    copy( _aTypes, rAddTypes, 5 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 6 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    copy( _aTypes, rAddTypes, 6 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 7 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    copy( _aTypes, rAddTypes, 7 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Type & rType8,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 8 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    _aTypes[7] = rType8;
    copy( _aTypes, rAddTypes, 8 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Type & rType8,
    const Type & rType9,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 9 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    _aTypes[7] = rType8;
    _aTypes[8] = rType9;
    copy( _aTypes, rAddTypes, 9 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Type & rType8,
    const Type & rType9,
    const Type & rType10,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 10 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    _aTypes[7] = rType8;
    _aTypes[8] = rType9;
    _aTypes[9] = rType10;
    copy( _aTypes, rAddTypes, 10 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Type & rType8,
    const Type & rType9,
    const Type & rType10,
    const Type & rType11,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 11 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    _aTypes[7] = rType8;
    _aTypes[8] = rType9;
    _aTypes[9] = rType10;
    _aTypes[10] = rType11;
    copy( _aTypes, rAddTypes, 11 );
}
//__________________________________________________________________________________________________
OTypeCollection::OTypeCollection(
    const Type & rType1,
    const Type & rType2,
    const Type & rType3,
    const Type & rType4,
    const Type & rType5,
    const Type & rType6,
    const Type & rType7,
    const Type & rType8,
    const Type & rType9,
    const Type & rType10,
    const Type & rType11,
    const Type & rType12,
    const Sequence< Type > & rAddTypes )
    SAL_THROW( () )
    : _aTypes( 12 + rAddTypes.getLength() )
{
    _aTypes[0] = rType1;
    _aTypes[1] = rType2;
    _aTypes[2] = rType3;
    _aTypes[3] = rType4;
    _aTypes[4] = rType5;
    _aTypes[5] = rType6;
    _aTypes[6] = rType7;
    _aTypes[7] = rType8;
    _aTypes[8] = rType9;
    _aTypes[9] = rType10;
    _aTypes[10] = rType11;
    _aTypes[11] = rType12;
    copy( _aTypes, rAddTypes, 12 );
}

}

