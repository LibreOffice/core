/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:52:41 $
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

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

//------------------------------------------------------------------------------
staruno::Sequence<sal_Int16> findValue(const staruno::Sequence< ::rtl::OUString >& _rList, const ::rtl::OUString& _rValue, sal_Bool _bOnlyFirst)
{
    sal_Int32 nLength = _rList.getLength();

    if( _bOnlyFirst )
    {
        //////////////////////////////////////////////////////////////////////
        // An welcher Position finde ich den Wert?
        sal_Int32 nPos = -1;
        const ::rtl::OUString* pTArray = _rList.getConstArray();
        for (sal_uInt32 i = 0; i < nLength; ++i, ++pTArray)
        {
            if( pTArray->equals(_rValue) )
            {
                nPos = i;
                break;
            }
        }

        //////////////////////////////////////////////////////////////////////
        // Sequence fuellen
        if( nPos>-1 )
        {
            staruno::Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return staruno::Sequence<sal_Int16>();

    }
    else
    {
        staruno::Sequence<sal_Int16> aRetSeq( nLength );
        sal_Int16* pReturn = aRetSeq.getArray();

        //////////////////////////////////////////////////////////////////////
        // Wie oft kommt der Wert vor?
        const ::rtl::OUString* pTArray = _rList.getConstArray();
        for (sal_uInt32 i = 0; i < nLength; ++i, ++pTArray)
        {
            if( pTArray->equals(_rValue) )
            {
                *pReturn = (sal_Int16)i;
                ++pReturn;
            }
        }

        aRetSeq.realloc(pReturn - aRetSeq.getArray());

        return aRetSeq;
    }
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

