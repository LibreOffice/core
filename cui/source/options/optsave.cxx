/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <string_view>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "optsave.hxx"
#include <comphelper/processfactory.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/saveopt.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <sfx2/sfxsids.hrc>
#include <sfx2/docfilt.hxx>
#include <unotools/optionsdlg.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#include <sfx2/fcontnr.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace comphelper;

#define CFG_PAGE_AND_GROUP          u"General", u"LoadSave"


struct SvxSaveTabPage_Impl
{
    Reference< XNameContainer > xFact;
    std::vector< OUString >     aFilterArr[APP_COUNT];
    std::vector< bool >         aODFArr[APP_COUNT];
    std::vector< OUString >     aUIFilterArr[APP_COUNT];
    OUString                    aDefaultArr[APP_COUNT];
    bool                    aDefaultReadonlyArr[APP_COUNT];
    bool                    bInitialized;

    SvxSaveTabPage_Impl();
};

SvxSaveTabPage_Impl::SvxSaveTabPage_Impl() : bInitialized( false )
{
}

// class SvxSaveTabPage --------------------------------------------------

SvxSaveTabPage::SvxSaveTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage( pPage, pController, "cui/ui/optsavepage.ui", "OptSavePage", &rCoreSet )
    , pImpl(new SvxSaveTabPage_Impl)
    , m_xLoadUserSettingsCB(m_xBuilder->weld_check_button("load_settings"))
    , m_xLoadDocPrinterCB(m_xBuilder->weld_check_button("load_docprinter"))
    , m_xDocInfoCB(m_xBuilder->weld_check_button("docinfo"))
    , m_xBackupCB(m_xBuilder->weld_check_button("backup"))
    , m_xAutoSaveCB(m_xBuilder->weld_check_button("autosave"))
    , m_xAutoSaveEdit(m_xBuilder->weld_spin_button("autosave_spin"))
    , m_xMinuteFT(m_xBuilder->weld_label("autosave_mins"))
    , m_xUserAutoSaveCB(m_xBuilder->weld_check_button("userautosave"))
    , m_xRelativeFsysCB(m_xBuilder->weld_check_button("relative_fsys"))
    , m_xRelativeInetCB(m_xBuilder->weld_check_button("relative_inet"))
    , m_xODFVersionLB(m_xBuilder->weld_combo_box("odfversion"))
    , m_xWarnAlienFormatCB(m_xBuilder->weld_check_button("warnalienformat"))
    , m_xDocTypeLB(m_xBuilder->weld_combo_box("doctype"))
    , m_xSaveAsFT(m_xBuilder->weld_label("saveas_label"))
    , m_xSaveAsLB(m_xBuilder->weld_combo_box("saveas"))
    , m_xODFWarningFI(m_xBuilder->weld_widget("odfwarning_image"))
    , m_xODFWarningFT(m_xBuilder->weld_label("odfwarning_label"))
{
    m_xODFVersionLB->set_id(0, OUString::number(SvtSaveOptions::ODFVER_011)); // 1.0/1.1
    m_xODFVersionLB->set_id(1, OUString::number(SvtSaveOptions::ODFVER_012)); // 1.2
    m_xODFVersionLB->set_id(2, OUString::number(SvtSaveOptions::ODFVER_012_EXT_COMPAT)); // 1.2 Extended (compatibility mode)
    m_xODFVersionLB->set_id(3, OUString::number(SvtSaveOptions::ODFVER_012_EXTENDED)); // 1.2 Extended
    m_xODFVersionLB->set_id(4, OUString::number(SvtSaveOptions::ODFVER_013)); // 1.3
    m_xODFVersionLB->set_id(5, OUString::number(SvtSaveOptions::ODFVER_LATEST)); // 1.3 Extended (recommended)

    m_xDocTypeLB->set_id(0, OUString::number(APP_WRITER)       );
    m_xDocTypeLB->set_id(1, OUString::number(APP_WRITER_WEB)   );
    m_xDocTypeLB->set_id(2, OUString::number(APP_WRITER_GLOBAL));
    m_xDocTypeLB->set_id(3, OUString::number(APP_CALC)         );
    m_xDocTypeLB->set_id(4, OUString::number(APP_IMPRESS)      );
    m_xDocTypeLB->set_id(5, OUString::number(APP_DRAW)         );
    m_xDocTypeLB->set_id(6, OUString::number(APP_MATH)         );

    m_xAutoSaveCB->connect_clicked( LINK( this, SvxSaveTabPage, AutoClickHdl_Impl ) );

    SvtModuleOptions aModuleOpt;
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
    {
        m_xSaveAsLB->remove_id(OUString::number(APP_MATH));
        m_xDocTypeLB->remove_id(OUString::number(APP_MATH));
    }
    else
    {
        pImpl->aDefaultArr[APP_MATH] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::MATH);
        pImpl->aDefaultReadonlyArr[APP_MATH] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::MATH);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
    {
        m_xSaveAsLB->remove_id(OUString::number(APP_DRAW));
        m_xDocTypeLB->remove_id(OUString::number(APP_DRAW));
    }
    else
    {
        pImpl->aDefaultArr[APP_DRAW] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::DRAW);
        pImpl->aDefaultReadonlyArr[APP_DRAW] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::DRAW);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        m_xSaveAsLB->remove_id(OUString::number(APP_IMPRESS));
        m_xDocTypeLB->remove_id(OUString::number(APP_IMPRESS));
    }
    else
    {
        pImpl->aDefaultArr[APP_IMPRESS] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::IMPRESS);
        pImpl->aDefaultReadonlyArr[APP_IMPRESS] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::IMPRESS);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        m_xSaveAsLB->remove_id(OUString::number(APP_CALC));
        m_xDocTypeLB->remove_id(OUString::number(APP_CALC));
    }
    else
    {
        pImpl->aDefaultArr[APP_CALC] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::CALC);
        pImpl->aDefaultReadonlyArr[APP_CALC] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::CALC);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
    {
        m_xSaveAsLB->remove_id(OUString::number(APP_WRITER));
        m_xSaveAsLB->remove_id(OUString::number(APP_WRITER_WEB));
        m_xSaveAsLB->remove_id(OUString::number(APP_WRITER_GLOBAL));
        m_xDocTypeLB->remove_id(OUString::number(APP_WRITER));
        m_xDocTypeLB->remove_id(OUString::number(APP_WRITER_WEB));
        m_xDocTypeLB->remove_id(OUString::number(APP_WRITER_GLOBAL));
    }
    else
    {
        pImpl->aDefaultArr[APP_WRITER] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITER);
        pImpl->aDefaultArr[APP_WRITER_WEB] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERWEB);
        pImpl->aDefaultArr[APP_WRITER_GLOBAL] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERGLOBAL);
        pImpl->aDefaultReadonlyArr[APP_WRITER] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::WRITER);
        pImpl->aDefaultReadonlyArr[APP_WRITER_WEB] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::WRITERWEB);
        pImpl->aDefaultReadonlyArr[APP_WRITER_GLOBAL] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::WRITERGLOBAL);
    }

    Link<weld::ComboBox&,void> aLink = LINK( this, SvxSaveTabPage, ODFVersionHdl_Impl );
    m_xODFVersionLB->connect_changed( aLink );
    aLink = LINK( this, SvxSaveTabPage, FilterHdl_Impl );
    m_xDocTypeLB->connect_changed( aLink );
    m_xSaveAsLB->connect_changed( aLink );

    DetectHiddenControls();
}

SvxSaveTabPage::~SvxSaveTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxSaveTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxSaveTabPage>(pPage, pController, *rAttrSet);
}

void SvxSaveTabPage::DetectHiddenControls()
{
    SvtOptionsDialogOptions aOptionsDlgOpt;

    if ( aOptionsDlgOpt.IsOptionHidden( u"Backup", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "Backup"
        m_xBackupCB->hide();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( u"AutoSave", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "AutoSave"
        m_xAutoSaveCB->hide();
        m_xAutoSaveEdit->hide();
        m_xMinuteFT->hide();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( u"UserAutoSave", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "UserAutoSave"
        m_xUserAutoSaveCB->hide();
    }

}

bool SvxSaveTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    SvtSaveOptions aSaveOpt;
    if(m_xLoadUserSettingsCB->get_state_changed_from_saved())
    {
        aSaveOpt.SetLoadUserSettings(m_xLoadUserSettingsCB->get_active());
    }

    if ( m_xLoadDocPrinterCB->get_state_changed_from_saved() )
        aSaveOpt.SetLoadDocumentPrinter( m_xLoadDocPrinterCB->get_active() );

    if ( m_xODFVersionLB->get_value_changed_from_saved() )
    {
        sal_Int32 nVersion = m_xODFVersionLB->get_active_id().toInt32();
        aSaveOpt.SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion( nVersion ) );
    }

    if ( m_xDocInfoCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_DOCINFO ),
                               m_xDocInfoCB->get_active() ) );
        bModified = true;
    }

    if ( m_xBackupCB->get_sensitive() && m_xBackupCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_BACKUP ),
                               m_xBackupCB->get_active() ) );
        bModified = true;
    }

    if ( m_xAutoSaveCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVE ),
                               m_xAutoSaveCB->get_active() ) );
        bModified = true;
    }
    if ( m_xWarnAlienFormatCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_WARNALIENFORMAT ),
                               m_xWarnAlienFormatCB->get_active() ) );
        bModified = true;
    }

    if ( m_xAutoSaveEdit->get_value_changed_from_saved() )
    {
        rSet->Put( SfxUInt16Item( GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                 static_cast<sal_uInt16>(m_xAutoSaveEdit->get_value()) ) );
        bModified = true;
    }

    if ( m_xUserAutoSaveCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_USERAUTOSAVE ),
                               m_xUserAutoSaveCB->get_active() ) );
        bModified = true;
    }
    // save relatively
    if ( m_xRelativeFsysCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_SAVEREL_FSYS ),
                               m_xRelativeFsysCB->get_active() ) );
        bModified = true;
    }

    if ( m_xRelativeInetCB->get_state_changed_from_saved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_SAVEREL_INET ),
                               m_xRelativeInetCB->get_active() ) );
        bModified = true;
    }

    SvtModuleOptions aModuleOpt;
    if(!pImpl->aDefaultArr[APP_MATH].isEmpty() &&
            pImpl->aDefaultArr[APP_MATH] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::MATH))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::MATH, pImpl->aDefaultArr[APP_MATH]);

    if( !pImpl->aDefaultArr[APP_DRAW].isEmpty() &&
            pImpl->aDefaultArr[APP_DRAW] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::DRAW))
            aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::DRAW, pImpl->aDefaultArr[APP_DRAW]);

    if(!pImpl->aDefaultArr[APP_IMPRESS].isEmpty() &&
            pImpl->aDefaultArr[APP_IMPRESS] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::IMPRESS))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::IMPRESS, pImpl->aDefaultArr[APP_IMPRESS]);

    if(!pImpl->aDefaultArr[APP_CALC].isEmpty() &&
            pImpl->aDefaultArr[APP_CALC] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::CALC))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::CALC, pImpl->aDefaultArr[APP_CALC]);

    if(!pImpl->aDefaultArr[APP_WRITER].isEmpty() &&
            pImpl->aDefaultArr[APP_WRITER] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITER))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITER, pImpl->aDefaultArr[APP_WRITER]);

    if(!pImpl->aDefaultArr[APP_WRITER_WEB].isEmpty() &&
            pImpl->aDefaultArr[APP_WRITER_WEB] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERWEB))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERWEB, pImpl->aDefaultArr[APP_WRITER_WEB]);

    if(!pImpl->aDefaultArr[APP_WRITER_GLOBAL].isEmpty() &&
            pImpl->aDefaultArr[APP_WRITER_GLOBAL] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERGLOBAL))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::EFactory::WRITERGLOBAL, pImpl->aDefaultArr[APP_WRITER_GLOBAL]);

    return bModified;
}

static bool isODFFormat( const OUString& sFilter )
{
    static const char* aODFFormats[] =
    {
        "writer8",
        "writer8_template",
        "writerglobal8",
        "writerglobal8_writer",
        "calc8",
        "calc8_template",
        "draw8",
        "draw8_template",
        "impress8",
        "impress8_template",
        "impress8_draw",
        "chart8",
        "math8",
        nullptr
    };

    bool bRet = false;
    int i = 0;
    while ( aODFFormats[i] != nullptr )
    {
        if ( sFilter.equalsAscii( aODFFormats[i++] ) )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}

void SvxSaveTabPage::Reset( const SfxItemSet* )
{
    SvtSaveOptions aSaveOpt;
    m_xLoadUserSettingsCB->set_active(aSaveOpt.IsLoadUserSettings());
    m_xLoadUserSettingsCB->save_state();
    m_xLoadUserSettingsCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::UseUserData));
    m_xLoadDocPrinterCB->set_active( aSaveOpt.IsLoadDocumentPrinter() );
    m_xLoadDocPrinterCB->save_state();
    m_xLoadDocPrinterCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::LoadDocPrinter));

    if ( !pImpl->bInitialized )
    {
        try
        {
            Reference< XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
            pImpl->xFact.set(xMSF->createInstance("com.sun.star.document.FilterFactory"), UNO_QUERY);

            DBG_ASSERT(pImpl->xFact.is(), "service com.sun.star.document.FilterFactory unavailable");
            Reference< XContainerQuery > xQuery(pImpl->xFact, UNO_QUERY);
            if(xQuery.is())
            {
                for (sal_Int32 n = 0, nEntryCount = m_xDocTypeLB->get_count(); n < nEntryCount; ++n)
                {
                    unsigned int nData = m_xDocTypeLB->get_id(n).toUInt32();
                    OUString sCommand = "getSortedFilterList():module=%1:iflags=" +
                               OUString::number(static_cast<int>(SfxFilterFlags::IMPORT|SfxFilterFlags::EXPORT)) +
                               ":eflags=" +
                               OUString::number(static_cast<int>(SfxFilterFlags::NOTINFILEDLG));
                    OUString sReplace;
                    switch(nData)
                    {
                        case  APP_WRITER        : sReplace = "com.sun.star.text.TextDocument";  break;
                        case  APP_WRITER_WEB    : sReplace = "com.sun.star.text.WebDocument";   break;
                        case  APP_WRITER_GLOBAL : sReplace = "com.sun.star.text.GlobalDocument";   break;
                        case  APP_CALC          : sReplace = "com.sun.star.sheet.SpreadsheetDocument";break;
                        case  APP_IMPRESS       : sReplace = "com.sun.star.presentation.PresentationDocument";break;
                        case  APP_DRAW          : sReplace = "com.sun.star.drawing.DrawingDocument";break;
                        case  APP_MATH          : sReplace = "com.sun.star.formula.FormulaProperties";break;
                        default: OSL_FAIL("illegal user data");
                    }
                    sCommand = sCommand.replaceFirst("%1", sReplace);
                    Reference< XEnumeration > xList = xQuery->createSubSetEnumerationByQuery(sCommand);
                    std::vector< OUString > lList;
                    std::vector<bool> lODFList;
                    while(xList->hasMoreElements())
                    {
                        SequenceAsHashMap aFilter(xList->nextElement());
                        OUString sFilter = aFilter.getUnpackedValueOrDefault("Name",OUString());
                        if (!sFilter.isEmpty())
                        {
                            lList.push_back(sFilter);
                            lODFList.push_back( isODFFormat( sFilter ) );
                        }
                    }
                    pImpl->aFilterArr[nData] = lList;
                    pImpl->aODFArr[nData] = lODFList;
                }
            }
            m_xDocTypeLB->set_active(0);
            FilterHdl_Impl(*m_xDocTypeLB);
        }
        catch(Exception const &)
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "exception in FilterFactory access" );
        }

        pImpl->bInitialized = true;
    }

    m_xDocInfoCB->set_active(aSaveOpt.IsDocInfoSave());
    m_xDocInfoCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::DocInfSave));

    m_xBackupCB->set_active(aSaveOpt.IsBackup());
    m_xBackupCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::Backup));

    m_xAutoSaveCB->set_active(aSaveOpt.IsAutoSave());
    m_xAutoSaveCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::AutoSave));

    m_xUserAutoSaveCB->set_active(aSaveOpt.IsUserAutoSave());
    m_xUserAutoSaveCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::UserAutoSave));

    m_xWarnAlienFormatCB->set_active(aSaveOpt.IsWarnAlienFormat());
    m_xWarnAlienFormatCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::WarnAlienFormat));

    m_xAutoSaveEdit->set_value(aSaveOpt.GetAutoSaveTime());
    m_xAutoSaveEdit->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::AutoSaveTime));

    // save relatively
    m_xRelativeFsysCB->set_active(aSaveOpt.IsSaveRelFSys());
    m_xRelativeFsysCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::SaveRelFsys));

    m_xRelativeInetCB->set_active(aSaveOpt.IsSaveRelINet());
    m_xRelativeInetCB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::SaveRelInet));

    sal_Int32 nDefaultVersion = aSaveOpt.GetODFDefaultVersion();
    m_xODFVersionLB->set_active_id(OUString::number(nDefaultVersion));
    m_xODFVersionLB->set_sensitive(!aSaveOpt.IsReadOnly(SvtSaveOptions::EOption::OdfDefaultVersion));

    AutoClickHdl_Impl(*m_xAutoSaveCB);
    ODFVersionHdl_Impl(*m_xODFVersionLB);

    m_xDocInfoCB->save_state();
    m_xBackupCB->save_state();
    m_xWarnAlienFormatCB->save_state();
    m_xAutoSaveCB->save_state();
    m_xAutoSaveEdit->save_value();

    m_xUserAutoSaveCB->save_state();

    m_xRelativeFsysCB->save_state();
    m_xRelativeInetCB->save_state();
    m_xODFVersionLB->save_value();
}

IMPL_LINK(SvxSaveTabPage, AutoClickHdl_Impl, weld::Button&, rBox, void)
{
    if (&rBox != m_xAutoSaveCB.get())
        return;

    if (m_xAutoSaveCB->get_active())
    {
        m_xAutoSaveEdit->set_sensitive(true);
        m_xMinuteFT->set_sensitive(true);
        m_xUserAutoSaveCB->set_sensitive(true);
    }
    else
    {
        m_xAutoSaveEdit->set_sensitive(false);
        m_xMinuteFT->set_sensitive(false);
        m_xUserAutoSaveCB->set_sensitive(false);
    }
}

static OUString lcl_ExtracUIName(const Sequence<PropertyValue> &rProperties, std::u16string_view rExtension)
{
    OUString sName;
    const PropertyValue* pPropVal = rProperties.getConstArray();
    const PropertyValue* const pEnd = pPropVal + rProperties.getLength();
    for( ; pPropVal != pEnd; pPropVal++ )
    {
        const OUString &rName = pPropVal->Name;
        if (rName == "UIName")
        {
            OUString sUIName;
            if ( ( pPropVal->Value >>= sUIName ) && sUIName.getLength() )
            {
                if (!rExtension.empty())
                {
                    return sUIName + " (" + rExtension + ")";
                }
                else
                {
                    return sUIName;
                }
            }
        }
        else if (rName == "Name")
        {
            pPropVal->Value >>= sName;
        }
    }

    OSL_ENSURE( false, "Filter without UIName!" );

    return sName;
}

IMPL_LINK( SvxSaveTabPage, FilterHdl_Impl, weld::ComboBox&, rBox, void )
{
    const int nCurPos = m_xDocTypeLB->get_active();

    int nData = -1;
    if (nCurPos < APP_COUNT)
        nData = m_xDocTypeLB->get_id(nCurPos).toInt32();

    if ( nData >= 0 && nData < APP_COUNT )
    {
        if(m_xDocTypeLB.get() == &rBox)
        {
            m_xSaveAsLB->clear();
            auto & rFilters = pImpl->aFilterArr[nData];
            if(pImpl->aUIFilterArr[nData].empty())
            {
                pImpl->aUIFilterArr[nData].resize(pImpl->aFilterArr[nData].size());
                auto & rUIFilters = pImpl->aUIFilterArr[nData];
                for(size_t nFilter = 0; nFilter < pImpl->aFilterArr[nData].size(); nFilter++)
                {
                    Any aProps = pImpl->xFact->getByName(rFilters[nFilter]);
                    // get the extension of the filter
                    OUString extension;
                    SfxFilterMatcher matcher;
                    std::shared_ptr<const SfxFilter> pFilter = matcher.GetFilter4FilterName(rFilters[nFilter]);
                    if (pFilter)
                    {
                        extension = pFilter->GetWildcard().getGlob().getToken(0, ';');
                    }
                    Sequence<PropertyValue> aProperties;
                    aProps >>= aProperties;
                    rUIFilters[nFilter] = lcl_ExtracUIName(aProperties, extension);
                }
            }
            auto const & rUIFilters = pImpl->aUIFilterArr[nData];
            OUString sSelect;
            for(size_t i = 0; i < pImpl->aUIFilterArr[nData].size(); i++)
            {
                OUString sId;
                if (pImpl->aODFArr[nData][i])
                    sId = OUString::number(reinterpret_cast<sal_Int64>(pImpl.get()));
                m_xSaveAsLB->append(sId, rUIFilters[i]);
                if (rFilters[i] == pImpl->aDefaultArr[nData])
                    sSelect = rUIFilters[i];
            }
            if (!sSelect.isEmpty())
            {
                m_xSaveAsLB->set_active_text(sSelect);
            }

            m_xSaveAsFT->set_sensitive(!pImpl->aDefaultReadonlyArr[nData]);
            m_xSaveAsLB->set_sensitive(!pImpl->aDefaultReadonlyArr[nData]);
        }
        else
        {
            OUString sSelect = rBox.get_active_text();
            auto const & rFilters = pImpl->aFilterArr[nData];
            auto const & rUIFilters = pImpl->aUIFilterArr[nData];
            for(size_t i = 0; i < pImpl->aUIFilterArr[nData].size(); i++)
                if(rUIFilters[i] == sSelect)
                {
                    sSelect = rFilters[i];
                    break;
                }

            pImpl->aDefaultArr[nData] = sSelect;
        }
    }

    ODFVersionHdl_Impl( *m_xSaveAsLB );
}

IMPL_LINK_NOARG(SvxSaveTabPage, ODFVersionHdl_Impl, weld::ComboBox&, void)
{
    sal_Int32 nVersion = m_xODFVersionLB->get_active_id().toInt32();
    bool bShown = SvtSaveOptions::ODFDefaultVersion(nVersion) != SvtSaveOptions::ODFVER_LATEST;
    if ( bShown )
    {
        bool bHasODFFormat = false;
        const int nCount = m_xSaveAsLB->get_count();
        for (int i = 0; i < nCount; ++i )
        {
            if ( m_xSaveAsLB->get_id(i).toInt64() != 0 )
            {
                bHasODFFormat = true;
                break;
            }
        }

        bShown = !bHasODFFormat
                || ( m_xSaveAsLB->get_active_id().toInt64() != 0);
    }

    m_xODFWarningFI->set_visible(bShown);
    m_xODFWarningFT->set_visible(bShown);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
