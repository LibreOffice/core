/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <comphelper/processfactory.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <vcl/builder.hxx>
#include "convert3to4.hxx"

namespace
{
typedef std::pair<css::uno::Reference<css::xml::dom::XNode>, OUString> named_node;

bool sortButtonNodes(const named_node& rA, const named_node& rB)
{
    OString sA(rA.second.toUtf8());
    OString sB(rB.second.toUtf8());
    //order within groups according to platform rules
    return getButtonPriority(sA) < getButtonPriority(sB);
}

// <property name="spacing">6</property>
css::uno::Reference<css::xml::dom::XNode>
CreateProperty(const css::uno::Reference<css::xml::dom::XDocument>& xDoc, const OUString& rPropName,
               const OUString& rValue)
{
    css::uno::Reference<css::xml::dom::XElement> xProperty = xDoc->createElement("property");
    css::uno::Reference<css::xml::dom::XAttr> xPropName = xDoc->createAttribute("name");
    xPropName->setValue(rPropName);
    xProperty->setAttributeNode(xPropName);
    css::uno::Reference<css::xml::dom::XText> xValue = xDoc->createTextNode(rValue);
    xProperty->appendChild(xValue);
    return xProperty;
}

bool ToplevelIsMessageDialog(const css::uno::Reference<css::xml::dom::XNode>& xNode)
{
    for (css::uno::Reference<css::xml::dom::XNode> xObjectCandidate = xNode->getParentNode();
         xObjectCandidate.is(); xObjectCandidate = xObjectCandidate->getParentNode())
    {
        if (xObjectCandidate->getNodeName() == "object")
        {
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xObjectMap
                = xObjectCandidate->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xObjectMap->getNamedItem("class");
            if (xClass->getNodeValue() == "GtkMessageDialog")
                return true;
        }
    }
    return false;
}

void insertAsFirstChild(const css::uno::Reference<css::xml::dom::XNode>& xParentNode,
                        const css::uno::Reference<css::xml::dom::XNode>& xChildNode)
{
    auto xFirstChild = xParentNode->getFirstChild();
    if (xFirstChild.is())
        xParentNode->insertBefore(xChildNode, xFirstChild);
    else
        xParentNode->appendChild(xChildNode);
}

void SetPropertyOnTopLevel(const css::uno::Reference<css::xml::dom::XNode>& xNode,
                           const css::uno::Reference<css::xml::dom::XNode>& xProperty)
{
    for (css::uno::Reference<css::xml::dom::XNode> xObjectCandidate = xNode->getParentNode();
         xObjectCandidate.is(); xObjectCandidate = xObjectCandidate->getParentNode())
    {
        if (xObjectCandidate->getNodeName() == "object")
        {
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xObjectMap
                = xObjectCandidate->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xObjectMap->getNamedItem("class");
            if (xClass->getNodeValue() == "GtkDialog")
            {
                insertAsFirstChild(xObjectCandidate, xProperty);
                break;
            }
        }
    }
}

OUString GetParentObjectType(const css::uno::Reference<css::xml::dom::XNode>& xNode)
{
    auto xParent = xNode->getParentNode();
    assert(xParent->getNodeName() == "object");
    css::uno::Reference<css::xml::dom::XNamedNodeMap> xParentMap = xParent->getAttributes();
    css::uno::Reference<css::xml::dom::XNode> xClass = xParentMap->getNamedItem("class");
    return xClass->getNodeValue();
}

css::uno::Reference<css::xml::dom::XNode>
GetChildObject(const css::uno::Reference<css::xml::dom::XNode>& xChild)
{
    for (css::uno::Reference<css::xml::dom::XNode> xObjectCandidate = xChild->getFirstChild();
         xObjectCandidate.is(); xObjectCandidate = xObjectCandidate->getNextSibling())
    {
        if (xObjectCandidate->getNodeName() == "object")
            return xObjectCandidate;
    }
    return nullptr;
}

// currently runs the risk of duplicate margin-* properties if there was already such as well
// as the border
void AddBorderAsMargins(const css::uno::Reference<css::xml::dom::XNode>& xNode,
                        const OUString& rBorderWidth)
{
    auto xDoc = xNode->getOwnerDocument();

    auto xMarginEnd = CreateProperty(xDoc, "margin-end", rBorderWidth);
    insertAsFirstChild(xNode, xMarginEnd);
    xNode->insertBefore(CreateProperty(xDoc, "margin-top", rBorderWidth), xMarginEnd);
    xNode->insertBefore(CreateProperty(xDoc, "margin-bottom", rBorderWidth), xMarginEnd);
    xNode->insertBefore(CreateProperty(xDoc, "margin-start", rBorderWidth), xMarginEnd);
}

struct MenuEntry
{
    bool m_bDrawAsRadio;
    css::uno::Reference<css::xml::dom::XNode> m_xPropertyLabel;

    MenuEntry(bool bDrawAsRadio, const css::uno::Reference<css::xml::dom::XNode>& rPropertyLabel)
        : m_bDrawAsRadio(bDrawAsRadio)
        , m_xPropertyLabel(rPropertyLabel)
    {
    }
};

MenuEntry ConvertMenu(css::uno::Reference<css::xml::dom::XNode>& xMenuSection,
                      const css::uno::Reference<css::xml::dom::XNode>& xNode)
{
    bool bDrawAsRadio = false;
    css::uno::Reference<css::xml::dom::XNode> xPropertyLabel;

    css::uno::Reference<css::xml::dom::XNode> xChild = xNode->getFirstChild();
    while (xChild.is())
    {
        if (xChild->getNodeName() == "property")
        {
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xName = xMap->getNamedItem("name");
            OUString sName(xName->getNodeValue().replace('_', '-'));

            if (sName == "label")
            {
                xPropertyLabel = xChild;
            }
            else if (sName == "draw-as-radio")
            {
                bDrawAsRadio = toBool(xChild->getFirstChild()->getNodeValue());
            }
        }

        auto xNextChild = xChild->getNextSibling();

        auto xCurrentMenuSection = xMenuSection;

        if (xChild->getNodeName() == "object")
        {
            auto xDoc = xChild->getOwnerDocument();

            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xMap->getNamedItem("class");
            OUString sClass(xClass->getNodeValue());

            if (sClass == "GtkMenuItem" || sClass == "GtkRadioMenuItem")
            {
                /* <item> */
                css::uno::Reference<css::xml::dom::XElement> xItem = xDoc->createElement("item");
                xMenuSection->appendChild(xItem);
            }
            else if (sClass == "GtkSeparatorMenuItem")
            {
                /* <section> */
                css::uno::Reference<css::xml::dom::XElement> xSection
                    = xDoc->createElement("section");
                xMenuSection->getParentNode()->appendChild(xSection);
                xMenuSection = xSection;
                xCurrentMenuSection = xMenuSection;
            }
            else if (sClass == "GtkMenu")
            {
                xMenuSection->removeChild(xMenuSection->getLastChild()); // remove preceding <item>

                css::uno::Reference<css::xml::dom::XElement> xSubMenu
                    = xDoc->createElement("submenu");
                css::uno::Reference<css::xml::dom::XAttr> xIdAttr = xDoc->createAttribute("id");

                css::uno::Reference<css::xml::dom::XNode> xId = xMap->getNamedItem("id");
                OUString sId(xId->getNodeValue());

                xIdAttr->setValue(sId);
                xSubMenu->setAttributeNode(xIdAttr);
                xMenuSection->appendChild(xSubMenu);

                css::uno::Reference<css::xml::dom::XElement> xSection
                    = xDoc->createElement("section");
                xSubMenu->appendChild(xSection);

                xMenuSection = xSubMenu;
            }
        }

        bool bChildDrawAsRadio = false;
        css::uno::Reference<css::xml::dom::XNode> xChildPropertyLabel;
        if (xChild->hasChildNodes())
        {
            MenuEntry aEntry = ConvertMenu(xMenuSection, xChild);
            bChildDrawAsRadio = aEntry.m_bDrawAsRadio;
            xChildPropertyLabel = aEntry.m_xPropertyLabel;
        }

        if (xChild->getNodeName() == "object")
        {
            xMenuSection = xCurrentMenuSection;

            auto xDoc = xChild->getOwnerDocument();

            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xMap->getNamedItem("class");
            OUString sClass(xClass->getNodeValue());

            if (sClass == "GtkMenuItem" || sClass == "GtkRadioMenuItem")
            {
                css::uno::Reference<css::xml::dom::XNode> xId = xMap->getNamedItem("id");
                OUString sId = xId->getNodeValue();

                /*
                    <attribute name='label' translatable='yes'>whatever</attribute>
                    <attribute name='action'>menu.action</attribute>
                    <attribute name='target'>id</attribute>
                */
                auto xItem = xMenuSection->getLastChild();

                if (xChildPropertyLabel)
                {
                    css::uno::Reference<css::xml::dom::XElement> xChildPropertyElem(
                        xChildPropertyLabel, css::uno::UNO_QUERY_THROW);

                    css::uno::Reference<css::xml::dom::XElement> xLabelAttr
                        = xDoc->createElement("attribute");

                    css::uno::Reference<css::xml::dom::XNamedNodeMap> xLabelMap
                        = xChildPropertyLabel->getAttributes();
                    while (xLabelMap->getLength())
                    {
                        css::uno::Reference<css::xml::dom::XAttr> xAttr(xLabelMap->item(0),
                                                                        css::uno::UNO_QUERY_THROW);
                        xLabelAttr->setAttributeNode(
                            xChildPropertyElem->removeAttributeNode(xAttr));
                    }
                    xLabelAttr->appendChild(
                        xChildPropertyLabel->removeChild(xChildPropertyLabel->getFirstChild()));

                    xChildPropertyLabel->getParentNode()->removeChild(xChildPropertyLabel);
                    xItem->appendChild(xLabelAttr);
                }

                css::uno::Reference<css::xml::dom::XElement> xActionAttr
                    = xDoc->createElement("attribute");
                css::uno::Reference<css::xml::dom::XAttr> xActionName
                    = xDoc->createAttribute("name");
                xActionName->setValue("action");
                xActionAttr->setAttributeNode(xActionName);
                if (bChildDrawAsRadio)
                    xActionAttr->appendChild(xDoc->createTextNode("menu.radio." + sId));
                else
                    xActionAttr->appendChild(xDoc->createTextNode("menu.normal." + sId));
                xItem->appendChild(xActionAttr);

                css::uno::Reference<css::xml::dom::XElement> xTargetAttr
                    = xDoc->createElement("attribute");
                css::uno::Reference<css::xml::dom::XAttr> xTargetName
                    = xDoc->createAttribute("name");
                xTargetName->setValue("target");
                xTargetAttr->setAttributeNode(xTargetName);
                xTargetAttr->appendChild(xDoc->createTextNode(sId));
                xItem->appendChild(xTargetAttr);

                css::uno::Reference<css::xml::dom::XElement> xHiddenWhenAttr
                    = xDoc->createElement("attribute");
                css::uno::Reference<css::xml::dom::XAttr> xHiddenWhenName
                    = xDoc->createAttribute("name");
                xHiddenWhenName->setValue("hidden-when");
                xHiddenWhenAttr->setAttributeNode(xHiddenWhenName);
                xHiddenWhenAttr->appendChild(xDoc->createTextNode("action-missing"));
                xItem->appendChild(xHiddenWhenAttr);
            }
        }

        xChild = xNextChild;
    }

    return MenuEntry(bDrawAsRadio, xPropertyLabel);
}

struct ConvertResult
{
    bool m_bChildCanFocus;
    bool m_bHasVisible;
    bool m_bHasIconSize;
    bool m_bAlwaysShowImage;
    bool m_bUseUnderline;
    bool m_bVertOrientation;
    bool m_bXAlign;
    GtkPositionType m_eImagePos;
    css::uno::Reference<css::xml::dom::XNode> m_xPropertyLabel;
    css::uno::Reference<css::xml::dom::XNode> m_xPropertyIconName;

    ConvertResult(bool bChildCanFocus, bool bHasVisible, bool bHasIconSize, bool bAlwaysShowImage,
                  bool bUseUnderline, bool bVertOrientation, bool bXAlign,
                  GtkPositionType eImagePos,
                  const css::uno::Reference<css::xml::dom::XNode>& rPropertyLabel,
                  const css::uno::Reference<css::xml::dom::XNode>& rPropertyIconName)
        : m_bChildCanFocus(bChildCanFocus)
        , m_bHasVisible(bHasVisible)
        , m_bHasIconSize(bHasIconSize)
        , m_bAlwaysShowImage(bAlwaysShowImage)
        , m_bUseUnderline(bUseUnderline)
        , m_bVertOrientation(bVertOrientation)
        , m_bXAlign(bXAlign)
        , m_eImagePos(eImagePos)
        , m_xPropertyLabel(rPropertyLabel)
        , m_xPropertyIconName(rPropertyIconName)
    {
    }
};

bool IsAllowedBuiltInIcon(std::u16string_view iconName)
{
    // limit the named icons to those known by VclBuilder
    return VclBuilder::mapStockToSymbol(iconName) != SymbolType::DONTKNOW;
}

ConvertResult Convert3To4(const css::uno::Reference<css::xml::dom::XNode>& xNode)
{
    css::uno::Reference<css::xml::dom::XNodeList> xNodeList = xNode->getChildNodes();
    if (!xNodeList.is())
    {
        return ConvertResult(false, false, false, false, false, false, false, GTK_POS_LEFT, nullptr,
                             nullptr);
    }

    std::vector<css::uno::Reference<css::xml::dom::XNode>> xRemoveList;

    OUString sBorderWidth;
    bool bChildCanFocus = false;
    bool bHasVisible = false;
    bool bHasIconSize = false;
    bool bAlwaysShowImage = false;
    GtkPositionType eImagePos = GTK_POS_LEFT;
    bool bUseUnderline = false;
    bool bVertOrientation = false;
    bool bXAlign = false;
    css::uno::Reference<css::xml::dom::XNode> xPropertyLabel;
    css::uno::Reference<css::xml::dom::XNode> xPropertyIconName;
    css::uno::Reference<css::xml::dom::XNode> xCantFocus;

    css::uno::Reference<css::xml::dom::XElement> xGeneratedImageChild;

    css::uno::Reference<css::xml::dom::XNode> xChild = xNode->getFirstChild();
    while (xChild.is())
    {
        if (xChild->getNodeName() == "requires")
        {
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xLib = xMap->getNamedItem("lib");
            assert(xLib->getNodeValue() == "gtk+");
            xLib->setNodeValue("gtk");
            css::uno::Reference<css::xml::dom::XNode> xVersion = xMap->getNamedItem("version");
            assert(xVersion->getNodeValue() == "3.20");
            xVersion->setNodeValue("4.0");
        }
        else if (xChild->getNodeName() == "property")
        {
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xName = xMap->getNamedItem("name");
            OUString sName(xName->getNodeValue().replace('_', '-'));

            if (sName == "border-width")
                sBorderWidth = xChild->getFirstChild()->getNodeValue();

            if (sName == "has-default")
            {
                css::uno::Reference<css::xml::dom::XNamedNodeMap> xParentMap
                    = xChild->getParentNode()->getAttributes();
                css::uno::Reference<css::xml::dom::XNode> xId = xParentMap->getNamedItem("id");
                auto xDoc = xChild->getOwnerDocument();
                auto xDefaultWidget = CreateProperty(xDoc, "default-widget", xId->getNodeValue());
                SetPropertyOnTopLevel(xChild, xDefaultWidget);
                xRemoveList.push_back(xChild);
            }

            if (sName == "has-focus" || sName == "is-focus")
            {
                css::uno::Reference<css::xml::dom::XNamedNodeMap> xParentMap
                    = xChild->getParentNode()->getAttributes();
                css::uno::Reference<css::xml::dom::XNode> xId = xParentMap->getNamedItem("id");
                auto xDoc = xChild->getOwnerDocument();
                auto xDefaultWidget = CreateProperty(xDoc, "focus-widget", xId->getNodeValue());
                SetPropertyOnTopLevel(xChild, xDefaultWidget);
                xRemoveList.push_back(xChild);
            }

            if (sName == "can-focus")
            {
                bChildCanFocus = toBool(xChild->getFirstChild()->getNodeValue());
                if (!bChildCanFocus)
                {
                    OUString sParentClass = GetParentObjectType(xChild);
                    if (sParentClass == "GtkBox" || sParentClass == "GtkGrid"
                        || sParentClass == "GtkViewport")
                    {
                        // e.g. for the case of notebooks without children yet, just remove the can't focus property
                        // from Boxes and Grids
                        xRemoveList.push_back(xChild);
                    }
                    else if (sParentClass == "GtkComboBoxText")
                    {
                        // this was always a bit finicky in gtk3, fix it up to default to can-focus
                        xRemoveList.push_back(xChild);
                    }
                    else
                    {
                        // otherwise mark the property as needing removal if there turns out to be a child
                        // with can-focus of true, in which case remove this parent conflicting property
                        xCantFocus = xChild;
                    }
                }
            }

            if (sName == "label")
            {
                OUString sParentClass = GetParentObjectType(xChild);
                if (sParentClass == "GtkToolButton" || sParentClass == "GtkMenuToolButton"
                    || sParentClass == "GtkToggleToolButton")
                {
                    xName->setNodeValue("tooltip-text");
                }
                xPropertyLabel = xChild;
            }

            if (sName == "visible")
                bHasVisible = true;

            if (sName == "icon-name")
                xPropertyIconName = xChild;

            if (sName == "show-arrow")
                xRemoveList.push_back(xChild);

            if (sName == "events")
                xRemoveList.push_back(xChild);

            if (sName == "constrain-to")
                xRemoveList.push_back(xChild);

            if (sName == "activates-default")
            {
                if (GetParentObjectType(xChild) == "GtkSpinButton")
                    xRemoveList.push_back(xChild);
            }

            if (sName == "width-chars")
            {
                if (GetParentObjectType(xChild) == "GtkEntry")
                {
                    // I don't quite get what the difference should be wrt width-chars and max-width-chars
                    // but glade doesn't write max-width-chars and in gtk4 where we have width-chars, e.g
                    // print dialog, then max-width-chars gives the effect we wanted with width-chars
                    auto xDoc = xChild->getOwnerDocument();
                    auto mMaxWidthChars = CreateProperty(xDoc, "max-width-chars",
                                                         xChild->getFirstChild()->getNodeValue());
                    xChild->getParentNode()->insertBefore(mMaxWidthChars, xChild);
                }
            }

            // remove 'Help' button label and replace with a help icon instead. Unless the toplevel is a message dialog
            if (sName == "label" && GetParentObjectType(xChild) == "GtkButton"
                && !ToplevelIsMessageDialog(xChild))
            {
                css::uno::Reference<css::xml::dom::XNamedNodeMap> xParentMap
                    = xChild->getParentNode()->getAttributes();
                css::uno::Reference<css::xml::dom::XNode> xId = xParentMap->getNamedItem("id");
                if (xId && xId->getNodeValue() == "help")
                {
                    auto xDoc = xChild->getOwnerDocument();
                    auto xIconName = CreateProperty(xDoc, "icon-name", "help-browser-symbolic");
                    xChild->getParentNode()->insertBefore(xIconName, xChild);
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "icon-size")
            {
                if (GetParentObjectType(xChild) == "GtkImage")
                {
                    bHasIconSize = true;

                    OUString sSize = xChild->getFirstChild()->getNodeValue();
                    /*
                      old:
                       3 -> GTK_ICON_SIZE_LARGE_TOOLBAR: Size appropriate for large toolbars (24px)
                       5 -> GTK_ICON_SIZE_DND: Size appropriate for drag and drop (32px)
                       6 -> GTK_ICON_SIZE_DIALOG: Size appropriate for dialogs (48px)

                      new:
                       2 -> GTK_ICON_SIZE_LARGE
                    */
                    if (sSize == "3" || sSize == "5" || sSize == "6")
                    {
                        auto xDoc = xChild->getOwnerDocument();
                        auto xIconSize = CreateProperty(xDoc, "icon-size", "2");
                        xChild->getParentNode()->insertBefore(xIconSize, xChild);
                    }

                    xRemoveList.push_back(xChild);
                }

                if (GetParentObjectType(xChild) == "GtkToolbar")
                    xRemoveList.push_back(xChild);
            }

            if (sName == "truncate-multiline")
            {
                if (GetParentObjectType(xChild) == "GtkSpinButton")
                    xRemoveList.push_back(xChild);
            }

            if (sName == "toolbar-style")
            {
                // is there an equivalent for this ?
                xRemoveList.push_back(xChild);
            }

            if (sName == "homogeneous")
            {
                // e.g. the buttonbox in xml filter dialog
                if (GetParentObjectType(xChild) == "GtkButtonBox")
                    xRemoveList.push_back(xChild);
            }

            if (sName == "shadow-type")
            {
                if (GetParentObjectType(xChild) == "GtkFrame")
                    xRemoveList.push_back(xChild);
                else if (GetParentObjectType(xChild) == "GtkScrolledWindow")
                {
                    bool bHasFrame = xChild->getFirstChild()->getNodeValue() != "none";
                    auto xDoc = xChild->getOwnerDocument();
                    auto xHasFrame = CreateProperty(
                        xDoc, "has-frame", bHasFrame ? OUString("True") : OUString("False"));
                    xChild->getParentNode()->insertBefore(xHasFrame, xChild);
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "always-show-image")
            {
                if (GetParentObjectType(xChild) == "GtkButton"
                    || GetParentObjectType(xChild) == "GtkMenuButton"
                    || GetParentObjectType(xChild) == "GtkToggleButton")
                {
                    // we will turn always-show-image into a GtkBox child for
                    // GtkButton and a GtkLabel child for the GtkBox and move
                    // the label property into it.
                    bAlwaysShowImage = toBool(xChild->getFirstChild()->getNodeValue());
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "image-position")
            {
                if (GetParentObjectType(xChild) == "GtkButton")
                {
                    // we will turn always-show-image into a GtkBox child for
                    // GtkButton and a GtkLabel child for the GtkBox and move
                    // the label property into it.
                    OUString sImagePos = xChild->getFirstChild()->getNodeValue();
                    if (sImagePos == "top")
                        eImagePos = GTK_POS_TOP;
                    else if (sImagePos == "bottom")
                        eImagePos = GTK_POS_BOTTOM;
                    else if (sImagePos == "right")
                        eImagePos = GTK_POS_RIGHT;
                    else
                        assert(sImagePos == "left");
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "use-underline")
                bUseUnderline = toBool(xChild->getFirstChild()->getNodeValue());

            if (sName == "orientation")
                bVertOrientation = xChild->getFirstChild()->getNodeValue() == "vertical";

            if (sName == "relief")
            {
                if (GetParentObjectType(xChild) == "GtkToggleButton"
                    || GetParentObjectType(xChild) == "GtkMenuButton"
                    || GetParentObjectType(xChild) == "GtkLinkButton"
                    || GetParentObjectType(xChild) == "GtkButton")
                {
                    assert(xChild->getFirstChild()->getNodeValue() == "none");
                    auto xDoc = xChild->getOwnerDocument();
                    auto xHasFrame = CreateProperty(xDoc, "has-frame", "False");
                    xChild->getParentNode()->insertBefore(xHasFrame, xChild);
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "xalign")
            {
                if (GetParentObjectType(xChild) == "GtkLinkButton"
                    || GetParentObjectType(xChild) == "GtkMenuButton"
                    || GetParentObjectType(xChild) == "GtkButton")
                {
                    // TODO expand into a GtkLabel child with alignment on that instead
                    assert(xChild->getFirstChild()->getNodeValue() == "0");
                    bXAlign = true;
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "use-popover")
            {
                if (GetParentObjectType(xChild) == "GtkMenuButton")
                    xRemoveList.push_back(xChild);
            }

            if (sName == "hscrollbar-policy")
            {
                if (GetParentObjectType(xChild) == "GtkScrolledWindow")
                {
                    if (xChild->getFirstChild()->getNodeValue() == "never")
                    {
                        auto xDoc = xChild->getOwnerDocument();
                        auto xHasFrame = CreateProperty(xDoc, "propagate-natural-width", "True");
                        xChild->getParentNode()->insertBefore(xHasFrame, xChild);
                    }
                }
            }

            if (sName == "vscrollbar-policy")
            {
                if (GetParentObjectType(xChild) == "GtkScrolledWindow")
                {
                    if (xChild->getFirstChild()->getNodeValue() == "never")
                    {
                        auto xDoc = xChild->getOwnerDocument();
                        auto xHasFrame = CreateProperty(xDoc, "propagate-natural-height", "True");
                        xChild->getParentNode()->insertBefore(xHasFrame, xChild);
                    }
                }
            }

            if (sName == "popup")
            {
                if (GetParentObjectType(xChild) == "GtkMenuButton")
                {
                    OUString sMenuName = xChild->getFirstChild()->getNodeValue();
                    auto xDoc = xChild->getOwnerDocument();
                    auto xPopover = CreateProperty(xDoc, "popover", sMenuName);
                    xChild->getParentNode()->insertBefore(xPopover, xChild);
                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "image")
            {
                if (GetParentObjectType(xChild) == "GtkButton"
                    || GetParentObjectType(xChild) == "GtkMenuButton"
                    || GetParentObjectType(xChild) == "GtkToggleButton")
                {
                    // find the image object, expected to be a child of "interface"
                    auto xObjectCandidate = xChild->getParentNode();
                    if (xObjectCandidate->getNodeName() == "object")
                    {
                        OUString sImageId = xChild->getFirstChild()->getNodeValue();

                        css::uno::Reference<css::xml::dom::XNode> xRootCandidate
                            = xChild->getParentNode();
                        while (xRootCandidate)
                        {
                            if (xRootCandidate->getNodeName() == "interface")
                                break;
                            xRootCandidate = xRootCandidate->getParentNode();
                        }

                        css::uno::Reference<css::xml::dom::XNode> xImageNode;

                        for (auto xImageCandidate = xRootCandidate->getFirstChild();
                             xImageCandidate.is();
                             xImageCandidate = xImageCandidate->getNextSibling())
                        {
                            css::uno::Reference<css::xml::dom::XNamedNodeMap> xImageCandidateMap
                                = xImageCandidate->getAttributes();
                            if (!xImageCandidateMap.is())
                                continue;
                            css::uno::Reference<css::xml::dom::XNode> xId
                                = xImageCandidateMap->getNamedItem("id");
                            if (xId && xId->getNodeValue() == sImageId)
                            {
                                xImageNode = xImageCandidate;
                                break;
                            }
                        }

                        auto xDoc = xChild->getOwnerDocument();

                        // relocate it to be a child of this GtkButton
                        xGeneratedImageChild = xDoc->createElement("child");
                        xGeneratedImageChild->appendChild(
                            xImageNode->getParentNode()->removeChild(xImageNode));
                        xObjectCandidate->appendChild(xGeneratedImageChild);
                    }

                    xRemoveList.push_back(xChild);
                }
            }

            if (sName == "draw-indicator")
            {
                assert(toBool(xChild->getFirstChild()->getNodeValue()));
                if (GetParentObjectType(xChild) == "GtkMenuButton" && gtk_get_minor_version() >= 4)
                {
                    auto xDoc = xChild->getOwnerDocument();
                    auto xAlwaysShowArrow = CreateProperty(xDoc, "always-show-arrow", "True");
                    xChild->getParentNode()->insertBefore(xAlwaysShowArrow, xChild);
                }
                xRemoveList.push_back(xChild);
            }

            if (sName == "type-hint" || sName == "skip-taskbar-hint" || sName == "can-default"
                || sName == "border-width" || sName == "layout-style" || sName == "no-show-all"
                || sName == "ignore-hidden" || sName == "window-position")
            {
                xRemoveList.push_back(xChild);
            }
        }
        else if (xChild->getNodeName() == "child")
        {
            bool bContentArea = false;

            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xName = xMap->getNamedItem("internal-child");
            if (xName)
            {
                OUString sName(xName->getNodeValue());
                if (sName == "vbox")
                {
                    xName->setNodeValue("content_area");
                    bContentArea = true;
                }
                else if (sName == "accessible")
                {
                    // TODO what's the replacement for this going to be?
                    xRemoveList.push_back(xChild);
                }
            }

            if (bContentArea)
            {
                css::uno::Reference<css::xml::dom::XNode> xObject = GetChildObject(xChild);
                if (xObject)
                {
                    auto xDoc = xChild->getOwnerDocument();

                    auto xVExpand = CreateProperty(xDoc, "vexpand", "True");
                    insertAsFirstChild(xObject, xVExpand);

                    if (!sBorderWidth.isEmpty())
                    {
                        AddBorderAsMargins(xObject, sBorderWidth);
                        sBorderWidth.clear();
                    }
                }
            }
        }
        else if (xChild->getNodeName() == "packing")
        {
            // remove "packing" and if its grid packing insert a replacement "layout" into
            // the associated "object"
            auto xDoc = xChild->getOwnerDocument();
            css::uno::Reference<css::xml::dom::XElement> xNew = xDoc->createElement("layout");

            bool bGridPacking = false;

            // iterate over all children and append them to the new element
            for (css::uno::Reference<css::xml::dom::XNode> xCurrent = xChild->getFirstChild();
                 xCurrent.is(); xCurrent = xChild->getFirstChild())
            {
                css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xCurrent->getAttributes();
                if (xMap.is())
                {
                    css::uno::Reference<css::xml::dom::XNode> xName = xMap->getNamedItem("name");
                    OUString sName(xName->getNodeValue().replace('_', '-'));
                    if (sName == "left-attach")
                    {
                        xName->setNodeValue("column");
                        bGridPacking = true;
                    }
                    else if (sName == "top-attach")
                    {
                        xName->setNodeValue("row");
                        bGridPacking = true;
                    }
                    else if (sName == "width")
                    {
                        xName->setNodeValue("column-span");
                        bGridPacking = true;
                    }
                    else if (sName == "height")
                    {
                        xName->setNodeValue("row-span");
                        bGridPacking = true;
                    }
                    else if (sName == "secondary")
                    {
                        // turn parent tag of <child> into <child type="start">
                        auto xParent = xChild->getParentNode();
                        css::uno::Reference<css::xml::dom::XAttr> xTypeStart
                            = xDoc->createAttribute("type");
                        xTypeStart->setValue("start");
                        css::uno::Reference<css::xml::dom::XElement> xElem(
                            xParent, css::uno::UNO_QUERY_THROW);
                        xElem->setAttributeNode(xTypeStart);
                    }
                    else if (sName == "pack-type")
                    {
                        // turn parent tag of <child> into <child type="start">
                        auto xParent = xChild->getParentNode();

                        css::uno::Reference<css::xml::dom::XNamedNodeMap> xParentMap
                            = xParent->getAttributes();
                        css::uno::Reference<css::xml::dom::XNode> xParentType
                            = xParentMap->getNamedItem("type");
                        assert(!xParentType || xParentType->getNodeValue() == "titlebar");
                        if (!xParentType)
                        {
                            css::uno::Reference<css::xml::dom::XAttr> xTypeStart
                                = xDoc->createAttribute("type");
                            xTypeStart->setValue(xCurrent->getFirstChild()->getNodeValue());
                            css::uno::Reference<css::xml::dom::XElement> xElem(
                                xParent, css::uno::UNO_QUERY_THROW);
                            xElem->setAttributeNode(xTypeStart);
                        }
                    }
                }
                xNew->appendChild(xChild->removeChild(xCurrent));
            }

            if (bGridPacking)
            {
                // go back to parent and find the object child and insert this "layout" as a
                // new child of the object
                auto xParent = xChild->getParentNode();
                css::uno::Reference<css::xml::dom::XNode> xObject = GetChildObject(xParent);
                if (xObject)
                    xObject->appendChild(xNew);
            }

            xRemoveList.push_back(xChild);
        }
        else if (xChild->getNodeName() == "accessibility")
        {
            // TODO <relation type="labelled-by" target="pagenumcb"/> -> <relation name="labelled-by">pagenumcb</relation>
            xRemoveList.push_back(xChild);
        }
        else if (xChild->getNodeName() == "accelerator")
        {
            // TODO is anything like this supported anymore in .ui files
            xRemoveList.push_back(xChild);
        }

        auto xNextChild = xChild->getNextSibling();

        if (xChild->getNodeName() == "object")
        {
            auto xDoc = xChild->getOwnerDocument();

            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xMap->getNamedItem("class");
            OUString sClass(xClass->getNodeValue());

            if (sClass == "GtkMenu")
            {
                css::uno::Reference<css::xml::dom::XNode> xId = xMap->getNamedItem("id");
                OUString sId(xId->getNodeValue() + "-menu-model");

                // <menu id='menubar'>
                css::uno::Reference<css::xml::dom::XElement> xMenu = xDoc->createElement("menu");
                css::uno::Reference<css::xml::dom::XAttr> xIdAttr = xDoc->createAttribute("id");
                xIdAttr->setValue(sId);
                xMenu->setAttributeNode(xIdAttr);
                xChild->getParentNode()->insertBefore(xMenu, xChild);

                css::uno::Reference<css::xml::dom::XElement> xSection
                    = xDoc->createElement("section");
                xMenu->appendChild(xSection);

                css::uno::Reference<css::xml::dom::XNode> xMenuSection(xSection);
                ConvertMenu(xMenuSection, xChild);

                // now remove GtkMenu contents
                while (true)
                {
                    auto xFirstChild = xChild->getFirstChild();
                    if (!xFirstChild.is())
                        break;
                    xChild->removeChild(xFirstChild);
                }

                // change to GtkPopoverMenu
                xClass->setNodeValue("GtkPopoverMenu");

                // <property name="menu-model">
                xChild->appendChild(CreateProperty(xDoc, "menu-model", sId));
                xChild->appendChild(CreateProperty(xDoc, "visible", "False"));
            }
        }

        bool bChildHasIconSize = false;
        bool bChildHasVisible = false;
        bool bChildAlwaysShowImage = false;
        GtkPositionType eChildImagePos = GTK_POS_LEFT;
        bool bChildUseUnderline = false;
        bool bChildVertOrientation = false;
        bool bChildXAlign = false;
        css::uno::Reference<css::xml::dom::XNode> xChildPropertyLabel;
        css::uno::Reference<css::xml::dom::XNode> xChildPropertyIconName;
        if (xChild->hasChildNodes() && xChild != xGeneratedImageChild)
        {
            auto aChildRes = Convert3To4(xChild);
            bChildCanFocus |= aChildRes.m_bChildCanFocus;
            if (bChildCanFocus && xCantFocus.is())
            {
                xNode->removeChild(xCantFocus);
                xCantFocus.clear();
            }
            if (xChild->getNodeName() == "object")
            {
                bChildHasVisible = aChildRes.m_bHasVisible;
                bChildHasIconSize = aChildRes.m_bHasIconSize;
                bChildAlwaysShowImage = aChildRes.m_bAlwaysShowImage;
                eChildImagePos = aChildRes.m_eImagePos;
                bChildUseUnderline = aChildRes.m_bUseUnderline;
                bChildVertOrientation = aChildRes.m_bVertOrientation;
                bChildXAlign = aChildRes.m_bXAlign;
                xChildPropertyLabel = aChildRes.m_xPropertyLabel;
                xChildPropertyIconName = aChildRes.m_xPropertyIconName;
            }
        }

        if (xChild->getNodeName() == "object")
        {
            auto xDoc = xChild->getOwnerDocument();

            css::uno::Reference<css::xml::dom::XNamedNodeMap> xMap = xChild->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xClass = xMap->getNamedItem("class");
            OUString sClass(xClass->getNodeValue());

            auto xInternalChildCandidate = xChild->getParentNode();
            css::uno::Reference<css::xml::dom::XNamedNodeMap> xInternalChildCandidateMap
                = xInternalChildCandidate->getAttributes();
            css::uno::Reference<css::xml::dom::XNode> xInternalChild
                = xInternalChildCandidateMap->getNamedItem("internal-child");

            // turn default gtk3 invisibility for widget objects into explicit invisible, but ignore internal-children
            if (!bChildHasVisible && !xInternalChild)
            {
                if (sClass == "GtkBox" || sClass == "GtkButton" || sClass == "GtkCalendar"
                    || sClass == "GtkCheckButton" || sClass == "GtkRadioButton"
                    || sClass == "GtkComboBox" || sClass == "GtkComboBoxText"
                    || sClass == "GtkDrawingArea" || sClass == "GtkEntry" || sClass == "GtkExpander"
                    || sClass == "GtkFrame" || sClass == "GtkGrid" || sClass == "GtkImage"
                    || sClass == "GtkLabel" || sClass == "GtkMenuButton" || sClass == "GtkNotebook"
                    || sClass == "GtkOverlay" || sClass == "GtkPaned" || sClass == "GtkProgressBar"
                    || sClass == "GtkScrolledWindow" || sClass == "GtkSeparator"
                    || sClass == "GtkSpinButton" || sClass == "GtkSpinner"
                    || sClass == "GtkTextView" || sClass == "GtkTreeView" || sClass == "GtkViewport"
                    || sClass == "GtkLinkButton" || sClass == "GtkToggleButton"
                    || sClass == "GtkButtonBox")

                {
                    auto xVisible = CreateProperty(xDoc, "visible", "False");
                    insertAsFirstChild(xChild, xVisible);
                }
            }

            if (sClass == "GtkButtonBox")
            {
                if (xInternalChild && xInternalChild->getNodeValue() == "action_area"
                    && !ToplevelIsMessageDialog(xChild))
                {
                    xClass->setNodeValue("GtkHeaderBar");
                    auto xSpacingNode = CreateProperty(xDoc, "show-title-buttons", "False");
                    insertAsFirstChild(xChild, xSpacingNode);

                    // move the replacement GtkHeaderBar up to before the content_area
                    auto xContentAreaCandidate = xChild->getParentNode();
                    while (xContentAreaCandidate)
                    {
                        css::uno::Reference<css::xml::dom::XNamedNodeMap> xChildMap
                            = xContentAreaCandidate->getAttributes();
                        css::uno::Reference<css::xml::dom::XNode> xName
                            = xChildMap->getNamedItem("internal-child");
                        if (xName && xName->getNodeValue() == "content_area")
                        {
                            auto xActionArea = xChild->getParentNode();

                            xActionArea->getParentNode()->removeChild(xActionArea);

                            css::uno::Reference<css::xml::dom::XAttr> xTypeTitleBar
                                = xDoc->createAttribute("type");
                            xTypeTitleBar->setValue("titlebar");
                            css::uno::Reference<css::xml::dom::XElement> xElem(
                                xActionArea, css::uno::UNO_QUERY_THROW);
                            xElem->setAttributeNode(xTypeTitleBar);
                            xElem->removeAttribute("internal-child");

                            xContentAreaCandidate->getParentNode()->insertBefore(
                                xActionArea, xContentAreaCandidate);

                            std::vector<named_node> aChildren;

                            css::uno::Reference<css::xml::dom::XNode> xTitleChild
                                = xChild->getFirstChild();
                            while (xTitleChild.is())
                            {
                                auto xNextTitleChild = xTitleChild->getNextSibling();
                                if (xTitleChild->getNodeName() == "child")
                                {
                                    OUString sNodeId;

                                    css::uno::Reference<css::xml::dom::XNode> xObject
                                        = GetChildObject(xTitleChild);
                                    if (xObject)
                                    {
                                        css::uno::Reference<css::xml::dom::XNamedNodeMap> xObjectMap
                                            = xObject->getAttributes();
                                        css::uno::Reference<css::xml::dom::XNode> xObjectId
                                            = xObjectMap->getNamedItem("id");
                                        sNodeId = xObjectId->getNodeValue();
                                    }

                                    aChildren.push_back(std::make_pair(xTitleChild, sNodeId));
                                }
                                else if (xTitleChild->getNodeName() == "property")
                                {
                                    // remove any <property name="homogeneous"> tag
                                    css::uno::Reference<css::xml::dom::XNamedNodeMap> xTitleChildMap
                                        = xTitleChild->getAttributes();
                                    css::uno::Reference<css::xml::dom::XNode> xPropName
                                        = xTitleChildMap->getNamedItem("name");
                                    OUString sPropName(xPropName->getNodeValue().replace('_', '-'));
                                    if (sPropName == "homogeneous")
                                        xChild->removeChild(xTitleChild);
                                }

                                xTitleChild = xNextTitleChild;
                            }

                            //sort child order within parent so that we match the platform button order
                            std::stable_sort(aChildren.begin(), aChildren.end(), sortButtonNodes);

                            int nNonHelpButtonCount = 0;

                            for (const auto& rTitleChild : aChildren)
                            {
                                xChild->removeChild(rTitleChild.first);
                                if (rTitleChild.second != "help")
                                    ++nNonHelpButtonCount;
                            }

                            std::reverse(aChildren.begin(), aChildren.end());

                            for (const auto& rTitleChild : aChildren)
                            {
                                xChild->appendChild(rTitleChild.first);

                                css::uno::Reference<css::xml::dom::XElement> xChildElem(
                                    rTitleChild.first, css::uno::UNO_QUERY_THROW);
                                if (!xChildElem->hasAttribute("type"))
                                {
                                    // turn parent tag of <child> into <child type="end"> except for cancel/close which we'll
                                    // put at start unless there is nothing at end
                                    css::uno::Reference<css::xml::dom::XAttr> xTypeEnd
                                        = xDoc->createAttribute("type");
                                    if (nNonHelpButtonCount >= 2
                                        && (rTitleChild.second == "cancel"
                                            || rTitleChild.second == "close"))
                                        xTypeEnd->setValue("start");
                                    else
                                        xTypeEnd->setValue("end");
                                    xChildElem->setAttributeNode(xTypeEnd);
                                }
                            }

                            auto xUseHeaderBar = CreateProperty(xDoc, "use-header-bar", "1");
                            SetPropertyOnTopLevel(xContentAreaCandidate, xUseHeaderBar);

                            break;
                        }
                        xContentAreaCandidate = xContentAreaCandidate->getParentNode();
                    }
                }
                else // GtkMessageDialog
                    xClass->setNodeValue("GtkBox");
            }
            else if (sClass == "GtkToolbar")
            {
                xClass->setNodeValue("GtkBox");
                css::uno::Reference<css::xml::dom::XElement> xStyle = xDoc->createElement("style");
                css::uno::Reference<css::xml::dom::XElement> xToolbarClass
                    = xDoc->createElement("class");
                css::uno::Reference<css::xml::dom::XAttr> xPropName = xDoc->createAttribute("name");
                xPropName->setValue("toolbar");
                xToolbarClass->setAttributeNode(xPropName);
                xStyle->appendChild(xToolbarClass);
                xChild->appendChild(xStyle);
            }
            else if (sClass == "GtkToolButton")
            {
                xClass->setNodeValue("GtkButton");
            }
            else if (sClass == "GtkToolItem")
            {
                xClass->setNodeValue("GtkBox");
            }
            else if (sClass == "GtkMenuToolButton")
            {
                xClass->setNodeValue("GtkMenuButton");
                if (gtk_get_minor_version() >= 4)
                {
                    auto xAlwaysShowArrow = CreateProperty(xDoc, "always-show-arrow", "True");
                    insertAsFirstChild(xChild, xAlwaysShowArrow);
                }
            }
            else if (sClass == "GtkRadioToolButton")
            {
                xClass->setNodeValue("GtkCheckButton");
            }
            else if (sClass == "GtkToggleToolButton")
            {
                xClass->setNodeValue("GtkToggleButton");
            }
            else if (sClass == "GtkSeparatorToolItem")
            {
                xClass->setNodeValue("GtkSeparator");
            }
            else if (sClass == "GtkBox")
            {
                // reverse the order of the pack-type=end widgets
                std::vector<css::uno::Reference<css::xml::dom::XNode>> aPackEnds;
                std::vector<css::uno::Reference<css::xml::dom::XNode>> aPackStarts;
                css::uno::Reference<css::xml::dom::XNode> xBoxChild = xChild->getFirstChild();
                while (xBoxChild.is())
                {
                    auto xNextBoxChild = xBoxChild->getNextSibling();

                    if (xBoxChild->getNodeName() == "child")
                    {
                        css::uno::Reference<css::xml::dom::XNamedNodeMap> xBoxChildMap
                            = xBoxChild->getAttributes();
                        css::uno::Reference<css::xml::dom::XNode> xType
                            = xBoxChildMap->getNamedItem("type");
                        if (xType && xType->getNodeValue() == "end")
                            aPackEnds.push_back(xChild->removeChild(xBoxChild));
                        else
                            aPackStarts.push_back(xBoxChild);
                    }

                    xBoxChild = xNextBoxChild;
                }

                if (!aPackEnds.empty())
                {
                    std::reverse(aPackEnds.begin(), aPackEnds.end());

                    if (!bChildVertOrientation)
                    {
                        bool bHasStartObject = false;
                        bool bLastStartExpands = false;
                        if (!aPackStarts.empty())
                        {
                            css::uno::Reference<css::xml::dom::XNode> xLastStartObject;
                            for (auto it = aPackStarts.rbegin(); it != aPackStarts.rend(); ++it)
                            {
                                xLastStartObject = GetChildObject(*it);
                                if (xLastStartObject.is())
                                {
                                    bHasStartObject = true;
                                    for (css::uno::Reference<css::xml::dom::XNode> xExpandCandidate
                                         = xLastStartObject->getFirstChild();
                                         xExpandCandidate.is();
                                         xExpandCandidate = xExpandCandidate->getNextSibling())
                                    {
                                        if (xExpandCandidate->getNodeName() == "property")
                                        {
                                            css::uno::Reference<css::xml::dom::XNamedNodeMap>
                                                xExpandMap = xExpandCandidate->getAttributes();
                                            css::uno::Reference<css::xml::dom::XNode> xName
                                                = xExpandMap->getNamedItem("name");
                                            OUString sPropName(xName->getNodeValue());
                                            if (sPropName == "hexpand")
                                            {
                                                bLastStartExpands
                                                    = toBool(xExpandCandidate->getFirstChild()
                                                                 ->getNodeValue());
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }

                        if (bHasStartObject && !bLastStartExpands)
                        {
                            auto xAlign = CreateProperty(xDoc, "halign", "end");
                            insertAsFirstChild(GetChildObject(aPackEnds[0]), xAlign);
                            auto xExpand = CreateProperty(xDoc, "hexpand", "True");
                            insertAsFirstChild(GetChildObject(aPackEnds[0]), xExpand);
                        }
                    }

                    for (auto& xPackEnd : aPackEnds)
                        xChild->appendChild(xPackEnd);
                }
            }
            else if (sClass == "GtkRadioButton")
            {
                xClass->setNodeValue("GtkCheckButton");
            }
            else if (sClass == "GtkImage")
            {
                /* a) keep symbolic icon-names as GtkImage, e.g. writer, format, columns, next/prev
                      buttons
                   b) assume that an explicit icon-size represents a request for a scaled icon
                      so keep those as GtkImage. e.g. hyperlink dialog notebook tab images and
                      calc paste special button images
                   c) turn everything else into a GtkPicture, e.g. help, about. If a GtkPicture
                      ends up used to display just a simple icon that's generally not a problem.
                */
                bool bKeepAsImage = false;
                if (bChildHasIconSize)
                    bKeepAsImage = true;
                else if (xChildPropertyIconName.is())
                {
                    OUString sIconName(xChildPropertyIconName->getFirstChild()->getNodeValue());
                    bool bHasSymbolicIconName = IsAllowedBuiltInIcon(sIconName);
                    if (bHasSymbolicIconName)
                    {
                        if (sIconName != "missing-image")
                            bKeepAsImage = true;
                        else
                        {
                            // If the symbolic icon-name is missing-image then decide to make
                            // it a GtkPicture if it has a parent widget and keep it as GtkImage
                            // if it has just the root "interface" as parent.
                            // for e.g. view, user interface
                            css::uno::Reference<css::xml::dom::XNode> xParent
                                = xChild->getParentNode();
                            bKeepAsImage = xParent->getNodeName() == "interface";
                            if (!bKeepAsImage)
                                xChild->removeChild(xChildPropertyIconName);
                        }
                    }
                    else
                    {
                        // private:graphicrepository/ would be turned by gio (?) into private:///graphicrepository/
                        // so use private:///graphicrepository/ here. At the moment we just want this to be transported
                        // as-is to postprocess_widget. Though it might be nice to register a protocol handler with gio
                        // to avoid us doing the load in that second pass.
                        auto xUri = CreateProperty(xDoc, "file",
                                                   "private:///graphicrepository/" + sIconName);
                        xChild->insertBefore(xUri, xChildPropertyIconName);
                        // calc, insert, header and footer, custom header menubutton icon
                        auto xCanShrink = CreateProperty(xDoc, "can-shrink", "False");
                        xChild->insertBefore(xCanShrink, xChildPropertyIconName);
                        xChild->removeChild(xChildPropertyIconName);
                    }
                }
                if (!bKeepAsImage)
                    xClass->setNodeValue("GtkPicture");
            }
            else if (sClass == "GtkPopover" && !bHasVisible)
            {
                auto xVisible = CreateProperty(xDoc, "visible", "False");
                insertAsFirstChild(xChild, xVisible);
            }

            // only create the child box for GtkButton/GtkToggleButton
            if (bChildAlwaysShowImage)
            {
                auto xImageCandidateNode = xChild->getLastChild();
                if (xImageCandidateNode && xImageCandidateNode->getNodeName() != "child")
                    xImageCandidateNode.clear();
                if (xImageCandidateNode)
                    xChild->removeChild(xImageCandidateNode);

                // for GtkMenuButton if this is a gearmenu with just an icon
                // then "icon-name" is used for the indicator and there is
                // expected to be no text. If there is a GtkPicture then treat
                // this like a GtkButton and presumably it's a ToggleMenuButton
                // and the relocation of contents happens in the builder
                if (sClass == "GtkMenuButton")
                {
                    bChildAlwaysShowImage = false;
                    if (xImageCandidateNode)
                    {
                        bChildAlwaysShowImage = true;
                        auto xImageObject = GetChildObject(xImageCandidateNode);
                        auto xProp = xImageObject->getFirstChild();
                        while (xProp.is())
                        {
                            if (xProp->getNodeName() == "property")
                            {
                                css::uno::Reference<css::xml::dom::XNamedNodeMap> xPropMap
                                    = xProp->getAttributes();
                                css::uno::Reference<css::xml::dom::XNode> xPropName
                                    = xPropMap->getNamedItem("name");
                                OUString sPropName(xPropName->getNodeValue().replace('_', '-'));
                                if (sPropName == "icon-name")
                                {
                                    OUString sIconName(xProp->getFirstChild()->getNodeValue());
                                    auto xIconName = CreateProperty(xDoc, "icon-name", sIconName);
                                    insertAsFirstChild(xChild, xIconName);
                                    bChildAlwaysShowImage = false;
                                    break;
                                }
                            }

                            xProp = xProp->getNextSibling();
                        }
                    }
                }

                if (bChildAlwaysShowImage)
                {
                    css::uno::Reference<css::xml::dom::XElement> xNewChildNode
                        = xDoc->createElement("child");
                    css::uno::Reference<css::xml::dom::XElement> xNewObjectNode
                        = xDoc->createElement("object");
                    css::uno::Reference<css::xml::dom::XAttr> xBoxClassName
                        = xDoc->createAttribute("class");
                    xBoxClassName->setValue("GtkBox");
                    xNewObjectNode->setAttributeNode(xBoxClassName);

                    if (eChildImagePos == GTK_POS_TOP || eChildImagePos == GTK_POS_BOTTOM)
                    {
                        auto xOrientation = CreateProperty(xDoc, "orientation", "vertical");
                        xNewObjectNode->appendChild(xOrientation);
                    }

                    xNewObjectNode->appendChild(CreateProperty(xDoc, "spacing", "6"));
                    if (!bChildXAlign)
                        xNewObjectNode->appendChild(CreateProperty(xDoc, "halign", "center"));

                    xNewChildNode->appendChild(xNewObjectNode);

                    xChild->appendChild(xNewChildNode);

                    css::uno::Reference<css::xml::dom::XElement> xNewLabelChildNode
                        = xDoc->createElement("child");
                    css::uno::Reference<css::xml::dom::XElement> xNewChildObjectNode
                        = xDoc->createElement("object");
                    css::uno::Reference<css::xml::dom::XAttr> xLabelClassName
                        = xDoc->createAttribute("class");
                    xLabelClassName->setValue("GtkLabel");
                    xNewChildObjectNode->setAttributeNode(xLabelClassName);
                    if (xChildPropertyLabel)
                    {
                        xNewChildObjectNode->appendChild(
                            xChildPropertyLabel->getParentNode()->removeChild(xChildPropertyLabel));
                    }
                    else
                    {
                        auto xNotVisible = CreateProperty(xDoc, "visible", "False");
                        xNewChildObjectNode->appendChild(xNotVisible);
                    }
                    if (bChildUseUnderline)
                    {
                        auto xUseUnderline = CreateProperty(xDoc, "use-underline", "True");
                        xNewChildObjectNode->appendChild(xUseUnderline);
                    }
                    xNewLabelChildNode->appendChild(xNewChildObjectNode);

                    if (eChildImagePos == GTK_POS_LEFT || eChildImagePos == GTK_POS_TOP)
                    {
                        if (xImageCandidateNode)
                            xNewObjectNode->appendChild(xImageCandidateNode);
                        xNewObjectNode->appendChild(xNewLabelChildNode);
                    }
                    else
                    {
                        xNewObjectNode->appendChild(xNewLabelChildNode);
                        if (xImageCandidateNode)
                            xNewObjectNode->appendChild(xImageCandidateNode);
                    }
                }
            }
        }

        xChild = xNextChild;
    }

    if (!sBorderWidth.isEmpty())
        AddBorderAsMargins(xNode, sBorderWidth);

    for (auto& xRemove : xRemoveList)
        xNode->removeChild(xRemove);

    // https://gitlab.gnome.org/GNOME/gtk/-/issues/4041 double encode ampersands if use-underline is used
    if (gtk_check_version(4, 3, 2) != nullptr)
    {
        if (xPropertyLabel)
        {
            auto xLabelText = xPropertyLabel->getFirstChild();
            if (xLabelText.is())
            {
                OString sText = xLabelText->getNodeValue().toUtf8();
                gchar* pText = g_markup_escape_text(sText.getStr(), sText.getLength());
                xLabelText->setNodeValue(OUString(pText, strlen(pText), RTL_TEXTENCODING_UTF8));
                g_free(pText);
            }
        }
    }

    return ConvertResult(bChildCanFocus, bHasVisible, bHasIconSize, bAlwaysShowImage, bUseUnderline,
                         bVertOrientation, bXAlign, eImagePos, xPropertyLabel, xPropertyIconName);
}
}

void builder_add_from_gtk3_file(GtkBuilder* pBuilder, const OUString& rUri)
{
    GError* err = nullptr;

    // load the xml
    css::uno::Reference<css::uno::XComponentContext> xContext
        = ::comphelper::getProcessComponentContext();
    css::uno::Reference<css::xml::dom::XDocumentBuilder> xBuilder
        = css::xml::dom::DocumentBuilder::create(xContext);
    css::uno::Reference<css::xml::dom::XDocument> xDocument = xBuilder->parseURI(rUri);

    // convert it from gtk3 to gtk4
    Convert3To4(xDocument);

    css::uno::Reference<css::beans::XPropertySet> xTempFile(css::io::TempFile::create(xContext),
                                                            css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::io::XStream> xTempStream(xTempFile, css::uno::UNO_QUERY_THROW);
    xTempFile->setPropertyValue("RemoveFile", css::uno::makeAny(false));

    // serialize it back to xml
    css::uno::Reference<css::xml::sax::XSAXSerializable> xSerializer(xDocument,
                                                                     css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::xml::sax::XWriter> xWriter = css::xml::sax::Writer::create(xContext);
    css::uno::Reference<css::io::XOutputStream> xTempOut = xTempStream->getOutputStream();
    xWriter->setOutputStream(xTempOut);
    xSerializer->serialize(
        css::uno::Reference<css::xml::sax::XDocumentHandler>(xWriter, css::uno::UNO_QUERY_THROW),
        css::uno::Sequence<css::beans::StringPair>());

    // feed it to GtkBuilder
    css::uno::Reference<css::io::XSeekable> xTempSeek(xTempStream, css::uno::UNO_QUERY_THROW);
    xTempSeek->seek(0);
    auto xInput = xTempStream->getInputStream();
    css::uno::Sequence<sal_Int8> bytes;
    sal_Int32 nToRead = xInput->available();
    while (true)
    {
        sal_Int32 nRead = xInput->readBytes(bytes, std::max<sal_Int32>(nToRead, 4096));
        if (!nRead)
            break;
        // fprintf(stderr, "text is %s\n", reinterpret_cast<const gchar*>(bytes.getArray()));
        auto rc = gtk_builder_add_from_string(
            pBuilder, reinterpret_cast<const gchar*>(bytes.getArray()), nRead, &err);
        if (!rc)
        {
            SAL_WARN("vcl.gtk",
                     "GtkInstanceBuilder: error when calling gtk_builder_add_from_string: "
                         << err->message);
            g_error_free(err);
        }
        assert(rc && "could not load UI file");
        // in the real world the first loop has read the entire file because its all 'available' without blocking
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
