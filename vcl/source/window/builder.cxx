/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

VclBuilder::VclBuilder(Window *pParent, rtl::OUString sUri)
{
    xmlreader::XmlReader reader(sUri);

    handleChild(pParent, reader);

    for (std::vector<Window*>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        Window *pWindow = *aI;
        if (pWindow)
        {
            pWindow->Show();
        }
    }
}

VclBuilder::~VclBuilder()
{
    for (std::vector<Window*>::reverse_iterator aI = m_aChildren.rbegin(),
         aEnd = m_aChildren.rend(); aI != aEnd; ++aI)
    {
        Window *pWindow = *aI;
        delete pWindow;
    }
}

Window *VclBuilder::makeObject(Window *pParent, const rtl::OString &name, bool bVertical)
{
    Window *pWindow = NULL;
    if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkDialog")))
    {
        pWindow = new Dialog(pParent, WB_SIZEMOVE);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkBox")))
    {
        if (bVertical)
            pWindow = new VclVBox(pParent);
        else
            pWindow = new VclHBox(pParent);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkButtonBox")))
    {
        if (bVertical)
            pWindow = new VclVButtonBox(pParent);
        else
            pWindow = new VclHButtonBox(pParent);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkButton")))
    {
        pWindow = new PushButton(pParent, WB_CENTER|WB_VCENTER);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkRadioButton")))
    {
        pWindow = new RadioButton(pParent, WB_CENTER|WB_VCENTER);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkLabel")))
    {
        pWindow = new FixedText(pParent, WB_CENTER|WB_VCENTER);
    }
    else
    {
        fprintf(stderr, "TO-DO, implement %s\n", name.getStr());
    }
    fprintf(stderr, "for %s, created %p child of %p\n", name.getStr(), pWindow, pParent);
    return pWindow;
}

Window *VclBuilder::insertObject(Window *pParent, const rtl::OString &rClass, stringmap &rMap)
{
    bool bVertical = false;
    stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("orientation")));
    if (aFind != rMap.end())
        bVertical = aFind->second.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("vertical"));

    Window *pCurrentChild = makeObject(pParent, rClass, bVertical);
    if (!pCurrentChild)
    {
        fprintf(stderr, "missing object!\n");
    }

    if (pCurrentChild)
    {
        m_aChildren.push_back(pCurrentChild);

        for (stringmap::iterator aI = rMap.begin(), aEnd = rMap.end(); aI != aEnd; ++aI)
        {
            const rtl::OString &rKey = aI->first;
            const rtl::OString &rValue = aI->second;
            if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("label")))
                pCurrentChild->SetText(rtl::OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
            else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("xalign")))
            {
                WinBits nBits = pCurrentChild->GetStyle();
                nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

                float f = rValue.toFloat();
                if (f == 0.0)
                    nBits |= WB_LEFT;
                else if (f == 1.0)
                    nBits |= WB_RIGHT;
                else if (f == 0.5)
                    nBits |= WB_CENTER;

                pCurrentChild->SetStyle(nBits);
            }
            else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("yalign")))
            {
                WinBits nBits = pCurrentChild->GetStyle();
                nBits &= ~(WB_TOP | WB_VCENTER | WB_BOTTOM);

                float f = rValue.toFloat();
                if (f == 0.0)
                    nBits |= WB_TOP;
                else if (f == 1.0)
                    nBits |= WB_BOTTOM;
                else if (f == 0.5)
                    nBits |= WB_CENTER;

                pCurrentChild->SetStyle(nBits);
            }
            else if
                (
                    rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("expand")) ||
                    rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("fill"))
                )
            {
                bool bTrue = (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1');
                pCurrentChild->setChildProperty(rKey, bTrue);
            }
            else
                fprintf(stderr, "unhandled property %s\n", rKey.getStr());
        }
    }

    if (!pCurrentChild)
    {
        fprintf(stderr, "missing object!\n");
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back();
    }
    rMap.clear();
    return pCurrentChild;
}

void VclBuilder::handleChild(Window *pParent, xmlreader::XmlReader &reader)
{
    int nLevel = 1;

    while(1)
    {
        xmlreader::Span name;
        int nsId;
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("object")))
            {
                handleObject(pParent, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }
}

void VclBuilder::handleObject(Window *pParent, xmlreader::XmlReader &reader)
{
    rtl::OString sClass;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        rtl::OString sFoo(name.begin, name.length);

        if (name.equals(RTL_CONSTASCII_STRINGPARAM("class")))
        {
            name = reader.getAttributeValue(false);
            sClass = rtl::OString(name.begin, name.length);
        }
    }

    int nLevel = 1;

    stringmap aProperties;

    Window *pCurrentChild = NULL;
    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        rtl::OString sFoo(name.begin, name.length);

        fprintf(stderr, "level tag %d %s\n", nLevel, sFoo.getStr());

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("child")))
            {
                if (!pCurrentChild)
                    pCurrentChild = insertObject(pParent, sClass, aProperties);
                handleChild(pCurrentChild, reader);
            }
            else
            {
                ++nLevel;
                if (name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
                    collectProperty(reader, aProperties);
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    if (!pCurrentChild)
        insertObject(pParent, sClass, aProperties);

    fprintf(stderr, "finished %s\n", sClass.getStr());
}

void VclBuilder::collectProperty(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        rtl::OString sFoo(name.begin, name.length);

        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            name = reader.getAttributeValue(false);
            rtl::OString sProperty(name.begin, name.length);
            reader.nextItem(
                xmlreader::XmlReader::TEXT_NORMALIZED, &name, &nsId);
            rtl::OString sValue(name.begin, name.length);
            rMap[sProperty] = sValue;
        }
    }
}


Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? NULL : m_aChildren[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
