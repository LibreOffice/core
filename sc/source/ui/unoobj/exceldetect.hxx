/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_UNOOBJ_EXCELDETECT_HXX
#define INCLUDED_SC_SOURCE_UI_UNOOBJ_EXCELDETECT_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
}}}}

class ScExcelBiffDetect : public cppu::WeakImplHelper<com::sun::star::document::XExtendedFilterDetection, com::sun::star::lang::XServiceInfo>
{
public:
    ScExcelBiffDetect( const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext );
    virtual ~ScExcelBiffDetect();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& aName ) throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XExtendedFilterDetection
    virtual OUString SAL_CALL detect( com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& lDescriptor )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
