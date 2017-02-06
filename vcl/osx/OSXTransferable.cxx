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

#include <sal/config.h>

#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sal/types.h>
#include <osl/diagnose.h>

#include "OSXTransferable.hxx"

#include "DataFlavorMapping.hxx"

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;

namespace
{
    bool isValidFlavor( const DataFlavor& aFlavor )
    {
      size_t len = aFlavor.MimeType.getLength();
      Type dtype = aFlavor.DataType;
      return ((len > 0) && ((dtype == cppu::UnoType<Sequence<sal_Int8>>::get()) || (dtype == cppu::UnoType<OUString>::get())));
    }

bool cmpAllContentTypeParameter(const Reference<XMimeContentType> & xLhs,
                                               const Reference<XMimeContentType> & xRhs)
{
  Sequence<OUString> xLhsFlavors = xLhs->getParameters();
  Sequence<OUString> xRhsFlavors = xRhs->getParameters();

  // Stop here if the number of parameters is different already
  if (xLhsFlavors.getLength() != xRhsFlavors.getLength())
    return false;

  try
    {
      OUString pLhs;
      OUString pRhs;

      for (sal_Int32 i = 0; i < xLhsFlavors.getLength(); i++)
        {
          pLhs = xLhs->getParameterValue(xLhsFlavors[i]);
          pRhs = xRhs->getParameterValue(xLhsFlavors[i]);

          if (!pLhs.equalsIgnoreAsciiCase(pRhs))
            {
              return false;
            }
        }
    }
  catch(IllegalArgumentException&)
    {
      return false;
    }

  return true;
}

} // unnamed namespace

OSXTransferable::OSXTransferable(const Reference<XMimeContentTypeFactory> & rXMimeCntFactory,
                                 DataFlavorMapperPtr_t pDataFlavorMapper,
                                 NSPasteboard* pasteboard) :
  mrXMimeCntFactory(rXMimeCntFactory),
  mDataFlavorMapper(pDataFlavorMapper),
  mPasteboard(pasteboard)
{
  [mPasteboard retain];

  initClipboardItemList();
}

OSXTransferable::~OSXTransferable()
{
  [mPasteboard release];
}

Any SAL_CALL OSXTransferable::getTransferData( const DataFlavor& aFlavor )
{
  if (!isValidFlavor(aFlavor) || !isDataFlavorSupported(aFlavor))
    {
      throw UnsupportedFlavorException("AquaClipboard: Unsupported data flavor",
                                       static_cast<XTransferable*>(this));
    }

  bool bInternal(false);
  NSString const * sysFormat =
      (aFlavor.MimeType.startsWith("image/png"))
      ? DataFlavorMapper::openOfficeImageToSystemFlavor( mPasteboard )
      : mDataFlavorMapper->openOfficeToSystemFlavor(aFlavor, bInternal);
  DataProviderPtr_t dp;

  if ([sysFormat caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
    {
      NSArray* sysData = [mPasteboard propertyListForType: const_cast<NSString *>(sysFormat)];
      dp = DataFlavorMapper::getDataProvider(sysFormat, sysData);
    }
  else
    {
      NSData* sysData = [mPasteboard dataForType: const_cast<NSString *>(sysFormat)];
      dp = DataFlavorMapper::getDataProvider(sysFormat, sysData);
    }

  if (dp.get() == nullptr)
    {
      throw UnsupportedFlavorException("AquaClipboard: Unsupported data flavor",
                                       static_cast<XTransferable*>(this));
    }

  return dp->getOOoData();
}

Sequence< DataFlavor > SAL_CALL OSXTransferable::getTransferDataFlavors(  )
{
  return mFlavorList;
}

sal_Bool SAL_CALL OSXTransferable::isDataFlavorSupported(const DataFlavor& aFlavor)
{
    for (sal_Int32 i = 0; i < mFlavorList.getLength(); i++)
      if (compareDataFlavors(aFlavor, mFlavorList[i]))
        return true;

    return false;
}

void OSXTransferable::initClipboardItemList()
{
  NSArray* pboardFormats = [mPasteboard types];

  if (pboardFormats == nullptr)
    {
      throw RuntimeException("AquaClipboard: Cannot get clipboard data",
                             static_cast<XTransferable*>(this));
    }

  mFlavorList = mDataFlavorMapper->typesArrayToFlavorSequence(pboardFormats);
}

/* Compares two DataFlavors. Returns true if both DataFlavor have the same media type
   and the number of parameter and all parameter values do match otherwise false
   is returned.
 */
bool OSXTransferable::compareDataFlavors(const DataFlavor& lhs, const DataFlavor& rhs )
{
    try
    {
        Reference<XMimeContentType> xLhs(mrXMimeCntFactory->createMimeContentType(lhs.MimeType));
        Reference<XMimeContentType> xRhs(mrXMimeCntFactory->createMimeContentType(rhs.MimeType));

        if (!xLhs->getFullMediaType().equalsIgnoreAsciiCase(xRhs->getFullMediaType()) ||
            !cmpAllContentTypeParameter(xLhs, xRhs))
          {
            return false;
          }
    }
    catch( IllegalArgumentException& )
    {
        OSL_FAIL( "Invalid content type detected" );
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
