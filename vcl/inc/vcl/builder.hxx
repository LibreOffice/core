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
#include <stack>
#include <vector>

class ListBox;
class MetricField;

class VCL_DLLPUBLIC VclBuilder
{
public:
    typedef std::map<rtl::OString, rtl::OString> stringmap;
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

    struct ListStore
    {
        std::vector<rtl::OString> m_aEntries;
    };

    struct ModelAndId
    {
        rtl::OString m_sID;
        ListStore *m_pModel;
        ModelAndId(const rtl::OString &rId, ListStore *pListStore)
            : m_sID(rId)
            , m_pModel(pListStore)
        {
        }
    };
    std::vector<ModelAndId> m_aModels;

    struct StringPair
    {
        rtl::OString m_sID;
        rtl::OString m_sValue;
        StringPair(const rtl::OString &rId, const rtl::OString &rValue)
            : m_sID(rId)
            , m_sValue(rValue)
        {
        }
    };

    typedef StringPair RadioButtonGroupMap;
    std::vector<RadioButtonGroupMap> m_aGroupMaps;

    typedef StringPair ComboBoxModelMap;
    std::vector<ComboBoxModelMap> m_aModelMaps;
    ListStore *get_model_by_name(rtl::OString sID);
    static void mungemodel(ListBox &rTarget, ListStore &rStore);

    typedef stringmap Adjustment;

    struct AdjustmentAndId
    {
        rtl::OString m_sID;
        Adjustment m_aAdjustment;
        AdjustmentAndId(const rtl::OString &rId, Adjustment &rAdjustment)
            : m_sID(rId)
        {
            m_aAdjustment.swap(rAdjustment);
        }
    };
    std::vector<AdjustmentAndId> m_aAdjustments;

    typedef StringPair SpinButtonAdjustmentMap;
    std::vector<SpinButtonAdjustmentMap> m_aAdjustmentMaps;
    Adjustment *get_adjustment_by_name(rtl::OString sID);
    static void mungeadjustment(MetricField &rTarget, Adjustment &rAdjustment);

    rtl::OString m_sID;
    Window *m_pParent;
public:
    VclBuilder(Window *pParent, rtl::OUString sUIFile, rtl::OString sID = rtl::OString());
    ~VclBuilder();
    Window *get_widget_root();
    Window *get_by_name(rtl::OString sID);
    rtl::OString get_by_window(const Window *pWindow);
    //for the purposes of retrofitting this to the existing code
    //look up sID, clone its properties into replacement and
    //splice replacement into the tree instead of it, without
    //taking ownership of it
    bool replace(rtl::OString sID, Window &rReplacement);
private:
    Window *insertObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rVec);
    Window *makeObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rVec);
    bool extractGroup(const rtl::OString &id, stringmap &rVec);
    bool extractModel(const rtl::OString &id, stringmap &rVec);
    bool extractAdjustment(const rtl::OString &id, stringmap &rVec);

    void handleChild(Window *pParent, xmlreader::XmlReader &reader);
    Window* handleObject(Window *pParent, xmlreader::XmlReader &reader);
    void handlePacking(Window *pCurrent, xmlreader::XmlReader &reader);
    void applyPackingProperty(Window *pCurrent, xmlreader::XmlReader &reader);
    void collectProperty(xmlreader::XmlReader &reader, stringmap &rVec);

    void handleListStore(xmlreader::XmlReader &reader, const rtl::OString &rID);
    void handleAdjustment(const rtl::OString &rID, stringmap &rProperties);
    void handleTabChild(Window *pParent, xmlreader::XmlReader &reader);

    //Helpers to retrofit all the existing code the the builder
    static void swapGuts(Window &rOrig, Window &rReplacement);
    static sal_uInt16 getPositionWithinParent(Window &rWindow);
    static void reorderWithinParent(Window &rWindow, sal_uInt16 nNewPosition);
};


//allows retro fitting existing dialogs/tabpages that load a resource
//to load a .ui file instead

class ResId;

class VCL_DLLPUBLIC VclBuilderContainer
{
protected:
    VclBuilder *m_pUIBuilder;
public:
    VclBuilderContainer();
    static VclBuilder* overrideResourceWithUIXML(Window *pWindow, const ResId& rResId);
    static bool replace_buildable(Window *pParent, sal_Int32 nID, Window &rReplacement);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
