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
        bool m_bOwned;
        WinAndId(const rtl::OString &rId, Window *pWindow)
            : m_sID(rId)
            , m_pWindow(pWindow)
            , m_bOwned(true)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;

    struct RadioButtonGroupMap
    {
        rtl::OString m_sID;
        rtl::OString m_sGroup;
        RadioButtonGroupMap(const rtl::OString &rId, const rtl::OString &rGroup)
            : m_sID(rId)
            , m_sGroup(rGroup)
        {
        }
    };
    std::vector<RadioButtonGroupMap> m_aGroups;

    rtl::OString m_sID;
    Window *m_pParent;
public:
    VclBuilder(Window *pParent, rtl::OUString sUIFile, rtl::OString sID = rtl::OString());
    ~VclBuilder();
    Window *get_widget_root();
    Window *get_by_name(rtl::OString sID);
    //for the purposes of retrofitting this to the existing code
    //look up sID, clone its properties into replacement and
    //splice replacement into the tree instead of it, without
    //taking ownership of it
    bool replace(rtl::OString sID, Window &rReplacement);

    typedef std::map<rtl::OString, rtl::OString> stringmap;
private:
    Window *insertObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rVec);
    Window *makeObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rVec);
    bool extractGroup(const rtl::OString &id, stringmap &rVec);

    void handleChild(Window *pParent, xmlreader::XmlReader &reader);
    Window* handleObject(Window *pParent, xmlreader::XmlReader &reader);
    void handlePacking(Window *pCurrent, xmlreader::XmlReader &reader);
    void applyPackingProperty(Window *pCurrent, xmlreader::XmlReader &reader);
    void collectProperty(xmlreader::XmlReader &reader, stringmap &rVec);

    //Helpers to retrofit all the existing code the the builder
    static void swapGuts(Window &rOrig, Window &rReplacement);
    static sal_uInt16 getPositionWithinParent(Window &rWindow);
    static void reorderWithinParent(Window &rWindow, sal_uInt16 nNewPosition);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
