/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <typeinfo>
#include <sal/log.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fldunit.hxx>
#include <vcl/dllapi.h>
#include <utility>
#include <vcl/menu.hxx>
#include <vcl/widgetbuilder.hxx>
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

class ComboBox;
class FormattedField;
class ListBox;
class MessageDialog;
class NumericFormatter;
class ScrollBar;
class SvTabListBox;
class Slider;
class VclExpander;
class VclMultiLineEdit;
struct NotebookBarAddonsItem;
namespace com::sun::star::frame { class XFrame; }

/// Creates a hierarchy of vcl::Windows (widgets) from a .ui file for dialogs, sidebar, etc.
class VCL_DLLPUBLIC VclBuilder : public WidgetBuilder<vcl::Window, VclPtr<vcl::Window>, PopupMenu, VclPtr<PopupMenu>>
{
public:
    /// These functions create a new widget with parent pParent and return it in rRet
    typedef void (*customMakeWidget)(VclPtr<vcl::Window> &rRet, const VclPtr<vcl::Window> &pParent, stringmap &rVec);

public:
    VclBuilder(vcl::Window* pParent, std::u16string_view sUIRootDir, const OUString& sUIFile,
               OUString sID = {},
               css::uno::Reference<css::frame::XFrame> xFrame
               = css::uno::Reference<css::frame::XFrame>(),
               bool bLegacy = true,
               const NotebookBarAddonsItem* pNotebookBarAddonsItem = nullptr);
    virtual ~VclBuilder();
    ///releases references and disposes all children.
    void disposeBuilder();

    //sID may not exist, but must be of type T if it does
    template <typename T = vcl::Window> T* get(std::u16string_view sID);

    vcl::Window*    get_widget_root();

    //release ownership of pWindow, i.e. don't delete it
    void            drop_ownership(const vcl::Window *pWindow);

    //see m_aDeferredProperties, you need this for toplevel dialogs
    //which build themselves from their ctor. The properties on
    //the top level are stored in m_aDeferredProperties and need
    //to be applied post ctor
    void            setDeferredProperties();

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
        OUString m_sID;
        VclPtr<vcl::Window> m_pWindow;
        PackingData m_aPackingData;
        WinAndId(OUString sId, vcl::Window *pWindow, bool bVertical)
            : m_sID(std::move(sId))
            , m_pWindow(pWindow)
            , m_aPackingData(bVertical)
        {
        }
    };
    std::vector<WinAndId> m_aChildren;

    struct ButtonImageWidgetMap
    {
        OUString m_sID;
        OUString m_sValue;
        bool m_bRadio;
        ButtonImageWidgetMap(OUString sId, OUString sValue, bool bRadio)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
            , m_bRadio(bRadio)
        {
        }
    };

    typedef StringPair TextBufferMap;
    typedef StringPair WidgetAdjustmentMap;
    typedef StringPair ButtonMenuMap;

    struct ComboBoxModelMap
    {
        OUString m_sID;
        OUString m_sValue;
        sal_Int32 m_nActiveId;
        ComboBoxModelMap(OUString sId, OUString sValue, sal_Int32 nActiveId)
            : m_sID(std::move(sId))
            , m_sValue(std::move(sValue))
            , m_nActiveId(nActiveId)
        {
        }
    };

    void     mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);
    void     mungeModel(ComboBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);
    void     mungeModel(SvTabListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId);

    void insertComboBoxOrListBoxItems(vcl::Window* pWindow, stringmap& rMap,
                                      const std::vector<ComboBoxTextItem>& rItems) override;

    static void     mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer);

    static void     mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(FormattedField &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment);
    static void     mungeAdjustment(Slider &rTarget, const Adjustment &rAdjustment);

    typedef std::map<OUString, int> ImageSizeMap;



    struct VclParserState
    {
        std::vector<ComboBoxModelMap> m_aModelMaps;

        std::vector<TextBufferMap> m_aTextBufferMaps;

        std::vector<WidgetAdjustmentMap> m_aNumericFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aFormattedFormatterAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aScrollAdjustmentMaps;
        std::vector<WidgetAdjustmentMap> m_aSliderAdjustmentMaps;

        std::vector<ButtonImageWidgetMap> m_aButtonImageWidgetMaps;
        ImageSizeMap m_aImageSizeMap;

        std::vector<ButtonMenuMap> m_aButtonMenuMaps;

        std::map<VclPtr<vcl::Window>, VclPtr<vcl::Window>> m_aRedundantParentWidgets;

        std::map<VclPtr<vcl::Window>, stringmap> m_aAtkInfo;

        std::vector< VclPtr<VclExpander> > m_aExpanderWidgets;

        std::vector< VclPtr<MessageDialog> > m_aMessageDialogs;

        ToolBoxItemId m_nLastToolbarId;

        sal_uInt16 m_nLastMenuItemId;

        VclParserState();
    };

    OUString    m_sID;
    VclPtr<vcl::Window> m_pParent;
    bool        m_bToplevelHasDeferredInit;
    bool        m_bToplevelHasDeferredProperties;
    bool        m_bToplevelParentFound;
    std::unique_ptr<VclParserState> m_pVclParserState;

    virtual void resetParserState() override;
    vcl::Window *get_by_name(std::u16string_view sID);
    void        delete_by_name(const OUString& sID);

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
    // tweak newly inserted child depending on window type
    void tweakInsertedChild(vcl::Window *pParent, vcl::Window* pCurrentChild,
                            std::string_view sType, std::string_view sInternalChild) override;

    VclPtr<vcl::Window> insertObject(vcl::Window* pParent, const OUString& rClass,
                                     std::string_view sType, const OUString& rID, stringmap& rProps,
                                     stringmap& rPangoAttributes, stringmap& rAtkProps) override;

    VclPtr<vcl::Window> makeObject(vcl::Window *pParent,
                    const OUString &rClass, const OUString &rID,
                    stringmap &rVec);

    void        connectNumericFormatterAdjustment(const OUString &id, const OUString &rAdjustment);
    void        connectFormattedFormatterAdjustment(const OUString &id, const OUString &rAdjustment);

    static int  getImageSize(const stringmap &rMap);

    void        extractModel(const OUString &id, stringmap &rVec);
    void        extractBuffer(const OUString &id, stringmap &rVec);
    static bool extractAdjustmentToMap(const OUString &id, stringmap &rVec, std::vector<WidgetAdjustmentMap>& rAdjustmentMap);
    void        extractButtonImage(const OUString &id, stringmap &rMap, bool bRadio);

    void applyPackingProperties(vcl::Window* pCurrent, vcl::Window* pParent,
                                const stringmap& rPackingProperties) override;
    virtual void applyTabChildProperties(vcl::Window* pParent, const std::vector<OUString>& rIDs,
                                         std::vector<vcl::EnumContext::Context>& rContext, stringmap& rProperties,
                                         stringmap& rAtkProperties) override;

    void insertMenuObject(PopupMenu* pParent, PopupMenu* pSubMenu, const OUString& rClass,
                          const OUString& rID, stringmap& rProps, stringmap& rAtkProps,
                          accelmap& rAccels) override;

    // if bToolbarItem=true, pParent is the ToolBox that the item belongs to, since there's no widget for the item itself
    void applyAtkProperties(vcl::Window* pWindow, const stringmap& rProperties,
                            bool bToolbarItem) override;

    void setMnemonicWidget(const OUString& rLabelId, const OUString& rMnemonicWidgetId) override;
    void setRadioButtonGroup(const OUString& rRadioButtonId, const OUString& rRadioGroupId) override;
    void setPriority(vcl::Window* pWindow, int nPriority) override;
    void setContext(vcl::Window* pWindow,
                    std::vector<vcl::EnumContext::Context>&& aContext) override;

    virtual bool isHorizontalTabControl(vcl::Window* pWindow) override;

    virtual VclPtr<PopupMenu> createMenu(const OUString& rID) override;

    PackingData get_window_packing_data(const vcl::Window *pWindow) const;
    void        set_window_packing_position(const vcl::Window *pWindow, sal_Int32 nPosition);

    static vcl::Window* prepareWidgetOwnScrolling(vcl::Window *pParent, WinBits &rWinStyle);
    void        cleanupWidgetOwnScrolling(vcl::Window *pScrollParent, vcl::Window *pWindow, stringmap &rMap);

    void set_response(std::u16string_view sID, int nResponse) override;

    OUString        get_by_window(const vcl::Window *pWindow) const;
    void            delete_by_window(vcl::Window *pWindow);
};

namespace BuilderUtils
{
    //apply the properties of rProps to pWindow
    void set_properties(vcl::Window *pWindow, const VclBuilder::stringmap &rProps);

    //Convert _ gtk markup to ~ vcl markup
    OUString convertMnemonicMarkup(std::u16string_view rIn);

    OUString extractCustomProperty(VclBuilder::stringmap &rMap);

    bool extractDropdown(VclBuilder::stringmap &rMap);

    //add a default value of 25 width-chars to a map if width-chars not set
    void ensureDefaultWidthChars(VclBuilder::stringmap &rMap);

    //Helpers to retrofit all the existing code to the builder
    void reorderWithinParent(std::vector< vcl::Window*>& rChilds, bool bIsButtonBox);
    void reorderWithinParent(vcl::Window &rWindow, sal_uInt16 nNewPosition);

    //Convert an accessibility role name to accessibility role number
    sal_Int16 getRoleFromName(std::u16string_view roleName);
}

//sID may not exist, but must be of type T if it does
template <typename T>
inline T* VclBuilder::get(std::u16string_view sID)
{
    vcl::Window *w = get_by_name(sID);
    SAL_WARN_IF(w && !dynamic_cast<T*>(w), "vcl.layout",
                ".ui widget \"" << OUStringToOString(sID, RTL_TEXTENCODING_UTF8)
                                << "\" needs to correspond to vcl type " << typeid(T).name());
    assert(!w || dynamic_cast<T*>(w));
    return static_cast<T*>(w);
}

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
VCL_DLLPUBLIC bool toBool(std::u16string_view rValue);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
