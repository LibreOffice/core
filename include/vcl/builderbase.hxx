/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>

#include <vcl/dllapi.h>
#include <vcl/EnumContext.hxx>

#include <map>
#include <vector>

namespace xmlreader
{
class XmlReader;
}

struct ComboBoxTextItem
{
    OUString m_sItem;
    OUString m_sId;
    ComboBoxTextItem(OUString sItem, OUString sId)
        : m_sItem(std::move(sItem))
        , m_sId(std::move(sId))
    {
    }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) BuilderBase
{
public:
    typedef std::map<OUString, OUString> stringmap;
    typedef std::map<OUString, std::pair<OUString, OUString>> accelmap;

    typedef stringmap Adjustment;
    typedef stringmap TextBuffer;

protected:
    BuilderBase(const OUString& rUIFile, bool bLegacy);

    struct ListStore
    {
        typedef std::vector<OUString> row;
        std::vector<row> m_aEntries;
    };

    struct SizeGroup
    {
        std::vector<OUString> m_aWidgets;
        stringmap m_aProperties;
        SizeGroup() {}
    };

    static void collectPangoAttribute(xmlreader::XmlReader& reader, stringmap& rMap);
    static void collectAtkRelationAttribute(xmlreader::XmlReader& reader, stringmap& rMap);
    static void collectAtkRoleAttribute(xmlreader::XmlReader& reader, stringmap& rMap);
    static void collectAccelerator(xmlreader::XmlReader& reader, accelmap& rMap);
    stringmap collectPackingProperties(xmlreader::XmlReader& reader);
    void collectProperty(xmlreader::XmlReader& rReader, stringmap& rMap) const;
    void extractClassAndIdAndCustomProperty(xmlreader::XmlReader& reader, OUString& rClass,
                                            OUString& rId, OUString& rCustomProperty);

    void handleActionWidget(xmlreader::XmlReader& reader);
    void handleInterfaceDomain(xmlreader::XmlReader& rReader);
    static bool isToolbarItemClass(std::u16string_view sClass);
    static std::vector<vcl::EnumContext::Context> handleStyle(xmlreader::XmlReader& reader,
                                                              int& nPriority);
    static OUString getStyleClass(xmlreader::XmlReader& reader);
    static bool hasOrientationVertical(stringmap& rMap);

    OUString getHelpRoot() { return m_sHelpRoot; }
    bool isLegacy() { return m_bLegacy; }
    const std::locale& getResLocale() const;
    const std::vector<SizeGroup>& getSizeGroups() const;

    OUString finalizeValue(const OString& rContext, const OString& rValue,
                           const bool bTranslate) const;

    stringmap handleAtkObject(xmlreader::XmlReader& reader) const;
    std::vector<ComboBoxTextItem> handleItems(xmlreader::XmlReader& reader) const;
    void handleListStore(xmlreader::XmlReader& reader, const OUString& rID,
                         std::u16string_view rClass);
    void handleRow(xmlreader::XmlReader& reader, const OUString& rID);

    void addAdjustment(const OUString& sID, const Adjustment& rAdjustment);
    const Adjustment* get_adjustment_by_name(const OUString& sID) const;

    void addTextBuffer(const OUString& sID, const TextBuffer& rTextBuffer);
    const TextBuffer* get_buffer_by_name(const OUString& sID) const;

    const ListStore* get_model_by_name(const OUString& sID) const;

    virtual void set_response(std::u16string_view sID, short nResponse) = 0;

    void handleSizeGroup(xmlreader::XmlReader& reader);

    virtual void resetParserState();

private:
    struct ParserState
    {
        std::locale m_aResLocale;

        std::map<OUString, ListStore> m_aModels;
        std::vector<SizeGroup> m_aSizeGroups;

        std::map<OUString, Adjustment> m_aAdjustments;
        std::map<OUString, TextBuffer> m_aTextBuffers;
    };

    std::unique_ptr<ParserState> m_pParserState;

    OUString m_sHelpRoot;
    bool m_bLegacy;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
