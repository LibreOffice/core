/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoobj.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:27:31 $
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
#ifndef _UNOOBJ_HXX
#define _UNOOBJ_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _SVDPOOL_HXX //autogen
#include <svx/svdpool.hxx>
#endif

#ifndef _SVX_UNOMASTER_HXX
#include <svx/unomaster.hxx>
#endif

#include <svx/unoipset.hxx>

#include <cppuhelper/implbase2.hxx>

class SdrObject;
class SdXImpressDocument;
class SdAnimationInfo;

class SdXShape : public SvxShapeMaster,
                 public ::com::sun::star::document::XEventsSupplier
{
    friend class SdUnoEventsAccess;

private:
    SvxShape* mpShape;
    SvxItemPropertySet  maPropSet;
    const SfxItemPropertyMap* mpMap;
    SdXImpressDocument* mpModel;

    void SetStyleSheet( const ::com::sun::star::uno::Any& rAny ) throw( ::com::sun::star::lang::IllegalArgumentException );
    ::com::sun::star::uno::Any GetStyleSheet() const throw( ::com::sun::star::beans::UnknownPropertyException  );

    // Intern
    SdAnimationInfo* GetAnimationInfo( sal_Bool bCreate = sal_False ) const throw();
    sal_Bool IsPresObj() const throw();
    void SetPresObj( sal_Bool bPresObj ) throw();

    sal_Bool IsEmptyPresObj() const throw();
    void SetEmptyPresObj( sal_Bool bEmpty ) throw();

    sal_Bool IsMasterDepend() const throw();
    void SetMasterDepend( sal_Bool bDepend ) throw();

    SdrObject* GetSdrObject() const throw();

    com::sun::star::uno::Sequence< sal_Int8 >* mpImplementationId;

public:
    SdXShape() throw();
    SdXShape(SvxShape* pShape, SdXImpressDocument* pModel) throw();
    virtual ~SdXShape() throw();

    virtual sal_Bool queryAggregation( const com::sun::star::uno::Type & rType, com::sun::star::uno::Any& aAny );
    virtual void dispose();
    virtual void modelChanged( SdrModel* pNewModel );
    virtual void pageChanged( SdrPage* pNewPage );
    virtual void objectChanged( SdrObject* pNewObj );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

private:
    void setOldEffect( const com::sun::star::uno::Any& aValue );
    void setOldTextEffect( const com::sun::star::uno::Any& aValue );
    void setOldSpeed( const com::sun::star::uno::Any& aValue );
    void setOldDimColor( const com::sun::star::uno::Any& aValue );
    void setOldDimHide( const com::sun::star::uno::Any& aValue );
    void setOldDimPrevious( const com::sun::star::uno::Any& aValue );
    void setOldPresOrder( const com::sun::star::uno::Any& aValue );
    void updateOldSoundEffect( SdAnimationInfo* pInfo );

    void getOldEffect( com::sun::star::uno::Any& rValue ) const;
    void getOldTextEffect( com::sun::star::uno::Any& rValue ) const;
    void getOldSpeed( com::sun::star::uno::Any& rValue ) const;
    void getOldSoundFile( com::sun::star::uno::Any& rValue ) const;
    void getOldSoundOn( com::sun::star::uno::Any& rValue ) const;
    void getOldDimColor( com::sun::star::uno::Any& rValue ) const;
    void getOldDimHide( com::sun::star::uno::Any& rValue ) const;
    void getOldDimPrev( com::sun::star::uno::Any& rValue ) const;
    void getOldPresOrder( com::sun::star::uno::Any& rValue ) const;
};

struct SvEventDescription;
const SvEventDescription* ImplGetSupportedMacroItems();

#endif


