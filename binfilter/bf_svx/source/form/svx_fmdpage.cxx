/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

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
#ifndef _FM_IMPLEMENTATION_IDS_HXX_
#include "fmimplids.hxx"
#endif
namespace binfilter {

/*N*/ DBG_NAME(SvxFmDrawPage)
/*N*/ SvxFmDrawPage::SvxFmDrawPage( SdrPage* pInPage ) :
/*N*/ 	SvxDrawPage( pInPage )
/*N*/ {
/*N*/ 	DBG_CTOR(SvxFmDrawPage,NULL);
    /*N*/ 	m_pHoldImplIdHelper = new ::binfilter::form::OImplementationIdsRef(); //STRIP008 m_pHoldImplIdHelper = new ::form::OImplementationIdsRef();
/*N*/ }

/*N*/ SvxFmDrawPage::~SvxFmDrawPage() throw ()
/*N*/ {
/*N*/ 	delete m_pHoldImplIdHelper;
/*N*/ 	DBG_DTOR(SvxFmDrawPage,NULL);
/*N*/ }

/*N*/ ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL SvxFmDrawPage::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); ::com::sun::star::uno::Sequence< sal_Int8 > a; return a; //STRIP001 
/*N*/ }

/*N*/ ::com::sun::star::uno::Any SAL_CALL SvxFmDrawPage::queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	::com::sun::star::uno::Any aRet = ::cppu::queryInterface(aType,
/*N*/ 		static_cast< ::com::sun::star::form::XFormsSupplier*>(this));
/*N*/ 	if(aRet.hasValue())
/*N*/ 		return aRet;
/*N*/ 
/*N*/ 	return SvxDrawPage::queryAggregation(aType);
/*N*/ }

/***********************************************************************
*                                                                      *
***********************************************************************/

/*N*/ SdrObject *SvxFmDrawPage::_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xDescr ) throw ()
/*N*/ {
/*N*/ 	::rtl::OUString aShapeType( xDescr->getShapeType() );
/*N*/ 
/*N*/ 	if (aShapeType  ==  ::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeControl"))
/*?*/ 		return new FmFormObj(OBJ_FM_CONTROL);
/*N*/ 	else
/*N*/ 		return SvxDrawPage::_CreateSdrObject( xDescr );
/*N*/ 
/*N*/ }

/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  SvxFmDrawPage::_CreateShape( SdrObject *pObj ) const throw ()
/*N*/ {
/*N*/ 	if( FmFormInventor == pObj->GetObjInventor() )
/*N*/ 	{
/*N*/ 		::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xShape = (SvxShape*)new SvxShapeControl( pObj );
/*N*/ 		return xShape;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return SvxDrawPage::_CreateShape( pObj );
/*N*/ }

// ::com::sun::star::form::XFormsSupplier
/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL SvxFmDrawPage::getForms(void) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  xForms;
/*N*/ 
/*N*/ 	FmFormPage *pFmPage = PTR_CAST( FmFormPage, GetSdrPage() );
/*N*/ 	if( pFmPage )
/*N*/ 		xForms = pFmPage->GetForms();
/*N*/ 
/*N*/ 	return xForms;
/*N*/ }

// ::com::sun::star::lang::XServiceInfo
/*N*/ ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvxFmDrawPage::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
    /* TODO: DG? Irgendwelche Services?
    ::com::sun::star::uno::Sequence aSeq( SvxDrawPage::getSupportedServiceNames() );
    addToSequence( aSeq, 1, ::rtl::OUString::createFromAscii("stardiv.form.superservice?") );
    return aSeq;
    */
/*N*/ 
/*N*/ 	return SvxDrawPage::getSupportedServiceNames();
/*N*/ }


}
