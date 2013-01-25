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
#include <vcl/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/vclmedit.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>

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

void VclBuilder::loadTranslations(const com::sun::star::lang::Locale &rLocale, const OUString& rUri)
{
    for (int i = rLocale.Country.isEmpty() ? 1 : 0; i < 2; ++i)
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
        aTransBuf.append("/res/").append(rLocale.Language);
        switch (i)
        {
            case 0:
                aTransBuf.append('-').append(rLocale.Country);
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

VclBuilder::VclBuilder(Window *pParent, OUString sUIDir, OUString sUIFile, OString sID)
    : m_sID(sID)
    , m_sHelpRoot(OUStringToOString(sUIFile, RTL_TEXTENCODING_UTF8))
    , m_sProductName(OUStringToOString(utl::ConfigManager::getProductName(), RTL_TEXTENCODING_UTF8))
    , m_pParent(pParent)
    , m_pParserState(new ParserState)
{
    m_bToplevelHasDeferredInit = (pParent && pParent->IsDialog()) ? ((Dialog*)pParent)->isDeferredInit() : false;

    sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
    if (nIdx != -1)
        m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
    m_sHelpRoot = m_sHelpRoot + OString('/');

    OUString sUri = sUIDir + sUIFile;

    com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    bool bEN_US = aLocale.Language == "en" && aLocale.Country == "US" && aLocale.Variant.isEmpty();
    if (!bEN_US)
        loadTranslations(aLocale, sUri);

    xmlreader::XmlReader reader(sUri);

    handleChild(pParent, reader);

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
            mungeModel(*pTarget, *pStore);
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
    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aSpinAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aSpinAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get<Window>(aI->m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeSpinAdjustment(*pTarget, *pAdjustment);
    }

    //Set ScrollBar adjustments when everything has been imported
    for (std::vector<WidgetAdjustmentMap>::iterator aI = m_pParserState->m_aScrollAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aScrollAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        ScrollBar *pTarget = get<ScrollBar>(aI->m_sID);
        const Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scrollbar/adjustment");
        if (pTarget && pAdjustment)
            mungeScrollAdjustment(*pTarget, *pAdjustment);
    }

    //Set button images when everything has been imported
    std::set<OString> aImagesToBeRemoved;
    for (std::vector<ButtonImageWidgetMap>::iterator aI = m_pParserState->m_aButtonImageWidgetMaps.begin(),
         aEnd = m_pParserState->m_aButtonImageWidgetMaps.end(); aI != aEnd; ++aI)
    {
        PushButton *pTarget = get<PushButton>(aI->m_sID);
        FixedImage *pImage = get<FixedImage>(aI->m_sValue);
        aImagesToBeRemoved.insert(aI->m_sValue);
        const OString &rImage = m_pParserState->m_aStockMap[aI->m_sValue];
        SymbolType eType = mapStockToSymbol(rImage);
        SAL_WARN_IF(!pTarget || !pImage || eType == SYMBOL_NOSYMBOL,
            "vcl", "missing elements of button/image/stock");
        if (!pTarget || eType == SYMBOL_NOSYMBOL)
            continue;

        //to-do, situation where image isn't a stock image
        if (pTarget && eType != SYMBOL_NOSYMBOL)
        {
            pTarget->SetSymbol(eType);
            if (eType == SYMBOL_IMAGE)
                pTarget->SetModeImage(Bitmap(VclResId(mapStockToImageResource(rImage))));
        }
    }

    //There may be duplicate use of an Image, so we used a set to collect and
    //now we can remove them from the tree after their final munge
    for (std::set<OString>::iterator aI = aImagesToBeRemoved.begin(),
        aEnd = aImagesToBeRemoved.end(); aI != aEnd; ++aI)
    {
        delete_by_name(*aI);
    }

    //Remove ScrollWindow parent widgets whose children in vcl implement scrolling
    //internally.
    for (std::map<Window*, Window*>::iterator aI = m_pParserState->m_aRedundantParentWidgets.begin(),
        aEnd = m_pParserState->m_aRedundantParentWidgets.end(); aI != aEnd; ++aI)
    {
        delete_by_window(aI->first);
    }

    //drop maps, etc. that we don't need again
    delete m_pParserState;

    SAL_WARN_IF(!m_sID.isEmpty() && !get_by_name(m_sID), "vcl.layout",
        "Requested top level widget \"" << m_sID.getStr() <<
        "\" not found in " << sUIFile);
}

VclBuilder::~VclBuilder()
{
    for (std::vector<WinAndId>::reverse_iterator aI = m_aChildren.rbegin(),
         aEnd = m_aChildren.rend(); aI != aEnd; ++aI)
    {
        if (aI->m_bOwned)
            delete aI->m_pWindow;
    }
}

void VclBuilder::handleTranslations(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;
    OString sType;

    OString sID, sProperty;

    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_RAW, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("e")))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals(RTL_CONSTASCII_STRINGPARAM("g")))
                    {
                        name = reader.getAttributeValue(false);
                        sID = OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                            sID = sID.copy(nDelim);
                    }
                    else if (name.equals(RTL_CONSTASCII_STRINGPARAM("i")))
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

namespace
{
    OString extractPattern(VclBuilder::stringmap &rMap)
    {
        OString sPattern;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("pattern"));
        if (aFind != rMap.end())
        {
            sPattern = aFind->second;
            rMap.erase(aFind);
        }
        return sPattern;
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
        SAL_WARN("vcl.layout", "unknown stock type: " << rType.getStr());
        return OUString();
    }

    Window * extractStockAndBuildButton(Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CENTER|WB_VCENTER|WB_3DLOOK;

        bool bIsStock = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("use-stock"));
        if (aFind != rMap.end())
        {
            bIsStock = toBool(aFind->second);
            rMap.erase(aFind);
        }

        Window *pWindow = NULL;

        if (bIsStock)
        {
            OString sType;
            aFind = rMap.find(OString("label"));
            if (aFind != rMap.end())
            {
                sType = aFind->second;
                rMap.erase(aFind);
            }

            if (sType == "gtk-ok")
                pWindow = new OKButton(pParent, nBits);
            else if (sType == "gtk-cancel")
                pWindow = new CancelButton(pParent, nBits);
            else if (sType == "gtk-help")
                pWindow = new HelpButton(pParent, nBits);
            else if (sType == "gtk-media-next")
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetSymbol(SYMBOL_NEXT);
                pWindow = pBtn;
            }
            else if (sType == "gtk-media-previous")
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetSymbol(SYMBOL_PREV);
                pWindow = pBtn;
            }
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

        return eUnit;
    }

    void ensureDefaultWidthChars(VclBuilder::stringmap &rMap)
    {
        OString sWidthChars("width-chars");
        VclBuilder::stringmap::iterator aFind = rMap.find(sWidthChars);
        if (aFind == rMap.end())
            rMap[sWidthChars] = "25";
    }
}

bool VclBuilder::extractGroup(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("group"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aGroupMaps.push_back(RadioButtonGroupMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractSpinAdjustment(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aSpinAdjustmentMaps.push_back(WidgetAdjustmentMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractScrollAdjustment(const OString &id, stringmap &rMap)
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

bool VclBuilder::extractModel(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("model"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aModelMaps.push_back(ComboBoxModelMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
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
        m_pParserState->m_aStockMap[id] = aFind->second;
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractImage(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("image"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aButtonImageWidgetMaps.push_back(ButtonImageWidgetMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }
#endif

Window *VclBuilder::makeObject(Window *pParent, const OString &name, const OString &id, stringmap &rMap)
{
    bool bIsPlaceHolder = name.isEmpty();
    bool bVertical = false;

    if (pParent && pParent->GetType() == WINDOW_TABCONTROL)
    {
        //We have to add a page

        //make default pageid == -position. Partitioning the
        //id space into negative numbers for auto-generated
        //ids and positive numbers for the handleTabChild
        //derived ids
        TabControl *pTabControl = static_cast<TabControl*>(pParent);
        sal_uInt16 nNewPageCount = pTabControl->GetPageCount()+1;
        sal_uInt16 nNewPageId = -nNewPageCount;
        pTabControl->InsertPage(nNewPageId, OUString());
        pTabControl->SetCurPageId(nNewPageId);

        if (!bIsPlaceHolder)
        {
            TabPage* pPage = new TabPage(pTabControl);
            pPage->Show();

            //Make up a name for it
            OString sTabPageId = get_by_window(pParent) +
                OString("-page") +
                OString::valueOf(static_cast<sal_Int32>(nNewPageCount));
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

    Window *pWindow = NULL;
    if (name == "GtkDialog")
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        pWindow = new Dialog(pParent, nBits);
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
        pWindow = new VclExpander(pParent);
    else if (name == "GtkAlignment")
        pWindow = new VclAlignment(pParent);
    else if (name == "GtkButton")
    {
        extractImage(id, rMap);
        pWindow = extractStockAndBuildButton(pParent, rMap);
    }
    else if (name == "GtkRadioButton")
    {
        extractGroup(id, rMap);
        pWindow = new RadioButton(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    }
    else if (name == "GtkCheckButton")
    {
        //maybe always import as TriStateBox and enable/disable tristate
        bool bIsTriState = extractInconsistent(rMap);
        CheckBox *pCheckBox = bIsTriState ?
            new TriStateBox(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK) :
            new CheckBox(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
        if (bIsTriState)
            pCheckBox->SetState(STATE_DONTKNOW);
        pWindow = pCheckBox;
    }
    else if (name == "GtkSpinButton")
    {
        extractSpinAdjustment(id, rMap);
        OString sPattern = extractPattern(rMap);
        OString sUnit = sPattern;

        for (sal_Int32 i = 0; i < sPattern.getLength(); ++i)
        {
            if (sPattern[i] != '.' && sPattern[i] != ',' && sPattern[i] != '0')
            {
                sUnit = sPattern.copy(i);
                break;
            }
        }

        FieldUnit eUnit = detectMetricUnit(sUnit);

        WinBits nBits = WB_RIGHT|WB_BORDER|WB_3DLOOK;
        if (!id.endsWith("-nospin"))
            nBits |= WB_SPIN;

        if (sPattern.isEmpty())
        {
            SAL_INFO("vcl.layout", "making numeric field for " << name.getStr() << " " << sUnit.getStr());
            pWindow = new NumericField(pParent, nBits);
        }
        else
        {
            SAL_INFO("vcl.layout", "making metric field for " << name.getStr() << " " << sUnit.getStr());
            MetricField *pField = new MetricField(pParent, nBits);
            pField->SetUnit(eUnit);
            pWindow = pField;
        }
    }
    else if (name == "GtkLinkButton")
        pWindow = new FixedHyperlink(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
    else if (name == "GtkComboBox")
    {
        extractModel(id, rMap);
        ListBox *pListBox = new ListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
        pListBox->EnableAutoSize(true);
        pWindow = pListBox;
    }
    else if (name == "GtkComboBoxText")
    {
        extractModel(id, rMap);
        if (extractEntry(rMap))
        {
            ComboBox* pComboBox = new ComboBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
            pComboBox->EnableAutoSize(true);
            pWindow = pComboBox;
        }
        else
        {
            MenuButton *pMenuButton = new MenuButton(pParent, WB_LEFT|WB_VCENTER|WB_3DLOOK);
            pMenuButton->mpOwnMenu = new PopupMenu; //this now belongs to the menubutton
            pMenuButton->SetPopupMenu(pMenuButton->mpOwnMenu);
            pMenuButton->SetShowDisplaySelectedItem(true);
            pWindow = pMenuButton;
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
        pWindow = new ListBox(pParent, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    }
    else if (name == "GtkLabel")
        pWindow = new FixedText(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    else if (name == "GtkImage")
    {
        extractStock(id, rMap);
        pWindow = new FixedImage(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
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
    else if (name == "GtkEntry")
    {
        pWindow = new Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
        ensureDefaultWidthChars(rMap);
    }
    else if (name == "GtkNotebook")
        pWindow = new TabControl(pParent, WB_STDTABCONTROL|WB_3DLOOK);
    else if (name == "GtkDrawingArea")
        pWindow = new Window(pParent);
    else if (name == "GtkTextView")
    {
        extractBuffer(id, rMap);

        WinBits nWinStyle = WB_LEFT | WB_BORDER;
        //VclMultiLineEdit manage their own scrolling,
        //so if it appears as a child of a scrolling window
        //shoehorn that scrolling settings to this
        //widget and remove the parent
        Window *pScrollParent = NULL;
        if (pParent && pParent->GetType() == WINDOW_SCROLLWINDOW)
        {
            WinBits nScrollBits = pParent->GetStyle();
            nScrollBits &= (WB_AUTOHSCROLL|WB_HSCROLL|WB_AUTOVSCROLL|WB_VSCROLL);
            nWinStyle |= nScrollBits;

            pScrollParent = pParent;
            pParent = pParent->GetParent();
        }

        pWindow = new VclMultiLineEdit(pParent, nWinStyle);

        if (pScrollParent)
        {
            sal_Int32 nWidthReq = pScrollParent->get_width_request();
            rMap[OString("width-request")] = OString::valueOf(nWidthReq);
            sal_Int32 nHeightReq = pScrollParent->get_height_request();
            rMap[OString("height-request")] = OString::valueOf(nHeightReq);

            m_pParserState->m_aRedundantParentWidgets[pScrollParent] = pWindow;
        }
    }
    else
    {
        sal_Int32 nDelim = name.indexOf(':');
        if (nDelim != -1)
        {
#ifndef DISABLE_DYNLOADING
            OUStringBuffer sModule;
#ifdef SAL_DLLPREFIX
            sModule.append(SAL_DLLPREFIX);
#endif
            sModule.append(OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8));
#ifdef SAL_DLLEXTENSION
            sModule.append(SAL_DLLEXTENSION);
#endif
#endif
            OUString sFunction(OStringToOUString(OString("make") + name.copy(nDelim+1), RTL_TEXTENCODING_UTF8));
#ifndef DISABLE_DYNLOADING
            osl::Module aModule;
            aModule.loadRelative(&thisModule, sModule.makeStringAndClear());
            customMakeWidget pFunction = (customMakeWidget)aModule.getFunctionSymbol(sFunction);
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

Window *VclBuilder::insertObject(Window *pParent, const OString &rClass,
    const OString &rID, stringmap &rProps, stringmap &rPango)
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
    }
    else
    {
        //if we're being inserting under a toplevel dialog whose init is
        //deferred due to waiting to encounter it in this .ui, and it hasn't
        //been seen yet, then make unattached widgets parent-less toplevels
        if (pParent == m_pParent && m_bToplevelHasDeferredInit)
            pParent = NULL;
        pCurrentChild = makeObject(pParent, rClass, rID, rProps);
    }

    if (pCurrentChild)
    {
        for (stringmap::iterator aI = rProps.begin(), aEnd = rProps.end(); aI != aEnd; ++aI)
        {
            const OString &rKey = aI->first;
            const OString &rValue = aI->second;
            pCurrentChild->set_property(rKey, rValue);
        }

        for (stringmap::iterator aI = rPango.begin(), aEnd = rPango.end(); aI != aEnd; ++aI)
        {
            const OString &rKey = aI->first;
            const OString &rValue = aI->second;
            pCurrentChild->set_font_attribute(rKey, rValue);
        }
    }

    rProps.clear();
    rPango.clear();

    if (!pCurrentChild)
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back().m_pWindow;
    return pCurrentChild;
}

sal_uInt16 VclBuilder::getPositionWithinParent(Window &rWindow)
{
    if (rWindow.mpWindowImpl->mpParent != rWindow.mpWindowImpl->mpRealParent)
    {
        assert(rWindow.mpWindowImpl->mpBorderWindow ==
            rWindow.mpWindowImpl->mpParent);
        assert(rWindow.mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent ==
            rWindow.mpWindowImpl->mpRealParent);
        return getPositionWithinParent(*rWindow.mpWindowImpl->mpBorderWindow);
    }

    assert(rWindow.GetParent() == rWindow.mpWindowImpl->mpRealParent);

    sal_uInt16 nPosition = 0;
    Window* pChild = rWindow.GetParent()->mpWindowImpl->mpFirstChild;
    while (pChild)
    {
        if (pChild == &rWindow)
            break;
        pChild = pChild->mpWindowImpl->mpNext;
        ++nPosition;
    }
    return nPosition;
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
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("object")))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals(RTL_CONSTASCII_STRINGPARAM("id")))
                    {
                        name = reader.getAttributeValue(false);
                        sID = OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                        {
                            OString sPattern = sID.copy(nDelim+1);
                            aProperties[OString("pattern")] = sPattern;
                            sID = sID.copy(0, nDelim);
                        }
                    }
                }
            }
            else if (name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
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
        pTabControl->SetPageText(pTabControl->GetCurPageId(), OStringToOUString(aFind->second, RTL_TEXTENCODING_UTF8));

        sal_Int32 nID = 0;
        //To make it easier to retro fit pre-builder dialog code we take the
        //notebook child id (falling back to notebook label id) and if its a
        //positive number use that as the page id so existing code can find the
        //right tabpage by id
        TabPage *pPage = pTabControl->GetTabPage(pTabControl->GetCurPageId());
        if (pPage)
        {
            VclBin *pContainer = static_cast<VclBin*>(pPage->GetWindow(WINDOW_FIRSTCHILD));
            Window *pChild = pContainer->get_child();
            nID = pChild ? get_by_window(pChild).toInt32() : 0;
        }
        if (nID == 0)
            nID = sID.toInt32();
        if (nID > 0)
            pTabControl->ReassignPageId(pTabControl->GetCurPageId(), nID);
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
    //honour relative box positions with pack group
    return m_pBuilder->get_window_packing_data(pA).m_nPosition < m_pBuilder->get_window_packing_data(pB).m_nPosition;
}

void VclBuilder::handleChild(Window *pParent, xmlreader::XmlReader &reader)
{
    Window *pCurrentChild = NULL;
    bool bIsInternalChild = false;

    xmlreader::Span name;
    int nsId;
    OString sType;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            name = reader.getAttributeValue(false);
            sType = OString(name.begin, name.length);
        }
        else if (name.equals(RTL_CONSTASCII_STRINGPARAM("internal-child")))
        {
            bIsInternalChild = true;
        }
    }

    if (sType == "tab")
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
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("object")) || name.equals(RTL_CONSTASCII_STRINGPARAM("placeholder")))
            {
                pCurrentChild = handleObject(pParent, reader);

                bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;

                if (bObjectInserted)
                {
                    //Internal-children default in glade to not having their visible bits set
                    //even though they are visible (generally anyway)
                    if (bIsInternalChild)
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
                        //To-Do make reorder a virtual in Window, move this foo
                        //there and see above
                        std::vector<Window*> aChilds;
                        for (Window* pChild = pCurrentChild->GetWindow(WINDOW_FIRSTCHILD); pChild;
                            pChild = pChild->GetWindow(WINDOW_NEXT))
                        {
                            aChilds.push_back(pChild);
                        }

                        //sort child order within parent so that tabbing
                        //between controls goes in a visually sensible sequence
                        std::stable_sort(aChilds.begin(), aChilds.end(), sortIntoBestTabTraversalOrder(this));
                        for (size_t i = 0; i < aChilds.size(); ++i)
                            reorderWithinParent(*aChilds[i], i);
                    }
                }
            }
            else if (name.equals(RTL_CONSTASCII_STRINGPARAM("packing")))
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

void VclBuilder::collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span.equals(RTL_CONSTASCII_STRINGPARAM("value")))
        {
            span = reader.getAttributeValue(false);
            sValue = OString(span.begin, span.length);
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
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("col")))
            {
                bool bTranslated = false;
                OString sProperty, sValue;
                sal_uInt32 nId = 0;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals(RTL_CONSTASCII_STRINGPARAM("id")))
                    {
                        name = reader.getAttributeValue(false);
                        nId = OString(name.begin, name.length).toInt32();
                    }
                    else if (nId == 0 && name.equals(RTL_CONSTASCII_STRINGPARAM("translatable")) && reader.getAttributeValue(false).equals(RTL_CONSTASCII_STRINGPARAM("yes")))
                    {
                        sValue = getTranslation(rID, OString::valueOf(nRowIndex));
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
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("row")))
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

Window* VclBuilder::handleObject(Window *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OString sPattern;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("class")))
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name.equals(RTL_CONSTASCII_STRINGPARAM("id")))
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sPattern = sID.copy(nDelim+1);
                sID = sID.copy(0, nDelim);
            }
        }
    }

    if (sClass == "GtkListStore")
    {
        handleListStore(reader, sID);
        return NULL;
    }

    int nLevel = 1;

    stringmap aProperties, aPangoAttributes;

    if (!sPattern.isEmpty())
        aProperties[OString("pattern")] = sPattern;

    Window *pCurrentChild = NULL;
    while(1)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;

        if (res == xmlreader::XmlReader::RESULT_BEGIN)
        {
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("child")))
            {
                if (!pCurrentChild)
                {
                    pCurrentChild = insertObject(pParent, sClass, sID,
                        aProperties, aPangoAttributes);
                }
                handleChild(pCurrentChild, reader);
            }
            else
            {
                ++nLevel;
                if (name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
                    collectProperty(reader, sID, aProperties);
                else if (name.equals(RTL_CONSTASCII_STRINGPARAM("attribute")))
                    collectPangoAttribute(reader, aPangoAttributes);
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
        pCurrentChild = insertObject(pParent, sClass, sID, aProperties, aPangoAttributes);

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
            if (name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
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
        pCurrent = m_pParserState->m_aRedundantParentWidgets[pCurrent];
        assert(pCurrent);
    }

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
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
                VclPackType ePackType = (sValue[0] == 'e' || sValue[0] == 'e') ? VCL_PACK_END : VCL_PACK_START;
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
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name.equals(RTL_CONSTASCII_STRINGPARAM("translatable")) && reader.getAttributeValue(false).equals(RTL_CONSTASCII_STRINGPARAM("yes")))
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
        rMap[sProperty] = sValue.replaceAll("%PRODUCTNAME", m_sProductName);
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

void VclBuilder::swapGuts(Window &rOrig, Window &rReplacement)
{
    sal_uInt16 nPosition = getPositionWithinParent(rOrig);

    rReplacement.take_properties(rOrig);

    reorderWithinParent(rReplacement, nPosition);

    assert(nPosition == getPositionWithinParent(rReplacement));
}

bool VclBuilder::replace(OString sID, Window &rReplacement)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
        {
            Window *pOrig = aI->m_pWindow;
            swapGuts(*pOrig, rReplacement);
            delete pOrig;

            aI->m_pWindow = &rReplacement;
            aI->m_bOwned = false;
            return true;
        }
    }
    SAL_WARN("vcl.layout", "no sign of :" << sID.getStr());
    return false;
}

void VclBuilder::mungeModel(ListBox &rTarget, const ListStore &rStore)
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
    if (!rStore.m_aEntries.empty())
        rTarget.SelectEntryPos(0);
}

void VclBuilder::mungeSpinAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment)
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

void VclBuilder::mungeScrollAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment)
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
