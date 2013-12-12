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



#ifndef INCLUDED_DATAFLAVORMAPPING_HXX_
#define INCLUDED_DATAFLAVORMAPPING_HXX_

#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <hash_map>
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
  DataFlavorMapper();
  ~DataFlavorMapper();


  /* Map a system data flavor to an OpenOffice data flavor.
     Return an empty string if there is not suiteable
     mapping from a system data flavor to a OpenOffice data
     flavor.
  */
  com::sun::star::datatransfer::DataFlavor systemToOpenOfficeFlavor( const NSString* systemDataFlavor) const;


  /* Map an OpenOffice data flavor to a system data flavor.
     If there is no suiteable mapping available NULL will
     be returned.
  */
  const NSString* openOfficeToSystemFlavor(const com::sun::star::datatransfer::DataFlavor& oooDataFlavor, bool& rbInternal) const;

  /* Select the best available image data type
     If there is no suiteable mapping available NULL will
     be returned.
  */
  NSString* openOfficeImageToSystemFlavor(NSPasteboard* pPasteboard) const;

  /* Get a data provider which is able to provide the data 'rTransferable' offers in a format that can
     be put on to the system clipboard.
   */
  DataProviderPtr_t getDataProvider( const NSString* systemFlavor,
                                    const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > rTransferable) const;



  /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
  DataProviderPtr_t getDataProvider( const NSString* systemFlavor, NSArray* systemData) const;


  /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
  DataProviderPtr_t getDataProvider( const NSString* systemFlavor, NSData* systemData) const;


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
  NSArray* getAllSupportedPboardTypes() const;

private:
  /* Determines if the provided Mime content type is valid.
   */
  bool isValidMimeContentType(const rtl::OUString& contentType) const;

private:
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  typedef std::hash_map< rtl::OUString, NSString*, rtl::OUStringHash > OfficeOnlyTypes;
  mutable OfficeOnlyTypes maOfficeOnlyTypes;
};

typedef boost::shared_ptr<DataFlavorMapper> DataFlavorMapperPtr_t;

#endif

