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
void writeREFERENCENAME(SvStream& rStrm, const OUString name)
{
    rStrm.WriteUInt16(0x0016); // id
    sal_Int32 size = name.getLength();
    rStrm.WriteUInt32(size); // sizeOfName
    exportString(rStrm, name); // name
    rStrm.WriteUInt16(0x003E); // reserved
    sal_Int32 unicodesize = size * 2;
    rStrm.WriteUInt32(unicodesize); // sizeOfNameUnicode
    exportUTF16String(rStrm, name); // nameUnicode
}

// section 2.3.4.2.2.5
void writeREFERENCEREGISTERED(SvStream& rStrm, const OUString libid)
{
    rStrm.WriteUInt16(0x000D); // id
    sal_Int32 sizeOfLibid = libid.getLength();
    sal_Int32 size = sizeOfLibid + 10; // size of Libid, sizeOfLibid(4 bytes), reserved 1(4 bytes) and reserved 2(2 bytes)
    rStrm.WriteUInt32(size); // size
    rStrm.WriteUInt32(sizeOfLibid); // sizeOfLibid
    exportString(rStrm, libid); // Libid
    rStrm.WriteUInt32(0x00000000); // reserved 1
    rStrm.WriteUInt16(0x0000); // reserved 2
}

// section 2.3.4.2.2.1
void writeREFERENCE(SvStream& rStrm, const OUString name, const OUString libid)
{
    writeREFERENCENAME(rStrm, name);
    writeREFERENCEREGISTERED(rStrm, libid);
}

// section 2.3.4.2.2
void writePROJECTREFERENCES(SvStream& rStrm)
{
    writeREFERENCE(rStrm, "stdole", "*\\G{00020430-0000-0000-C000-000000000046}#2.0#0#C:\\Windows\\SysWOW64\\stdole2.tlb#OLE Automation");
    writeREFERENCE(rStrm, "Office", "*\\G{2DF8D04C-5BFA-101B-BDE5-00AA0044DE52}#2.0#0#C:\\Program Files (x86)\\Common Files\\Microsoft Shared\\OFFICE14\\MSO.DLL#Microsoft Office 14.0 Object Library");
}

// section 2.3.4.2.3.1
void writePROJECTCOOKIE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0013); // id
    rStrm.WriteUInt32(0x00000002); // size
    rStrm.WriteUInt16(0xFFFF); // cookie
}

// section 2.3.4.2.3.2.1
void writeMODULENAME(SvStream& rStrm, const OUString name)
{
    rStrm.WriteUInt16(0x0019); // id
    sal_Int32 n = name.getLength(); // sizeOfModuleName
    rStrm.WriteUInt32(n);
    exportString(rStrm, name); // ModuleName
}

// section 2.3.4.2.3.2.2
void writeMODULENAMEUNICODE(SvStream& rStrm, const OUString name)
{
    rStrm.WriteUInt16(0x0047); // id
    sal_Int32 n = name.getLength() * 2; // sizeOfModuleNameUnicode
    rStrm.WriteUInt32(n);
    exportUTF16String(rStrm, name); // ModuleNameUnicode
}

// section 2.3.4.2.3.2.3
void writeMODULESTREAMNAME(SvStream& rStrm, const OUString streamName)
{
    rStrm.WriteUInt16(0x001A); // id
    sal_Int32 n = streamName.getLength(); // sizeOfStreamName
    rStrm.WriteUInt32(n);
    exportString(rStrm, streamName); // StreamName
    rStrm.WriteUInt16(0x0032); // reserved
    rStrm.WriteUInt32(n * 2); // sizeOfStreamNameUnicode
    exportUTF16String(rStrm, streamName); // StreamNameUnicode
}

// section 2.3.4.2.3.2.4
void writeMODULEDOCSTRING(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x001C); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfDocString
    rStrm.WriteUInt16(0x0048); // reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfDocStringUnicode
}

// section 2.3.4.2.3.2.5
void writeMODULEOFFSET(SvStream& rStrm, sal_uInt32 offset)
{
    rStrm.WriteUInt16(0x0031); // id
    rStrm.WriteUInt32(0x00000004); // sizeOfTextOffset
    rStrm.WriteUInt32(offset); // TextOffset
}

// section 2.3.4.2.3.2.6
void writeMODULEHELPCONTEXT(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x001E); // id
    rStrm.WriteUInt32(0x00000004); // sizeOfHelpContext
    rStrm.WriteUInt32(0x00000000); // HelpContext
}

// section 2.3.4.2.3.2.7
void writeMODULECOOKIE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x002C); // id
    rStrm.WriteUInt32(0x00000002); // sizeOfHelpContext
    rStrm.WriteUInt16(0xFFFF); // HelpContext
}

// section 2.3.4.2.3.2.8
void writeMODULETYPE(SvStream& rStrm, const OUString type)
{
    if(type == "procedure")
        rStrm.WriteUInt16(0x0021); // id for a procedural module
    else
        rStrm.WriteUInt16(0x0022); // id for document, class or design module
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3.2
void writePROJECTMODULE(SvStream& rStrm, const OUString name, const OUString streamName, sal_uInt32 offset, const OUString type)
{
    writeMODULENAME(rStrm, name);
    writeMODULENAMEUNICODE(rStrm, name);
    writeMODULESTREAMNAME(rStrm, streamName);
    writeMODULEDOCSTRING(rStrm);
    writeMODULEOFFSET(rStrm, offset);
    writeMODULEHELPCONTEXT(rStrm);
    writeMODULECOOKIE(rStrm);
    writeMODULETYPE(rStrm, type);
    rStrm.WriteUInt16(0x002B); // terminator
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3
void writePROJECTMODULES(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x000F); // id
    rStrm.WriteUInt32(0x00000002); // size of Count
    sal_Int16 count = 5; // Number of modules
    rStrm.WriteUInt16(count); // Count
    writePROJECTCOOKIE(rStrm);
    writePROJECTMODULE(rStrm, "Module1", "Module1", 0x00000379, "procedure");
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm)
{
    writePROJECTINFORMATION(rStrm);
    writePROJECTREFERENCES(rStrm);
    writePROJECTMODULES(rStrm);
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