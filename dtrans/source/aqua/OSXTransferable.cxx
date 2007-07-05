/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OSXTransferable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:10:07 $
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

} // end namespace private


ClipboardItem::ClipboardItem(PasteboardItemID id,
                         CFStringRef systemFlavor,
                         const DataFlavor& oOOFlavor) :
  mId(id),
  mSystemDataFlavor(systemFlavor),
  mOOoDataFlavor(oOOFlavor)
{
  CFRetain(mSystemDataFlavor);
}

ClipboardItem::~ClipboardItem()
{
  CFRelease(mSystemDataFlavor);
}


OSXTransferable::OSXTransferable(const Reference< XMultiServiceFactory > ServiceManager,
                                 const Reference<XMimeContentTypeFactory> rXMimeCntFactory,
                                 DataFlavorMapperPtr_t pDataFlavorMapper) :
  mrSrvMgr(ServiceManager),
  mrXMimeCntFactory(rXMimeCntFactory),
  mpDataFlavorMapper(pDataFlavorMapper)
{
    if (PasteboardCreate(kPasteboardClipboard, &mrClipboard) != noErr)
    {
      throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create Mac OS X system cliboard")),
                             static_cast<XTransferable*>(this));
    }

    initClipboardItemList();
}


OSXTransferable::~OSXTransferable()
{
  CFRelease(mrClipboard);
}


Any SAL_CALL OSXTransferable::getTransferData( const DataFlavor& aFlavor )
  throw( UnsupportedFlavorException, IOException, RuntimeException )
{
    ClipboardItemPtr_t clipboardItem;
    if (!isValidFlavor(aFlavor))
      {
        throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Unsupported data flavor")),
                                         static_cast<XTransferable*>(this));
      }

    clipboardItem = findMatchingClipboardItem(aFlavor);
    if (!clipboardItem)
      {
        throw UnsupportedFlavorException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Unsupported data flavor")),
                                         static_cast<XTransferable*>(this));
      }

    return getClipboardItemData(clipboardItem);
}


Any OSXTransferable::getClipboardItemData(ClipboardItemPtr_t clipboardItem)
{
  CFDataRef flavorData;
  if (PasteboardCopyItemFlavorData(mrClipboard, clipboardItem->mId, clipboardItem->mSystemDataFlavor, &flavorData) != noErr)
    throw IOException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot retrieve clipboard data")),
                      static_cast<XTransferable*>(this));

  DataProviderPtr_t dp = mpDataFlavorMapper->getDataProvider(clipboardItem->mSystemDataFlavor, flavorData);

  return dp->getOOoData();
}


bool OSXTransferable::isUnicodeText(ClipboardItemPtr_t clipboardItem)
{
  return (clipboardItem->mOOoDataFlavor.DataType == CPPUTYPE_OUSTRING);
}


ClipboardItemPtr_t OSXTransferable::findMatchingClipboardItem(const DataFlavor& aFlavor)
{
  ClipboardItemContainer_t::iterator iter = mClipboardItems.begin();
  ClipboardItemContainer_t::iterator  iter_end = mClipboardItems.end();

  for (/*nop*/; iter != iter_end; ++iter)
    {
      if (compareDataFlavors((*iter)->mOOoDataFlavor, aFlavor))
        return *iter;
    }

  return ClipboardItemPtr_t();
}


Sequence< DataFlavor > SAL_CALL OSXTransferable::getTransferDataFlavors(  )
    throw( RuntimeException )
{
  Sequence<DataFlavor> flavorList;
  ClipboardItemContainer_t::const_iterator iter = mClipboardItems.begin();
  ClipboardItemContainer_t::const_iterator  iter_end = mClipboardItems.end();

  for ( /* nop */; iter != iter_end; ++iter)
    {
      DataFlavor df = (*iter)->mOOoDataFlavor;
      flavorList.realloc(flavorList.getLength() + 1);
      flavorList[flavorList.getLength() - 1] = (*iter)->mOOoDataFlavor;
    }

  return flavorList;
}


sal_Bool SAL_CALL OSXTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw( RuntimeException )
{
    for (sal_Int32 i = 0; i < m_FlavorList.getLength(); i++)
      if (compareDataFlavors(aFlavor, m_FlavorList[i]))
        return sal_True;

    return sal_False;
}


void OSXTransferable::initClipboardItemList()
{
  ItemCount item_count;
  if (PasteboardGetItemCount(mrClipboard, &item_count) != noErr)
    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot get clipboard data")),
                                    static_cast<XTransferable*>(this));

  for (UInt32 item = 1; item <= item_count; item++)
    {
      PasteboardItemID itemID;

      if (PasteboardGetItemIdentifier(mrClipboard, item, &itemID) != noErr)
        continue;

      addClipboardItemFlavors(itemID);
    }
}


void OSXTransferable::addClipboardItemFlavors(PasteboardItemID clipboardItemId)
{
  CFArrayRef flavorTypeArray;
  if (PasteboardCopyItemFlavors(mrClipboard, clipboardItemId, &flavorTypeArray) != noErr)
    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot get clipboard data")),
                           static_cast<XTransferable*>(this));

  CFIndex flavorCount = CFArrayGetCount(flavorTypeArray);
  ClipboardItemPtr_t clipboardItem;

  for (CFIndex flavorIndex = 0; flavorIndex < flavorCount; flavorIndex++)
    {
      CFStringRef systemFlavor = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);
      DataFlavor oOOFlavor = mpDataFlavorMapper->systemToOpenOfficeFlavor(systemFlavor);

      if (isValidFlavor(oOOFlavor))
        {
          mClipboardItems.push_back(ClipboardItemPtr_t(new ClipboardItem(clipboardItemId, systemFlavor, oOOFlavor)));
        }
    }
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
