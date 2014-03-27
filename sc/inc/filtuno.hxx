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

#ifndef SC_FILTUNO_HXX
#define SC_FILTUNO_HXX

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase5.hxx>
#include "scdllapi.h"

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    ScFilterOptionsObj_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& );


class ScFilterOptionsObj : public ::cppu::WeakImplHelper5<
                            ::com::sun::star::beans::XPropertyAccess,
                            ::com::sun::star::ui::dialogs::XExecutableDialog,
                            ::com::sun::star::document::XImporter,
                            ::com::sun::star::document::XExporter,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    OUString     aFileName;
    OUString     aFilterName;
    OUString     aFilterOptions;
    css::uno::Reference< css::io::XInputStream > xInputStream;
    bool         bExport;

public:
                            ScFilterOptionsObj();
    virtual                 ~ScFilterOptionsObj();

    static OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_Static();

                            // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                            SAL_CALL getPropertyValues() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aProps )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                        ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XExecutableDialog
    virtual void SAL_CALL   setTitle( const OUString& aTitle )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL execute() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XImporter
    virtual void SAL_CALL   setTargetDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XComponent >& xDoc )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XExporter
    virtual void SAL_CALL   setSourceDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XComponent >& xDoc )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
