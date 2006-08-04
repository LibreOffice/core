/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: desktopcontext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-04 12:32:24 $
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

#ifndef _DESKTOP_DESKTOPCONTEXT_HXX_
#define _DESKTOP_DESKTOPCONTEXT_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

namespace desktop
{
    class DesktopContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
    {
    public:
    DesktopContext( const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > & ctx);

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
            com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_xNextContext;
    };
}

#endif // _DESKTOP_DESKTOPCONTEXT_HXX_
