/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pageuno.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:47:40 $
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

#ifndef SC_PAGEUNO_HXX
#define SC_PAGEUNO_HXX

#ifndef _SVX_FMDPAGE_HXX
#include <svx/fmdpage.hxx>
#endif

//------------------------------------------------------------------------

//  SvxFmDrawPage subclass to create ScShapeObj for shapes

class ScPageObj : public SvxFmDrawPage
{
public:
                            ScPageObj( SdrPage* pPage );
        virtual                                 ~ScPageObj() throw();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                            _CreateShape( SdrObject *pObj ) const throw();

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& rServiceName )
        throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames()
        throw(com::sun::star::uno::RuntimeException);
};

#endif

