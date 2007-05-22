/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModifyListenerCallBack.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:19:19 $
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

#ifndef CHART2_MODIFYLISTENERCALLBACK_HXX
#define CHART2_MODIFYLISTENERCALLBACK_HXX

// header for class Link
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

namespace chart {

/** Use this class as a member if you want to listen on a XModifyBroadcaster
without becoming a XModifyListener yourself
 */

class ModifyListenerCallBack_impl;

class ModifyListenerCallBack
{
public:
    explicit ModifyListenerCallBack( const Link& rCallBack );

    virtual ~ModifyListenerCallBack();

    void startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster );
    void stopListening();

private: //methods
    ModifyListenerCallBack();
    ModifyListenerCallBack( const ModifyListenerCallBack& );

private: //member
    ModifyListenerCallBack_impl* pModifyListener_impl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   m_xModifyListener;
};

} // namespace chart

#endif

