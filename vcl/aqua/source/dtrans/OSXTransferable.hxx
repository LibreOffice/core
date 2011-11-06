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




#ifndef _TRANSFERABLE_HXX_
#define _TRANSFERABLE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include "DataFlavorMapping.hxx"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <vector>


class OSXTransferable : public ::cppu::WeakImplHelper1<com::sun::star::datatransfer::XTransferable>,
                        private ::boost::noncopyable
{
public:
  typedef com::sun::star::uno::Sequence< sal_Int8 > ByteSequence_t;

  explicit OSXTransferable(com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> rXMimeCntFactory,
                           DataFlavorMapperPtr_t pDataFlavorMapper,
                           NSPasteboard* pasteboard);

  virtual ~OSXTransferable();

  //------------------------------------------------------------------------
  // XTransferable
  //------------------------------------------------------------------------

  virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
    throw( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
    throw( ::com::sun::star::uno::RuntimeException );

  virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
    throw( ::com::sun::star::uno::RuntimeException );

  //------------------------------------------------------------------------
  // Helper functions not part of the XTransferable interface
  //------------------------------------------------------------------------

  void initClipboardItemList();

  //com::sun::star::uno::Any getClipboardItemData(ClipboardItemPtr_t clipboardItem);

  bool isUnicodeText(const com::sun::star::datatransfer::DataFlavor& flavor);

  bool compareDataFlavors( const com::sun::star::datatransfer::DataFlavor& lhs,
                           const com::sun::star::datatransfer::DataFlavor& rhs );

  bool cmpAllContentTypeParameter( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType > xLhs,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType > xRhs ) const;

private:
  com::sun::star::uno::Sequence< com::sun::star::datatransfer::DataFlavor > mFlavorList;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  DataFlavorMapperPtr_t mDataFlavorMapper;
  NSPasteboard* mPasteboard;
};

#endif
