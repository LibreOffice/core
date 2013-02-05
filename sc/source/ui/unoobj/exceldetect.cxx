/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "exceldetect.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include "svl/itemset.hxx"
#include "svl/eitem.hxx"
#include "sfx2/app.hxx"
#include "sfx2/docfile.hxx"
#include "sfx2/sfxsids.hrc"
#include "comphelper/mediadescriptor.hxx"
#include "sot/storage.hxx"

using namespace com::sun::star;
using comphelper::MediaDescriptor;

ScExcelBiffDetect::ScExcelBiffDetect( const uno::Reference<uno::XComponentContext>& /*xContext*/ ) {}
ScExcelBiffDetect::~ScExcelBiffDetect() {}

OUString ScExcelBiffDetect::getImplementationName() throw (uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

sal_Bool ScExcelBiffDetect::supportsService( const OUString& aName ) throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aSrvNames = getSupportedServiceNames();
    const OUString* pArray = aSrvNames.getConstArray();
    for (sal_Int32 i = 0; i < aSrvNames.getLength(); ++i, ++pArray)
    {
        if (*pArray == aName)
            return true;
    }
    return false;
}

uno::Sequence<OUString> ScExcelBiffDetect::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

namespace {

bool hasStream(const uno::Reference<io::XInputStream>& xInStream, const OUString& rName)
{
    SfxMedium aMedium;
    aMedium.UseInteractionHandler(true);
    aMedium.setStreamToLoadFrom(xInStream, true);
    SvStream* pStream = aMedium.GetInStream();
    if (!pStream)
        return false;

    pStream->Seek(STREAM_SEEK_TO_END);
    sal_Size nSize = pStream->Tell();
    pStream->Seek(0);

    if (!nSize)
        // 0-size stream.  Failed.
        return false;

    SotStorageRef xStorage = new SotStorage(pStream, false);
    if (!xStorage.Is() || xStorage->GetError())
        return false;

    return xStorage->IsStream(rName);
}

bool isExcel40(const uno::Reference<io::XInputStream>& xInStream)
{
    SfxMedium aMedium;
    aMedium.UseInteractionHandler(true);
    aMedium.setStreamToLoadFrom(xInStream, true);
    SvStream* pStream = aMedium.GetInStream();
    if (!pStream)
        return false;

    pStream->Seek(STREAM_SEEK_TO_END);
    sal_Size nSize = pStream->Tell();
    pStream->Seek(0);

    if (nSize < 4)
        return false;

    sal_uInt16 nBofId, nBofSize;
    *pStream >> nBofId >> nBofSize;

    if (nBofId != 0x0409)
        // This ID signifies Excel 4.0 format.  It must be 0x0409.
        return false;

    if (nBofSize < 4 || 16 < nBofSize)
        // BOF record must be sized between 4 and 16 for Excel 4.0 stream.
        return false;

    sal_Size nPos = pStream->Tell();
    if (nSize - nPos < nBofSize)
        // BOF record doesn't have required bytes.
        return false;

    return true;
}

bool isTemplate(const OUString& rType)
{
    return rType.indexOf("_VorlageTemplate") != -1;
}

}

OUString ScExcelBiffDetect::detect( uno::Sequence<beans::PropertyValue>& lDescriptor )
    throw (uno::RuntimeException)
{
    MediaDescriptor aMediaDesc(lDescriptor);
    OUString aType;
    aMediaDesc[MediaDescriptor::PROP_TYPENAME()] >>= aType;
    if (aType.isEmpty())
        // Type is not given.  We can't proceed.
        return OUString();

    aMediaDesc.addInputStream();
    uno::Reference<io::XInputStream> xInStream(aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY);
    if (!xInStream.is())
        // No input stream.
        return OUString();

    if (aType == "calc_MS_Excel_97" || aType == "calc_MS_Excel_97_VorlageTemplate")
    {
        // See if this stream is a Excel 97/XP/2003 (BIFF8) stream.
        if (!hasStream(xInStream, "Workbook"))
            // BIFF8 is expected to contain a stream named "Workbook".
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 97 Vorlage/Template") : OUString("MS Excel 97");
        return aType;
    }

    if (aType == "calc_MS_Excel_95" || aType == "calc_MS_Excel_95_VorlageTemplate")
    {
        // See if this stream is a Excel 95 (BIFF5) stream.
        if (!hasStream(xInStream, "Book"))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 95 Vorlage/Template") : OUString("MS Excel 95");
        return aType;
    }

    if (aType == "calc_MS_Excel_5095" || aType == "calc_MS_Excel_5095_VorlageTemplate")
    {
        // See if this stream is a Excel 5.0/95 stream.
        if (!hasStream(xInStream, "Book"))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 5.0/95 Vorlage/Template") : OUString("MS Excel 5.0/95");
        return aType;
    }

    if (aType == "calc_MS_Excel_40" || aType == "calc_MS_Excel_40_VorlageTemplate")
    {
        // See if this stream is a Excel 4.0 stream.
        if (!isExcel40(xInStream))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 4.0 Vorlage/Template") : OUString("MS Excel 4.0");
        return aType;
    }

    // failed!
    return OUString();
}

uno::Sequence<OUString> ScExcelBiffDetect::impl_getStaticSupportedServiceNames()
{
    uno::Sequence<OUString> aNames(1);
    aNames[0] = "com.sun.star.frame.ExtendedTypeDetection";
    return aNames;
}

OUString ScExcelBiffDetect::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.calc.ExcelBiffFormatDetector");
}

uno::Reference<uno::XInterface> ScExcelBiffDetect::impl_createInstance(
    const uno::Reference<uno::XComponentContext>& xContext )
        throw (com::sun::star::uno::Exception)
{
    return static_cast<cppu::OWeakObject*>(new ScExcelBiffDetect(xContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
