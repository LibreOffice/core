/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataFlavorMapping.cxx,v $
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

#include <DataFlavorMapping.hxx>
#include "HtmlFmtFlt.hxx"
#include "PictToBmpFlt.hxx"
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include "com/sun/star/uno/Sequence.hxx"

#include <rtl/ustring.hxx>
#include <rtl/memory.h>
#include <osl/endian.h>

#include <vector>
#include <stdio.h>

#include <premac.h>
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

  OUString NSStringToOUString(NSString* cfString)
  {
    BOOST_ASSERT(cfString && "Invalid parameter");

    const char* utf8Str = [cfString UTF8String];
    unsigned int len = rtl_str_getLength(utf8Str);

    return OUString(utf8Str, len, RTL_TEXTENCODING_UTF8);
  }

  NSString* OUStringToNSString(const OUString& ustring)
  {
    OString utf8Str = OUStringToOString(ustring, RTL_TEXTENCODING_UTF8);
    return [NSString stringWithCString: utf8Str.getStr() encoding: NSUTF8StringEncoding];
  }


  const NSString* PBTYPE_UT16 = @"CorePasteboardFlavorType 0x75743136";
  const NSString* PBTYPE_OBJD = @"CorePasteboardFlavorType 0x4F424A44";
  const NSString* PBTYPE_EMBS = @"CorePasteboardFlavorType 0x454D4253";
  const NSString* PBTYPE_LKSD = @"CorePasteboardFlavorType 0x4C4B5344";
  const NSString* PBTYPE_LNKS = @"CorePasteboardFlavorType 0x4C4E4B53";
  const NSString* PBTYPE_PICT = @"CorePasteboardFlavorType 0x50494354";
  const NSString* PBTYPE_HTML = @"CorePasteboardFlavorType 0x48544D4C";
  const NSString* PBTYPE_SODX = @"application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
  const NSString* PBTYPE_SESX = @"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const NSString* PBTYPE_SLSDX = @"application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"";
  const NSString* PBTYPE_ESX = @"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const NSString* PBTYPE_LSX = @"application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"";
  const NSString* PBTYPE_EOX = @"application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"";
  const NSString* PBTYPE_SVXB = @"application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"";
  const NSString* PBTYPE_GDIMF = @"application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"";
  const NSString* PBTYPE_WMF = @"application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
  const NSString* PBTYPE_EMF = @"application/x-openoffice-emf;windows_formatname=\"Image EMF\"";

  const char* FLAVOR_SODX = "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
  const char* FLAVOR_SESX = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const char* FLAVOR_SLSDX = "application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"";
  const char* FLAVOR_ESX = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const char* FLAVOR_LSX = "application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"";
  const char* FLAVOR_EOX = "application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"";
  const char* FLAVOR_SVXB = "application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"";
  const char* FLAVOR_GDIMF = "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"";
  const char* FLAVOR_WMF = "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
  const char* FLAVOR_EMF = "application/x-openoffice-emf;windows_formatname=\"Image EMF\"";


  struct FlavorMap
  {
    NSString* SystemFlavor;
    const char* OOoFlavor;
    const char* HumanPresentableName;
    Type DataType;
  };

  /* At the moment it appears as if only MS Office pastes "public.html" to the clipboard.
   */
  FlavorMap flavorMap[] =
    {
      { NSStringPboardType, "text/plain;charset=utf-16", "Unicode Text (UTF-16)", CPPUTYPE_OUSTRING },
      { NSRTFPboardType, "text/richtext", "Rich Text Format", CPPUTYPE_SEQINT8 },
      { NSPICTPboardType, "image/bmp", "Windows Bitmap", CPPUTYPE_SEQINT8 },
      { NSHTMLPboardType, "text/html", "Plain Html", CPPUTYPE_SEQINT8 },
      { NSFilenamesPboardType, "application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", CPPUTYPE_SEQINT8 },
      { PBTYPE_SESX, FLAVOR_SESX, "Star Embed Source (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_SLSDX, FLAVOR_SLSDX, "Star Link Source Descriptor (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_ESX, FLAVOR_ESX, "Star Embed Source (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_LSX, FLAVOR_LSX, "Star Link Source (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_EOX, FLAVOR_EOX, "Star Embedded Object (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_SVXB, FLAVOR_SVXB, "SVXB (StarView Bitmap/Animation", CPPUTYPE_SEQINT8 },
      { PBTYPE_GDIMF, FLAVOR_GDIMF, "GDIMetaFile", CPPUTYPE_SEQINT8 },
      { PBTYPE_WMF, FLAVOR_WMF, "Windows MetaFile", CPPUTYPE_SEQINT8 },
      { PBTYPE_EMF, FLAVOR_EMF, "Windows Enhanced MetaFile", CPPUTYPE_SEQINT8 },
      { PBTYPE_SODX, FLAVOR_SODX, "Star Object Descriptor (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_OBJD, "application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", CPPUTYPE_SEQINT8 },
      { PBTYPE_EMBS, "application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embedded Object", CPPUTYPE_SEQINT8 },
      { PBTYPE_LKSD, "application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", CPPUTYPE_SEQINT8 },
      { PBTYPE_LNKS, "application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", CPPUTYPE_SEQINT8 }
      //      { PBTYPE_UT16, "text/plain;charset=utf-16", "Unicode Text (UTF-16)", CPPUTYPE_OUSTRING }
      //      { kUTTypePICT, @"PICT", "image/x-macpict;windows_formatname=\"Mac Pict\"", "Mac Pict", CPPUTYPE_SEQINT8 }
      //      { kUTTypeHTML, @"HTML", "text/html", "Plain Html", CPPUTYPE_SEQINT8 }
    };


  #define SIZE_FLAVOR_MAP (sizeof(flavorMap)/sizeof(FlavorMap))


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
  DataProviderBaseImpl(id data);
  virtual ~DataProviderBaseImpl();

protected:
  Any mData;
  //NSData* mSystemData;
  id mSystemData;
};

DataProviderBaseImpl::DataProviderBaseImpl(const Any& data) :
  mData(data),
  mSystemData(nil)
{
}

DataProviderBaseImpl::DataProviderBaseImpl(id data) :
  mSystemData(data)
{
  [mSystemData retain];
}


DataProviderBaseImpl::~DataProviderBaseImpl()
{
  if (mSystemData)
    {
      [mSystemData release];
    }
}

//#################################

class UniDataProvider : public DataProviderBaseImpl
{
public:
  UniDataProvider(const Any& data);

  UniDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

UniDataProvider::UniDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

UniDataProvider::UniDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* UniDataProvider::getSystemData()
{
  OUString ustr;
  mData >>= ustr;

  OString strUtf8;
  ustr.convertToString(&strUtf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

  return [NSData dataWithBytes: strUtf8.getStr() length: strUtf8.getLength()];
}

Any UniDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      oOOData = makeAny(OUString(reinterpret_cast<const sal_Char*>([mSystemData bytes]),
                                 [mSystemData length],
                                 RTL_TEXTENCODING_UTF8));
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

  ByteSequenceDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

ByteSequenceDataProvider::ByteSequenceDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

ByteSequenceDataProvider::ByteSequenceDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}


NSData* ByteSequenceDataProvider::getSystemData()
{
   Sequence<sal_Int8> rawData;
   mData >>= rawData;

   return [NSData dataWithBytes: rawData.getArray() length: rawData.getLength()];
}

Any ByteSequenceDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      unsigned int flavorDataLength = [mSystemData length];
      Sequence<sal_Int8> byteSequence;
      byteSequence.realloc(flavorDataLength);
      memcpy(byteSequence.getArray(), [mSystemData bytes], flavorDataLength);
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

  ObjDescDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

ObjDescDataProvider::ObjDescDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

ObjDescDataProvider::ObjDescDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* ObjDescDataProvider::getSystemData()
{
   Sequence<sal_Int8> rawData;
   mData >>= rawData;

   return [NSData dataWithBytes: rawData.getArray() length: rawData.getLength()];
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
      const OSXOleObjectDescriptor* pOSXObjDesc = reinterpret_cast<const OSXOleObjectDescriptor*>([mSystemData bytes]);
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

  HTMLFormatDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

HTMLFormatDataProvider::HTMLFormatDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

HTMLFormatDataProvider::HTMLFormatDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* HTMLFormatDataProvider::getSystemData()
{
  Sequence<sal_Int8> textHtmlData;
  mData >>= textHtmlData;

  Sequence<sal_Int8> htmlFormatData = TextHtmlToHTMLFormat(textHtmlData);

  return [NSData dataWithBytes: htmlFormatData.getArray() length: htmlFormatData.getLength()];
}

Any HTMLFormatDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      unsigned int flavorDataLength = [mSystemData length];
      Sequence<sal_Int8> unkHtmlData;

      unkHtmlData.realloc(flavorDataLength);
      memcpy(unkHtmlData.getArray(), [mSystemData bytes], flavorDataLength);

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

  BMPDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

BMPDataProvider::BMPDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

BMPDataProvider::BMPDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* BMPDataProvider::getSystemData()
{
  Sequence<sal_Int8> bmpData;
  mData >>= bmpData;

  Sequence<sal_Int8> pictData;
  NSData* sysData = NULL;

  if (BMPtoPICT(bmpData, pictData))
    {
      sysData = [NSData dataWithBytes: pictData.getArray() length: pictData.getLength()];
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
      unsigned int flavorDataLength = [mSystemData length];
      Sequence<sal_Int8> pictData(flavorDataLength);

      memcpy(pictData.getArray(), [mSystemData bytes], flavorDataLength);

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

//######################

class FileListDataProvider : public DataProviderBaseImpl
{
public:
  FileListDataProvider(const Any& data);
  FileListDataProvider(NSArray* data);

  virtual NSData* getSystemData();
  virtual Any getOOoData();
};

FileListDataProvider::FileListDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

FileListDataProvider::FileListDataProvider(NSArray* data) :
  DataProviderBaseImpl(data)
{
}

NSData* FileListDataProvider::getSystemData()
{
  return [NSData data];
}

Any FileListDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
    {
      size_t length = [mSystemData count];
      size_t lenSeqRequired = 0;

      for (size_t i = 0; i < length; i++)
        {
          NSString* fname = [mSystemData objectAtIndex: i];
          lenSeqRequired += [fname maximumLengthOfBytesUsingEncoding: NSUnicodeStringEncoding] + sizeof(unichar);
        }

      Sequence<sal_Int8> oOOFileList(lenSeqRequired);
      unichar* pBuffer = reinterpret_cast<unichar*>(oOOFileList.getArray());
      rtl_zeroMemory(pBuffer, lenSeqRequired);

      for (size_t i = 0; i < length; i++)
        {
          NSString* fname = [mSystemData objectAtIndex: i];
          [fname getCharacters: pBuffer];
          size_t l = [fname length];
          pBuffer += l + 1;
        }

      oOOData = makeAny(oOOFileList);
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

//###########################

DataFlavorMapper::DataFlavorMapper(const Reference<XComponentContext>& context) :
  mXComponentContext(context)
{
  Reference<XMultiComponentFactory> mrServiceManager = mXComponentContext->getServiceManager();
    mrXMimeCntFactory = Reference<XMimeContentTypeFactory>(mrServiceManager->createInstanceWithContext(
       OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.MimeContentTypeFactory")), mXComponentContext), UNO_QUERY);

  if (!mrXMimeCntFactory.is())
    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create com.sun.star.datatransfer.MimeContentTypeFactory")), NULL);
}

DataFlavor DataFlavorMapper::systemToOpenOfficeFlavor(NSString* systemDataFlavor) const
{
  DataFlavor oOOFlavor;

  for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      if ([systemDataFlavor caseInsensitiveCompare: flavorMap[i].SystemFlavor] == NSOrderedSame)
        {
          oOOFlavor.MimeType = OUString::createFromAscii(flavorMap[i].OOoFlavor);
          oOOFlavor.HumanPresentableName = OUString(RTL_CONSTASCII_USTRINGPARAM(flavorMap[i].HumanPresentableName));
          oOOFlavor.DataType = flavorMap[i].DataType;
          return oOOFlavor;
        }
    } // for

  return oOOFlavor;
}

NSString* DataFlavorMapper::openOfficeToSystemFlavor(const DataFlavor& oOOFlavor) const
{
  NSString* sysFlavor = NULL;

  for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      if (oOOFlavor.MimeType.compareToAscii(flavorMap[i].OOoFlavor, strlen(flavorMap[i].OOoFlavor)) == 0)
        {
          sysFlavor = flavorMap[i].SystemFlavor;
        }
    }

  return sysFlavor;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(NSString* systemFlavor, Reference<XTransferable> rTransferable) const
{
  DataProviderPtr_t dp;

  try
    {
      DataFlavor oOOFlavor = systemToOpenOfficeFlavor(systemFlavor);

      Any data = rTransferable->getTransferData(oOOFlavor);

      if (isByteSequenceType(data.getValueType()))
        {
          if ([systemFlavor caseInsensitiveCompare: NSHTMLPboardType] == NSOrderedSame)
            {
              dp = DataProviderPtr_t(new HTMLFormatDataProvider(data));
            }
          else if ([systemFlavor caseInsensitiveCompare: NSPICTPboardType] == NSOrderedSame)
            {
              dp = DataProviderPtr_t(new BMPDataProvider(data));
            }
          else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
            {
              dp = DataProviderPtr_t(new FileListDataProvider(data));
            }
          else
            {
              dp = DataProviderPtr_t(new ByteSequenceDataProvider(data));
            }
        }
      else // Must be OUString type
        {
          BOOST_ASSERT(isOUStringType(data.getValueType()));
          dp = DataProviderPtr_t(new UniDataProvider(data));
        }
    }
  catch(UnsupportedFlavorException&)
    {
      // Somebody violates the contract of the clipboard
      // interface @see XTransferable
    }

  return dp;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(const NSString* systemFlavor, NSArray* systemData) const
{
  return DataProviderPtr_t(new FileListDataProvider(systemData));
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(const NSString* systemFlavor, NSData* systemData) const
{
  DataProviderPtr_t dp;

  if ([systemFlavor caseInsensitiveCompare: NSStringPboardType] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new UniDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: NSHTMLPboardType] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new HTMLFormatDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: PBTYPE_OBJD] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new ObjDescDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: NSPICTPboardType] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new BMPDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
    {
      //dp = DataProviderPtr_t(new FileListDataProvider(systemData));
    }
  else
    {
      dp = DataProviderPtr_t(new ByteSequenceDataProvider(systemData));
    }

  return dp;
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

NSArray* DataFlavorMapper::flavorSequenceToTypesArray(const com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor>& flavors) const
{
  sal_uInt32 nFlavors = flavors.getLength();
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: 1];

  for (sal_uInt32 i = 0; i < nFlavors; i++)
    {
      NSString* str = openOfficeToSystemFlavor(flavors[i]);

      if (str != NULL)
        {
          [array addObject: str];
        }
    }

  return [array autorelease];
}

com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor> DataFlavorMapper::typesArrayToFlavorSequence(NSArray* types) const
{
  int nFormats = [types count];
  Sequence<DataFlavor> flavors;

  for (int i = 0; i < nFormats; i++)
    {
      NSString* sysFormat = [types objectAtIndex: i];
      DataFlavor oOOFlavor = systemToOpenOfficeFlavor(sysFormat);

      if (isValidFlavor(oOOFlavor))
        {
          flavors.realloc(flavors.getLength() + 1);
          flavors[flavors.getLength() - 1] = oOOFlavor;
        }
    }

  return flavors;
}


NSArray* DataFlavorMapper::getAllSupportedPboardTypes() const
{
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: SIZE_FLAVOR_MAP];

  for (sal_uInt32 i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      [array addObject: flavorMap[i].SystemFlavor];
    }

  return [array autorelease];
}
