/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: veto.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:15:28 $
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
#define DBACCESS_VETO_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XVETO_HPP_
#include <com/sun/star/util/XVeto.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= Veto
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::util::XVeto
                                    >   Veto_Base;
    /** implements ::com::sun::star::util::XVeto
    */
    class Veto : public Veto_Base
    {
    private:
        const ::rtl::OUString               m_sReason;
        const ::com::sun::star::uno::Any    m_aDetails;

    public:
        Veto( const ::rtl::OUString& _rReason, const ::com::sun::star::uno::Any& _rDetails );

        virtual ::rtl::OUString SAL_CALL getReason() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getDetails() throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~Veto();

    private:
        Veto();                         // never implemented
        Veto( const Veto& );            // never implemented
        Veto& operator=( const Veto& ); // never implemented
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_VETO_HXX

