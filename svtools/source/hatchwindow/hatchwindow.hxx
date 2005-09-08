/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatchwindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:50:19 $
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

#ifndef _SVT_HATCHWINDOW_HXX
#define _SVT_HATCHWINDOW_HXX

#ifndef _COM_SUN_STAR_EMBED_XHATCHWINDOW_HPP_
#include <com/sun/star/embed/XHatchWindow.hpp>
#endif

#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/typeprovider.hxx>


class VCLXHatchWindow : public ::com::sun::star::embed::XHatchWindow,
                        public VCLXWindow
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XHatchWindowController > m_xController;

public:
    VCLXHatchWindow();
    ~VCLXHatchWindow();

    void initializeWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
                const ::com::sun::star::awt::Rectangle& aBounds,
                const ::com::sun::star::awt::Size& aSize );

    void QueryObjAreaPixel( Rectangle & );
    void RequestObjAreaPixel( const Rectangle & );
    void InplaceDeactivate();

    // XInterface
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL acquire() throw();
    void SAL_CALL release() throw();

    // XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XHatchWindow
    virtual void SAL_CALL setController( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XHatchWindowController >& xController ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // _SVT_HATCHWINDOW_HXX

