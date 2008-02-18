/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OSXTransferable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:49:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _TRANSFERABLE_HXX_
#include "OSXTransferable.hxx"
#endif

#include "DataFlavorMapping.hxx"

using namespace rtl;
using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

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


OSXTransferable::OSXTransferable(const Reference< XComponentContext >& context,
                                 const Reference<XMimeContentTypeFactory> rXMimeCntFactory,
                                 DataFlavorMapperPtr_t pDataFlavorMapper,
                                 NSPasteboard* pasteboard) :
  mXComponentContext(context),
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

  NSString* sysFormat = (NSString*)mDataFlavorMapper->openOfficeToSystemFlavor(aFlavor);
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
