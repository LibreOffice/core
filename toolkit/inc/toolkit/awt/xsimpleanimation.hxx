/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsimpleanimation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 13:50:57 $
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
#ifndef TOOLKIT_AWT_XSIMPLEANIMATION_HXX
#define TOOLKIT_AWT_XSIMPLEANIMATION_HXX

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_
#include <toolkit/helper/listenermultiplexer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XSIMPLEANIMATION_HPP_
#include <com/sun/star/awt/XSimpleAnimation.hpp>
#endif

//........................................................................
namespace toolkit
{
    class Throbber_Impl;
//........................................................................

    //====================================================================
    //= XSimpleAnimation
    //====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSimpleAnimation
                                >   XSimpleAnimation_Base;

    class XSimpleAnimation :public VCLXWindow
                           ,public XSimpleAnimation_Base
    {
    private:
        //::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > maImageList;
        sal_Bool    mbRepeat;
        sal_Int32   mnStepTime;

        Throbber_Impl   *mpThrobber;

    public:
        XSimpleAnimation();

    protected:
        ~XSimpleAnimation();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XSimpleAnimation
        virtual void SAL_CALL start() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stop() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList )
                                            throw (::com::sun::star::uno::RuntimeException);
        // VclWindowPeer
        virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

        // VCLXWindow
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

    private:
        XSimpleAnimation( const XSimpleAnimation& );            // never implemented
        XSimpleAnimation& operator=( const XSimpleAnimation& ); // never implemented
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_AWT_XSIMPLEANIMATION_HXX

