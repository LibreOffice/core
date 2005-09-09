/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxregion.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:45:42 $
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

#ifndef _TOOLKIT_AWT_VCLXREGION_HXX_
#define _TOOLKIT_AWT_VCLXREGION_HXX_


#ifndef _COM_SUN_STAR_AWT_XREGION_HPP_
#include <com/sun/star/awt/XRegion.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <vcl/region.hxx>

//  ----------------------------------------------------
//  class VCLXRegion
//  ----------------------------------------------------

class VCLXRegion :  public ::com::sun::star::awt::XRegion,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex    maMutex;
    Region          maRegion;

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }

public:
                    VCLXRegion();
                    ~VCLXRegion();

    void            SetRegion( const Region& rRegion )  { maRegion = rRegion; }
    const Region&   GetRegion() const                   { return maRegion; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXRegion*                                          GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XRegion
     ::com::sun::star::awt::Rectangle       SAL_CALL getBounds() throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL clear() throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL unionRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL intersectRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL excludeRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL xOrRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL unionRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL intersectRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL excludeRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException);
     void                                   SAL_CALL xOrRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException);
     ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > SAL_CALL getRectangles() throw(::com::sun::star::uno::RuntimeException);

};



#endif // _TOOLKIT_AWT_VCLXREGION_HXX_

