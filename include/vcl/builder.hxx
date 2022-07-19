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
#include <sal/log.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fldunit.hxx>
#include <vcl/dllapi.h>
#include <utility>
#include <vcl/window.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/toolboxid.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/EnumContext.hxx>

#include <memory>
#include <map>
#include <string_view>
#include <vector>
#ifdef check
#  //some problem with MacOSX and a check define
#  undef check
#endif

class Button;
class ComboBox;
class FormattedField;
class ListBox;
class Menu;
class MessageDialog;
class NumericFormatter;
class PopupMenu;
class SalInstanceBuilder;
class ScreenshotTest;
class ScrollBar;
class SvTabListBox;
class Slider;
class DateField;
class TimeField;
class ToolBox;
class VclExpander;
class VclMultiLineEdit;
struct NotebookBarAddonsItem;
namespace xmlreader { class XmlReader; }
namespace com::sun::star::frame { class XFrame; }

struct ComboBoxTextItem
{
    OUString m_sItem;
    OString m_sId;
    ComboBoxTextItem(OUString sItem, OString sId)
        : m_sItem(std::move(sItem))
        , m_sId(std::move(sId))
    {
    }
};

/// Creates a hierarchy of vcl::Windows (widgets) from a .ui file for dialogs, sidebar, etc.
class VCL_DLLPUBLIC VclBuilder
{
public:
    typedef std::map<OString, OUString> stringmap;
    typedef std::map<OString, std::pair<OString, OString>> accelmap;
    /// These functions create a new widget with parent pParent and return it in rRet
    typedef void (*customMakeWidget)(VclPtr<vcl::Window> &rRet, const VclPtr<vcl::Window> &pParent, stringmap &rVec);

public:
    VclBuilder(vcl::Window* pParent, const OUString& sUIRootDir, const OUString& sUIFile,
               OString sID = OString(),
               css::uno::Reference<css::frame::XFrame> xFrame
               = css::uno::Reference<css::frame::XFrame>(),
               bool bLegacy = true,
               const NotebookBarAddonsItem* pNotebookBarAddonsItem = nullptr);
    ~VclBuilder();
    ///releases references and disposes all children.
    void disposeBuilder();
    //sID must exist and be of type T
    template <typename T> T* get(VclPtr<T>& ret, const OString& sID);

    //sID may not exist, but must be of type T if it does
    template <typename T = vcl::Window> T* get(const OString& sID);

    vcl::Window*    get_widget_root();

    //sID may not exist
    PopupMenu*      get_menu(std::string_view sID);

    //release ownership of pWindow, i.e. don't delete it
    void            drop_ownership(const vcl::Window *pWindow);

    //see m_aDeferredProperties, you need this for toplevel dialogs
    //which build themselves from their ctor. The properties on
    //the top level are stored in m_aDeferredProperties and need
    //to be applied post ctor
    void            setDeferredProperties();

    /// return UI-File name (without '.ui')
    const OString& getUIFile() const
    {
        return m_sHelpRoot;
    }

    static SymbolType mapStockToSymbol(std::u16string_view icon_name);

private:
    VclBuilder(const VclBuilder&) = delete;
    VclBuilder& operator=(const VclBuilder&) = delete;

    // owner for ListBox/ComboBox UserData
    std::vector<std::unique_ptr<OUString>> m_aUserData;

    //If the toplevel window has any properties which need to be set on it,
    //but the toplevel is the owner of the builder, then its ctor
    //has not been completed during the building, so properties for it
    //are collected here and need to be set afterwards, e.g. during
    //Show or Execute
    stringmap      m_aDeferredProperties;

    std::unique_ptr<NotebookBarAddonsItem> m_pNotebookBarAddonsItem;

    struct PackingData
    {
        bool m_bVerticalOrient;
        sal_Int32 m_nPosition;
        PackingData(bool bVerticalOrient = false)
            : m_bVerticalOrient(bVerticalOrient)
            , m_nPosition(-1)
        {
        }
    };

    struct WinAndId
    {
        OString m_sID;
        VclPtr<vcl::Window> m_pWindow;
        PackingData m_aPackingData;
        WinAndId(OString sId, vcl::Window *pWindow, bool bVertical)
            : m_sID(std::move(sId))
            , m_pWindow(pWindow)
            , m_aPackingData(bVertical)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;

    struct MenuAndId
    {
        OString m_sID;
        VclPtr<Menu> m_pMenu;
        MenuAndId(OString sId, Menu *pMenu);
    };
    std::vector<MenuAndId> m_aMenus;

    struct StringPair
    {
        OString m_sID;
        OString m_sValue;
        StringPair(OString sId, OString sValue)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
        {
        }
    };

    struct UStringPair
    {
        OString m_sID;
        OUString m_sValue;
        UStringPair(OString sId, OUString sValue)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
        {
        }
    };

    typedef StringPair RadioButtonGroupMap;

    struct ButtonImageWidgetMap
    {
        OString m_sID;
        OUString m_sValue;
        bool m_bRadio;
        ButtonImageWidgetMap(OString sId, OUString sValue, bool bRadio)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
            , m_bRadio(bRadio)
        {
        }
    };

    typedef UStringPair TextBufferMap;
    typedef UStringPair WidgetAdjustmentMap;
    typedef UStringPair ButtonMenuMap;
    typedef UStringPair MnemonicWidgetMap;

    struct ComboBoxModelMap
    {
        OString m_sID;
        OUString m_sValue;
        sal_Int32 m_nActiveId;
        ComboBoxModelMap(OString sId, OUString sValue, sal_Int32 nActiveId)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
            , m_nActiveId(nActiveId)
        {
        }
    };

    struct ListStore
    {
        typedef std::vector<OUString> row;
        std::vector<row> m_aEntries;
    };

    const ListStore* get_model_by_name(const OString& sID) const;
    void     mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);
    void     mungeModel(ComboBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);
    void     mungeModel(SvTabListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);

    typedef stringmap TextBuffer;
    const TextBuffer* get_buffer_by_name(const OString& sID) const;

    static void     mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer);

    typedef stringmap Adjustment;
    const Adjustment* get_adjustment_by_name(const OString& sID) const;

    static void     mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(FormattedField &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(Slider &rTarget, const Adjustment &rAdjustment);

    typedef std::map<OString, int> ImageSizeMap;

    struct SizeGroup
    {
        std::vector<OString> m_aWidgets;
        stringmap m_aProperties;
        SizeGroup() {}
    };


    struct ParserState
    {
        std::locale m_aResLocale;

        std::vector<RadioButtonGroupMap> m_aGroupMaps;

        std::vector<ComboBoxModelMap> m_aModelMaps;
        std::map<OString, ListStore> m_aModels;

        std::vector<TextBufferMap> m_aTextBufferMaps;
        std::map<OString, TextBuffer> m_aTextBuffers;

        std::vector<WidgetAdjustmentMap> m_aNumericFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aFormattedFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aScrollAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aSliderAdjustmentMaps;

        std::map<OString, Adjustment> m_aAdjustments;

        std::vector<ButtonImageWidgetMap> m_aButtonImageWidgetMaps;
        ImageSizeMap m_aImageSizeMap;

        std::vector<ButtonMenuMap> m_aButtonMenuMaps;

        std::map<VclPtr<vcl::Window>, VclPtr<vcl::Window>> m_aRedundantParentWidgets;

        std::vector<SizeGroup> m_aSizeGroups;

        std::map<VclPtr<vcl::Window>, stringmap> m_aAtkInfo;

        std::vector<MnemonicWidgetMap> m_aMnemonicWidgetMaps;

        std::vector< VclPtr<VclExpander> > m_aExpanderWidgets;

        std::vector< VclPtr<MessageDialog> > m_aMessageDialogs;

        ToolBoxItemId m_nLastToolbarId;

        sal_uInt16 m_nLastMenuItemId;

        ParserState();
    };

    OString     m_sID;
    OString     m_sHelpRoot;
    ResHookProc m_pStringReplace;
    VclPtr<vcl::Window> m_pParent;
    bool        m_bToplevelHasDeferredInit;
    bool        m_bToplevelHasDeferredProperties;
    bool        m_bToplevelParentFound;
    bool        m_bLegacy;
    std::unique_ptr<ParserState> m_pParserState;

    vcl::Window *get_by_name(std::string_view sID);
    void        delete_by_name(const OString& sID);

    class sortIntoBestTabTraversalOrder
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

    void        connectNumericFormatterAdjustment(const OString &id, const OUString &rAdjustment);
    void        connectFormattedFormatterAdjustment(const OString &id, const OUString &rAdjustment);

    static int  getImageSize(const stringmap &rMap);

    void        extractGroup(const OString &id, stringmap &rVec);
    void        extractModel(const OString &id, stringmap &rVec);
    void        extractBuffer(const OString &id, stringmap &rVec);
    static bool extractAdjustmentToMap(const OString &id, stringmap &rVec, std::vector<WidgetAdjustmentMap>& rAdjustmentMap);
    void        extractButtonImage(const OString &id, stringmap &rMap, bool bRadio);
    void        extractMnemonicWidget(const OString &id, stringmap &rMap);

    // either pParent or pAtkProps must be set, pParent for a child of a widget, pAtkProps for
    // collecting the atk info for a GtkMenuItem
    void        handleChild(vcl::Window *pParent, stringmap *pAtkProps, xmlreader::XmlReader &reader);
    VclPtr<vcl::Window> handleObject(vcl::Window *pParent, stringmap *pAtkProps, xmlreader::XmlReader &reader);
    void        handlePacking(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader);
    static std::vector<vcl::EnumContext::Context> handleStyle(xmlreader::XmlReader &reader, int &nPriority);
    static OString getStyleClass(xmlreader::XmlReader &reader);
    void        applyPackingProperty(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader);
    void        collectProperty(xmlreader::XmlReader &reader, stringmap &rVec) const;
    static void collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    static void collectAtkRelationAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    static void collectAtkRoleAttribute(xmlreader::XmlReader &reader, stringmap &rMap);
    static void collectAccelerator(xmlreader::XmlReader &reader, accelmap &rMap);

    void        insertMenuObject(
                   Menu *pParent,
                   PopupMenu *pSubMenu,
                   const OString &rClass,
                   const OString &rID,
                   stringmap &rProps,
                   stringmap &rAtkProps,
                   accelmap &rAccels);

    void        handleMenuChild(Menu *pParent, xmlreader::XmlReader &reader);
    void        handleMenuObject(Menu *pParent, xmlreader::XmlReader &reader);

    void        handleListStore(xmlreader::XmlReader &reader, const OString &rID, std::string_view rClass);
    void        handleRow(xmlreader::XmlReader &reader, const OString &rID);
    void        handleTabChild(vcl::Window *pParent, xmlreader::XmlReader &reader);
    VclPtr<Menu> handleMenu(xmlreader::XmlReader &reader, const OString &rID, bool bMenuBar);
    std::vector<ComboBoxTextItem> handleItems(xmlreader::XmlReader &reader) const;

    void        handleSizeGroup(xmlreader::XmlReader &reader);

    stringmap   handleAtkObject(xmlreader::XmlReader &reader) const;

    static void applyAtkProperties(vcl::Window *pWindow, const stringmap& rProperties);

    void        handleActionWidget(xmlreader::XmlReader &reader);

    PackingData get_window_packing_data(const vcl::Window *pWindow) const;
    void        set_window_packing_position(const vcl::Window *pWindow, sal_Int32 nPosition);

    static vcl::Window* prepareWidgetOwnScrolling(vcl::Window *pParent, WinBits &rWinStyle);
    void        cleanupWidgetOwnScrolling(vcl::Window *pScrollParent, vcl::Window *pWindow, stringmap &rMap);

    void        set_response(std::string_view sID, short nResponse);

    OString         get_by_window(const vcl::Window *pWindow) const;
    void            delete_by_window(vcl::Window *pWindow);
};

namespace BuilderUtils
{
    //apply the properties of rProps to pWindow
    VCL_DLLPUBLIC void set_properties(vcl::Window *pWindow, const VclBuilder::stringmap &rProps);

    //Convert _ gtk markup to ~ vcl markup
    VCL_DLLPUBLIC OUString convertMnemonicMarkup(std::u16string_view rIn);

    VCL_DLLPUBLIC OUString extractCustomProperty(VclBuilder::stringmap &rMap);

    VCL_DLLPUBLIC bool extractDropdown(VclBuilder::stringmap &rMap);

    //add a default value of 25 width-chars to a map if width-chars not set
    VCL_DLLPUBLIC void ensureDefaultWidthChars(VclBuilder::stringmap &rMap);

    //Helpers to retrofit all the existing code to the builder
    VCL_DLLPUBLIC void reorderWithinParent(std::vector< vcl::Window*>& rChilds, bool bIsButtonBox);
    VCL_DLLPUBLIC void reorderWithinParent(vcl::Window &rWindow, sal_uInt16 nNewPosition);

    //Convert an accessibility role name to accessibility role number
    VCL_DLLPUBLIC sal_Int16 getRoleFromName(const OString& roleName);
}

template <typename T>
inline T* VclBuilder::get(VclPtr<T>& ret, const OString& sID)
{
    vcl::Window *w = get_by_name(sID);
    SAL_WARN_IF(!w, "vcl.layout", "widget \"" << sID << "\" not found in .ui");
    SAL_WARN_IF(!dynamic_cast<T*>(w),
       "vcl.layout", ".ui widget \"" << sID << "\" needs to correspond to vcl type " << typeid(T).name());
    assert(w);
    assert(dynamic_cast<T*>(w));
    ret = static_cast<T*>(w);
    return ret.get();
}

//sID may not exist, but must be of type T if it does
template <typename T>
inline T* VclBuilder::get(const OString& sID)
{
    vcl::Window *w = get_by_name(sID);
    SAL_WARN_IF(w && !dynamic_cast<T*>(w),
        "vcl.layout", ".ui widget \"" << sID << "\" needs to correspond to vcl type " << typeid(T).name());
    assert(!w || dynamic_cast<T*>(w));
    return static_cast<T*>(w);
}

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
VCL_DLLPUBLIC bool toBool(std::u16string_view rValue);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
