/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxtopwindow.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 12:55:06 $
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

#ifndef _TOOLKIT_AWT_VCLXTOPWINDOW_HXX_
#define _TOOLKIT_AWT_VCLXTOPWINDOW_HXX_

#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTWINDOWPEER_HPP_
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENUBAR_HPP_
#include <com/sun/star/awt/XMenuBar.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <toolkit/awt/vclxcontainer.hxx>

//  ----------------------------------------------------
//  class VCLXTopWindow
//  ----------------------------------------------------

class VCLXTopWindow :   public ::com::sun::star::awt::XTopWindow,
            public ::com::sun::star::awt::XSystemDependentWindowPeer,
                        public VCLXContainer
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar> mxMenuBar;
    bool m_bWHWND;
public:
    VCLXTopWindow(bool bWHWND = false);
    ~VCLXTopWindow();

    // ::com::sun::star::awt::XSystemDependendtWindowPeer
    ::com::sun::star::uno::Any SAL_CALL getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw(::com::sun::star::uno::RuntimeException);

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }
};




#endif // _TOOLKIT_AWT_VCLXTOPWINDOW_HXX_

