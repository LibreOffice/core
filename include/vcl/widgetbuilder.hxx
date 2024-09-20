/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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
    WidgetBuilder(const OUString& rUIFile, bool bLegacy)
        : BuilderBase(rUIFile, bLegacy)
    {
    }
    virtual ~WidgetBuilder() = default;

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
                    pCurrentChild = handleObject(pParent, pAtkProps, reader, bToolbarItem);

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

    virtual void applyPackingProperties(Widget* pCurrentChild, Widget* pParent,
                                        const stringmap& rPackingProperties)
        = 0;
    virtual void tweakInsertedChild(Widget* pParent, Widget* pCurrentChild, std::string_view sType,
                                    std::string_view sInternalChild)
        = 0;

    // These methods are currently only implemented by VclBuilder and should be
    // refactored as described in the class documentation above (split into
    // parsing done in this class + overridable methods that don't need XmlReader
    // that get implemented in the sublasses)
    //
    // Until that's done, other subclasses can be used to handle only those .ui files
    // not using the corresponding features (attributes/objects in the .ui file).
    virtual WidgetPtr handleObject(Widget* /*pParent*/, stringmap* /*pAtkProps*/,
                                   xmlreader::XmlReader& /*reader*/, bool /*bToolbarItem*/)
    {
        assert(false && "Functionality not implemented by this subclass yet.");
        return nullptr;
    }
    virtual void handleTabChild(Widget* /*pParent*/, xmlreader::XmlReader& /*reader*/)
    {
        assert(false && "Functionality not implemented by this subclass yet.");
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
