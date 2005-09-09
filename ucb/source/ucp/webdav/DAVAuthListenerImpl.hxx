/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVAuthListenerImpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:05:42 $
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

#ifndef _DAVAUTHLISTENERIMPL_HXX_
#define _DAVAUTHLISTENERIMPL_HXX_

#ifndef _DAVAUTHLISTENER_HXX_
#include "DAVAuthListener.hxx"
#endif


namespace webdav_ucp
{

//=========================================================================

//=========================================================================
//=========================================================================
//
// class DAVAuthListenerImpl.
//
//=========================================================================
//=========================================================================


    class DAVAuthListener_Impl : public DAVAuthListener
    {
    public:

        DAVAuthListener_Impl(
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment>& xEnv )
            : m_xEnv( xEnv
            )
        {
        }

        virtual int authenticate( const ::rtl::OUString & inRealm,
                                  const ::rtl::OUString & inHostName,
                                  ::rtl::OUString & inoutUserName,
                                  ::rtl::OUString & outPassWord );
    private:

        com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > m_xEnv;
        rtl::OUString m_aPrevPassword;
        rtl::OUString m_aPrevUsername;
    };

}

#endif
