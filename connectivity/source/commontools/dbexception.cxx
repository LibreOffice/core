/*************************************************************************
 *
 *  $RCSfile: dbexception.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 08:50:41 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace comphelper;

//.........................................................................
namespace dbtools
{
//.........................................................................

//==============================================================================
//= SQLExceptionInfo - encapsulating the type info of an SQLException-derived class
//==============================================================================
//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo()
    :m_eType(UNDEFINED)
{
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const starsdbc::SQLException& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const starsdbc::SQLWarning& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const starsdb::SQLContext& _rError)
{
    m_aContent <<= _rError;
    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const SQLExceptionInfo& _rCopySource)
    :m_aContent(_rCopySource.m_aContent)
    ,m_eType(_rCopySource.m_eType)
{
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const starsdb::SQLErrorEvent& _rError)
{
    const staruno::Type& aSQLExceptionType = ::getCppuType(reinterpret_cast<starsdbc::SQLException*>(NULL));
    staruno::Type aReasonType = _rError.Reason.getValueType();

    sal_Bool bValid = isAssignableFrom(aSQLExceptionType, aReasonType);
    OSL_ENSHURE(bValid, "SQLExceptionInfo::SQLExceptionInfo : invalid argument (does not contain an SQLException) !");
    if (bValid)
        m_aContent = _rError.Reason;

    implDetermineType();
}

//------------------------------------------------------------------------------
SQLExceptionInfo::SQLExceptionInfo(const staruno::Any& _rError)
{
    const staruno::Type& aSQLExceptionType = ::getCppuType(reinterpret_cast<starsdbc::SQLException*>(NULL));
    sal_Bool bValid = isAssignableFrom(aSQLExceptionType, _rError.getValueType());
    if (bValid)
        m_aContent = _rError;
    // no assertion here : if used with the NextException member of an SQLException bValid==sal_False is allowed.

    implDetermineType();
}

//------------------------------------------------------------------------------
void SQLExceptionInfo::implDetermineType()
{
    staruno::Type aContentType = m_aContent.getValueType();
    if (isA(aContentType, static_cast<starsdb::SQLContext*>(NULL)))
        m_eType = SQL_CONTEXT;
    else if (isA(aContentType, static_cast<starsdbc::SQLWarning*>(NULL)))
        m_eType = SQL_WARNING;
    else if (isA(aContentType, static_cast<starsdbc::SQLException*>(NULL)))
        m_eType = SQL_EXCEPTION;
    else
        m_eType = UNDEFINED;
}

//------------------------------------------------------------------------------
sal_Bool SQLExceptionInfo::isKindOf(TYPE _eType) const
{
    switch (_eType)
    {
        case SQL_CONTEXT:
            return (m_eType == SQL_CONTEXT);
        case SQL_WARNING:
            return (m_eType == SQL_CONTEXT) || (m_eType == SQL_WARNING);
        case SQL_EXCEPTION:
            return (m_eType == SQL_CONTEXT) || (m_eType == SQL_WARNING) || (m_eType == SQL_EXCEPTION);
        case UNDEFINED:
            return (m_eType == UNDEFINED);
    }
    return sal_False;
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const starsdbc::SQLException*() const
{
    OSL_ENSHURE(isKindOf(SQL_EXCEPTION), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const starsdbc::SQLException*>(m_aContent.getValue());
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const starsdbc::SQLWarning*() const
{
    OSL_ENSHURE(isKindOf(SQL_WARNING), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const starsdbc::SQLWarning*>(m_aContent.getValue());
}

//------------------------------------------------------------------------------
SQLExceptionInfo::operator const starsdb::SQLContext*() const
{
    OSL_ENSHURE(isKindOf(SQL_CONTEXT), "SQLExceptionInfo::operator SQLException* : invalid call !");
    return reinterpret_cast<const starsdb::SQLContext*>(m_aContent.getValue());
}

//==============================================================================
//= SQLExceptionIteratorHelper - iterating through an SQLException chain
//==============================================================================

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const starsdbc::SQLException* _pStart, NODES_INCLUDED _eMask)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_EXCEPTION)
                // no other chance without RTTI
            ,m_eMask(_eMask)
{
    // initially check the start of the chain against the include mask
    if (m_pCurrent && (m_eMask >= NI_EXCEPTIONS))
        next();
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const starsdbc::SQLWarning* _pStart, NODES_INCLUDED _eMask)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_WARNING)
                // no other chance without RTTI
            ,m_eMask(_eMask)
{
    // initially check the start of the chain against the include mask
    if (m_pCurrent && (m_eMask >= NI_WARNINGS))
        next();
}

//------------------------------------------------------------------------------
SQLExceptionIteratorHelper::SQLExceptionIteratorHelper(const starsdb::SQLContext* _pStart, NODES_INCLUDED _eMask)
            :m_pCurrent(_pStart)
            ,m_eCurrentType(SQLExceptionInfo::SQL_CONTEXT)
                // no other chance without RTTI
            ,m_eMask(_eMask)
{
    // initially check the start of the chain against the include mask
    if (m_pCurrent && (m_eMask >= NI_CONTEXTINFOS))
        next();
}

//------------------------------------------------------------------------------
const starsdbc::SQLException* SQLExceptionIteratorHelper::next()
{
    OSL_ENSHURE(hasMoreElements(), "SQLExceptionIteratorHelper::next : invalid call (please use hasMoreElements) !");

    const starsdbc::SQLException* pReturn = m_pCurrent;
    if (m_pCurrent)
    {   // check for the next element within the chain
        const staruno::Type& aSqlExceptionCompare = ::getCppuType(reinterpret_cast<starsdbc::SQLException*>(NULL));
        const staruno::Type& aSqlWarningCompare = ::getCppuType(reinterpret_cast<starsdbc::SQLWarning*>(NULL));
        const staruno::Type& aSqlContextCompare = ::getCppuType(reinterpret_cast<starsdb::SQLContext*>(NULL));

        const starsdbc::SQLException* pSearch           = m_pCurrent;
        SQLExceptionInfo::TYPE eSearchType  = m_eCurrentType;

        sal_Bool bIncludeThis = sal_False;
        while (pSearch && !bIncludeThis)
        {
            if (!pSearch->NextException.hasValue())
            {   // last chain element
                pSearch = NULL;
                break;
            }
            staruno::Type aNextElementType = pSearch->NextException.getValueType();
            if (!isAssignableFrom(aSqlExceptionCompare, aNextElementType))
            {
                // the next chain element isn't an SQLException
                OSL_ENSHURE(sal_False, "SQLExceptionIteratorHelper::next : the exception chain is invalid !");
                pSearch = NULL;
                break;
            }

            // the next element
            SQLExceptionInfo aInfo(pSearch->NextException);
            eSearchType = aInfo.getType();
            switch (eSearchType)
            {
                case SQLExceptionInfo::SQL_CONTEXT:
                    pSearch = reinterpret_cast<const starsdb::SQLContext*>(pSearch->NextException.getValue());
                    bIncludeThis = eSearchType >= NI_CONTEXTINFOS;
                    break;

                case SQLExceptionInfo::SQL_WARNING:
                    pSearch = reinterpret_cast<const starsdbc::SQLWarning*>(pSearch->NextException.getValue());
                    bIncludeThis = eSearchType >= NI_WARNINGS;
                    break;

                case SQLExceptionInfo::SQL_EXCEPTION:
                    pSearch = reinterpret_cast<const starsdbc::SQLException*>(pSearch->NextException.getValue());
                    bIncludeThis = eSearchType >= NI_EXCEPTIONS;
                    break;

                default:
                    pSearch = NULL;
                    bIncludeThis = sal_False;
                    break;
            }
        }

        m_pCurrent = pSearch;
        m_eCurrentType = eSearchType;
    }

    return pReturn;
}

//.........................................................................
}   // namespace dbtools
//.........................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 29.09.00 08:17:11  fs
 ************************************************************************/

