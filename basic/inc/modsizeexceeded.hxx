/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modsizeexceeded.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 15:04:09 $
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

#ifndef _BASIC_MODSIZEEXCEEDED_HXX
#define _BASIC_MODSIZEEXCEEDED_HXX

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <cppuhelper/implbase1.hxx>

class ModuleSizeExceeded : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // c++ interface
    public:
    ModuleSizeExceeded( const com::sun::star::uno::Sequence< ::rtl::OUString>& sModules );

    sal_Bool isAbort() const;
    sal_Bool isApprove() const;

    // uno interface
    public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException ) { return m_lContinuations; }
    com::sun::star::uno::Any SAL_CALL getRequest() throw( com::sun::star::uno::RuntimeException )
    {
        return m_aRequest;
    }
    // member
    private:
    rtl::OUString m_sMods;
    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > m_lContinuations;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > m_xAbort;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation> m_xApprove;
};

#endif

