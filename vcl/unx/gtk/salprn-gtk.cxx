/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unx/gtk/gtkprintwrapper.hxx"

#include "unx/gtk/gtkdata.hxx"
#include "unx/gtk/gtkframe.hxx"
#include "unx/gtk/gtkinst.hxx"
#include "unx/gtk/gtkprn.hxx"

#include "vcl/configsettings.hxx"
#include "vcl/help.hxx"
#include "vcl/print.hxx"
#include "vcl/svapp.hxx"
#include "vcl/window.hxx"

#include <gtk/gtk.h>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/view/PrintableState.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <officecfg/Office/Common.hxx>

#include <rtl/ustring.hxx>

#include <unotools/streamwrap.hxx>

#include <cstring>
#include <map>

namespace frame = com::sun::star::frame;
namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace uno = com::sun::star::uno;
namespace document = com::sun::star::document;
namespace sheet = com::sun::star::sheet;
namespace io = com::sun::star::io;
namespace view = com::sun::star::view;

using vcl::unx::GtkPrintWrapper;

using uno::UNO_QUERY;

class GtkPrintDialog
{
public:
    explicit GtkPrintDialog(vcl::PrinterController& io_rController);
    bool run();
    GtkPrinter* getPrinter() const
    {
        return m_xWrapper->print_unix_dialog_get_selected_printer(GTK_PRINT_UNIX_DIALOG(m_pDialog));
    }
    GtkPrintSettings* getSettings() const
    {
        return m_xWrapper->print_unix_dialog_get_settings(GTK_PRINT_UNIX_DIALOG(m_pDialog));
    }
    void updateControllerPrintRange();
#if 0
    void ExportAsPDF(const OUString &rFileURL, GtkPrintSettings* pSettings) const;
#endif
    ~GtkPrintDialog();

    static void UIOption_CheckHdl(GtkWidget* i_pWidget, GtkPrintDialog* io_pThis)
    {
        io_pThis->impl_UIOption_CheckHdl(i_pWidget);
    }
    static void UIOption_RadioHdl(GtkWidget* i_pWidget, GtkPrintDialog* io_pThis)
    {
        io_pThis->impl_UIOption_RadioHdl(i_pWidget);
    }
    static void UIOption_SelectHdl(GtkWidget* i_pWidget, GtkPrintDialog* io_pThis)
    {
        io_pThis->impl_UIOption_SelectHdl(i_pWidget);
    }

private:
    beans::PropertyValue* impl_queryPropertyValue(GtkWidget* i_pWidget) const;
    void impl_checkOptionalControlDependencies();

    void impl_UIOption_CheckHdl(GtkWidget* i_pWidget);
    void impl_UIOption_RadioHdl(GtkWidget* i_pWidget);
    void impl_UIOption_SelectHdl(GtkWidget* i_pWidget);

    void impl_initDialog();
    void impl_initCustomTab();
    void impl_initPrintContent(uno::Sequence<sal_Bool> const& i_rDisabled);

    void impl_readFromSettings();
    void impl_storeToSettings() const;

private:
    GtkWidget* m_pDialog;
    vcl::PrinterController& m_rController;
    std::map<GtkWidget*, OUString> m_aControlToPropertyMap;
    std::map<GtkWidget*, sal_Int32> m_aControlToNumValMap;
    std::shared_ptr<GtkPrintWrapper> m_xWrapper;
};

struct GtkSalPrinter_Impl
{
    OString m_sSpoolFile;
    OUString m_sJobName;
    GtkPrinter* m_pPrinter;
    GtkPrintSettings* m_pSettings;

    GtkSalPrinter_Impl();
    ~GtkSalPrinter_Impl();
};

GtkSalPrinter_Impl::GtkSalPrinter_Impl()
    : m_pPrinter(nullptr)
    , m_pSettings(nullptr)
{
}

GtkSalPrinter_Impl::~GtkSalPrinter_Impl()
{
    if (m_pPrinter)
    {
        g_object_unref(G_OBJECT(m_pPrinter));
        m_pPrinter = nullptr;
    }
    if (m_pSettings)
    {
        g_object_unref(G_OBJECT(m_pSettings));
        m_pSettings = nullptr;
    }
}

namespace
{

static GtkInstance const&
lcl_getGtkSalInstance()
{
    // we _know_ this is GtkInstance
    return *static_cast<GtkInstance*>(GetGtkSalData()->m_pInstance);
}

static bool
lcl_useSystemPrintDialog()
{
    return officecfg::Office::Common::Misc::UseSystemPrintDialog::get()
        && officecfg::Office::Common::Misc::ExperimentalMode::get()
        && lcl_getGtkSalInstance().getPrintWrapper()->supportsPrinting();
}

}

GtkSalPrinter::GtkSalPrinter(SalInfoPrinter* const i_pInfoPrinter)
    : PspSalPrinter(i_pInfoPrinter)
{
}

bool
GtkSalPrinter::impl_doJob(
        const OUString* const i_pFileName,
        const OUString& i_rJobName,
        const OUString& i_rAppName,
        ImplJobSetup* const io_pSetupData,
        const int i_nCopies,
        const bool i_bCollate,
        vcl::PrinterController& io_rController)
{
    io_rController.setJobState(view::PrintableState_JOB_STARTED);
    io_rController.jobStarted();
    const bool bJobStarted(
            PspSalPrinter::StartJob(i_pFileName, i_rJobName, i_rAppName,
                i_nCopies, i_bCollate, true, io_pSetupData))
        ;

    if (bJobStarted)
    {
        io_rController.createProgressDialog();
        const int nPages(io_rController.getFilteredPageCount());
        for (int nPage(0); nPage != nPages; ++nPage)
        {
            if (nPage == nPages - 1)
                io_rController.setLastPage(true);
            io_rController.printFilteredPage(nPage);
        }
        io_rController.setJobState(view::PrintableState_JOB_COMPLETED);
    }

    return bJobStarted;
}

bool
GtkSalPrinter::StartJob(
        const OUString* const i_pFileName,
        const OUString& i_rJobName,
        const OUString& i_rAppName,
        ImplJobSetup* io_pSetupData,
        vcl::PrinterController& io_rController)
{
    if (!lcl_useSystemPrintDialog())
        return PspSalPrinter::StartJob(i_pFileName, i_rJobName, i_rAppName, io_pSetupData, io_rController);

    assert(!m_xImpl);

    m_xImpl.reset(new GtkSalPrinter_Impl());
    m_xImpl->m_sJobName = i_rJobName;

    OString sFileName;
    if (i_pFileName)
        sFileName = OUStringToOString(*i_pFileName, osl_getThreadTextEncoding());

    GtkPrintDialog aDialog(io_rController);
    if (!aDialog.run())
    {
        io_rController.abortJob();
        return false;
    }
    aDialog.updateControllerPrintRange();
    m_xImpl->m_pPrinter = aDialog.getPrinter();
    m_xImpl->m_pSettings = aDialog.getSettings();

#if 0
    if (const gchar *uri = gtk_print_settings_get(m_xImpl->m_pSettings, GTK_PRINT_SETTINGS_OUTPUT_URI))
    {
        const gchar *pStr = gtk_print_settings_get(m_xImpl->m_pSettings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT);
        if (pStr && !strcmp(pStr, "pdf"))
        {
            aDialog.ExportAsPDF(OUString((const sal_Char *)uri, strlen((const sal_Char *)uri), RTL_TEXTENCODING_UTF8), m_xImpl->m_pSettings);
            bDoJob = false;
        }
    }

    if (!bDoJob)
        return false;
#endif
    int nCopies = 1;
    bool bCollate = false;

    //To-Do proper name, watch for encodings
    sFileName = OString("/tmp/hacking.ps");
    m_xImpl->m_sSpoolFile = sFileName;

    OUString aFileName = OStringToOUString(sFileName, osl_getThreadTextEncoding());

    //To-Do, swap ps/pdf for gtk_printer_accepts_ps()/gtk_printer_accepts_pdf() ?

    return impl_doJob(&aFileName, i_rJobName, i_rAppName, io_pSetupData, nCopies, bCollate, io_rController);
}

bool
GtkSalPrinter::EndJob()
{
    bool bRet = PspSalPrinter::EndJob();

    if (!lcl_useSystemPrintDialog())
        return bRet;

    assert(m_xImpl);

    if (!bRet || m_xImpl->m_sSpoolFile.isEmpty())
        return bRet;

    std::shared_ptr<GtkPrintWrapper> const xWrapper(lcl_getGtkSalInstance().getPrintWrapper());

    GtkPageSetup* pPageSetup = xWrapper->page_setup_new();
#if 0
    //todo
    gtk_page_setup_set_orientation(pPageSetup,);
    gtk_page_setup_set_paper_size(pPageSetup,);
    gtk_page_setup_set_top_margin(pPageSetup,);
    gtk_page_setup_set_bottom_margin(pPageSetup,);
    gtk_page_setup_set_left_margin(pPageSetup,);
    gtk_page_setup_set_right_margin(pPageSetup,);
#endif

    GtkPrintJob* const pJob = xWrapper->print_job_new(
        OUStringToOString(m_xImpl->m_sJobName, RTL_TEXTENCODING_UTF8).getStr(),
        m_xImpl->m_pPrinter, m_xImpl->m_pSettings, pPageSetup);

    GError* error = nullptr;
    bRet = xWrapper->print_job_set_source_file(pJob, m_xImpl->m_sSpoolFile.getStr(), &error);
    if (bRet)
        xWrapper->print_job_send(pJob, nullptr, nullptr, nullptr);
    else
    {
        //To-Do, do something with this
        fprintf(stderr, "error was %s\n", error->message);
        g_error_free(error);
    }

    g_object_unref(pPageSetup);
    m_xImpl.reset();

    //To-Do, remove temp spool file

    return bRet;
}

namespace
{

static void
lcl_setHelpText(
        GtkWidget* const io_pWidget,
        const uno::Sequence<OUString>& i_rHelpTexts,
        const sal_Int32 i_nIndex)
{
    if (i_nIndex >= 0 && i_nIndex < i_rHelpTexts.getLength())
        gtk_widget_set_tooltip_text(io_pWidget,
            OUStringToOString(i_rHelpTexts.getConstArray()[i_nIndex], RTL_TEXTENCODING_UTF8).getStr());
}

static GtkWidget*
lcl_makeFrame(
        GtkWidget* const i_pChild,
        const OUString &i_rText,
        const uno::Sequence<OUString> &i_rHelpTexts,
        sal_Int32* const io_pCurHelpText)
{
    GtkWidget* const pLabel = gtk_label_new(nullptr);
    lcl_setHelpText(pLabel, i_rHelpTexts, !io_pCurHelpText ? 0 : (*io_pCurHelpText)++);
    gtk_misc_set_alignment(GTK_MISC(pLabel), 0.0, 0.5);

    {
        gchar* const pText = g_markup_printf_escaped("<b>%s</b>",
            OUStringToOString(i_rText, RTL_TEXTENCODING_UTF8).getStr());
        gtk_label_set_markup_with_mnemonic(GTK_LABEL(pLabel), pText);
        g_free(pText);
    }

    GtkWidget* const pFrame = gtk_vbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(pFrame), pLabel, FALSE, FALSE, 0);

    GtkWidget* const pAlignment = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(pAlignment), 0, 0, 12, 0);
    gtk_box_pack_start(GTK_BOX(pFrame), pAlignment, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(pAlignment), i_pChild);
    return pFrame;
}

static void
lcl_extractHelpTextsOrIds(
        const beans::PropertyValue& rEntry,
        uno::Sequence<OUString>& rHelpStrings)
{
    if (!(rEntry.Value >>= rHelpStrings))
    {
        OUString aHelpString;
        if ((rEntry.Value >>= aHelpString))
        {
            rHelpStrings.realloc(1);
            *rHelpStrings.getArray() = aHelpString;
        }
    }
}

static GtkWidget*
lcl_combo_box_text_new()
{
#if GTK_CHECK_VERSION(3,0,0)
    return gtk_combo_box_text_new();
#else
    return gtk_combo_box_new_text();
#endif
}

static void
lcl_combo_box_text_append(GtkWidget* const pWidget, gchar const* const pText)
{
#if GTK_CHECK_VERSION(3,0,0)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pWidget), pText);
#else
    gtk_combo_box_append_text(GTK_COMBO_BOX(pWidget), pText);
#endif
}

}

GtkPrintDialog::GtkPrintDialog(vcl::PrinterController& io_rController)
    : m_rController(io_rController)
    , m_xWrapper(lcl_getGtkSalInstance().getPrintWrapper())
{
    assert(m_xWrapper->supportsPrinting());
    impl_initDialog();
    impl_initCustomTab();
    impl_readFromSettings();
}

void
GtkPrintDialog::impl_initDialog()
{
    //To-Do, like fpicker, set UI language
    m_pDialog = m_xWrapper->print_unix_dialog_new(nullptr, nullptr);

    vcl::Window* const pTopWindow(Application::GetActiveTopWindow());
    if (pTopWindow)
    {
        GtkSalFrame* const pFrame(dynamic_cast<GtkSalFrame*>(pTopWindow->ImplGetFrame()));
        if (pFrame)
        {
            GtkWindow* const pParent(GTK_WINDOW(pFrame->getWindow()));
            if (pParent)
                gtk_window_set_transient_for(GTK_WINDOW(m_pDialog), pParent);
        }
    }

    m_xWrapper->print_unix_dialog_set_manual_capabilities(GTK_PRINT_UNIX_DIALOG(m_pDialog),
        GtkPrintCapabilities(GTK_PRINT_CAPABILITY_COPIES
            | GTK_PRINT_CAPABILITY_COLLATE
            | GTK_PRINT_CAPABILITY_REVERSE
            | GTK_PRINT_CAPABILITY_GENERATE_PS
            | GTK_PRINT_CAPABILITY_NUMBER_UP
            | GTK_PRINT_CAPABILITY_NUMBER_UP_LAYOUT
       ));
}

void
GtkPrintDialog::impl_initCustomTab()
{
    typedef std::map<OUString, GtkWidget*> DependencyMap_t;
    typedef std::vector<std::pair<GtkWidget*, OUString> > CustomTabs_t;

    const uno::Sequence<beans::PropertyValue>& rOptions(m_rController.getUIOptions());
    DependencyMap_t aPropertyToDependencyRowMap;
    CustomTabs_t aCustomTabs;
    GtkWidget* pCurParent = nullptr;
    GtkWidget* pCurTabPage = nullptr;
    GtkWidget* pCurSubGroup = nullptr;
    GtkWidget* pStandardPrintRangeContainer = nullptr;
    bool bIgnoreSubgroup = false;
    for (int i = 0; i != rOptions.getLength(); i++)
    {
        uno::Sequence<beans::PropertyValue> aOptProp;
        rOptions[i].Value >>= aOptProp;

        OUString aCtrlType;
        OUString aText;
        OUString aPropertyName;
        uno::Sequence<OUString> aChoices;
        uno::Sequence<sal_Bool> aChoicesDisabled;
        uno::Sequence<OUString> aHelpTexts;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        sal_Int32 nCurHelpText = 0;
        OUString aDependsOnName;
        sal_Int32 nDependsOnValue = 0;
        bool bUseDependencyRow = false;
        bool bIgnore = false;
        GtkWidget* pGroup = nullptr;
        bool bGtkInternal = false;

        //Fix fdo#69381
        //Next options if this one is empty
        if ( aOptProp.getLength() == 0)
            continue;

        for (int n = 0; n != aOptProp.getLength(); n++)
        {
            const beans::PropertyValue& rEntry(aOptProp[ n ]);
            if ( rEntry.Name == "Text" )
            {
                OUString aValue;
                rEntry.Value >>= aValue;
                aText = aValue.replace('~', '_');
            }
            else if ( rEntry.Name == "ControlType" )
                rEntry.Value >>= aCtrlType;
            else if ( rEntry.Name == "Choices" )
                rEntry.Value >>= aChoices;
            else if ( rEntry.Name == "ChoicesDisabled" )
                rEntry.Value >>= aChoicesDisabled;
            else if ( rEntry.Name == "Property" )
            {
                beans::PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
            }
            else if ( rEntry.Name == "DependsOnName" )
                rEntry.Value >>= aDependsOnName;
            else if ( rEntry.Name == "DependsOnEntry" )
                rEntry.Value >>= nDependsOnValue;
            else if ( rEntry.Name == "AttachToDependency" )
                rEntry.Value >>= bUseDependencyRow;
            else if ( rEntry.Name == "MinValue" )
                rEntry.Value >>= nMinValue;
            else if ( rEntry.Name == "MaxValue" )
                rEntry.Value >>= nMaxValue;
            else if ( rEntry.Name == "HelpId" )
            {
                uno::Sequence<OUString> aHelpIds;
                lcl_extractHelpTextsOrIds(rEntry, aHelpIds);
                Help* const pHelp = Application::GetHelp();
                if (pHelp)
                {
                    const int nLen = aHelpIds.getLength();
                    aHelpTexts.realloc(nLen);
                    for (int j = 0; j != nLen; ++j)
                        aHelpTexts[j] = pHelp->GetHelpText(aHelpIds[j], nullptr);
                }
                else // fallback
                    aHelpTexts = aHelpIds;
            }
            else if ( rEntry.Name == "HelpText" )
                lcl_extractHelpTextsOrIds(rEntry, aHelpTexts);
            else if ( rEntry.Name == "InternalUIOnly" )
                rEntry.Value >>= bIgnore;
            else if ( rEntry.Name == "Enabled" )
            {
                // Ignore this. We use UIControlOptions::isUIOptionEnabled
                // to check whether a control should be enabled.
            }
            else if ( rEntry.Name == "GroupingHint" )
            {
                // Ignore this. We cannot add/modify controls to/on existing
                // tabs of the Gtk print dialog.
            }
            else
            {
                SAL_INFO("vcl.gtk", "unhandled UI option entry: " << rEntry.Name);
            }
        }

        if ( aPropertyName == "PrintContent" )
            bGtkInternal = true;

        if (aCtrlType == "Group" || !pCurParent)
        {
            pCurTabPage = gtk_vbox_new(FALSE, 12);
            gtk_container_set_border_width(GTK_CONTAINER(pCurTabPage), 6);
            lcl_setHelpText(pCurTabPage, aHelpTexts, 0);

            pCurParent = pCurTabPage;
            aCustomTabs.push_back(std::make_pair(pCurTabPage, aText));
        }
        else if (aCtrlType == "Subgroup" && (pCurParent /*|| bOnJobPageValue*/))
        {
            bIgnoreSubgroup = bIgnore;
            if (bIgnore)
                continue;
            pCurParent = gtk_vbox_new(FALSE, 12);
            gtk_container_set_border_width(GTK_CONTAINER(pCurParent), 0);

            pCurSubGroup = lcl_makeFrame(pCurParent, aText, aHelpTexts, nullptr);
            gtk_box_pack_start(GTK_BOX(pCurTabPage), pCurSubGroup, FALSE, FALSE, 0);
        }
        // special case: we need to map these to controls of the gtk print dialog
        else if (bGtkInternal)
        {
            if ( aPropertyName == "PrintContent" )
            {
                // What to print? And, more importantly, is there a selection?
                impl_initPrintContent(aChoicesDisabled);
            }
        }
        else if (bIgnoreSubgroup || bIgnore)
            continue;
        else
        {
            // change handlers for all the controls set up in this block
            // should be set _after_ the control has been made (in)active,
            // because:
            // 1. value of the property is _known_--we are using it to
            //    _set_ the control, right?--no need to change it back .-)
            // 2. it may cause warning because the widget may not
            //    have been placed in m_aControlToPropertyMap yet

            GtkWidget* pWidget = nullptr;
            beans::PropertyValue* pVal = nullptr;
            if (aCtrlType == "Bool" && pCurParent)
            {
                pWidget = gtk_check_button_new_with_mnemonic(
                    OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
                lcl_setHelpText(pWidget, aHelpTexts, 0);
                m_aControlToPropertyMap[pWidget] = aPropertyName;

                bool bVal = false;
                pVal = m_rController.getValue(aPropertyName);
                if (pVal)
                    pVal->Value >>= bVal;
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pWidget), bVal);
                gtk_widget_set_sensitive(pWidget,
                    m_rController.isUIOptionEnabled(aPropertyName) && pVal != nullptr);
                g_signal_connect(pWidget, "toggled", G_CALLBACK(GtkPrintDialog::UIOption_CheckHdl), this);
            }
            else if (aCtrlType == "Radio" && pCurParent)
            {
                GtkWidget* const pVbox = gtk_vbox_new(FALSE, 12);
                gtk_container_set_border_width(GTK_CONTAINER(pVbox), 0);

                if (!aText.isEmpty())
                    pGroup = lcl_makeFrame(pVbox, aText, aHelpTexts, &nCurHelpText);

                sal_Int32 nSelectVal = 0;
                pVal = m_rController.getValue(aPropertyName);
                if (pVal && pVal->Value.hasValue())
                    pVal->Value >>= nSelectVal;

                for (sal_Int32 m = 0; m != aChoices.getLength(); m++)
                {
                    pWidget = gtk_radio_button_new_with_mnemonic_from_widget(
                        GTK_RADIO_BUTTON(m == 0 ? nullptr : pWidget),
                        OUStringToOString(aChoices[m].replace('~', '_'), RTL_TEXTENCODING_UTF8).getStr());
                    lcl_setHelpText(pWidget, aHelpTexts, nCurHelpText++);
                    m_aControlToPropertyMap[pWidget] = aPropertyName;
                    m_aControlToNumValMap[pWidget] = m;
                    GtkWidget* const pRow = gtk_hbox_new(FALSE, 12);
                    gtk_box_pack_start(GTK_BOX(pVbox), pRow, FALSE, FALSE, 0);
                    gtk_box_pack_start(GTK_BOX(pRow), pWidget, FALSE, FALSE, 0);
                    aPropertyToDependencyRowMap[aPropertyName + OUString::number(m)] = pRow;
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pWidget), m == nSelectVal);
                    gtk_widget_set_sensitive(pWidget,
                        m_rController.isUIOptionEnabled(aPropertyName) && pVal != nullptr);
                    g_signal_connect(pWidget, "toggled",
                            G_CALLBACK(GtkPrintDialog::UIOption_RadioHdl), this);
                }

                if (pGroup)
                    pWidget = pGroup;
                else
                    pWidget = pVbox;
            }
            else if ((aCtrlType == "List"   ||
                       aCtrlType == "Range"  ||
                       aCtrlType == "Edit"
                    ) && pCurParent)
            {
                GtkWidget* const pHbox = gtk_hbox_new(FALSE, 12);
                gtk_container_set_border_width(GTK_CONTAINER(pHbox), 0);

                if ( aCtrlType == "List" )
                {
                   pWidget = lcl_combo_box_text_new();

                   for (sal_Int32 m = 0; m != aChoices.getLength(); m++)
                   {
                       lcl_combo_box_text_append(pWidget,
                           OUStringToOString(aChoices[m], RTL_TEXTENCODING_UTF8).getStr());
                   }

                   sal_Int32 nSelectVal = 0;
                   pVal = m_rController.getValue(aPropertyName);
                   if (pVal && pVal->Value.hasValue())
                       pVal->Value >>= nSelectVal;
                   gtk_combo_box_set_active(GTK_COMBO_BOX(pWidget), nSelectVal);
                   g_signal_connect(pWidget, "changed", G_CALLBACK(GtkPrintDialog::UIOption_SelectHdl), this);
                }
                else if (aCtrlType == "Edit" && pCurParent)
                {
                   pWidget = gtk_entry_new();

                   OUString aCurVal;
                   pVal = m_rController.getValue(aPropertyName);
                   if (pVal && pVal->Value.hasValue())
                       pVal->Value >>= aCurVal;
                   gtk_entry_set_text(GTK_ENTRY(pWidget),
                       OUStringToOString(aCurVal, RTL_TEXTENCODING_UTF8).getStr());
                }
                else if (aCtrlType == "Range" && pCurParent)
                {
                    pWidget = gtk_spin_button_new_with_range(nMinValue, nMaxValue, 1.0);

                    sal_Int64 nCurVal = 0;
                    pVal = m_rController.getValue(aPropertyName);
                    if (pVal && pVal->Value.hasValue())
                        pVal->Value >>= nCurVal;
                    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pWidget), nCurVal);
                }

                lcl_setHelpText(pWidget, aHelpTexts, 0);
                m_aControlToPropertyMap[pWidget] = aPropertyName;

                gtk_widget_set_sensitive(pWidget,
                    m_rController.isUIOptionEnabled(aPropertyName) && pVal != nullptr);

                if (!aText.isEmpty())
                {
                    GtkWidget* const pLabel = gtk_label_new_with_mnemonic(
                        OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
                    gtk_label_set_mnemonic_widget(GTK_LABEL(pLabel), pWidget);
                    gtk_box_pack_start(GTK_BOX(pHbox), pLabel, FALSE, FALSE, 0);
                }

                gtk_box_pack_start(GTK_BOX(pHbox), pWidget, FALSE, FALSE, 0);

                pWidget = pHbox;

            }
            else
                SAL_INFO("vcl.gtk", "unhandled option type: " << aCtrlType);

            GtkWidget* pRow = nullptr;
            if (pWidget)
            {
                if (bUseDependencyRow && !aDependsOnName.isEmpty())
                {
                    pRow = aPropertyToDependencyRowMap[aDependsOnName + OUString::number(nDependsOnValue)];
                    if (!pRow)
                    {
                        gtk_widget_destroy(pWidget);
                        pWidget = nullptr;
                    }
                }
            }
            if (pWidget)
            {
                if (!pRow)
                {
                    pRow = gtk_hbox_new(FALSE, 12);
                    gtk_box_pack_start(GTK_BOX(pCurParent), pRow, FALSE, FALSE, 0);
                }
                if (!pGroup)
                    aPropertyToDependencyRowMap[aPropertyName + OUString::number(0)] = pRow;
                gtk_box_pack_start(GTK_BOX(pRow), pWidget, FALSE, FALSE, 0);
            }
        }
    }

    if (pStandardPrintRangeContainer)
        gtk_widget_destroy(pStandardPrintRangeContainer);

    CustomTabs_t::const_reverse_iterator aEnd = aCustomTabs.rend();
    for (CustomTabs_t::const_reverse_iterator aI = aCustomTabs.rbegin(); aI != aEnd; ++aI)
    {
        gtk_widget_show_all(aI->first);
        m_xWrapper->print_unix_dialog_add_custom_tab(GTK_PRINT_UNIX_DIALOG(m_pDialog), aI->first,
            gtk_label_new(OUStringToOString(aI->second, RTL_TEXTENCODING_UTF8).getStr()));
    }
}

void
GtkPrintDialog::impl_initPrintContent(uno::Sequence<sal_Bool> const& i_rDisabled)
{
    SAL_WARN_IF(i_rDisabled.getLength() != 3, "vcl.gtk", "there is more choices than we expected");
    if (i_rDisabled.getLength() != 3)
        return;

    GtkPrintUnixDialog* const pDialog(GTK_PRINT_UNIX_DIALOG(m_pDialog));

    // XXX: This is a hack that depends on the number and the ordering of
    // the controls in the rDisabled sequence (cf. the initialization of
    // the "PrintContent" UI option in SwPrintUIOptions::SwPrintUIOptions,
    // sw/source/core/view/printdata.cxx)
    if (m_xWrapper->supportsPrintSelection() && !i_rDisabled[2])
    {
        m_xWrapper->print_unix_dialog_set_support_selection(pDialog, TRUE);
        m_xWrapper->print_unix_dialog_set_has_selection(pDialog, TRUE);
    }

    beans::PropertyValue* const pPrintContent(
            m_rController.getValue(OUString("PrintContent")));

    if (pPrintContent)
    {
        sal_Int32 nSelectionType(0);
        pPrintContent->Value >>= nSelectionType;
        GtkPrintSettings* const pSettings(getSettings());
        GtkPrintPages ePrintPages(GTK_PRINT_PAGES_ALL);
        switch (nSelectionType)
        {
            case 0:
                ePrintPages = GTK_PRINT_PAGES_ALL;
                break;
            case 1:
                ePrintPages = GTK_PRINT_PAGES_RANGES;
                break;
            case 2:
                if (m_xWrapper->supportsPrintSelection())
                    ePrintPages = GTK_PRINT_PAGES_SELECTION;
                else
                    SAL_INFO("vcl.gtk", "the application wants to print a selection, but the present gtk version does not support it");
                break;
            default:
                SAL_WARN("vcl.gtk", "unexpected selection type: " << nSelectionType);
        }
        m_xWrapper->print_settings_set_print_pages(pSettings, ePrintPages);
        m_xWrapper->print_unix_dialog_set_settings(pDialog, pSettings);
        g_object_unref(G_OBJECT(pSettings));
    }
}

void
GtkPrintDialog::impl_checkOptionalControlDependencies()
{
    for (std::map<GtkWidget*, OUString>::iterator it = m_aControlToPropertyMap.begin();
         it != m_aControlToPropertyMap.end(); ++it)
    {
        gtk_widget_set_sensitive(it->first, m_rController.isUIOptionEnabled(it->second));
    }
}

beans::PropertyValue*
GtkPrintDialog::impl_queryPropertyValue(GtkWidget* const i_pWidget) const
{
    beans::PropertyValue* pVal(nullptr);
    std::map<GtkWidget*, OUString>::const_iterator aIt(m_aControlToPropertyMap.find(i_pWidget));
    if (aIt != m_aControlToPropertyMap.end())
    {
        pVal = m_rController.getValue(aIt->second);
        SAL_WARN_IF(!pVal, "vcl.gtk", "property value not found");
    }
    else
    {
        SAL_WARN("vcl.gtk", "changed control not in property map");
    }
    return pVal;
}

void
GtkPrintDialog::impl_UIOption_CheckHdl(GtkWidget* const i_pWidget)
{
    beans::PropertyValue* const pVal = impl_queryPropertyValue(i_pWidget);
    if (pVal)
    {
        const bool bVal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(i_pWidget));
        pVal->Value <<= bVal;

        impl_checkOptionalControlDependencies();
    }
}

void
GtkPrintDialog::impl_UIOption_RadioHdl(GtkWidget* const i_pWidget)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(i_pWidget)))
    {
        beans::PropertyValue* const pVal = impl_queryPropertyValue(i_pWidget);
        std::map<GtkWidget*, sal_Int32>::const_iterator it = m_aControlToNumValMap.find(i_pWidget);
        if (pVal && it != m_aControlToNumValMap.end())
        {

            const sal_Int32 nVal = it->second;
            pVal->Value <<= nVal;

            impl_checkOptionalControlDependencies();
        }
    }
}

void
GtkPrintDialog::impl_UIOption_SelectHdl(GtkWidget* const i_pWidget)
{
    beans::PropertyValue* const pVal = impl_queryPropertyValue(i_pWidget);
    if (pVal)
    {
        const sal_Int32 nVal(gtk_combo_box_get_active(GTK_COMBO_BOX(i_pWidget)));
        pVal->Value <<= nVal;

        impl_checkOptionalControlDependencies();
    }
}

bool
GtkPrintDialog::run()
{
    bool bDoJob = false;
    bool bContinue = true;
    while (bContinue)
    {
        bContinue = false;
        const gint nStatus = gtk_dialog_run(GTK_DIALOG(m_pDialog));
        switch (nStatus)
        {
            case GTK_RESPONSE_HELP:
                fprintf(stderr, "To-Do: Help ?\n");
                bContinue = true;
                break;
            case GTK_RESPONSE_OK:
                bDoJob = true;
                break;
            default:
                break;
        }
    }
    gtk_widget_hide(m_pDialog);
    impl_storeToSettings();
    return bDoJob;
}

#if 0
void GtkPrintDialog::ExportAsPDF(const OUString &rFileURL, GtkPrintSettings *pSettings) const
{
    uno::Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
    uno::Reference < XFrame > xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame.set(xDesktop, UNO_QUERY);

    uno::Reference < XFilter > xFilter(
        ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.PDFFilter"),
        UNO_QUERY);

    if (xFilter.is())
    {
        uno::Reference< XController > xController;
        uno::Reference< XComponent > xDoc;
        if (xFrame.is())
            xController = xFrame->getController();
        if (xController.is())
            xDoc.set(xController->getModel(), UNO_QUERY);

        SvFileStream aStream(rFileURL, STREAM_READWRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC);
        uno::Reference< XOutputStream > xOStm(new utl::OOutputStreamWrapper(aStream));

        uno::Reference< XExporter > xExport(xFilter, UNO_QUERY);
        xExport->setSourceDocument(xDoc);
        uno::Sequence<beans::PropertyValue> aFilterData(2);
        aFilterData[0].Name = "PageLayout";
        aFilterData[0].Value <<= sal_Int32(0);
        aFilterData[1].Name = "FirstPageOnLeft";
        aFilterData[1].Value <<= sal_False;

        const gchar *pStr = gtk_print_settings_get(pSettings, GTK_PRINT_SETTINGS_PRINT_PAGES);
        if (pStr && !strcmp(pStr, "ranges"))
        {
            String aRangeText;
            gint num_ranges;
            const GtkPageRange* pRanges = gtk_print_settings_get_page_ranges(pSettings, &num_ranges);
            for (gint i = 0; i < num_ranges; ++i)
            {
                aRangeText.Append(OUString::number(pRanges[i].start+1));
                if (pRanges[i].start != pRanges[i].end)
                {
                    aRangeText.AppendAscii("-");
                    aRangeText.Append(OUString::number(pRanges[i].end+1));
                }

                if (i != num_ranges-1)
                    aRangeText.AppendAscii(",");
            }
            aFilterData.realloc(aFilterData.getLength()+1);
            aFilterData[aFilterData.getLength()-1].Name = "PageRange";
            aFilterData[aFilterData.getLength()-1].Value <<= OUString(aRangeText);
        }
        else if (pStr && !strcmp(pStr, "current"))
        {
            try
            {
                   uno::Reference< XSpreadsheetView > xSpreadsheetView;
                   uno::Reference< XSpreadsheet> xSheet;
                   uno::Reference< XSpreadsheetDocument > xSheetDoc;
                   uno::Reference< XIndexAccess > xSheets;
                   uno::Reference< XNamed > xName;

                   if (xController.is())
                       xSpreadsheetView.set(xController, UNO_QUERY);
                   if (xSpreadsheetView.is())
                       xSheet.set(xSpreadsheetView->getActiveSheet());
                   if (xSheet.is())
                       xName.set(xSheet, UNO_QUERY);
                   if (xName.is())
                       xSheetDoc.set(xController->getModel(), UNO_QUERY);
                   if (xSheetDoc.is())
                       xSheets.set(xSheetDoc->getSheets(), UNO_QUERY);
                   if (xSheets.is())
                   {
                       const OUString &rName = xName->getName();

                       sal_Int32 i;

                       for (i = 0; i < xSheets->getCount(); ++i)
                       {
                           uno::Reference < XNamed > xItem =
                               uno::Reference < XNamed >(xSheets->getByIndex(i), UNO_QUERY);
                           if (rName == xItem->getName())
                               break;
                       }

                       if (i < xSheets->getCount())
                       {
                            aFilterData.realloc(aFilterData.getLength()+1);
                            aFilterData[aFilterData.getLength()-1].Name = "PageRange";
                            aFilterData[aFilterData.getLength()-1].Value <<= OUString(OUString::number(i + 1));
                       }
                   }
            }
            catch (...) {}
        }
        if (gtk_print_unix_dialog_get_has_selection(GTK_PRINT_UNIX_DIALOG(m_pDialog)))
        {
            uno::Any aSelection;
            try
            {
                if (xController.is())
                {
                    uno::Reference<view::XSelectionSupplier> xView(xController, UNO_QUERY);
                    if (xView.is())
                        xView->getSelection() >>= aSelection;
                }
            }
            catch (const uno::RuntimeException &)
            {
            }
            if (aSelection.hasValue())
            {
                aFilterData.realloc(aFilterData.getLength()+1);
                aFilterData[aFilterData.getLength()-1].Name = "Selection";
                aFilterData[aFilterData.getLength()-1].Value <<= aSelection;
            }
        }
        uno::Sequence<beans::PropertyValue> aArgs(2);
        aArgs[0].Name = "FilterData";
        aArgs[0].Value <<= aFilterData;
        aArgs[1].Name = "OutputStream";
        aArgs[1].Value <<= xOStm;
        xFilter->filter(aArgs);
    }
}
#endif

void
GtkPrintDialog::updateControllerPrintRange()
{
    GtkPrintSettings* const pSettings(getSettings());
    // TODO: use get_print_pages
    if (const gchar* const pStr = m_xWrapper->print_settings_get(pSettings, GTK_PRINT_SETTINGS_PRINT_PAGES))
    {
        beans::PropertyValue* pVal = m_rController.getValue(OUString("PrintRange"));
        if (!pVal)
            pVal = m_rController.getValue(OUString("PrintContent"));
        SAL_WARN_IF(!pVal, "vcl.gtk", "Nothing to map standard print options to!");
        if (pVal)
        {
            sal_Int32 nVal = 0;
            if (!strcmp(pStr, "all"))
                nVal = 0;
            else if (!strcmp(pStr, "ranges"))
                nVal = 1;
            else if (!strcmp(pStr, "selection"))
                nVal = 2;
            pVal->Value <<= nVal;

            if (nVal == 1)
            {
                pVal = m_rController.getValue(OUString("PageRange"));
                SAL_WARN_IF(!pVal, "vcl.gtk", "PageRange doesn't exist!");
                if (pVal)
                {
                    OUStringBuffer sBuf;
                    gint num_ranges;
                    const GtkPageRange* const pRanges = m_xWrapper->print_settings_get_page_ranges(pSettings, &num_ranges);
                    for (gint i = 0; i != num_ranges && pRanges; ++i)
                    {
                        sBuf.append(sal_Int32(pRanges[i].start+1));
                        if (pRanges[i].start != pRanges[i].end)
                        {
                            sBuf.append('-');
                            sBuf.append(sal_Int32(pRanges[i].end+1));
                        }

                        if (i != num_ranges-1)
                            sBuf.append(',');
                    }
                    pVal->Value <<= sBuf.makeStringAndClear();
                }
            }
        }
    }
    g_object_unref(G_OBJECT(pSettings));
}

GtkPrintDialog::~GtkPrintDialog()
{
    gtk_widget_destroy(m_pDialog);
}

void
GtkPrintDialog::impl_readFromSettings()
{
    vcl::SettingsConfigItem* const pItem(vcl::SettingsConfigItem::get());
    GtkPrintSettings* const pSettings(getSettings());

    const OUString aPrintDialogStr("PrintDialog");
    const OUString aCopyCount(pItem->getValue(aPrintDialogStr,
                "CopyCount"));
    const OUString aCollate(pItem->getValue(aPrintDialogStr,
                "Collate"));

    bool bChanged(false);

    const gint nOldCopyCount(m_xWrapper->print_settings_get_n_copies(pSettings));
    const sal_Int32 nCopyCount(aCopyCount.toInt32());
    if (nCopyCount > 0 && nOldCopyCount != nCopyCount)
    {
        bChanged = true;
        m_xWrapper->print_settings_set_n_copies(pSettings, sal::static_int_cast<gint>(nCopyCount));
    }

    const bool bOldCollate(m_xWrapper->print_settings_get_collate(pSettings));
    const bool bCollate(aCollate.equalsIgnoreAsciiCase("true"));
    if (bOldCollate != bCollate)
    {
        bChanged = true;
        m_xWrapper->print_settings_set_collate(pSettings, bCollate);
    }
    // TODO: wth was this var. meant for?
    (void) bChanged;

    m_xWrapper->print_unix_dialog_set_settings(GTK_PRINT_UNIX_DIALOG(m_pDialog), pSettings);
    g_object_unref(G_OBJECT(pSettings));
}

void
GtkPrintDialog::impl_storeToSettings()
const
{
    vcl::SettingsConfigItem* const pItem(vcl::SettingsConfigItem::get());
    GtkPrintSettings* const pSettings(getSettings());

    const OUString aPrintDialogStr("PrintDialog");
    pItem->setValue(aPrintDialogStr,
            "CopyCount",
            OUString::number(m_xWrapper->print_settings_get_n_copies(pSettings)));
    pItem->setValue(aPrintDialogStr,
            "Collate",
            m_xWrapper->print_settings_get_collate(pSettings)
                ? OUString("true")
                : OUString("false"))
        ;
    // pItem->setValue(aPrintDialog, OUString("ToFile"), );
    g_object_unref(G_OBJECT(pSettings));
    pItem->Commit();
}

sal_uLong
GtkSalInfoPrinter::GetCapabilities(
        const ImplJobSetup* const i_pSetupData,
        const PrinterCapType i_nType)
{
    if (i_nType == PrinterCapType::ExternalDialog && lcl_useSystemPrintDialog())
        return 1;
    return PspSalInfoPrinter::GetCapabilities(i_pSetupData, i_nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
