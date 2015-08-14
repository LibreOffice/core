/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/ole/vbaexport.hxx>

#include <tools/stream.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <oox/helper/binaryoutputstream.hxx>
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

#define USE_UTF8_CODEPAGE 0
#if USE_UTF8_CODEPAGE
#define CODEPAGE_MS 65001
#define CODEPAGE RTL_TEXTENCODING_UTF8
#else
#define CODEPAGE_MS 1252
#define CODEPAGE RTL_TEXTENCODING_MS_1252
#endif

namespace {

void exportString(SvStream& rStrm, const OUString& rString)
{
    OString aStringCorrectCodepage = OUStringToOString(rString, CODEPAGE);
    rStrm.WriteOString(aStringCorrectCodepage);
}

void exportUTF16String(SvStream& rStrm, const OUString& rString)
{
    sal_Int32 n = rString.getLength();
    const sal_Unicode* pString = rString.getStr();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode character = pString[i];
        rStrm.WriteUnicode(character);
    }
}

}

VbaExport::VbaExport(css::uno::Reference<css::frame::XModel> xModel):
    mxModel(xModel)
{
    maProjectName = "How to get the correct project name?";
}

namespace {

// section 2.3.4.2.1.1
void writePROJECTSYSKIND(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0001); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000001); // SysKind, hard coded to 32-bin windows for now
}

// section 2.3.4.2.1.2
void writePROJECTLCID(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0002); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // Lcid
}

// section 2.3.4.2.1.3
void writePROJECTLCIDINVOKE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0014); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // LcidInvoke
}

// section 2.3.4.2.1.4
void writePROJECTCODEPAGE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0003); // id
    rStrm.WriteUInt32(0x00000002); // size
    rStrm.WriteUInt16(CODEPAGE_MS); // CodePage
}

//section 2.3.4.2.1.5
void writePROJECTNAME(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0004); // id
    sal_uInt32 sizeOfProjectName = 0x0000000a; // for project name "VBAProject"
    rStrm.WriteUInt32(sizeOfProjectName); // sizeOfProjectName
    exportString(rStrm, "VBAProject"); // ProjectName
}

//section 2.3.4.2.1.6
void writePROJECTDOCSTRING(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0005); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfDocString
    rStrm.WriteUInt16(0x0040); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfDocStringUnicode, MUST be even
}

//section 2.3.4.2.1.7
void writePROJECTHELPFILEPATH(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0006); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfHelpFile1
    rStrm.WriteUInt16(0x003D); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfHelpFile2
}

//section 2.3.4.2.1.8
void writePROJECTHELPCONTEXT(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0007); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000000); // HelpContext
}

//section 2.3.4.2.1.9
void writePROJECTLIBFLAGS(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0008); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000000); // ProjectLibFlags
}

//section 2.3.4.2.1.10
void writePROJECTVERSION(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0009); // id
    rStrm.WriteUInt32(0x00000004); // Reserved
    rStrm.WriteUInt32(1467127224); // VersionMajor // TODO: where is this magic number comming from
    rStrm.WriteUInt16(5); // VersionMinor // TODO: where is this magic number coming from
}

//section 2.3.4.2.1.11
void writePROJECTCONSTANTS(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x000C); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfConstants
    rStrm.WriteUInt16(0x003C); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfConstantsUnicode
}

// section 2.3.4.2.1
void writePROJECTINFORMATION(SvStream& rStrm)
{
    writePROJECTSYSKIND(rStrm);
    writePROJECTLCID(rStrm);
    writePROJECTLCIDINVOKE(rStrm);
    writePROJECTCODEPAGE(rStrm);
    writePROJECTNAME(rStrm);
    writePROJECTDOCSTRING(rStrm);
    writePROJECTHELPFILEPATH(rStrm);
    writePROJECTHELPCONTEXT(rStrm);
    writePROJECTLIBFLAGS(rStrm);
    writePROJECTVERSION(rStrm);
    writePROJECTCONSTANTS(rStrm);
}

// section 2.3.4.2.2.2
void writeREFERENCENAME(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0016); // id
    rStrm.WriteUInt32(6); // sizeOfName
    exportString(rStrm, "stdole"); // name
    rStrm.WriteUInt16(0x003E); // reserved
    rStrm.WriteUInt32(12); // sizeOfNameUnicode
    exportUTF16String(rStrm, "stdole"); // nameUnicode
}

// section 2.3.4.2.2.5
void writeREFERENCEREGISTERED(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x000D); // id
    rStrm.WriteUInt32(104); // size of sizeOfLibid, Libid, reserved 1 and reserved 2
    rStrm.WriteUInt32(94); // sizeOfLibid
    exportString(rStrm, "*\\G{00020430-0000-0000-C000-000000000046}#2.0#0#C:\\Windows\\SysWOW64\\stdole2.tlb#OLE Automation");
    rStrm.WriteUInt32(0x00000000); // reserved 1
    rStrm.WriteUInt16(0x0000); // reserved 2
}

// section 2.3.4.2.2.1
void writeREFERENCE(SvStream& rStrm)
{
    writeREFERENCENAME(rStrm);
    writeREFERENCEREGISTERED(rStrm);
}

// section 2.3.4.2.2
void writePROJECTREFERENCES(SvStream& rStrm)
{
    writeREFERENCE(rStrm);
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm)
{
    writePROJECTINFORMATION(rStrm);
    writePROJECTREFERENCES(rStrm);
}

}

void VbaExport::exportVBA()
{
    // start here with the VBA export
    const OUString aDirFileName("/tmp/vba_dir_out.bin");
    SvFileStream aDirStream(aDirFileName, StreamMode::WRITE);

    // export
    exportDirStream(aDirStream);
}

css::uno::Reference<css::container::XNameContainer> VbaExport::getBasicLibrary()
{
    css::uno::Reference<css::container::XNameContainer> xLibrary;
    try
    {
        oox::PropertySet aDocProp(mxModel);
        css::uno::Reference<css::script::XLibraryContainer> xLibContainer(aDocProp.getAnyProperty(oox::PROP_BasicLibraries), css::uno::UNO_QUERY_THROW);
        xLibrary.set( xLibContainer->getByName(maProjectName), css::uno::UNO_QUERY_THROW );
    }
    catch(...)
    {
    }

    return xLibrary;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */