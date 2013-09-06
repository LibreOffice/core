/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/module.hxx>
#include <sal/log.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclmedit.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>

using namespace com::sun::star;

#ifdef DISABLE_DYNLOADING
#include <dlfcn.h>              //  For RTLD_DEFAULT
#endif

namespace
{
    sal_uInt16 mapStockToImageResource(OString sType)
    {
        sal_uInt16 nRet = 0;
        if (sType == "gtk-index")
            nRet = SV_RESID_BITMAP_INDEX;
        else if (sType == "gtk-refresh")
            nRet = SV_RESID_BITMAP_REFRESH;
        return nRet;
    }

    SymbolType mapStockToSymbol(OString sType)
    {
        SymbolType eRet = SYMBOL_NOSYMBOL;
        if (sType == "gtk-media-next")
            eRet = SYMBOL_NEXT;
        else if (sType == "gtk-media-previous")
            eRet = SYMBOL_PREV;
        else if (sType == "gtk-media-play")
            eRet = SYMBOL_PLAY;
        else if (sType == "gtk-goto-first")
            eRet = SYMBOL_FIRST;
        else if (sType == "gtk-goto-last")
            eRet = SYMBOL_LAST;
        else if (sType == "gtk-go-back")
            eRet = SYMBOL_ARROW_LEFT;
        else if (sType == "gtk-go-forward")
            eRet = SYMBOL_ARROW_RIGHT;
        else if (sType == "gtk-go-up")
            eRet = SYMBOL_ARROW_UP;
        else if (sType == "gtk-go-down")
            eRet = SYMBOL_ARROW_DOWN;
        else if (sType == "gtk-missing-image")
            eRet = SYMBOL_IMAGE;
        else if (sType == "gtk-help")
            eRet = SYMBOL_HELP;
        else if (sType == "gtk-close")
            eRet = SYMBOL_CLOSE;
        else if (mapStockToImageResource(sType))
            eRet = SYMBOL_IMAGE;
        return eRet;
    }
}

void VclBuilder::loadTranslations(const LanguageTag &rLanguageTag, const OUString& rUri)
{
    /* FIXME-BCP47: support language tags with
     * LanguageTag::getFallbackStrings() ? */
    for (int i = rLanguageTag.getCountry().isEmpty() ? 1 : 0; i < 2; ++i)
    {
        OUStringBuffer aTransBuf;
        sal_Int32 nLastSlash = rUri.lastIndexOf('/');
        if (nLastSlash != -1)
            aTransBuf.append(rUri.copy(0, nLastSlash));
        else
        {
            aTransBuf.append('.');
            nLastSlash = 0;
        }
        aTransBuf.append("/res/").append(rLanguageTag.getLanguage());
        switch (i)
        {
            case 0:
                aTransBuf.append('-').append(rLanguageTag.getCountry());
                break;
            default:
                break;
        }
        sal_Int32 nEndName = rUri.lastIndexOf('.');
        if (nEndName == -1)
            nEndName = rUri.getLength();
        aTransBuf.append(rUri.copy(nLastSlash, nEndName-nLastSlash));

        OUString sTransUri = aTransBuf.makeStringAndClear();
        try
        {
            xmlreader::XmlReader reader(sTransUri);
            handleTranslations(reader);
            break;
        }
        catch (const ::com::sun::star::uno::Exception &)
        {
        }
    }
}

#if defined SAL_LOG_WARN
namespace
{
    bool isButtonType(WindowType nType)
    {
        return nType == WINDOW_PUSHBUTTON ||
               nType == WINDOW_OKBUTTON ||
               nType == WINDOW_CANCELBUTTON ||
               nType == WINDOW_HELPBUTTON ||
               nType == WINDOW_IMAGEBUTTON ||
               nType == WINDOW_MENUBUTTON ||
               nType == WINDOW_MOREBUTTON ||
               nType == WINDOW_SPINBUTTON;
    }
}
#endif

VclBuilder::VclBuilder(Window *pParent, OUString sUIDir, OUString sUIFile, OString sID, const uno::Reference<frame::XFrame>& rFrame)
    : m_sID(sID)
    , m_sHelpRoot(OUStringToOString(sUIFile, RTL_TEXTENCODING_UTF8))
    , m_pStringReplace(ResMgr::GetReadStringHook())
    , m_pParent(pParent)
    , m_bToplevelParentFound(false)
    , m_pParserState(new ParserState)
    , m_xFrame(rFrame)
{
    m_bToplevelHasDeferredInit = (pParent && pParent->IsDialog()) ? ((Dialog*)pParent)->isDeferredInit() : false;
    m_bToplevelHasDeferredProperties = m_bToplevelHasDeferredInit;

    sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
    if (nIdx != -1)
        m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
    m_sHelpRoot = m_sHelpRoot + OString('/');

    OUString sUri = sUIDir + sUIFile;

    const LanguageTag& rLanguageTag = Application::GetSettings().GetUILanguageTag();
    bool bEN_US = (rLanguageTag.getBcp47() == "en-US");
    if (!bEN_US)
        loadTranslations(rLanguageTag, sUri);

    try
    {
        xmlreader::XmlReader reader(sUri);

        handleChild(pParent, reader);
    }
    catch (const ::com::sun::star::uno::Exception &rExcept)
    {
        SAL_WARN("vcl.layout", "Unable to read .ui file: " << rExcept.Message);
        throw;
    }

    //Set Mnemonic widgets when everything has been imported
    for (std::vector<MnemonicWidgetMap>::iterator aI = m_pParserState->m_aMnemonicWidgetMaps.begin(),
        aEnd = m_pParserState->m_aMnemonicWidgetMaps.end(); aI != aEnd; ++aI)
    {
        FixedText *pOne = get<FixedText>(aI->m_sID);
        Window *pOther = get<Window>(aI->m_sValue);
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing member of Mnemonic Widget Mapping");
        if (pOne && pOther)
            pOne->set_mnemonic_widget(pOther);
    }

    //Set a11y relations when everything has been imported
    for (AtkMap::iterator aI = m_pParserState->m_aAtkInfo.begin(),
         aEnd = m_pParserState->m_aAtkInfo.end(); aI != aEnd; ++aI)
    {
        Window *pSource = aI->first;
        const stringmap &rMap = aI->second;

        for (stringmap::const_iterator aP = rMap.begin(),
            aEndP = rMap.end(); aP != aEndP; ++aP)
        {
            const OString &rTarget = aP->second;
            Window *pTarget = get<Window>(rTarget);
            SAL_WARN_IF(!pTarget, "vcl", "missing member of a11y relation: "
                << rTarget.getStr());
            if (!pTarget)
                continue;
            const OString &rType = aP->first;
            if (rType == "labelled-by")
                pSource->SetAccessibleRelationLabeledBy(pTarget);
            else if (rType == "label-for")
                pSource->SetAccessibleRelationLabelFor(pTarget);
            else if (rType == "member-of")
                pSource->SetAccessibleRelationMemberOf(pTarget);
            else
            {
                SAL_INFO("vcl.layout", "unhandled a11y relation :" << rType.getStr());
            }
        }
    }

    //Set radiobutton groups when everything has been imported
    for (std::vector<RadioButtonGroupMap>::iterator aI = m_pParserState->m_aGroupMaps.begin(),
         aEnd = m_pParserState->m_aGroupMaps.end(); aI != aEnd; ++aI)
    {
        RadioButton *pOne = get<RadioButton>(aI->m_sID);
        RadioButton *pOther = get<RadioButton>(aI->m_sValue);
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing member of radiobutton group");
        if (pOne && pOther)
            pOne->group(*pOther);
    }

    //Set ComboBox models when everything has been imported
    for (std::vector<ComboBoxModelMap>::iterator aI = m_pParserState->m_aModelMaps.begin(),
         aEnd = m_pParserState->m_aModelMaps.end(); aI != aEnd; ++aI)
    {
        ListBox *pTarget = get<ListBox>(aI->m_sID);
        const ListStore *pStore = get_model_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pStore, "vcl", "missing elements of combobox/liststore");
        if (pTarget && pStore)
            mungeModel(*pTarget, *pStore, aI->m_nActiveId);
    }

    //Set TextView buffers when everything has been imported
    for (std::vector<TextBufferMap>::iterator aI = m_pParserState->m_aTextBufferMaps.begin(),
         aEnd = m_pParserState->m_aTextBufferMaps.end(); aI != aEnd; ++aI)
    {
        VclMultiLineEdit *pTarget = get<VclMultiLineEdit>(aI->m_sID);
        const TextBuffer *pBuffer = get_buffer_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pBuffer, "vcl", "missing elements of textview/textbuffer");
        if (pTarget && pBuffer)
            mungeTextBuffer(*pTarget, *pBuffer);
    }

    //Set SpinButton adjustments when everything has been imported
    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aNumericFormatterAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aNumericFormatterAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get<Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aTimeFormatterAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aTimeFormatterAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        TimeField *pTarget = dynamic_cast<TimeField*>(get<Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aDateFormatterAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aDateFormatterAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        DateField *pTarget = dynamic_cast<DateField*>(get<Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set ScrollBar adjustments when everything has been imported
    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aScrollAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aScrollAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        ScrollBar *pTarget = get<ScrollBar>(aI->m_sID);
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scrollbar/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set size-groups when all widgets have been imported
    for (std::vector<SizeGroup>::iterator aI = m_pParserState->m_aSizeGroups.begin(),
        aEnd = m_pParserState->m_aSizeGroups.end(); aI != aEnd; ++aI)
    {
        boost::shared_ptr< VclSizeGroup > xGroup(new VclSizeGroup);

        for (stringmap::iterator aP = aI->m_aProperties.begin(),
            aEndP = aI->m_aProperties.end(); aP != aEndP; ++aP)
        {
            const OString &rKey = aP->first;
            const OString &rValue = aP->second;
            xGroup->set_property(rKey, rValue);
        }

        for (std::vector<OString>::iterator aW = aI->m_aWidgets.begin(),
            aEndW = aI->m_aWidgets.end(); aW != aEndW; ++aW)
        {
            Window* pWindow = get<Window>(aW->getStr());
            pWindow->add_to_size_group(xGroup);
        }
    }

    //Set button images when everything has been imported
    std::set<OString> aImagesToBeRemoved;
    for (std::vector<ButtonImageWidgetMap>::iterator aI = m_pParserState->m_aButtonImageWidgetMaps.begin(),
         aEnd = m_pParserState->m_aButtonImageWidgetMaps.end(); aI != aEnd; ++aI)
    {
        PushButton *pTargetButton = NULL;
        RadioButton *pTargetRadio = NULL;
        Button *pTarget = NULL;

        if (!aI->m_bRadio)
        {
            pTargetButton = get<PushButton>(aI->m_sID);
            pTarget = pTargetButton;
        }
        else
        {
            pTargetRadio = get<RadioButton>(aI->m_sID);
            pTarget = pTargetRadio;
        }

        FixedImage *pImage = get<FixedImage>(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pImage,
            "vcl", "missing elements of button/image/stock");
        if (!pTarget || !pImage)
            continue;
        aImagesToBeRemoved.insert(aI->m_sValue);

        VclBuilder::StockMap::iterator aFind = m_pParserState->m_aStockMap.find(aI->m_sValue);
        if (aFind == m_pParserState->m_aStockMap.end())
        {
            if (!aI->m_bRadio)
                pTargetButton->SetModeImage(pImage->GetImage());
            else
                pTargetRadio->SetModeRadioImage(pImage->GetImage());
        }
        else
        {
            const stockinfo &rImageInfo = aFind->second;
            SymbolType eType = mapStockToSymbol(rImageInfo.m_sStock);
            SAL_WARN_IF(eType == SYMBOL_NOSYMBOL, "vcl", "missing stock image element for button");
            if (eType == SYMBOL_NOSYMBOL)
                continue;
            if (!aI->m_bRadio)
                pTargetButton->SetSymbol(eType);
            else
                SAL_WARN_IF(eType != SYMBOL_IMAGE, "vcl.layout", "inimplemented symbol type for radiobuttons");
            if (eType == SYMBOL_IMAGE)
            {
                Bitmap aBitmap(VclResId(mapStockToImageResource(rImageInfo.m_sStock)));
                if (!aI->m_bRadio)
                    pTargetButton->SetModeImage(aBitmap);
                else
                    pTargetRadio->SetModeRadioImage(aBitmap);
            }
            switch (rImageInfo.m_nSize)
            {
                case 1:
                    pTarget->SetSmallSymbol();
                    break;
                case 4:
                    break;
                default:
                    SAL_WARN("vcl.layout", "unsupported image size " << rImageInfo.m_nSize);
                    break;
            }
        }
    }

    //There may be duplicate use of an Image, so we used a set to collect and
    //now we can remove them from the tree after their final munge
    for (std::set<OString>::iterator aI = aImagesToBeRemoved.begin(),
        aEnd = aImagesToBeRemoved.end(); aI != aEnd; ++aI)
    {
        delete_by_name(*aI);
    }

    //Set button menus when everything has been imported
    for (std::vector<ButtonMenuMap>::iterator aI = m_pParserState->m_aButtonMenuMaps.begin(),
         aEnd = m_pParserState->m_aButtonMenuMaps.end(); aI != aEnd; ++aI)
    {
        MenuButton *pTarget = get<MenuButton>(aI->m_sID);
        PopupMenu *pMenu = get_menu(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pMenu,
            "vcl", "missing elements of button/menu");
        if (!pTarget || !pMenu)
            continue;
        pTarget->SetPopupMenu(pMenu);
    }

    //Remove ScrollWindow parent widgets whose children in vcl implement scrolling
    //internally.
    for (std::map<Window*, Window*>::iterator aI = m_pParserState->m_aRedundantParentWidgets.begin(),
        aEnd = m_pParserState->m_aRedundantParentWidgets.end(); aI != aEnd; ++aI)
    {
        delete_by_window(aI->first);
    }

    //fdo#67378 merge the label into the disclosure button
    for (std::vector<VclExpander*>::iterator aI = m_pParserState->m_aExpanderWidgets.begin(),
        aEnd = m_pParserState->m_aExpanderWidgets.end(); aI != aEnd; ++aI)
    {
        VclExpander *pOne = *aI;

        Window *pChild = pOne->get_child();
        Window* pLabel = pOne->GetWindow(WINDOW_LASTCHILD);
        if (pLabel && pLabel != pChild && pLabel->GetType() == WINDOW_FIXEDTEXT)
        {
            FixedText *pLabelWidget = static_cast<FixedText*>(pLabel);
            pOne->set_label(pLabelWidget->GetText());
            delete_by_window(pLabel);
        }
    }

    //drop maps, etc. that we don't need again
    delete m_pParserState;

    SAL_WARN_IF(!m_sID.isEmpty() && (!m_bToplevelParentFound && !get_by_name(m_sID)), "vcl.layout",
        "Requested top level widget \"" << m_sID.getStr() <<
        "\" not found in " << sUIFile);

#if defined SAL_LOG_WARN
    if (m_bToplevelParentFound && m_pParent->IsDialog())
    {
        int nButtons = 0;
        bool bHasDefButton = false;
        for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
             aEnd = m_aChildren.end(); aI != aEnd; ++aI)
        {
            if (isButtonType(aI->m_pWindow->GetType()))
            {
                ++nButtons;
                if (aI->m_pWindow->GetStyle() & WB_DEFBUTTON)
                {
                    bHasDefButton = true;
                    break;
                }
            }
        }
        SAL_WARN_IF(nButtons && !bHasDefButton, "vcl.layout", "No default button defined");
    }
#endif
}

VclBuilder::~VclBuilder()
{
    for (std::vector<WinAndId>::reverse_iterator aI = m_aChildren.rbegin(),
         aEnd = m_aChildren.rend(); aI != aEnd; ++aI)
    {
        delete aI->m_pWindow;
    }

    for (std::vector<MenuAndId>::reverse_iterator aI = m_aMenus.rbegin(),
         aEnd = m_aMenus.rend(); aI != aEnd; ++aI)
    {
        delete aI->m_pMenu;
    }
}

void VclBuilder::handleTranslations(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    OString sID, sProperty;

    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_RAW, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("e"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("g"))
                    {
                        name = reader.getAttributeValue(false);
                        sID = OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                            sID = sID.copy(nDelim);
                    }
                    else if (name.equals("i"))
                    {
                        name = reader.getAttributeValue(false);
                        sProperty = OString(name.begin, name.length);
                    }
                }
            }
        }

        if (res == xmlreader::XmlReader::RESULT_TEXT && !sID.isEmpty())
        {
            OString sTranslation(name.begin, name.length);
            m_pParserState->m_aTranslations[sID][sProperty] = sTranslation;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
            sID = OString();

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }
}

OString VclBuilder::extractCustomProperty(VclBuilder::stringmap &rMap)
{
    OString sCustomProperty;
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("customproperty"));
    if (aFind != rMap.end())
    {
        sCustomProperty = aFind->second;
        rMap.erase(aFind);
    }
    return sCustomProperty;
}

namespace
{
    bool extractResizable(VclBuilder::stringmap &rMap)
    {
        bool bResizable = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("resizable"));
        if (aFind != rMap.end())
        {
            bResizable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bResizable;
    }

    bool extractEntry(VclBuilder::stringmap &rMap)
    {
        bool bHasEntry = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("has-entry"));
        if (aFind != rMap.end())
        {
            bHasEntry = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bHasEntry;
    }

    bool extractOrientation(VclBuilder::stringmap &rMap)
    {
        bool bVertical = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("orientation"));
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("vertical");
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractInconsistent(VclBuilder::stringmap &rMap)
    {
        bool bInconsistent = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("inconsistent"));
        if (aFind != rMap.end())
        {
            bInconsistent = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bInconsistent;
    }

    OString extractIconName(VclBuilder::stringmap &rMap)
    {
        OString sIconName;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("icon-name"));
        if (aFind != rMap.end())
        {
            sIconName = aFind->second;
            rMap.erase(aFind);
        }
        return sIconName;
    }

    OUString getStockText(const OString &rType)
    {
        if (rType == "gtk-ok")
            return (VclResId(SV_BUTTONTEXT_OK).toString());
        else if (rType == "gtk-cancel")
            return (VclResId(SV_BUTTONTEXT_CANCEL).toString());
        else if (rType == "gtk-help")
            return (VclResId(SV_BUTTONTEXT_HELP).toString());
        else if (rType == "gtk-close")
            return (VclResId(SV_BUTTONTEXT_CLOSE).toString());
        else if (rType == "gtk-revert-to-saved")
            return (VclResId(SV_BUTTONTEXT_RESET).toString());
        else if (rType == "gtk-add")
            return (VclResId(SV_BUTTONTEXT_ADD).toString());
        else if (rType == "gtk-delete")
            return (VclResId(SV_BUTTONTEXT_DELETE).toString());
        else if (rType == "gtk-remove")
            return (VclResId(SV_BUTTONTEXT_REMOVE).toString());
        else if (rType == "gtk-new")
            return (VclResId(SV_BUTTONTEXT_NEW).toString());
        else if (rType == "gtk-edit")
            return (VclResId(SV_BUTTONTEXT_EDIT).toString());
        else if (rType == "gtk-apply")
            return (VclResId(SV_BUTTONTEXT_APPLY).toString());
        else if (rType == "gtk-save")
            return (VclResId(SV_BUTTONTEXT_SAVE).toString());
        else if (rType == "gtk-open")
            return (VclResId(SV_BUTTONTEXT_OPEN).toString());
        else if (rType == "gtk-undo")
            return (VclResId(SV_BUTTONTEXT_UNDO).toString());
        else if (rType == "gtk-paste")
            return (VclResId(SV_BUTTONTEXT_PASTE).toString());
        else if (rType == "gtk-media-next")
            return (VclResId(SV_BUTTONTEXT_NEXT).toString());
        else if (rType == "gtk-go-up")
            return (VclResId(SV_BUTTONTEXT_GO_UP).toString());
        else if (rType == "gtk-go-down")
            return (VclResId(SV_BUTTONTEXT_GO_DOWN).toString());
        else if (rType == "gtk-clear")
            return (VclResId(SV_BUTTONTEXT_CLEAR).toString());
        else if (rType == "gtk-media-play")
            return (VclResId(SV_BUTTONTEXT_PLAY).toString());
        SAL_WARN("vcl.layout", "unknown stock type: " << rType.getStr());
        return OUString();
    }

    bool extractStock(VclBuilder::stringmap &rMap)
    {
        bool bIsStock = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("use-stock"));
        if (aFind != rMap.end())
        {
            bIsStock = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bIsStock;
    }

    WinBits extractRelief(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("relief"));
        if (aFind != rMap.end())
        {
            if (aFind->second == "half")
                nBits = WB_FLATBUTTON | WB_BEVELBUTTON;
            else if (aFind->second == "none")
                nBits = WB_FLATBUTTON;
            rMap.erase(aFind);
        }
        return nBits;
    }

    OString extractLabel(VclBuilder::stringmap &rMap)
    {
        OString sType;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("label"));
        if (aFind != rMap.end())
        {
            sType = aFind->second;
            rMap.erase(aFind);
        }
        return sType;
    }

    OString extractActionName(VclBuilder::stringmap &rMap)
    {
        OString sActionName;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("action-name"));
        if (aFind != rMap.end())
        {
            sActionName = aFind->second;
            rMap.erase(aFind);
        }
        return sActionName;
    }

    Size extractSizeRequest(VclBuilder::stringmap &rMap)
    {
        OString sWidthRequest("0");
        OString sHeightRequest("0");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("width-request"));
        if (aFind != rMap.end())
        {
            sWidthRequest = aFind->second;
            rMap.erase(aFind);
        }
        aFind = rMap.find(OString("height-request"));
        if (aFind != rMap.end())
        {
            sHeightRequest = aFind->second;
            rMap.erase(aFind);
        }
        return Size(sWidthRequest.toInt32(), sHeightRequest.toInt32());
    }

    OString extractTooltipText(VclBuilder::stringmap &rMap)
    {
        OString sTooltipText;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("tooltip-text"));
        if (aFind == rMap.end())
            aFind = rMap.find(OString("tooltip-markup"));
        if (aFind != rMap.end())
        {
            sTooltipText = aFind->second;
            rMap.erase(aFind);
        }
        return sTooltipText;
    }

    Button* extractStockAndBuildPushButton(Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CENTER|WB_VCENTER;

        nBits |= extractRelief(rMap);

        bool bIsStock = extractStock(rMap);

        Button *pWindow = NULL;

        if (bIsStock)
        {
            OString sType = extractLabel(rMap);
            if (sType == "gtk-ok")
                pWindow = new OKButton(pParent, nBits);
            else if (sType == "gtk-cancel")
                pWindow = new CancelButton(pParent, nBits);
            else if (sType == "gtk-close")
                pWindow = new CloseButton(pParent, nBits);
            else if (sType == "gtk-help")
                pWindow = new HelpButton(pParent, nBits);
            else
            {
                pWindow = new PushButton(pParent, nBits);
                pWindow->SetText(getStockText(sType));
            }
        }

        if (!pWindow)
            pWindow = new PushButton(pParent, nBits);
        return pWindow;
    }

    Button * extractStockAndBuildMenuButton(Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        Button *pWindow = new MenuButton(pParent, nBits);

        if (extractStock(rMap))
        {
            pWindow->SetText(getStockText(extractLabel(rMap)));
        }

        return pWindow;
    }

    OString extractUnit(const OString& sPattern)
    {
        OString sUnit(sPattern);
        for (sal_Int32 i = 0; i < sPattern.getLength(); ++i)
        {
            if (sPattern[i] != '.' && sPattern[i] != ',' && sPattern[i] != '0')
            {
                sUnit = sPattern.copy(i);
                break;
            }
        }
        return sUnit;
    }

    int extractDecimalDigits(const OString& sPattern)
    {
        int nDigits = 0;
        bool bAfterPoint = false;
        for (sal_Int32 i = 0; i < sPattern.getLength(); ++i)
        {
            if (sPattern[i] == '.' || sPattern[i] == ',')
                bAfterPoint = true;
            else if (sPattern[i] == '0')
            {
                if (bAfterPoint)
                    ++nDigits;
            }
            else
                break;
        }
        return nDigits;
    }

    FieldUnit detectMetricUnit(OString sUnit)
    {
        FieldUnit eUnit = FUNIT_NONE;

        if (sUnit == "mm")
            eUnit = FUNIT_MM;
        else if (sUnit == "cm")
            eUnit = FUNIT_CM;
        else if (sUnit == "m")
            eUnit = FUNIT_M;
        else if (sUnit == "km")
            eUnit = FUNIT_KM;
        else if ((sUnit == "twips") || (sUnit == "twip"))
            eUnit = FUNIT_TWIP;
        else if (sUnit == "pt")
            eUnit = FUNIT_POINT;
        else if (sUnit == "pc")
            eUnit = FUNIT_PICA;
        else if (sUnit == "\"" || (sUnit == "in") || (sUnit == "inch"))
            eUnit = FUNIT_INCH;
        else if ((sUnit == "'") || (sUnit == "ft") || (sUnit == "foot") || (sUnit == "feet"))
            eUnit = FUNIT_FOOT;
        else if (sUnit == "mile" || (sUnit == "miles"))
            eUnit = FUNIT_MILE;
        else if (sUnit == "ch")
            eUnit = FUNIT_CHAR;
        else if (sUnit == "line")
            eUnit = FUNIT_LINE;
        else if (sUnit == "%")
            eUnit = FUNIT_PERCENT;
        else if ((sUnit == "pixels") || (sUnit == "pixel") || (sUnit == "px"))
            eUnit = FUNIT_PIXEL;
        else if ((sUnit == "degrees") || (sUnit == "degree"))
            eUnit = FUNIT_DEGREE;
        else if ((sUnit == "sec") || (sUnit == "seconds") || (sUnit == "second"))
            eUnit = FUNIT_SECOND;
        else if ((sUnit == "ms") || (sUnit == "milliseconds") || (sUnit == "millisecond"))
            eUnit = FUNIT_MILLISECOND;
        else if (sUnit != "0")
            eUnit = FUNIT_CUSTOM;

        return eUnit;
    }
}

void VclBuilder::ensureDefaultWidthChars(VclBuilder::stringmap &rMap)
{
    OString sWidthChars("width-chars");
    VclBuilder::stringmap::iterator aFind = rMap.find(sWidthChars);
    if (aFind == rMap.end())
        rMap[sWidthChars] = "25";
}

bool VclBuilder::extractGroup(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("group"));
    if (aFind != rMap.end())
    {
        OString sID = aFind->second;
        sal_Int32 nDelim = sID.indexOf(':');
        if (nDelim != -1)
            sID = sID.copy(0, nDelim);
        m_pParserState->m_aGroupMaps.push_back(RadioButtonGroupMap(id, sID));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

void VclBuilder::connectNumericFormatterAdjustment(const OString &id, const OString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aNumericFormatterAdjustmentMaps.push_back(WidgetAdjustmentMap(id, rAdjustment));
}

void VclBuilder::connectTimeFormatterAdjustment(const OString &id, const OString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aTimeFormatterAdjustmentMaps.push_back(WidgetAdjustmentMap(id, rAdjustment));
}

void VclBuilder::connectDateFormatterAdjustment(const OString &id, const OString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aDateFormatterAdjustmentMaps.push_back(WidgetAdjustmentMap(id, rAdjustment));
}

bool VclBuilder::extractScrollAdjustment(const OString &id, VclBuilder::stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aScrollAdjustmentMaps.push_back(WidgetAdjustmentMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

namespace
{
    sal_Int32 extractActive(VclBuilder::stringmap &rMap)
    {
        sal_Int32 nActiveId = 0;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("active"));
        if (aFind != rMap.end())
        {
            nActiveId = aFind->second.toInt32();
            rMap.erase(aFind);
        }
        return nActiveId;
    }

    bool extractSelectable(VclBuilder::stringmap &rMap)
    {
        bool bSelectable = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("selectable"));
        if (aFind != rMap.end())
        {
            bSelectable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bSelectable;
    }

    OString extractAdjustment(VclBuilder::stringmap &rMap)
    {
        OString sAdjustment;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
        if (aFind != rMap.end())
        {
            sAdjustment= aFind->second;
            rMap.erase(aFind);
            return sAdjustment;
        }
        return sAdjustment;
    }
}

bool VclBuilder::extractModel(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("model"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aModelMaps.push_back(ComboBoxModelMap(id, aFind->second,
            extractActive(rMap)));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractDropdown(VclBuilder::stringmap &rMap)
{
    bool bDropdown = true;
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("dropdown"));
    if (aFind != rMap.end())
    {
        bDropdown = toBool(aFind->second);
        rMap.erase(aFind);
    }
    return bDropdown;
}

bool VclBuilder::extractBuffer(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("buffer"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aTextBufferMaps.push_back(TextBufferMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractStock(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("stock"));
    if (aFind != rMap.end())
    {
        stockinfo aInfo;
        aInfo.m_sStock = aFind->second;
        rMap.erase(aFind);
        aFind = rMap.find(OString("icon-size"));
        if (aFind != rMap.end())
        {
            aInfo.m_nSize = aFind->second.toInt32();
            rMap.erase(aFind);
        }
        m_pParserState->m_aStockMap[id] = aInfo;
        return true;
    }
    return false;
}

bool VclBuilder::extractButtonImage(const OString &id, stringmap &rMap, bool bRadio)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("image"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aButtonImageWidgetMaps.push_back(ButtonImageWidgetMap(id, aFind->second, bRadio));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

void VclBuilder::extractMnemonicWidget(const OString &rLabelID, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("mnemonic-widget"));
    if (aFind != rMap.end())
    {
        OString sID = aFind->second;
        sal_Int32 nDelim = sID.indexOf(':');
        if (nDelim != -1)
            sID = sID.copy(0, nDelim);
        m_pParserState->m_aMnemonicWidgetMaps.push_back(MnemonicWidgetMap(rLabelID, sID));
        rMap.erase(aFind);
    }
}

Window* VclBuilder::prepareWidgetOwnScrolling(Window *pParent, WinBits &rWinStyle)
{
    //For Widgets that manage their own scrolling, if one appears as a child of
    //a scrolling window shoehorn that scrolling settings to this widget and
    //return the real parent to use
    if (pParent && pParent->GetType() == WINDOW_SCROLLWINDOW)
    {
        WinBits nScrollBits = pParent->GetStyle();
        nScrollBits &= (WB_AUTOHSCROLL|WB_HSCROLL|WB_AUTOVSCROLL|WB_VSCROLL);
        rWinStyle |= nScrollBits;
        pParent = pParent->GetParent();
    }

    return pParent;
}

void VclBuilder::cleanupWidgetOwnScrolling(Window *pScrollParent, Window *pWindow, stringmap &rMap)
{
    //remove the redundant scrolling parent
    sal_Int32 nWidthReq = pScrollParent->get_width_request();
    rMap[OString("width-request")] = OString::number(nWidthReq);
    sal_Int32 nHeightReq = pScrollParent->get_height_request();
    rMap[OString("height-request")] = OString::number(nHeightReq);

    m_pParserState->m_aRedundantParentWidgets[pScrollParent] = pWindow;
}

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }
#endif

Window *VclBuilder::makeObject(Window *pParent, const OString &name, const OString &id,
    stringmap &rMap, const std::vector<OString> &rItems)
{
    bool bIsPlaceHolder = name.isEmpty();
    bool bVertical = false;

    if (pParent && pParent->GetType() == WINDOW_TABCONTROL)
    {
        //We have to add a page

        //make default pageid == position
        TabControl *pTabControl = static_cast<TabControl*>(pParent);
        sal_uInt16 nNewPageCount = pTabControl->GetPageCount()+1;
        sal_uInt16 nNewPageId = nNewPageCount;
        pTabControl->InsertPage(nNewPageId, OUString());
        pTabControl->SetCurPageId(nNewPageId);

        if (!bIsPlaceHolder)
        {
            TabPage* pPage = new TabPage(pTabControl);
            pPage->Show();

            //Make up a name for it
            OString sTabPageId = get_by_window(pParent) +
                OString("-page") +
                OString::number(nNewPageCount);
            m_aChildren.push_back(WinAndId(sTabPageId, pPage, false));
            pPage->SetHelpId(m_sHelpRoot + sTabPageId);

            //And give the page one container as a child to make it a layout enabled
            //tab page
            VclBin* pContainer = new VclBin(pPage);
            pContainer->Show();
            m_aChildren.push_back(WinAndId(OString(), pContainer, false));
            pContainer->SetHelpId(m_sHelpRoot + sTabPageId + OString("-bin"));
            pParent = pContainer;

            pTabControl->SetTabPage(nNewPageId, pPage);
        }
    }

    if (bIsPlaceHolder || name == "GtkTreeSelection")
        return NULL;

    extractButtonImage(id, rMap, name == "GtkRadioButton");

    Window *pWindow = NULL;
    if (name == "GtkDialog")
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        pWindow = new Dialog(pParent, nBits);
    }
    else if (name == "GtkMessageDialog")
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        pWindow = new MessageDialog(pParent, nBits);
    }
    else if (name == "GtkBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            pWindow = new VclVBox(pParent);
        else
            pWindow = new VclHBox(pParent);
    }
    else if (name == "GtkHBox")
        pWindow = new VclHBox(pParent);
    else if (name == "GtkVBox")
        pWindow = new VclVBox(pParent);
    else if (name == "GtkButtonBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            pWindow = new VclVButtonBox(pParent);
        else
            pWindow = new VclHButtonBox(pParent);
    }
    else if (name == "GtkHButtonBox")
        pWindow = new VclHButtonBox(pParent);
    else if (name == "GtkVButtonBox")
        pWindow = new VclVButtonBox(pParent);
    else if (name == "GtkGrid")
        pWindow = new VclGrid(pParent);
    else if (name == "GtkFrame")
        pWindow = new VclFrame(pParent);
    else if (name == "GtkExpander")
    {
        VclExpander *pExpander = new VclExpander(pParent);
        m_pParserState->m_aExpanderWidgets.push_back(pExpander);
        pWindow = pExpander;
    }
    else if (name == "GtkAlignment")
        pWindow = new VclAlignment(pParent);
    else if (name == "GtkButton")
    {
        Button *pButton;
        OString sMenu = extractCustomProperty(rMap);
        if (sMenu.isEmpty())
            pButton = extractStockAndBuildPushButton(pParent, rMap);
        else
        {
            pButton = extractStockAndBuildMenuButton(pParent, rMap);
            m_pParserState->m_aButtonMenuMaps.push_back(ButtonMenuMap(id, sMenu));
        }
        pButton->SetImageAlign(IMAGEALIGN_LEFT); //default to left
        pWindow = pButton;
    }
    else if (name == "GtkRadioButton")
    {
        extractGroup(id, rMap);
        WinBits nBits = WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OString sWrap = extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        RadioButton *pButton = new RadioButton(pParent, nBits);
        pButton->SetImageAlign(IMAGEALIGN_LEFT); //default to left
        pWindow = pButton;
    }
    else if (name == "GtkCheckButton")
    {
        WinBits nBits = WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OString sWrap = extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        //maybe always import as TriStateBox and enable/disable tristate
        bool bIsTriState = extractInconsistent(rMap);
        CheckBox *pCheckBox = bIsTriState ?
            new TriStateBox(pParent, nBits) :
            new CheckBox(pParent, nBits);
        if (bIsTriState)
            pCheckBox->SetState(STATE_DONTKNOW);
        pCheckBox->SetImageAlign(IMAGEALIGN_LEFT); //default to left
        pWindow = pCheckBox;
    }
    else if (name == "GtkSpinButton")
    {
        OString sAdjustment = extractAdjustment(rMap);
        OString sPattern = extractCustomProperty(rMap);
        OString sUnit = extractUnit(sPattern);

        WinBits nBits = WB_LEFT|WB_BORDER|WB_3DLOOK;
        if (!id.endsWith("-nospin"))
            nBits |= WB_SPIN | WB_REPEAT;

        if (sPattern.isEmpty())
        {
            connectNumericFormatterAdjustment(id, sAdjustment);
            SAL_INFO("vcl.layout", "making numeric field for " << name.getStr() << " " << sUnit.getStr());
            pWindow = new NumericField(pParent, nBits);
        }
        else
        {
            if (sPattern == "hh:mm")
            {
                connectTimeFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making time field for " << name.getStr() << " " << sUnit.getStr());
                TimeField *pField = new TimeField(pParent, nBits);
                pWindow = pField;
            }
            else if (sPattern == "yy:mm:dd")
            {
                connectDateFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making date field for " << name.getStr() << " " << sUnit.getStr());
                DateField *pField = new DateField(pParent, nBits);
                pWindow = pField;
            }
            else
            {
                connectNumericFormatterAdjustment(id, sAdjustment);
                FieldUnit eUnit = detectMetricUnit(sUnit);
                SAL_INFO("vcl.layout", "making metric field for " << name.getStr() << " " << sUnit.getStr());
                MetricField *pField = new MetricField(pParent, nBits);
                pField->SetUnit(eUnit);
                if (eUnit == FUNIT_CUSTOM)
                    pField->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
                pWindow = pField;
            }
        }
    }
    else if (name == "GtkLinkButton")
        pWindow = new FixedHyperlink(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
    else if ((name == "GtkComboBox") || (name == "GtkComboBoxText") || (name == "VclComboBoxText"))
    {
        OString sPattern = extractCustomProperty(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = VclBuilder::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            OString sAdjustment = extractAdjustment(rMap);
            connectNumericFormatterAdjustment(id, sAdjustment);
            OString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_WARN("vcl.layout", "making metric box for " << name.getStr() << " " << sUnit.getStr()
                << " use a VclComboBoxNumeric instead");
            MetricBox *pBox = new MetricBox(pParent, nBits);
            pBox->EnableAutoSize(true);
            pBox->SetUnit(eUnit);
            pBox->SetDecimalDigits(extractDecimalDigits(sPattern));
            if (eUnit == FUNIT_CUSTOM)
                pBox->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
            pWindow = pBox;
        }
        else if (extractEntry(rMap))
        {
            ComboBox* pComboBox = new ComboBox(pParent, nBits);
            pComboBox->EnableAutoSize(true);
            if (!rItems.empty())
            {
                sal_uInt16 nActiveId = extractActive(rMap);
                for (std::vector<OString>::const_iterator aI = rItems.begin(), aEnd = rItems.end(); aI != aEnd; ++aI)
                    pComboBox->InsertEntry(OStringToOUString(*aI, RTL_TEXTENCODING_UTF8));
                if (nActiveId < rItems.size())
                    pComboBox->SelectEntryPos(nActiveId);
            }
            pWindow = pComboBox;
        }
        else
        {
            ListBox *pListBox = new ListBox(pParent, nBits|WB_SIMPLEMODE);
            pListBox->EnableAutoSize(true);
            if (!rItems.empty())
            {
                sal_uInt16 nActiveId = extractActive(rMap);
                for (std::vector<OString>::const_iterator aI = rItems.begin(), aEnd = rItems.end(); aI != aEnd; ++aI)
                    pListBox->InsertEntry(OStringToOUString(*aI, RTL_TEXTENCODING_UTF8));
                if (nActiveId < rItems.size())
                    pListBox->SelectEntryPos(nActiveId);
            }
            pWindow = pListBox;
        }
    }
    else if (name == "VclComboBoxNumeric")
    {
        OString sPattern = extractCustomProperty(rMap);
        OString sAdjustment = extractAdjustment(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = VclBuilder::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            OString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_INFO("vcl.layout", "making metric box for " << name.getStr() << " " << sUnit.getStr());
            connectNumericFormatterAdjustment(id, sAdjustment);
            MetricBox *pBox = new MetricBox(pParent, nBits|WB_BORDER);
            if (bDropdown)
                pBox->EnableAutoSize(true);
            pBox->SetUnit(eUnit);
            if (eUnit == FUNIT_CUSTOM)
                pBox->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
            pWindow = pBox;
        }
        else
        {
            SAL_INFO("vcl.layout", "making numeric box for " << name.getStr());
            connectNumericFormatterAdjustment(id, sAdjustment);
            NumericBox* pBox = new NumericBox(pParent, nBits);
            if (bDropdown)
                pBox->EnableAutoSize(true);
            pWindow = pBox;
        }
    }
    else if (name == "GtkTreeView")
    {
        //To-Do
        //a) move svtools SvTreeViewBox into vcl
        //b) make that the default target for GtkTreeView
        //c) remove the non-drop down mode of ListBox and convert
        //   everything over to SvTreeViewBox
        //d) remove the users of makeSvTreeViewBox
        extractModel(id, rMap);
        WinBits nWinStyle = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
        OString sBorder = extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        //ListBox manages its own scrolling,
        Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        pWindow = new ListBox(pRealParent, nWinStyle);
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, pWindow, rMap);
    }
    else if (name == "GtkLabel")
    {
        extractMnemonicWidget(id, rMap);
        if (extractSelectable(rMap))
            pWindow = new SelectableFixedText(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
        else
            pWindow = new FixedText(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    }
    else if (name == "GtkImage")
    {
        extractStock(id, rMap);
        pWindow = new FixedImage(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
        //such parentless GtkImages are temps used to set icons on buttons
        //default them to hidden to stop e.g. insert->index entry flicking temp
        //full screen windows
        if (!pParent)
        {
            rMap["visible"] = "false";
        }

    }
    else if (name == "GtkSeparator")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            pWindow = new FixedLine(pParent, WB_VERT);
        else
            pWindow = new FixedLine(pParent, WB_HORZ);
    }
    else if (name == "GtkScrollbar")
    {
        extractScrollAdjustment(id, rMap);
        bVertical = extractOrientation(rMap);
        if (bVertical)
            pWindow = new ScrollBar(pParent, WB_VERT);
        else
            pWindow = new ScrollBar(pParent, WB_HORZ);
    }
    else if (name == "GtkProgressBar")
    {
        extractScrollAdjustment(id, rMap);
        bVertical = extractOrientation(rMap);
        if (bVertical)
            pWindow = new ProgressBar(pParent, WB_VERT);
        else
            pWindow = new ProgressBar(pParent, WB_HORZ);
    }
    else if (name == "GtkScrolledWindow")
    {
        pWindow = new VclScrolledWindow(pParent);
    }
    else if (name == "GtkEventBox")
    {
        pWindow = new VclEventBox(pParent);
    }
    else if (name == "GtkEntry")
    {
        pWindow = new Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
        ensureDefaultWidthChars(rMap);
    }
    else if (name == "GtkNotebook")
        pWindow = new TabControl(pParent, WB_STDTABCONTROL|WB_3DLOOK);
    else if (name == "GtkDrawingArea")
    {
        OString sBorder = extractCustomProperty(rMap);
        pWindow = new Window(pParent, sBorder.isEmpty() ? 0 : WB_BORDER);
    }
    else if (name == "GtkTextView")
    {
        extractBuffer(id, rMap);

        WinBits nWinStyle = WB_LEFT;
        OString sBorder = extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        //VclMultiLineEdit manages its own scrolling,
        Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        pWindow = new VclMultiLineEdit(pRealParent, nWinStyle);
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, pWindow, rMap);
    }
    else if (name == "GtkToolbar")
    {
        pWindow = new ToolBox(pParent, WB_3DLOOK | WB_TABSTOP);
    }
    else if (name == "GtkToolButton" || name == "GtkMenuToolButton")
    {
        ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
        if (pToolBox)
        {
            OUString aCommand(OStringToOUString(extractActionName(rMap), RTL_TEXTENCODING_UTF8));

            sal_uInt16 nItemId = 0;
            ToolBoxItemBits nBits = 0;
            if (name == "GtkMenuToolButton")
                nBits |= TIB_DROPDOWN;

            if (!aCommand.isEmpty() && m_xFrame.is())
            {
                pToolBox->InsertItem(aCommand, m_xFrame, nBits, extractSizeRequest(rMap));
                nItemId = pToolBox->GetItemId(aCommand);
            }
            else
            {
                const sal_uInt16 COMMAND_ITEMID_START = 30000;
                nItemId = COMMAND_ITEMID_START + pToolBox->GetItemCount();
                pToolBox->InsertItem(nItemId,
                    OStringToOUString(extractLabel(rMap), RTL_TEXTENCODING_UTF8), nBits);
                pToolBox->SetItemCommand(nItemId, aCommand);
                pToolBox->SetHelpId(nItemId, m_sHelpRoot + id);
            }

            OString sTooltip(extractTooltipText(rMap));
            if (!sTooltip.isEmpty())
                pToolBox->SetQuickHelpText(nItemId, OStringToOUString(sTooltip, RTL_TEXTENCODING_UTF8));

            OString sIconName(extractIconName(rMap));
            if (!sIconName.isEmpty())
                pToolBox->SetItemImage(nItemId, FixedImage::loadThemeImage(sIconName));

            return NULL; // no widget to be created
        }
    }
    else
    {
        sal_Int32 nDelim = name.indexOf('-');
        if (nDelim != -1)
        {
#ifndef DISABLE_DYNLOADING
            OUStringBuffer sModuleBuf;
#ifdef SAL_DLLPREFIX
            sModuleBuf.append(SAL_DLLPREFIX);
#endif
            sModuleBuf.append(OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8));
#ifdef SAL_DLLEXTENSION
            sModuleBuf.append(SAL_DLLEXTENSION);
#endif
#endif
            OUString sFunction(OStringToOUString(OString("make") + name.copy(nDelim+1), RTL_TEXTENCODING_UTF8));
#ifndef DISABLE_DYNLOADING
            OUString sModule = sModuleBuf.makeStringAndClear();
            ModuleMap::iterator aI = m_aModuleMap.find(sModule);
            osl::Module* pModule = NULL;
            if (aI == m_aModuleMap.end())
            {
                pModule = new osl::Module;
                pModule->loadRelative(&thisModule, sModule);
                aI = m_aModuleMap.insert(sModule, pModule).first;
            }
            customMakeWidget pFunction = (customMakeWidget)aI->second->getFunctionSymbol(sFunction);
#else
            customMakeWidget pFunction = (customMakeWidget)osl_getFunctionSymbol((oslModule) RTLD_DEFAULT, sFunction.pData);
#endif
            if (pFunction)
                pWindow = (*pFunction)(pParent, rMap);
        }
    }
    SAL_WARN_IF(!pWindow, "vcl.layout", "probably need to implement " << name.getStr() << " or add a make" << name.getStr() << " function");
    if (pWindow)
    {
        pWindow->SetHelpId(m_sHelpRoot + id);
        SAL_INFO("vcl.layout", "for " << name.getStr() <<
            ", created " << pWindow << " child of " <<
            pParent << "(" << pWindow->mpWindowImpl->mpParent << "/" <<
            pWindow->mpWindowImpl->mpRealParent << "/" <<
            pWindow->mpWindowImpl->mpBorderWindow << ") with helpid " <<
            pWindow->GetHelpId().getStr());
        m_aChildren.push_back(WinAndId(id, pWindow, bVertical));
    }
    return pWindow;
}

namespace
{
    //return true for window types which exist in vcl but are not themselves
    //represented in the .ui format, i.e. only their children exist.
    bool isConsideredGtkPseudo(Window *pWindow)
    {
        return pWindow->GetType() == WINDOW_TABPAGE;
    }
}

//Any properties from .ui load we couldn't set because of potential virtual methods
//during ctor are applied here
void VclBuilder::setDeferredProperties()
{
    if (!m_bToplevelHasDeferredProperties)
        return;
    stringmap aDeferredProperties;
    aDeferredProperties.swap(m_aDeferredProperties);
    m_bToplevelHasDeferredProperties = false;
    set_properties(m_pParent, aDeferredProperties);
}

void VclBuilder::set_properties(Window *pWindow, const stringmap &rProps)
{
    for (stringmap::const_iterator aI = rProps.begin(), aEnd = rProps.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;
        pWindow->set_property(rKey, rValue);
    }
}

Window *VclBuilder::insertObject(Window *pParent, const OString &rClass,
    const OString &rID, stringmap &rProps, stringmap &rPango,
    stringmap &rAtk,
    std::vector<OString> &rItems)
{
    Window *pCurrentChild = NULL;

    if (m_pParent && !isConsideredGtkPseudo(m_pParent) && !m_sID.isEmpty() && rID.equals(m_sID))
    {
        pCurrentChild = m_pParent;
        //toplevels default to resizable
        if (pCurrentChild->IsDialog())
        {
            Dialog *pDialog = (Dialog*)pCurrentChild;
            pDialog->doDeferredInit(extractResizable(rProps));
            m_bToplevelHasDeferredInit = false;
        }
        if (pCurrentChild->GetHelpId().isEmpty())
        {
            pCurrentChild->SetHelpId(m_sHelpRoot + m_sID);
            SAL_INFO("vcl.layout", "for toplevel dialog " << this << " " <<
                rID.getStr() << ", set helpid " <<
                pCurrentChild->GetHelpId().getStr());
        }
        m_bToplevelParentFound = true;
    }
    else
    {
        //if we're being inserting under a toplevel dialog whose init is
        //deferred due to waiting to encounter it in this .ui, and it hasn't
        //been seen yet, then make unattached widgets parent-less toplevels
        if (pParent == m_pParent && m_bToplevelHasDeferredInit)
            pParent = NULL;
        pCurrentChild = makeObject(pParent, rClass, rID, rProps, rItems);
    }

    if (pCurrentChild)
    {
        if (pCurrentChild == m_pParent && m_bToplevelHasDeferredProperties)
            m_aDeferredProperties = rProps;
        else
            set_properties(pCurrentChild, rProps);

        for (stringmap::iterator aI = rPango.begin(), aEnd = rPango.end(); aI != aEnd; ++aI)
        {
            const OString &rKey = aI->first;
            const OString &rValue = aI->second;
            pCurrentChild->set_font_attribute(rKey, rValue);
        }

        m_pParserState->m_aAtkInfo[pCurrentChild] = rAtk;
    }

    rProps.clear();
    rPango.clear();
    rAtk.clear();
    rItems.clear();

    if (!pCurrentChild)
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back().m_pWindow;
    return pCurrentChild;
}

void VclBuilder::reorderWithinParent(Window &rWindow, sal_uInt16 nNewPosition)
{
    if (rWindow.mpWindowImpl->mpParent != rWindow.mpWindowImpl->mpRealParent)
    {
        assert(rWindow.mpWindowImpl->mpBorderWindow ==
            rWindow.mpWindowImpl->mpParent);
        assert(rWindow.mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent ==
            rWindow.mpWindowImpl->mpRealParent);
        reorderWithinParent(*rWindow.mpWindowImpl->mpBorderWindow, nNewPosition);
        return;
    }
    rWindow.reorderWithinParent(nNewPosition);
}

void VclBuilder::handleTabChild(Window *pParent, xmlreader::XmlReader &reader)
{
    OString sID;

    int nLevel = 1;
    stringmap aProperties;
    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("object"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("id"))
                    {
                        name = reader.getAttributeValue(false);
                        sID = OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                        {
                            OString sPattern = sID.copy(nDelim+1);
                            aProperties[OString("customproperty")] = sPattern;
                            sID = sID.copy(0, nDelim);
                        }
                    }
                }
            }
            else if (name.equals("property"))
                collectProperty(reader, sID, aProperties);
        }

        if (res == xmlreader::XmlReader::RESULT_END)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }

    TabControl *pTabControl = static_cast<TabControl*>(pParent);
    VclBuilder::stringmap::iterator aFind = aProperties.find(OString("label"));
    if (aFind != aProperties.end())
    {
        sal_uInt16 nPageId = pTabControl->GetCurPageId();
        pTabControl->SetPageText(nPageId,
            OStringToOUString(aFind->second, RTL_TEXTENCODING_UTF8));
        pTabControl->SetPageName(nPageId, sID);
    }
    else
        pTabControl->RemovePage(pTabControl->GetCurPageId());
}

//so that tabbing between controls goes in a visually sensible sequence
//we sort these into a best-tab-order sequence
bool VclBuilder::sortIntoBestTabTraversalOrder::operator()(const Window *pA, const Window *pB) const
{
    //sort child order within parent list by grid position
    sal_Int32 nTopA = pA->get_grid_top_attach();
    sal_Int32 nTopB = pB->get_grid_top_attach();
    if (nTopA < nTopB)
        return true;
    if (nTopA > nTopB)
        return false;
    sal_Int32 nLeftA = pA->get_grid_left_attach();
    sal_Int32 nLeftB = pB->get_grid_left_attach();
    if (nLeftA < nLeftB)
        return true;
    if (nLeftA > nLeftB)
        return false;
    //sort into two groups of pack start and pack end
    VclPackType ePackA = pA->get_pack_type();
    VclPackType ePackB = pB->get_pack_type();
    if (ePackA < ePackB)
        return true;
    if (ePackA > ePackB)
        return false;
    bool bVerticalContainer = m_pBuilder->get_window_packing_data(pA->GetParent()).m_bVerticalOrient;
    bool bPackA = pA->get_secondary();
    bool bPackB = pB->get_secondary();
    if (!bVerticalContainer)
    {
        //for horizontal boxes group secondaries before primaries
        if (bPackA > bPackB)
            return true;
        if (bPackA < bPackB)
            return false;
    }
    else
    {
        //for vertical boxes group secondaries after primaries
        if (bPackA < bPackB)
            return true;
        if (bPackA > bPackB)
            return false;
    }
    //honour relative box positions with pack group, (numerical order is reversed
    //for VCL_PACK_END, they are packed from the end back, but here we need
    //them in visual layout order so that tabbing works as expected)
    sal_Int32 nPackA = m_pBuilder->get_window_packing_data(pA).m_nPosition;
    sal_Int32 nPackB = m_pBuilder->get_window_packing_data(pB).m_nPosition;
    if (nPackA < nPackB)
        return ePackA == VCL_PACK_START ? true : false;
    if (nPackA > nPackB)
        return ePackA == VCL_PACK_START ? false : true;
    //sort labels of Frames before body
    if (pA->GetParent() == pB->GetParent())
    {
        const VclFrame *pFrameParent = dynamic_cast<const VclFrame*>(pA->GetParent());
        if (pFrameParent)
        {
            const Window *pLabel = pFrameParent->get_label_widget();
            int nFramePosA = (pA == pLabel) ? 0 : 1;
            int nFramePosB = (pB == pLabel) ? 0 : 1;
            return nFramePosA < nFramePosB;
        }
    }
    return false;
}

void VclBuilder::handleChild(Window *pParent, xmlreader::XmlReader &reader)
{
    Window *pCurrentChild = NULL;

    xmlreader::Span name;
    int nsId;
    OString sType, sInternalChild;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("type"))
        {
            name = reader.getAttributeValue(false);
            sType = OString(name.begin, name.length);
        }
        else if (name.equals("internal-child"))
        {
            name = reader.getAttributeValue(false);
            sInternalChild = OString(name.begin, name.length);
        }
    }

    if (sType.equals("tab"))
    {
        handleTabChild(pParent, reader);
        return;
    }

    int nLevel = 1;
    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                pCurrentChild = handleObject(pParent, reader);

                bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;

                if (bObjectInserted)
                {
                    //Internal-children default in glade to not having their visible bits set
                    //even though they are visible (generally anyway)
                    if (!sInternalChild.isEmpty())
                        pCurrentChild->Show();

                    //Select the first page if its a notebook
                    if (pCurrentChild->GetType() == WINDOW_TABCONTROL)
                    {
                        TabControl *pTabControl = static_cast<TabControl*>(pCurrentChild);
                        pTabControl->SetCurPageId(pTabControl->GetPageId(0));

                        //To-Do add reorder capability to the TabControl
                    }
                    else
                    {
                        // We want to sort labels before contents of frames
                        // for key board traversal, especially if there
                        // are multiple widgets using the same mnemonic
                        if (sType.equals("label"))
                        {
                            if (VclFrame *pFrameParent = dynamic_cast<VclFrame*>(pParent))
                                pFrameParent->designate_label(pCurrentChild);
                        }
                        if (sInternalChild.equals("vbox") || sInternalChild.equals("messagedialog-vbox"))
                        {
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pParent))
                                pBoxParent->set_content_area(static_cast<VclBox*>(pCurrentChild));
                        }
                        else if (sInternalChild.equals("action_area") || sInternalChild.equals("messagedialog-action_area"))
                        {
                            Window *pContentArea = pCurrentChild->GetParent();
                            assert(pContentArea && pContentArea->GetType() == WINDOW_CONTAINER);
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pContentArea ? pContentArea->GetParent() : NULL))
                            {
                                pBoxParent->set_action_area(static_cast<VclButtonBox*>(pCurrentChild));
                            }
                        }

                        //To-Do make reorder a virtual in Window, move this foo
                        //there and see above
                        std::vector<Window*> aChilds;
                        for (Window* pChild = pCurrentChild->GetWindow(WINDOW_FIRSTCHILD); pChild;
                            pChild = pChild->GetWindow(WINDOW_NEXT))
                        {
                            aChilds.push_back(pChild);
                        }

                        bool bIsButtonBox = dynamic_cast<VclButtonBox*>(pCurrentChild) != NULL;

                        //sort child order within parent so that tabbing
                        //between controls goes in a visually sensible sequence
                        std::stable_sort(aChilds.begin(), aChilds.end(), sortIntoBestTabTraversalOrder(this));
                        reorderWithinParent(aChilds, bIsButtonBox);
                    }
                }
            }
            else if (name.equals("packing"))
            {
                handlePacking(pCurrentChild, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }
}

void VclBuilder::reorderWithinParent(std::vector<Window*>& rChilds, bool bIsButtonBox)
{
    for (size_t i = 0; i < rChilds.size(); ++i)
    {
        reorderWithinParent(*rChilds[i], i);

        if (!bIsButtonBox)
            continue;

        //The first member of the group for legacy code needs WB_GROUP set and the
        //others not
        WinBits nBits = rChilds[i]->GetStyle();
        nBits &= ~WB_GROUP;
        if (i == 0)
            nBits |= WB_GROUP;
        rChilds[i]->SetStyle(nBits);
    }
}

void VclBuilder::collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals("name"))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span.equals("value"))
        {
            span = reader.getAttributeValue(false);
            sValue = OString(span.begin, span.length);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = sValue;
}

void VclBuilder::collectAtkAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals("type"))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span.equals("target"))
        {
            span = reader.getAttributeValue(false);
            sValue = OString(span.begin, span.length);
            sal_Int32 nDelim = sValue.indexOf(':');
            if (nDelim != -1)
                sValue = sValue.copy(0, nDelim);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = sValue;
}

void VclBuilder::handleAdjustment(const OString &rID, stringmap &rProperties)
{
    m_pParserState->m_aAdjustments[rID] = rProperties;
}

void VclBuilder::handleTextBuffer(const OString &rID, stringmap &rProperties)
{
    m_pParserState->m_aTextBuffers[rID] = rProperties;
}

void VclBuilder::handleRow(xmlreader::XmlReader &reader, const OString &rID, sal_Int32 nRowIndex)
{
    int nLevel = 1;

    ListStore::row aRow;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("col"))
            {
                bool bTranslated = false;
                OString sValue;
                sal_uInt32 nId = 0;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("id"))
                    {
                        name = reader.getAttributeValue(false);
                        nId = OString(name.begin, name.length).toInt32();
                    }
                    else if (nId == 0 && name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
                    {
                        sValue = getTranslation(rID, OString::number(nRowIndex));
                        bTranslated = !sValue.isEmpty();
                    }
                }

                reader.nextItem(
                    xmlreader::XmlReader::TEXT_RAW, &name, &nsId);

                if (!bTranslated)
                    sValue = OString(name.begin, name.length);

                if (aRow.size() < nId+1)
                    aRow.resize(nId+1);
                aRow[nId] = sValue;
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    m_pParserState->m_aModels[rID].m_aEntries.push_back(aRow);
}

void VclBuilder::handleListStore(xmlreader::XmlReader &reader, const OString &rID)
{
    int nLevel = 1;
    sal_Int32 nRowIndex = 0;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("row"))
                handleRow(reader, rID, nRowIndex++);
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::handleAtkObject(xmlreader::XmlReader &reader, const OString &rID, Window *pWindow)
{
    assert(pWindow);

    int nLevel = 1;

    stringmap aProperties;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("property"))
                collectProperty(reader, rID, aProperties);
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    for (stringmap::iterator aI = aProperties.begin(), aEnd = aProperties.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey.match("AtkObject::"))
            pWindow->set_property(rKey.copy(RTL_CONSTASCII_LENGTH("AtkObject::")), rValue);
        else
            SAL_WARN("vcl.layout", "unhandled atk prop: " << rKey.getStr());
    }
}

std::vector<OString> VclBuilder::handleItems(xmlreader::XmlReader &reader, const OString &rID)
{
    int nLevel = 1;

    std::vector<OString> aItems;
    sal_Int32 nItemIndex = 0;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("item"))
            {
                bool bTranslated = false;
                OString sValue;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
                    {
                        sValue = getTranslation(rID, OString::number(nItemIndex));
                        bTranslated = !sValue.isEmpty();
                    }
                }

                reader.nextItem(
                    xmlreader::XmlReader::TEXT_RAW, &name, &nsId);

                if (!bTranslated)
                    sValue = OString(name.begin, name.length);

                if (m_pStringReplace)
                {
                    OUString sTmp = (*m_pStringReplace)(OStringToOUString(sValue, RTL_TEXTENCODING_UTF8));
                    sValue = OUStringToOString(sTmp, RTL_TEXTENCODING_UTF8);
                }

                aItems.push_back(sValue);
                ++nItemIndex;
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aItems;
}

void VclBuilder::handleMenu(xmlreader::XmlReader &reader, const OString &rID)
{
    PopupMenu *pCurrentMenu = new PopupMenu;

    int nLevel = 1;

    stringmap aProperties;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("child"))
            {
                handleMenuChild(pCurrentMenu, reader);
            }
            else
            {
                ++nLevel;
                if (name.equals("property"))
                    collectProperty(reader, rID, aProperties);
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    m_aMenus.push_back(MenuAndId(rID, pCurrentMenu));
}

void VclBuilder::handleMenuChild(PopupMenu *pParent, xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    int nLevel = 1;
    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                handleMenuObject(pParent, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }
}

void VclBuilder::handleMenuObject(PopupMenu *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OString sCustomProperty;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("class"))
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name.equals("id"))
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sCustomProperty = sID.copy(nDelim+1);
                sID = sID.copy(0, nDelim);
            }
        }
    }

    int nLevel = 1;

    stringmap aProperties, aAccelerators;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("property"))
                collectProperty(reader, sID, aProperties);
            else if (name.equals("accelerator"))
                collectAccelerator(reader, aAccelerators);
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    insertMenuObject(pParent, sClass, sID, aProperties, aAccelerators);
}

void VclBuilder::handleSizeGroup(xmlreader::XmlReader &reader, const OString &rID)
{
    m_pParserState->m_aSizeGroups.push_back(SizeGroup(rID));
    SizeGroup &rSizeGroup = m_pParserState->m_aSizeGroups.back();

    int nLevel = 1;

    while(1)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("widget"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("name"))
                    {
                        name = reader.getAttributeValue(false);
                        OString sWidget = OString(name.begin, name.length);
                        sal_Int32 nDelim = sWidget.indexOf(':');
                        if (nDelim != -1)
                            sWidget = sWidget.copy(0, nDelim);
                        rSizeGroup.m_aWidgets.push_back(sWidget);
                    }
                }
            }
            else
            {
                if (name.equals("property"))
                    collectProperty(reader, rID, rSizeGroup.m_aProperties);
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

OString VclBuilder::convertMnemonicMarkup(const OString &rIn)
{
    OStringBuffer aRet(rIn);
    for (sal_Int32 nI = 0; nI < aRet.getLength(); ++nI)
    {
        if (aRet[nI] == '_' && nI+1 < aRet.getLength())
        {
            if (aRet[nI+1] != '_')
                aRet[nI] = MNEMONIC_CHAR;
            else
                aRet.remove(nI, 1);
            ++nI;
        }
    }
    return aRet.makeStringAndClear();
}

namespace
{
    KeyCode makeKeyCode(const OString &rKey)
    {
        if (rKey == "Insert")
            return KeyCode(KEY_INSERT);
        else if (rKey == "Delete")
            return KeyCode(KEY_DELETE);

        assert (rKey.getLength() == 1);
        sal_Char cChar = rKey.toChar();

        if (cChar >= 'a' && cChar <= 'z')
            return KeyCode(KEY_A + (cChar - 'a'));
        else if (cChar >= 'A' && cChar <= 'Z')
            return KeyCode(KEY_A + (cChar - 'A'));
        else if (cChar >= '0' && cChar <= '9')
            return KeyCode(KEY_0 + (cChar - 'A'));

        return KeyCode(cChar);
    }
}

void VclBuilder::insertMenuObject(PopupMenu *pParent, const OString &rClass, const OString &rID,
    stringmap &rProps, stringmap &rAccels)
{
    sal_uInt16 nOldCount = pParent->GetItemCount();
    sal_uInt16 nNewId = nOldCount + 1;

    if (rClass == "GtkMenuItem")
    {
        OUString sLabel(OStringToOUString(convertMnemonicMarkup(extractLabel(rProps)), RTL_TEXTENCODING_UTF8));
        pParent->InsertItem(nNewId, sLabel, MIB_TEXT, rID);
    }
    else if (rClass == "GtkSeparatorMenuItem")
    {
        pParent->InsertSeparator(rID);
    }

    SAL_WARN_IF(nOldCount == pParent->GetItemCount(), "vcl.layout", "probably need to implement " << rClass.getStr());

    if (nOldCount != pParent->GetItemCount())
    {
        pParent->SetHelpId(nNewId, m_sHelpRoot + rID);

        for (stringmap::iterator aI = rProps.begin(), aEnd = rProps.end(); aI != aEnd; ++aI)
        {
            const OString &rKey = aI->first;
            const OString &rValue = aI->second;

            if (rKey == "tooltip-markup")
                pParent->SetTipHelpText(nNewId, OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
            else if (rKey == "tooltip-text")
                pParent->SetTipHelpText(nNewId, OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
            else if (rKey == "visible")
                pParent->ShowItem(nNewId, toBool(rValue));
            else if (rKey == "has-default" && toBool(rValue))
                pParent->SetSelectedEntry(nNewId);
            else
                SAL_INFO("vcl.layout", "unhandled property: " << rKey.getStr());
        }

        for (stringmap::iterator aI = rAccels.begin(), aEnd = rAccels.end(); aI != aEnd; ++aI)
        {
            const OString &rSignal = aI->first;
            const OString &rValue = aI->second;

            if (rSignal == "activate")
                pParent->SetAccelKey(nNewId, makeKeyCode(rValue));
            else
                SAL_INFO("vcl.layout", "unhandled accelerator for: " << rSignal.getStr());
        }
    }

    rProps.clear();
}

Window* VclBuilder::handleObject(Window *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OString sCustomProperty;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("class"))
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name.equals("id"))
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sCustomProperty = sID.copy(nDelim+1);
                sID = sID.copy(0, nDelim);
            }
        }
    }

    if (sClass == "GtkListStore")
    {
        handleListStore(reader, sID);
        return NULL;
    }
    else if (sClass == "GtkMenu")
    {
        handleMenu(reader, sID);
        return NULL;
    }
    else if (sClass == "GtkSizeGroup")
    {
        handleSizeGroup(reader, sID);
        return NULL;
    }
    else if (sClass == "AtkObject")
    {
        handleAtkObject(reader, sID, pParent);
        return NULL;
    }

    int nLevel = 1;

    stringmap aProperties, aPangoAttributes, aAtkAttributes;
    std::vector<OString> aItems;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    Window *pCurrentChild = NULL;
    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals("child"))
            {
                if (!pCurrentChild)
                {
                    pCurrentChild = insertObject(pParent, sClass, sID,
                        aProperties, aPangoAttributes, aAtkAttributes, aItems);
                }
                handleChild(pCurrentChild, reader);
            }
            else if (name.equals("items"))
                aItems = handleItems(reader, sID);
            else
            {
                ++nLevel;
                if (name.equals("property"))
                    collectProperty(reader, sID, aProperties);
                else if (name.equals("attribute"))
                    collectPangoAttribute(reader, aPangoAttributes);
                else if (name.equals("relation"))
                    collectAtkAttribute(reader, aAtkAttributes);
                else if (name.equals("action-widget"))
                    handleActionWidget(reader);
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    if (sClass == "GtkAdjustment")
    {
        handleAdjustment(sID, aProperties);
        return NULL;
    }
    else if (sClass == "GtkTextBuffer")
    {
        handleTextBuffer(sID, aProperties);
        return NULL;
    }

    if (!pCurrentChild)
    {
        pCurrentChild = insertObject(pParent, sClass, sID, aProperties,
            aPangoAttributes, aAtkAttributes, aItems);
    }

    return pCurrentChild;
}

void VclBuilder::handlePacking(Window *pCurrent, xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    int nLevel = 1;

    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            ++nLevel;
            if (name.equals("property"))
                applyPackingProperty(pCurrent, reader);
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::applyPackingProperty(Window *pCurrent,
    xmlreader::XmlReader &reader)
{
    if (!pCurrent)
        return;

    xmlreader::Span name;
    int nsId;

    if (pCurrent->GetType() == WINDOW_SCROLLWINDOW)
    {
        std::map<Window*, Window*>::iterator aFind = m_pParserState->m_aRedundantParentWidgets.find(pCurrent);
        if (aFind != m_pParserState->m_aRedundantParentWidgets.end())
        {
            pCurrent = aFind->second;
            assert(pCurrent);
        }
    }

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            OString sKey(name.begin, name.length);
            sKey = sKey.replace('_', '-');
            reader.nextItem(
                xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
            OString sValue(name.begin, name.length);

            if (sKey == "expand")
            {
                bool bTrue = (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1');
                pCurrent->set_expand(bTrue);
            }
            else if (sKey == "fill")
            {
                bool bTrue = (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1');
                pCurrent->set_fill(bTrue);
            }
            else if (sKey == "pack-type")
            {
                VclPackType ePackType = (sValue[0] == 'e' || sValue[0] == 'E') ? VCL_PACK_END : VCL_PACK_START;
                pCurrent->set_pack_type(ePackType);
            }
            else if (sKey == "left-attach")
            {
                pCurrent->set_grid_left_attach(sValue.toInt32());
            }
            else if (sKey == "top-attach")
            {
                pCurrent->set_grid_top_attach(sValue.toInt32());
            }
            else if (sKey == "width")
            {
                pCurrent->set_grid_width(sValue.toInt32());
            }
            else if (sKey == "height")
            {
                pCurrent->set_grid_height(sValue.toInt32());
            }
            else if (sKey == "padding")
            {
                pCurrent->set_padding(sValue.toInt32());
            }
            else if (sKey == "position")
            {
                set_window_packing_position(pCurrent, sValue.toInt32());
            }
            else if (sKey == "secondary")
            {
                pCurrent->set_secondary(toBool(sValue));
            }
            else
            {
                SAL_WARN("vcl.layout", "unknown packing: " << sKey.getStr());
            }
        }
    }
}

OString VclBuilder::getTranslation(const OString &rID, const OString &rProperty) const
{
    Translations::const_iterator aWidgetFind = m_pParserState->m_aTranslations.find(rID);
    if (aWidgetFind != m_pParserState->m_aTranslations.end())
    {
        const WidgetTranslations &rWidgetTranslations = aWidgetFind->second;
        WidgetTranslations::const_iterator aPropertyFind = rWidgetTranslations.find(rProperty);
        if (aPropertyFind != rWidgetTranslations.end())
            return aPropertyFind->second;
    }
    return OString();
}

void VclBuilder::collectProperty(xmlreader::XmlReader &reader, const OString &rID, stringmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    OString sProperty;
    OString sValue;

    bool bTranslated = false;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
        {
            sValue = getTranslation(rID, sProperty);
            bTranslated = !sValue.isEmpty();
        }

    }

    reader.nextItem(xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
    if (!bTranslated)
        sValue = OString(name.begin, name.length);

    if (!sProperty.isEmpty())
    {
        sProperty = sProperty.replace('_', '-');
        if (m_pStringReplace)
        {
            OUString sTmp = (*m_pStringReplace)(OStringToOUString(sValue, RTL_TEXTENCODING_UTF8));
            rMap[sProperty] = OUStringToOString(sTmp, RTL_TEXTENCODING_UTF8);
        }
        else
        {
            rMap[sProperty] = sValue;
        }
    }
}

void VclBuilder::handleActionWidget(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    OString sResponse;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("response"))
        {
            name = reader.getAttributeValue(false);
            sResponse = OString(name.begin, name.length);
        }
    }

    reader.nextItem(xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
    OString sID = OString(name.begin, name.length);
    sal_Int32 nDelim = sID.indexOf(':');
    if (nDelim != -1)
        sID = sID.copy(0, nDelim);
    set_response(sID, sResponse.toInt32());
}


void VclBuilder::collectAccelerator(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("key"))
        {
            name = reader.getAttributeValue(false);
            sValue = OString(name.begin, name.length);
        }
        else if (name.equals("signal"))
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }

    }

    if (!sProperty.isEmpty() && !sValue.isEmpty())
    {
        rMap[sProperty] = sValue;
    }
}

Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? NULL : m_aChildren[0].m_pWindow;
}

Window *VclBuilder::get_by_name(OString sID)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pWindow;
    }

    return NULL;
}

PopupMenu *VclBuilder::get_menu(OString sID)
{
    for (std::vector<MenuAndId>::iterator aI = m_aMenus.begin(),
         aEnd = m_aMenus.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pMenu;
    }

    return NULL;
}

short VclBuilder::get_response(const Window *pWindow) const
{
    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
        {
            return aI->m_nResponseId;
        }
    }

    //how did we not find sID ?
    assert(false);
    return RET_CANCEL;
}

void VclBuilder::set_response(OString sID, short nResponse)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
        {
            aI->m_nResponseId = nResponse;
            return;
        }
    }

    //how did we not find sID ?
    assert(false);
}

void VclBuilder::delete_by_name(OString sID)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
        {
            delete aI->m_pWindow;
            m_aChildren.erase(aI);
            break;
        }
    }
}

void VclBuilder::delete_by_window(const Window *pWindow)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
        {
            delete aI->m_pWindow;
            m_aChildren.erase(aI);
            break;
        }
    }
}

OString VclBuilder::get_by_window(const Window *pWindow) const
{
    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            return aI->m_sID;
    }

    return OString();
}

VclBuilder::PackingData VclBuilder::get_window_packing_data(const Window *pWindow) const
{
    //We've stored the return of new Control, some of these get
    //border windows placed around them which are what you get
    //from GetChild, so scoot up a level if necessary to get the
    //window whose position value we have
    const Window *pPropHolder = pWindow->mpWindowImpl->mpClientWindow ?
        pWindow->mpWindowImpl->mpClientWindow : pWindow;

    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pPropHolder)
            return aI->m_aPackingData;
    }

    return PackingData();
}

void VclBuilder::set_window_packing_position(const Window *pWindow, sal_Int32 nPosition)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            aI->m_aPackingData.m_nPosition = nPosition;
    }
}

const VclBuilder::ListStore *VclBuilder::get_model_by_name(OString sID) const
{
    std::map<OString, ListStore>::const_iterator aI = m_pParserState->m_aModels.find(sID);
    if (aI != m_pParserState->m_aModels.end())
        return &(aI->second);
    return NULL;
}

const VclBuilder::TextBuffer *VclBuilder::get_buffer_by_name(OString sID) const
{
    std::map<OString, TextBuffer>::const_iterator aI = m_pParserState->m_aTextBuffers.find(sID);
    if (aI != m_pParserState->m_aTextBuffers.end())
        return &(aI->second);
    return NULL;
}

const VclBuilder::Adjustment *VclBuilder::get_adjustment_by_name(OString sID) const
{
    std::map<OString, Adjustment>::const_iterator aI = m_pParserState->m_aAdjustments.find(sID);
    if (aI != m_pParserState->m_aAdjustments.end())
        return &(aI->second);
    return NULL;
}

void VclBuilder::mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId)
{
    for (std::vector<ListStore::row>::const_iterator aI = rStore.m_aEntries.begin(), aEnd = rStore.m_aEntries.end();
        aI != aEnd; ++aI)
    {
        const ListStore::row &rRow = *aI;
        sal_uInt16 nEntry = rTarget.InsertEntry(OStringToOUString(rRow[0], RTL_TEXTENCODING_UTF8));
        if (rRow.size() > 1)
        {
            sal_IntPtr nValue = rRow[1].toInt32();
            rTarget.SetEntryData(nEntry, (void*)nValue);
        }
    }
    if (nActiveId < rStore.m_aEntries.size())
        rTarget.SelectEntryPos(nActiveId);
}

void VclBuilder::mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment)
{
    int nMul = rtl_math_pow10Exp(1, rTarget.GetDecimalDigits());

    for (stringmap::const_iterator aI = rAdjustment.begin(), aEnd = rAdjustment.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey == "upper")
        {
            sal_Int64 nUpper = rValue.toDouble() * nMul;
            rTarget.SetMax(nUpper);
            rTarget.SetLast(nUpper);
        }
        else if (rKey == "lower")
        {
            sal_Int64 nLower = rValue.toDouble() * nMul;
            rTarget.SetMin(nLower);
            rTarget.SetFirst(nLower);
        }
        else if (rKey == "value")
        {
            sal_Int64 nValue = rValue.toDouble() * nMul;
            rTarget.SetValue(nValue);
        }
        else if (rKey == "step-increment")
        {
            sal_Int64 nSpinSize = rValue.toDouble() * nMul;
            rTarget.SetSpinSize(nSpinSize);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey.getStr());
        }
    }
}

void VclBuilder::mungeAdjustment(TimeField &rTarget, const Adjustment &rAdjustment)
{
    for (stringmap::const_iterator aI = rAdjustment.begin(), aEnd = rAdjustment.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey == "upper")
        {
            Time aUpper(rValue.toInt32());
            rTarget.SetMax(aUpper);
            rTarget.SetLast(aUpper);
        }
        else if (rKey == "lower")
        {
            Time aLower(rValue.toInt32());
            rTarget.SetMin(aLower);
            rTarget.SetFirst(aLower);
        }
        else if (rKey == "value")
        {
            Time aValue(rValue.toInt32());
            rTarget.SetTime(aValue);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey.getStr());
        }
    }
}

void VclBuilder::mungeAdjustment(DateField &rTarget, const Adjustment &rAdjustment)
{
    for (stringmap::const_iterator aI = rAdjustment.begin(), aEnd = rAdjustment.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey == "upper")
        {
            Date aUpper(rValue.toInt32());
            rTarget.SetMax(aUpper);
            rTarget.SetLast(aUpper);
        }
        else if (rKey == "lower")
        {
            Date aLower(rValue.toInt32());
            rTarget.SetMin(aLower);
            rTarget.SetFirst(aLower);
        }
        else if (rKey == "value")
        {
            Date aValue(rValue.toInt32());
            rTarget.SetDate(aValue);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey.getStr());
        }
    }
}

void VclBuilder::mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment)
{
    for (stringmap::const_iterator aI = rAdjustment.begin(), aEnd = rAdjustment.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey == "upper")
            rTarget.SetRangeMax(rValue.toInt32());
        else if (rKey == "lower")
            rTarget.SetRangeMin(rValue.toInt32());
        else if (rKey == "value")
            rTarget.SetThumbPos(rValue.toInt32());
        else if (rKey == "step-increment")
            rTarget.SetLineSize(rValue.toInt32());
        else if (rKey == "page-increment")
            rTarget.SetPageSize(rValue.toInt32());
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey.getStr());
        }
    }
}

void VclBuilder::mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer)
{
    for (stringmap::const_iterator aI = rTextBuffer.begin(), aEnd = rTextBuffer.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;

        if (rKey == "text")
            rTarget.SetText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey.getStr());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
