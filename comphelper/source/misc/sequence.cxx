/*************************************************************************
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

