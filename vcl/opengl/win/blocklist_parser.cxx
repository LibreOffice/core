/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "blocklist_parser.hxx"

WinBlocklistParser::WinBlocklistParser(const OUString& rURL,
        std::vector<wgl::DriverInfo>& rDriverList)
    : maURL(rURL)
    , mrDriverList(rDriverList)
    , meBlockType(BlockType::UNKNOWN)
{
}

void WinBlocklistParser::parse()
{
    xmlreader::XmlReader aReader(maURL);
    handleContent(aReader);
}

namespace {

wgl::OperatingSystem getOperatingSystem(const OString& rString)
{
    if (rString == "all")
    {
        return wgl::DRIVER_OS_ALL;
    }
    else if (rString == "xp")
    {
        return wgl::DRIVER_OS_WINDOWS_XP;
    }
    else if (rString == "server2003")
    {
        return wgl::DRIVER_OS_WINDOWS_SERVER_2003;
    }
    else if (rString == "vista")
    {
        return wgl::DRIVER_OS_WINDOWS_VISTA;
    }
    else if (rString == "7")
    {
        return wgl::DRIVER_OS_WINDOWS_7;
    }
    else if (rString == "8")
    {
        return wgl::DRIVER_OS_WINDOWS_8;
    }
    else if (rString == "8_1")
    {
        return wgl::DRIVER_OS_WINDOWS_8_1;
    }
    else if (rString == "10")
    {
        return wgl::DRIVER_OS_WINDOWS_10;
    }

    return wgl::DRIVER_OS_UNKNOWN;
}

wgl::VersionComparisonOp getComparison(const OString& rString)
{
    if (rString == "less")
    {
        return wgl::DRIVER_LESS_THAN;
    }
    else if (rString == "less_equal")
    {
        return wgl::DRIVER_LESS_THAN_OR_EQUAL;
    }
    else if (rString == "greater")
    {
        return wgl::DRIVER_GREATER_THAN;
    }
    else if (rString == "greater_equal")
    {
        return wgl::DRIVER_GREATER_THAN_OR_EQUAL;
    }
    else if (rString == "equal")
    {
        return wgl::DRIVER_EQUAL;
    }
    else if (rString == "not_equal")
    {
        return wgl::DRIVER_NOT_EQUAL;
    }
    else if (rString == "between_exclusive")
    {
        return wgl::DRIVER_BETWEEN_EXCLUSIVE;
    }
    else if (rString == "between_inclusive")
    {
        return wgl::DRIVER_BETWEEN_INCLUSIVE;
    }
    else if (rString == "between_inclusive_start")
    {
        return wgl::DRIVER_BETWEEN_INCLUSIVE_START;
    }

    throw InvalidFileException();
}

uint64_t getVersion(const OString& rString)
{
    OUString aString = OStringToOUString(rString, RTL_TEXTENCODING_UTF8);
    uint64_t nVersion;
    bool bResult = wgl::ParseDriverVersion(aString, nVersion);

    if (!bResult)
    {
        throw InvalidFileException();
    }

    return nVersion;
}

}

void WinBlocklistParser::handleDevice(wgl::DriverInfo& rDriver, xmlreader::XmlReader& rReader)
{
    if (meBlockType == BlockType::WHITELIST)
    {
        rDriver.mbWhitelisted = true;
    }
    else if (meBlockType == BlockType::BLACKLIST)
    {
        rDriver.mbWhitelisted = false;
    }
    else if (meBlockType == BlockType::UNKNOWN)
    {
        throw InvalidFileException();
    }

    xmlreader::Span name;
    int nsId;

    while (rReader.nextAttribute(&nsId, &name))
    {
        if (name.equals("os"))
        {
            xmlreader::Span name = rReader.getAttributeValue(false);
            OString sOS(name.begin, name.length);
            rDriver.meOperatingSystem = getOperatingSystem(sOS);
        }
        else if (name.equals("vendor"))
        {
            name = rReader.getAttributeValue(false);
            OString sVendor(name.begin, name.length);

            // TODO: moggi: check that only valid vendors are imported
            OUString aVendor = OStringToOUString(sVendor, RTL_TEXTENCODING_UTF8);
            rDriver.maAdapterVendor = aVendor;
        }
        else if (name.equals("compare"))
        {
            name = rReader.getAttributeValue(false);
            OString sCompare(name.begin, name.length);
            rDriver.meComparisonOp = getComparison(sCompare);
        }
        else if (name.equals("version"))
        {
            name = rReader.getAttributeValue(false);
            OString sVersion(name.begin, name.length);
            rDriver.mnDriverVersion = getVersion(sVersion);
        }
        else if (name.equals("minVersion"))
        {
            name = rReader.getAttributeValue(false);
            OString sMinVersion(name.begin, name.length);
            rDriver.mnDriverVersion = getVersion(sMinVersion);
        }
        else if (name.equals("maxVersion"))
        {
            name = rReader.getAttributeValue(false);
            OString sMaxVersion(name.begin, name.length);
            rDriver.mnDriverVersionMax = getVersion(sMaxVersion);
        }
        else
        {
            OString aAttrName(name.begin, name.length);
            SAL_WARN("vcl.opengl.win", "unsupported attribute: " << aAttrName);
        }
    }

    int nLevel = 1;
    bool bInMsg = false;
    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = rReader.nextItem(
                xmlreader::XmlReader::TEXT_NORMALIZED, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (nLevel > 2)
                throw InvalidFileException();

            if (name.equals("msg"))
            {
                bInMsg = true;
            }
            else if (name.equals("device"))
            {
                bool bSuccess = rReader.nextAttribute(&nsId, &name);
                if (!bSuccess || !name.equals("id"))
                    throw InvalidFileException();

                name = rReader.getAttributeValue(false);
                OString aDeviceId(name.begin, name.length);
                rDriver.maDevices.push_back(OStringToOUString(aDeviceId, RTL_TEXTENCODING_UTF8));
            }
            else
                throw InvalidFileException();
        }
        else if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
            bInMsg = false;
            if (!nLevel)
                break;
        }
        else if (res == xmlreader::XmlReader::RESULT_TEXT)
        {
            if (bInMsg)
            {
                OString sMsg(name.begin, name.length);
                rDriver.maMsg = OStringToOUString(sMsg, RTL_TEXTENCODING_UTF8);
            }
        }
    }
}

void WinBlocklistParser::handleList(xmlreader::XmlReader& rReader)
{
    xmlreader::Span name;
    int nsId;

    while (true)
    {
        xmlreader::XmlReader::Result res = rReader.nextItem(
                xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("entry"))
            {
                wgl::DriverInfo aDriver;
                handleDevice(aDriver, rReader);
                mrDriverList.push_back(aDriver);
            }
            else if (name.equals("entryRange"))
            {
                wgl::DriverInfo aDriver;
                handleDevice(aDriver, rReader);
                mrDriverList.push_back(aDriver);
            }
            else
            {
                throw InvalidFileException();
            }
        }
        else if (res == xmlreader::XmlReader::RESULT_END)
        {
            break;
        }
    }
}

void WinBlocklistParser::handleContent(xmlreader::XmlReader& rReader)
{
    while (true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = rReader.nextItem(
                xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("whitelist"))
            {
                meBlockType = BlockType::WHITELIST;
                handleList(rReader);
            }
            else if (name.equals("blacklist"))
            {
                meBlockType = BlockType::BLACKLIST;
                handleList(rReader);
            }
            else if (name.equals("root"))
            {
            }
            else
            {
                throw InvalidFileException();
            }
        }
        else if (res == xmlreader::XmlReader::RESULT_END)
        {
            if (name.equals("whitelist")
                    ||
                    name.equals("blacklist"))
            {
                meBlockType = BlockType::UNKNOWN;
            }
        }
        else if (res == xmlreader::XmlReader::RESULT_DONE)
        {
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
