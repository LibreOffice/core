/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlfontdialog.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:09:38 $
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

#ifndef _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef EXTENSIONS_PROPCTRLR_MODULEPRC_HXX
#include "modulepcr.hxx"
#endif

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;
//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= OControlFontDialog
    //====================================================================
    class OControlFontDialog;
    typedef ::svt::OGenericUnoDialog                                        OControlFontDialog_DBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OControlFontDialog >   OControlFontDialog_PBase;

    class OControlFontDialog
                :public OControlFontDialog_DBase
                ,public OControlFontDialog_PBase
                ,public PcrClient
    {
    protected:
        // <properties>
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                m_xControlModel;
        // </properties>

        SfxItemSet*             m_pFontItems;           // item set for the dialog
        SfxItemPool*            m_pItemPool;            // item pool for the item set for the dialog
        SfxPoolItem**           m_pItemPoolDefaults;    // pool defaults

    public:
        OControlFontDialog(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& _rxContext);
        ~OControlFontDialog();

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >&);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void    destroyDialog();
        virtual void    executedDialog(sal_Int16 _nExecutionResult);
    };

//........................................................................
}   // namespace pcr
//........................................................................

#endif // _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_

