/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BUILDER_HXX
#define INCLUDED_VCL_BUILDER_HXX

#include <typeinfo>
#include <osl/module.hxx>
#include <sal/log.hxx>
#include <tools/resmgr.hxx>
#include <tools/fldunit.hxx>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <vcl/vclptr.hxx>

#include <memory>
#include <map>
#include <set>
#include <stack>
#include <vector>
#ifdef check
#  //some problem with MacOSX and a check define
#  undef check
#endif

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class ListBox;
class NumericFormatter;
class PopupMenu;
class ScrollBar;
class Slider;
class DateField;
class TimeField;
class VclExpander;
class VclMultiLineEdit;
namespace xmlreader { class XmlReader; }

class VCL_DLLPUBLIC VclBuilder
{
public:
    typedef std::map<OString, OString> stringmap;
    /// These functions create a new widget with parent @pParent and return it in @rRet
    typedef void (*customMakeWidget)(VclPtr<vcl::Window> &rRet, VclPtr<vcl::Window> &pParent, stringmap &rVec);

public:
                    VclBuilder(
                            vcl::Window *pParent,
                            const OUString& sUIRootDir,
                            const OUString& sUIFile,
                            const OString& sID = OString(),
                            const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
                    ~VclBuilder();

    ///releases references and disposes all children.
    void disposeBuilder();

    //sID must exist and be of type T
    template <typename T> T* get(VclPtr<T>& ret, const OString& sID);

    //sID may not exist, but must be of type T if it does
    template <typename T /*= vcl::Window if we had c++11*/> T* get(const OString& sID);

    vcl::Window*    get_widget_root();

    //sID may not exist
    PopupMenu*      get_menu(const OString& sID);

    //given an sID return the response value for that widget
    short           get_response(const vcl::Window *pWindow) const;

    OString         get_by_window(const vcl::Window *pWindow) const;
    void            delete_by_window(vcl::Window *pWindow);

    //release ownership of pWindow, i.e. don't delete it
    void            drop_ownership(const vcl::Window *pWindow);

    //apply the properties of rProps to pWindow
    static void     set_properties(vcl::Window *pWindow, const stringmap &rProps);

    //Convert _ gtk markup to ~ vcl markup
    static          OString convertMnemonicMarkup(const OString &rIn);

    static OString  extractCustomProperty(stringmap &rMap);
    static FieldUnit detectUnit(OString const&);

    static bool     extractDropdown(stringmap &rMap);

    //add a default value of 25 width-chars to a map if width-chars not set
    static void     ensureDefaultWidthChars(VclBuilder::stringmap &rMap);

    //see m_aDeferredProperties, you need this for toplevel dialogs
    //which build themselves from their ctor. The properties on
    //the top level are stored in m_aDeferredProperties and need
    //to be applied post ctor
    void            setDeferredProperties();

    //Helpers to retrofit all the existing code to the builder
    static void     reorderWithinParent(std::vector< vcl::Window*>& rChilds, bool bIsButtonBox);
    static void     reorderWithinParent(vcl::Window &rWindow, sal_uInt16 nNewPosition);

    /// Get label of the command (like of .uno:Save) from the description service
    static OUString getCommandProperty(const OUString& rProperty, const OUString& rCommand,
                                       const css::uno::Reference<css::uno::XComponentContext>& rContext,
                                       const OUString& rModuleId);

    /// Get image of the command (like of .uno:Save) from the description service
    static Image    getCommandImage(
                        const OUString& rCommand,
                        bool bLarge,
                        const css::uno::Reference<css::uno::XComponentContext>& rContext,
                        const css::uno::Reference<css::frame::XFrame>& rFrame,
                        const OUString& rModuleId );

    css::uno::Reference<css::frame::XFrame> getFrame() { return m_xFrame; }

private:
    VclBuilder(const VclBuilder&) = delete;
    VclBuilder& operator=(const VclBuilder&) = delete;

    typedef std::map<OUString, std::unique_ptr<osl::Module>> ModuleMap;

    //We store these until the builder is deleted, that way we can use the
    //ui-previewer on custom widgets and guarantee the modules they are from
    //exist for the duration of the dialog
    ModuleMap       m_aModuleMap;

    //If the toplevel window has any properties which need to be set on it,
    //but the toplevel is the owner of the builder, then its ctor
    //has not been completed during the building, so properties for it
    //are collected here and need to be set afterwards, e.g. during
    //Show or Execute
    stringmap       m_aDeferredProperties;

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
        VclPtr<vcl::Window> m_pWindow;
        short m_nResponseId;
        PackingData m_aPackingData;
        WinAndId(const OString &rId, vcl::Window *pWindow, bool bVertical)
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

    const ListStore* get_model_by_name(const OString& sID) const;
    static void     mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);

    typedef stringmap TextBuffer;
    const TextBuffer*
                    get_buffer_by_name(const OString& sID) const;

    static void     mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer);

    typedef stringmap Adjustment;
    const Adjustment* get_adjustment_by_name(const OString& sID) const;

    static void     mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(DateField &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(TimeField &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(Slider &rTarget, const Adjustment &rAdjustment);

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

    typedef std::map< VclPtr<vcl::Window>, stringmap> AtkMap;

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
        std::vector<WidgetAdjustmentMap> m_aSliderAdjustmentMaps;

        std::map<OString, Adjustment> m_aAdjustments;

        std::vector<ButtonImageWidgetMap> m_aButtonImageWidgetMaps;
        StockMap m_aStockMap;

        std::vector<ButtonMenuMap> m_aButtonMenuMaps;

        Translations m_aTranslations;

        std::map< VclPtr<vcl::Window>, VclPtr<vcl::Window> > m_aRedundantParentWidgets;

        std::vector<SizeGroup> m_aSizeGroups;

        AtkMap m_aAtkInfo;

        std::vector<MnemonicWidgetMap> m_aMnemonicWidgetMaps;

        std::vector< VclPtr<VclExpander> > m_aExpanderWidgets;

        sal_uInt16 m_nLastToolbarId;

        ParserState();
    };

    void        loadTranslations(const LanguageTag &rLanguageTag, const OUString &rUri);
    OString     getTranslation(const OString &rId, const OString &rProperty) const;

    OString     m_sID;
    OString     m_sHelpRoot;
    ResHookProc m_pStringReplace;
    VclPtr<vcl::Window> m_pParent;
    bool        m_bToplevelHasDeferredInit;
    bool        m_bToplevelHasDeferredProperties;
    bool        m_bToplevelParentFound;
    ParserState *m_pParserState;

    vcl::Window *get_by_name(const OString& sID);
    void        delete_by_name(const OString& sID);

    class sortIntoBestTabTraversalOrder
        : public std::binary_function<const vcl::Window*, const vcl::Window*, bool>
    {
    public:
                sortIntoBestTabTraversalOrder(VclBuilder *pBuilder)
                    : m_pBuilder(pBuilder) {}

        bool    operator()(const vcl::Window *pA, const vcl::Window *pB) const;

    private:
        VclBuilder *m_pBuilder;
    };

    /// XFrame to be able to extract labels and other properties of the UNO commands (like of .uno:Bold).
    css::uno::Reference<css::frame::XFrame> m_xFrame;

private:
    VclPtr<vcl::Window> insertObject(vcl::Window *pParent,
                    const OString &rClass, const OString &rID,
                    stringmap &rProps, stringmap &rPangoAttributes,
                    stringmap &rAtkProps);

    VclPtr<vcl::Window> makeObject(vcl::Window *pParent,
                    const OString &rClass, const OString &rID,
                    stringmap &rVec);

    void        connectNumericFormatterAdjustment(const OString &id, const OString &rAdjustment);
    void        connectTimeFormatterAdjustment(const OString &id, const OString &rAdjustment);
    void        connectDateFormatterAdjustment(const OString &id, const OString &rAdjustment);

    bool        extractGroup(const OString &id, stringmap &rVec);
    bool        extractModel(const OString &id, stringmap &rVec);
    bool        extractBuffer(const OString &id, stringmap &rVec);
    static bool extractAdjustmentToMap(const OString &id, stringmap &rVec, std::vector<WidgetAdjustmentMap>& rAdjustmentMap);
    bool        extractButtonImage(const OString &id, stringmap &rMap, bool bRadio);
    bool        extractStock(const OString &id, stringmap &rMap);
    void        extractMnemonicWidget(const OString &id, stringmap &rMap);

    void        handleTranslations(xmlreader::XmlReader &reader);

    void        handleChild(vcl::Window *pParent, xmlreader::XmlReader &reader);
    VclPtr<vcl::Window> handleObject(vcl::Window *pParent, xmlreader::XmlReader &reader);
    void        handlePacking(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader);
    void        applyPackingProperty(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader);
    void        collectProperty(xmlreader::XmlReader &reader, const OString &rID, stringmap &rVec);
    static void collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    static void collectAtkAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    static void collectAccelerator(xmlreader::XmlReader &reader, stringmap &rMap);

    void        insertMenuObject(
                   PopupMenu *pParent,
                   const OString &rClass,
                   const OString &rID,
                   stringmap &rProps,
                   stringmap &rAccels);

    void        handleMenuChild(PopupMenu *pParent, xmlreader::XmlReader &reader);
    void        handleMenuObject(PopupMenu *pParent, xmlreader::XmlReader &reader);

    void        handleListStore(xmlreader::XmlReader &reader, const OString &rID);
    void        handleRow(xmlreader::XmlReader &reader, const OString &rID, sal_Int32 nRowIndex);
    void        handleAdjustment(const OString &rID, stringmap &rProperties);
    void        handleTextBuffer(const OString &rID, stringmap &rProperties);
    void        handleTabChild(vcl::Window *pParent, xmlreader::XmlReader &reader);
    void        handleMenu(xmlreader::XmlReader &reader, const OString &rID);
    std::vector<OString> handleItems(xmlreader::XmlReader &reader, const OString &rID);

    void        handleSizeGroup(xmlreader::XmlReader &reader, const OString &rID);

    void        handleAtkObject(xmlreader::XmlReader &reader, const OString &rID, vcl::Window *pWindow);

    void        handleActionWidget(xmlreader::XmlReader &reader);

    PackingData get_window_packing_data(const vcl::Window *pWindow) const;
    void        set_window_packing_position(const vcl::Window *pWindow, sal_Int32 nPosition);

    static vcl::Window* prepareWidgetOwnScrolling(vcl::Window *pParent, WinBits &rWinStyle);
    void        cleanupWidgetOwnScrolling(vcl::Window *pScrollParent, vcl::Window *pWindow, stringmap &rMap);

    void        set_response(const OString& sID, short nResponse);
};

template <typename T>
inline T* VclBuilder::get(VclPtr<T>& ret, const OString& sID)
{
    vcl::Window *w = get_by_name(sID);
    SAL_WARN_IF(!w, "vcl.layout", "widget \"" << sID.getStr() << "\" not found in .ui");
    SAL_WARN_IF(!dynamic_cast<T*>(w),
       "vcl.layout", ".ui widget \"" << sID.getStr() << "\" needs to correspond to vcl type " << typeid(T).name());
    assert(w);
    assert(dynamic_cast<T*>(w));
    ret = static_cast<T*>(w);
    return ret.get();
}

//sID may not exist, but must be of type T if it does
template <typename T /*= vcl::Window if we had c++11*/>
inline T* VclBuilder::get(const OString& sID)
{
    vcl::Window *w = get_by_name(sID);
    SAL_WARN_IF(w && !dynamic_cast<T*>(w),
        "vcl.layout", ".ui widget \"" << sID.getStr() << "\" needs to correspond to vcl type " << typeid(T).name());
    assert(!w || dynamic_cast<T*>(w));
    return static_cast<T*>(w);
}



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
//
//With the new 'dispose' framework, it is necessary to force the builder
//dispose before the Window dispose; so a Dialog::dispose() method would
//finish: disposeBuilder(); SystemWindow::dispose() to capture this ordering.

class VCL_DLLPUBLIC VclBuilderContainer
{
public:
                    VclBuilderContainer();
    virtual         ~VclBuilderContainer();
    void            disposeBuilder();

    static OUString getUIRootDir();
    bool            hasBuilder() const { return m_pUIBuilder != nullptr; }

    css::uno::Reference<css::frame::XFrame> getFrame() { return m_pUIBuilder->getFrame(); }

    template <typename T> T* get(VclPtr<T>& ret, const OString& sID)
    {
        return m_pUIBuilder->get<T>(ret, sID);
    }
    template <typename T /*= vcl::Window if we had c++11*/> T* get(const OString & sID)
    {
        return m_pUIBuilder->get<T>(sID);
    }
    PopupMenu* get_menu(const OString & sID)
    {
        return m_pUIBuilder->get_menu(sID);
    }
    void setDeferredProperties()
    {
        if (!m_pUIBuilder)
            return;
        m_pUIBuilder->setDeferredProperties();
    }

protected:
    VclBuilder *m_pUIBuilder;
};

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
bool VCL_DLLPUBLIC toBool(const OString &rValue);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
