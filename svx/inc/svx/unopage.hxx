/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:45:28 $
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
#ifndef _SVX_UNOWPAGE_HXX
#define _SVX_UNOWPAGE_HXX

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUPER_HPP_
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPECOMBINER_HPP_
#include <com/sun/star/drawing/XShapeCombiner.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEBINDER_HPP_
#include <com/sun/star/drawing/XShapeBinder.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _SVX_MUTXHELP_HXX
#include <svx/mutxhelp.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <cppuhelper/implbase5.hxx>
#include <unotools/servicehelper.hxx>

#include <svx/unoprov.hxx>

class SdrPage;
class SdrModel;
class SdrView;
class SdrPageView;
class SdrObject;
class List;
class SvxShapeDescriptor;
class SvxShape;
class SvxShapeGroup;
class SvxShapeConnector;
class SvxShapeList;
class SvxDrawPageList;

/***********************************************************************
* Macros fuer Umrechnung Twips<->100tel mm                             *
***********************************************************************/
#define TWIPS_TO_MM(val) ((val * 127 + 36) / 72)
#define MM_TO_TWIPS(val) ((val * 72 + 63) / 127)

/***********************************************************************
*                                                                      *
***********************************************************************/
class SVX_DLLPUBLIC SvxDrawPage : public ::cppu::WeakAggImplHelper5< ::com::sun::star::drawing::XDrawPage,
                                               ::com::sun::star::drawing::XShapeGrouper,
                                               ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::lang::XUnoTunnel,
                                               ::com::sun::star::lang::XComponent>,
                    public SfxListener,
                    protected SvxMutexHelper
{
 protected:
    cppu::OBroadcastHelper mrBHelper;

    SdrPage*        mpPage;
    SdrModel*       mpModel;
    SdrView*        mpView;

    void    _SelectObjectsInView( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& aShapes, SdrPageView*   pPageView ) throw ();
    void    _SelectObjectInView( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, SdrPageView*  pPageView ) throw();

    virtual void disposing() throw();

 public:
    SvxDrawPage( SdrPage* pPage ) throw();
    SvxDrawPage() throw();
    virtual ~SvxDrawPage() throw();

    // Internals
    SdrPage* GetSdrPage() const { return mpPage; }
    void ChangeModel( SdrModel* pNewModel );

    // Erzeugen eines SdrObjects und Einfugen in die SdrPage
    SdrObject *CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    // Typ und Inventor bestimmen
    void GetTypeAndInventor( sal_uInt16& rType, sal_uInt32& rInventor, const ::rtl::OUString& aName ) const throw();

    // Erzeugen eines SdrObjects anhand einer Description. Kann von
    // abgeleiteten Klassen dazu benutzt werden, eigene Shapes zu
    // unterstuetzen (z.B. Controls)
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    static SvxShape* CreateShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor, SdrObject *pObj = NULL, SvxDrawPage *pPage = NULL ) throw();

    // Die folgende Methode wird gerufen, wenn ein SvxShape-Objekt angelegt
    // werden soll. abgeleitete Klassen koennen hier eine Ableitung oder
    // ein ein SvxShape aggregierenden Objekt anlegen.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > _CreateShape( SdrObject *pObj ) const throw();

    static SvxDrawPage* GetPageForSdrPage( SdrPage* pPage ) throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxDrawPage )

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XInterface
    virtual void SAL_CALL release() throw();

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XShapeGrouper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup > SAL_CALL group( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ungroup( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup >& aGroup ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};

#endif

