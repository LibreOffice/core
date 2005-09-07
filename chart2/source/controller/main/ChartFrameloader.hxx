/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartFrameloader.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:33:37 $
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
#ifndef _CHARTFRAMELOADER_HXX
#define _CHARTFRAMELOADER_HXX

#include "ServiceMacros.hxx"

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSYNCHRONOUSFRAMELOADER_HPP_
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class ChartFrameLoader : public ::cppu::WeakImplHelper2<
         ::com::sun::star::frame::XSynchronousFrameLoader
         , ::com::sun::star::lang::XServiceInfo
            //comprehends XComponent (required interface)
    //  ,public ::com::sun::star::uno::XWeak            // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::uno::XInterface       // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::lang::XTypeProvider   // implemented by WeakImplHelper
        >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;
    sal_Bool            m_bCancelRequired;
    ::osl::Condition    m_oCancelFinished;

private:
        sal_Bool impl_checkCancel();
    //no default constructor
    ChartFrameLoader(){}
public:
    ChartFrameLoader(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartFrameLoader();

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XServiceInfo
    //-----------------------------------------------------------------

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartFrameLoader)

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XFrameLoader
    //-----------------------------------------------------------------

    virtual sal_Bool SAL_CALL
        load( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >& rMediaDescriptor
                ,const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XFrame >& xFrame )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        cancel()            throw (::com::sun::star::uno::RuntimeException);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
