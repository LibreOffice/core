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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/fmpage.hxx>
#include "fmobj.hxx"
#include <svx/fmglob.hxx>
#include <svx/fmdpage.hxx>
#include <svx/unoshape.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::form::XFormsSupplier2;

SvxFmDrawPage::SvxFmDrawPage( SdrPage* pInPage ) :
    SvxDrawPage( pInPage )
{
}

SvxFmDrawPage::~SvxFmDrawPage() throw ()
{
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

    if  (GetSdrPage()
            && (   aShapeType.equalsAscii( "com.sun.star.drawing.ShapeControl" )   // compatibility
        ||  aShapeType.equalsAscii( "com.sun.star.drawing.ControlShape" )
        ))
    {
        return new FmFormObj( GetSdrPage()->getSdrModelFromSdrPage(), ::rtl::OUString(), OBJ_FM_CONTROL );
    }
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

    FmFormPage *pFmPage = dynamic_cast< FmFormPage* >( GetSdrPage() );
    if( pFmPage )
        xForms = pFmPage->GetForms();

    return xForms;
}

// XFormsSupplier2
sal_Bool SAL_CALL SvxFmDrawPage::hasForms(void) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bHas = sal_False;
    FmFormPage* pFormPage = dynamic_cast< FmFormPage* >( GetSdrPage() );
    if ( pFormPage )
        bHas = pFormPage->GetForms( false ).is();
    return bHas;
}

// ::com::sun::star::lang::XServiceInfo
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvxFmDrawPage::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return SvxDrawPage::getSupportedServiceNames();
}


