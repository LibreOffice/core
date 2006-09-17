/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: querytokenizer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:38:09 $
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
#include "precompiled_filter.hxx"

#include "querytokenizer.hxx"

//_______________________________________________
// includes

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

/*-----------------------------------------------
    01.08.2003 10:42
-----------------------------------------------*/
QueryTokenizer::QueryTokenizer(const ::rtl::OUString& sQuery)
    : m_bValid(sal_True)
{
    sal_Int32 token = 0;
    while(token != -1)
    {
        ::rtl::OUString sToken = sQuery.getToken(0, ':', token);
        if (sToken.getLength())
        {
            sal_Int32 equal = sToken.indexOf('=');

            if (equal == 0)
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nFound non boolean query parameter ... but its key is empty. Will be ignored!\n");

            ::rtl::OUString sKey;
            ::rtl::OUString sVal;

            sKey = sToken;
            if (equal > 0)
            {
                sKey = sToken.copy(0      , equal                       );
                sVal = sToken.copy(equal+1, sToken.getLength()-(equal+1));
            }

            if (find(sKey) != end())
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nQuery contains same param more then once. Last one wins :-)\n");

            (*this)[sKey] = sVal;
        }
    }
}

/*-----------------------------------------------
    01.08.2003 10:28
-----------------------------------------------*/
QueryTokenizer::~QueryTokenizer()
{
    /*TODO*/
}

/*-----------------------------------------------
    01.08.2003 10:53
-----------------------------------------------*/
sal_Bool QueryTokenizer::valid() const
{
    return m_bValid;
}

    } // namespace config
} // namespace filter
