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
#ifndef _VCLBUILDER_HXX
#define _VCLBUILDER_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <xmlreader/xmlreader.hxx>
#include <map>
#include <vector>

class VCL_DLLPUBLIC VclBuilder
{
private:
    //todo merge into Windows UniqueID/HelpID ?
    struct WinAndId
    {
        rtl::OString m_sID;
        Window *m_pWindow;
        WinAndId(const rtl::OString &rId, Window *pWindow)
            : m_sID(rId)
            , m_pWindow(pWindow)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;
    rtl::OString m_sID;
    Window *m_pParent;
public:
    VclBuilder(Window *pParent, rtl::OUString sUIFile, rtl::OString sID = rtl::OString());
    ~VclBuilder();
    Window *get_widget_root();
    Window *get_by_name(rtl::OString sID);

    typedef std::map<rtl::OString, rtl::OString> stringmap;
private:
    Window *insertObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rVec);
    Window *makeObject(Window *pParent, const rtl::OString &rClass, stringmap &rVec);

    void handleChild(Window *pParent, xmlreader::XmlReader &reader);
    Window* handleObject(Window *pParent, xmlreader::XmlReader &reader);
    void handlePacking(Window *pCurrent, xmlreader::XmlReader &reader);
    void applyPackingProperty(Window *pCurrent, xmlreader::XmlReader &reader);
    void collectProperty(xmlreader::XmlReader &reader, stringmap &rVec);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
