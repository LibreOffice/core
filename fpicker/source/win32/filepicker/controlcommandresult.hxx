/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlcommandresult.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONTROLCOMMANDRESULT_HXX_
#define _CONTROLCOMMANDRESULT_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

//---------------------------------------------
// declaration
//---------------------------------------------

class CControlCommandResult
{
public:
    CControlCommandResult( sal_Bool bResult = sal_False ) :
        m_bResult( bResult )
    {
    }

    virtual ~CControlCommandResult( )
    {
    }

    sal_Bool SAL_CALL hasResult( ) const
    {
        return m_bResult;
    }

private:
    sal_Bool m_bResult;
};

//---------------------------------------------
//
//---------------------------------------------

class CValueCommandResult : public CControlCommandResult
{
public:
    CValueCommandResult( sal_Bool bResult, const ::com::sun::star::uno::Any& aValue ) :
        CControlCommandResult( bResult ),
        m_aValue( aValue )
    {
    }

    ::com::sun::star::uno::Any SAL_CALL getValue( ) const
    {
        return m_aValue;
    }

private:
    ::com::sun::star::uno::Any m_aValue;
};

//---------------------------------------------
//
//---------------------------------------------

class CLabelCommandResult : public CControlCommandResult
{
public:
    CLabelCommandResult( sal_Bool bResult, const rtl::OUString& aLabel ) :
        CControlCommandResult( bResult ),
        m_aLabel( aLabel )
    {
    }

    rtl::OUString SAL_CALL getLabel( ) const
    {
        return m_aLabel;
    }

private:
    rtl::OUString m_aLabel;
};

#endif
