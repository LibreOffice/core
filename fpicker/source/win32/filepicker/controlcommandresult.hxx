/*************************************************************************
 *
 *  $RCSfile: controlcommandresult.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:54:08 $
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

#ifndef _CONTROLCOMMANDRESULT_HXX_
#define _CONTROLCOMMANDRESULT_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
