/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
class NumericFormatter;
class ScrollBar;

class VCL_DLLPUBLIC VclBuilder
{
public:
    typedef std::map<OString, OString> stringmap;
    typedef Window* (*customMakeWidget)(Window *pParent, stringmap &rVec);
private:
    struct WinAndId
    {
        OString m_sID;
        Window *m_pWindow;
        sal_Int32 m_nPosition;
        bool m_bOwned;
        WinAndId(const OString &rId, Window *pWindow)
            : m_sID(rId)
            , m_pWindow(pWindow)
            , m_nPosition(-1)
            , m_bOwned(true)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;

    struct ListStore
    {
        typedef std::vector<OString> row;
        std::vector<row> m_aEntries;
    };

    struct ModelAndId
    {
        OString m_sID;
        ListStore *m_pModel;
        ModelAndId(const OString &rId, ListStore *pListStore)
            : m_sID(rId)
            , m_pModel(pListStore)
        {
        }
    };

    struct StringPair
    {
        OString m_sID;
        OString m_sValue;
        StringPair(const OString &rId, const OString &rValue)
            : m_sID(rId)
            , m_sValue(rValue)
        {
        }
    };

    typedef StringPair RadioButtonGroupMap;
    typedef StringPair ComboBoxModelMap;
    typedef StringPair ButtonImageWidgetMap;

    ListStore *get_model_by_name(OString sID);
    static void mungemodel(ListBox &rTarget, ListStore &rStore);

    typedef stringmap Adjustment;

    struct AdjustmentAndId
    {
        OString m_sID;
        Adjustment m_aAdjustment;
        AdjustmentAndId(const OString &rId, Adjustment &rAdjustment)
            : m_sID(rId)
        {
            m_aAdjustment.swap(rAdjustment);
        }
    };

    typedef StringPair WidgetAdjustmentMap;

    Adjustment *get_adjustment_by_name(OString sID);
    static void mungeSpinAdjustment(NumericFormatter &rTarget, Adjustment &rAdjustment);
    static void mungeScrollAdjustment(ScrollBar &rTarget, Adjustment &rAdjustment);

    typedef std::map<OString, OString> WidgetTranslations;
    typedef std::map<OString, WidgetTranslations> Translations;

    typedef std::map<OString, OString> StockMap;
    struct ParserState
    {
        std::vector<RadioButtonGroupMap> m_aGroupMaps;
        std::vector<ComboBoxModelMap> m_aModelMaps;
        std::vector<ModelAndId> m_aModels;
        std::vector<AdjustmentAndId> m_aAdjustments;
        std::vector<WidgetAdjustmentMap> m_aSpinAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aScrollAdjustmentMaps;
        std::vector<ButtonImageWidgetMap> m_aButtonImageWidgetMaps;
        StockMap m_aStockMap;
        Translations m_aTranslations;
    };

    OString getTranslation(const OString &rId, const OString &rProperty) const;

    OString m_sID;
    OString m_sHelpRoot;
    Window *m_pParent;
    bool m_bToplevelHasDeferredInit;
    ParserState *m_pParserState;

    Window *get_by_name(OString sID);
    void delete_by_name(OString sID);

    class sortIntoBestTabTraversalOrder
        : public std::binary_function<const Window*, const Window*, bool>
    {
        VclBuilder *m_pBuilder;
    public:
        sortIntoBestTabTraversalOrder(VclBuilder *pBuilder)
            : m_pBuilder(pBuilder)
        {
        }
        bool operator()(const Window *pA, const Window *pB) const;
    };
public:
    VclBuilder(Window *pParent, OUString sUIRootDir, OUString sUIFile, OString sID = OString());
    ~VclBuilder();
    Window *get_widget_root();
    //sID must exist and be of type T
    template <typename T> T* get(T*& ret, OString sID)
    {
        Window *w = get_by_name(sID);
        assert(w && dynamic_cast<T*>(w));
        ret = static_cast<T*>(w);
        return ret;
    }
    //sID may not exist, but must be of type T if it does
    template <typename T /*=Window if we had c++11*/> T* get(OString sID)
    {
        Window *w = get_by_name(sID);
        assert(!w || dynamic_cast<T*>(w));
        return static_cast<T*>(w);
    }
    OString get_by_window(const Window *pWindow) const;
    void delete_by_window(const Window *pWindow);
    //for the purposes of retrofitting this to the existing code
    //look up sID, clone its properties into replacement and
    //splice replacement into the tree instead of it, without
    //taking ownership of it
    bool replace(OString sID, Window &rReplacement);
private:
    Window *insertObject(Window *pParent, const OString &rClass, const OString &rID, stringmap &rVec);
    Window *makeObject(Window *pParent, const OString &rClass, const OString &rID, stringmap &rVec);
    bool extractGroup(const OString &id, stringmap &rVec);
    bool extractModel(const OString &id, stringmap &rVec);
    bool extractSpinAdjustment(const OString &id, stringmap &rVec);
    bool extractScrollAdjustment(const OString &id, stringmap &rVec);
    bool extractImage(const OString &id, stringmap &rMap);
    bool extractStock(const OString &id, stringmap &rMap);

    void handleTranslations(xmlreader::XmlReader &reader);

    void handleChild(Window *pParent, xmlreader::XmlReader &reader);
    Window* handleObject(Window *pParent, xmlreader::XmlReader &reader);
    void handlePacking(Window *pCurrent, xmlreader::XmlReader &reader);
    void applyPackingProperty(Window *pCurrent, xmlreader::XmlReader &reader);
    void collectProperty(xmlreader::XmlReader &reader, const OString &rID, stringmap &rVec);

    void handleListStore(xmlreader::XmlReader &reader, const OString &rID);
    void handleRow(xmlreader::XmlReader &reader, const OString &rID, sal_Int32 nRowIndex);
    void handleAdjustment(const OString &rID, stringmap &rProperties);
    void handleTabChild(Window *pParent, xmlreader::XmlReader &reader);

    sal_Int32 get_window_packing_position(const Window *pWindow) const;
    void set_window_packing_position(const Window *pWindow, sal_Int32 nPosition);

    //Helpers to retrofit all the existing code the the builder
    static void swapGuts(Window &rOrig, Window &rReplacement);
    static sal_uInt16 getPositionWithinParent(Window &rWindow);
    static void reorderWithinParent(Window &rWindow, sal_uInt16 nNewPosition);
};


//allows retro fitting existing dialogs/tabpages that load a resource
//to load a .ui file instead
//
//vcl requires the Window Children of a Parent Window to be destroyed before
//the Parent Window.  VclBuilderContainer owns the VclBuilder which owns the
//Children Window. So the VclBuilderContainer dtor must be called before
//the Parent Window dtor.
//
//i.e.  class Dialog : public SystemWindow, public VclBuilderContainer
//not   class Dialog : public VclBuilderContainer, public SystemWindow
class ResId;

class VCL_DLLPUBLIC VclBuilderContainer
{
protected:
    VclBuilder *m_pUIBuilder;
public:
    VclBuilderContainer();
    virtual ~VclBuilderContainer();
    static OUString getUIRootDir();
    static VclBuilder* overrideResourceWithUIXML(Window *pWindow, const ResId& rResId);
    static bool replace_buildable(Window *pParent, const ResId& rResId, Window &rReplacement);
    template <typename T> T* get(T*& ret, OString sID)
    {
        return m_pUIBuilder->get<T>(ret, sID);
    }
    template <typename T /*=Window if we had c++11*/> T* get(OString sID)
    {
        return m_pUIBuilder->get<T>(sID);
    }
};

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
bool VCL_DLLPUBLIC toBool(const OString &rValue);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
