/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#ifndef INCLUDED_VCL_IOS_DATAFLAVORMAPPING_HXX
#define INCLUDED_VCL_IOS_DATAFLAVORMAPPING_HXX

#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#include <memory>
#include <unordered_map>

/* An interface to get the clipboard data in either
   system or OOo format.
 */
class DataProvider
{
public:
    virtual ~DataProvider(){};

    /* Get the clipboard data in the system format.
     The caller has to retain/release the returned
     CFDataRef on demand.
   */
    virtual NSData* getSystemData() = 0;

    /* Get the clipboard data in OOo format.
   */
    virtual css::uno::Any getOOoData() = 0;
};

typedef std::unique_ptr<DataProvider> DataProviderPtr_t;

class DataFlavorMapper
{
public:
    /* Initialize a DataFavorMapper instance. Throws a RuntimeException in case the XMimeContentTypeFactory service
     cannot be created.
   */
    DataFlavorMapper();
    ~DataFlavorMapper();

    /* Map a system data flavor to an OpenOffice data flavor.
     Return an empty string if there is not suitable
     mapping from a system data flavor to an OpenOffice data
     flavor.
  */
    css::datatransfer::DataFlavor systemToOpenOfficeFlavor(const NSString* systemDataFlavor) const;

    /* Map an OpenOffice data flavor to a system data flavor.
     If there is no suitable mapping available NULL will
     be returned.
  */
    NSString* openOfficeToSystemFlavor(const css::datatransfer::DataFlavor& oooDataFlavor,
                                       bool& rbInternal) const;

    /* Select the best available image data type
     If there is no suitable mapping available NULL will
     be returned.
  */
    static NSString* openOfficeImageToSystemFlavor();

    /* Get a data provider which is able to provide the data 'rTransferable' offers in a format that can
     be put on to the system clipboard.
   */
    DataProviderPtr_t getDataProvider(
        const NSString* systemFlavor,
        const css::uno::Reference<css::datatransfer::XTransferable>& rTransferable) const;

    /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
    static DataProviderPtr_t getDataProvider(const NSString* systemFlavor, NSArray* systemData);

    /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
    static DataProviderPtr_t getDataProvider(const NSString* systemFlavor, NSData* systemData);

    /* Translate a sequence of DataFlavors into a NSArray of system types.
     Only those DataFlavors for which a suitable mapping to a system
     type exist will be contained in the returned types array.
   */
    NSArray* flavorSequenceToTypesArray(
        const css::uno::Sequence<css::datatransfer::DataFlavor>& flavors) const;

    /* Translate a NSArray of system types into a sequence of DataFlavors.
     Only those types for which a suitable mapping to a DataFlavor
     exist will be contained in the new DataFlavor Sequence.
  */
    css::uno::Sequence<css::datatransfer::DataFlavor>
    typesArrayToFlavorSequence(NSArray* types) const;

private:
    /* Determines if the provided Mime content type is valid.
   */
    bool isValidMimeContentType(const OUString& contentType) const;

private:
    css::uno::Reference<css::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
    typedef std::unordered_map<OUString, NSString*> OfficeOnlyTypes;
    mutable OfficeOnlyTypes maOfficeOnlyTypes;
};

#endif // INCLUDED_VCL_IOS_DATAFLAVORMAPPING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
