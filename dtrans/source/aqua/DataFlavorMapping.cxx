/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataFlavorMapping.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:08:57 $
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

#include <DataFlavorMapping.hxx>
#include "HtmlFmtFlt.hxx"
#include "PictToBmpFlt.hxx"

#ifndef _COM_SUN_STAR_DATATRANSFER_UNSUPPORTEDFLAVOREXCEPTION_HPP_
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif

#include <rtl/ustring.hxx>
#include <osl/endian.h>

#include <vector>

#include <stdio.h>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace ::com::sun::star::datatransfer;
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace std;

namespace // private
{
  const Type CPPUTYPE_SEQINT8  = getCppuType((Sequence<sal_Int8>*)0);
  const Type CPPUTYPE_OUSTRING = getCppuType( (OUString*)0 );

  /* Determine whether or not a DataFlavor is valid.
   */
  bool isValidFlavor(const DataFlavor& aFlavor)
  {
    size_t len = aFlavor.MimeType.getLength();
    Type dtype = aFlavor.DataType;
    return ((len > 0) && ((dtype == CPPUTYPE_SEQINT8) || (dtype == CPPUTYPE_OUSTRING)));
  }

 typedef vector<sal_Unicode> UnicodeBuffer;

  OUString CFStringToOUString(CFStringRef cfString)
  {
    BOOST_ASSERT(cfString && "Invalid parameter");

    CFIndex len = CFStringGetLength(cfString);
    UnicodeBuffer buff(len);
    CFRange range = { 0, len };

    CFStringGetCharacters(cfString, range, &buff[0]);

    return OUString(&buff[0], len);
  }

  struct FlavorMap
  {
    CFStringRef SystemFlavor;
    CFStringRef LegacyClipboardId;
    char* OOoFlavor;
    char* HumanPresentableName;
    Type DataType;
  };

  const CFStringRef EMPTY_STRING = CFSTR("");

  /* At the moment it appears as if only MS Office pastes "public.html" to the clipboard.
   */
  FlavorMap flavorMap[] =
    {
      { kUTTypeUTF16PlainText, CFSTR("ut16"), "text/plain;charset=utf-16", "Unicode Text (UTF-16)", CPPUTYPE_OUSTRING },
      { kUTTypeUTF8PlainText, CFSTR("utf8"), "text/plain;charset=utf-8", "Unicode Text (UTF-8)", CPPUTYPE_SEQINT8 },
      { kUTTypeRTF, CFSTR("RTF"), "text/richtext", "Rich Text Format", CPPUTYPE_SEQINT8 },
      { kUTTypeHTML, CFSTR("HTML"), "text/html", "Plain Html", CPPUTYPE_SEQINT8 },
      { EMPTY_STRING, CFSTR("OBJD"), "application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", CPPUTYPE_SEQINT8 },
      { EMPTY_STRING, CFSTR("EMBS"), "application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embedded Object", CPPUTYPE_SEQINT8 },
      { EMPTY_STRING, CFSTR("LKSD"), "application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", CPPUTYPE_SEQINT8 },
      { EMPTY_STRING, CFSTR("LNKS"), "application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", CPPUTYPE_SEQINT8 },
      //      { kUTTypePICT, CFSTR("PICT"), "image/x-macpict;windows_formatname=\"Mac Pict\"", "Mac Pict", CPPUTYPE_SEQINT8 }
      { kUTTypePICT, CFSTR("PICT"), "image/bmp", "Windows Bitmap", CPPUTYPE_SEQINT8 }
    };

#define SIZE_FLAVOR_MAP (sizeof(flavorMap)/sizeof(FlavorMap))

  /* Lookup a legacy clipboard id in the flavor table and return a
     corresponding OOo DataFlavor if possible.
  */
  DataFlavor getOOoFlavor(CFStringRef legacyClipboardId)
  {
    DataFlavor oOOFlavor;

    for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
      {
        if (CFStringCompare(legacyClipboardId, flavorMap[i].LegacyClipboardId, 0) == 0)
          {
            oOOFlavor.MimeType = OUString::createFromAscii(flavorMap[i].OOoFlavor);
            oOOFlavor.HumanPresentableName = OUString::createFromAscii(flavorMap[i].HumanPresentableName);
            oOOFlavor.DataType = flavorMap[i].DataType;
            break;
          }
      }

    return oOOFlavor;
  }

  CFStringRef getLegacyCLipboardId(const DataFlavor& oOOFlavor)
  {
    for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
      {
        if (oOOFlavor.MimeType.compareToAscii(flavorMap[i].OOoFlavor) == 0)
          {
            return flavorMap[i].LegacyClipboardId;
          }
      }
    return NULL;
  }

  inline bool isByteSequenceType(const Type& theType)
  {
    return (theType == CPPUTYPE_SEQINT8);
  }

  inline bool isOUStringType(const Type& theType)
  {
    return (theType == CPPUTYPE_OUSTRING);
  }

} // namespace private


//###########################

/* A base class for other data provider.
 */
class DataProviderBaseImpl : public DataProvider
{
public:
  DataProviderBaseImpl(const Any& data);
  DataProviderBaseImpl(CFDataRef data);
  virtual ~DataProviderBaseImpl();

protected:
  Any mData;
  CFDataRef mSystemData;
};

DataProviderBaseImpl::DataProviderBaseImpl(const Any& data) :
  mData(data),
  mSystemData(NULL)
{
}

DataProviderBaseImpl::DataProviderBaseImpl(CFDataRef data) :
  mSystemData(data)
{
  CFRetain(mSystemData);
}

DataProviderBaseImpl::~DataProviderBaseImpl()
{
  if (mSystemData)
    {
      CFRelease(mSystemData);
    }
}

//#################################

class UniDataProvider : public DataProviderBaseImpl
{
public:
  UniDataProvider(const Any& data);

  UniDataProvider(CFDataRef data);

  virtual CFDataRef getSystemData();

  virtual Any getOOoData();
};

UniDataProvider::UniDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

UniDataProvider::UniDataProvider(CFDataRef data) :
  DataProviderBaseImpl(data)
{
}

CFDataRef UniDataProvider::getSystemData()
{
  OUString ustr;
  mData >>= ustr;

  CFDataRef sysData = CFDataCreate(kCFAllocatorDefault,
                                   reinterpret_cast<const UInt8*>(ustr.getStr()),
                                   ustr.getLength() * sizeof(sal_Unicode));

  return sysData;
}

Any UniDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      CFIndex flavorDataLength = CFDataGetLength(mSystemData);
      const sal_Unicode* pUtf16 = reinterpret_cast<const sal_Unicode*>(CFDataGetBytePtr(mSystemData));
      oOOData = makeAny(OUString(pUtf16, flavorDataLength / sizeof(sal_Unicode)));
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

//###########################

class ByteSequenceDataProvider : public DataProviderBaseImpl
{
public:
  ByteSequenceDataProvider(const Any& data);

  ByteSequenceDataProvider(CFDataRef data);

  virtual CFDataRef getSystemData();

  virtual Any getOOoData();
};

ByteSequenceDataProvider::ByteSequenceDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

ByteSequenceDataProvider::ByteSequenceDataProvider(CFDataRef data) :
  DataProviderBaseImpl(data)
{
}

CFDataRef ByteSequenceDataProvider::getSystemData()
{
   Sequence<sal_Int8> rawData;
   mData >>= rawData;

   CFDataRef sysData = CFDataCreate(kCFAllocatorDefault,
                                    reinterpret_cast<const UInt8*>(rawData.getArray()),
                                    rawData.getLength());

   return sysData;
}

Any ByteSequenceDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      CFIndex flavorDataLength = CFDataGetLength(mSystemData);
      Sequence<sal_Int8> byteSequence;
      byteSequence.realloc(flavorDataLength);
      memcpy(byteSequence.getArray(), CFDataGetBytePtr(mSystemData), flavorDataLength);
      oOOData = makeAny(byteSequence);
    }
  else
    {
      oOOData =  mData;
    }

  return oOOData;
}

//###########################


class ObjDescDataProvider : public DataProviderBaseImpl
{
public:
  ObjDescDataProvider(const Any& data);

  ObjDescDataProvider(CFDataRef data);

  virtual CFDataRef getSystemData();

  virtual Any getOOoData();
};

ObjDescDataProvider::ObjDescDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

ObjDescDataProvider::ObjDescDataProvider(CFDataRef data) :
  DataProviderBaseImpl(data)
{
}

CFDataRef ObjDescDataProvider::getSystemData()
{
   Sequence<sal_Int8> rawData;
   mData >>= rawData;

   CFDataRef sysData = CFDataCreate(kCFAllocatorDefault,
                                    reinterpret_cast<const UInt8*>(rawData.getArray()),
                                    rawData.getLength());

   return sysData;
}

/* On Mac OS X the OBJECTDESCRIPTOR struct is different
   to that on Windows and MS Office 2004 seems to put
   it with big endian alignment on to the clipboard.
   Furthermore there is an additional 4 byte value
   between dwStatus and dwFulUserTypeName.
   That means we have to convert it to the expected
   structure (see OleObjectDescriptor in
   so3/source/dialog/pastedlg.cxx).
 */

/* Some dummy declarations
 */
struct ClsId
{
  sal_uInt32 dummy1;
  sal_uInt32 dummy2;
  sal_uInt32 dummy3;
  sal_uInt32 dummy4;
};

struct Size_
{
  sal_uInt32 dummy1;
  sal_uInt32 dummy2;
};

typedef Size_ Point_;

/* This is the declaration of the original OBJECTDESCRIPTOR
   struct on Windows:
*/
struct OleObjectDescriptor
{
  sal_uInt32 cbSize;
  ClsId clsid;
  sal_uInt32 dwDrawAspect;
  Size_ sizel;
  Point_        pointl;
  sal_uInt32    dwStatus;
  sal_uInt32    dwFullUserTypeName;
  sal_uInt32    dwSrcOfCopy;
};

struct OSXOleObjectDescriptor
{
  sal_uInt32 cbSize;
  ClsId clsid;
  sal_uInt32 dwDrawAspect;
  Size_ sizel;
  Point_ pointl;
  sal_uInt32 dwStatus;
  sal_uInt32 dummy;
  sal_uInt32 dwFullUserTypeName;
  sal_uInt32 dwSrcOfCopy;
};


Any ObjDescDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      const OSXOleObjectDescriptor* pOSXObjDesc = reinterpret_cast<const OSXOleObjectDescriptor*>(CFDataGetBytePtr(mSystemData));
      size_t sz = OSL_SWAPDWORD(pOSXObjDesc->cbSize) - sizeof(pOSXObjDesc->dummy);
      Sequence<sal_Int8> byteSequence(sz);
      OleObjectDescriptor* pObjDesc = reinterpret_cast<OleObjectDescriptor*>(byteSequence.getArray());

      pObjDesc->cbSize = sz;
      pObjDesc->clsid.dummy1 = OSL_SWAPDWORD(pOSXObjDesc->clsid.dummy1);
      pObjDesc->clsid.dummy2 = OSL_SWAPDWORD(pOSXObjDesc->clsid.dummy1);
      pObjDesc->clsid.dummy3 = OSL_SWAPDWORD(pOSXObjDesc->clsid.dummy1);
      pObjDesc->clsid.dummy4 = OSL_SWAPDWORD(pOSXObjDesc->clsid.dummy1);
      pObjDesc->dwDrawAspect = OSL_SWAPDWORD(pOSXObjDesc->dwDrawAspect);
      pObjDesc->sizel.dummy1 = OSL_SWAPDWORD(pOSXObjDesc->sizel.dummy1);
      pObjDesc->sizel.dummy2 = OSL_SWAPDWORD(pOSXObjDesc->sizel.dummy2);
      pObjDesc->pointl.dummy1 = OSL_SWAPDWORD(pOSXObjDesc->pointl.dummy1);
      pObjDesc->pointl.dummy2 = OSL_SWAPDWORD(pOSXObjDesc->pointl.dummy2);
      pObjDesc->dwStatus = OSL_SWAPDWORD(pOSXObjDesc->dwStatus);
      pObjDesc->dwFullUserTypeName = OSL_SWAPDWORD(pOSXObjDesc->dwFullUserTypeName) - sizeof(pOSXObjDesc->dummy);
      pObjDesc->dwSrcOfCopy = OSL_SWAPDWORD(pOSXObjDesc->dwSrcOfCopy) - sizeof(pOSXObjDesc->dummy);

      char* pDest = reinterpret_cast<char*>(reinterpret_cast<char*>(pObjDesc) + pObjDesc->dwFullUserTypeName);
      const char* pSrc = reinterpret_cast<const char*>(reinterpret_cast<const char*>(pOSXObjDesc) + OSL_SWAPDWORD(pOSXObjDesc->dwFullUserTypeName));
      size_t len = OSL_SWAPDWORD(pOSXObjDesc->cbSize) - OSL_SWAPDWORD(pOSXObjDesc->dwFullUserTypeName);

      memcpy(pDest, pSrc, len);

      // swap FullUserTypeName bytes
      sal_Unicode* p = reinterpret_cast<sal_Unicode*>(pDest);
      while (*p)
        {
          *p = OSL_SWAPWORD(*p);
          p++;
        }

      // swap SrcOfCopy bytes
      p = reinterpret_cast<sal_Unicode*>(reinterpret_cast<char*>(pObjDesc) + pObjDesc->dwSrcOfCopy);
      while (*p)
        {
          *p = OSL_SWAPWORD(*p);
          p++;
        }

      oOOData = makeAny(byteSequence);
    }
  else
    {
      oOOData =  mData;
    }

  return oOOData;
}

//###########################

class HTMLFormatDataProvider : public DataProviderBaseImpl
{
public:
  HTMLFormatDataProvider(const Any& data);

  HTMLFormatDataProvider(CFDataRef data);

  virtual CFDataRef getSystemData();

  virtual Any getOOoData();
};

HTMLFormatDataProvider::HTMLFormatDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

HTMLFormatDataProvider::HTMLFormatDataProvider(CFDataRef data) :
  DataProviderBaseImpl(data)
{
}

CFDataRef HTMLFormatDataProvider::getSystemData()
{
  Sequence<sal_Int8> textHtmlData;
  mData >>= textHtmlData;

  Sequence<sal_Int8> htmlFormatData = TextHtmlToHTMLFormat(textHtmlData);

  CFDataRef sysData = CFDataCreate(kCFAllocatorDefault,
                                   reinterpret_cast<const UInt8*>(htmlFormatData.getArray()),
                                   htmlFormatData.getLength());

  return sysData;
}

Any HTMLFormatDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      CFIndex flavorDataLength = CFDataGetLength(mSystemData);
      Sequence<sal_Int8> unkHtmlData;

      unkHtmlData.realloc(flavorDataLength);
      memcpy(unkHtmlData.getArray(), CFDataGetBytePtr(mSystemData), flavorDataLength);

      Sequence<sal_Int8>* pPlainHtml = &unkHtmlData;
      Sequence<sal_Int8> plainHtml;

      if (isHTMLFormat(unkHtmlData))
        {
          plainHtml = HTMLFormatToTextHtml(unkHtmlData);
          pPlainHtml = &plainHtml;
        }

      oOOData = makeAny(*pPlainHtml);
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

//###########################

class BMPDataProvider : public DataProviderBaseImpl
{
public:
  BMPDataProvider(const Any& data);

  BMPDataProvider(CFDataRef data);

  virtual CFDataRef getSystemData();

  virtual Any getOOoData();
};

BMPDataProvider::BMPDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

BMPDataProvider::BMPDataProvider(CFDataRef data) :
  DataProviderBaseImpl(data)
{
}

CFDataRef BMPDataProvider::getSystemData()
{
  Sequence<sal_Int8> bmpData;
  mData >>= bmpData;

  Sequence<sal_Int8> pictData;
  CFDataRef sysData = NULL;

  if (BMPtoPICT(bmpData, pictData))
    {
      sysData = CFDataCreate(kCFAllocatorDefault,
                             reinterpret_cast<const UInt8*>(pictData.getArray()),
                             pictData.getLength());
    }

  return sysData;
}

/* At the moment the OOo 'PCT' filter is not good enough to be used
   and there is no flavor defined for exchanging 'PCT' with OOo so
   we will at the moment convert 'PCT' to a Windows BMP and provide
   this to OOo
*/
Any BMPDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      CFIndex flavorDataLength = CFDataGetLength(mSystemData);
      Sequence<sal_Int8> pictData(flavorDataLength);

      memcpy(pictData.getArray(), CFDataGetBytePtr(mSystemData), flavorDataLength);

      Sequence<sal_Int8> bmpData;

      if (PICTtoBMP(pictData, bmpData))
        {
          oOOData = makeAny(bmpData);
        }
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}


//###########################

DataFlavorMapper::DataFlavorMapper(const Reference< XMultiServiceFactory > rServiceManager) :
  mrServiceManager(rServiceManager)
{
    mrXMimeCntFactory = Reference<XMimeContentTypeFactory>(mrServiceManager->createInstance(
      OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.MimeContentTypeFactory"))), UNO_QUERY);

  if (!mrXMimeCntFactory.is())
    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create com.sun.star.datatransfer.MimeContentTypeFactory")), NULL);
}

DataFlavor DataFlavorMapper::systemToOpenOfficeFlavor(CFStringRef systemDataFlavor) const
{
  if (isDynamicSystemFlavor(systemDataFlavor))
    {
      return resolveDynamicSystemFlavor(systemDataFlavor);
    }

  DataFlavor oOOFlavor;

  for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      if (UTTypeEqual(systemDataFlavor, flavorMap[i].SystemFlavor))
        {
          oOOFlavor.MimeType = OUString::createFromAscii(flavorMap[i].OOoFlavor);
          oOOFlavor.HumanPresentableName = OUString(RTL_CONSTASCII_USTRINGPARAM(flavorMap[i].HumanPresentableName));
          oOOFlavor.DataType = flavorMap[i].DataType;
          break;
        }
    } // for

  return oOOFlavor;
}

CFStringRef DataFlavorMapper::openOfficeToSystemFlavor(const DataFlavor& oOOFlavor) const
{
  for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      if (oOOFlavor.MimeType.compareToAscii(flavorMap[i].OOoFlavor, strlen(flavorMap[i].OOoFlavor)) == 0)
        {
          return flavorMap[i].SystemFlavor;
        }
    }

  // no equivalent system type found, then register a dynamic type
  return registerDynamicSystemFlavor(oOOFlavor);
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(CFStringRef systemFlavor, Reference<XTransferable> rTransferable) const
{
  DataProviderPtr_t dp;

  try
    {
      DataFlavor oOOFlavor = systemToOpenOfficeFlavor(systemFlavor);

      Any data = rTransferable->getTransferData(oOOFlavor);

      if (isByteSequenceType(data.getValueType()))
        {
          if (UTTypeEqual(systemFlavor, kUTTypeHTML))
            {
              dp = DataProviderPtr_t(new HTMLFormatDataProvider(data));
            }
          else if (UTTypeEqual(systemFlavor, kUTTypePICT))
            {
              dp = DataProviderPtr_t(new BMPDataProvider(data));
            }
          else
            {
              dp = DataProviderPtr_t(new ByteSequenceDataProvider(data));
            }
        }
      else if (isOUStringType(data.getValueType()))
        {
          dp = DataProviderPtr_t(new UniDataProvider(data));
        }
      else
        {
          BOOST_ASSERT(false && "AquaClipboard: Invalid data type provided!");
        }
    }
  catch(UnsupportedFlavorException&)
    {
      // Something is wrong the transferable doesn't have
      // the data it claimed to deliver
    }

  return dp;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(const CFStringRef systemFlavor, CFDataRef systemData) const
{
  DataProviderPtr_t dp;

  if (UTTypeEqual(kUTTypeUTF16PlainText, systemFlavor))
    {
      dp = DataProviderPtr_t(new UniDataProvider(systemData));
    }
  else if (UTTypeEqual(kUTTypeHTML, systemFlavor))
    {
      dp = DataProviderPtr_t(new HTMLFormatDataProvider(systemData));
    }
  else if (UTTypeEqual(CFSTR("dyn.agk8y8uwnku"), systemFlavor)) // OBJD
    {
      dp = DataProviderPtr_t(new ObjDescDataProvider(systemData));
    }
  else if (UTTypeEqual(kUTTypePICT, systemFlavor))
    {
      dp = DataProviderPtr_t(new BMPDataProvider(systemData));
      }
  else
    {
      dp = DataProviderPtr_t(new ByteSequenceDataProvider(systemData));
    }

  return dp;
}


inline bool DataFlavorMapper::isDynamicSystemFlavor(const CFStringRef flavor) const
{
  return CFStringHasPrefix(flavor, CFSTR("dyn."));
}


const CFStringRef tagClasses[] = {
  kUTTagClassOSType,
  kUTTagClassMIMEType,
  kUTTagClassFilenameExtension,
  kUTTagClassNSPboardType
};

#define SIZE_TAG_CLASSES (sizeof(tagClasses)/sizeof(tagClasses[0]))


DataFlavor DataFlavorMapper::resolveDynamicSystemFlavor(CFStringRef dynFlavor) const
{
  CFStringRef resolvedType = NULL;

  for (size_t i = 0; i < SIZE_TAG_CLASSES; i++)
    {
      if ((resolvedType = UTTypeCopyPreferredTagWithClass(dynFlavor, tagClasses[i])) != NULL)
        break;
    }

  DataFlavor dflv;

  if (resolvedType)
    {
      dflv = getOOoFlavor(resolvedType);

      if (!isValidFlavor(dflv))
        {
          OUString cntType = CFStringToOUString(resolvedType);

          if (isValidMimeContentType(cntType))
            {
              // Not able to map the resolved type to a valid flavor
              // seraching the legacy clipboard id namespace so we
              // register a flavor dynamically if the content type
              // a well formed
              dflv.MimeType = cntType;
              dflv.DataType = CPPUTYPE_SEQINT8;
            }
        }

      CFRelease(resolvedType);
    }

  return dflv;
}

CFStringRef DataFlavorMapper::registerDynamicSystemFlavor(const DataFlavor& flavor) const
{
  CFStringRef dynFlavor = NULL;

  if (flavor.MimeType.compareToAscii("application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"") == 0)
    {
      dynFlavor = UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType,
                                                        CFSTR("OBJD"),
                                                        NULL);
    }
  else if (flavor.MimeType.compareToAscii("application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"") == 0)
    {
      dynFlavor = UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType,
                                                        CFSTR("EMBS"),
                                                        NULL);
    }
  else if (flavor.MimeType.compareToAscii("application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"") == 0)
    {
      dynFlavor = UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType,
                                                        CFSTR("LKSD"),
                                                        NULL);
    }
  else if (flavor.MimeType.compareToAscii("application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"") == 0)
    {
      dynFlavor = UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType,
                                                        CFSTR("LNKS"),
                                                        NULL);
    }
  else
    {
      CFStringRef cntType = CFStringCreateWithCharactersNoCopy(NULL,
                                                               flavor.MimeType.getStr(),
                                                               flavor.MimeType.getLength(),
                                                               kCFAllocatorNull);

      if (cntType)
        {
          /* We have to register dynamic types using tag class 'kUTTagClassNSPboardType'.
             Using 'kUTTagClassMIMEType' for instance will be handled case-insensitive
             internally. The backward converted string differs from the one we register
             here with regards to case sensitivity. Because the upper layers only do
             a simple case-sensitive string comparision of the data flavor mime types,
             we'll miss to get the requested information. :(
           */
          dynFlavor = UTTypeCreatePreferredIdentifierForTag(kUTTagClassNSPboardType,
                                                            cntType,
                                                            NULL);
        }
    }

  return dynFlavor;
}


bool DataFlavorMapper::isValidMimeContentType(const rtl::OUString& contentType) const
{
  bool result = true;

  try
    {
      Reference<XMimeContentType> xCntType(mrXMimeCntFactory->createMimeContentType(contentType));
    }
  catch( IllegalArgumentException& )
    {
      result = false;
    }

  return result;
}
