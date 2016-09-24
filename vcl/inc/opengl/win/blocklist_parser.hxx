/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/dllapi.h>
#include <xmlreader/xmlreader.hxx>
#include <vector>
#include "opengl/win/WinDeviceInfo.hxx"

class InvalidFileException
{
};

class VCL_DLLPUBLIC WinBlocklistParser
{
public:
    WinBlocklistParser(const OUString& rURL, std::vector<wgl::DriverInfo>& rDriverList);
    void parse();

private:
    void handleEntry(wgl::DriverInfo& rDriver, xmlreader::XmlReader& rReader);
    void handleDevices(wgl::DriverInfo& rDriver, xmlreader::XmlReader& rReader);
    void handleList(xmlreader::XmlReader& rReader);
    void handleContent(xmlreader::XmlReader& rReader);

    enum class BlockType
    {
        WHITELIST,
        BLACKLIST,
        UNKNOWN
    };

    BlockType meBlockType;
    std::vector<wgl::DriverInfo>& mrDriverList;
    OUString maURL;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
