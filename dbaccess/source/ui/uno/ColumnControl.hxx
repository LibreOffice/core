/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnControl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:46:36 $
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
#ifndef DBAUI_COLUMNCONTROL_HXX
#define DBAUI_COLUMNCONTROL_HXX

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

namespace dbaui
{
    class OColumnControl :  public UnoControl
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
    public:
        OColumnControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

        // UnoControl
        virtual ::rtl::OUString GetComponentServiceName();

        // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

        // ::com::sun::star::awt::XControl
        virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException);
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_COLUMNCONTROL_HXX
