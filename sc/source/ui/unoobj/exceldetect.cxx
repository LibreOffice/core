/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "exceldetect.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "svl/itemset.hxx"
#include "svl/eitem.hxx"
#include "sfx2/app.hxx"
#include "sfx2/docfile.hxx"
#include "sfx2/sfxsids.hrc"
#include "unotools/mediadescriptor.hxx"
#include "sot/storage.hxx"

using namespace com::sun::star;
using utl::MediaDescriptor;

ScExcelBiffDetect::ScExcelBiffDetect( const uno::Reference<uno::XComponentContext>& /*xContext*/ ) {}
ScExcelBiffDetect::~ScExcelBiffDetect() {}

OUString ScExcelBiffDetect::getImplementationName() throw (uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

sal_Bool ScExcelBiffDetect::supportsService( const OUString& aName ) throw (uno::RuntimeException)
{
    return cppu::supportsService(this, aName);
}

uno::Sequence<OUString> ScExcelBiffDetect::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

namespace {

bool hasStream(const uno::Reference<io::XInputStream>& xInStream, const OUString& rName)
{
    SfxMedium aMedium;
    aMedium.UseInteractionHandler(false);
    aMedium.setStreamToLoadFrom(xInStream, true);
    SvStream* pStream = aMedium.GetInStream();
    if (!pStream)
        return false;

    pStream->Seek(STREAM_SEEK_TO_END);
    sal_Size nSize = pStream->Tell();
    pStream->Seek(0);

    if (!nSize)
        
        return false;

    SotStorageRef xStorage = new SotStorage(pStream, false);
    if (!xStorage.Is() || xStorage->GetError())
        return false;

    return xStorage->IsStream(rName);
}

/**
 * We detect BIFF 2, 3 and 4 file types together since the only thing that
 * set them apart is the BOF ID.
 */
bool isExcel40(const uno::Reference<io::XInputStream>& xInStream)
{
    SfxMedium aMedium;
    aMedium.UseInteractionHandler(false);
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
    pStream->ReadUInt16( nBofId ).ReadUInt16( nBofSize );

    switch (nBofId)
    {
        case 0x0009: 
        case 0x0209: 
        case 0x0409: 
        case 0x0809: 
            break;
        default:
            return false;
    }

    if (nBofSize < 4 || 16 < nBofSize)
        
        return false;

    sal_Size nPos = pStream->Tell();
    if (nSize - nPos < nBofSize)
        
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
        
        return OUString();

    aMediaDesc.addInputStream();
    uno::Reference<io::XInputStream> xInStream(aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY);
    if (!xInStream.is())
        
        return OUString();

    if (aType == "calc_MS_Excel_97" || aType == "calc_MS_Excel_97_VorlageTemplate")
    {
        
        if (!hasStream(xInStream, "Workbook"))
            
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 97 Vorlage/Template") : OUString("MS Excel 97");
        return aType;
    }

    if (aType == "calc_MS_Excel_95" || aType == "calc_MS_Excel_95_VorlageTemplate")
    {
        
        if (!hasStream(xInStream, "Book"))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 95 Vorlage/Template") : OUString("MS Excel 95");
        return aType;
    }

    if (aType == "calc_MS_Excel_5095" || aType == "calc_MS_Excel_5095_VorlageTemplate")
    {
        
        if (!hasStream(xInStream, "Book"))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 5.0/95 Vorlage/Template") : OUString("MS Excel 5.0/95");
        return aType;
    }

    if (aType == "calc_MS_Excel_40" || aType == "calc_MS_Excel_40_VorlageTemplate")
    {
        
        if (!isExcel40(xInStream))
            return OUString();

        aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= isTemplate(aType) ? OUString("MS Excel 4.0 Vorlage/Template") : OUString("MS Excel 4.0");
        return aType;
    }

    
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
