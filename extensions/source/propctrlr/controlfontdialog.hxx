/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_

#include <svtools/genericunodialog.hxx>
#include "modulepcr.hxx"

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

namespace pcr
{



    //= OControlFontDialog

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
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >&);

        // XInitialization
        virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

         // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent) SAL_OVERRIDE;
        virtual void    destroyDialog() SAL_OVERRIDE;
        virtual void    executedDialog(sal_Int16 _nExecutionResult) SAL_OVERRIDE;
    };


}   // namespace pcr


#endif // _EXTENSIONS_PROPCTRLR_CONTROLFONTDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
