/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVRequestEnvironment.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:02:43 $
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
#ifndef _DAVREQUESTENVIRONMENT_HXX_
#define _DAVREQUESTENVIRONMENT_HXX_

#include <vector>

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _DAVAUTHLISTENER_HXX_
#include "DAVAuthListener.hxx"
#endif

namespace webdav_ucp
{
    typedef std::pair< rtl::OUString, rtl::OUString > DAVRequestHeader;
    typedef std::vector< DAVRequestHeader > DAVRequestHeaders;

struct DAVRequestEnvironment
{
    rtl::OUString m_aRequestURI;
    rtl::Reference< DAVAuthListener >     m_xAuthListener;
//    rtl::Reference< DAVStatusListener >   m_xStatusListener;
//    rtl::Reference< DAVProgressListener > m_xStatusListener;
    DAVRequestHeaders                     m_aRequestHeaders;
    uno::Reference< ucb::XCommandEnvironment > m_xEnv;

DAVRequestEnvironment( const rtl::OUString & rRequestURI,
                           const rtl::Reference< DAVAuthListener > & xListener,
                           const DAVRequestHeaders & rRequestHeaders,
                           const uno::Reference< ucb::XCommandEnvironment > & xEnv)
    : m_aRequestURI( rRequestURI ),
      m_xAuthListener( xListener ),
      m_aRequestHeaders( rRequestHeaders ),
      m_xEnv( xEnv ){}

    DAVRequestEnvironment() {}
};

} // namespace webdav_ucp

#endif // _DAVREQUESTENVIRONMENT_HXX_
