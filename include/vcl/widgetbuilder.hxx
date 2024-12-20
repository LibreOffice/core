/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/Exception.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <vcl/builderbase.hxx>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>

/* Template class for a Builder to create a hierarchy of widgets from a .ui file
 * for dialogs, sidebar, etc.
 *
 * This class parses the .ui file and calls overridable methods
 * so subclasses can create the widgets of a specific toolkit.
 *
 * The VclBuilder subclass is the implementation using LibreOffice's own VCL toolkit
 * and the QtBuilder subclass uses native Qt widgets.
 */
template <typename Widget, typename WidgetPtr, typename MenuClass, typename MenuPtr>
class WidgetBuilder : public BuilderBase
{
protected:
    struct MenuAndId
    {
        OUString m_sID;
        MenuPtr m_pMenu;
        MenuAndId(OUString sId, MenuClass* pMenu)
            : m_sID(std::move(sId))
            , m_pMenu(pMenu)
        {
        }
    };

    WidgetBuilder(std::u16string_view sUIDir, const OUString& rUIFile, bool bLegacy)
        : BuilderBase(sUIDir, rUIFile, bLegacy)
    {
    }
    virtual ~WidgetBuilder() = default;

    std::vector<MenuAndId> m_aMenus;

public:
    //sID may not exist
    MenuClass* get_menu(std::u16string_view sID)
    {
        for (auto const& menu : m_aMenus)
        {
            if (menu.m_sID == sID)
                return menu.m_pMenu;
        }

        return nullptr;
    }

protected:
    void processUIFile(Widget* pParent)
    {
        try
        {
            xmlreader::XmlReader reader(getUIFileUrl());
            handleChild(pParent, nullptr, reader);
        }
        catch (const css::uno::Exception& rExcept)
        {
            TOOLS_WARN_EXCEPTION("vcl.builder", "Unable to read .ui file " << getUIFileUrl());
            reportException(rExcept);
            assert(false && "missing ui file or missing gb_CppunitTest_use_uiconfigs dependency");
            throw;
        }

        // Set Mnemonic widgets when everything has been imported
        for (const MnemonicWidgetMap& rMnemonic : getMnemonicWidgetMaps())
        {
            setMnemonicWidget(rMnemonic.m_sID, rMnemonic.m_sValue);
        }

        // Set radiobutton groups when everything has been imported
        for (const RadioButtonGroupMap& rGroup : getRadioButtonGroupMaps())
            setRadioButtonGroup(rGroup.m_sID, rGroup.m_sValue);
    }

    // either pParent or pAtkProps must be set, pParent for a child of a widget, pAtkProps for
    // collecting the atk info for a GtkMenuItem or tab child
    void handleChild(Widget* pParent, stringmap* pAtkProps, xmlreader::XmlReader& reader,
                     bool bToolbarItem = false)
    {
        xmlreader::Span name;
        int nsId;
        OString sType, sInternalChild;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "type")
            {
                name = reader.getAttributeValue(false);
                sType = OString(name.begin, name.length);
            }
            else if (name == "internal-child")
            {
                name = reader.getAttributeValue(false);
                sInternalChild = OString(name.begin, name.length);
            }
        }

        if (sType == "tab")
        {
            handleTabChild(pParent, reader);
            return;
        }

        WidgetPtr pCurrentChild = nullptr;
        int nLevel = 1;
        while (true)
        {
            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "object" || name == "placeholder")
                {
                    pCurrentChild = handleObject(pParent, pAtkProps, reader, sType, sInternalChild,
                                                 bToolbarItem);

                    bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;
                    if (bObjectInserted)
                        tweakInsertedChild(pParent, pCurrentChild, sType, sInternalChild);
                }
                else if (name == "packing")
                {
                    const stringmap aPackingProperties = collectPackingProperties(reader);
                    applyPackingProperties(pCurrentChild, pParent, aPackingProperties);
                }
                else if (name == "interface")
                {
                    while (reader.nextAttribute(&nsId, &name))
                    {
                        if (name == "domain")
                            handleInterfaceDomain(reader);
                    }
                    ++nLevel;
                }
                else
                    ++nLevel;
            }

            if (res == xmlreader::XmlReader::Result::End)
                --nLevel;

            if (!nLevel)
                break;

            if (res == xmlreader::XmlReader::Result::Done)
                break;
        }
    }

    WidgetPtr handleObject(Widget* pParent, stringmap* pAtkProps, xmlreader::XmlReader& reader,
                           std::string_view sType, std::string_view sInternalChild,
                           bool bToolbarItem)
    {
        OUString sClass;
        OUString sID;
        OUString sCustomProperty;
        extractClassAndIdAndCustomProperty(reader, sClass, sID, sCustomProperty);

        if (sClass == "GtkListStore" || sClass == "GtkTreeStore")
        {
            handleListStore(reader, sID, sClass);
            return nullptr;
        }
        else if (sClass == "GtkMenu")
        {
            handleMenu(reader, sID);
            return nullptr;
        }
        else if (sClass == "GtkSizeGroup")
        {
            handleSizeGroup(reader);
            return nullptr;
        }
        else if (sClass == "AtkObject")
        {
            assert((pParent || pAtkProps) && "must have one set");
            assert(!(pParent && pAtkProps) && "must not have both");
            auto aAtkProperties = handleAtkObject(reader);
            if (pParent)
                applyAtkProperties(pParent, aAtkProperties, bToolbarItem);
            if (pAtkProps)
                *pAtkProps = std::move(aAtkProperties);
            return nullptr;
        }

        int nLevel = 1;

        stringmap aProperties, aPangoAttributes;
        stringmap aAtkAttributes;
        std::vector<ComboBoxTextItem> aItems;

        if (!sCustomProperty.isEmpty())
            aProperties[u"customproperty"_ustr] = sCustomProperty;

        // Internal-children default in glade to not having their visible bits set
        // even though they are visible (generally anyway)
        if (!sInternalChild.empty())
            aProperties[u"visible"_ustr] = "True";

        WidgetPtr pCurrentChild = nullptr;
        while (true)
        {
            xmlreader::Span name;
            int nsId;
            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Done)
                break;

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "child")
                {
                    if (!pCurrentChild)
                    {
                        pCurrentChild = insertObject(pParent, sClass, sType, sID, aProperties,
                                                     aPangoAttributes, aAtkAttributes);
                    }
                    handleChild(pCurrentChild, nullptr, reader, isToolbarItemClass(sClass));
                }
                else if (name == "items")
                    aItems = handleItems(reader);
                else if (name == "style")
                {
                    int nPriority = 0;
                    std::vector<vcl::EnumContext::Context> aContext
                        = handleStyle(reader, nPriority);
                    if (nPriority != 0)
                        setPriority(pCurrentChild, nPriority);
                    if (!aContext.empty())
                        setContext(pCurrentChild, std::move(aContext));
                }
                else
                {
                    ++nLevel;
                    if (name == "property")
                        collectProperty(reader, aProperties);
                    else if (name == "attribute")
                        collectPangoAttribute(reader, aPangoAttributes);
                    else if (name == "relation")
                        collectAtkRelationAttribute(reader, aAtkAttributes);
                    else if (name == "role")
                        collectAtkRoleAttribute(reader, aAtkAttributes);
                    else if (name == "action-widget")
                        handleActionWidget(reader);
                }
            }

            if (res == xmlreader::XmlReader::Result::End)
            {
                --nLevel;
            }

            if (!nLevel)
                break;
        }

        if (sClass == "GtkAdjustment")
        {
            addAdjustment(sID, aProperties);
            return nullptr;
        }
        else if (sClass == "GtkTextBuffer")
        {
            addTextBuffer(sID, aProperties);
            return nullptr;
        }

        if (!pCurrentChild)
        {
            pCurrentChild = insertObject(pParent, sClass, sType, sID, aProperties, aPangoAttributes,
                                         aAtkAttributes);
        }

        if (!aItems.empty())
            insertComboBoxOrListBoxItems(pCurrentChild, aProperties, aItems);

        return pCurrentChild;
    }

    void handleTabChild(Widget* pParent, xmlreader::XmlReader& reader)
    {
        std::vector<OUString> sIDs;

        int nLevel = 1;
        stringmap aProperties;
        stringmap aAtkProperties;
        std::vector<vcl::EnumContext::Context> context;

        while (true)
        {
            xmlreader::Span name;
            int nsId;

            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                ++nLevel;
                if (name == "object")
                {
                    while (reader.nextAttribute(&nsId, &name))
                    {
                        if (name == "id")
                        {
                            name = reader.getAttributeValue(false);
                            OUString sID(name.begin, name.length, RTL_TEXTENCODING_UTF8);
                            sal_Int32 nDelim = sID.indexOf(':');
                            if (nDelim != -1)
                            {
                                aProperties[u"customproperty"_ustr] = sID.copy(nDelim + 1);
                                sID = sID.copy(0, nDelim);
                            }
                            sIDs.push_back(sID);
                        }
                    }
                }
                else if (name == "style")
                {
                    int nPriority = 0;
                    context = handleStyle(reader, nPriority);
                    --nLevel;
                }
                else if (name == "property")
                    collectProperty(reader, aProperties);
                else if (name == "child" && isHorizontalTabControl(pParent))
                {
                    // just to collect the atk properties (if any) for the label
                    handleChild(nullptr, &aAtkProperties, reader);
                    --nLevel;
                }
            }

            if (res == xmlreader::XmlReader::Result::End)
                --nLevel;

            if (!nLevel)
                break;

            if (res == xmlreader::XmlReader::Result::Done)
                break;
        }

        if (!pParent)
            return;

        applyTabChildProperties(pParent, sIDs, context, aProperties, aAtkProperties);
    }

    void handleMenu(xmlreader::XmlReader& reader, const OUString& rID)
    {
        MenuPtr pCurrentMenu = createMenu(rID);

        int nLevel = 1;

        stringmap aProperties;

        while (true)
        {
            xmlreader::Span name;
            int nsId;

            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Done)
                break;

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "child")
                {
                    handleMenuChild(pCurrentMenu, reader);
                }
                else
                {
                    ++nLevel;
                    if (name == "property")
                        collectProperty(reader, aProperties);
                }
            }

            if (res == xmlreader::XmlReader::Result::End)
            {
                --nLevel;
            }

            if (!nLevel)
                break;
        }

        m_aMenus.emplace_back(rID, pCurrentMenu);
    }

    void handleMenuChild(MenuClass* pParent, xmlreader::XmlReader& reader)
    {
        xmlreader::Span name;
        int nsId;

        int nLevel = 1;
        while (true)
        {
            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "object" || name == "placeholder")
                {
                    handleMenuObject(pParent, reader);
                }
                else
                    ++nLevel;
            }

            if (res == xmlreader::XmlReader::Result::End)
                --nLevel;

            if (!nLevel)
                break;

            if (res == xmlreader::XmlReader::Result::Done)
                break;
        }
    }

    void handleMenuObject(MenuClass* pParent, xmlreader::XmlReader& reader)
    {
        OUString sClass;
        OUString sID;
        OUString sCustomProperty;
        MenuClass* pSubMenu = nullptr;

        xmlreader::Span name;
        int nsId;

        while (reader.nextAttribute(&nsId, &name))
        {
            if (name == "class")
            {
                name = reader.getAttributeValue(false);
                sClass = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
            }
            else if (name == "id")
            {
                name = reader.getAttributeValue(false);
                sID = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
                if (isLegacy())
                {
                    sal_Int32 nDelim = sID.indexOf(':');
                    if (nDelim != -1)
                    {
                        sCustomProperty = sID.subView(nDelim + 1);
                        sID = sID.copy(0, nDelim);
                    }
                }
            }
        }

        int nLevel = 1;

        stringmap aProperties;
        stringmap aAtkProperties;
        accelmap aAccelerators;

        if (!sCustomProperty.isEmpty())
            aProperties[u"customproperty"_ustr] = sCustomProperty;

        while (true)
        {
            xmlreader::XmlReader::Result res
                = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);

            if (res == xmlreader::XmlReader::Result::Done)
                break;

            if (res == xmlreader::XmlReader::Result::Begin)
            {
                if (name == "child")
                {
                    size_t nChildMenuIdx = m_aMenus.size();
                    handleChild(nullptr, &aAtkProperties, reader);
                    bool bSubMenuInserted = m_aMenus.size() > nChildMenuIdx;
                    if (bSubMenuInserted)
                        pSubMenu = m_aMenus[nChildMenuIdx].m_pMenu;
                }
                else
                {
                    ++nLevel;
                    if (name == "property")
                        collectProperty(reader, aProperties);
                    else if (name == "accelerator")
                        collectAccelerator(reader, aAccelerators);
                }
            }

            if (res == xmlreader::XmlReader::Result::End)
            {
                --nLevel;
            }

            if (!nLevel)
                break;
        }

        insertMenuObject(pParent, pSubMenu, sClass, sID, aProperties, aAtkProperties,
                         aAccelerators);
    }

    virtual void applyAtkProperties(Widget* pWidget, const stringmap& rProperties,
                                    bool bToolbarItem)
        = 0;
    virtual void applyPackingProperties(Widget* pCurrentChild, Widget* pParent,
                                        const stringmap& rPackingProperties)
        = 0;
    virtual void applyTabChildProperties(Widget* pParent, const std::vector<OUString>& rIDs,
                                         std::vector<vcl::EnumContext::Context>& rContext,
                                         stringmap& rProperties, stringmap& rAtkProperties)
        = 0;
    virtual void insertComboBoxOrListBoxItems(Widget* pWidget, stringmap& rMap,
                                              const std::vector<ComboBoxTextItem>& rItems)
        = 0;

    virtual WidgetPtr insertObject(Widget* pParent, const OUString& rClass, std::string_view sType,
                                   const OUString& rID, stringmap& rProps,
                                   stringmap& rPangoAttributes, stringmap& rAtkProps)
        = 0;

    virtual void tweakInsertedChild(Widget* pParent, Widget* pCurrentChild, std::string_view sType,
                                    std::string_view sInternalChild)
        = 0;

    virtual void setMnemonicWidget(const OUString& rLabelId, const OUString& rMnemonicWidgetId) = 0;
    virtual void setRadioButtonGroup(const OUString& rRadioButtonId, const OUString& rRadioGroupId)
        = 0;
    virtual void setPriority(Widget* pWidget, int nPriority) = 0;
    virtual void setContext(Widget* pWidget, std::vector<vcl::EnumContext::Context>&& aContext) = 0;

    // Whether the given widget is a horizontal, i.e. non-vertical tab control
    virtual bool isHorizontalTabControl(Widget* pWidget) = 0;

    virtual MenuPtr createMenu(const OUString& rID) = 0;
    virtual void insertMenuObject(MenuClass* pParent, MenuClass* pSubMenu, const OUString& rClass,
                                  const OUString& rID, stringmap& rProps, stringmap& rAtkProps,
                                  accelmap& rAccels)
        = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
