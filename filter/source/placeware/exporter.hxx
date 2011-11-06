/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
