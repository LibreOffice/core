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

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>

#include "optsave.hrc"
#include <cuires.hrc>

#include "optsave.hxx"
#include <dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/saveopt.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <sfx2/docfilt.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/fixed.hxx>
#include <unotools/configitem.hxx>
#include <unotools/optionsdlg.hxx>

#include <vcl/msgbox.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace comphelper;

#define CFG_PAGE_AND_GROUP          "General", "LoadSave"


struct SvxSaveTabPage_Impl
{
    Reference< XNameContainer > xFact;
    Sequence< OUString >        aFilterArr[APP_COUNT];
    Sequence< sal_Bool >        aAlienArr[APP_COUNT];
    Sequence< sal_Bool >        aODFArr[APP_COUNT];
    Sequence< OUString >        aUIFilterArr[APP_COUNT];
    OUString                    aDefaultArr[APP_COUNT];
    bool                    aDefaultReadonlyArr[APP_COUNT];
    bool                    bInitialized;

    SvxSaveTabPage_Impl();
    ~SvxSaveTabPage_Impl();
};

SvxSaveTabPage_Impl::SvxSaveTabPage_Impl() : bInitialized( false )
{
}

SvxSaveTabPage_Impl::~SvxSaveTabPage_Impl()
{
}

// class SvxSaveTabPage --------------------------------------------------

SvxSaveTabPage::SvxSaveTabPage( vcl::Window* pParent, const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, "OptSavePage", "cui/ui/optsavepage.ui", &rCoreSet ),
    pImpl( new SvxSaveTabPage_Impl )
{
    get(aLoadUserSettingsCB, "load_settings");
    get(aLoadDocPrinterCB,  "load_docprinter");

    get(aDocInfoCB, "docinfo");
    get(aBackupCB, "backup");
    get(aAutoSaveCB, "autosave");
    get(aAutoSaveEdit, "autosave_spin");
    get(aMinuteFT, "autosave_mins");
    get(aUserAutoSaveCB, "userautosave");
    get(aRelativeFsysCB, "relative_fsys");
    get(aRelativeInetCB, "relative_inet");

    get(aODFVersionLB, "odfversion");
    get(aWarnAlienFormatCB, "warnalienformat");
    get(aDocTypeLB, "doctype");
    get(aSaveAsFT, "saveas_label");
    get(aSaveAsLB, "saveas");
    get(aODFWarningFI, "odfwarning_image");
    get(aODFWarningFT, "odfwarning_label");


    aODFVersionLB->SetEntryData(0, reinterpret_cast<void*>(2         )); // 1.0/1.1
    aODFVersionLB->SetEntryData(1, reinterpret_cast<void*>(4         )); // 1.2
    aODFVersionLB->SetEntryData(2, reinterpret_cast<void*>(8         )); // 1.2 Extended (compatibility mode)
    aODFVersionLB->SetEntryData(3, reinterpret_cast<void*>(0x7fffffff)); // 1.2 Extended (recommended)

    aDocTypeLB->SetEntryData(0, reinterpret_cast<void*>(APP_WRITER)       );
    aDocTypeLB->SetEntryData(1, reinterpret_cast<void*>(APP_WRITER_WEB)   );
    aDocTypeLB->SetEntryData(2, reinterpret_cast<void*>(APP_WRITER_GLOBAL));
    aDocTypeLB->SetEntryData(3, reinterpret_cast<void*>(APP_CALC)         );
    aDocTypeLB->SetEntryData(4, reinterpret_cast<void*>(APP_IMPRESS)      );
    aDocTypeLB->SetEntryData(5, reinterpret_cast<void*>(APP_DRAW)         );
    aDocTypeLB->SetEntryData(6, reinterpret_cast<void*>(APP_MATH)         );

    aAutoSaveCB->SetClickHdl( LINK( this, SvxSaveTabPage, AutoClickHdl_Impl ) );
    aAutoSaveEdit->SetMaxTextLen( 2 );

    SvtModuleOptions aModuleOpt;
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
    {
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_MATH) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_MATH) ));
    }
    else
    {
        pImpl->aDefaultArr[APP_MATH] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::MATH);
        pImpl->aDefaultReadonlyArr[APP_MATH] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::MATH);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
    {
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_DRAW) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_DRAW) ));
    }
    else
    {
        pImpl->aDefaultArr[APP_DRAW] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::DRAW);
        pImpl->aDefaultReadonlyArr[APP_DRAW] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::DRAW);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_IMPRESS) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_IMPRESS) ));
    }
    else
    {
        pImpl->aDefaultArr[APP_IMPRESS] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::IMPRESS);
        pImpl->aDefaultReadonlyArr[APP_IMPRESS] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::IMPRESS);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_CALC) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_CALC) ));
    }
    else
    {
        pImpl->aDefaultArr[APP_CALC] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::EFactory::CALC);
        pImpl->aDefaultReadonlyArr[APP_CALC] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::EFactory::CALC);
    }

    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
    {
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER) ));
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER_WEB) ));
        aSaveAsLB->RemoveEntry(aSaveAsLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER_GLOBAL) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER_WEB) ));
        aDocTypeLB->RemoveEntry(aDocTypeLB->GetEntryPos( reinterpret_cast<void*>(APP_WRITER_GLOBAL) ));
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

    Link<ListBox&,void> aLink = LINK( this, SvxSaveTabPage, ODFVersionHdl_Impl );
    aODFVersionLB->SetSelectHdl( aLink );
    aLink = LINK( this, SvxSaveTabPage, FilterHdl_Impl );
    aDocTypeLB->SetSelectHdl( aLink );
    aSaveAsLB->SetSelectHdl( aLink );

    DetectHiddenControls();
}


SvxSaveTabPage::~SvxSaveTabPage()
{
    disposeOnce();
}

void SvxSaveTabPage::dispose()
{
    pImpl.reset();
    aLoadUserSettingsCB.clear();
    aLoadDocPrinterCB.clear();
    aDocInfoCB.clear();
    aBackupCB.clear();
    aAutoSaveCB.clear();
    aAutoSaveEdit.clear();
    aMinuteFT.clear();
    aUserAutoSaveCB.clear();
    aRelativeFsysCB.clear();
    aRelativeInetCB.clear();
    aODFVersionLB.clear();
    aWarnAlienFormatCB.clear();
    aDocTypeLB.clear();
    aSaveAsFT.clear();
    aSaveAsLB.clear();
    aODFWarningFI.clear();
    aODFWarningFT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxSaveTabPage::Create( vcl::Window* pParent,
                                           const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxSaveTabPage>::Create( pParent, *rAttrSet );
}

void SvxSaveTabPage::DetectHiddenControls()
{
    SvtOptionsDialogOptions aOptionsDlgOpt;

    if ( aOptionsDlgOpt.IsOptionHidden( "Backup", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "Backup"
        aBackupCB->Hide();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( "AutoSave", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "AutoSave"
        aAutoSaveCB->Hide();
        aAutoSaveEdit->Hide();
        aMinuteFT->Hide();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( "UserAutoSave", CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "UserAutoSave"
        aUserAutoSaveCB->Hide();
    }

}

bool SvxSaveTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    SvtSaveOptions aSaveOpt;
    if(aLoadUserSettingsCB->IsValueChangedFromSaved())
    {
        aSaveOpt.SetLoadUserSettings(aLoadUserSettingsCB->IsChecked());
    }

    if ( aLoadDocPrinterCB->IsValueChangedFromSaved() )
        aSaveOpt.SetLoadDocumentPrinter( aLoadDocPrinterCB->IsChecked() );

    if ( aODFVersionLB->IsValueChangedFromSaved() )
    {
        sal_IntPtr nVersion = sal_IntPtr( aODFVersionLB->GetSelectEntryData() );
        aSaveOpt.SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion( nVersion ) );
    }

    if ( aDocInfoCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_DOCINFO ),
                               aDocInfoCB->IsChecked() ) );
        bModified = true;
    }

    if ( aBackupCB->IsEnabled() && aBackupCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_BACKUP ),
                               aBackupCB->IsChecked() ) );
        bModified = true;
    }

    if ( aAutoSaveCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVE ),
                               aAutoSaveCB->IsChecked() ) );
        bModified = true;
    }
    if ( aWarnAlienFormatCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_WARNALIENFORMAT ),
                               aWarnAlienFormatCB->IsChecked() ) );
        bModified = true;
    }

    if ( aAutoSaveEdit->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxUInt16Item( GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                 (sal_uInt16)aAutoSaveEdit->GetValue() ) );
        bModified = true;
    }

    if ( aUserAutoSaveCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_USERAUTOSAVE ),
                               aUserAutoSaveCB->IsChecked() ) );
        bModified = true;
    }
    // save relatively
    if ( aRelativeFsysCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_SAVEREL_FSYS ),
                               aRelativeFsysCB->IsChecked() ) );
        bModified = true;
    }

    if ( aRelativeInetCB->IsValueChangedFromSaved() )
    {
        rSet->Put( SfxBoolItem( GetWhich( SID_SAVEREL_INET ),
                               aRelativeInetCB->IsChecked() ) );
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

bool isODFFormat( const OUString& sFilter )
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
    aLoadUserSettingsCB->Check(aSaveOpt.IsLoadUserSettings());
    aLoadUserSettingsCB->SaveValue();
    aLoadUserSettingsCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_USEUSERDATA));
    aLoadDocPrinterCB->Check( aSaveOpt.IsLoadDocumentPrinter() );
    aLoadDocPrinterCB->SaveValue();
    aLoadDocPrinterCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_LOADDOCPRINTER));

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
                for(sal_Int32 n = 0; n < aDocTypeLB->GetEntryCount(); n++)
                {
                    sal_IntPtr nData = reinterpret_cast<sal_IntPtr>(aDocTypeLB->GetEntryData(n));
                    OUString sCommand;
                    sCommand = "matchByDocumentService=%1:iflags=" +
                               OUString::number(static_cast<int>(SfxFilterFlags::IMPORT|SfxFilterFlags::EXPORT)) +
                               ":eflags=" +
                               OUString::number(static_cast<int>(SfxFilterFlags::NOTINFILEDLG)) +
                               ":default_first";
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
                    std::vector< sal_Bool > lAlienList;
                    std::vector< sal_Bool > lODFList;
                    while(xList->hasMoreElements())
                    {
                        SequenceAsHashMap aFilter(xList->nextElement());
                        OUString sFilter = aFilter.getUnpackedValueOrDefault("Name",OUString());
                        if (!sFilter.isEmpty())
                        {
                            SfxFilterFlags nFlags = static_cast<SfxFilterFlags>(aFilter.getUnpackedValueOrDefault("Flags",sal_Int32()));
                            lList.push_back(sFilter);
                            lAlienList.push_back(bool(nFlags & SfxFilterFlags::ALIEN));
                            lODFList.push_back( isODFFormat( sFilter ) );
                        }
                    }
                    pImpl->aFilterArr[nData] = comphelper::containerToSequence(lList);
                    pImpl->aAlienArr[nData] = comphelper::containerToSequence(lAlienList);
                    pImpl->aODFArr[nData] = comphelper::containerToSequence(lODFList);
                }
            }
            aDocTypeLB->SelectEntryPos(0);
            FilterHdl_Impl(*aDocTypeLB);
        }
        catch(Exception& e)
        {
            (void) e;
            OSL_FAIL(
                OUStringToOString(
                    "exception in FilterFactory access: " + e.Message,
                    RTL_TEXTENCODING_UTF8).
                getStr());
        }

        pImpl->bInitialized = true;
    }

    aDocInfoCB->Check(aSaveOpt.IsDocInfoSave());
    aDocInfoCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_DOCINFSAVE));

    aBackupCB->Check(aSaveOpt.IsBackup());
    aBackupCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_BACKUP));

    aAutoSaveCB->Check(aSaveOpt.IsAutoSave());
    aAutoSaveCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVE));

    aUserAutoSaveCB->Check(aSaveOpt.IsUserAutoSave());
    aUserAutoSaveCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_USERAUTOSAVE));

    aWarnAlienFormatCB->Check(aSaveOpt.IsWarnAlienFormat());
    aWarnAlienFormatCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_WARNALIENFORMAT));

    aAutoSaveEdit->SetValue(aSaveOpt.GetAutoSaveTime());
    aAutoSaveEdit->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVETIME));

    // save relatively
    aRelativeFsysCB->Check(aSaveOpt.IsSaveRelFSys());
    aRelativeFsysCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELFSYS));

    aRelativeInetCB->Check(aSaveOpt.IsSaveRelINet());
    aRelativeInetCB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELINET));

    void* pDefaultVersion = reinterpret_cast<void*>( aSaveOpt.GetODFDefaultVersion() );
    aODFVersionLB->SelectEntryPos( aODFVersionLB->GetEntryPos( pDefaultVersion ) );
    aODFVersionLB->Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_ODFDEFAULTVERSION));

    AutoClickHdl_Impl( aAutoSaveCB );
    ODFVersionHdl_Impl( *aODFVersionLB );

    aDocInfoCB->SaveValue();
    aBackupCB->SaveValue();
    aWarnAlienFormatCB->SaveValue();
    aAutoSaveCB->SaveValue();
    aAutoSaveEdit->SaveValue();

    aUserAutoSaveCB->SaveValue();

    aRelativeFsysCB->SaveValue();
    aRelativeInetCB->SaveValue();
    aODFVersionLB->SaveValue();
}


IMPL_LINK_TYPED( SvxSaveTabPage, AutoClickHdl_Impl, Button*, pBox, void )
{
    if ( pBox == aAutoSaveCB )
    {
        if ( aAutoSaveCB->IsChecked() )
        {
            aAutoSaveEdit->Enable();
            aMinuteFT->Enable();
            aUserAutoSaveCB->Enable();
        }
        else
        {
            aAutoSaveEdit->Disable();
            aMinuteFT->Disable();
            aUserAutoSaveCB->Disable();
        }
    }
}

static OUString lcl_ExtracUIName(const Sequence<PropertyValue> &rProperties)
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
                return sUIName;
        }
        else if (rName == "Name")
        {
            pPropVal->Value >>= sName;
        }
    }

    OSL_ENSURE( false, "Filter without UIName!" );

    return sName;
}

IMPL_LINK_TYPED( SvxSaveTabPage, FilterHdl_Impl, ListBox&, rBox, void )
{
    const sal_Int32 nCurPos = aDocTypeLB->GetSelectEntryPos();

    sal_IntPtr nData = -1;
    if(nCurPos < APP_COUNT)
        nData = reinterpret_cast<sal_IntPtr>( aDocTypeLB->GetEntryData(nCurPos) );

    if ( nData >= 0 && nData < APP_COUNT )
    {
        if(aDocTypeLB == &rBox)
        {
            aSaveAsLB->Clear();
            const OUString* pFilters = pImpl->aFilterArr[nData].getConstArray();
            if(!pImpl->aUIFilterArr[nData].getLength())
            {
                pImpl->aUIFilterArr[nData].realloc(pImpl->aFilterArr[nData].getLength());
                OUString* pUIFilters = pImpl->aUIFilterArr[nData].getArray();
                for(int nFilter = 0; nFilter < pImpl->aFilterArr[nData].getLength(); nFilter++)
                {
                    Any aProps = pImpl->xFact->getByName(pFilters[nFilter]);
                    Sequence<PropertyValue> aProperties;
                    aProps >>= aProperties;
                    pUIFilters[nFilter] = lcl_ExtracUIName(aProperties);
                }
            }
            const OUString* pUIFilters = pImpl->aUIFilterArr[nData].getConstArray();
            OUString sSelect;
            for(int i = 0; i < pImpl->aUIFilterArr[nData].getLength(); i++)
            {
                const sal_Int32 nEntryPos = aSaveAsLB->InsertEntry(pUIFilters[i]);
                if ( pImpl->aODFArr[nData][i] )
                    aSaveAsLB->SetEntryData( nEntryPos, static_cast<void*>(pImpl.get()) );
                if(pFilters[i] == pImpl->aDefaultArr[nData])
                    sSelect = pUIFilters[i];
            }
            if(!sSelect.isEmpty())
            {
                aSaveAsLB->SelectEntry(sSelect);
            }

            aSaveAsFT->Enable(!pImpl->aDefaultReadonlyArr[nData]);
            aSaveAsLB->Enable(!pImpl->aDefaultReadonlyArr[nData]);
        }
        else
        {
            OUString sSelect = rBox.GetSelectEntry();
            const OUString* pFilters = pImpl->aFilterArr[nData].getConstArray();
            OUString* pUIFilters = pImpl->aUIFilterArr[nData].getArray();
            for(int i = 0; i < pImpl->aUIFilterArr[nData].getLength(); i++)
                if(pUIFilters[i] == sSelect)
                {
                    sSelect = pFilters[i];
                    break;
                }

            pImpl->aDefaultArr[nData] = sSelect;
        }
    }

    ODFVersionHdl_Impl( *aSaveAsLB );
};

IMPL_LINK_NOARG_TYPED(SvxSaveTabPage, ODFVersionHdl_Impl, ListBox&, void)
{
    sal_IntPtr nVersion = sal_IntPtr( aODFVersionLB->GetSelectEntryData() );
    bool bShown = SvtSaveOptions::ODFDefaultVersion( nVersion ) != SvtSaveOptions::ODFVER_LATEST;
    if ( bShown )
    {
        bool bHasODFFormat = false;
        const sal_Int32 nCount = aSaveAsLB->GetEntryCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            if ( aSaveAsLB->GetEntryData(i) != nullptr )
            {
                bHasODFFormat = true;
                break;
            }
        }

        bShown = !bHasODFFormat
                || ( aSaveAsLB->GetSelectEntryData() != nullptr );
    }

    aODFWarningFI->Show( bShown );
    aODFWarningFT->Show( bShown );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
