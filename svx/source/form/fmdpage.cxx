/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmdpage.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:45:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif

#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SVX_FMDPAGE_HXX
#include "fmdpage.hxx"
#endif

#ifndef _SVX_UNOSHAPE_HXX //autogen
#include "unoshape.hxx"
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::form::XFormsSupplier2;

DBG_NAME(SvxFmDrawPage)
SvxFmDrawPage::SvxFmDrawPage( SdrPage* pInPage ) :
    SvxDrawPage( pInPage )
{
    DBG_CTOR(SvxFmDrawPage,NULL);
}

SvxFmDrawPage::~SvxFmDrawPage() throw ()
{
    DBG_DTOR(SvxFmDrawPage,NULL);
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL SvxFmDrawPage::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OImplementationId* pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

Any SAL_CALL SvxFmDrawPage::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface   (   _rType
                                        ,   static_cast< XFormsSupplier2* >( this )
                                        ,   static_cast< XFormsSupplier* >( this )
                                        );
    if ( !aRet.hasValue() )
        aRet = SvxDrawPage::queryAggregation( _rType );

    return aRet;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SvxFmDrawPage::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes(SvxDrawPage::getTypes());
    aTypes.realloc(aTypes.getLength() + 1);
    ::com::sun::star::uno::Type* pTypes = aTypes.getArray();

    pTypes[aTypes.getLength()-1] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormsSupplier>*)0);
    return aTypes;
}

SdrObject *SvxFmDrawPage::_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xDescr ) throw ()
{
    ::rtl::OUString aShapeType( xDescr->getShapeType() );

    if (aShapeType  ==  ::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeControl"))
        return new FmFormObj(OBJ_FM_CONTROL);
    else
        return SvxDrawPage::_CreateSdrObject( xDescr );

}

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  SvxFmDrawPage::_CreateShape( SdrObject *pObj ) const throw ()
{
    if( FmFormInventor == pObj->GetObjInventor() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xShape = (SvxShape*)new SvxShapeControl( pObj );
        return xShape;
    }
    else
        return SvxDrawPage::_CreateShape( pObj );
}

// XFormsSupplier
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL SvxFmDrawPage::getForms(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  xForms;

    FmFormPage *pFmPage = PTR_CAST( FmFormPage, GetSdrPage() );
    if( pFmPage )
        xForms = pFmPage->GetForms();

    return xForms;
}

// XFormsSupplier2
sal_Bool SAL_CALL SvxFmDrawPage::hasForms(void) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bHas = sal_False;
    FmFormPage* pFormPage = PTR_CAST( FmFormPage, GetSdrPage() );
    if ( pFormPage )
        bHas = pFormPage->GetForms( false ).is();
    return bHas;
}

// ::com::sun::star::lang::XServiceInfo
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvxFmDrawPage::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return SvxDrawPage::getSupportedServiceNames();
}


