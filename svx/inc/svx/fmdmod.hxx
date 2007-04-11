/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmdmod.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:50:20 $
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
#ifndef _SVX_FMDMOD_HXX
#define _SVX_FMDMOD_HXX

#ifndef SVX_UNOMOD_HXX
#include <svx/unomod.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SVX_DLLPUBLIC SvxFmMSFactory : public SvxUnoDrawMSFactory
{

public:
    SvxFmMSFactory() {};

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(const ::rtl::OUString& ServiceSpecifier) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};

#endif // _SVX_FMDMOD_HXX

