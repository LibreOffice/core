/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
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
#include <vcl/IPrioritable.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/throbber.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/settings.hxx>
#include <vcl/slider.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>
#include <xmlreader/xmlreader.hxx>

#ifdef DISABLE_DYNLOADING
#include <dlfcn.h>
#endif

namespace
{
    sal_uInt16 mapStockToImageResource(const OString& sType)
    {
        sal_uInt16 nRet = 0;
        if (sType == "gtk-index")
            nRet = SV_RESID_BITMAP_INDEX;
        else if (sType == "gtk-refresh")
            nRet = SV_RESID_BITMAP_REFRESH;
        return nRet;
    }

    SymbolType mapStockToSymbol(const OString& sType)
    {
        SymbolType eRet = SymbolType::DONTKNOW;
        if (sType == "gtk-media-next")
            eRet = SymbolType::NEXT;
        else if (sType == "gtk-media-previous")
            eRet = SymbolType::PREV;
        else if (sType == "gtk-media-play")
            eRet = SymbolType::PLAY;
        else if (sType == "gtk-media-stop")
            eRet = SymbolType::STOP;
        else if (sType == "gtk-goto-first")
            eRet = SymbolType::FIRST;
        else if (sType == "gtk-goto-last")
            eRet = SymbolType::LAST;
        else if (sType == "gtk-go-back")
            eRet = SymbolType::ARROW_LEFT;
        else if (sType == "gtk-go-forward")
            eRet = SymbolType::ARROW_RIGHT;
        else if (sType == "gtk-go-up")
            eRet = SymbolType::ARROW_UP;
        else if (sType == "gtk-go-down")
            eRet = SymbolType::ARROW_DOWN;
        else if (sType == "gtk-missing-image")
            eRet = SymbolType::IMAGE;
        else if (sType == "gtk-help")
            eRet = SymbolType::HELP;
        else if (sType == "gtk-close")
            eRet = SymbolType::CLOSE;
        else if (mapStockToImageResource(sType))
            eRet = SymbolType::IMAGE;
        return eRet;
    }

    void setupFromActionName(Button *pButton, VclBuilder::stringmap &rMap, const css::uno::Reference<css::frame::XFrame>& rFrame);
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
        aTransBuf.append("/res/");
        OUString sLang(rLanguageTag.getLanguage());
        switch (i)
        {
            case 0:
                sLang = sLang + "-" + rLanguageTag.getCountry();
                break;
            default:
                break;
        }
        aTransBuf.append(sLang);
        aTransBuf.append(".zip");
        sal_Int32 nEndName = rUri.lastIndexOf('.');
        if (nEndName == -1)
            nEndName = rUri.getLength();
        OUString sZippedFile(rUri.copy(nLastSlash + 1, nEndName - nLastSlash - 1) + "/" + sLang + ".ui");
        try
        {
            css::uno::Reference<css::packages::zip::XZipFileAccess2> xNameAccess =
                css::packages::zip::ZipFileAccess::createWithURL(
                        comphelper::getProcessComponentContext(), aTransBuf.makeStringAndClear());
            if (!xNameAccess.is())
                continue;
            css::uno::Reference<css::io::XInputStream> xInputStream(xNameAccess->getByName(sZippedFile), css::uno::UNO_QUERY);
            if (!xInputStream.is())
                continue;
            OStringBuffer sStr;
            for (;;)
            {
                sal_Int32 const size = 2048;
                css::uno::Sequence< sal_Int8 > data(size);
                sal_Int32 n = xInputStream->readBytes(data, size);
                sStr.append(reinterpret_cast<const sal_Char *>(data.getConstArray()), n);
                if (n < size)
                    break;
            }

            xmlreader::XmlReader reader(sStr.getStr(), sStr.getLength());
            handleTranslations(reader);
            break;
        }
        catch (const css::uno::Exception &)
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

VclBuilder::VclBuilder(vcl::Window *pParent, const OUString& sUIDir, const OUString& sUIFile, const OString& sID, const css::uno::Reference<css::frame::XFrame>& rFrame)
    : m_sID(sID)
    , m_sHelpRoot(OUStringToOString(sUIFile, RTL_TEXTENCODING_UTF8))
    , m_pStringReplace(ResMgr::GetReadStringHook())
    , m_pParent(pParent)
    , m_bToplevelParentFound(false)
    , m_pParserState(new ParserState)
    , m_xFrame(rFrame)
{
    m_bToplevelHasDeferredInit = pParent &&
        ((pParent->IsSystemWindow() && static_cast<SystemWindow*>(pParent)->isDeferredInit()) ||
         (pParent->IsDockingWindow() && static_cast<DockingWindow*>(pParent)->isDeferredInit()));
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
    catch (const css::uno::Exception &rExcept)
    {
        SAL_WARN("vcl.layout", "Unable to read .ui file: " << rExcept.Message);
        throw;
    }

    //Set Mnemonic widgets when everything has been imported
    for (std::vector<MnemonicWidgetMap>::iterator aI = m_pParserState->m_aMnemonicWidgetMaps.begin(),
        aEnd = m_pParserState->m_aMnemonicWidgetMaps.end(); aI != aEnd; ++aI)
    {
        FixedText *pOne = get<FixedText>(aI->m_sID);
        vcl::Window *pOther = get<vcl::Window>(aI->m_sValue);
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing either source " << aI->m_sID << " or target " << aI->m_sValue << " member of Mnemonic Widget Mapping");
        if (pOne && pOther)
            pOne->set_mnemonic_widget(pOther);
    }

    //Set a11y relations when everything has been imported
    for (AtkMap::iterator aI = m_pParserState->m_aAtkInfo.begin(),
         aEnd = m_pParserState->m_aAtkInfo.end(); aI != aEnd; ++aI)
    {
        vcl::Window *pSource = aI->first;
        const stringmap &rMap = aI->second;

        for (stringmap::const_iterator aP = rMap.begin(),
            aEndP = rMap.end(); aP != aEndP; ++aP)
        {
            const OString &rTarget = aP->second;
            vcl::Window *pTarget = get<vcl::Window>(rTarget);
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
        // pStore may be empty
        const ListStore *pStore = get_model_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget, "vcl", "missing elements of combobox");
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
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get<vcl::Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget, "vcl", "missing NumericFormatter element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aTimeFormatterAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aTimeFormatterAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        TimeField *pTarget = dynamic_cast<TimeField*>(get<vcl::Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aDateFormatterAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aDateFormatterAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        DateField *pTarget = dynamic_cast<DateField*>(get<vcl::Window>(aI->m_sID));
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

    //Set Scale(Slider) adjustments
    std::vector<WidgetAdjustmentMap>::iterator aIterator;
    for (aIterator  = m_pParserState->m_aSliderAdjustmentMaps.begin();
         aIterator != m_pParserState->m_aSliderAdjustmentMaps.end(); ++aIterator)
    {
        Slider* pTarget = dynamic_cast<Slider*>(get<vcl::Window>(aIterator->m_sID));
        const Adjustment* pAdjustment = get_adjustment_by_name(aIterator->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scale(slider)/adjustment");
        if (pTarget && pAdjustment)
        {
            mungeAdjustment(*pTarget, *pAdjustment);
        }
    }

    //Set size-groups when all widgets have been imported
    for (std::vector<SizeGroup>::iterator aI = m_pParserState->m_aSizeGroups.begin(),
        aEnd = m_pParserState->m_aSizeGroups.end(); aI != aEnd; ++aI)
    {
        std::shared_ptr<VclSizeGroup> xGroup(std::make_shared<VclSizeGroup>());

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
            vcl::Window* pWindow = get<vcl::Window>(aW->getStr());
            pWindow->add_to_size_group(xGroup);
        }
    }

    //Set button images when everything has been imported
    std::set<OString> aImagesToBeRemoved;
    for (std::vector<ButtonImageWidgetMap>::iterator aI = m_pParserState->m_aButtonImageWidgetMaps.begin(),
         aEnd = m_pParserState->m_aButtonImageWidgetMaps.end(); aI != aEnd; ++aI)
    {
        PushButton *pTargetButton = nullptr;
        RadioButton *pTargetRadio = nullptr;
        Button *pTarget = nullptr;

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
            SAL_WARN_IF(eType == SymbolType::DONTKNOW, "vcl", "missing stock image element for button");
            if (eType == SymbolType::DONTKNOW)
                continue;
            if (!aI->m_bRadio)
            {
                pTargetButton->SetSymbol(eType);
                //fdo#76457 keep symbol images small e.g. tools->customize->menu
                //but images the right size. Really the PushButton::CalcMinimumSize
                //and PushButton::ImplDrawPushButton are the better place to handle
                //this, but its such a train-wreck
                if (eType != SymbolType::IMAGE)
                    pTargetButton->SetStyle(pTargetButton->GetStyle() | WB_SMALLSTYLE);
            }
            else
                SAL_WARN_IF(eType != SymbolType::IMAGE, "vcl.layout", "inimplemented symbol type for radiobuttons");
            if (eType == SymbolType::IMAGE)
            {
                Bitmap aBitmap(VclResId(mapStockToImageResource(rImageInfo.m_sStock)));
                Image const aImage(aBitmap);
                if (!aI->m_bRadio)
                    pTargetButton->SetModeImage(aImage);
                else
                    pTargetRadio->SetModeRadioImage(aImage);
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
    for (auto aI = m_pParserState->m_aRedundantParentWidgets.begin(),
        aEnd = m_pParserState->m_aRedundantParentWidgets.end(); aI != aEnd; ++aI)
    {
        delete_by_window(aI->first);
    }

    //fdo#67378 merge the label into the disclosure button
    for (VclPtr<VclExpander> const & pOne : m_pParserState->m_aExpanderWidgets)
    {
        vcl::Window *pChild = pOne->get_child();
        vcl::Window* pLabel = pOne->GetWindow(GetWindowType::LastChild);
        if (pLabel && pLabel != pChild && pLabel->GetType() == WINDOW_FIXEDTEXT)
        {
            FixedText *pLabelWidget = static_cast<FixedText*>(pLabel);
            pOne->set_label(pLabelWidget->GetText());
            delete_by_window(pLabel);
        }
    }

    //drop maps, etc. that we don't need again
    m_pParserState.reset();

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
        SAL_WARN_IF(nButtons && !bHasDefButton, "vcl.layout", "No default button defined in " << sUIFile);
    }
#endif
}

VclBuilder::~VclBuilder()
{
    disposeBuilder();
}

void VclBuilder::disposeBuilder()
{
    for (std::vector<WinAndId>::reverse_iterator aI = m_aChildren.rbegin(),
         aEnd = m_aChildren.rend(); aI != aEnd; ++aI)
    {
        aI->m_pWindow.disposeAndClear();
    }
    m_aChildren.clear();

    for (std::vector<MenuAndId>::reverse_iterator aI = m_aMenus.rbegin(),
         aEnd = m_aMenus.rend(); aI != aEnd; ++aI)
    {
        aI->m_pMenu.disposeAndClear();
    }
    m_aMenus.clear();
    m_pParent.clear();
}

void VclBuilder::handleTranslations(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    OString sID, sProperty;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::Raw, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
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

        if (res == xmlreader::XmlReader::Result::Text && !sID.isEmpty())
        {
            OString sTranslation(name.begin, name.length);
            m_pParserState->m_aTranslations[sID][sProperty] = sTranslation;
        }

        if (res == xmlreader::XmlReader::Result::End)
            sID.clear();

        if (res == xmlreader::XmlReader::Result::Done)
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
    bool extractDrawValue(VclBuilder::stringmap& rMap)
    {
        bool bDrawValue = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("draw_value"));
        if (aFind != rMap.end())
        {
            bDrawValue = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDrawValue;
    }

    OString extractValuePos(VclBuilder::stringmap& rMap)
    {
        OString sRet("top");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("value_pos"));
        if (aFind != rMap.end())
        {
            sRet = aFind->second;
            rMap.erase(aFind);
        }
        return sRet;
    }

    OString extractTypeHint(VclBuilder::stringmap &rMap)
    {
        OString sRet("normal");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("type-hint"));
        if (aFind != rMap.end())
        {
            sRet = aFind->second;
            rMap.erase(aFind);
        }
        return sRet;
    }

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

    bool extractDecorated(VclBuilder::stringmap &rMap)
    {
        bool bDecorated = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("decorated"));
        if (aFind != rMap.end())
        {
            bDecorated = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDecorated;
    }

    bool extractCloseable(VclBuilder::stringmap &rMap)
    {
        bool bCloseable = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("deletable"));
        if (aFind != rMap.end())
        {
            bCloseable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bCloseable;
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
        else if (rType == "gtk-find")
            return (VclResId(SV_BUTTONTEXT_FIND).toString());
        else if (rType == "gtk-stop")
            return (VclResId(SV_BUTTONTEXT_STOP).toString());
        else if (rType == "gtk-connect")
            return (VclResId(SV_BUTTONTEXT_CONNECT).toString());
        else if (rType == "gtk-yes")
            return (VclResId(SV_BUTTONTEXT_YES).toString());
        else if (rType == "gtk-no")
            return (VclResId(SV_BUTTONTEXT_NO).toString());
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

    bool extractVisible(VclBuilder::stringmap &rMap)
    {
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("visible"));
        if (aFind != rMap.end())
        {
            return toBool(aFind->second);
        }
        return false;
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

    void setupFromActionName(Button *pButton, VclBuilder::stringmap &rMap, const css::uno::Reference<css::frame::XFrame>& rFrame)
    {
        if (!rFrame.is())
            return;

        OUString aCommand(OStringToOUString(extractActionName(rMap), RTL_TEXTENCODING_UTF8));
        if (aCommand.isEmpty())
            return;

        OUString aLabel(vcl::CommandInfoProvider::Instance().GetLabelForCommand(aCommand, rFrame));
        if (!aLabel.isEmpty())
            pButton->SetText(aLabel);

        OUString aTooltip(vcl::CommandInfoProvider::Instance().GetTooltipForCommand(aCommand, rFrame));
        if (!aTooltip.isEmpty())
            pButton->SetQuickHelpText(aTooltip);

        Image aImage(vcl::CommandInfoProvider::Instance().GetImageForCommand(aCommand, rFrame));
        pButton->SetModeImage(aImage);

        pButton->SetCommandHandler(aCommand);
    }

    VclPtr<Button> extractStockAndBuildPushButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER;

        nBits |= extractRelief(rMap);

        bool bIsStock = extractStock(rMap);

        VclPtr<Button> xWindow;

        if (bIsStock)
        {
            OString sType = extractLabel(rMap);
            if (sType == "gtk-ok")
                xWindow = VclPtr<OKButton>::Create(pParent, nBits);
            else if (sType == "gtk-cancel")
                xWindow = VclPtr<CancelButton>::Create(pParent, nBits);
            else if (sType == "gtk-close")
                xWindow = VclPtr<CloseButton>::Create(pParent, nBits);
            else if (sType == "gtk-help")
                xWindow = VclPtr<HelpButton>::Create(pParent, nBits);
            else
            {
                xWindow = VclPtr<PushButton>::Create(pParent, nBits);
                xWindow->SetText(getStockText(sType));
            }
        }

        if (!xWindow)
            xWindow = VclPtr<PushButton>::Create(pParent, nBits);
        return xWindow;
    }

    VclPtr<Button> extractStockAndBuildMenuButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<Button> xWindow = VclPtr<MenuButton>::Create(pParent, nBits);

        if (extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }

        return xWindow;
    }

    VclPtr<Button> extractStockAndBuildMenuToggleButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<Button> xWindow = VclPtr<MenuToggleButton>::Create(pParent, nBits);

        if (extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }

        return xWindow;
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

    FieldUnit detectMetricUnit(const OString& sUnit)
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

    WinBits extractDeferredBits(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK|WB_HIDE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        if (extractCloseable(rMap))
            nBits |= WB_CLOSEABLE;
        OString sBorder = VclBuilder::extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nBits |= WB_BORDER;
        if (!extractDecorated(rMap))
            nBits |= WB_OWNERDRAWDECORATION;
        OString sType(extractTypeHint(rMap));
        if (sType == "utility")
            nBits |= WB_SYSTEMWINDOW | WB_DIALOGCONTROL | WB_MOVEABLE;
        else if (sType == "popup-menu")
            nBits |= WB_SYSTEMWINDOW | WB_DIALOGCONTROL | WB_POPUP;
        else if (sType == "dock")
            nBits |= WB_DOCKABLE | WB_MOVEABLE;
        else
            nBits |= WB_MOVEABLE;
        return nBits;
    }
}

FieldUnit VclBuilder::detectUnit(OString const& rString)
{
    OString const unit(extractUnit(rString));
    return detectMetricUnit(unit);
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

bool VclBuilder::extractAdjustmentToMap(const OString& id, VclBuilder::stringmap& rMap, std::vector<WidgetAdjustmentMap>& rAdjustmentMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
    if (aFind != rMap.end())
    {
        rAdjustmentMap.push_back(WidgetAdjustmentMap(id, aFind->second));
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

vcl::Window* VclBuilder::prepareWidgetOwnScrolling(vcl::Window *pParent, WinBits &rWinStyle)
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

void VclBuilder::cleanupWidgetOwnScrolling(vcl::Window *pScrollParent, vcl::Window *pWindow, stringmap &rMap)
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

VclPtr<vcl::Window> VclBuilder::makeObject(vcl::Window *pParent, const OString &name, const OString &id,
    stringmap &rMap)
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
            VclPtrInstance<TabPage> pPage(pTabControl);
            pPage->Show();

            //Make up a name for it
            OString sTabPageId = get_by_window(pParent) +
                OString("-page") +
                OString::number(nNewPageCount);
            m_aChildren.push_back(WinAndId(sTabPageId, pPage, false));
            pPage->SetHelpId(m_sHelpRoot + sTabPageId);

            //And give the page one container as a child to make it a layout enabled
            //tab page
            VclPtrInstance<VclBin> pContainer(pPage);
            pContainer->Show();
            m_aChildren.push_back(WinAndId(OString(), pContainer, false));
            pContainer->SetHelpId(m_sHelpRoot + sTabPageId + OString("-bin"));
            pParent = pContainer;

            pTabControl->SetTabPage(nNewPageId, pPage);
        }
    }

    if (bIsPlaceHolder || name == "GtkTreeSelection")
        return nullptr;

    extractButtonImage(id, rMap, name == "GtkRadioButton");

    VclPtr<vcl::Window> xWindow;
    if (name == "GtkDialog")
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        xWindow = VclPtr<Dialog>::Create(pParent, nBits);
    }
    else if (name == "GtkMessageDialog")
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        xWindow = VclPtr<MessageDialog>::Create(pParent, nBits);
    }
    else if (name == "GtkBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            xWindow = VclPtr<VclVBox>::Create(pParent);
        else
            xWindow = VclPtr<VclHBox>::Create(pParent);
    }
    else if (name == "GtkPaned")
    {
        bVertical = extractOrientation(rMap);
        assert(bVertical && "hori not implemented, shouldn't be hard though");
        xWindow = VclPtr<VclVPaned>::Create(pParent);
    }
    else if (name == "GtkHBox")
        xWindow = VclPtr<VclHBox>::Create(pParent);
    else if (name == "GtkVBox")
        xWindow = VclPtr<VclVBox>::Create(pParent);
    else if (name == "GtkButtonBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            xWindow = VclPtr<VclVButtonBox>::Create(pParent);
        else
            xWindow = VclPtr<VclHButtonBox>::Create(pParent);
    }
    else if (name == "GtkHButtonBox")
        xWindow = VclPtr<VclHButtonBox>::Create(pParent);
    else if (name == "GtkVButtonBox")
        xWindow = VclPtr<VclVButtonBox>::Create(pParent);
    else if (name == "GtkGrid")
        xWindow = VclPtr<VclGrid>::Create(pParent);
    else if (name == "GtkFrame")
        xWindow = VclPtr<VclFrame>::Create(pParent);
    else if (name == "GtkExpander")
    {
        VclPtrInstance<VclExpander> pExpander(pParent);
        m_pParserState->m_aExpanderWidgets.push_back(pExpander);
        xWindow = pExpander;
    }
    else if (name == "GtkAlignment")
        xWindow = VclPtr<VclAlignment>::Create(pParent);
    else if (name == "GtkButton")
    {
        VclPtr<Button> xButton;
        OString sMenu = extractCustomProperty(rMap);
        if (sMenu.isEmpty())
            xButton = extractStockAndBuildPushButton(pParent, rMap);
        else
        {
            xButton = extractStockAndBuildMenuButton(pParent, rMap);
            m_pParserState->m_aButtonMenuMaps.push_back(ButtonMenuMap(id, sMenu));
        }
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkToggleButton")
    {
        VclPtr<Button> xButton;
        OString sMenu = extractCustomProperty(rMap);
        assert(sMenu.getLength() && "not implemented yet");
        xButton = extractStockAndBuildMenuToggleButton(pParent, rMap);
        m_pParserState->m_aButtonMenuMaps.push_back(ButtonMenuMap(id, sMenu));
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkRadioButton")
    {
        extractGroup(id, rMap);
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OString sWrap = extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        VclPtr<RadioButton> xButton = VclPtr<RadioButton>::Create(pParent, nBits);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        xWindow = xButton;
    }
    else if (name == "GtkCheckButton")
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OString sWrap = extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        //maybe always import as TriStateBox and enable/disable tristate
        bool bIsTriState = extractInconsistent(rMap);
        VclPtr<CheckBox> xCheckBox;
        if (bIsTriState)
            xCheckBox = VclPtr<TriStateBox>::Create(pParent, nBits);
        else
            xCheckBox = VclPtr<CheckBox>::Create(pParent, nBits);
        if (bIsTriState)
            xCheckBox->SetState(TRISTATE_INDET);
        xCheckBox->SetImageAlign(ImageAlign::Left); //default to left
        xWindow = xCheckBox;
    }
    else if (name == "GtkSpinButton")
    {
        OString sAdjustment = extractAdjustment(rMap);
        OString sPattern = extractCustomProperty(rMap);
        OString sUnit = extractUnit(sPattern);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_BORDER|WB_3DLOOK;
        if (!id.endsWith("-nospin"))
            nBits |= WB_SPIN | WB_REPEAT;

        if (sPattern.isEmpty())
        {
            connectNumericFormatterAdjustment(id, sAdjustment);
            SAL_INFO("vcl.layout", "making numeric field for " << name.getStr() << " " << sUnit.getStr());
            xWindow = VclPtr<NumericField>::Create(pParent, nBits);
        }
        else
        {
            if (sPattern == "hh:mm")
            {
                connectTimeFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making time field for " << name.getStr() << " " << sUnit.getStr());
                xWindow = VclPtr<TimeField>::Create(pParent, nBits);
            }
            else if (sPattern == "yy:mm:dd")
            {
                connectDateFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making date field for " << name.getStr() << " " << sUnit.getStr());
                xWindow = VclPtr<DateField>::Create(pParent, nBits);
            }
            else
            {
                connectNumericFormatterAdjustment(id, sAdjustment);
                FieldUnit eUnit = detectMetricUnit(sUnit);
                SAL_INFO("vcl.layout", "making metric field for " << name.getStr() << " " << sUnit.getStr());
                VclPtrInstance<MetricField> xField(pParent, nBits);
                xField->SetUnit(eUnit);
                if (eUnit == FUNIT_CUSTOM)
                    xField->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
                xWindow = xField;
            }
        }
    }
    else if (name == "GtkLinkButton")
        xWindow = VclPtr<FixedHyperlink>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
    else if ((name == "GtkComboBox") || (name == "GtkComboBoxText") || (name == "VclComboBoxText"))
    {
        OString sPattern = extractCustomProperty(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = VclBuilder::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            OString sAdjustment = extractAdjustment(rMap);
            connectNumericFormatterAdjustment(id, sAdjustment);
            OString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_WARN("vcl.layout", "making metric box for type: " << name.getStr()
                << " unit: " << sUnit.getStr()
                << " name: " << id.getStr()
                << " use a VclComboBoxNumeric instead");
            VclPtrInstance<MetricBox> xBox(pParent, nBits);
            xBox->EnableAutoSize(true);
            xBox->SetUnit(eUnit);
            xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
            if (eUnit == FUNIT_CUSTOM)
                xBox->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
            xWindow = xBox;
        }
        else if (extractEntry(rMap))
        {
            VclPtrInstance<ComboBox> xComboBox(pParent, nBits);
            xComboBox->EnableAutoSize(true);
            xWindow = xComboBox;
        }
        else
        {
            VclPtrInstance<ListBox> xListBox(pParent, nBits|WB_SIMPLEMODE);
            xListBox->EnableAutoSize(true);
            xWindow = xListBox;
        }
    }
    else if (name == "VclComboBoxNumeric")
    {
        OString sPattern = extractCustomProperty(rMap);
        OString sAdjustment = extractAdjustment(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = VclBuilder::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            connectNumericFormatterAdjustment(id, sAdjustment);
            OString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_INFO("vcl.layout", "making metric box for " << name.getStr() << " " << sUnit.getStr());
            VclPtrInstance<MetricBox> xBox(pParent, nBits);
            xBox->EnableAutoSize(true);
            xBox->SetUnit(eUnit);
            xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
            if (eUnit == FUNIT_CUSTOM)
                xBox->SetCustomUnitText(OStringToOUString(sUnit, RTL_TEXTENCODING_UTF8));
            xWindow = xBox;
        }
        else
        {
            SAL_INFO("vcl.layout", "making numeric box for " << name.getStr());
            connectNumericFormatterAdjustment(id, sAdjustment);
            VclPtrInstance<NumericBox> xBox(pParent, nBits);
            if (bDropdown)
                xBox->EnableAutoSize(true);
            xWindow = xBox;
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
        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
        OString sBorder = extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        //ListBox manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        xWindow = VclPtr<ListBox>::Create(pRealParent, nWinStyle);
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, xWindow, rMap);
    }
    else if (name == "GtkLabel")
    {
        WinBits nWinStyle = WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OString sBorder = extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        extractMnemonicWidget(id, rMap);
        if (extractSelectable(rMap))
            xWindow = VclPtr<SelectableFixedText>::Create(pParent, nWinStyle);
        else
            xWindow = VclPtr<FixedText>::Create(pParent, nWinStyle);
    }
    else if (name == "GtkImage")
    {
        extractStock(id, rMap);
        xWindow = VclPtr<FixedImage>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_SCALE);
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
        xWindow = VclPtr<FixedLine>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkScrollbar")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
        bVertical = extractOrientation(rMap);
        xWindow = VclPtr<ScrollBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkProgressBar")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
        bVertical = extractOrientation(rMap);
        xWindow = VclPtr<ProgressBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkScrolledWindow")
    {
        xWindow = VclPtr<VclScrolledWindow>::Create(pParent);
    }
    else if (name == "GtkViewport")
    {
        xWindow = VclPtr<VclViewport>::Create(pParent);
    }
    else if (name == "GtkEventBox")
    {
        xWindow = VclPtr<VclEventBox>::Create(pParent);
    }
    else if (name == "GtkEntry")
    {
        xWindow = VclPtr<Edit>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
        ensureDefaultWidthChars(rMap);
    }
    else if (name == "GtkNotebook")
    {
        xWindow = VclPtr<TabControl>::Create(pParent, WB_STDTABCONTROL|WB_3DLOOK);
    }
    else if (name == "GtkDrawingArea")
    {
        OString sBorder = extractCustomProperty(rMap);
        xWindow = VclPtr<vcl::Window>::Create(pParent, sBorder.isEmpty() ? 0 : WB_BORDER);
    }
    else if (name == "GtkTextView")
    {
        extractBuffer(id, rMap);

        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT;
        OString sBorder = extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        //VclMultiLineEdit manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        xWindow = VclPtr<VclMultiLineEdit>::Create(pRealParent, nWinStyle);
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, xWindow, rMap);
    }
    else if (name == "GtkSpinner")
    {
        xWindow = VclPtr<Throbber>::Create(pParent, WB_3DLOOK);
    }
    else if (name == "GtkScale")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aSliderAdjustmentMaps);
        bool bDrawValue = extractDrawValue(rMap);
        if (bDrawValue)
        {
            OString sValuePos = extractValuePos(rMap);
            (void)sValuePos;
        }
        bVertical = extractOrientation(rMap);

        WinBits nWinStyle = bVertical ? WB_VERT : WB_HORZ;

        xWindow = VclPtr<Slider>::Create(pParent, nWinStyle);
    }
    else if (name == "GtkToolbar")
    {
        xWindow = VclPtr<ToolBox>::Create(pParent, WB_3DLOOK | WB_TABSTOP);
    }
    else if (name == "GtkToolButton" || name == "GtkMenuToolButton" ||
             name == "GtkToggleToolButton" || name == "GtkRadioToolButton")
    {
        ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
        if (pToolBox)
        {
            OUString aCommand(OStringToOUString(extractActionName(rMap), RTL_TEXTENCODING_UTF8));

            sal_uInt16 nItemId = 0;
            ToolBoxItemBits nBits = ToolBoxItemBits::NONE;
            if (name == "GtkMenuToolButton")
                nBits |= ToolBoxItemBits::DROPDOWN;
            else if (name == "GtkToggleToolButton")
                nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::CHECKABLE;
            else if (name == "GtkRadioToolButton")
                nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::RADIOCHECK;

            if (!aCommand.isEmpty() && m_xFrame.is())
            {
                pToolBox->InsertItem(aCommand, m_xFrame, nBits, extractSizeRequest(rMap));
                nItemId = pToolBox->GetItemId(aCommand);
            }
            else
            {
                nItemId = pToolBox->GetItemCount() + 1;
                pToolBox->InsertItem(nItemId, OStringToOUString(extractLabel(rMap), RTL_TEXTENCODING_UTF8), nBits);
                pToolBox->SetItemCommand(nItemId, aCommand);
                pToolBox->SetHelpId(nItemId, m_sHelpRoot + id);
            }

            OString sTooltip(extractTooltipText(rMap));
            if (!sTooltip.isEmpty())
                pToolBox->SetQuickHelpText(nItemId, OStringToOUString(sTooltip, RTL_TEXTENCODING_UTF8));

            OString sIconName(extractIconName(rMap));
            if (!sIconName.isEmpty())
                pToolBox->SetItemImage(nItemId, FixedImage::loadThemeImage(sIconName));

            if (!extractVisible(rMap))
                pToolBox->HideItem(nItemId);

            m_pParserState->m_nLastToolbarId = nItemId;

            return nullptr; // no widget to be created
        }
    }
    else if (name == "GtkSeparatorToolItem")
    {
        ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
        if (pToolBox)
        {
            pToolBox->InsertSeparator();
            return nullptr; // no widget to be created
        }
    }
    else if (name == "GtkWindow")
    {
        WinBits nBits = extractDeferredBits(rMap);
        if (nBits & WB_DOCKABLE)
            xWindow = VclPtr<DockingWindow>::Create(pParent, nBits|WB_MOVEABLE);
        else
            xWindow = VclPtr<FloatingWindow>::Create(pParent, nBits|WB_MOVEABLE);
    }
    else
    {
#ifndef SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif
        sal_Int32 nDelim = name.indexOf('-');
        if (nDelim != -1)
        {
#ifndef DISABLE_DYNLOADING
            OUStringBuffer sModuleBuf;
            sModuleBuf.append(SAL_DLLPREFIX);
            sModuleBuf.append(OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8));
            sModuleBuf.append(SAL_DLLEXTENSION);
#endif
            OUString sFunction(OStringToOUString(OString("make") + name.copy(nDelim+1), RTL_TEXTENCODING_UTF8));
#ifndef DISABLE_DYNLOADING
            OUString sModule = sModuleBuf.makeStringAndClear();
            ModuleMap::iterator aI = m_aModuleMap.find(sModule);
            if (aI == m_aModuleMap.end())
            {
                osl::Module* pModule = new osl::Module;
#if ENABLE_MERGELIBS
                sModuleBuf.append(SAL_DLLPREFIX);
                sModuleBuf.append("mergedlo");
                sModuleBuf.append(SAL_DLLEXTENSION);
                OUString sMergedModule = sModuleBuf.makeStringAndClear();
                pModule->loadRelative(&thisModule, sMergedModule);
                if (!pModule->getFunctionSymbol(sFunction))
                {
                    pModule->loadRelative(&thisModule, sModule);
                }
#else
                pModule->loadRelative(&thisModule, sModule);
#endif
                aI = m_aModuleMap.insert(std::make_pair(sModule, std::unique_ptr<osl::Module>(pModule))).first;
            }
            customMakeWidget pFunction = reinterpret_cast<customMakeWidget>(aI->second->getFunctionSymbol(sFunction));
#else
            customMakeWidget pFunction = reinterpret_cast<customMakeWidget>(osl_getFunctionSymbol((oslModule) RTLD_DEFAULT, sFunction.pData));
#endif
            if (pFunction)
            {
                VclPtr<vcl::Window> xParent(pParent);
                pFunction(xWindow, xParent, rMap);
                if (xWindow->GetType() == WINDOW_PUSHBUTTON)
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                else if (xWindow->GetType() == WINDOW_MENUBUTTON)
                {
                    OString sMenu = extractCustomProperty(rMap);
                    if (!sMenu.isEmpty())
                        m_pParserState->m_aButtonMenuMaps.push_back(ButtonMenuMap(id, sMenu));
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                }
            }
        }
    }
    SAL_WARN_IF(!xWindow, "vcl.layout", "probably need to implement " << name.getStr() << " or add a make" << name.getStr() << " function");
    if (xWindow)
    {
        xWindow->SetHelpId(m_sHelpRoot + id);
        SAL_INFO("vcl.layout", "for " << name.getStr() <<
            ", created " << xWindow.get() << " child of " <<
            pParent << "(" << xWindow->mpWindowImpl->mpParent.get() << "/" <<
            xWindow->mpWindowImpl->mpRealParent.get() << "/" <<
            xWindow->mpWindowImpl->mpBorderWindow.get() << ") with helpid " <<
            xWindow->GetHelpId().getStr());
        m_aChildren.push_back(WinAndId(id, xWindow, bVertical));
    }
    return xWindow;
}

namespace
{
    //return true for window types which exist in vcl but are not themselves
    //represented in the .ui format, i.e. only their children exist.
    bool isConsideredGtkPseudo(vcl::Window *pWindow)
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

void VclBuilder::set_properties(vcl::Window *pWindow, const stringmap &rProps)
{
    for (stringmap::const_iterator aI = rProps.begin(), aEnd = rProps.end(); aI != aEnd; ++aI)
    {
        const OString &rKey = aI->first;
        const OString &rValue = aI->second;
        pWindow->set_property(rKey, rValue);
    }
}

VclPtr<vcl::Window> VclBuilder::insertObject(vcl::Window *pParent, const OString &rClass,
    const OString &rID, stringmap &rProps, stringmap &rPango, stringmap &rAtk)
{
    VclPtr<vcl::Window> pCurrentChild;

    if (m_pParent && !isConsideredGtkPseudo(m_pParent) && !m_sID.isEmpty() && rID.equals(m_sID))
    {
        pCurrentChild = m_pParent;

        //toplevels default to resizable and apparently you can't change them
        //afterwards, so we need to wait until now before we can truly
        //initialize the dialog.
        if (pParent && pParent->IsSystemWindow())
        {
            SystemWindow *pSysWin = static_cast<SystemWindow*>(pCurrentChild.get());
            pSysWin->doDeferredInit(extractDeferredBits(rProps));
            m_bToplevelHasDeferredInit = false;
        }
        else if (pParent && pParent->IsDockingWindow())
        {
            DockingWindow *pDockWin = static_cast<DockingWindow*>(pCurrentChild.get());
            pDockWin->doDeferredInit(extractDeferredBits(rProps));
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
        if (pParent == m_pParent.get() && m_bToplevelHasDeferredInit)
            pParent = nullptr;
        pCurrentChild = makeObject(pParent, rClass, rID, rProps);
    }

    if (pCurrentChild)
    {
        pCurrentChild->set_id(OStringToOUString(rID, RTL_TEXTENCODING_UTF8));
        if (pCurrentChild == m_pParent.get() && m_bToplevelHasDeferredProperties)
            m_aDeferredProperties = rProps;
        else
            set_properties(pCurrentChild, rProps);

        for (stringmap::iterator aI = rPango.begin(), aEnd = rPango.end(); aI != aEnd; ++aI)
        {
            const OString &rKey = aI->first;
            const OString &rValue = aI->second;
            pCurrentChild->set_font_attribute(rKey, rValue);
        }

        m_pParserState->m_aAtkInfo[VclPtr<vcl::Window>(pCurrentChild)] = rAtk;
    }

    rProps.clear();
    rPango.clear();
    rAtk.clear();

    if (!pCurrentChild)
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back().m_pWindow.get();
    return pCurrentChild;
}

void VclBuilder::reorderWithinParent(vcl::Window &rWindow, sal_uInt16 nNewPosition)
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

void VclBuilder::handleTabChild(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    OString sID;

    int nLevel = 1;
    stringmap aProperties;
    std::vector<vcl::EnumContext::Context> context;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
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
            else if (name.equals("style"))
            {
                int nPriority = 0;
                context = handleStyle(reader, nPriority);
                --nLevel;
            }
            else if (name.equals("property"))
                collectProperty(reader, sID, aProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
            break;
    }

    if (!pParent)
        return;

    TabControl *pTabControl = static_cast<TabControl*>(pParent);
    VclBuilder::stringmap::iterator aFind = aProperties.find(OString("label"));
    if (aFind != aProperties.end())
    {
        sal_uInt16 nPageId = pTabControl->GetCurPageId();
        pTabControl->SetPageText(nPageId,
            OStringToOUString(aFind->second, RTL_TEXTENCODING_UTF8));
        pTabControl->SetPageName(nPageId, sID);
        if (context.size() != 0)
        {
            TabPage* pPage = pTabControl->GetTabPage(nPageId);
            pPage->SetContext(context);
        }
    }
    else
        pTabControl->RemovePage(pTabControl->GetCurPageId());
}

//so that tabbing between controls goes in a visually sensible sequence
//we sort these into a best-tab-order sequence
bool VclBuilder::sortIntoBestTabTraversalOrder::operator()(const vcl::Window *pA, const vcl::Window *pB) const
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
    //for VclPackType::End, they are packed from the end back, but here we need
    //them in visual layout order so that tabbing works as expected)
    sal_Int32 nPackA = m_pBuilder->get_window_packing_data(pA).m_nPosition;
    sal_Int32 nPackB = m_pBuilder->get_window_packing_data(pB).m_nPosition;
    if (nPackA < nPackB)
        return ePackA == VclPackType::Start;
    if (nPackA > nPackB)
        return ePackA != VclPackType::Start;
    //sort labels of Frames before body
    if (pA->GetParent() == pB->GetParent())
    {
        const VclFrame *pFrameParent = dynamic_cast<const VclFrame*>(pA->GetParent());
        if (pFrameParent)
        {
            const vcl::Window *pLabel = pFrameParent->get_label_widget();
            int nFramePosA = (pA == pLabel) ? 0 : 1;
            int nFramePosB = (pB == pLabel) ? 0 : 1;
            return nFramePosA < nFramePosB;
        }
    }
    return false;
}

void VclBuilder::handleChild(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    vcl::Window *pCurrentChild = nullptr;

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
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                pCurrentChild = handleObject(pParent, reader).get();

                bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;

                if (bObjectInserted)
                {
                    //Internal-children default in glade to not having their visible bits set
                    //even though they are visible (generally anyway)
                    if (!sInternalChild.isEmpty())
                        pCurrentChild->Show();

                    //Select the first page if it's a notebook
                    if (pCurrentChild->GetType() == WINDOW_TABCONTROL)
                    {
                        TabControl *pTabControl = static_cast<TabControl*>(pCurrentChild);
                        pTabControl->SetCurPageId(pTabControl->GetPageId(0));

                        //To-Do add reorder capability to the TabControl
                    }
                    else
                    {
                        // We want to sort labels before contents of frames
                        // for keyboard traversal, especially if there
                        // are multiple widgets using the same mnemonic
                        if (sType.equals("label"))
                        {
                            if (VclFrame *pFrameParent = dynamic_cast<VclFrame*>(pParent))
                                pFrameParent->designate_label(pCurrentChild);
                        }
                        if (sInternalChild.startsWith("vbox") || sInternalChild.startsWith("messagedialog-vbox"))
                        {
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pParent))
                                pBoxParent->set_content_area(static_cast<VclBox*>(pCurrentChild)); // FIXME-VCLPTR
                        }
                        else if (sInternalChild.startsWith("action_area") || sInternalChild.startsWith("messagedialog-action_area"))
                        {
                            vcl::Window *pContentArea = pCurrentChild->GetParent();
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pContentArea ? pContentArea->GetParent() : nullptr))
                            {
                                pBoxParent->set_action_area(static_cast<VclButtonBox*>(pCurrentChild)); // FIXME-VCLPTR
                            }
                        }

                        //To-Do make reorder a virtual in Window, move this foo
                        //there and see above
                        std::vector<vcl::Window*> aChilds;
                        for (vcl::Window* pChild = pCurrentChild->GetWindow(GetWindowType::FirstChild); pChild;
                            pChild = pChild->GetWindow(GetWindowType::Next))
                        {
                            aChilds.push_back(pChild);
                        }

                        bool bIsButtonBox = dynamic_cast<VclButtonBox*>(pCurrentChild) != nullptr;

                        //sort child order within parent so that tabbing
                        //between controls goes in a visually sensible sequence
                        std::stable_sort(aChilds.begin(), aChilds.end(), sortIntoBestTabTraversalOrder(this));
                        reorderWithinParent(aChilds, bIsButtonBox);
                    }
                }
            }
            else if (name.equals("packing"))
            {
                handlePacking(pCurrentChild, pParent, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
            break;
    }
}

void VclBuilder::reorderWithinParent(std::vector<vcl::Window*>& rChilds, bool bIsButtonBox)
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

void VclBuilder::handleRow(xmlreader::XmlReader &reader, const OString &rID, sal_Int32 nRowIndex)
{
    int nLevel = 1;

    ListStore::row aRow;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
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
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                if (!bTranslated)
                    sValue = OString(name.begin, name.length);

                if (aRow.size() < nId+1)
                    aRow.resize(nId+1);
                aRow[nId] = sValue;
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
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

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("row"))
                handleRow(reader, rID, nRowIndex++);
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::handleAtkObject(xmlreader::XmlReader &reader, const OString &rID, vcl::Window *pWindow)
{
    assert(pWindow);

    int nLevel = 1;

    stringmap aProperties;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("property"))
                collectProperty(reader, rID, aProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
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

        if (pWindow && rKey.match("AtkObject::"))
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

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
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
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

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

        if (res == xmlreader::XmlReader::Result::End)
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
    VclPtr<PopupMenu> pCurrentMenu = VclPtr<PopupMenu>::Create();

    int nLevel = 1;

    stringmap aProperties;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
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

        if (res == xmlreader::XmlReader::Result::End)
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
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                handleMenuObject(pParent, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
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

    stringmap aProperties;
    accelmap aAccelerators;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("property"))
                collectProperty(reader, sID, aProperties);
            else if (name.equals("accelerator"))
                collectAccelerator(reader, aAccelerators);
        }

        if (res == xmlreader::XmlReader::Result::End)
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
    m_pParserState->m_aSizeGroups.push_back(SizeGroup());
    SizeGroup &rSizeGroup = m_pParserState->m_aSizeGroups.back();

    int nLevel = 1;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
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

        if (res == xmlreader::XmlReader::Result::End)
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
    vcl::KeyCode makeKeyCode(const std::pair<OString,OString> &rKey)
    {
        bool bShift = rKey.second.indexOf("GDK_SHIFT_MASK") != -1;
        bool bMod1 = rKey.second.indexOf("GDK_CONTROL_MASK") != -1;
        bool bMod2 = rKey.second.indexOf("GDK_MOD1_MASK") != -1;
        bool bMod3 = rKey.second.indexOf("GDK_MOD2_MASK") != -1;

        if (rKey.first == "Insert")
            return vcl::KeyCode(KEY_INSERT, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Delete")
            return vcl::KeyCode(KEY_DELETE, bShift, bMod1, bMod2, bMod3);

        assert (rKey.first.getLength() == 1);
        sal_Char cChar = rKey.first.toChar();

        if (cChar >= 'a' && cChar <= 'z')
            return vcl::KeyCode(KEY_A + (cChar - 'a'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= 'A' && cChar <= 'Z')
            return vcl::KeyCode(KEY_A + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= '0' && cChar <= '9')
            return vcl::KeyCode(KEY_0 + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);

        return vcl::KeyCode(cChar, bShift, bMod1, bMod2, bMod3);
    }
}

void VclBuilder::insertMenuObject(PopupMenu *pParent, const OString &rClass, const OString &rID,
    stringmap &rProps, accelmap &rAccels)
{
    sal_uInt16 nOldCount = pParent->GetItemCount();
    sal_uInt16 nNewId = nOldCount + 1;

    if (rClass == "GtkMenuItem")
    {
        OUString sLabel(OStringToOUString(convertMnemonicMarkup(extractLabel(rProps)), RTL_TEXTENCODING_UTF8));
        OUString aCommand(OStringToOUString(extractActionName(rProps), RTL_TEXTENCODING_UTF8));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::TEXT, rID);
        pParent->SetItemCommand(nNewId, aCommand);
    }
    else if (rClass == "GtkCheckMenuItem")
    {
        OUString sLabel(OStringToOUString(convertMnemonicMarkup(extractLabel(rProps)), RTL_TEXTENCODING_UTF8));
        OUString aCommand(OStringToOUString(extractActionName(rProps), RTL_TEXTENCODING_UTF8));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::CHECKABLE, rID);
        pParent->SetItemCommand(nNewId, aCommand);
    }
    else if (rClass == "GtkRadioMenuItem")
    {
        OUString sLabel(OStringToOUString(convertMnemonicMarkup(extractLabel(rProps)), RTL_TEXTENCODING_UTF8));
        OUString aCommand(OStringToOUString(extractActionName(rProps), RTL_TEXTENCODING_UTF8));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::CHECKABLE | MenuItemBits::RADIOCHECK, rID);
        pParent->SetItemCommand(nNewId, aCommand);
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

        for (accelmap::iterator aI = rAccels.begin(), aEnd = rAccels.end(); aI != aEnd; ++aI)
        {
            const OString &rSignal = aI->first;
            const auto &rValue = aI->second;

            if (rSignal == "activate")
                pParent->SetAccelKey(nNewId, makeKeyCode(rValue));
            else
                SAL_INFO("vcl.layout", "unhandled accelerator for: " << rSignal.getStr());
        }
    }

    rProps.clear();
}

/// Insert items to a ComboBox or a ListBox.
/// They have no common ancestor that would have 'InsertEntry()', so use a template.
template<typename T> bool insertItems(vcl::Window *pWindow, VclBuilder::stringmap &rMap, const std::vector<OString> &rItems)
{
    T *pContainer = dynamic_cast<T*>(pWindow);
    if (!pContainer)
        return false;

    sal_uInt16 nActiveId = extractActive(rMap);
    for (std::vector<OString>::const_iterator aI = rItems.begin(), aEnd = rItems.end(); aI != aEnd; ++aI)
        pContainer->InsertEntry(OStringToOUString(*aI, RTL_TEXTENCODING_UTF8));
    if (nActiveId < rItems.size())
        pContainer->SelectEntryPos(nActiveId);

    return true;
}

VclPtr<vcl::Window> VclBuilder::handleObject(vcl::Window *pParent, xmlreader::XmlReader &reader)
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
        return nullptr;
    }
    else if (sClass == "GtkMenu")
    {
        handleMenu(reader, sID);
        return nullptr;
    }
    else if (sClass == "GtkSizeGroup")
    {
        handleSizeGroup(reader, sID);
        return nullptr;
    }
    else if (sClass == "AtkObject")
    {
        handleAtkObject(reader, sID, pParent);
        return nullptr;
    }

    int nLevel = 1;

    stringmap aProperties, aPangoAttributes, aAtkAttributes;
    std::vector<OString> aItems;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    VclPtr<vcl::Window> pCurrentChild;
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("child"))
            {
                if (!pCurrentChild)
                {
                    pCurrentChild = insertObject(pParent, sClass, sID,
                        aProperties, aPangoAttributes, aAtkAttributes);
                }
                handleChild(pCurrentChild, reader);
            }
            else if (name.equals("items"))
                aItems = handleItems(reader, sID);
            else if (name.equals("style"))
            {
                int nPriority = 0;
                std::vector<vcl::EnumContext::Context> aContext = handleStyle(reader, nPriority);
                if (nPriority != 0)
                    dynamic_cast<vcl::IPrioritable*>(pCurrentChild.get())->SetPriority(nPriority);
                if (aContext.size() != 0)
                    dynamic_cast<vcl::IContext*>(pCurrentChild.get())->SetContext(aContext);
            }
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

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    if (sClass == "GtkAdjustment")
    {
        m_pParserState->m_aAdjustments[sID] = aProperties;
        return nullptr;
    }
    else if (sClass == "GtkTextBuffer")
    {
        m_pParserState->m_aTextBuffers[sID] = aProperties;
        return nullptr;
    }

    if (!pCurrentChild)
    {
        pCurrentChild = insertObject(pParent, sClass, sID, aProperties,
            aPangoAttributes, aAtkAttributes);
    }

    if (!aItems.empty())
    {
        // try to fill-in the items
        if (!insertItems<ComboBox>(pCurrentChild, aProperties, aItems))
            insertItems<ListBox>(pCurrentChild, aProperties, aItems);
    }

    return pCurrentChild;
}

void VclBuilder::handlePacking(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    int nLevel = 1;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("property"))
                applyPackingProperty(pCurrent, pParent, reader);
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::applyPackingProperty(vcl::Window *pCurrent,
    vcl::Window *pParent,
    xmlreader::XmlReader &reader)
{
    if (!pCurrent)
        return;

    //ToolBoxItems are not true widgets just elements
    //of the ToolBox itself
    ToolBox *pToolBoxParent = nullptr;
    if (pCurrent == pParent)
        pToolBoxParent = dynamic_cast<ToolBox*>(pParent);

    xmlreader::Span name;
    int nsId;

    if (pCurrent->GetType() == WINDOW_SCROLLWINDOW)
    {
        auto aFind = m_pParserState->m_aRedundantParentWidgets.find(VclPtr<vcl::Window>(pCurrent));
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
                xmlreader::XmlReader::Text::Raw, &name, &nsId);
            OString sValue(name.begin, name.length);

            if (sKey == "expand" || sKey == "resize")
            {
                bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                if (pToolBoxParent)
                    pToolBoxParent->SetItemExpand(m_pParserState->m_nLastToolbarId, bTrue);
                else
                    pCurrent->set_expand(bTrue);
                continue;
            }

            if (pToolBoxParent)
                continue;

            if (sKey == "fill")
            {
                bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                pCurrent->set_fill(bTrue);
            }
            else if (sKey == "pack-type")
            {
                VclPackType ePackType = (!sValue.isEmpty() && (sValue[0] == 'e' || sValue[0] == 'E')) ? VclPackType::End : VclPackType::Start;
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
            else if (sKey == "non-homogeneous")
            {
                pCurrent->set_non_homogeneous(toBool(sValue));
            }
            else if (sKey == "homogeneous")
            {
                pCurrent->set_non_homogeneous(!toBool(sValue));
            }
            else
            {
                SAL_WARN("vcl.layout", "unknown packing: " << sKey.getStr());
            }
        }
    }
}

std::vector<vcl::EnumContext::Context> VclBuilder::handleStyle(xmlreader::XmlReader &reader, int &nPriority)
{
    std::vector<vcl::EnumContext::Context> aContext;

    xmlreader::Span name;
    int nsId;

    int nLevel = 1;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("class"))
            {
                OString classStyle = getStyleClass(reader);

                if (classStyle.startsWith("context-"))
                {
                    OString sContext = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString sContext2 = OUString(sContext.getStr(), sContext.getLength(), RTL_TEXTENCODING_UTF8);
                    aContext.push_back(vcl::EnumContext::GetContextEnum(sContext2));
                }
                else if (classStyle.startsWith("priority-"))
                {
                    OString aPriority = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString aPriority2 = OUString(aPriority.getStr(), aPriority.getLength(), RTL_TEXTENCODING_UTF8);
                    nPriority = aPriority2.toInt32();
                }
                else
                {
                    SAL_WARN("vcl.layout", "unknown class: " << classStyle.getStr());
                }
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aContext;
}

OString VclBuilder::getStyleClass(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;
    OString aRet;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            aRet = OString (name.begin, name.length);
        }
    }

    return aRet;
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

    reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
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

    reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    OString sID = OString(name.begin, name.length);
    sal_Int32 nDelim = sID.indexOf(':');
    if (nDelim != -1)
        sID = sID.copy(0, nDelim);
    set_response(sID, sResponse.toInt32());
}

void VclBuilder::collectAccelerator(xmlreader::XmlReader &reader, accelmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    OString sProperty;
    OString sValue;
    OString sModifiers;

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
        else if (name.equals("modifiers"))
        {
            name = reader.getAttributeValue(false);
            sModifiers = OString(name.begin, name.length);
        }
    }

    if (!sProperty.isEmpty() && !sValue.isEmpty())
    {
        rMap[sProperty] = std::make_pair(sValue, sModifiers);
    }
}

vcl::Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? nullptr : m_aChildren[0].m_pWindow.get();
}

vcl::Window *VclBuilder::get_by_name(const OString& sID)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pWindow;
    }

    return nullptr;
}

PopupMenu *VclBuilder::get_menu(const OString& sID)
{
    for (std::vector<MenuAndId>::iterator aI = m_aMenus.begin(),
         aEnd = m_aMenus.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pMenu;
    }

    return nullptr;
}

short VclBuilder::get_response(const vcl::Window *pWindow) const
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

void VclBuilder::set_response(const OString& sID, short nResponse)
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

void VclBuilder::delete_by_name(const OString& sID)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
        {
            aI->m_pWindow.disposeAndClear();
            m_aChildren.erase(aI);
            break;
        }
    }
}

void VclBuilder::delete_by_window(vcl::Window *pWindow)
{
    drop_ownership(pWindow);
    pWindow->disposeOnce();
}

void VclBuilder::drop_ownership(const vcl::Window *pWindow)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
        {
            m_aChildren.erase(aI);
            break;
        }
    }
}

OString VclBuilder::get_by_window(const vcl::Window *pWindow) const
{
    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            return aI->m_sID;
    }

    return OString();
}

VclBuilder::PackingData VclBuilder::get_window_packing_data(const vcl::Window *pWindow) const
{
    //We've stored the return of new Control, some of these get
    //border windows placed around them which are what you get
    //from GetChild, so scoot up a level if necessary to get the
    //window whose position value we have
    const vcl::Window *pPropHolder = pWindow->mpWindowImpl->mpClientWindow ?
        pWindow->mpWindowImpl->mpClientWindow : pWindow;

    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pPropHolder)
            return aI->m_aPackingData;
    }

    return PackingData();
}

void VclBuilder::set_window_packing_position(const vcl::Window *pWindow, sal_Int32 nPosition)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            aI->m_aPackingData.m_nPosition = nPosition;
    }
}

const VclBuilder::ListStore *VclBuilder::get_model_by_name(const OString& sID) const
{
    std::map<OString, ListStore>::const_iterator aI = m_pParserState->m_aModels.find(sID);
    if (aI != m_pParserState->m_aModels.end())
        return &(aI->second);
    return nullptr;
}

const VclBuilder::TextBuffer *VclBuilder::get_buffer_by_name(const OString& sID) const
{
    std::map<OString, TextBuffer>::const_iterator aI = m_pParserState->m_aTextBuffers.find(sID);
    if (aI != m_pParserState->m_aTextBuffers.end())
        return &(aI->second);
    return nullptr;
}

const VclBuilder::Adjustment *VclBuilder::get_adjustment_by_name(const OString& sID) const
{
    std::map<OString, Adjustment>::const_iterator aI = m_pParserState->m_aAdjustments.find(sID);
    if (aI != m_pParserState->m_aAdjustments.end())
        return &(aI->second);
    return nullptr;
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
            rTarget.SetEntryData(nEntry, reinterpret_cast<void*>(nValue));
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
            tools::Time aUpper(rValue.toInt32());
            rTarget.SetMax(aUpper);
            rTarget.SetLast(aUpper);
        }
        else if (rKey == "lower")
        {
            tools::Time aLower(rValue.toInt32());
            rTarget.SetMin(aLower);
            rTarget.SetFirst(aLower);
        }
        else if (rKey == "value")
        {
            tools::Time aValue(rValue.toInt32());
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

void VclBuilder::mungeAdjustment(Slider& rTarget, const Adjustment& rAdjustment)
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

VclBuilder::ParserState::ParserState()
    : m_nLastToolbarId(0)
{}

VclBuilder::MenuAndId::MenuAndId(const OString &rId, PopupMenu *pMenu)
            : m_sID(rId)
            , m_pMenu(pMenu)
{};

VclBuilder::MenuAndId::~MenuAndId() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
