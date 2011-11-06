/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_FMDPAGE_HXX
#define _SVX_FMDPAGE_HXX

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <svx/unopage.hxx>
#include <comphelper/uno3.hxx>
#include "svx/svxdllapi.h"

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

