/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnPeer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:35:06 $
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
#ifndef DBAUI_COLUMNPEER_HXX
#define DBAUI_COLUMNPEER_HXX

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    class OFieldDescription;
    class OColumnPeer   :   public VCLXWindow
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
        OFieldDescription*                                                              m_pActFieldDescr;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xColumn;
    public:

        OColumnPeer(Window* _pParent
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

        void setColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn);
        void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xCon);
        void setEditWidth(sal_Int32 _nWidth);
        // VCLXWindow
        virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................
#endif //DBAUI_COLUMNPEER_HXX


