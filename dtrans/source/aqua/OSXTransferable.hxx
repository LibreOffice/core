/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSXTransferable.hxx,v $
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

  explicit OSXTransferable(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& context,
                           ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> rXMimeCntFactory,
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
  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mXComponentContext;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  DataFlavorMapperPtr_t mDataFlavorMapper;
  NSPasteboard* mPasteboard;
};

#endif
