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


#ifndef _TRANSFERABLE_HXX_
#define _TRANSFERABLE_HXX_

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
