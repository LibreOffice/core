/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PLACEWARE_EXPORTER_HXX
#define _PLACEWARE_EXPORTER_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

class PageEntry;

class PlaceWareExporter
{
public:
    PlaceWareExporter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~PlaceWareExporter();

    sal_Bool doExport( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDoc,
                        ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOutputStream,
                            const rtl::OUString& rURL,
                                ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xHandler,
                                ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator >& rxStatusIndicator );

private:
    PageEntry* exportPage( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >&xDrawPage );

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > mxGraphicExporter;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > mxInteractionHandler;
};

#endif
