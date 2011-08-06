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
      throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("IosClipboard: Unsupported data flavor")),
                                       static_cast<XTransferable*>(this));
    }

  throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("IosClipboard: Unsupported data flavor")),
                                   static_cast<XTransferable*>(this));
  // ???
  return Any();
}


bool iOSTransferable::isUnicodeText(const DataFlavor& flavor)
{
  return (flavor.DataType == CPPUTYPE_OUSTRING);
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
      throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("IosClipboard: Cannot get clipboard data")),
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
