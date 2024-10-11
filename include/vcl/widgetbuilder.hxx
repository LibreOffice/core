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
 * The idea is for this class to parse the .ui file and call overridable methods
 * so subclasses can create the widgets of a specific toolkit.
 *
 * VclBuilder is the implementation using LibreOffice's own VCL toolkit
 * and there is a work-in-progress implementation using native Qt widgets
 * at https://gerrit.libreoffice.org/c/core/+/161831 .
 *
 * Currently, .ui file parsing isn't yet fully done by this class as described
 * above, but needs further refactoring to split the corresponding VclBuilder
 * methods into methods that do the parsing (which should reside in this class)
 * and overridable methods to actually create the widgets,... (which should reside in
 * VclBuilder and other subclasses).
 */
template <typename Widget, typename WidgetPtr> class WidgetBuilder : public BuilderBase
{
protected:
    WidgetBuilder(std::u16string_view sUIDir, const OUString& rUIFile, bool bLegacy)
        : BuilderBase(sUIDir, rUIFile, bLegacy)
    {
    }
    virtual ~WidgetBuilder() = default;

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
                    pCurrentChild
                        = handleObject(pParent, pAtkProps, reader, sInternalChild, bToolbarItem);

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
                           std::string_view sInternalChild, bool bToolbarItem)
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
            handleMenu(reader, pParent, sID, false);
            return nullptr;
        }
        else if (sClass == "GtkMenuBar")
        {
            handleMenu(reader, pParent, sID, true);
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
                        pCurrentChild = insertObject(pParent, sClass, sID, aProperties,
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
            pCurrentChild
                = insertObject(pParent, sClass, sID, aProperties, aPangoAttributes, aAtkAttributes);
        }

        if (!aItems.empty())
            insertComboBoxOrListBoxItems(pCurrentChild, aProperties, aItems);

        return pCurrentChild;
    }

    virtual void applyAtkProperties(Widget* pWidget, const stringmap& rProperties,
                                    bool bToolbarItem)
        = 0;
    virtual void applyPackingProperties(Widget* pCurrentChild, Widget* pParent,
                                        const stringmap& rPackingProperties)
        = 0;
    virtual void insertComboBoxOrListBoxItems(Widget* pWidget, stringmap& rMap,
                                              const std::vector<ComboBoxTextItem>& rItems)
        = 0;

    virtual WidgetPtr insertObject(Widget* pParent, const OUString& rClass, const OUString& rID,
                                   stringmap& rProps, stringmap& rPangoAttributes,
                                   stringmap& rAtkProps)
        = 0;

    virtual void tweakInsertedChild(Widget* pParent, Widget* pCurrentChild, std::string_view sType,
                                    std::string_view sInternalChild)
        = 0;

    virtual void setMnemonicWidget(const OUString& rLabelId, const OUString& rMnemonicWidgetId) = 0;
    virtual void setPriority(Widget* pWidget, int nPriority) = 0;
    virtual void setContext(Widget* pWidget, std::vector<vcl::EnumContext::Context>&& aContext) = 0;

    // These methods are currently only implemented by VclBuilder and should be
    // refactored as described in the class documentation above (split into
    // parsing done in this class + overridable methods that don't need XmlReader
    // that get implemented in the subclasses)
    //
    // Until that's done, other subclasses can be used to handle only those .ui files
    // not using the corresponding features (attributes/objects in the .ui file).
    virtual void handleMenu(xmlreader::XmlReader& /*reader*/, Widget* /*pParent*/,
                            const OUString& /*rID*/, bool /*bMenuBar*/)
    {
        assert(false && "Functionality not implemented by this subclass yet.");
    }

    virtual void handleTabChild(Widget* /*pParent*/, xmlreader::XmlReader& /*reader*/)

    {
        assert(false && "Functionality not implemented by this subclass yet.");
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
