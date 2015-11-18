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

#ifndef INCLUDED_VCL_OSX_OSXTRANSFERABLE_HXX
#define INCLUDED_VCL_OSX_OSXTRANSFERABLE_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include "DataFlavorMapping.hxx"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>

class OSXTransferable : public ::cppu::WeakImplHelper<css::datatransfer::XTransferable>,
                        private ::boost::noncopyable
{
public:
  explicit OSXTransferable(css::uno::Reference< css::datatransfer::XMimeContentTypeFactory> rXMimeCntFactory,
                           DataFlavorMapperPtr_t pDataFlavorMapper,
                           NSPasteboard* pasteboard);

  virtual ~OSXTransferable();

  // XTransferable

  virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor )
    throw( css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception ) override;

  virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
    throw( css::uno::RuntimeException, std::exception ) override;

  virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor )
    throw( css::uno::RuntimeException, std::exception ) override;

  // Helper functions not part of the XTransferable interface

  void initClipboardItemList();

  //css::uno::Any getClipboardItemData(ClipboardItemPtr_t clipboardItem);

  bool compareDataFlavors( const css::datatransfer::DataFlavor& lhs,
                           const css::datatransfer::DataFlavor& rhs );

private:
  css::uno::Sequence< css::datatransfer::DataFlavor > mFlavorList;
  css::uno::Reference< css::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  DataFlavorMapperPtr_t mDataFlavorMapper;
  NSPasteboard* mPasteboard;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
