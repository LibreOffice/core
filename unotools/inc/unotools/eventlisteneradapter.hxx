/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventlisteneradapter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:52:14 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#define _UNOTOOLS_EVENTLISTENERADAPTER_HXX_

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

//.........................................................................
namespace utl
{
//.........................................................................

    struct OEventListenerAdapterImpl;
    //=====================================================================
    //= OEventListenerAdapter
    //=====================================================================
    /** base class for non-UNO dispose listeners
    */
    class UNOTOOLS_DLLPUBLIC OEventListenerAdapter
    {
        friend class OEventListenerImpl;

    private:
        UNOTOOLS_DLLPRIVATE OEventListenerAdapter( const OEventListenerAdapter& _rSource ); // never implemented
        UNOTOOLS_DLLPRIVATE const OEventListenerAdapter& operator=( const OEventListenerAdapter& _rSource );    // never implemented

    protected:
        OEventListenerAdapterImpl*  m_pImpl;

    protected:
                OEventListenerAdapter();
        virtual ~OEventListenerAdapter();

        void startComponentListening( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp );
        void stopComponentListening( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp );
        void stopAllComponentListening(  );

        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource ) = 0;
    };

//.........................................................................
}   // namespace utl
//.........................................................................

#endif // _UNOTOOLS_EVENTLISTENERADAPTER_HXX_

