/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataFlavorMapping.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_DATAFLAVORMAPPING_HXX_
#define INCLUDED_DATAFLAVORMAPPING_HXX_

#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <memory>
#include <boost/shared_ptr.hpp>


/* An interface to get the clipboard data in either
   system or OOo format.
 */
class DataProvider
{
public:
  virtual ~DataProvider() {};

  /* Get the clipboard data in the system format.
     The caller has to retain/release the returned
     CFDataRef on demand.
   */
  virtual NSData* getSystemData() = 0;

  /* Get the clipboard data in OOo format.
   */
  virtual com::sun::star::uno::Any getOOoData() = 0;
};

typedef std::auto_ptr<DataProvider> DataProviderPtr_t;


//################################


class DataFlavorMapper
{
public:
  /* Initialialize a DataFavorMapper instance. Throws a RuntimeException in case the XMimeContentTypeFactory service
     cannot be created.
   */
  DataFlavorMapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& context);


  /* Map a system data flavor to an OpenOffice data flavor.
     Return an empty string if there is not suiteable
     mapping from a system data flavor to a OpenOffice data
     flavor.
  */
  com::sun::star::datatransfer::DataFlavor systemToOpenOfficeFlavor(NSString* systemDataFlavor) const;


  /* Map an OpenOffice data flavor to a system data flavor.
     If there is no suiteable mapping available NULL will
     be returned.
  */
  NSString* openOfficeToSystemFlavor(const com::sun::star::datatransfer::DataFlavor& oooDataFlavor) const;


  /* Get a data provider which is able to provide the data 'rTransferable' offers in a format that can
     be put on to the system clipboard.
   */
  DataProviderPtr_t getDataProvider(NSString* systemFlavor,
                                    const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > rTransferable) const;



  /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
  DataProviderPtr_t getDataProvider(const NSString* systemFlavor, NSArray* systemData) const;


  /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
  DataProviderPtr_t getDataProvider(const NSString* systemFlavor, NSData* systemData) const;


  /* Translate a sequence of DataFlavors into a NSArray of system types.
     Only those DataFlavors for which a suitable mapping to a system
     type exist will be contained in the returned types array.
   */
  NSArray* flavorSequenceToTypesArray(const com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor>& flavors) const;

  /* Translate a NSArray of system types into a sequence of DataFlavors.
     Only those types for which a suitable mapping to a DataFlavor
     exist will be contained in the new DataFlavor Sequence.
  */
  com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor> typesArrayToFlavorSequence(NSArray* types) const;

  /* Returns an NSArray containing all pasteboard types supported by OOo
   */
  NSArray* DataFlavorMapper::getAllSupportedPboardTypes() const;

private:
  /* Determines if the provided Mime content type is valid.
   */
  bool isValidMimeContentType(const rtl::OUString& contentType) const;

private:
  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> mXComponentContext;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
};

typedef boost::shared_ptr<DataFlavorMapper> DataFlavorMapperPtr_t;

#endif
