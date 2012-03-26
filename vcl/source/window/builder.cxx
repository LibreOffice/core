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
 * Portions created by the Initial Developer are Copyright (C) 2011 the
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

    while(1)
    {
        xmlreader::Span name;
        int nsId;
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("object")))
        {
            handleObject(pParent, reader);
        }

        rtl::OString sFoo(name.begin, name.length);
        fprintf(stderr, "interface level is %s\n", sFoo.getStr());
        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }

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

Window *VclBuilder::makeObject(Window *pParent, xmlreader::Span &name)
{
    Window *pWindow = NULL;
    if (name.equals(RTL_CONSTASCII_STRINGPARAM("GtkDialog")))
    {
        pWindow = new Dialog(pParent, WB_SIZEMOVE);
    }
    else if (name.equals(RTL_CONSTASCII_STRINGPARAM("GtkBox")))
    {
        pWindow = new VclHBox(pParent);
    }
    else if (name.equals(RTL_CONSTASCII_STRINGPARAM("GtkButton")))
    {
        pWindow = new PushButton(pParent);
    }
    else if (name.equals(RTL_CONSTASCII_STRINGPARAM("GtkLabel")))
    {
        pWindow = new FixedText(pParent);
    }
    else
    {
        fprintf(stderr, "TO-DO, implement %s\n",
            rtl::OString(name.begin, name.length).getStr());
    }
    fprintf(stderr, "created %p child of %p\n", pWindow, pParent);
    return pWindow;
}

void VclBuilder::handleObject(Window *pParent, xmlreader::XmlReader &reader)
{
    Window *pCurrentChild = NULL;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name)) {
        rtl::OString sFoo(name.begin, name.length);
        fprintf(stderr, "objectlevel attribute: is %s\n", sFoo.getStr());

        if (name.equals(RTL_CONSTASCII_STRINGPARAM("class")))
        {
            name = reader.getAttributeValue(false);
            pCurrentChild = makeObject(pParent, name);
            if (!pCurrentChild)
            {
                fprintf(stderr, "missing object!\n");
            }

            if (pCurrentChild)
                m_aChildren.push_back(pCurrentChild);
        }
    }

    if (!pCurrentChild)
    {
        fprintf(stderr, "missing object!\n");
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back();
    }

    int nLevel = 1;

    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        rtl::OString sFoo(name.begin, name.length);
        fprintf(stderr, "objectlevel: is %s %d\n", sFoo.getStr(),
            res);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
            ++nLevel;

        if (res == xmlreader::XmlReader::RESULT_BEGIN &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("object")))
        {
            handleObject(pCurrentChild, reader);
        }

        if (res == xmlreader::XmlReader::RESULT_BEGIN &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
        {
            handleProperty(pCurrentChild, reader);
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::handleProperty(Window *pWindow, xmlreader::XmlReader &reader)
{
    if (!pWindow)
        return;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name)) {
        rtl::OString sFoo(name.begin, name.length);
        fprintf(stderr, "property attribute: is %s\n", sFoo.getStr());

        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            name = reader.getAttributeValue(false);
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("label")))
            {
                reader.nextItem(
                    xmlreader::XmlReader::TEXT_NORMALIZED, &name, &nsId);
                pWindow->SetText(rtl::OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8));
            }
        }
    }
}


Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? NULL : m_aChildren[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
