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

#include <sal/types.h>

#include "iOSTransferable.hxx"

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

using ::rtl::OUString;

namespace // private
{
    bool isValidFlavor( const DataFlavor& aFlavor )
    {
      size_t len = aFlavor.MimeType.getLength();
      Type dtype = aFlavor.DataType;
      return ((len > 0) && ((dtype == getCppuType((Sequence<sal_Int8>*)0)) || (dtype == getCppuType((OUString*)0))));
    }

} // namespace private


iOSTransferable::iOSTransferable(const Reference<XMimeContentTypeFactory> rXMimeCntFactory,
                                 UIPasteboard* pasteboard) :
  mrXMimeCntFactory(rXMimeCntFactory),
  mPasteboard(pasteboard)
{
  [mPasteboard retain];

  initClipboardItemList();
}


iOSTransferable::~iOSTransferable()
{
  [mPasteboard release];
}


Any SAL_CALL iOSTransferable::getTransferData( const DataFlavor& aFlavor )
  throw( UnsupportedFlavorException, IOException, RuntimeException )
{
  if (!isValidFlavor(aFlavor) || !isDataFlavorSupported(aFlavor))
    {
      throw UnsupportedFlavorException(OUString("IosClipboard: Unsupported data flavor"),
                                       static_cast<XTransferable*>(this));
    }

  throw UnsupportedFlavorException(OUString("IosClipboard: Unsupported data flavor"),
                                   static_cast<XTransferable*>(this));
  // ???
  return Any();
}


bool iOSTransferable::isUnicodeText(const DataFlavor& flavor)
{
  return (flavor.DataType == getCppuType((OUString*)0));
}


Sequence< DataFlavor > SAL_CALL iOSTransferable::getTransferDataFlavors(  )
    throw( RuntimeException )
{
  return mFlavorList;
}


sal_Bool SAL_CALL iOSTransferable::isDataFlavorSupported(const DataFlavor& aFlavor)
    throw( RuntimeException )
{
    for (sal_Int32 i = 0; i < mFlavorList.getLength(); i++)
      if (compareDataFlavors(aFlavor, mFlavorList[i]))
        return sal_True;

    return sal_False;
}


void iOSTransferable::initClipboardItemList()
{
  NSArray* pboardFormats = [mPasteboard pasteboardTypes];

  if (pboardFormats == NULL)
    {
      throw RuntimeException(OUString("IosClipboard: Cannot get clipboard data"),
                             static_cast<XTransferable*>(this));
    }

  // ???
}


/* Compares two DataFlavors. Returns true if both DataFlavor have the same media type
   and the number of parameter and all parameter values do match otherwise false
   is returned.
 */
bool iOSTransferable::compareDataFlavors(const DataFlavor& lhs, const DataFlavor& rhs )
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


bool iOSTransferable::cmpAllContentTypeParameter(const Reference<XMimeContentType> xLhs,
                                               const Reference<XMimeContentType> xRhs) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
