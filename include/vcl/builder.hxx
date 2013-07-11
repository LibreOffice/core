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

#include <typeinfo>
#include <osl/module.hxx>
#include <tools/resmgr.hxx> //for poxy ResHookProc typedef
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <xmlreader/xmlreader.hxx>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <com/sun/star/frame/XFrame.hpp>

class ListBox;
class NumericFormatter;
class PopupMenu;
class ScrollBar;
class DateField;
class TimeField;
class VclMultiLineEdit;

class VCL_DLLPUBLIC VclBuilder: private boost::noncopyable
{
public:
    typedef std::map<OString, OString> stringmap;
    typedef Window* (*customMakeWidget)(Window *pParent, stringmap &rVec);
private:
    typedef boost::ptr_map<OUString, osl::Module> ModuleMap;
    //We store these until the builder is deleted, that way we can use the
    //ui-previewer on custom widgets and guarantee the modules they are from
    //exist for the duration of the dialog
    ModuleMap m_aModuleMap;

    //If the toplevel window has any properties which need to be set on it,
    //but the toplevel is the owner of the builder, then its ctor
    //has not been completed during the building, so properties for it
    //are collected here and need to be set afterwards, e.g. during
    //Show or Execute
    stringmap m_aDeferredProperties;

    struct PackingData
    {
        bool m_bVerticalOrient;
        sal_Int32 m_nPosition;
        PackingData(bool bVerticalOrient = false, sal_Int32 nPosition = -1)
            : m_bVerticalOrient(bVerticalOrient)
            , m_nPosition(nPosition)
        {
        }
    };

    struct WinAndId
    {
        OString m_sID;
        Window *m_pWindow;
        short m_nResponseId;
        PackingData m_aPackingData;
        WinAndId(const OString &rId, Window *pWindow, bool bVertical)
            : m_sID(rId)
            , m_pWindow(pWindow)
            , m_nResponseId(RET_CANCEL)
            , m_aPackingData(bVertical)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;

    struct MenuAndId
    {
        OString m_sID;
        PopupMenu *m_pMenu;
        MenuAndId(const OString &rId, PopupMenu *pMenu)
            : m_sID(rId)
            , m_pMenu(pMenu)
        {
        }
    };
    std::vector<MenuAndId> m_aMenus;

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

    struct ButtonImageWidgetMap
    {
        OString m_sID;
        OString m_sValue;
        bool m_bRadio;
        ButtonImageWidgetMap(const OString &rId, const OString &rValue, bool bRadio)
            : m_sID(rId)
            , m_sValue(rValue)
            , m_bRadio(bRadio)
        {
        }
    };

    typedef StringPair TextBufferMap;
    typedef StringPair WidgetAdjustmentMap;
    typedef StringPair ButtonMenuMap;
    typedef StringPair MnemonicWidgetMap;

    struct ComboBoxModelMap
    {
        OString m_sID;
        OString m_sValue;
        sal_Int32 m_nActiveId;
        ComboBoxModelMap(const OString &rId, const OString &rValue, sal_Int32 nActiveId)
            : m_sID(rId)
            , m_sValue(rValue)
            , m_nActiveId(nActiveId)
        {
        }
    };

    struct ListStore
    {
        typedef std::vector<OString> row;
        std::vector<row> m_aEntries;
    };
    const ListStore* get_model_by_name(OString sID) const;
    static void mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);

    typedef stringmap TextBuffer;
    const TextBuffer* get_buffer_by_name(OString sID) const;
    static void mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer);

    typedef stringmap Adjustment;
    const Adjustment *get_adjustment_by_name(OString sID) const;
    static void mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment);
    static void mungeAdjustment(DateField &rTarget, const Adjustment &rAdjustment);
    static void mungeAdjustment(TimeField &rTarget, const Adjustment &rAdjustment);
    static void mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment);

    typedef std::map<OString, OString> WidgetTranslations;
    typedef std::map<OString, WidgetTranslations> Translations;

    struct stockinfo
    {
        OString m_sStock;
        int m_nSize;
        stockinfo() : m_nSize(4) {}
    };

    typedef std::map<OString, stockinfo> StockMap;

    struct SizeGroup
    {
        OString m_sID;
        std::vector<OString> m_aWidgets;
        stringmap m_aProperties;
        SizeGroup(const OString &rId)
            : m_sID(rId)
        {
        }
    };

    typedef std::map<Window*, stringmap> AtkMap;

    struct ParserState
    {
        std::vector<RadioButtonGroupMap> m_aGroupMaps;

        std::vector<ComboBoxModelMap> m_aModelMaps;
        std::map<OString, ListStore> m_aModels;

        std::vector<TextBufferMap> m_aTextBufferMaps;
        std::map<OString, TextBuffer> m_aTextBuffers;

        std::vector<WidgetAdjustmentMap> m_aNumericFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aTimeFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aDateFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aScrollAdjustmentMaps;
        std::map<OString, Adjustment> m_aAdjustments;

        std::vector<ButtonImageWidgetMap> m_aButtonImageWidgetMaps;
        StockMap m_aStockMap;

        std::vector<ButtonMenuMap> m_aButtonMenuMaps;

        Translations m_aTranslations;

        std::map<Window*, Window*> m_aRedundantParentWidgets;

        std::vector<SizeGroup> m_aSizeGroups;

        AtkMap m_aAtkInfo;

        std::vector<MnemonicWidgetMap> m_aMnemonicWidgetMaps;
    };

    void loadTranslations(const LanguageTag &rLanguageTag, const OUString &rUri);
    OString getTranslation(const OString &rId, const OString &rProperty) const;

    OString m_sID;
    OString m_sHelpRoot;
    ResHookProc m_pStringReplace;
    Window *m_pParent;
    bool m_bToplevelHasDeferredInit;
    bool m_bToplevelHasDeferredProperties;
    bool m_bToplevelParentFound;
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

    /// XFrame to be able to extract labels and other properties of the UNO commands (like of .uno:Bold).
    com::sun::star::uno::Reference<com::sun::star::frame::XFrame> m_xFrame;

public:
    VclBuilder(Window *pParent, OUString sUIRootDir, OUString sUIFile, OString sID = OString(),
            const com::sun::star::uno::Reference<com::sun::star::frame::XFrame> &rFrame = com::sun::star::uno::Reference<com::sun::star::frame::XFrame>());
    ~VclBuilder();
    Window *get_widget_root();
    //sID must exist and be of type T
    template <typename T> T* get(T*& ret, OString sID)
    {
        Window *w = get_by_name(sID);
        SAL_WARN_IF(!w, "vcl.layout", "widget \"" << sID.getStr() << "\" not found in .ui");
        SAL_WARN_IF(!dynamic_cast<T*>(w),
            "vcl.layout", ".ui widget \"" << sID.getStr() << "\" needs to correspond to vcl type " << typeid(T).name());
        assert(w && dynamic_cast<T*>(w));
        ret = static_cast<T*>(w);
        return ret;
    }
    PopupMenu* get_menu(PopupMenu*& ret, OString sID)
    {
        ret = get_menu(sID);
        SAL_WARN_IF(!ret, "vcl.layout", "menu \"" << sID.getStr() << "\" not found in .ui");
        assert(ret);
        return ret;
    }
    //sID may not exist, but must be of type T if it does
    template <typename T /*=Window if we had c++11*/> T* get(OString sID)
    {
        Window *w = get_by_name(sID);
        SAL_WARN_IF(w && !dynamic_cast<T*>(w),
            "vcl.layout", ".ui widget \"" << sID.getStr() << "\" needs to correspond to vcl type " << typeid(T).name());
        assert(!w || dynamic_cast<T*>(w));
        return static_cast<T*>(w);
    }
    //sID may not exist
    PopupMenu* get_menu(OString sID);

    //given an sID return the response value for that widget
    short get_response(const Window *pWindow) const;

    OString get_by_window(const Window *pWindow) const;
    void delete_by_window(const Window *pWindow);

    //apply the properties of rProps to pWindow
    static void set_properties(Window *pWindow, const stringmap &rProps);

    //Convert _ gtk markup to ~ vcl markup
    static OString convertMnemonicMarkup(const OString &rIn);

    static OString extractCustomProperty(stringmap &rMap);

    static bool extractDropdown(stringmap &rMap);

    //add a default value of 25 width-chars to a map if width-chars not set
    static void ensureDefaultWidthChars(VclBuilder::stringmap &rMap);

    //see m_aDeferredProperties, you need this for toplevel dialogs
    //which build themselves from their ctor. The properties on
    //the top level are stored in m_aDeferredProperties and need
    //to be applied post ctor
    void setDeferredProperties();

    //Helpers to retrofit all the existing code to the builder
    static void reorderWithinParent(std::vector<Window*>& rChilds, bool bIsButtonBox);
    static void reorderWithinParent(Window &rWindow, sal_uInt16 nNewPosition);
private:
    Window *insertObject(Window *pParent, const OString &rClass, const OString &rID,
        stringmap &rProps, stringmap &rPangoAttributes,
        stringmap &rAtkProps, std::vector<OString> &rItems);

    Window *makeObject(Window *pParent, const OString &rClass, const OString &rID,
        stringmap &rVec, const std::vector<OString> &rItems);

    void connectNumericFormatterAdjustment(const OString &id, const OString &rAdjustment);
    void connectTimeFormatterAdjustment(const OString &id, const OString &rAdjustment);
    void connectDateFormatterAdjustment(const OString &id, const OString &rAdjustment);

    bool extractGroup(const OString &id, stringmap &rVec);
    bool extractModel(const OString &id, stringmap &rVec);
    bool extractBuffer(const OString &id, stringmap &rVec);
    bool extractScrollAdjustment(const OString &id, stringmap &rVec);
    bool extractButtonImage(const OString &id, stringmap &rMap, bool bRadio);
    bool extractStock(const OString &id, stringmap &rMap);
    void extractMnemonicWidget(const OString &id, stringmap &rMap);

    void handleTranslations(xmlreader::XmlReader &reader);

    void handleChild(Window *pParent, xmlreader::XmlReader &reader);
    Window* handleObject(Window *pParent, xmlreader::XmlReader &reader);
    void handlePacking(Window *pCurrent, xmlreader::XmlReader &reader);
    void applyPackingProperty(Window *pCurrent, xmlreader::XmlReader &reader);
    void collectProperty(xmlreader::XmlReader &reader, const OString &rID, stringmap &rVec);
    void collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    void collectAtkAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    void collectAccelerator(xmlreader::XmlReader &reader, stringmap &rMap);

    void insertMenuObject(PopupMenu *pParent, const OString &rClass, const OString &rID,
        stringmap &rProps, stringmap &rAccels);
    void handleMenuChild(PopupMenu *pParent, xmlreader::XmlReader &reader);
    void handleMenuObject(PopupMenu *pParent, xmlreader::XmlReader &reader);

    void handleListStore(xmlreader::XmlReader &reader, const OString &rID);
    void handleRow(xmlreader::XmlReader &reader, const OString &rID, sal_Int32 nRowIndex);
    void handleAdjustment(const OString &rID, stringmap &rProperties);
    void handleTextBuffer(const OString &rID, stringmap &rProperties);
    void handleTabChild(Window *pParent, xmlreader::XmlReader &reader);
    void handleMenu(xmlreader::XmlReader &reader, const OString &rID);
    std::vector<OString> handleItems(xmlreader::XmlReader &reader, const OString &rID);

    void handleSizeGroup(xmlreader::XmlReader &reader, const OString &rID);

    void handleAtkObject(xmlreader::XmlReader &reader, const OString &rID, Window *pWindow);

    void handleActionWidget(xmlreader::XmlReader &reader);

    PackingData get_window_packing_data(const Window *pWindow) const;
    void set_window_packing_position(const Window *pWindow, sal_Int32 nPosition);

    Window* prepareWidgetOwnScrolling(Window *pParent, WinBits &rWinStyle);
    void cleanupWidgetOwnScrolling(Window *pScrollParent, Window *pWindow, stringmap &rMap);

    void set_response(OString sID, short nResponse);
};


//helper baseclass to ease retro fitting dialogs/tabpages that load a resource
//to load a .ui file instead
//
//vcl requires the Window Children of a Parent Window to be destroyed before
//the Parent Window.  VclBuilderContainer owns the VclBuilder which owns the
//Children Window. So the VclBuilderContainer dtor must be called before
//the Parent Window dtor.
//
//i.e.  class Dialog : public SystemWindow, public VclBuilderContainer
//not   class Dialog : public VclBuilderContainer, public SystemWindow

class VCL_DLLPUBLIC VclBuilderContainer
{
protected:
    VclBuilder *m_pUIBuilder;
public:
    VclBuilderContainer();
    virtual ~VclBuilderContainer();
    static OUString getUIRootDir();
    bool hasBuilder() const { return m_pUIBuilder != NULL; }
    template <typename T> T* get(T*& ret, OString sID)
    {
        return m_pUIBuilder->get<T>(ret, sID);
    }
    template <typename T /*=Window if we had c++11*/> T* get(OString sID)
    {
        return m_pUIBuilder->get<T>(sID);
    }
    PopupMenu* get_menu(PopupMenu*& ret, OString sID)
    {
        return m_pUIBuilder->get_menu(ret, sID);
    }
    PopupMenu* get_menu(OString sID)
    {
        return m_pUIBuilder->get_menu(sID);
    }
    void setDeferredProperties()
    {
        if (!m_pUIBuilder)
            return;
        m_pUIBuilder->setDeferredProperties();
    }
};

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
bool VCL_DLLPUBLIC toBool(const OString &rValue);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
