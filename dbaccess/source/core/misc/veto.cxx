/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: veto.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:17:17 $
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

#ifndef DBACCESS_VETO_HXX
#include "veto.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::RuntimeException;
    /** === end UNO using === **/

    //====================================================================
    //= Veto
    //====================================================================
    //--------------------------------------------------------------------
    Veto::Veto( const ::rtl::OUString& _rReason, const Any& _rDetails )
        :m_sReason( _rReason )
        ,m_aDetails( _rDetails )
    {
    }

    //--------------------------------------------------------------------
    Veto::~Veto()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Veto::getReason() throw (RuntimeException)
    {
        return m_sReason;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL Veto::getDetails() throw (RuntimeException)
    {
        return m_aDetails;
    }

//........................................................................
} // namespace dbaccess
//........................................................................

