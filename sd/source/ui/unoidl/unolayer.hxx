/*************************************************************************
 *
 *  $RCSfile: unolayer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-19 09:52:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNOLAYER_HXX
#define _UNOLAYER_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XLAYER_HPP_
#include <com/sun/star/drawing/XLayer.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XLAYERMANAGER_HPP_
#include <com/sun/star/drawing/XLayerManager.hpp>
#endif

#include <cppuhelper/implbase3.hxx>
#include <unotools/servicehelper.hxx>

#include <unomodel.hxx>

class SdrLayer;
class SdView;
class SdLayerManager;
class SdXImpressDocument;

enum LayerAttribute { VISIBLE, PRINTABLE, LOCKED };

/***********************************************************************
*                                                                      *
***********************************************************************/
class SdLayer : public ::cppu::WeakImplHelper3< ::com::sun::star::drawing::XLayer,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::lang::XUnoTunnel>
{
private:
    SdLayerManager*     pLayerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayerManager > mxLayerManager;

    SdrLayer*           pLayer;
    SvxItemPropertySet  aPropSet;

    sal_Bool get( LayerAttribute what ) throw();
    void set( LayerAttribute what, sal_Bool flag ) throw();

public:
    SdLayer( SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_ ) throw();
    virtual ~SdLayer() throw();

    // intern
    SdrLayer* GetSdrLayer() const throw() { return pLayer; }

    static String convertToInternalName( const ::rtl::OUString& rName );
    static ::rtl::OUString convertToExternalName( const String& rName );

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayer )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

#include <cppuhelper/implbase4.hxx>

class SdLayerManager : public ::cppu::WeakImplHelper4< ::com::sun::star::drawing::XLayerManager,
                                                       ::com::sun::star::container::XNameAccess,
                                                       ::com::sun::star::lang::XServiceInfo,
                                                       ::com::sun::star::lang::XUnoTunnel >
{
    friend class SdLayer;

private:
    SdXImpressDocument& rModel;

    SdView* GetView() const throw();
#ifndef SVX_LIGHT
    SdDrawDocShell* GetDocShell() const throw() { return rModel.pDocShell; }
#endif

    void UpdateLayerView( sal_Bool modify = sal_True ) const throw();

public:
    SdLayerManager( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdLayerManager() throw();

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayerManager )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XLayerManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attachShapeToLayer( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL getLayerForShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);
};

#endif


