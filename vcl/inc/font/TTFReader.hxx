/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <font/TTFStructure.hxx>
#include <vcl/font/FontDataContainer.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

namespace font
{
/** Handles reading of the name table */
class NameTableHandler
{
private:
    FontDataContainer const& mrFontDataContainer;

    const TableDirectoryEntry* mpTableDirectoryEntry;
    const char* mpNameTablePointer;
    const NameTable* mpNameTable;

    const char* getTablePointer(const TableDirectoryEntry* pEntry, size_t nEntrySize)
    {
        size_t nSize = mrFontDataContainer.size();
        if (pEntry->offset > nSize)
        {
            SAL_WARN("vcl.fonts", "Table offset beyond end of available data");
            return nullptr;
        }
        if (nEntrySize > nSize - pEntry->offset)
        {
            SAL_WARN("vcl.fonts", "Insufficient available data for table entry");
            return nullptr;
        }
        return mrFontDataContainer.getPointer() + pEntry->offset;
    }

public:
    NameTableHandler(FontDataContainer const& rFontDataContainer,
                     const TableDirectoryEntry* pTableDirectoryEntry)
        : mrFontDataContainer(rFontDataContainer)
        , mpTableDirectoryEntry(pTableDirectoryEntry)
        , mpNameTablePointer(getTablePointer(mpTableDirectoryEntry, sizeof(NameTable)))
        , mpNameTable(reinterpret_cast<const NameTable*>(mpNameTablePointer))
    {
    }

    sal_uInt32 getTableOffset() { return mpTableDirectoryEntry->offset; }

    const NameTable* getNameTable() { return mpNameTable; }

    /** Number of tables */
    sal_uInt16 getNumberOfRecords() { return mpNameTable ? mpNameTable->nCount : 0; }

    /** Get a name table record for index */
    const NameRecord* getNameRecord(sal_uInt32 index)
    {
        const char* pPointer = mpNameTablePointer + sizeof(NameTable);
        pPointer += sizeof(NameRecord) * index;
        return reinterpret_cast<const NameRecord*>(pPointer);
    }

    /** Get offset to english unicode string
     *
     * See: https://learn.microsoft.com/en-us/typography/opentype/spec/name#name-ids
     */
    bool findEnglishUnicodeNameOffset(font::NameID eNameID, sal_uInt64& rOffset,
                                      sal_uInt16& rLength)
    {
        rOffset = 0;
        rLength = 0;

        for (sal_uInt16 n = 0, nCount = getNumberOfRecords(); n < nCount; n++)
        {
            const font::NameRecord* pNameRecord = getNameRecord(n);

            if (pNameRecord->nPlatformID == 3 // Windows
                && pNameRecord->nEncodingID == 1 // Unicode BMP
                && pNameRecord->nLanguageID == 0x0409 // en-us
                && pNameRecord->nNameID == sal_uInt16(eNameID))
            {
                rLength = pNameRecord->nLength;
                rOffset = getTableOffset() + getNameTable()->nStorageOffset
                          + pNameRecord->nStringOffset;
                return true;
            }
        }
        return false;
    }
};

/** Handles reading of table entries */
class TableEntriesHandler
{
private:
    FontDataContainer const& mrFontDataContainer;
    const char* mpFirstPosition;
    sal_uInt16 mnNumberOfTables;

    const char* getTablePointer(const TableDirectoryEntry* pEntry, size_t nEntrySize)
    {
        size_t nSize = mrFontDataContainer.size();
        if (pEntry->offset > nSize)
        {
            SAL_WARN("vcl.fonts", "Table offset beyond end of available data");
            return nullptr;
        }
        if (nEntrySize > nSize - pEntry->offset)
        {
            SAL_WARN("vcl.fonts", "Insufficient available data for table entry");
            return nullptr;
        }
        return mrFontDataContainer.getPointer() + pEntry->offset;
    }

public:
    TableEntriesHandler(FontDataContainer const& rFontDataContainer)
        : mrFontDataContainer(rFontDataContainer)
    {
        const char* pData = mrFontDataContainer.getPointer();
        assert(mrFontDataContainer.size() >= sizeof(TableDirectory));
        mpFirstPosition = pData + sizeof(TableDirectory);

        const TableDirectory* pDirectory = reinterpret_cast<const TableDirectory*>(pData);
        mnNumberOfTables = pDirectory->nNumberOfTables;

        size_t nAvailableData = mrFontDataContainer.size() - sizeof(TableDirectory);
        size_t nMaxRecordsPossible = nAvailableData / sizeof(TableDirectoryEntry);
        if (mnNumberOfTables > nMaxRecordsPossible)
        {
            SAL_WARN("vcl.fonts", "Font claimed to have " << mnNumberOfTables
                                                          << " table records, but only space for "
                                                          << nMaxRecordsPossible);
            mnNumberOfTables = nMaxRecordsPossible;
        }
    }

    const TableDirectoryEntry* getEntry(sal_uInt32 nTag)
    {
        for (sal_uInt32 i = 0; i < mnNumberOfTables; i++)
        {
            const char* pPosition = mpFirstPosition + sizeof(TableDirectoryEntry) * i;
            const auto* pEntry = reinterpret_cast<const TableDirectoryEntry*>(pPosition);

            if (nTag == pEntry->tag)
                return pEntry;
        }
        return nullptr;
    }

    const OS2Table* getOS2Table()
    {
        const auto* pEntry = getEntry(T_OS2);
        if (!pEntry)
            return nullptr;
        return reinterpret_cast<const OS2Table*>(getTablePointer(pEntry, sizeof(OS2Table)));
    }

    const HeadTable* getHeadTable()
    {
        const auto* pEntry = getEntry(T_head);
        if (!pEntry)
            return nullptr;
        return reinterpret_cast<const HeadTable*>(getTablePointer(pEntry, sizeof(HeadTable)));
    }

    const NameTable* getNameTable()
    {
        const auto* pEntry = getEntry(T_name);
        if (!pEntry)
            return nullptr;
        return reinterpret_cast<const NameTable*>(getTablePointer(pEntry, sizeof(NameTable)));
    }

    std::unique_ptr<NameTableHandler> getNameTableHandler()
    {
        const auto* pEntry = getEntry(T_name);
        if (!pEntry)
            return nullptr;

        return std::unique_ptr<NameTableHandler>(new NameTableHandler(mrFontDataContainer, pEntry));
    }
};

/** Entry point handler for the TTF Font */
class TTFFont
{
private:
    FontDataContainer const& mrFontDataContainer;

    const char* getTablePointer(const TableDirectoryEntry* pEntry)
    {
        return mrFontDataContainer.getPointer() + pEntry->offset;
    }

public:
    TTFFont(FontDataContainer const& rFontDataContainer)
        : mrFontDataContainer(rFontDataContainer)
    {
    }

    std::unique_ptr<TableEntriesHandler> getTableEntriesHandler()
    {
        size_t nSize = mrFontDataContainer.size();
        if (nSize < sizeof(TableDirectory))
        {
            SAL_WARN("vcl.fonts", "Font Data shorter than a TableDirectory");
            return nullptr;
        }
        return std::make_unique<TableEntriesHandler>(mrFontDataContainer);
    }

    /** Gets the string from a name table */
    OUString getNameTableString(sal_uInt64 nOffset, sal_uInt16 nLength)
    {
        const auto* pString = reinterpret_cast<const o3tl::sal_uInt16_BE*>(
            mrFontDataContainer.getPointer() + nOffset);
        OUStringBuffer aStringBuffer;
        for (sal_uInt16 i = 0; i < (nLength / 2); i++)
            aStringBuffer.append(sal_Unicode(pString[i]));
        return aStringBuffer.makeStringAndClear();
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
