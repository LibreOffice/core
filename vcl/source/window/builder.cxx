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
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>

VclBuilder::VclBuilder(Window *pParent, rtl::OUString sUIDir, rtl::OUString sUIFile, rtl::OString sID)
    : m_sID(sID)
    , m_sHelpRoot(rtl::OUStringToOString(sUIFile, RTL_TEXTENCODING_UTF8))
    , m_pParent(pParent)
    , m_pParserState(new ParserState)
{
    sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
    if (nIdx != -1)
        m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
    m_sHelpRoot = m_sHelpRoot + rtl::OString('/');

    rtl::OUString sUri = sUIDir + sUIFile;

    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
    for (int i = aLocale.Country.isEmpty() ? 1 : 0; i < 2; ++i)
    {
        rtl::OUStringBuffer aTransBuf;
        sal_Int32 nLastSlash = sUri.lastIndexOf('/');
        aTransBuf.append(sUri.copy(0, nLastSlash)).append("/res/").append(aLocale.Language);
        switch (i)
        {
            case 0:
                aTransBuf.append('-').append(aLocale.Country);
                break;
            default:
                break;
        }
        aTransBuf.append(sUri.copy(nLastSlash));

        rtl::OUString sTransUri = aTransBuf.makeStringAndClear();
        try
        {
            xmlreader::XmlReader reader(sTransUri);
            handleTranslations(reader);
        }
        catch (const ::com::sun::star::uno::Exception &)
        {
        }
    }

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
        ListStore *pStore = get_model_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pStore, "vcl", "missing elements of combobox/liststore");
        if (pTarget && pStore)
            mungemodel(*pTarget, *pStore);
    }
    for (std::vector<ModelAndId>::iterator aI = m_pParserState->m_aModels.begin(),
         aEnd = m_pParserState->m_aModels.end(); aI != aEnd; ++aI)
    {
        delete aI->m_pModel;
    }

    //Set SpinButton adjustments when everything has been imported
    for (std::vector<SpinButtonAdjustmentMap>::iterator aI = m_pParserState->m_aAdjustmentMaps.begin(),
         aEnd = m_pParserState->m_aAdjustmentMaps.end(); aI != aEnd; ++aI)
    {
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get<Window>(aI->m_sID));
        Adjustment *pAdjustment = get_adjustment_by_name(aI->m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeadjustment(*pTarget, *pAdjustment);
    }

    //drop maps, etc. now
    delete m_pParserState;
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
    rtl::OString sType;

    rtl::OString sID, sProperty;

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
                        sID = rtl::OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                            sID = sID.copy(nDelim);
                    }
                    else if (name.equals(RTL_CONSTASCII_STRINGPARAM("i")))
                    {
                        name = reader.getAttributeValue(false);
                        sProperty = rtl::OString(name.begin, name.length);
                    }
                }
            }
        }

        if (res == xmlreader::XmlReader::RESULT_TEXT && !sID.isEmpty())
        {
            rtl::OString sTranslation(name.begin, name.length);
            m_pParserState->m_aTranslations[sID][sProperty] = sTranslation;
        }

        if (res == xmlreader::XmlReader::RESULT_END)
            sID = rtl::OString();

        if (res == xmlreader::XmlReader::RESULT_DONE)
            break;
    }
}

namespace
{
    rtl::OString extractPattern(VclBuilder::stringmap &rMap)
    {
        rtl::OString sPattern;
        VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("pattern")));
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
        VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("resizable")));
        if (aFind != rMap.end())
        {
            bResizable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bResizable;
    }

    bool extractOrientation(VclBuilder::stringmap &rMap)
    {
        bool bVertical = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("orientation")));
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("vertical"));
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractInconsistent(VclBuilder::stringmap &rMap)
    {
        bool bInconsistent = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("inconsistent")));
        if (aFind != rMap.end())
        {
            bInconsistent = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bInconsistent;
    }

    Window * extractStockAndBuildButton(Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CENTER|WB_VCENTER|WB_3DLOOK;

        bool bIsStock = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("use-stock")));
        if (aFind != rMap.end())
        {
            bIsStock = toBool(aFind->second);
            rMap.erase(aFind);
        }

        Window *pWindow = NULL;

        if (bIsStock)
        {
            rtl::OString sType;
            aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("label")));
            if (aFind != rMap.end())
            {
                sType = aFind->second;
                rMap.erase(aFind);
            }

            if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-ok")))
                pWindow = new OKButton(pParent, nBits);
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-cancel")))
                pWindow = new CancelButton(pParent, nBits);
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-help")))
                pWindow = new HelpButton(pParent, nBits);
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-media-next")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetSymbol(SYMBOL_NEXT);
                pWindow = pBtn;
            }
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-media-previous")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetSymbol(SYMBOL_PREV);
                pWindow = pBtn;
            }
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-close")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetText(VclResId(SV_BUTTONTEXT_CLOSE).toString());
                pWindow = pBtn;
            }
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-revert-to-saved")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetText(VclResId(SV_BUTTONTEXT_RESET).toString());
                pWindow = pBtn;
            }
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-add")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetText(VclResId(SV_BUTTONTEXT_ADD).toString());
                pWindow = pBtn;
            }
            else if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("gtk-delete")))
            {
                PushButton *pBtn = new PushButton(pParent, nBits);
                pBtn->SetText(VclResId(SV_BUTTONTEXT_DELETE).toString());
                pWindow = pBtn;
            }
            else
            {
                SAL_WARN("vcl.layout", "unknown stock type: " << sType.getStr());
            }
        }

        if (!pWindow)
            pWindow = new PushButton(pParent, nBits);
        return pWindow;
    }

    FieldUnit detectMetricUnit(rtl::OString sUnit)
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
        rtl::OString sWidthChars(RTL_CONSTASCII_STRINGPARAM("width-chars"));
        VclBuilder::stringmap::iterator aFind = rMap.find(sWidthChars);
        if (aFind == rMap.end())
            rMap[sWidthChars] = "25";
    }
}

bool VclBuilder::extractGroup(const rtl::OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("group")));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aGroupMaps.push_back(RadioButtonGroupMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractAdjustment(const rtl::OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("adjustment")));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aAdjustmentMaps.push_back(SpinButtonAdjustmentMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

bool VclBuilder::extractModel(const rtl::OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("model")));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aModelMaps.push_back(ComboBoxModelMap(id, aFind->second));
        rMap.erase(aFind);
        return true;
    }
    return false;
}

extern "C" { static void SAL_CALL thisModule() {} }

Window *VclBuilder::makeObject(Window *pParent, const rtl::OString &name, const rtl::OString &id, stringmap &rMap)
{
    bool bIsPlaceHolder = name.isEmpty();

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
        pTabControl->InsertPage(nNewPageId, rtl::OUString());
        pTabControl->SetCurPageId(nNewPageId);

        if (!bIsPlaceHolder)
        {
            TabPage* pPage = new TabPage(pTabControl);
            pPage->Show();

            //Make up a name for it
            rtl::OString sTabPageId = get_by_window(pParent) +
                rtl::OString("-page") +
                rtl::OString::valueOf(static_cast<sal_Int32>(nNewPageCount));
            m_aChildren.push_back(WinAndId(sTabPageId, pPage));
            pPage->SetHelpId(m_sHelpRoot + sTabPageId);

            //And give the page one container as a child to make it a layout enabled
            //tab page
            VclBin* pContainer = new VclBin(pPage);
            pContainer->Show();
            m_aChildren.push_back(WinAndId(rtl::OString(), pContainer));
            pContainer->SetHelpId(m_sHelpRoot + sTabPageId + rtl::OString("-bin"));
            pParent = pContainer;

            pTabControl->SetTabPage(nNewPageId, pPage);
        }
    }

    if (bIsPlaceHolder || name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkTreeSelection")))
        return NULL;

    Window *pWindow = NULL;
    if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkDialog")))
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        pWindow = new Dialog(pParent, nBits);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkBox")))
    {
        if (extractOrientation(rMap))
            pWindow = new VclVBox(pParent);
        else
            pWindow = new VclHBox(pParent);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkButtonBox")))
    {
        if (extractOrientation(rMap))
            pWindow = new VclVButtonBox(pParent);
        else
            pWindow = new VclHButtonBox(pParent);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkGrid")))
        pWindow = new VclGrid(pParent);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkFrame")))
        pWindow = new VclFrame(pParent);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkExpander")))
        pWindow = new VclExpander(pParent);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkAlignment")))
        pWindow = new VclAlignment(pParent);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkButton")))
        pWindow = extractStockAndBuildButton(pParent, rMap);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkRadioButton")))
    {
        extractGroup(id, rMap);
        pWindow = new RadioButton(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkCheckButton")))
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
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkSpinButton")))
    {
        extractAdjustment(id, rMap);
        rtl::OString sPattern = extractPattern(rMap);
        rtl::OString sUnit = sPattern;

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
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkComboBox")))
    {
        extractModel(id, rMap);
        ListBox *pListBox = new ListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
        pListBox->SetBestDropDownLineCount();
        pWindow = pListBox;
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkComboBoxText")))
    {
        extractModel(id, rMap);
        ComboBox* pComboBox = new ComboBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
        pComboBox->SetBestDropDownLineCount();
        pWindow = pComboBox;
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkTreeView")))
    {
        extractModel(id, rMap);
        pWindow = new ListBox(pParent, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkLabel")))
        pWindow = new FixedText(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkImage")))
        pWindow = new FixedImage(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkSeparator")))
    {
        if (extractOrientation(rMap))
            pWindow = new FixedLine(pParent, WB_VERT);
        else
            pWindow = new FixedLine(pParent, WB_HORZ);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkEntry")))
    {
        pWindow = new Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
        ensureDefaultWidthChars(rMap);
    }
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkNotebook")))
        pWindow = new TabControl(pParent, WB_STDTABCONTROL|WB_3DLOOK);
    else if (name.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkDrawingArea")))
        pWindow = new Window(pParent);
    else
    {
        sal_Int32 nDelim = name.indexOf(':');
        if (nDelim != -1)
        {
            rtl::OUStringBuffer sModule;
#ifdef SAL_DLLPREFIX
            sModule.append(SAL_DLLPREFIX);
#endif
            sModule.append(rtl::OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8));
#ifdef SAL_DLLEXTENSION
            sModule.append(SAL_DLLEXTENSION);
#endif
            rtl::OUString sFunction(rtl::OStringToOUString(rtl::OString("make") + name.copy(nDelim+1), RTL_TEXTENCODING_UTF8));
            osl::Module aModule;
            aModule.loadRelative(&thisModule, sModule.makeStringAndClear());
            customMakeWidget pFunction = (customMakeWidget)aModule.getFunctionSymbol(sFunction);
            if (pFunction)
                pWindow = (*pFunction)(pParent, rMap);
        }
    }
    SAL_WARN_IF(!pWindow, "vcl.layout", "problably need to implement " << name.getStr() << "or add a make" << name.getStr() << " function");
    if (pWindow)
    {
        pWindow->SetHelpId(m_sHelpRoot + id);
        SAL_INFO("vcl.layout", "for " << name.getStr() <<
            ", created << " << pWindow << " child of " <<
            pParent << "(" << pWindow->mpWindowImpl->mpParent << "/" <<
            pWindow->mpWindowImpl->mpRealParent << "/" <<
            pWindow->mpWindowImpl->mpBorderWindow << ") with helpid " <<
            pWindow->GetHelpId().getStr());
        m_aChildren.push_back(WinAndId(id, pWindow));
    }
    return pWindow;
}

namespace
{
    //return true for window types which exist in vcl but are not themselves
    //represented in the .ui format, i.e. only their children exist.
    bool isConsideredPseudo(Window *pWindow)
    {
        return pWindow->GetType() == WINDOW_TABPAGE;
    }
}

Window *VclBuilder::insertObject(Window *pParent, const rtl::OString &rClass, const rtl::OString &rID, stringmap &rMap)
{
    Window *pCurrentChild = NULL;

    if (m_pParent && !isConsideredPseudo(m_pParent) && !m_sID.isEmpty() && rID.equals(m_sID))
    {
        pCurrentChild = m_pParent;
        //toplevels default to resizable
        if (pCurrentChild->IsDialog())
        {
            Dialog *pDialog = (Dialog*)pCurrentChild;
            pDialog->doDeferredInit(extractResizable(rMap));
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
        pCurrentChild = makeObject(pParent, rClass, rID, rMap);
    }

    if (pCurrentChild)
    {
        for (stringmap::iterator aI = rMap.begin(), aEnd = rMap.end(); aI != aEnd; ++aI)
        {
            const rtl::OString &rKey = aI->first;
            const rtl::OString &rValue = aI->second;
            pCurrentChild->set_property(rKey, rValue);
        }
    }

    rMap.clear();

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
    rtl::OString sID;

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
                        sID = rtl::OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                        {
                            rtl::OString sPattern = sID.copy(nDelim+1);
                            aProperties[rtl::OString("pattern")] = sPattern;
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
    VclBuilder::stringmap::iterator aFind = aProperties.find(rtl::OString(RTL_CONSTASCII_STRINGPARAM("label")));
    if (aFind != aProperties.end())
    {
        pTabControl->SetPageText(pTabControl->GetCurPageId(), rtl::OStringToOUString(aFind->second, RTL_TEXTENCODING_UTF8));

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

namespace
{
    bool sortByGridPositions(Window *pA, Window *pB)
    {
        sal_Int32 nTopA = pA->get_grid_top_attach();
        sal_Int32 nTopB = pB->get_grid_top_attach();
        if (nTopA < nTopB)
            return true;
        if (nTopA > nTopB)
            return false;
        sal_Int32 nLeftA = pA->get_grid_left_attach();
        sal_Int32 nLeftB = pB->get_grid_left_attach();
        return nLeftA < nLeftB;
    }
}

void VclBuilder::handleChild(Window *pParent, xmlreader::XmlReader &reader)
{
    Window *pCurrentChild = NULL;

    xmlreader::Span name;
    int nsId;
    rtl::OString sType;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            name = reader.getAttributeValue(false);
            sType = rtl::OString(name.begin, name.length);
        }
    }

    if (sType.equalsL(RTL_CONSTASCII_STRINGPARAM("tab")))
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

                        //sort child order within parent list by grid position
                        //so that tabbing between controls goes in a visually sensible sequence
                        std::stable_sort(aChilds.begin(), aChilds.end(), sortByGridPositions);
                        for (size_t i = 0; i < aChilds.size(); ++i)
                            reorderWithinParent(*aChilds[i], i);

                        //honour box positions if there is any
                        for (size_t i = 0; i < aChilds.size(); ++i)
                        {
                            sal_Int32 nPosition = get_window_packing_position(aChilds[i]);
                            if (nPosition == -1)
                                continue;
                            reorderWithinParent(*aChilds[i], nPosition);
                        }
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

void VclBuilder::handleAdjustment(const rtl::OString &rID, stringmap &rProperties)
{
    m_pParserState->m_aAdjustments.push_back(AdjustmentAndId(rID, rProperties));
}

void VclBuilder::handleRow(xmlreader::XmlReader &reader, const rtl::OString &rID, sal_Int32 nRowIndex)
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
                rtl::OString sProperty, sValue;
                sal_uInt32 nId = 0;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals(RTL_CONSTASCII_STRINGPARAM("id")))
                    {
                        name = reader.getAttributeValue(false);
                        nId = rtl::OString(name.begin, name.length).toInt32();
                    }
                    else if (nId == 0 && name.equals(RTL_CONSTASCII_STRINGPARAM("translatable")) && reader.getAttributeValue(false).equals(RTL_CONSTASCII_STRINGPARAM("yes")))
                    {
                        sValue = getTranslation(rID, rtl::OString::valueOf(nRowIndex));
                        bTranslated = !sValue.isEmpty();
                    }
                }

                reader.nextItem(
                    xmlreader::XmlReader::TEXT_RAW, &name, &nsId);

                if (!bTranslated)
                    sValue = rtl::OString(name.begin, name.length);

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

    if (!aRow.empty())
        m_pParserState->m_aModels.back().m_pModel->m_aEntries.push_back(aRow);
}

void VclBuilder::handleListStore(xmlreader::XmlReader &reader, const rtl::OString &rID)
{
    m_pParserState->m_aModels.push_back(ModelAndId(rID, new ListStore));

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
    rtl::OString sClass;
    rtl::OString sID;
    rtl::OString sPattern;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("class")))
        {
            name = reader.getAttributeValue(false);
            sClass = rtl::OString(name.begin, name.length);
        }
        else if (name.equals(RTL_CONSTASCII_STRINGPARAM("id")))
        {
            name = reader.getAttributeValue(false);
            sID = rtl::OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sPattern = sID.copy(nDelim+1);
                sID = sID.copy(0, nDelim);
            }
        }
    }

    if (sClass.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkListStore")))
    {
        handleListStore(reader, sID);
        return NULL;
    }

    int nLevel = 1;

    stringmap aProperties;

    if (!sPattern.isEmpty())
        aProperties[rtl::OString("pattern")] = sPattern;

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
                    pCurrentChild = insertObject(pParent, sClass, sID, aProperties);
                handleChild(pCurrentChild, reader);
            }
            else
            {
                ++nLevel;
                if (name.equals(RTL_CONSTASCII_STRINGPARAM("property")))
                    collectProperty(reader, sID, aProperties);
            }
        }

        if (res == xmlreader::XmlReader::RESULT_END)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    if (sClass.equalsL(RTL_CONSTASCII_STRINGPARAM("GtkAdjustment")))
    {
        handleAdjustment(sID, aProperties);
        return NULL;
    }

    if (!pCurrentChild)
        pCurrentChild = insertObject(pParent, sClass, sID, aProperties);

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

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            name = reader.getAttributeValue(false);
            rtl::OString sKey(name.begin, name.length);
            sKey = sKey.replace('_', '-');
            reader.nextItem(
                xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
            rtl::OString sValue(name.begin, name.length);

            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("expand")))
            {
                bool bTrue = (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1');
                pCurrent->set_expand(bTrue);
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("fill")))
            {
                bool bTrue = (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1');
                pCurrent->set_fill(bTrue);
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("pack-type")))
            {
                VclPackType ePackType = (sValue[0] == 'e' || sValue[0] == 'e') ? VCL_PACK_END : VCL_PACK_START;
                pCurrent->set_pack_type(ePackType);
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("left-attach")))
            {
                pCurrent->set_grid_left_attach(sValue.toInt32());
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("top-attach")))
            {
                pCurrent->set_grid_top_attach(sValue.toInt32());
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("width")))
            {
                pCurrent->set_grid_width(sValue.toInt32());
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("height")))
            {
                pCurrent->set_grid_height(sValue.toInt32());
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("padding")))
            {
                pCurrent->set_padding(sValue.toInt32());
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("position")))
            {
                set_window_packing_position(pCurrent, sValue.toInt32());
            }
            else
            {
                SAL_WARN("vcl.layout", "unknown packing: " << sKey.getStr());
            }
        }
    }
}

rtl::OString VclBuilder::getTranslation(const rtl::OString &rID, const rtl::OString &rProperty) const
{
    Translations::const_iterator aWidgetFind = m_pParserState->m_aTranslations.find(rID);
    if (aWidgetFind != m_pParserState->m_aTranslations.end())
    {
        const WidgetTranslations &rWidgetTranslations = aWidgetFind->second;
        WidgetTranslations::const_iterator aPropertyFind = rWidgetTranslations.find(rProperty);
        if (aPropertyFind != rWidgetTranslations.end())
            return aPropertyFind->second;
    }
    return rtl::OString();
}

void VclBuilder::collectProperty(xmlreader::XmlReader &reader, const rtl::OString &rID, stringmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    rtl::OString sProperty;
    rtl::OString sValue;

    bool bTranslated = false;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            name = reader.getAttributeValue(false);
            sProperty = rtl::OString(name.begin, name.length);
        }
        else if (name.equals(RTL_CONSTASCII_STRINGPARAM("translatable")) && reader.getAttributeValue(false).equals(RTL_CONSTASCII_STRINGPARAM("yes")))
        {
            sValue = getTranslation(rID, sProperty);
            bTranslated = !sValue.isEmpty();
        }

    }

    reader.nextItem(xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
    if (!bTranslated)
        sValue = rtl::OString(name.begin, name.length);

    if (!sProperty.isEmpty())
    {
        sProperty = sProperty.replace('_', '-');
        //replace '_' with '-' except for property values that
        //refer to widget ids themselves. TO-DO, drop conversion
        //and just use foo_bar properties throughout
        if (sProperty.equalsL(RTL_CONSTASCII_STRINGPARAM("group")))
            rMap[sProperty] = sValue;
        else
            rMap[sProperty] = sValue.replace('_', '-');
    }
}

Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? NULL : m_aChildren[0].m_pWindow;
}

Window *VclBuilder::get_by_name(rtl::OString sID)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pWindow;
    }

    return NULL;
}

rtl::OString VclBuilder::get_by_window(const Window *pWindow) const
{
    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            return aI->m_sID;
    }

    return rtl::OString();
}

sal_Int32 VclBuilder::get_window_packing_position(const Window *pWindow) const
{
    for (std::vector<WinAndId>::const_iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            return aI->m_nPosition;
    }

    return -1;
}

void VclBuilder::set_window_packing_position(const Window *pWindow, sal_Int32 nPosition)
{
    for (std::vector<WinAndId>::iterator aI = m_aChildren.begin(),
         aEnd = m_aChildren.end(); aI != aEnd; ++aI)
    {
        if (aI->m_pWindow == pWindow)
            aI->m_nPosition = nPosition;
    }
}

VclBuilder::ListStore *VclBuilder::get_model_by_name(rtl::OString sID)
{
    for (std::vector<ModelAndId>::iterator aI = m_pParserState->m_aModels.begin(),
         aEnd = m_pParserState->m_aModels.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return aI->m_pModel;
    }

    return NULL;
}

VclBuilder::Adjustment *VclBuilder::get_adjustment_by_name(rtl::OString sID)
{
    for (std::vector<AdjustmentAndId>::iterator aI = m_pParserState->m_aAdjustments.begin(),
         aEnd = m_pParserState->m_aAdjustments.end(); aI != aEnd; ++aI)
    {
        if (aI->m_sID.equals(sID))
            return &(aI->m_aAdjustment);
    }

    return NULL;
}

void VclBuilder::swapGuts(Window &rOrig, Window &rReplacement)
{
    sal_uInt16 nPosition = getPositionWithinParent(rOrig);

    rReplacement.take_properties(rOrig);

    reorderWithinParent(rReplacement, nPosition);

    assert(nPosition == getPositionWithinParent(rReplacement));
}

bool VclBuilder::replace(rtl::OString sID, Window &rReplacement)
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

void VclBuilder::mungemodel(ListBox &rTarget, ListStore &rStore)
{
    for (std::vector<ListStore::row>::iterator aI = rStore.m_aEntries.begin(), aEnd = rStore.m_aEntries.end();
        aI != aEnd; ++aI)
    {
        const ListStore::row &rRow = *aI;
        sal_uInt16 nEntry = rTarget.InsertEntry(rtl::OStringToOUString(rRow[0], RTL_TEXTENCODING_UTF8));
        if (rRow.size() > 1)
        {
            sal_IntPtr nValue = rRow[1].toInt32();
            rTarget.SetEntryData(nEntry, (void*)nValue);
        }
    }
    if (!rStore.m_aEntries.empty())
        rTarget.SelectEntryPos(0);
}

void VclBuilder::mungeadjustment(NumericFormatter &rTarget, Adjustment &rAdjustment)
{
    int nMul = rtl_math_pow10Exp(1, rTarget.GetDecimalDigits());

    for (stringmap::iterator aI = rAdjustment.begin(), aEnd = rAdjustment.end(); aI != aEnd; ++aI)
    {
        const rtl::OString &rKey = aI->first;
        const rtl::OString &rValue = aI->second;

        if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("upper")))
        {
            sal_Int64 nUpper = rValue.toDouble() * nMul;
            rTarget.SetMax(nUpper);
            rTarget.SetLast(nUpper);
        }
        else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("lower")))
        {
            sal_Int64 nLower = rValue.toDouble() * nMul;
            rTarget.SetMin(nLower);
            rTarget.SetFirst(nLower);
        }
        else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("value")))
        {
            sal_Int64 nValue = rValue.toDouble() * nMul;
            rTarget.SetValue(nValue);
        }
        else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("step-increment")))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
