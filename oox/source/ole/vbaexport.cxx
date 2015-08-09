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

VbaExport::VbaExport(css::uno::Reference<css::frame::XModel> xModel):
    mxModel(xModel)
{
    maProjectName = "How to get the correct project name?";
}

namespace {

// section 2.3.4.2.1.3
void writePROJECTLCIDINVOKE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0014); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // LcidInvoke
}

// section 2.3.4.2.1.2
void writePROJECTLCID(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0002); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // Lcid
}

// section 2.3.4.2.1.1
void writePROJECTSYSKIND(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0001); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000002); // SysKind, hard coded to 32-bin windows for now
}

// section 2.3.4.2.1
void writePROJECTINFORMATION(SvStream& rStrm)
{
    writePROJECTSYSKIND(rStrm);
    writePROJECTLCID(rStrm);
    writePROJECTLCIDINVOKE(rStrm);
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm)
{
    writePROJECTINFORMATION(rStrm);
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
