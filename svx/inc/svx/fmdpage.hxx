/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmdpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:50:33 $
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
#ifndef _SVX_FMDPAGE_HXX
#define _SVX_FMDPAGE_HXX

#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER2_HPP_
#include <com/sun/star/form/XFormsSupplier2.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
#include <com/sun/star/form/XFormControllerListener.hpp>
#endif

#ifndef _SVX_UNOWPAGE_HXX
#include <svx/unopage.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//==================================================================
// SvxFmDrawPage
//==================================================================
class SVX_DLLPUBLIC SvxFmDrawPage   :public SvxDrawPage
                                    ,public ::com::sun::star::form::XFormsSupplier2
{
protected:

    // Erzeugen eines SdrObjects anhand einer Description. Kann von
    // abgeleiteten Klassen dazu benutzt werden, eigene ::com::sun::star::drawing::Shapes zu
    // unterstuetzen (z.B. Controls)
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )throw ();

    // Die folgende Methode wird gerufen, wenn ein SvxShape-Objekt angelegt
    // werden soll. abgeleitete Klassen koennen hier eine Ableitung oder
    // ein ein SvxShape aggregierendes Objekt anlegen.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();

public:
    SvxFmDrawPage( SdrPage* pPage );
    virtual ~SvxFmDrawPage() throw ();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(SvxFmDrawPage, SvxDrawPage);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // XFormsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getForms(void) throw( ::com::sun::star::uno::RuntimeException );

    // XFormsSupplier2
    virtual sal_Bool SAL_CALL hasForms(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};

#endif // _SVX_FMDPAGE_HXX

