/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optsave.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:32:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#define _SVX_OPTSAVE_CXX

#include "optsave.hrc"
#include <svx/dialogs.hrc>

#include "optsave.hxx"
#include <svx/dialmgr.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONSDLG_HXX
#include <svtools/optionsdlg.hxx>
#endif

#include <vcl/msgbox.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace comphelper;
using rtl::OUString;

#define C2U(cChar)                  OUString::createFromAscii(cChar)
#define C2S(cChar)                  String( RTL_CONSTASCII_STRINGPARAM(cChar) )
#define CFG_PAGE_AND_GROUP          C2S("General"), C2S("LoadSave")
// !! you have to update these index, if you changed the list of the child windows !!
#define WININDEX_AUTOSAVE           ((USHORT)6)
#define WININDEX_SAVEURL_RELFSYS    ((USHORT)9)

// -------------------- --------------------------------------------------
class FilterWarningDialog_Impl : public ModalDialog
{
    OKButton        aOk;
    CancelButton    aCancel;
    FixedImage      aImage;
    FixedInfo       aFilterWarningFT;

    public:
    FilterWarningDialog_Impl(Window* pParent);

    void            SetFilterName(const String& rFilterUIName);
};
// ----------------------------------------------------------------------
FilterWarningDialog_Impl::FilterWarningDialog_Impl(Window* pParent) :
    ModalDialog(pParent, SVX_RES( RID_SVXDLG_FILTER_WARNING ) ),
    aOk(                this, SVX_RES(PB_OK               )),
    aCancel(            this, SVX_RES(PB_CANCEL           )),
    aImage(             this, SVX_RES(IMG_WARNING         )),
    aFilterWarningFT(   this, SVX_RES(FT_FILTER_WARNING   ))
{
    FreeResource();
    aImage.SetImage(WarningBox::GetStandardImage());
}
// ----------------------------------------------------------------------
void    FilterWarningDialog_Impl::SetFilterName(const String& rFilterUIName)
{
    String sTmp(aFilterWarningFT.GetText());
    sTmp.SearchAndReplaceAscii("%1", rFilterUIName);
    aFilterWarningFT.SetText(sTmp);
}
// ----------------------------------------------------------------------
#ifdef FILTER_WARNING_ENABLED
class SvxAlienFilterWarningConfig_Impl : public utl::ConfigItem
{
    sal_Bool bWarning;
    com::sun::star::uno::Sequence< OUString > aPropNames;

    public:
        SvxAlienFilterWarningConfig_Impl();
        ~SvxAlienFilterWarningConfig_Impl();

    virtual void            Commit();

    void                    ResetWarning()
                            {
                                if(bWarning)
                                {
                                    bWarning = sal_False;
                                    ConfigItem::SetModified();
                                }

                            }
    sal_Bool                IsWarning()const{return bWarning;}
};
// ----------------------------------------------------------------------
SvxAlienFilterWarningConfig_Impl::SvxAlienFilterWarningConfig_Impl() :
    ConfigItem(C2U("TypeDetection.Misc/Defaults"),
        CONFIG_MODE_IMMEDIATE_UPDATE),
    aPropNames(1),
    bWarning(sal_True)
{
    aPropNames.getArray()[0] = C2U("ShowAlienFilterWarning");
    Sequence<Any> aValues = GetProperties(aPropNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aPropNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aPropNames.getLength() &&
        pValues[0].hasValue() &&
            pValues[0].getValueType() == ::getBooleanCppuType())
        bWarning = *(sal_Bool*)pValues[0].getValue();
}
// ----------------------------------------------------------------------
SvxAlienFilterWarningConfig_Impl::~SvxAlienFilterWarningConfig_Impl()
{
    if(IsModified())
        Commit();
}
// ----------------------------------------------------------------------
void SvxAlienFilterWarningConfig_Impl::Commit()
{
    Sequence<Any> aValues(aPropNames.getLength());
    Any* pValues = aValues.getArray();
    pValues[0].setValue(&bWarning, ::getBooleanCppuType());
    PutProperties(aPropNames, aValues);
}
#endif // FILTER_WARNING_ENABLED
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

struct SvxSaveTabPage_Impl
{
    Reference< XNameContainer > xFact;
    Sequence< OUString >        aFilterArr[APP_COUNT];
    Sequence< sal_Bool >        aAlienArr[APP_COUNT];
    Sequence< sal_Bool >        aODFArr[APP_COUNT];
    Sequence< OUString >        aUIFilterArr[APP_COUNT];
    OUString                    aDefaultArr[APP_COUNT];
    sal_Bool                    aDefaultReadonlyArr[APP_COUNT];
    sal_Bool                    bInitialized;

    SvxSaveTabPage_Impl();
    ~SvxSaveTabPage_Impl();
};

SvxSaveTabPage_Impl::SvxSaveTabPage_Impl() : bInitialized( sal_False )
{
}

SvxSaveTabPage_Impl::~SvxSaveTabPage_Impl()
{
}

// class SvxSaveTabPage --------------------------------------------------

SfxSaveTabPage::SfxSaveTabPage( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SFXPAGE_SAVE ), rCoreSet ),

    aLoadFL             ( this, SVX_RES( LB_LOAD ) ),
    aLoadUserSettingsCB ( this, SVX_RES( CB_LOAD_SETTINGS ) ),
    aLoadDocPrinterCB   ( this, SVX_RES( CB_LOAD_DOCPRINTER ) ),

    aSaveFL             ( this, SVX_RES( GB_SAVE ) ),
    aDocInfoCB          ( this, SVX_RES( BTN_DOCINFO ) ),
    aBackupFI           ( this, SVX_RES( FI_BACKUP ) ),
    aBackupCB           ( this, SVX_RES( BTN_BACKUP ) ),
    aAutoSaveCB         ( this, SVX_RES( BTN_AUTOSAVE ) ),
    aAutoSaveEdit       ( this, SVX_RES( ED_AUTOSAVE ) ),
    aMinuteFT           ( this, SVX_RES( FT_MINUTE ) ),
    aRelativeFsysCB     ( this, SVX_RES( BTN_RELATIVE_FSYS ) ),
    aRelativeInetCB     ( this, SVX_RES( BTN_RELATIVE_INET ) ),

    aDefaultFormatFL    ( this, SVX_RES( FL_FILTER ) ),
    aODFVersionFT       ( this, SVX_RES( FT_ODF_VERSION ) ),
    aODFVersionLB       ( this, SVX_RES( LB_ODF_VERSION ) ),
    aSizeOptimizationCB ( this, SVX_RES( BTN_NOPRETTYPRINTING ) ),
    aWarnAlienFormatCB  ( this, SVX_RES( BTN_WARNALIENFORMAT ) ),
    aDocTypeFT          ( this, SVX_RES( FT_APP ) ),
    aDocTypeLB          ( this, SVX_RES( LB_APP ) ),
    aSaveAsFT           ( this, SVX_RES( FT_FILTER ) ),
    aSaveAsFI           ( this, SVX_RES( FI_FILTER ) ),
    aSaveAsLB           ( this, SVX_RES( LB_FILTER ) ),
    aODFWarningFI       ( this, SVX_RES( FI_ODF_WARNING ) ),
    aODFWarningFT       ( this, SVX_RES( FT_WARN ) ),

    pImpl               ( new SvxSaveTabPage_Impl )

{
    sal_Bool bHighContrast = GetDisplayBackground().GetColor().IsDark();
    aODFWarningFI.SetImage(
        Image( SVX_RES( bHighContrast ? IMG_ODF_WARNING_HC : IMG_ODF_WARNING ) ) );

    FreeResource();

    Link aLink = LINK( this, SfxSaveTabPage, AutoClickHdl_Impl );
    aAutoSaveCB.SetClickHdl( aLink );
    aAutoSaveEdit.SetMaxTextLen( 2 );

    SvtModuleOptions aModuleOpt;
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_MATH ));
    else
    {
        pImpl->aDefaultArr[APP_MATH] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_MATH);
        pImpl->aDefaultReadonlyArr[APP_MATH] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_MATH);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_DRAW ));
    else
    {
        pImpl->aDefaultArr[APP_DRAW] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_DRAW);
        pImpl->aDefaultReadonlyArr[APP_DRAW] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_DRAW);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_IMPRESS ));
    else
    {
        pImpl->aDefaultArr[APP_IMPRESS] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_IMPRESS);
        pImpl->aDefaultReadonlyArr[APP_IMPRESS] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_IMPRESS);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_CALC ));
    else
    {
        pImpl->aDefaultArr[APP_CALC] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_CALC);
        pImpl->aDefaultReadonlyArr[APP_CALC] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_CALC);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
    {
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_WRITER ));
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_WRITER_WEB ));
        aSaveAsLB.RemoveEntry(aSaveAsLB.GetEntryPos( (void*) APP_WRITER_GLOBAL ));
    }
    else
    {
        pImpl->aDefaultArr[APP_WRITER] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITER);
        pImpl->aDefaultArr[APP_WRITER_WEB] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITERWEB);
        pImpl->aDefaultArr[APP_WRITER_GLOBAL] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITERGLOBAL);
        pImpl->aDefaultReadonlyArr[APP_WRITER] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_WRITER);
        pImpl->aDefaultReadonlyArr[APP_WRITER_WEB] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_WRITERWEB);
        pImpl->aDefaultReadonlyArr[APP_WRITER_GLOBAL] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_WRITERGLOBAL);
    }

    aLink = LINK( this, SfxSaveTabPage, ODFVersionHdl_Impl );
    aODFVersionLB.SetSelectHdl( aLink );
    aLink = LINK( this, SfxSaveTabPage, FilterHdl_Impl );
    aDocTypeLB.SetSelectHdl( aLink );
    aSaveAsLB.SetSelectHdl( aLink );

    DetectHiddenControls();
}

// -----------------------------------------------------------------------

SfxSaveTabPage::~SfxSaveTabPage()
{
    delete pImpl;
}

// -----------------------------------------------------------------------

SfxTabPage* SfxSaveTabPage::Create( Window* pParent,
                                    const SfxItemSet& rAttrSet )
{
    return ( new SfxSaveTabPage( pParent, rAttrSet ) );
}

/* -----------------------------05.04.01 13:10--------------------------------

 ---------------------------------------------------------------------------*/
OUString lcl_ExtractUIName(const Sequence<PropertyValue> rProperties)
{
    OUString sRet;
    const PropertyValue* pProperties = rProperties.getConstArray();
    for(int nProp = 0; nProp < rProperties.getLength(); nProp++)
    {
        if(!pProperties[nProp].Name.compareToAscii("UIName"))
        {
            pProperties[nProp].Value >>= sRet;
            break;
        }
    }
    return sRet;
}
// -----------------------------------------------------------------------
bool SfxSaveTabPage::AcceptFilter( USHORT nPos )
{
    const OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
    sal_Bool bAlien = sal_False, bODF = sal_False;
    OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getArray();
    OUString sUIName;
    for(int nFilter = 0; nFilter < pImpl->aFilterArr[nPos].getLength(); nFilter++)
    {
        if( pImpl->aDefaultArr[nPos] == pFilters[nFilter] )
        {
            bAlien = pImpl->aAlienArr[nPos][nFilter];
            bODF = pImpl->aODFArr[nPos][nFilter];
            sUIName = pUIFilters[nFilter];;
            break;
        }
    }
    bool bSet = true;
    return bSet;
}
// -----------------------------------------------------------------------
void SfxSaveTabPage::DetectHiddenControls()
{
    long nDelta = 0;
    // the index of the first child window which perhaps have to move upwards
    USHORT nWinIndex = WININDEX_SAVEURL_RELFSYS;
    SvtOptionsDialogOptions aOptionsDlgOpt;

    if ( aOptionsDlgOpt.IsOptionHidden( C2S("Backup"), CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "Backup"
        aBackupFI.Hide();
        aBackupCB.Hide();
        // the other controls have to move upwards the height of checkbox + space
        nDelta = aAutoSaveCB.GetPosPixel().Y() - aBackupCB.GetPosPixel().Y();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( C2S("AutoSave"), CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "AutoSave"
        aAutoSaveCB.Hide();
        aAutoSaveEdit.Hide();
        aMinuteFT.Hide();
        // the other controls have to move upwards the height of checkbox + space
        nDelta += aRelativeFsysCB.GetPosPixel().Y() - aAutoSaveCB.GetPosPixel().Y();
    }
    else if ( nDelta > 0 )
        // the "AutoSave" controls have to move upwards too
        nWinIndex = WININDEX_AUTOSAVE;

    if ( nDelta > 0 )
    {
        USHORT i, nChildCount = GetChildCount();
        for ( i = nWinIndex; i < nChildCount; ++i )
        {
            Window* pWin = GetChild(i);
            Point aPos = pWin->GetPosPixel();
            aPos.Y() -= nDelta;
            pWin->SetPosPixel( aPos );
        }
    }
}
// -----------------------------------------------------------------------
BOOL SfxSaveTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
    SvtSaveOptions aSaveOpt;
    if(aLoadUserSettingsCB.IsChecked() != aLoadUserSettingsCB.GetSavedValue())
    {
        aSaveOpt.SetLoadUserSettings(aLoadUserSettingsCB.IsChecked());
    }

    if ( aLoadDocPrinterCB.IsChecked() != aLoadDocPrinterCB.GetSavedValue() )
        aSaveOpt.SetLoadDocumentPrinter( aLoadDocPrinterCB.IsChecked() );

    if ( aODFVersionLB.GetSelectEntryPos() != aODFVersionLB.GetSavedValue() )
    {
        long nVersion = long( aODFVersionLB.GetEntryData( aODFVersionLB.GetSelectEntryPos() ) );
        aSaveOpt.SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion( nVersion ) );
    }

    if ( aDocInfoCB.IsChecked() != aDocInfoCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_DOCINFO ),
                               aDocInfoCB.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aBackupCB.IsEnabled() && aBackupCB.IsChecked() != aBackupCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_BACKUP ),
                               aBackupCB.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aSizeOptimizationCB.IsChecked() != aSizeOptimizationCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PRETTYPRINTING ), !aSizeOptimizationCB.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSaveCB.IsChecked() != aAutoSaveCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVE ),
                               aAutoSaveCB.IsChecked() ) );
        bModified |= TRUE;
    }
    if ( aWarnAlienFormatCB.IsChecked() != aWarnAlienFormatCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_WARNALIENFORMAT ),
                               aWarnAlienFormatCB.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSaveEdit.GetText() != aAutoSaveEdit.GetSavedValue() )
    {
        rSet.Put( SfxUInt16Item( GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                 (UINT16)aAutoSaveEdit.GetValue() ) );
        bModified |= TRUE;
    }
    // relativ speichern
    if ( aRelativeFsysCB.IsChecked() != aRelativeFsysCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_SAVEREL_FSYS ),
                               aRelativeFsysCB.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aRelativeInetCB.IsChecked() != aRelativeInetCB.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_SAVEREL_INET ),
                               aRelativeInetCB.IsChecked() ) );
        bModified |= TRUE;
    }

    SvtModuleOptions aModuleOpt;
    if(pImpl->aDefaultArr[APP_MATH].getLength() &&
            pImpl->aDefaultArr[APP_MATH] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_MATH) &&
            AcceptFilter( APP_MATH ))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_MATH, pImpl->aDefaultArr[APP_MATH]);

    if( pImpl->aDefaultArr[APP_DRAW].getLength() &&
            pImpl->aDefaultArr[APP_DRAW] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_DRAW) &&
            AcceptFilter( APP_DRAW ))
            aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_DRAW, pImpl->aDefaultArr[APP_DRAW]);

    if(pImpl->aDefaultArr[APP_IMPRESS].getLength() &&
            pImpl->aDefaultArr[APP_IMPRESS] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_IMPRESS)&&
            AcceptFilter( APP_IMPRESS ))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_IMPRESS, pImpl->aDefaultArr[APP_IMPRESS]);

    if(pImpl->aDefaultArr[APP_CALC].getLength() &&
            pImpl->aDefaultArr[APP_CALC] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_CALC)&&
            AcceptFilter( APP_CALC ))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_CALC, pImpl->aDefaultArr[APP_CALC]);

    if(pImpl->aDefaultArr[APP_WRITER].getLength() &&
            pImpl->aDefaultArr[APP_WRITER] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITER)&&
            AcceptFilter( APP_WRITER))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_WRITER, pImpl->aDefaultArr[APP_WRITER]);

    if(pImpl->aDefaultArr[APP_WRITER_WEB].getLength() &&
            pImpl->aDefaultArr[APP_WRITER_WEB] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITERWEB)&&
            AcceptFilter( APP_WRITER_WEB ))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_WRITERWEB, pImpl->aDefaultArr[APP_WRITER_WEB]);

    if(pImpl->aDefaultArr[APP_WRITER_GLOBAL].getLength() &&
            pImpl->aDefaultArr[APP_WRITER_GLOBAL] != aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_WRITERGLOBAL)&&
            AcceptFilter( APP_WRITER_GLOBAL ))
        aModuleOpt.SetFactoryDefaultFilter(SvtModuleOptions::E_WRITERGLOBAL, pImpl->aDefaultArr[APP_WRITER_GLOBAL]);

    return bModified;
}

// -----------------------------------------------------------------------

sal_Bool isODFFormat( OUString sFilter )
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
        NULL
    };

    sal_Bool bRet = sal_False;
    int i = 0;
    while ( aODFFormats[i] != NULL )
    {
        if ( sFilter.equalsAscii( aODFFormats[i++] ) )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

void SfxSaveTabPage::Reset( const SfxItemSet& )
{
    SvtSaveOptions aSaveOpt;
    aLoadUserSettingsCB.Check(aSaveOpt.IsLoadUserSettings());
    aLoadUserSettingsCB.SaveValue();
    aLoadDocPrinterCB.Check( aSaveOpt.IsLoadDocumentPrinter() );
    aLoadDocPrinterCB.SaveValue();

    if ( !pImpl->bInitialized )
    {
        try
        {
            Reference< XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
            pImpl->xFact = Reference<XNameContainer>(
                    xMSF->createInstance(C2U("com.sun.star.document.FilterFactory")), UNO_QUERY);

            DBG_ASSERT(pImpl->xFact.is(), "service com.sun.star.document.FilterFactory unavailable")
            Reference< XContainerQuery > xQuery(pImpl->xFact, UNO_QUERY);
            if(xQuery.is())
            {
                for(USHORT n = 0; n < aDocTypeLB.GetEntryCount(); n++)
                {
                    long nData = (long) aDocTypeLB.GetEntryData(n);
                    OUString sCommand;
                    sCommand = C2U("matchByDocumentService=%1:iflags=");
                    sCommand += String::CreateFromInt32(SFX_FILTER_IMPORT|SFX_FILTER_EXPORT);
                    sCommand += C2U(":eflags=");
                    sCommand += String::CreateFromInt32(SFX_FILTER_NOTINFILEDLG);
                    sCommand += C2U(":default_first");
                    String sReplace;
                    switch(nData)
                    {
                        case  APP_WRITER        : sReplace = C2U("com.sun.star.text.TextDocument");  break;
                        case  APP_WRITER_WEB    : sReplace = C2U("com.sun.star.text.WebDocument");   break;
                        case  APP_WRITER_GLOBAL : sReplace = C2U("com.sun.star.text.GlobalDocument");   break;
                        case  APP_CALC          : sReplace = C2U("com.sun.star.sheet.SpreadsheetDocument");break;
                        case  APP_IMPRESS       : sReplace = C2U("com.sun.star.presentation.PresentationDocument");break;
                        case  APP_DRAW          : sReplace = C2U("com.sun.star.drawing.DrawingDocument");break;
                        case  APP_MATH          : sReplace = C2U("com.sun.star.formula.FormulaProperties");break;
                        default: DBG_ERROR("illegal user data");
                    }
                    String sTmp(sCommand);
                    sTmp.SearchAndReplaceAscii("%1", sReplace);
                    sCommand = sTmp;
                    Reference< XEnumeration > xList = xQuery->createSubSetEnumerationByQuery(sCommand);
                    SequenceAsVector< OUString > lList;
                    SequenceAsVector< sal_Bool > lAlienList;
                    SequenceAsVector< sal_Bool > lODFList;
                    while(xList->hasMoreElements())
                    {
                        SequenceAsHashMap aFilter(xList->nextElement());
                        OUString sFilter = aFilter.getUnpackedValueOrDefault(OUString::createFromAscii("Name"),OUString());
                        if (sFilter.getLength())
                        {
                            sal_Int32 nFlags = aFilter.getUnpackedValueOrDefault(OUString::createFromAscii("Flags"),sal_Int32());
                            lList.push_back(sFilter);
                            lAlienList.push_back(0 != (nFlags & SFX_FILTER_ALIEN));
                            lODFList.push_back( isODFFormat( sFilter ) );
                        }
                    }
                    pImpl->aFilterArr[nData] = lList.getAsConstList();
                    pImpl->aAlienArr[nData] = lAlienList.getAsConstList();
                    pImpl->aODFArr[nData] = lODFList.getAsConstList();
                }
            }
            aDocTypeLB.SelectEntryPos(0);
            FilterHdl_Impl(&aDocTypeLB);
        }
        catch(Exception& )
        {
            DBG_ERROR("exception in FilterFactory access")
        }

        pImpl->bInitialized = sal_True;
    }

    aDocInfoCB.Check(aSaveOpt.IsDocInfoSave());
//    aDocInfoCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_DOCINFSAVE));

    aBackupCB.Check(aSaveOpt.IsBackup());
    BOOL bBackupRO = aSaveOpt.IsReadOnly(SvtSaveOptions::E_BACKUP);
    aBackupCB.Enable(!bBackupRO);
    aBackupFI.Show(bBackupRO);

    aAutoSaveCB.Check(aSaveOpt.IsAutoSave());
    aWarnAlienFormatCB.Check(aSaveOpt.IsWarnAlienFormat());
    aWarnAlienFormatCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_WARNALIENFORMAT));
//    aAutoSaveCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVE));

    // the pretty printing
    aSizeOptimizationCB.Check( !aSaveOpt.IsPrettyPrinting());
//    aSizeOptimizationCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_DOPRETTYPRINTING ));


    aAutoSaveEdit.SetValue( aSaveOpt.GetAutoSaveTime() );
//    aAutoSaveEdit.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVETIME));

    // relativ speichern
    aRelativeFsysCB.Check( aSaveOpt.IsSaveRelFSys() );
//    aRelativeFsysCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELFSYS));

    aRelativeInetCB.Check( aSaveOpt.IsSaveRelINet() );
//    aRelativeInetCB.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELINET));

    void* pDefaultVersion = (void*)long( aSaveOpt.GetODFDefaultVersion() );
    aODFVersionLB.SelectEntryPos( aODFVersionLB.GetEntryPos( pDefaultVersion ) );

    AutoClickHdl_Impl( &aAutoSaveCB );
    ODFVersionHdl_Impl( &aODFVersionLB );

    aDocInfoCB.SaveValue();
    aBackupCB.SaveValue();
    aWarnAlienFormatCB.SaveValue();
    aSizeOptimizationCB.SaveValue();
    aAutoSaveCB.SaveValue();
    aAutoSaveEdit.SaveValue();
//  aAutoSavePromptBtn.SaveValue();

    aRelativeFsysCB.SaveValue();
    aRelativeInetCB.SaveValue();
    aODFVersionLB.SaveValue();
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxSaveTabPage, AutoClickHdl_Impl, CheckBox *, pBox )
{
    if ( pBox == &aAutoSaveCB )
    {
        if ( aAutoSaveCB.IsChecked() )
        {
            aAutoSaveEdit.Enable();
            aMinuteFT.Enable();
//          aAutoSavePromptBtn.Enable();
            aAutoSaveEdit.GrabFocus();
        }
        else
        {
            aAutoSaveEdit.Disable();
            aMinuteFT.Disable();
//          aAutoSavePromptBtn.Disable();
        }
    }
    return 0;
}
/* -----------------------------05.04.01 13:10--------------------------------

 ---------------------------------------------------------------------------*/
OUString lcl_ExtracUIName(const Sequence<PropertyValue> rProperties)
{
    OUString sRet;
    sal_Int32 nFlags;
    const PropertyValue* pProperties = rProperties.getConstArray();
    for(int nProp = 0; nProp < rProperties.getLength(); nProp++)
    {
        if(!pProperties[nProp].Name.compareToAscii("UIName"))
        {
            pProperties[nProp].Value >>= sRet;
//!            break;
        }
        else if(!pProperties[nProp].Name.compareToAscii("Flags"))
        {
            if ( pProperties[nProp].Value >>= nFlags )
            {
                nFlags &= 0x100;
            }
        }
        else if(!pProperties[nProp].Name.compareToAscii("Name"))
        {
            pProperties[nProp].Value >>= sRet;
        }
    }
    return sRet;
}
/* -----------------------------05.04.01 13:37--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SfxSaveTabPage, FilterHdl_Impl, ListBox *, pBox )
{
    if(&aDocTypeLB == pBox)
    {
        USHORT nAppPos = pBox->GetSelectEntryPos();
        if ( nAppPos < APP_COUNT )
        {
            aSaveAsLB.Clear();
            const OUString* pFilters = pImpl->aFilterArr[nAppPos].getConstArray();
            if(!pImpl->aUIFilterArr[nAppPos].getLength())
            {
                pImpl->aUIFilterArr[nAppPos].realloc(pImpl->aFilterArr[nAppPos].getLength());
                OUString* pUIFilters = pImpl->aUIFilterArr[nAppPos].getArray();
                for(int nFilter = 0; nFilter < pImpl->aFilterArr[nAppPos].getLength(); nFilter++)
                {
                    Any aProps = pImpl->xFact->getByName(pFilters[nFilter]);
                    Sequence<PropertyValue> aProperties;
                    aProps >>= aProperties;
                    pUIFilters[nFilter] = lcl_ExtracUIName(aProperties);
                }
            }
            const OUString* pUIFilters = pImpl->aUIFilterArr[nAppPos].getConstArray();
            OUString sSelect;
            for(int i = 0; i < pImpl->aUIFilterArr[nAppPos].getLength(); i++)
            {
                USHORT nEntryPos = aSaveAsLB.InsertEntry(pUIFilters[i]);
                if ( pImpl->aODFArr[nAppPos][i] )
                    aSaveAsLB.SetEntryData( nEntryPos, (void*)pImpl );
                if(pFilters[i] == pImpl->aDefaultArr[nAppPos])
                    sSelect = pUIFilters[i];
            }
            if(sSelect.getLength())
                aSaveAsLB.SelectEntry(sSelect);
            aSaveAsFI.Show(pImpl->aDefaultReadonlyArr[nAppPos]);
            aSaveAsFT.Enable(!pImpl->aDefaultReadonlyArr[nAppPos]);
            aSaveAsLB.Enable(!pImpl->aDefaultReadonlyArr[nAppPos]);
        }
    }
    else
    {
        OUString sSelect = pBox->GetSelectEntry();
        USHORT nPos = aDocTypeLB.GetSelectEntryPos();
        const OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
        OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getArray();
        for(int i = 0; i < pImpl->aUIFilterArr[nPos].getLength(); i++)
            if(pUIFilters[i] == sSelect)
            {
                sSelect = pFilters[i];
                break;
            }

        pImpl->aDefaultArr[nPos] = sSelect;
    }

    ODFVersionHdl_Impl( &aSaveAsLB );
    return 0;
};

IMPL_LINK( SfxSaveTabPage, ODFVersionHdl_Impl, ListBox *, EMPTYARG )
{
    long nVersion = long( aODFVersionLB.GetEntryData( aODFVersionLB.GetSelectEntryPos() ) );
    bool bShown = SvtSaveOptions::ODFDefaultVersion( nVersion ) == SvtSaveOptions::ODFVER_012;
    if ( bShown )
    {
        bool bHasODFFormat = false;
        USHORT i = 0, nCount = aSaveAsLB.GetEntryCount();
        for ( ; i < nCount; ++ i )
        {
            if ( aSaveAsLB.GetEntryData(i) != NULL )
            {
                bHasODFFormat = true;
                break;
            }
        }

        bShown = !bHasODFFormat
                || ( aSaveAsLB.GetEntryData( aSaveAsLB.GetSelectEntryPos() ) != NULL );
    }

    aODFWarningFI.Show( !bShown );
    aODFWarningFT.Show( !bShown );

    return 0;
}

