/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <sal/log.hxx>

#include "DataFlavorMapping.hxx"
#include "HtmlFmtFlt.hxx"
#include "PictToBmpFlt.hxx"
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/ustring.hxx>
#include <osl/endian.h>

#include <cassert>
#include <string.h>
#include <string_view>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace std;

namespace
{
  /* Determine whether or not a DataFlavor is valid.
   */
  bool isValidFlavor(const DataFlavor& aFlavor)
  {
    size_t len = aFlavor.MimeType.getLength();
    Type dtype = aFlavor.DataType;
    return ((len > 0) && ((dtype == cppu::UnoType<Sequence<sal_Int8>>::get()) || (dtype == cppu::UnoType<OUString>::get())));
  }

  OUString NSStringToOUString( const NSString* cfString)
  {
    assert(cfString && "Invalid parameter");

    const char* utf8Str = [cfString UTF8String];
    unsigned int len = rtl_str_getLength(utf8Str);

    return OUString(utf8Str, len, RTL_TEXTENCODING_UTF8);
  }

  NSString* OUStringToNSString(std::u16string_view ustring)
  {
    OString utf8Str = OUStringToOString(ustring, RTL_TEXTENCODING_UTF8);
    return [NSString stringWithCString: utf8Str.getStr() encoding: NSUTF8StringEncoding];
  }

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

  const char* FLAVOR_DUMMY_INTERNAL = "application/x-openoffice-internal";

  struct FlavorMap
  {
    const NSString* SystemFlavor;
    const char* OOoFlavor;
    const char* HumanPresentableName;
    bool DataTypeOUString; // sequence<byte> otherwise
  };

  // The SystemFlavor member is nil for the cases where there is no predefined pasteboard type UTI
  // and we use the internal MIME type (media type) also on the pasteboard. That is OK in macOS,
  // there is no requirement that the types are well-formed UTIs. It is different on iOS, I think,
  // though. For an introduction to UTIs, see for instance
  // https://alastairs-place.net/blog/2012/06/06/utis-are-better-than-you-think-and-heres-why/
  //
  // In those cases the MIME type might actually have parameters appended, separated by semicolons.
  // At least the FLAVOR_SODX one must have at least a typename="%PRODUCTNAME %PRODUCTVERSION
  // Spreadsheet" parameter (with macros expanded and translated) for LO to recognise it. See
  // lcl_TestFormat() in sc/source/ui/view/cellsh.cxx.

  const FlavorMap flavorMap[] =
    {
      { NSPasteboardTypeString, "text/plain;charset=utf-16", "Unicode Text (UTF-16)", true },
      { NSPasteboardTypeRTF, "text/rtf", "Rich Text Format", false },
      { NSPasteboardTypePDF, "application/pdf", "PDF File", false },
      { NSPasteboardTypeTIFF, "image/png", "Portable Network Graphics", false },
      { NSPasteboardTypeHTML, "text/html", "Plain Html", false },
SAL_WNODEPRECATED_DECLARATIONS_PUSH
          // "'NSFilenamesPboardType' is deprecated: first deprecated in macOS 10.14 - Create
          // multiple pasteboard items with NSPasteboardTypeFileURL or kUTTypeFileURL instead"
      { NSFilenamesPboardType, "application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", false },
SAL_WNODEPRECATED_DECLARATIONS_POP
      { nil, FLAVOR_SESX, "Star Embed Source (XML)", false },
      { nil, FLAVOR_SLSDX, "Star Link Source Descriptor (XML)", false },
      { nil, FLAVOR_ESX, "Star Embed Source (XML)", false },
      { nil, FLAVOR_LSX, "Star Link Source (XML)", false },
      { nil, FLAVOR_EOX, "Star Embedded Object (XML)", false },
      { nil, FLAVOR_SVXB, "SVXB (StarView Bitmap/Animation", false },
      { nil, FLAVOR_GDIMF, "GDIMetaFile", false },
      { nil, FLAVOR_WMF, "Windows MetaFile", false },
      { nil, FLAVOR_EMF, "Windows Enhanced MetaFile", false },
      { nil, FLAVOR_SODX, "Star Object Descriptor (XML)", false },
      { nil, FLAVOR_DUMMY_INTERNAL, "internal data",false }
    };

  #define SIZE_FLAVOR_MAP (sizeof(flavorMap)/sizeof(FlavorMap))

  bool isByteSequenceType(const Type& theType)
  {
    return (theType == cppu::UnoType<Sequence<sal_Int8>>::get());
  }

  bool isOUStringType(const Type& theType)
  {
    return (theType == cppu::UnoType<OUString>::get() );
  }

/* A base class for other data provider.
 */
class DataProviderBaseImpl : public DataProvider
{
public:
  DataProviderBaseImpl(const Any& data);
  DataProviderBaseImpl(id data);
  virtual ~DataProviderBaseImpl() override;

protected:
  Any mData;
  //NSData* mSystemData;
  id mSystemData;
};

} // unnamed namespace

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

namespace {

class UniDataProvider : public DataProviderBaseImpl
{
public:
  UniDataProvider(const Any& data);

  UniDataProvider(NSData* data);

  virtual NSData* getSystemData() override;

  virtual Any getOOoData() override;
};

}

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
      oOOData <<= OUString(static_cast<const char*>([mSystemData bytes]),
                                 [mSystemData length],
                                 RTL_TEXTENCODING_UTF8);
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

namespace {

class ByteSequenceDataProvider : public DataProviderBaseImpl
{
public:
  ByteSequenceDataProvider(const Any& data);

  ByteSequenceDataProvider(NSData* data);

  virtual NSData* getSystemData() override;

  virtual Any getOOoData() override;
};

}

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
      oOOData <<= byteSequence;
    }
  else
    {
      oOOData =  mData;
    }

  return oOOData;
}

namespace {

class HTMLFormatDataProvider : public DataProviderBaseImpl
{
public:
  HTMLFormatDataProvider(NSData* data);

  virtual NSData* getSystemData() override;

  virtual Any getOOoData() override;
};

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

      oOOData <<= *pPlainHtml;
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

namespace {


class PNGDataProvider : public DataProviderBaseImpl
{
    NSBitmapImageFileType meImageType;
public:
    PNGDataProvider( const Any&, NSBitmapImageFileType);

    PNGDataProvider( NSData*, NSBitmapImageFileType);

    virtual NSData* getSystemData() override;

    virtual Any getOOoData() override;
};

}

PNGDataProvider::PNGDataProvider( const Any& data, NSBitmapImageFileType eImageType) :
  DataProviderBaseImpl(data),
  meImageType( eImageType )
{
}

PNGDataProvider::PNGDataProvider( NSData* data, NSBitmapImageFileType eImageType) :
  DataProviderBaseImpl(data),
  meImageType( eImageType )
{
}

NSData* PNGDataProvider::getSystemData()
{
    Sequence<sal_Int8> pngData;
    mData >>= pngData;

    Sequence<sal_Int8> imgData;
    NSData* sysData = nullptr;
    if( PNGToImage( pngData, imgData, meImageType))
        sysData = [NSData dataWithBytes: imgData.getArray() length: imgData.getLength()];

    return sysData;
}

/* The AOO 'PCT' filter is not yet good enough to be used
   and there is no flavor defined for exchanging 'PCT' with AOO
   so we convert 'PCT' to a PNG and provide this to AOO
*/
Any PNGDataProvider::getOOoData()
{
    Any oOOData;

    if( mSystemData)
    {
        const unsigned int flavorDataLength = [mSystemData length];
        Sequence<sal_Int8> imgData( flavorDataLength);
        memcpy( imgData.getArray(), [mSystemData bytes], flavorDataLength);

        Sequence<sal_Int8> pngData;
        if( ImageToPNG( imgData, pngData))
            oOOData <<= pngData;
    }
    else
    {
        oOOData = mData;
    }

    return oOOData;
}

namespace {

class FileListDataProvider : public DataProviderBaseImpl
{
public:
  FileListDataProvider(const Any& data);
  FileListDataProvider(NSArray* data);

  virtual NSData* getSystemData() override;
  virtual Any getOOoData() override;
};

}

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
      memset(pBuffer, 0, lenSeqRequired);

      for (size_t i = 0; i < length; i++)
        {
          NSString* fname = [mSystemData objectAtIndex: i];
          [fname getCharacters: pBuffer];
          size_t l = [fname length];
          pBuffer += l + 1;
        }

      oOOData <<= oOOFileList;
    }
  else
    {
      oOOData = mData;
    }

  return oOOData;
}

DataFlavorMapper::DataFlavorMapper()
{
    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
    mrXMimeCntFactory = MimeContentTypeFactory::create( xContext );
}

DataFlavorMapper::~DataFlavorMapper()
{
    // release potential NSStrings
    for( OfficeOnlyTypes::iterator it = maOfficeOnlyTypes.begin(); it != maOfficeOnlyTypes.end(); ++it )
    {
        [it->second release];
        it->second = nil;
    }
}

DataFlavor DataFlavorMapper::systemToOpenOfficeFlavor( const NSString* systemDataFlavor) const
{
    DataFlavor oOOFlavor;

    for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
        if ((flavorMap[i].SystemFlavor == nil && ([systemDataFlavor isEqualToString:[NSString stringWithUTF8String:flavorMap[i].OOoFlavor]]
                                                  ||
                                                  [systemDataFlavor hasPrefix:[[NSString stringWithUTF8String:flavorMap[i].OOoFlavor] stringByAppendingString:@";"]]))
            ||
            (flavorMap[i].SystemFlavor != nil && [systemDataFlavor isEqualToString:const_cast<NSString*>(flavorMap[i].SystemFlavor)]))
        {
          if (flavorMap[i].SystemFlavor == nil)
              oOOFlavor.MimeType = NSStringToOUString(systemDataFlavor);
          else
              oOOFlavor.MimeType = OUString::createFromAscii(flavorMap[i].OOoFlavor);
          oOOFlavor.HumanPresentableName = OUString::createFromAscii(flavorMap[i].HumanPresentableName);
          oOOFlavor.DataType = flavorMap[i].DataTypeOUString ? cppu::UnoType<OUString>::get() : cppu::UnoType<Sequence<sal_Int8>>::get();
          return oOOFlavor;
        }
    } // for

    // look if this might be an internal type; if it comes in here it must have
    // been through openOfficeToSystemFlavor before, so it should then be in the map
    OUString aTryFlavor( NSStringToOUString( systemDataFlavor ) );
    if( maOfficeOnlyTypes.find( aTryFlavor ) != maOfficeOnlyTypes.end() )
    {
        oOOFlavor.MimeType = aTryFlavor;
        oOOFlavor.HumanPresentableName.clear();
        oOOFlavor.DataType = cppu::UnoType<Sequence<sal_Int8>>::get();
    }

    return oOOFlavor;
}

const NSString* DataFlavorMapper::openOfficeToSystemFlavor( const DataFlavor& oOOFlavor, bool& rbInternal) const
{
    const NSString* sysFlavor = nullptr;
    rbInternal = false;

    for( size_t i = 0; i < SIZE_FLAVOR_MAP; ++i )
    {
       if (oOOFlavor.MimeType.startsWith(OUString::createFromAscii(flavorMap[i].OOoFlavor)))
        {
            if (flavorMap[i].SystemFlavor != nil)
                sysFlavor = flavorMap[i].SystemFlavor;
            else
                sysFlavor = OUStringToNSString(oOOFlavor.MimeType);
        }
    }

    if(!sysFlavor)
    {
        rbInternal = true;
        OfficeOnlyTypes::const_iterator it = maOfficeOnlyTypes.find( oOOFlavor.MimeType );

        if( it == maOfficeOnlyTypes.end() )
            sysFlavor = maOfficeOnlyTypes[ oOOFlavor.MimeType ] = OUStringToNSString( oOOFlavor.MimeType );
        else
            sysFlavor = it->second;
    }

    return sysFlavor;
}

NSString* DataFlavorMapper::openOfficeImageToSystemFlavor(NSPasteboard* pPasteboard)
{
    NSArray *supportedTypes = [NSArray arrayWithObjects: NSPasteboardTypeTIFF, nil];
    NSString *sysFlavor = [pPasteboard availableTypeFromArray:supportedTypes];
    return sysFlavor;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider( const NSString* systemFlavor, Reference<XTransferable> const & rTransferable) const
{
  DataProviderPtr_t dp;

  try
    {
      DataFlavor oOOFlavor = systemToOpenOfficeFlavor(systemFlavor);

      Any data = rTransferable->getTransferData(oOOFlavor);

      if (isByteSequenceType(data.getValueType()))
        {
          /*
             the HTMLFormatDataProvider prepends segment information to HTML
             this is useful for exchange with MS Word (which brings this stuff from Windows)
             but annoying for other applications. Since this extension is not a standard datatype
             on the Mac, let us not provide but provide normal HTML

          if ([systemFlavor caseInsensitiveCompare: NSHTMLPboardType] == NSOrderedSame)
            {
              dp = DataProviderPtr_t(new HTMLFormatDataProvider(data));
            }
          else
          */
          if ([systemFlavor caseInsensitiveCompare: NSPasteboardTypeTIFF] == NSOrderedSame)
            {
              dp = DataProviderPtr_t( new PNGDataProvider( data, NSBitmapImageFileTypeTIFF));
            }
SAL_WNODEPRECATED_DECLARATIONS_PUSH
              // "'NSFilenamesPboardType' is deprecated: first deprecated in macOS 10.14 - Create
              // multiple pasteboard items with NSPasteboardTypeFileURL or kUTTypeFileURL instead"
          else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
SAL_WNODEPRECATED_DECLARATIONS_POP
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
          SAL_WARN_IF(
              !isOUStringType(data.getValueType()), "vcl",
              "must be OUString type");
          dp = DataProviderPtr_t(new UniDataProvider(data));
        }
    }
  catch( const UnsupportedFlavorException& e )
    {
     SAL_WARN( "vcl.osx.clipboard", "DataFlavorMapper::getDataProvider(): Exception: " << e.Message );
      // Somebody violates the contract of the clipboard
      // interface @see XTransferable
    }

  return dp;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider( const NSString* /*systemFlavor*/, NSArray* systemData)
{
  return DataProviderPtr_t(new FileListDataProvider(systemData));
}

DataProviderPtr_t DataFlavorMapper::getDataProvider( const NSString* systemFlavor, NSData* systemData)
{
  DataProviderPtr_t dp;

  if ([systemFlavor caseInsensitiveCompare: NSPasteboardTypeString] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new UniDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: NSPasteboardTypeHTML] == NSOrderedSame)
    {
      dp = DataProviderPtr_t(new HTMLFormatDataProvider(systemData));
    }
  else if ([systemFlavor caseInsensitiveCompare: NSPasteboardTypeTIFF] == NSOrderedSame)
    {
      dp = DataProviderPtr_t( new PNGDataProvider(systemData, NSBitmapImageFileTypeTIFF));
    }
SAL_WNODEPRECATED_DECLARATIONS_PUSH
      // "'NSFilenamesPboardType' is deprecated: first deprecated in macOS 10.14 - Create multiple
      // pasteboard items with NSPasteboardTypeFileURL or kUTTypeFileURL instead"
  else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
SAL_WNODEPRECATED_DECLARATIONS_POP
    {
      //dp = DataProviderPtr_t(new FileListDataProvider(systemData));
    }
  else
    {
      dp = DataProviderPtr_t(new ByteSequenceDataProvider(systemData));
    }

  return dp;
}

bool DataFlavorMapper::isValidMimeContentType(const OUString& contentType) const
{
  bool result = true;

  try
    {
      Reference<XMimeContentType> xCntType(mrXMimeCntFactory->createMimeContentType(contentType));
    }
  catch( const IllegalArgumentException& e )
    {
      SAL_WARN("vcl.osx.clipboard", "DataFlavorMapper::isValidMimeContentType(): Exception: " << e.Message);
      result = false;
    }

  return result;
}

NSArray* DataFlavorMapper::flavorSequenceToTypesArray(const css::uno::Sequence<css::datatransfer::DataFlavor>& flavors) const
{
  sal_uInt32 nFlavors = flavors.getLength();
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: 1];

  bool bNeedDummyInternalFlavor(false);

  for (sal_uInt32 i = 0; i < nFlavors; i++)
  {
      if( flavors[i].MimeType.startsWith("image/bmp") )
      {
          [array addObject: NSPasteboardTypeTIFF];
      }
      else
      {
          const NSString* str = openOfficeToSystemFlavor(flavors[i], bNeedDummyInternalFlavor);

          if (str != nullptr)
          {
              [str retain];
              [array addObject: str];
          }
      }
  }

   // #i89462# #i90747#
   // in case no system flavor was found to report
   // report at least one so D&D between OOo targets works
  if( [array count] == 0 || bNeedDummyInternalFlavor)
  {
      [array addObject: [NSString stringWithUTF8String: FLAVOR_DUMMY_INTERNAL]];
  }

  return [array autorelease];
}

css::uno::Sequence<css::datatransfer::DataFlavor> DataFlavorMapper::typesArrayToFlavorSequence(NSArray* types) const
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

NSArray* DataFlavorMapper::getAllSupportedPboardTypes()
{
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: SIZE_FLAVOR_MAP];

  for (sal_uInt32 i = 0; i < SIZE_FLAVOR_MAP; i++)
    {
      if (flavorMap[i].SystemFlavor != nil)
          [array addObject: flavorMap[i].SystemFlavor];
      else
          [array addObject: [NSString stringWithUTF8String: flavorMap[i].OOoFlavor]];
    }

  return [array autorelease];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
