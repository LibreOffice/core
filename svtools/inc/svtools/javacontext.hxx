/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javacontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:25:47 $
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

#ifndef _SVTOOLS_JAVACONTEXT_HXX_
#define _SVTOOLS_JAVACONTEXT_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>


namespace svt
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.
    class SVT_DLLPUBLIC JavaContext :
        public com::sun::star::uno::XCurrentContext
    {

    public:
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx);

        /** The parameter bShowErrorsOnce controls whether a message box is
            only displayed once for a reocurring Java error. That is only
            the first time JavaInteractionHandler.handle is called with a
            particular Request then the message box is shown. Afterwards
            nothing happens.
         */
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx,
                     bool bReportErrorOnce);
        virtual ~JavaContext();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL acquire() throw ();

        virtual void SAL_CALL release() throw ();

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
        SVT_DLLPRIVATE JavaContext(); //not implemented
        SVT_DLLPRIVATE JavaContext(JavaContext&); //not implemented
        SVT_DLLPRIVATE JavaContext& operator = (JavaContext&); //not implemented

        oslInterlockedCount m_aRefCount;

        com::sun::star::uno::Reference<
            com::sun::star::uno::XCurrentContext > m_xNextContext;
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionHandler> m_xHandler;
        bool m_bShowErrorsOnce;
    };
}

#endif
