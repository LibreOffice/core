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

#include <svl/zforlist.hxx>
#include <svtools/grfmgr.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/lingucfg.hxx>
#include <svl/szitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/imgmgr.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/fontoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/printwarningoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <unotools/configitem.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <svtools/langtab.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/configmgr.hxx>
#include "cuioptgenrl.hxx"
#include "optpath.hxx"
#include "optsave.hxx"
#include "optlingu.hxx"
#include <svx/xpool.hxx>
#include <svx/dlgutil.hxx>
#include "cuitabarea.hxx"
#include <cuires.hrc>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include <dialmgr.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/searchopt.hxx>
#include <sal/macros.h>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/office/Quickstart.hpp>

#include <vcl/svapp.hxx>

#include "optgdlg.hxx"
#include <svx/ofaitem.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/optionsdrawinglayer.hxx>

#include <config_vclplug.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::utl;

// class OfaMiscTabPage --------------------------------------------------

int OfaMiscTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( *pSet_ );
    return LEAVE_PAGE;
}

namespace
{
        static OUString impl_SystemFileOpenServiceName()
        {
            const OUString &rDesktopEnvironment = Application::GetDesktopEnvironment();

            if ( rDesktopEnvironment.equalsIgnoreAsciiCase("kde4") )
            {
                #if ENABLE_KDE4
                return OUString("com.sun.star.ui.dialogs.KDE4FilePicker" );
                #else
                return OUString();
                #endif
            }
            else if ( rDesktopEnvironment.equalsIgnoreAsciiCase("kde") )
            {
                #if ENABLE_KDE
                return OUString("com.sun.star.ui.dialogs.KDEFilePicker");
                #else
                return OUString();
                #endif
            }
            else if ( rDesktopEnvironment.equalsIgnoreAsciiCase("tde") )
            {
                #if ENABLE_TDE
                return OUString("com.sun.star.ui.dialogs.TDEFilePicker");
                #else
                return OUString();
                #endif
            }
            #if defined WNT
            return OUString("com.sun.star.ui.dialogs.SystemFilePicker");
            #elif defined MACOSX
            return OUString("com.sun.star.ui.dialogs.AquaFilePicker");
            #else
            return OUString();
            #endif
        }

        static bool lcl_HasSystemFilePicker()
        {
            if( Application::hasNativeFileSelection() )
                return true;

            // Otherwise fall-back on querying services
            bool bRet = false;
            Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();

            Reference< XContentEnumerationAccess > xEnumAccess( xFactory, UNO_QUERY );
            Reference< XSet > xSet( xFactory, UNO_QUERY );

            if ( ! xEnumAccess.is() || ! xSet.is() )
                return bRet;

            try
            {
                OUString aFileService = impl_SystemFileOpenServiceName();
                Reference< XEnumeration > xEnum = xEnumAccess->createContentEnumeration( aFileService );
                if ( xEnum.is() && xEnum->hasMoreElements() )
                    bRet = true;
            }
            catch (const IllegalArgumentException&)
            {
            }
            catch (const ElementExistException&)
            {
            }
            return bRet;
        }
}

// -----------------------------------------------------------------------

OfaMiscTabPage::OfaMiscTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptGeneralPage", "cui/ui/optgeneralpage.ui", rSet)
{
    get(m_pToolTipsCB, "tooltips");
    get(m_pExtHelpCB, "exthelp");
    if (!lcl_HasSystemFilePicker())
        get<VclContainer>("filedlgframe")->Hide();
#if !defined(MACOSX) && ! ENABLE_GTK
    get<VclContainer>("printdlgframe")->Hide();
#endif
    get(m_pFileDlgCB, "filedlg");
    get(m_pPrintDlgCB, "printdlg");
    get(m_pDocStatusCB, "docstatus");
    get(m_pSaveAlwaysCB, "savealways");
    get(m_pYearFrame, "yearframe");
    get(m_pYearValueField, "year");
    get(m_pToYearFT, "toyear");

    if (m_pFileDlgCB->IsVisible() && SvtMiscOptions().IsUseSystemFileDialogReadOnly())
    {
        m_pFileDlgROImage->Show();
        m_pFileDlgCB->Disable();
    }

    m_aStrDateInfo = m_pToYearFT->GetText();
    m_pYearValueField->SetModifyHdl( LINK( this, OfaMiscTabPage, TwoFigureHdl ) );
    Link aLink = LINK( this, OfaMiscTabPage, TwoFigureConfigHdl );
    m_pYearValueField->SetDownHdl( aLink );
    m_pYearValueField->SetUpHdl( aLink );
    m_pYearValueField->SetLoseFocusHdl( aLink );
    m_pYearValueField->SetFirstHdl( aLink );
    TwoFigureConfigHdl(m_pYearValueField);

    SetExchangeSupport();

    aLink = LINK( this, OfaMiscTabPage, HelpCheckHdl_Impl );
    m_pToolTipsCB->SetClickHdl( aLink );
}

// -----------------------------------------------------------------------

OfaMiscTabPage::~OfaMiscTabPage()
{
}

// -----------------------------------------------------------------------

SfxTabPage* OfaMiscTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaMiscTabPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------

sal_Bool OfaMiscTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    SvtHelpOptions aHelpOptions;
    sal_Bool bChecked = m_pToolTipsCB->IsChecked();
    if ( bChecked != m_pToolTipsCB->GetSavedValue() )
        aHelpOptions.SetHelpTips( bChecked );
    bChecked = ( m_pExtHelpCB->IsChecked() && m_pToolTipsCB->IsChecked() );
    if ( bChecked != m_pExtHelpCB->GetSavedValue() )
        aHelpOptions.SetExtendedHelp( bChecked );

    if ( m_pFileDlgCB->IsChecked() != m_pFileDlgCB->GetSavedValue() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetUseSystemFileDialog( !m_pFileDlgCB->IsChecked() );
        bModified = sal_True;
    }

    if ( m_pPrintDlgCB->IsChecked() != m_pPrintDlgCB->GetSavedValue() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetUseSystemPrintDialog( !m_pPrintDlgCB->IsChecked() );
        bModified = sal_True;
    }

    if ( m_pDocStatusCB->IsChecked() != m_pDocStatusCB->GetSavedValue() )
    {
        SvtPrintWarningOptions aPrintOptions;
        aPrintOptions.SetModifyDocumentOnPrintingAllowed( m_pDocStatusCB->IsChecked() );
        bModified = sal_True;
    }

    if ( m_pSaveAlwaysCB->IsChecked() != m_pSaveAlwaysCB->GetSavedValue() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetSaveAlwaysAllowed( m_pSaveAlwaysCB->IsChecked() );
        bModified = sal_True;
    }

    const SfxUInt16Item* pUInt16Item =
        PTR_CAST( SfxUInt16Item, GetOldItem( rSet, SID_ATTR_YEAR2000 ) );
    sal_uInt16 nNum = (sal_uInt16)m_pYearValueField->GetText().toInt32();
    if ( pUInt16Item && pUInt16Item->GetValue() != nNum )
    {
        bModified = sal_True;
        rSet.Put( SfxUInt16Item( SID_ATTR_YEAR2000, nNum ) );
    }

    return bModified;
}

// -----------------------------------------------------------------------

void OfaMiscTabPage::Reset( const SfxItemSet& rSet )
{
    SvtHelpOptions aHelpOptions;
    m_pToolTipsCB->Check( aHelpOptions.IsHelpTips() );
    m_pExtHelpCB->Check( aHelpOptions.IsHelpTips() && aHelpOptions.IsExtendedHelp() );

    m_pToolTipsCB->SaveValue();
    m_pExtHelpCB->SaveValue();

    SvtMiscOptions aMiscOpt;
    m_pFileDlgCB->Check( !aMiscOpt.UseSystemFileDialog() );
    m_pFileDlgCB->SaveValue();
    m_pPrintDlgCB->Check( !aMiscOpt.UseSystemPrintDialog() );
    m_pPrintDlgCB->SaveValue();
    m_pSaveAlwaysCB->Check( aMiscOpt.IsSaveAlwaysAllowed() );
    m_pSaveAlwaysCB->SaveValue();

    SvtPrintWarningOptions aPrintOptions;
    m_pDocStatusCB->Check(aPrintOptions.IsModifyDocumentOnPrintingAllowed());
    m_pDocStatusCB->SaveValue();

    const SfxPoolItem* pItem = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, sal_False, &pItem ) )
    {
        m_pYearValueField->SetValue( ((SfxUInt16Item*)pItem)->GetValue() );
        TwoFigureConfigHdl(m_pYearValueField);
    }
    else
    {
        m_pYearFrame->Enable(sal_False);
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, TwoFigureHdl, NumericField*, pEd )
{
    (void)pEd;

    OUString aOutput( m_aStrDateInfo );
    OUString aStr( m_pYearValueField->GetText() );
    OUString sSep( SvtSysLocale().GetLocaleData().getNumThousandSep() );
    sal_Int32 nIndex = 0;
    while ((nIndex = aStr.indexOf( sSep, nIndex)) != -1)
        aStr = aStr.replaceAt( nIndex, sSep.getLength(), "");
    sal_Int32 nNum = aStr.toInt32();
    if ( aStr.getLength() != 4 || nNum < m_pYearValueField->GetMin() || nNum > m_pYearValueField->GetMax() )
        aOutput += "????";
    else
    {
        nNum += 99;
        aOutput += OUString::number( nNum );
    }
    m_pToYearFT->SetText( aOutput );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMiscTabPage, TwoFigureConfigHdl, NumericField*, pEd )
{
    sal_Int64 nNum = m_pYearValueField->GetValue();
    OUString aOutput(OUString::number(nNum));
    m_pYearValueField->SetText(aOutput);
    m_pYearValueField->SetSelection( Selection( 0, aOutput.getLength() ) );
    TwoFigureHdl( pEd );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(OfaMiscTabPage, HelpCheckHdl_Impl)
{
    m_pExtHelpCB->Enable( m_pToolTipsCB->IsChecked() );
    return 0;
}

// -----------------------------------------------------------------------

class CanvasSettings
{
public:
    CanvasSettings();

    sal_Bool    IsHardwareAccelerationEnabled() const;
    sal_Bool    IsHardwareAccelerationAvailable() const;
    void    EnabledHardwareAcceleration( sal_Bool _bEnabled ) const;

private:
    typedef std::vector< std::pair<OUString,Sequence<OUString> > > ServiceVector;

    Reference<XNameAccess> mxForceFlagNameAccess;
    ServiceVector          maAvailableImplementations;
    mutable sal_Bool           mbHWAccelAvailable;
    mutable sal_Bool           mbHWAccelChecked;
};

// -------------------------------------------------------------------
CanvasSettings::CanvasSettings() :
    mxForceFlagNameAccess(),
    mbHWAccelAvailable(sal_False),
    mbHWAccelChecked(sal_False)
{
    try
    {
        Reference<XMultiServiceFactory> xConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));

        Any propValue(
            makeAny( NamedValue(
                         OUString("nodepath"),
                         makeAny( OUString("/org.openoffice.Office.Canvas") ) ) ) );

        mxForceFlagNameAccess.set(
            xConfigProvider->createInstanceWithArguments(
                OUString("com.sun.star.configuration.ConfigurationUpdateAccess"),
                Sequence<Any>( &propValue, 1 ) ),
            UNO_QUERY_THROW );

        propValue = makeAny(
            NamedValue(
                OUString("nodepath"),
                makeAny( OUString("/org.openoffice.Office.Canvas/CanvasServiceList") ) ) );

        Reference<XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUString("com.sun.star.configuration.ConfigurationAccess"),
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
        Reference<XHierarchicalNameAccess> xHierarchicalNameAccess(
            xNameAccess, UNO_QUERY_THROW);

        Sequence<OUString> serviceNames = xNameAccess->getElementNames();
        const OUString* pCurr = serviceNames.getConstArray();
        const OUString* const pEnd = pCurr + serviceNames.getLength();
        while( pCurr != pEnd )
        {
            Reference<XNameAccess> xEntryNameAccess(
                xHierarchicalNameAccess->getByHierarchicalName(*pCurr),
                UNO_QUERY );

            if( xEntryNameAccess.is() )
            {
                Sequence<OUString> preferredImplementations;
                if( (xEntryNameAccess->getByName("PreferredImplementations") >>= preferredImplementations) )
                    maAvailableImplementations.push_back( std::make_pair(*pCurr,preferredImplementations) );
            }

            ++pCurr;
        }
    }
    catch (const Exception&)
    {
    }
}

// -------------------------------------------------------------------
sal_Bool CanvasSettings::IsHardwareAccelerationAvailable() const
{
    if( !mbHWAccelChecked )
    {
        mbHWAccelChecked = true;

        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();

        // check whether any of the service lists has an
        // implementation that presents the "HardwareAcceleration" property
        ServiceVector::const_iterator       aCurr=maAvailableImplementations.begin();
        const ServiceVector::const_iterator aEnd=maAvailableImplementations.end();
        while( aCurr != aEnd )
        {
            const OUString* pCurrImpl = aCurr->second.getConstArray();
            const OUString* const pEndImpl = pCurrImpl + aCurr->second.getLength();

            while( pCurrImpl != pEndImpl )
            {
                try
                {
                    Reference<XPropertySet> xPropSet( xFactory->createInstance(
                                                          pCurrImpl->trim() ),
                                                      UNO_QUERY_THROW );
                    bool bHasAccel(false);
                    if( (xPropSet->getPropertyValue("HardwareAcceleration") >>= bHasAccel) )
                        if( bHasAccel )
                        {
                            mbHWAccelAvailable = true;
                            return mbHWAccelAvailable;
                        }
                }
                catch (const Exception&)
                {
                }

                ++pCurrImpl;
            }

            ++aCurr;
        }
    }

    return mbHWAccelAvailable;
}

// -------------------------------------------------------------------
sal_Bool CanvasSettings::IsHardwareAccelerationEnabled() const
{
    bool bForceLastEntry(false);
    if( !mxForceFlagNameAccess.is() )
        return true;

    if( !(mxForceFlagNameAccess->getByName("ForceSafeServiceImpl") >>= bForceLastEntry) )
        return true;

    return !bForceLastEntry;
}

// -------------------------------------------------------------------
void CanvasSettings::EnabledHardwareAcceleration( sal_Bool _bEnabled ) const
{
    Reference< XNameReplace > xNameReplace(
        mxForceFlagNameAccess, UNO_QUERY );

    if( !xNameReplace.is() )
        return;

    xNameReplace->replaceByName( OUString("ForceSafeServiceImpl"),
                                 makeAny(!_bEnabled) );

    Reference< XChangesBatch > xChangesBatch(
        mxForceFlagNameAccess, UNO_QUERY );

    if( !xChangesBatch.is() )
        return;

    xChangesBatch->commitChanges();
}

// class OfaViewTabPage --------------------------------------------------

OfaViewTabPage::OfaViewTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptViewPage", "cui/ui/optviewpage.ui", rSet)
    , nSizeLB_InitialSelection(0)
    , nStyleLB_InitialSelection(0)
    , pAppearanceCfg(new SvtTabAppearanceCfg)
    , pCanvasSettings(new CanvasSettings)
    , mpDrawinglayerOpt(new SvtOptionsDrawinglayer)
{
    get(m_pWindowSizeMF, "windowsize");
    get(m_pIconSizeLB, "iconsize");
    get(m_pIconStyleLB, "iconstyle");
    get(m_pSystemFont, "systemfont");
    get(m_pFontAntiAliasing, "aafont");
    get(m_pAAPointLimitLabel, "aafrom");
    get(m_pAAPointLimit, "aanf");
    get(m_pMenuIconsLB, "menuicons");
    get(m_pFontShowCB, "showfontpreview");
    get(m_pFontHistoryCB, "showfonthistory");
    get(m_pUseHardwareAccell, "useaccel");
    get(m_pUseAntiAliase, "useaa");
    get(m_pMousePosLB, "mousepos");
    get(m_pMouseMiddleLB, "mousemiddle");
    // #i97672#
    get(m_pSelectionCB, "trans");
    get(m_pSelectionMF, "transmf");

#if defined( UNX )
    m_pFontAntiAliasing->SetToggleHdl( LINK( this, OfaViewTabPage, OnAntialiasingToggled ) );
#else
    // on this platform, we do not have the anti aliasing options
    m_pFontAntiAliasing->Hide();
    m_pAAPointLimitLabel->Hide();
    m_pAAPointLimit->Hide();

#endif

    // #i97672#
    m_pSelectionCB->SetToggleHdl( LINK( this, OfaViewTabPage, OnSelectionToggled ) );

    if( ! Application::ValidateSystemFont() )
    {
        m_pSystemFont->Check(false);
        m_pSystemFont->Enable(false);
    }

    const StyleSettings& aStyleSettings = Application::GetSettings().GetStyleSettings();

    // remove non-installed icon themes
    if( m_pIconStyleLB->GetEntryCount() == STYLE_SYMBOLS_THEMES_MAX )
    {
        // do not check 0th item == auto; it is not a real theme
        aIconStyleItemId[0] = 0;
        sal_uLong nItem = 1;
        for ( sal_uLong n=0; ++n < STYLE_SYMBOLS_THEMES_MAX; )
        {
            if ( aStyleSettings.CheckSymbolStyle( n ) )
            {
                // existing style => save the item id
                aIconStyleItemId[n] = nItem++;
            }
            else
            {
                // non-existing style => remove item;
                m_pIconStyleLB->RemoveEntry( nItem );
                aIconStyleItemId[n] = 0;
            }
        }
    }

    // add real theme name to 'auto' theme, e.g. 'auto' => 'auto (classic)'
    if( m_pIconStyleLB->GetEntryCount() > 1 )
    {
        OUString aAutoStr( m_pIconStyleLB->GetEntry( 0 ) );

        aAutoStr += " (";

        // prefer the icon style set by the desktop native widgets modules
        sal_uLong nAutoStyle = aStyleSettings.GetPreferredSymbolsStyle();
        // fallback to the statically defined values
        if ( nAutoStyle == STYLE_SYMBOLS_AUTO || !aIconStyleItemId[nAutoStyle] )
            nAutoStyle = aStyleSettings.GetAutoSymbolsStyle();
        if ( aIconStyleItemId[nAutoStyle] )
            aAutoStr += m_pIconStyleLB->GetEntry( aIconStyleItemId[nAutoStyle] );

        m_pIconStyleLB->RemoveEntry( 0 );
        m_pIconStyleLB->InsertEntry( aAutoStr += OUString(")" ), 0 );
        // separate auto and other icon themes
        m_pIconStyleLB->SetSeparatorPos( 0 );
    }
}

OfaViewTabPage::~OfaViewTabPage()
{
    delete mpDrawinglayerOpt;
    delete pCanvasSettings;
    delete pAppearanceCfg;
}

#if defined( UNX )
IMPL_LINK( OfaViewTabPage, OnAntialiasingToggled, void*, NOTINTERESTEDIN )
{
    (void)NOTINTERESTEDIN;

    sal_Bool bAAEnabled = m_pFontAntiAliasing->IsChecked();

    m_pAAPointLimitLabel->Enable( bAAEnabled );
    m_pAAPointLimit->Enable( bAAEnabled );

    return 0L;
}
#endif

// #i97672#
IMPL_LINK( OfaViewTabPage, OnSelectionToggled, void*, NOTINTERESTEDIN )
{
    (void)NOTINTERESTEDIN;
    const bool bSelectionEnabled(m_pSelectionCB->IsChecked());
    m_pSelectionMF->Enable(bSelectionEnabled);
    return 0;
}

SfxTabPage* OfaViewTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaViewTabPage(pParent, rAttrSet);
}

sal_Bool OfaViewTabPage::FillItemSet( SfxItemSet& )
{
    SvtFontOptions aFontOpt;
    SvtMenuOptions aMenuOpt;

    sal_Bool bModified = sal_False;
    sal_Bool bMenuOptModified = sal_False;
    bool bRepaintWindows(false);

    SvtMiscOptions aMiscOptions;
    sal_uInt16 nSizeLB_NewSelection = m_pIconSizeLB->GetSelectEntryPos();
    if( nSizeLB_InitialSelection != nSizeLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        sal_Int16 eSet = SFX_SYMBOLS_SIZE_AUTO;
        switch( nSizeLB_NewSelection )
        {
            case 0: eSet = SFX_SYMBOLS_SIZE_AUTO;  break;
            case 1: eSet = SFX_SYMBOLS_SIZE_SMALL; break;
            case 2: eSet = SFX_SYMBOLS_SIZE_LARGE; break;
            default:
                OSL_FAIL( "OfaViewTabPage::FillItemSet(): This state of m_pIconSizeLB should not be possible!" );
        }
        aMiscOptions.SetSymbolsSize( eSet );
    }

    sal_uInt16 nStyleLB_NewSelection = m_pIconStyleLB->GetSelectEntryPos();
    if( nStyleLB_InitialSelection != nStyleLB_NewSelection )
    {
        // find the style name in the aIconStyleItemId table
        // items from the non-installed icon themes were removed
        for ( sal_uLong n=0; n < STYLE_SYMBOLS_THEMES_MAX; n++ )
        {
            if ( aIconStyleItemId[n] == nStyleLB_NewSelection )
            {
                aMiscOptions.SetSymbolsStyle( n );
                n = STYLE_SYMBOLS_THEMES_MAX;
            }
        }
    }

    sal_Bool bAppearanceChanged = sal_False;


    // Screen Scaling
    sal_uInt16 nOldScale = pAppearanceCfg->GetScaleFactor();
    sal_uInt16 nNewScale = (sal_uInt16)m_pWindowSizeMF->GetValue();

    if ( nNewScale != nOldScale )
    {
        pAppearanceCfg->SetScaleFactor(nNewScale);
        bAppearanceChanged = sal_True;
    }

    // Mouse Snap Mode
    short eOldSnap = pAppearanceCfg->GetSnapMode();
    short eNewSnap = m_pMousePosLB->GetSelectEntryPos();
    if(eNewSnap > 2)
        eNewSnap = 2;

    if ( eNewSnap != eOldSnap )
    {
        pAppearanceCfg->SetSnapMode(eNewSnap );
        bAppearanceChanged = sal_True;
    }

    // Middle Mouse Button
    short eOldMiddleMouse = pAppearanceCfg->GetMiddleMouseButton();
    short eNewMiddleMouse = m_pMouseMiddleLB->GetSelectEntryPos();
    if(eNewMiddleMouse > 2)
        eNewMiddleMouse = 2;

    if ( eNewMiddleMouse != eOldMiddleMouse )
    {
        pAppearanceCfg->SetMiddleMouseButton( eNewMiddleMouse );
        bAppearanceChanged = sal_True;
    }

#if defined( UNX )
    if ( m_pFontAntiAliasing->IsChecked() != m_pFontAntiAliasing->GetSavedValue() )
    {
        pAppearanceCfg->SetFontAntiAliasing( m_pFontAntiAliasing->IsChecked() );
        bAppearanceChanged = sal_True;
    }

    if ( m_pAAPointLimit->GetValue() != m_pAAPointLimit->GetSavedValue().toInt32() )
    {
        pAppearanceCfg->SetFontAntialiasingMinPixelHeight( m_pAAPointLimit->GetValue() );
        bAppearanceChanged = sal_True;
    }
#endif

    if ( m_pFontShowCB->IsChecked() != m_pFontShowCB->GetSavedValue() )
    {
        aFontOpt.EnableFontWYSIWYG( m_pFontShowCB->IsChecked() );
        bModified = sal_True;
    }

    if(m_pMenuIconsLB->GetSelectEntryPos() != m_pMenuIconsLB->GetSavedValue())
    {
        aMenuOpt.SetMenuIconsState( m_pMenuIconsLB->GetSelectEntryPos() == 0 ? 2 : m_pMenuIconsLB->GetSelectEntryPos() - 1);
        bModified = sal_True;
        bMenuOptModified = sal_True;
        bAppearanceChanged = sal_True;
    }

    if ( m_pFontHistoryCB->IsChecked() != m_pFontHistoryCB->GetSavedValue() )
    {
        aFontOpt.EnableFontHistory( m_pFontHistoryCB->IsChecked() );
        bModified = sal_True;
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if(m_pUseHardwareAccell->IsEnabled())
    {
        if(m_pUseHardwareAccell->IsChecked() != m_pUseHardwareAccell->GetSavedValue())
        {
            pCanvasSettings->EnabledHardwareAcceleration(m_pUseHardwareAccell->IsChecked());
            bModified = sal_True;
        }
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if(m_pUseAntiAliase->IsEnabled())
    {
        if(m_pUseAntiAliase->IsChecked() != mpDrawinglayerOpt->IsAntiAliasing())
        {
            mpDrawinglayerOpt->SetAntiAliasing(m_pUseAntiAliase->IsChecked());
            bModified = sal_True;
            bRepaintWindows = true;
        }
    }

    // #i97672#
    if(m_pSelectionCB->IsEnabled())
    {
        const bool bNewSelection(m_pSelectionCB->IsChecked());
        const sal_uInt16 nNewTransparence((sal_uInt16)m_pSelectionMF->GetValue());

        if(bNewSelection != (bool)mpDrawinglayerOpt->IsTransparentSelection())
        {
            mpDrawinglayerOpt->SetTransparentSelection(m_pSelectionCB->IsChecked());
            bModified = sal_True;
            bRepaintWindows = true;
        }

        // #i104150# even read the value when m_pSelectionMF is disabled; it may have been
        // modified by enabling-modify-disabling by the user
        if(nNewTransparence != mpDrawinglayerOpt->GetTransparentSelectionPercent())
        {
            mpDrawinglayerOpt->SetTransparentSelectionPercent(nNewTransparence);
            bModified = sal_True;
            bRepaintWindows = true;
        }
    }

    SvtAccessibilityOptions     aAccessibilityOptions;
    if( aAccessibilityOptions.GetIsSystemFont() != m_pSystemFont->IsChecked() &&
        m_pSystemFont->IsEnabled() )
    {
        aAccessibilityOptions.SetIsSystemFont( m_pSystemFont->IsChecked() );
        bModified = sal_True;
        bMenuOptModified = sal_True;
    }

    if( bMenuOptModified )
    {
        // Set changed settings to the application instance
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        if( m_pSystemFont->IsEnabled() )
            aStyleSettings.SetUseSystemUIFonts( m_pSystemFont->IsChecked() );
        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings( aAllSettings );
        Application::SetSettings(aAllSettings);
    }

    if ( bAppearanceChanged )
    {
        pAppearanceCfg->Commit();
        pAppearanceCfg->SetApplicationDefaults ( GetpApp() );
    }

    if(bRepaintWindows)
    {
        Window* pAppWindow = Application::GetFirstTopLevelWindow();

        while(pAppWindow)
        {
            pAppWindow->Invalidate();
            pAppWindow = Application::GetNextTopLevelWindow(pAppWindow);
        }
    }

    return bModified;
}

void OfaViewTabPage::Reset( const SfxItemSet& )
{
    SvtMiscOptions aMiscOptions;

    if( aMiscOptions.GetSymbolsSize() != SFX_SYMBOLS_SIZE_AUTO )
        nSizeLB_InitialSelection = ( aMiscOptions.AreCurrentSymbolsLarge() )? 2 : 1;
    m_pIconSizeLB->SelectEntryPos( nSizeLB_InitialSelection );
    m_pIconSizeLB->SaveValue();

    if( aMiscOptions.GetSymbolsStyle() != STYLE_SYMBOLS_AUTO )
        nStyleLB_InitialSelection = aIconStyleItemId[aMiscOptions.GetCurrentSymbolsStyle()];

    m_pIconStyleLB->SelectEntryPos( nStyleLB_InitialSelection );
    m_pIconStyleLB->SaveValue();

    if( m_pSystemFont->IsEnabled() )
    {
        SvtAccessibilityOptions aAccessibilityOptions;
        m_pSystemFont->Check( aAccessibilityOptions.GetIsSystemFont() );
    }

    // Screen Scaling
    m_pWindowSizeMF->SetValue ( pAppearanceCfg->GetScaleFactor() );
    // Mouse Snap
    m_pMousePosLB->SelectEntryPos(pAppearanceCfg->GetSnapMode());
    m_pMousePosLB->SaveValue();

    // Mouse Snap
    m_pMouseMiddleLB->SelectEntryPos(pAppearanceCfg->GetMiddleMouseButton());
    m_pMouseMiddleLB->SaveValue();

#if defined( UNX )
    m_pFontAntiAliasing->Check( pAppearanceCfg->IsFontAntiAliasing() );
    m_pAAPointLimit->SetValue( pAppearanceCfg->GetFontAntialiasingMinPixelHeight() );
#endif

    // WorkingSet
    SvtFontOptions aFontOpt;
    m_pFontShowCB->Check( aFontOpt.IsFontWYSIWYGEnabled() );
    SvtMenuOptions aMenuOpt;
    m_pMenuIconsLB->SelectEntryPos(aMenuOpt.GetMenuIconsState() == 2 ? 0 : aMenuOpt.GetMenuIconsState() + 1);
    m_pMenuIconsLB->SaveValue();
    m_pFontHistoryCB->Check( aFontOpt.IsFontHistoryEnabled() );

    { // #i95644# HW accel (unified to disable mechanism)
        if(pCanvasSettings->IsHardwareAccelerationAvailable())
        {
            m_pUseHardwareAccell->Check(pCanvasSettings->IsHardwareAccelerationEnabled());
        }
        else
        {
            m_pUseHardwareAccell->Check(false);
            m_pUseHardwareAccell->Disable();
        }

        m_pUseHardwareAccell->SaveValue();
    }

    { // #i95644# AntiAliasing
        if(mpDrawinglayerOpt->IsAAPossibleOnThisSystem())
        {
            m_pUseAntiAliase->Check(mpDrawinglayerOpt->IsAntiAliasing());
        }
        else
        {
            m_pUseAntiAliase->Check(false);
            m_pUseAntiAliase->Disable();
        }

        m_pUseAntiAliase->SaveValue();
    }

    {
        // #i97672# Selection
        // check if transparent selection is possible on this system
        const bool bTransparentSelectionPossible(
            !GetSettings().GetStyleSettings().GetHighContrastMode()
            && supportsOperation(OutDevSupport_TransparentRect));

        // enter values
        if(bTransparentSelectionPossible)
        {
            m_pSelectionCB->Check(mpDrawinglayerOpt->IsTransparentSelection());
        }
        else
        {
            m_pSelectionCB->Enable(false);
        }

        m_pSelectionMF->SetValue(mpDrawinglayerOpt->GetTransparentSelectionPercent());
        m_pSelectionMF->Enable(mpDrawinglayerOpt->IsTransparentSelection() && bTransparentSelectionPossible);
    }

#if defined( UNX )
    m_pFontAntiAliasing->SaveValue();
    m_pAAPointLimit->SaveValue();
#endif
    m_pFontShowCB->SaveValue();
    m_pFontHistoryCB->SaveValue();

#if defined( UNX )
    LINK( this, OfaViewTabPage, OnAntialiasingToggled ).Call( NULL );
#endif
}

struct LanguageConfig_Impl
{
    SvtLanguageOptions aLanguageOptions;
    SvtSysLocaleOptions aSysLocaleOptions;
    SvtLinguConfig aLinguConfig;
};

static sal_Bool bLanguageCurrentDoc_Impl = sal_False;

// some things we'll need...
static const OUString sAccessSrvc("com.sun.star.configuration.ConfigurationAccess");
static const OUString sAccessUpdSrvc("com.sun.star.configuration.ConfigurationUpdateAccess");
static const OUString sInstalledLocalesPath("org.openoffice.Setup/Office/InstalledLocales");
static OUString sUserLocalePath("org.openoffice.Office.Linguistic/General");
//static const OUString sUserLocalePath("org.openoffice.Office/Linguistic");
static const OUString sUserLocaleKey("UILocale");
static Sequence< OUString > seqInstalledLanguages;

static OUString lcl_getDatePatternsConfigString( const LocaleDataWrapper& rLocaleWrapper )
{
    Sequence< OUString > aDateAcceptancePatterns = rLocaleWrapper.getDateAcceptancePatterns();
    sal_Int32 nPatterns = aDateAcceptancePatterns.getLength();
    OUStringBuffer aBuf( nPatterns * 6 );   // 6 := length of Y-M-D;
    SAL_WARN_IF( !nPatterns, "cui.options", "No date acceptance pattern");
    if (nPatterns)
    {
        const OUString* pPatterns = aDateAcceptancePatterns.getConstArray();
        aBuf.append( pPatterns[0]);
        for (sal_Int32 i=1; i < nPatterns; ++i)
            aBuf.append(';').append( pPatterns[i]);
    }
    return aBuf.makeStringAndClear();
}

OfaLanguagesTabPage::OfaLanguagesTabPage( Window* pParent, const SfxItemSet& rSet ) :
      SfxTabPage( pParent,"OptLanguagesPage","cui/ui/optlanguagespage.ui", rSet ),
     pLangConfig(new LanguageConfig_Impl)
{
    get(m_pUserInterfaceLB, "userinterface");
    m_pUserInterfaceLB->SetStyle(m_pUserInterfaceLB->GetStyle() | WB_SORT);
    get(m_pLocaleSettingLB, "localesetting");
    m_pLocaleSettingLB->SetStyle(m_pLocaleSettingLB->GetStyle() | WB_SORT);
    get(m_pLocaleSettingFT, "localesettingFT");
    get(m_pDecimalSeparatorCB, "decimalseparator");
    get(m_pCurrencyFT, "defaultcurrency");
    get(m_pCurrencyLB, "currencylb");
    m_pCurrencyLB->SetStyle(m_pCurrencyLB->GetStyle() | WB_SORT);
    get(m_pDatePatternsFT,"dataaccpatterns");
    get(m_pDatePatternsED, "datepatterns");

    get(m_pWesternLanguageLB, "westernlanguage");
    m_pWesternLanguageLB->SetStyle(m_pWesternLanguageLB->GetStyle() | WB_SORT);
    get(m_pWesternLanguageFT, "western");
    get(m_pAsianLanguageLB, "asianlanguage");
    m_pAsianLanguageLB->SetStyle(m_pAsianLanguageLB->GetStyle() | WB_SORT);
    get(m_pComplexLanguageLB, "complexlanguage");
    m_pComplexLanguageLB->SetStyle(m_pComplexLanguageLB->GetStyle() | WB_SORT);
    get(m_pCurrentDocCB, "currentdoc");
    get(m_pAsianSupportCB, "asiansupport");
    get(m_pCTLSupportCB, "ctlsupport");
    get(m_pIgnoreLanguageChangeCB, "ignorelanguagechange");

    // initialize user interface language selection
    SvtLanguageTable* pLanguageTable = new SvtLanguageTable;
    const OUString aStr( pLanguageTable->GetString( LANGUAGE_SYSTEM ) );

    OUString aUILang = aStr +
                       " - " +
                       pLanguageTable->GetString( Application::GetSettings().GetUILanguageTag().getLanguageType(), true );

    m_pUserInterfaceLB->InsertEntry(aUILang);
    m_pUserInterfaceLB->SetEntryData(0, 0);
    m_pUserInterfaceLB->SelectEntryPos(0);
    try
    {
        Reference< XMultiServiceFactory > theConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        Reference< XNameAccess > theNameAccess;

        // find out which locales are currently installed and add them to the listbox
        theArgs[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(sInstalledLocalesPath)));
    theNameAccess = Reference< XNameAccess > (
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs ), UNO_QUERY_THROW );
        seqInstalledLanguages = theNameAccess->getElementNames();
        LanguageType aLang = LANGUAGE_DONTKNOW;
        for (sal_IntPtr i=0; i<seqInstalledLanguages.getLength(); i++)
        {
            aLang = LanguageTag(seqInstalledLanguages[i]).getLanguageType();
            if (aLang != LANGUAGE_DONTKNOW)
            {
                //sal_uInt16 p = m_pUserInterfaceLB->InsertLanguage(aLang);
                OUString aLangStr( pLanguageTable->GetString( aLang, true ) );
                sal_uInt16 p = m_pUserInterfaceLB->InsertEntry(aLangStr);
                m_pUserInterfaceLB->SetEntryData(p, (void*)(i+1));
            }
        }

        // find out whether the user has a specific locale specified
        Sequence< Any > theArgs2(1);
        theArgs2[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(sUserLocalePath)));
        theNameAccess = Reference< XNameAccess > (
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs2 ), UNO_QUERY_THROW );
        if (theNameAccess->hasByName(sUserLocaleKey))
            theNameAccess->getByName(sUserLocaleKey) >>= m_sUserLocaleValue;
        // select the user specified locale in the listbox
        if (!m_sUserLocaleValue.isEmpty())
        {
            sal_Int32 d = 0;
            for (sal_uInt16 i=0; i < m_pUserInterfaceLB->GetEntryCount(); i++)
            {
                d = (sal_Int32)(sal_IntPtr)m_pUserInterfaceLB->GetEntryData(i);
                if ( d > 0 && seqInstalledLanguages.getLength() > d-1 && seqInstalledLanguages[d-1].equals(m_sUserLocaleValue))
                    m_pUserInterfaceLB->SelectEntryPos(i);
            }
        }

    }
    catch (const Exception &e)
    {
        // we'll just leave the box in it's default setting and won't
        // even give it event handler...
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(aMsg.getStr());
    }

    m_pWesternLanguageLB->SetLanguageList( LANG_LIST_WESTERN | LANG_LIST_ONLY_KNOWN, sal_True,  sal_False, sal_True );
    m_pWesternLanguageLB->InsertDefaultLanguage( ::com::sun::star::i18n::ScriptType::LATIN );
    m_pAsianLanguageLB->SetLanguageList( LANG_LIST_CJK     | LANG_LIST_ONLY_KNOWN, sal_True,  sal_False, sal_True );
    m_pAsianLanguageLB->InsertDefaultLanguage( ::com::sun::star::i18n::ScriptType::ASIAN );
    m_pComplexLanguageLB->SetLanguageList( LANG_LIST_CTL     | LANG_LIST_ONLY_KNOWN, sal_True,  sal_False, sal_True );
    m_pComplexLanguageLB->InsertDefaultLanguage( ::com::sun::star::i18n::ScriptType::COMPLEX );

    m_pLocaleSettingLB->SetLanguageList( LANG_LIST_ALL     | LANG_LIST_ONLY_KNOWN, sal_False, sal_False, sal_False);
    m_pLocaleSettingLB->InsertSystemLanguage( );

    const NfCurrencyTable& rCurrTab = SvNumberFormatter::GetTheCurrencyTable();
    const NfCurrencyEntry& rCurr = SvNumberFormatter::GetCurrencyEntry( LANGUAGE_SYSTEM );
    // insert SYSTEM entry
    OUString aDefaultCurr = aStr + " - " + rCurr.GetBankSymbol();
    m_pCurrencyLB->InsertEntry( aDefaultCurr );
    // all currencies
    OUString aTwoSpace( "  " );
    sal_uInt16 nCurrCount = rCurrTab.size();
    // first entry is SYSTEM, skip it
    for ( sal_uInt16 j=1; j < nCurrCount; ++j )
    {
        const NfCurrencyEntry* pCurr = &rCurrTab[j];
        OUString aStr_ = pCurr->GetBankSymbol() +
                         aTwoSpace +
                         pCurr->GetSymbol();
        aStr_ = ApplyLreOrRleEmbedding( aStr_ ) +
                aTwoSpace +
                ApplyLreOrRleEmbedding( pLanguageTable->GetString( pCurr->GetLanguage() ) );
        sal_uInt16 nPos = m_pCurrencyLB->InsertEntry( aStr_ );
        m_pCurrencyLB->SetEntryData( nPos, (void*) pCurr );
    }
    delete pLanguageTable;

    m_pLocaleSettingLB->SetSelectHdl( LINK( this, OfaLanguagesTabPage, LocaleSettingHdl ) );
    m_pDatePatternsED->SetModifyHdl( LINK( this, OfaLanguagesTabPage, DatePatternsHdl ) );

    Link aLink( LINK( this, OfaLanguagesTabPage, SupportHdl ) );
    m_pAsianSupportCB->SetClickHdl( aLink );
    m_pCTLSupportCB->SetClickHdl( aLink );

    m_pAsianSupportCB->Check( m_bOldAsian = pLangConfig->aLanguageOptions.IsAnyEnabled() );
    m_pAsianSupportCB->SaveValue();
    sal_Bool bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK);
    m_pAsianSupportCB->Enable(!bReadonly);
    SupportHdl( m_pAsianSupportCB );

    m_pCTLSupportCB->Check( m_bOldCtl = pLangConfig->aLanguageOptions.IsCTLFontEnabled() );
    m_pCTLSupportCB->SaveValue();
    bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT);
    m_pCTLSupportCB->Enable(!bReadonly);
    SupportHdl( m_pCTLSupportCB );

    m_pIgnoreLanguageChangeCB->Check( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange() );
}

OfaLanguagesTabPage::~OfaLanguagesTabPage()
{
    delete pLangConfig;
}

SfxTabPage* OfaLanguagesTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaLanguagesTabPage(pParent, rAttrSet);
}

static void lcl_UpdateAndDelete(SfxVoidItem* pInvalidItems[], SfxBoolItem* pBoolItems[], sal_uInt16 nCount)
{
    SfxViewFrame* pCurrentFrm = SfxViewFrame::Current();
    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while(pViewFrm)
    {
        SfxBindings& rBind = pViewFrm->GetBindings();
        for(sal_Int16 i = 0; i < nCount; i++)
        {
            if(pCurrentFrm == pViewFrm)
                rBind.InvalidateAll(sal_False);
            rBind.SetState( *pInvalidItems[i] );
            rBind.SetState( *pBoolItems[i] );
        }
        pViewFrm = SfxViewFrame::GetNext(*pViewFrm);
    }
    for(sal_Int16 i = 0; i < nCount; i++)
    {
        delete pInvalidItems[i];
        delete pBoolItems[i] ;
    }
}

sal_Bool OfaLanguagesTabPage::FillItemSet( SfxItemSet& rSet )
{
    // lock configuration broadcasters so that we can coordinate the notifications
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( sal_True );
    pLangConfig->aLanguageOptions.BlockBroadcasts( sal_True );
    pLangConfig->aLinguConfig.BlockBroadcasts( sal_True );

    /*
     * Sequence checking only matters when CTL support is enabled.
     *
     * So we only need to check for sequence checking if
     * a) previously it was unchecked and is now checked or
     * b) it was already checked but the CTL language has changed
     */
    if (
         m_pCTLSupportCB->IsChecked() &&
         (m_pCTLSupportCB->GetSavedValue() != STATE_CHECK ||
         m_pComplexLanguageLB->GetSavedValue() != m_pComplexLanguageLB->GetSelectEntryPos())
       )
    {
        //sequence checking has to be switched on depending on the selected CTL language
        LanguageType eCTLLang = m_pComplexLanguageLB->GetSelectLanguage();
        sal_Bool bOn = MsLangId::needsSequenceChecking( eCTLLang);
        pLangConfig->aLanguageOptions.SetCTLSequenceCheckingRestricted(bOn);
        pLangConfig->aLanguageOptions.SetCTLSequenceChecking(bOn);
        pLangConfig->aLanguageOptions.SetCTLSequenceCheckingTypeAndReplace(bOn);
    }
    try
    {
        // handle settings for UI Language
        // a change of setting needs to bring up a warning message
        OUString aLangString;
        sal_Int32 d = (sal_Int32)(sal_IntPtr)m_pUserInterfaceLB->GetEntryData(m_pUserInterfaceLB->GetSelectEntryPos());
        if( d > 0 && seqInstalledLanguages.getLength() > d-1)
            aLangString = seqInstalledLanguages[d-1];

        /*
        if( m_pUserInterfaceLB->GetSelectEntryPos() > 0)
            aLangString = ConvertLanguageToIsoString(m_pUserInterfaceLB->GetSelectLanguage());
        */
        Reference< XMultiServiceFactory > theConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        theArgs[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(sUserLocalePath)));
        Reference< XPropertySet >xProp(
            theConfigProvider->createInstanceWithArguments(sAccessUpdSrvc, theArgs ), UNO_QUERY_THROW );
        if ( !m_sUserLocaleValue.equals(aLangString))
        {
            // OSL_FAIL("UserInterface language was changed, restart.");
            // write new value
            xProp->setPropertyValue(sUserLocaleKey, makeAny(aLangString));
            Reference< XChangesBatch >(xProp, UNO_QUERY_THROW)->commitChanges();
            // display info
            InfoBox aBox(this, CUI_RES(RID_SVX_MSGBOX_LANGUAGE_RESTART));
            aBox.Execute();

            // tell quickstarter to stop being a veto listener

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext());
            css::office::Quickstart::createAndSetVeto(xContext, false, false, false/*DisableVeto*/);
        }
    }
    catch (const Exception& e)
    {
        // we'll just leave the box in it's default setting and won't
        // even give it event handler...
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(aMsg.getStr());
    }

    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    LanguageType eOldLocale = (!sLang.isEmpty() ?
        LanguageTag( sLang ).getLanguageType() : LANGUAGE_SYSTEM);
    LanguageType eNewLocale = m_pLocaleSettingLB->GetSelectLanguage();
    if ( eOldLocale != eNewLocale )
    {
        // an empty string denotes SYSTEM locale
        OUString sNewLang;
        if ( eNewLocale != LANGUAGE_SYSTEM )
            sNewLang = LanguageTag( eNewLocale).getBcp47();

        // locale nowadays get to AppSettings via notification
        // this will happen after releasing the lock on the ConfigurationBroadcaster at
        // the end of this method
        pLangConfig->aSysLocaleOptions.SetLocaleConfigString( sNewLang );
        rSet.Put( SfxBoolItem( SID_OPT_LOCALE_CHANGED, sal_True ) );

        sal_uInt16 nNewType = SvtLanguageOptions::GetScriptTypeOfLanguage( eNewLocale );
        bool bNewCJK = ( nNewType & SCRIPTTYPE_ASIAN ) != 0;
        SvtCompatibilityOptions aCompatOpts;
        aCompatOpts.SetDefault( COMPATIBILITY_PROPERTYNAME_EXPANDWORDSPACE, !bNewCJK );
    }

    if(m_pDecimalSeparatorCB->GetSavedValue() != m_pDecimalSeparatorCB->IsChecked())
        pLangConfig->aSysLocaleOptions.SetDecimalSeparatorAsLocale(m_pDecimalSeparatorCB->IsChecked());

    if(m_pIgnoreLanguageChangeCB->GetSavedValue() != m_pIgnoreLanguageChangeCB->IsChecked())
        pLangConfig->aSysLocaleOptions.SetIgnoreLanguageChange(m_pIgnoreLanguageChangeCB->IsChecked());

    // Configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default.
    OUString sOldCurr = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    sal_uInt16 nCurrPos = m_pCurrencyLB->GetSelectEntryPos();
    const NfCurrencyEntry* pCurr = (const NfCurrencyEntry*)
        m_pCurrencyLB->GetEntryData( nCurrPos );
    OUString sNewCurr;
    if ( pCurr )
        sNewCurr = SvtSysLocaleOptions::CreateCurrencyConfigString(
            pCurr->GetBankSymbol(), pCurr->GetLanguage() );
    if ( sOldCurr != sNewCurr )
        pLangConfig->aSysLocaleOptions.SetCurrencyConfigString( sNewCurr );

    // Configured date acceptance patterns, for example Y-M-D;M-D or empty for
    // locale default.
    if (m_pDatePatternsED->GetText() != m_pDatePatternsED->GetSavedValue())
        pLangConfig->aSysLocaleOptions.SetDatePatternsConfigString( m_pDatePatternsED->GetText());

    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    Reference< css::linguistic2::XLinguProperties > xLinguProp = LinguMgr::GetLinguPropertySet();
    sal_Bool bCurrentDocCBChecked = m_pCurrentDocCB->IsChecked();
    if(m_pCurrentDocCB->IsEnabled())
        bLanguageCurrentDoc_Impl = bCurrentDocCBChecked;
    sal_Bool bCurrentDocCBChanged = bCurrentDocCBChecked != m_pCurrentDocCB->GetSavedValue();

    sal_Bool bValChanged = m_pWesternLanguageLB->GetSavedValue() != m_pWesternLanguageLB->GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pWesternLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag( eSelectLang).getLocale( false );
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, ::com::sun::star::i18n::ScriptType::LATIN),
                SID_ATTR_LANGUAGE));
        }
    }
    bValChanged = m_pAsianLanguageLB->GetSavedValue() != m_pAsianLanguageLB->GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pAsianLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag( eSelectLang).getLocale( false );
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale_CJK" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CJK( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, ::com::sun::star::i18n::ScriptType::ASIAN),
                SID_ATTR_CHAR_CJK_LANGUAGE));
        }
    }
    bValChanged = m_pComplexLanguageLB->GetSavedValue() != m_pComplexLanguageLB->GetSelectEntryPos();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pComplexLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag( eSelectLang).getLocale( false );
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale_CTL" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CTL( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet.Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, ::com::sun::star::i18n::ScriptType::COMPLEX),
                SID_ATTR_CHAR_CTL_LANGUAGE));
        }
    }

    if(m_pAsianSupportCB->GetSavedValue() != m_pAsianSupportCB->IsChecked() )
    {
        sal_Bool bChecked = m_pAsianSupportCB->IsChecked();
        pLangConfig->aLanguageOptions.SetAll(bChecked);

        //iterate over all bindings to invalidate vertical text direction
        const sal_uInt16 STATE_COUNT = 2;

        SfxBoolItem* pBoolItems[STATE_COUNT];
        pBoolItems[0] = new SfxBoolItem(SID_VERTICALTEXT_STATE, sal_False);
        pBoolItems[1] = new SfxBoolItem(SID_TEXT_FITTOSIZE_VERTICAL, sal_False);

        SfxVoidItem* pInvalidItems[STATE_COUNT];
        pInvalidItems[0] = new SfxVoidItem(SID_VERTICALTEXT_STATE);
        pInvalidItems[1] = new SfxVoidItem(SID_TEXT_FITTOSIZE_VERTICAL);

        lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( m_pCTLSupportCB->GetSavedValue() != m_pCTLSupportCB->IsChecked() )
    {
        SvtSearchOptions aOpt;
        aOpt.SetIgnoreDiacritics_CTL (true);
        pLangConfig->aLanguageOptions.SetCTLFontEnabled( m_pCTLSupportCB->IsChecked() );

        const sal_uInt16 STATE_COUNT = 1;
        SfxBoolItem* pBoolItems[STATE_COUNT];
        pBoolItems[0] = new SfxBoolItem(SID_CTLFONT_STATE, sal_False);
        SfxVoidItem* pInvalidItems[STATE_COUNT];
        pInvalidItems[0] = new SfxVoidItem(SID_CTLFONT_STATE);
        lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( pLangConfig->aSysLocaleOptions.IsModified() )
        pLangConfig->aSysLocaleOptions.Commit();

    // first release the lock on the ConfigurationBroadcaster for Locale changes
    // it seems that our code relies on the fact that before other changes like e.g. currency
    // are broadcasted locale changes have been done
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( sal_False );
    pLangConfig->aLanguageOptions.BlockBroadcasts( sal_False );
    pLangConfig->aLinguConfig.BlockBroadcasts( sal_False );

    return sal_False;
}
//-----------------------------------------------------------------------------
void OfaLanguagesTabPage::Reset( const SfxItemSet& rSet )
{
    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    if ( !sLang.isEmpty() )
        m_pLocaleSettingLB->SelectLanguage(LanguageTag(sLang).getLanguageType());
    else
        m_pLocaleSettingLB->SelectLanguage( LANGUAGE_USER_SYSTEM_CONFIG );
    sal_Bool bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_LOCALE);
    m_pLocaleSettingLB->Enable(!bReadonly);
    m_pLocaleSettingFT->Enable(!bReadonly);

    //
    m_pDecimalSeparatorCB->Check( pLangConfig->aSysLocaleOptions.IsDecimalSeparatorAsLocale());
    m_pDecimalSeparatorCB->SaveValue();

    m_pIgnoreLanguageChangeCB->Check( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange());
    m_pIgnoreLanguageChangeCB->SaveValue();

    // let LocaleSettingHdl enable/disable checkboxes for CJK/CTL support
    // #i15812# must be done *before* the configured currency is set
    // and update the decimal separator used for the given locale
    LocaleSettingHdl(m_pLocaleSettingLB);

    // configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default
    OUString aAbbrev;
    LanguageType eLang;
    const NfCurrencyEntry* pCurr = NULL;
    sLang = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    if ( !sLang.isEmpty() )
    {
        SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( aAbbrev, eLang, sLang );
        pCurr = SvNumberFormatter::GetCurrencyEntry( aAbbrev, eLang );
    }
    // if pCurr==NULL the SYSTEM entry is selected
    sal_uInt16 nPos = m_pCurrencyLB->GetEntryPos( (void*) pCurr );
    m_pCurrencyLB->SelectEntryPos( nPos );
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_CURRENCY);
    m_pCurrencyLB->Enable(!bReadonly);
    m_pCurrencyFT->Enable(!bReadonly);

    // date acceptance patterns
    OUString aDatePatternsString = pLangConfig->aSysLocaleOptions.GetDatePatternsConfigString();
    if (aDatePatternsString.isEmpty())
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDatePatternsString = lcl_getDatePatternsConfigString( rLocaleWrapper);
    }
    m_pDatePatternsED->SetText( aDatePatternsString);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_DATEPATTERNS);
    m_pDatePatternsED->Enable(!bReadonly);
    m_pDatePatternsFT->Enable(!bReadonly);
    m_pDatePatternsED->SaveValue();

    //western/CJK/CLK language
    LanguageType eCurLang = LANGUAGE_NONE;
    LanguageType eCurLangCJK = LANGUAGE_NONE;
    LanguageType eCurLangCTL = LANGUAGE_NONE;
    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    //collect the configuration values first
    m_pCurrentDocCB->Enable(sal_False);
    //
    Any aWestLang;
    Any aCJKLang;
    Any aCTLLang;
    try
    {
        aWestLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale");
        Locale aLocale;
        aWestLang >>= aLocale;

        eCurLang = LanguageTag( aLocale ).getLanguageType( false);

        aCJKLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale_CJK");
        aLocale = Locale();
        aCJKLang >>= aLocale;
        eCurLangCJK = LanguageTag( aLocale ).getLanguageType( false);

        aCTLLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale_CTL");
        aLocale = Locale();
        aCTLLang >>= aLocale;
        eCurLangCTL = LanguageTag( aLocale ).getLanguageType( false);
    }
    catch (const Exception&)
    {
    }
    //overwrite them by the values provided by the DocShell
    if(pCurrentDocShell)
    {
        m_pCurrentDocCB->Enable(sal_True);
        m_pCurrentDocCB->Check(bLanguageCurrentDoc_Impl);
        const SfxPoolItem* pLang;
        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_LANGUAGE, sal_False, &pLang))
        {
            LanguageType eTempCurLang = ((const SvxLanguageItem*)pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLang, ::com::sun::star::i18n::ScriptType::LATIN) != eTempCurLang)
                eCurLang = eTempCurLang;
        }

        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, sal_False, &pLang))
        {
            LanguageType eTempCurLang = ((const SvxLanguageItem*)pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLangCJK, ::com::sun::star::i18n::ScriptType::ASIAN) != eTempCurLang)
                eCurLangCJK = eTempCurLang;
        }

        if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, sal_False, &pLang))
        {
            LanguageType eTempCurLang = ((const SvxLanguageItem*)pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLangCTL, ::com::sun::star::i18n::ScriptType::COMPLEX) != eTempCurLang)
                eCurLangCTL = eTempCurLang;
        }
    }
    if(LANGUAGE_NONE == eCurLang || LANGUAGE_DONTKNOW == eCurLang)
        m_pWesternLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pWesternLanguageLB->SelectLanguage(eCurLang);

    if(LANGUAGE_NONE == eCurLangCJK || LANGUAGE_DONTKNOW == eCurLangCJK)
        m_pAsianLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pAsianLanguageLB->SelectLanguage(eCurLangCJK);

    if(LANGUAGE_NONE == eCurLangCTL || LANGUAGE_DONTKNOW == eCurLangCTL)
        m_pComplexLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pComplexLanguageLB->SelectLanguage(eCurLangCTL);

    m_pWesternLanguageLB->SaveValue();
    m_pAsianLanguageLB->SaveValue();
    m_pComplexLanguageLB->SaveValue();
    m_pIgnoreLanguageChangeCB->SaveValue();
    m_pCurrentDocCB->SaveValue();

    sal_Bool bEnable = !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale" );
    m_pWesternLanguageFT->Enable( bEnable );
    m_pWesternLanguageLB->Enable( bEnable );



    // #i15812# controls for CJK/CTL already enabled/disabled from LocaleSettingHdl
#if 0
    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale_CJK" ) && m_pAsianSupportCB->IsChecked() );
    m_pAsianLanguageLB->Enable( bEnable );

    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale_CTL" ) && m_pCTLSupportCB->IsChecked() );
    m_pComplexLanguageLB->Enable( bEnable );
#endif
    // check the box "For the current document only"
    // set the focus to the Western Language box
    const SfxPoolItem* pLang = 0;
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_SET_DOCUMENT_LANGUAGE, sal_False, &pLang ) &&( (const SfxBoolItem*)pLang)->GetValue() == sal_True )
    {
        m_pWesternLanguageLB->GrabFocus();
        m_pCurrentDocCB->Enable(sal_True);
        m_pCurrentDocCB->Check(sal_True);
    }
}

IMPL_LINK(  OfaLanguagesTabPage, SupportHdl, CheckBox*, pBox )
{
    DBG_ASSERT( pBox, "OfaLanguagesTabPage::SupportHdl(): pBox invalid" );

    sal_Bool bCheck = pBox->IsChecked();
    if ( m_pAsianSupportCB == pBox )
    {
        sal_Bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CJK");
        bCheck = ( bCheck && !bReadonly );
        m_pAsianLanguageLB->Enable( bCheck );
        if( pBox->IsEnabled() )
            m_bOldAsian = bCheck;
    }
    else if ( m_pCTLSupportCB == pBox )
    {
        sal_Bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CTL");
        bCheck = ( bCheck && !bReadonly  );
        m_pComplexLanguageLB->Enable( bCheck );
        if( pBox->IsEnabled() )
            m_bOldCtl = bCheck;
    }
    else
        SAL_WARN( "cui.options", "OfaLanguagesTabPage::SupportHdl(): wrong pBox" );

    return 0;
}

namespace
{
    void lcl_checkLanguageCheckBox(CheckBox* _rCB,sal_Bool _bNewValue,sal_Bool _bOldValue)
    {
        if ( _bNewValue )
            _rCB->Check(sal_True);
        else
            _rCB->Check( _bOldValue );
// #i15082# do not call SaveValue() in running dialog...
//      _rCB.SaveValue();
        _rCB->Enable( !_bNewValue );
    }
}

IMPL_LINK( OfaLanguagesTabPage, LocaleSettingHdl, SvxLanguageBox*, pBox )
{
    LanguageType eLang = pBox->GetSelectLanguage();
    sal_uInt16 nType = SvtLanguageOptions::GetScriptTypeOfLanguage(eLang);
    // first check if CTL must be enabled
    // #103299# - if CTL font setting is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT))
    {
        bool bIsCTLFixed = (nType & SCRIPTTYPE_COMPLEX) != 0;
        lcl_checkLanguageCheckBox(m_pCTLSupportCB, bIsCTLFixed, m_bOldCtl);
        SupportHdl( m_pCTLSupportCB );
    }
    // second check if CJK must be enabled
    // #103299# - if CJK support is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK))
    {
        bool bIsCJKFixed = (nType & SCRIPTTYPE_ASIAN) != 0;
        lcl_checkLanguageCheckBox(m_pAsianSupportCB, bIsCJKFixed, m_bOldAsian);
        SupportHdl( m_pAsianSupportCB );
    }

    sal_uInt16 nPos;
    if ( eLang == LANGUAGE_USER_SYSTEM_CONFIG )
        nPos = m_pCurrencyLB->GetEntryPos( (void*) NULL );
    else
    {
        const NfCurrencyEntry* pCurr = &SvNumberFormatter::GetCurrencyEntry( eLang );
        nPos = m_pCurrencyLB->GetEntryPos( (void*) pCurr );
    }
    m_pCurrencyLB->SelectEntryPos( nPos );

    // obtain corresponding locale data
    LanguageTag aLanguageTag( eLang);
    LocaleDataWrapper aLocaleWrapper( aLanguageTag );

    // update the decimal separator key of the related CheckBox
    OUString sTempLabel(m_pDecimalSeparatorCB->GetText());
    sTempLabel = sTempLabel.replaceFirst("%1", aLocaleWrapper.getNumDecimalSep() );
    m_pDecimalSeparatorCB->SetText(sTempLabel);

    // update the date acceptance patterns
    OUString aDatePatternsString = lcl_getDatePatternsConfigString( aLocaleWrapper);
    m_pDatePatternsED->SetText( aDatePatternsString);

    return 0;
}

IMPL_LINK( OfaLanguagesTabPage, DatePatternsHdl, Edit*, pEd )
{
    OUString aPatterns( pEd->GetText());
    bool bValid = true;
    if (!aPatterns.isEmpty())
    {
        for (sal_Int32 nIndex=0; nIndex >= 0 && bValid; /*nop*/)
        {
            OUString aPat( aPatterns.getToken( 0, ';', nIndex));
            if (aPat.isEmpty() && nIndex < 0)
            {
                // Indicating failure when about to append a pattern is too
                // confusing. Empty patterns are ignored anyway when sequencing
                // to SvtSysLocale.
                continue;   // for
            }
            else if (aPat.getLength() < 2)
                bValid = false;
            else
            {
                bool bY, bM, bD;
                bY = bM = bD = false;
                bool bSep = true;
                for (sal_Int32 i = 0; i < aPat.getLength() && bValid; /*nop*/)
                {
                    sal_uInt32 c = aPat.iterateCodePoints( &i);
                    // Only one Y,M,D per pattern, separated by any character(s).
                    switch (c)
                    {
                        case 'Y':
                            if (bY || !bSep)
                                bValid = false;
                            bY = true;
                            bSep = false;
                            break;
                        case 'M':
                            if (bM || !bSep)
                                bValid = false;
                            bM = true;
                            bSep = false;
                            break;
                        case 'D':
                            if (bD || !bSep)
                                bValid = false;
                            bD = true;
                            bSep = false;
                            break;
                        default:
                            bSep = true;
                    }
                }
                // At least one of Y,M,D
                bValid &= (bY || bM || bD);
            }
        }
    }
    if (bValid)
    {
        pEd->SetControlForeground();
        pEd->SetControlBackground();
    }
    else
    {
        // color to use as background for an invalid pattern
        #define INVALID_PATTERN_BACKGROUND_COLOR ::Color(0xff6563)
#if 0
        // color to use as foreground for an invalid pattern
        #define INVALID_PATTERN_FOREGROUND_COLOR Color(COL_WHITE)
        //! Gives white on white!?!
        pEd->SetControlBackground( INVALID_PATTERN_BACKGROUND_COLOR);
        pEd->SetControlForeground( INVALID_PATTERN_FOREGROUND_COLOR);
#else
        pEd->SetControlForeground( INVALID_PATTERN_BACKGROUND_COLOR);
#endif
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
