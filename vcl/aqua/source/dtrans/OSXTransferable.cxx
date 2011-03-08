/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <sal/types.h>

#include "OSXTransferable.hxx"

#include "DataFlavorMapping.hxx"

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

using ::rtl::OUString;

const Type CPPUTYPE_SEQINT8  = getCppuType((Sequence<sal_Int8>*)0);
const Type CPPUTYPE_OUSTRING = getCppuType((OUString*)0);

namespace // private
{
    bool isValidFlavor( const DataFlavor& aFlavor )
    {
      size_t len = aFlavor.MimeType.getLength();
      Type dtype = aFlavor.DataType;
      return ((len > 0) && ((dtype == CPPUTYPE_SEQINT8) || (dtype == CPPUTYPE_OUSTRING)));
    }

} // namespace private


OSXTransferable::OSXTransferable(const Reference<XMimeContentTypeFactory> rXMimeCntFactory,
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
  throw( UnsupportedFlavorException, IOException, RuntimeException )
{
  if (!isValidFlavor(aFlavor) || !isDataFlavorSupported(aFlavor))
    {
      throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Unsupported data flavor")),
                                       static_cast<XTransferable*>(this));
    }

  NSString* sysFormat =
      (aFlavor.MimeType.compareToAscii( "image/bmp", 9 ) == 0)
      ? mDataFlavorMapper->openOfficeImageToSystemFlavor( mPasteboard )
      : mDataFlavorMapper->openOfficeToSystemFlavor(aFlavor);
  DataProviderPtr_t dp;

  if ([sysFormat caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
    {
      NSArray* sysData = [mPasteboard propertyListForType: sysFormat];
      dp = mDataFlavorMapper->getDataProvider(sysFormat, sysData);
    }
  else
    {
      NSData* sysData = [mPasteboard dataForType: sysFormat];
      dp = mDataFlavorMapper->getDataProvider(sysFormat, sysData);
    }

  if (dp.get() == NULL)
    {
      throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Unsupported data flavor")),
                                       static_cast<XTransferable*>(this));
    }

  return dp->getOOoData();
}


bool OSXTransferable::isUnicodeText(const DataFlavor& flavor)
{
  return (flavor.DataType == CPPUTYPE_OUSTRING);
}


Sequence< DataFlavor > SAL_CALL OSXTransferable::getTransferDataFlavors(  )
    throw( RuntimeException )
{
  return mFlavorList;
}


sal_Bool SAL_CALL OSXTransferable::isDataFlavorSupported(const DataFlavor& aFlavor)
    throw( RuntimeException )
{
    for (sal_Int32 i = 0; i < mFlavorList.getLength(); i++)
      if (compareDataFlavors(aFlavor, mFlavorList[i]))
        return sal_True;

    return sal_False;
}


void OSXTransferable::initClipboardItemList()
{
  NSArray* pboardFormats = [mPasteboard types];

  if (pboardFormats == NULL)
    {
      throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot get clipboard data")),
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
        OSL_ENSURE( sal_False, "Invalid content type detected" );
        return false;
    }

    return true;
}


bool OSXTransferable::cmpAllContentTypeParameter(const Reference<XMimeContentType> xLhs,
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
