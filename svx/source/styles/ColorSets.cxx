/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ColorSets.hxx>

#include <optional>
#include <unordered_set>
#include <vector>

#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <o3tl/numeric.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWalker.hxx>
#include <tools/XmlWriter.hxx>
#include <vcl/UserResourceScanner.hxx>
#include <unotools/pathoptions.hxx>
#include <o3tl/enumrange.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

using namespace com::sun::star;

namespace svx
{
namespace
{

class DocumentThemeScanner : public vcl::UserResourceScanner
{
private:
    std::vector<model::ColorSet>& mrColorSets;

public:
    DocumentThemeScanner(std::vector<model::ColorSet>& rColorSets)
        : mrColorSets(rColorSets)
    {}

private:
    static model::ThemeColorType getThemeColor(std::string_view sColorName)
    {
        model::ThemeColorType eTheme = model::ThemeColorType::Unknown;

        static constexpr auto constThemeColorTypeMapping =  frozen::make_unordered_map<std::string_view, model::ThemeColorType>({
            { "dark1",  model::ThemeColorType::Dark1 },
            { "light1",  model::ThemeColorType::Light1 },
            { "dark2",  model::ThemeColorType::Dark2 },
            { "light2",  model::ThemeColorType::Light2 },
            { "accent1",  model::ThemeColorType::Accent1 },
            { "accent2",  model::ThemeColorType::Accent2 },
            { "accent3",  model::ThemeColorType::Accent3 },
            { "accent4",  model::ThemeColorType::Accent4 },
            { "accent5",  model::ThemeColorType::Accent5 },
            { "accent6",  model::ThemeColorType::Accent6 },
            { "hyperlink",  model::ThemeColorType::Hyperlink },
            { "followed-hyperlink",  model::ThemeColorType::FollowedHyperlink },
        });
        auto iterator = constThemeColorTypeMapping.find(sColorName);
        if (iterator != constThemeColorTypeMapping.end())
            eTheme = iterator->second;
        return eTheme;
    }

    bool addResource(const OUString& rPath) override
    {
        SvFileStream aFileStream(rPath, StreamMode::READ);

        tools::XmlWalker aWalker;
        if (!aWalker.open(&aFileStream))
            return false;

        if (aWalker.name() != "theme")
            return false;

        OString aThemeNameUTF8 = aWalker.attribute("name"_ostr);
        OUString aThemeName = OStringToOUString(aThemeNameUTF8, RTL_TEXTENCODING_UTF8);

        model::ColorSet aColorSet(aThemeName);

        aWalker.children();
        while (aWalker.isValid())
        {
            if (aWalker.name() == "theme-colors")
            {
                aWalker.children();
                while (aWalker.isValid())
                {
                    if (aWalker.name() == "color")
                    {
                        OString aName = aWalker.attribute("name"_ostr);
                        auto eThemeColor = getThemeColor(aName);
                        OString aColorString = aWalker.attribute("color"_ostr);
                        Color aColor;
                        if (eThemeColor != model::ThemeColorType::Unknown && color::createFromString(aColorString, aColor))
                            aColorSet.add(eThemeColor, aColor);
                    }
                    aWalker.next();
                }
                aWalker.parent();
            }
            aWalker.next();
        }
        aWalker.parent();

        mrColorSets.push_back(aColorSet);

        return true;
    }

    bool isValidResource(const OUString& rFilename) override
    {
        if (rFilename.isEmpty())
            return false;

        if (!rFilename.endsWithIgnoreAsciiCase(u".theme"))
            return false;

        osl::FileStatus aFileStatus(osl_FileStatus_Mask_Type);
        if (!vcl::file::readFileStatus(aFileStatus, rFilename))
            return false;

        if (!aFileStatus.isRegular())
            return false;

        return true;
    }
};

} // end anonymous namespace

ColorSets::ColorSets()
{
    init();
}

ColorSets& ColorSets::get()
{
    static std::optional<ColorSets> sColorSet;
    if (!sColorSet)
        sColorSet = ColorSets();
    return *sColorSet;
}

void ColorSets::init()
{
    SvtPathOptions aPathOptions;
    OUString aURLString = aPathOptions.GetDocumentThemePath();

    DocumentThemeScanner aScanner(maColorSets);
    aScanner.addPaths(aURLString);

    std::deque<OUString> aURLs;
    vcl::file::splitPathString(aURLString, aURLs);
    if (aURLs.size() > 0)
        maUserFolder = aURLs[0];
}

model::ColorSet const* ColorSets::getColorSet(std::u16string_view rName) const
{
    for (const model::ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return &rColorSet;
    }
    return nullptr;
}
namespace
{

OUString findUniqueName(std::unordered_set<OUString> const& rNames, OUString const& rNewName)
{
    auto iterator = rNames.find(rNewName);
    if (iterator == rNames.cend())
        return rNewName;

    int i = 1;
    OUString aName;
    do
    {
        aName = rNewName + "_" + OUString::number(i);
        i++;
        iterator = rNames.find(aName);
    } while (iterator != rNames.cend());

    return aName;
}

} // end anonymous namespace

void ColorSets::insert(model::ColorSet const& rNewColorSet, IdenticalNameAction eAction)
{
    if (eAction == IdenticalNameAction::Overwrite)
    {
        for (model::ColorSet& rColorSet : maColorSets)
        {
            if (rColorSet.getName() == rNewColorSet.getName())
            {
                rColorSet = rNewColorSet;
                return;
            }
        }
        // color set not found, so insert it
        maColorSets.push_back(rNewColorSet);
        writeToUserFolder(rNewColorSet);
    }
    else if (eAction == IdenticalNameAction::AutoRename)
    {
        std::unordered_set<OUString> aNames;
        for (model::ColorSet& rColorSet : maColorSets)
            aNames.insert(rColorSet.getName());

        OUString aName = findUniqueName(aNames, rNewColorSet.getName());

        model::ColorSet aNewColorSet = rNewColorSet;
        aNewColorSet.setName(aName);

        maColorSets.push_back(aNewColorSet);
        writeToUserFolder(aNewColorSet);
    }
}

void ColorSets::writeToUserFolder(model::ColorSet const& rNewColorSet)
{
    static constexpr auto constThemeColorTypeToName = frozen::make_unordered_map<model::ThemeColorType, std::string_view>({
        { model::ThemeColorType::Dark1, "dark1" },
        { model::ThemeColorType::Light1, "light1" },
        { model::ThemeColorType::Dark2, "dark2" },
        { model::ThemeColorType::Light2, "light2" },
        { model::ThemeColorType::Accent1, "accent1" },
        { model::ThemeColorType::Accent2, "accent2" },
        { model::ThemeColorType::Accent3, "accent3" },
        { model::ThemeColorType::Accent4, "accent4" },
        { model::ThemeColorType::Accent5, "accent5" },
        { model::ThemeColorType::Accent6, "accent6" },
        { model::ThemeColorType::Hyperlink, "hyperlink" },
        { model::ThemeColorType::FollowedHyperlink, "followed-hyperlink" }
    });

    SvFileStream aFileStream(maUserFolder + "/" + rNewColorSet.getName() + ".theme", StreamMode::WRITE | StreamMode::TRUNC);

    tools::XmlWriter aWriter(&aFileStream);
    aWriter.startDocument();
    aWriter.startElement("theme");
    aWriter.attribute("name", rNewColorSet.getName());

    aWriter.startElement("theme-colors");
    aWriter.attribute("name", rNewColorSet.getName());

    for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
    {
        auto iterator = constThemeColorTypeToName.find(eThemeColorType);
        if (iterator != constThemeColorTypeToName.end())
        {
            Color aColor = rNewColorSet.getColor(eThemeColorType);
            aWriter.startElement("color");
            aWriter.attribute("name", OString(iterator->second));
            aWriter.attribute("color", "#"_ostr + aColor.AsRGBHexString().toUtf8());
            aWriter.endElement();
        }
    }

    aWriter.endElement();

    aWriter.endElement();
    aWriter.endDocument();
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
