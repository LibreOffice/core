/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbexchange.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:47:57 $
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
#ifndef DBAUI_DBEXCHANGE_HXX
#define DBAUI_DBEXCHANGE_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include <vector>

namespace dbaui
{

    class ORTFImportExport;
    class OHTMLImportExport;
    typedef ::cppu::ImplHelper1< ::com::sun::star::lang::XEventListener > TDataClipboard_BASE;

    class ODataClipboard :      public ::svx::ODataAccessObjectTransferable
                            ,   public TDataClipboard_BASE

    {
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener> > m_aEventListeners;
        OHTMLImportExport*      m_pHtml;
        ORTFImportExport*       m_pRtf;

    public:
        ODataClipboard(
            const ::rtl::OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const ::rtl::OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        ODataClipboard(
            const ::rtl::OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const ::rtl::OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /** with this ctor, only the object descriptor format will be provided
        */
        ODataClipboard(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxLivingForm,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rSelectedRows,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxResultSet
        );

        DECLARE_XINTERFACE( )

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
        virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    };
}

#endif //  DBAUI_DBEXCHANGE_HXX







