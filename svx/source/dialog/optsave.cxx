/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optsave.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 09:19:52 $
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
#include "dialogs.hrc"

#include "optsave.hxx"
#include "dialmgr.hxx"

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

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace rtl;
using namespace comphelper;

#define C2U(cChar)                  OUString::createFromAscii(cChar)
#define C2S(cChar)                  String( RTL_CONSTASCII_STRINGPARAM(cChar) )
#define CFG_PAGE_AND_GROUP          C2S("General"), C2S("LoadSave")
// !! you have to update these index, if you changed the list of the child windows !!
#define WININDEX_AUTOSAVE           ((USHORT)6)
#define WININDEX_NOPRETTYPRINTING   ((USHORT)9)

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
    aOk(                this, ResId(PB_OK               )),
    aCancel(            this, ResId(PB_CANCEL           )),
    aImage(             this, ResId(IMG_WARNING         )),
    aFilterWarningFT(   this, ResId(FT_FILTER_WARNING   ))
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
    com::sun::star::uno::Sequence<rtl::OUString> aPropNames;

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
    Reference<XNameContainer>   xFact;
    Sequence< ::rtl::OUString>   aFilterArr[APP_COUNT];
    Sequence<sal_Bool>          aAlienArr[APP_COUNT];
    Sequence<OUString>          aUIFilterArr[APP_COUNT];
    rtl::OUString               aDefaultArr[APP_COUNT];
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
    aLoadFL             ( this, ResId( LB_LOAD ) ),
    aLoadUserSettingsCB ( this, ResId( CB_LOAD_SETTINGS ) ),
    aSaveBox            ( this, ResId( GB_SAVE ) ),
    aDocInfoBtn         ( this, ResId( BTN_DOCINFO ) ),
    aBackupFI           ( this, ResId( FI_BACKUP ) ),
    aBackupBtn          ( this, ResId( BTN_BACKUP ) ),
    aAutoSaveBtn        ( this, ResId( BTN_AUTOSAVE ) ),
    aAutoSaveEdit       ( this, ResId( ED_AUTOSAVE ) ),
    aNoPrettyPrintingBtn( this, ResId( BTN_NOPRETTYPRINTING ) ),
    aMinuteText         ( this, ResId( FT_MINUTE ) ),
    aWarnAlienFormatBtn ( this, ResId( BTN_WARNALIENFORMAT ) ),
    aRelBox             ( this, ResId( GB_RELATIVE ) ),
    aRelFsysBtn         ( this, ResId( BTN_RELATIVE_FSYS ) ),
    aRelInetBtn         ( this, ResId( BTN_RELATIVE_INET ) ),
    aFilterFL           ( this, ResId( FL_FILTER ) ),
    aApplicationFT      ( this, ResId( FT_APP ) ),
    aApplicationLB      ( this, ResId( LB_APP ) ),
    aFiltersFT          ( this, ResId( FT_FILTER ) ),
    aFiltersFI          ( this, ResId( FI_FILTER ) ),
    aFiltersLB          ( this, ResId( LB_FILTER ) ),
    aWarningFT          ( this, ResId( FT_WARN ) ),
    pImpl( new SvxSaveTabPage_Impl )
{
    FreeResource();

    Link aLink = LINK( this, SfxSaveTabPage, AutoClickHdl_Impl );
    aAutoSaveBtn.SetClickHdl( aLink );
    aAutoSaveEdit.SetMaxTextLen( 2 );

    SvtModuleOptions aModuleOpt;
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_MATH ));
    else
    {
        pImpl->aDefaultArr[APP_MATH] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_MATH);
        pImpl->aDefaultReadonlyArr[APP_MATH] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_MATH);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_DRAW ));
    else
    {
        pImpl->aDefaultArr[APP_DRAW] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_DRAW);
        pImpl->aDefaultReadonlyArr[APP_DRAW] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_DRAW);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_IMPRESS ));
    else
    {
        pImpl->aDefaultArr[APP_IMPRESS] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_IMPRESS);
        pImpl->aDefaultReadonlyArr[APP_IMPRESS] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_IMPRESS);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_CALC ));
    else
    {
        pImpl->aDefaultArr[APP_CALC] = aModuleOpt.GetFactoryDefaultFilter(SvtModuleOptions::E_CALC);
        pImpl->aDefaultReadonlyArr[APP_CALC] = aModuleOpt.IsDefaultFilterReadonly(SvtModuleOptions::E_CALC);
    }
    if ( !aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
    {
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_WRITER ));
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_WRITER_WEB ));
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_WRITER_GLOBAL ));
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

    Link aLk = LINK(this, SfxSaveTabPage, FilterHdl_Impl);
    aApplicationLB.SetSelectHdl(aLk);
    aFiltersLB.SetSelectHdl(aLk);

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
    const ::rtl::OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
    sal_Bool bAlien = sal_False;
    ::rtl::OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getArray();
    ::rtl::OUString sUIName;
    for(int nFilter = 0; nFilter < pImpl->aFilterArr[nPos].getLength(); nFilter++)
    {
        if( pImpl->aDefaultArr[nPos] == pFilters[nFilter] )
        {
            bAlien = pImpl->aAlienArr[nPos][nFilter];
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
    USHORT nWinIndex = WININDEX_NOPRETTYPRINTING;
    SvtOptionsDialogOptions aOptionsDlgOpt;

    if ( aOptionsDlgOpt.IsOptionHidden( C2S("Backup"), CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "Backup"
        aBackupFI.Hide();
        aBackupBtn.Hide();
        // the other controls have to move upwards the height of checkbox + space
        nDelta = aAutoSaveBtn.GetPosPixel().Y() - aBackupBtn.GetPosPixel().Y();
    }

    if ( aOptionsDlgOpt.IsOptionHidden( C2S("AutoSave"), CFG_PAGE_AND_GROUP ) )
    {
        // hide controls of "AutoSave"
        aAutoSaveBtn.Hide();
        aAutoSaveEdit.Hide();
        aMinuteText.Hide();
        // the other controls have to move upwards the height of checkbox + space
        nDelta += aNoPrettyPrintingBtn.GetPosPixel().Y() - aAutoSaveBtn.GetPosPixel().Y();
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

    if ( aDocInfoBtn.IsChecked() != aDocInfoBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_DOCINFO ),
                               aDocInfoBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aBackupBtn.IsEnabled() && aBackupBtn.IsChecked() != aBackupBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_BACKUP ),
                               aBackupBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aNoPrettyPrintingBtn.IsChecked() != aNoPrettyPrintingBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PRETTYPRINTING ), !aNoPrettyPrintingBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSaveBtn.IsChecked() != aAutoSaveBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVE ),
                               aAutoSaveBtn.IsChecked() ) );
        bModified |= TRUE;
    }
    if ( aWarnAlienFormatBtn.IsChecked() != aWarnAlienFormatBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_WARNALIENFORMAT ),
                               aWarnAlienFormatBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSaveEdit.GetText() != aAutoSaveEdit.GetSavedValue() )
    {
        rSet.Put( SfxUInt16Item( GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                 (UINT16)aAutoSaveEdit.GetValue() ) );
        bModified |= TRUE;
    }
    // relativ speichern
    if ( aRelFsysBtn.IsChecked() != aRelFsysBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_SAVEREL_FSYS ),
                               aRelFsysBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aRelInetBtn.IsChecked() != aRelInetBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_SAVEREL_INET ),
                               aRelInetBtn.IsChecked() ) );
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

void SfxSaveTabPage::Reset( const SfxItemSet& )
{
    SvtSaveOptions aSaveOpt;
    aLoadUserSettingsCB.Check(aSaveOpt.IsLoadUserSettings());
    aLoadUserSettingsCB.SaveValue();

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
                for(int n = 0; n < aApplicationLB.GetEntryCount(); n++)
                {
                    long nData = (long) aApplicationLB.GetEntryData(n);
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
                    SequenceAsVector< ::rtl::OUString > lList;
                    SequenceAsVector< sal_Bool > lAlienList;
                    while(xList->hasMoreElements())
                    {
                        SequenceAsHashMap aFilter(xList->nextElement());
                        OUString sFilter = aFilter.getUnpackedValueOrDefault(OUString::createFromAscii("Name"),OUString());
                        if (sFilter.getLength())
                        {
                            sal_Int32 nFlags = aFilter.getUnpackedValueOrDefault(OUString::createFromAscii("Flags"),sal_Int32());
                            lList.push_back(sFilter);
                            lAlienList.push_back(0 != (nFlags & SFX_FILTER_ALIEN));
                        }
                    }
                    pImpl->aFilterArr[nData] = lList.getAsConstList();
                    pImpl->aAlienArr[nData] = lAlienList.getAsConstList();
                }
            }
            aApplicationLB.SelectEntryPos(0);
            FilterHdl_Impl(&aApplicationLB);
        }
        catch(Exception& )
        {
            DBG_ERROR("exception in FilterFactory access")
        }

        pImpl->bInitialized = sal_True;
    }

    aDocInfoBtn.Check(aSaveOpt.IsDocInfoSave());
//    aDocInfoBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_DOCINFSAVE));

    aBackupBtn.Check(aSaveOpt.IsBackup());
    BOOL bBackupRO = aSaveOpt.IsReadOnly(SvtSaveOptions::E_BACKUP);
    aBackupBtn.Enable(!bBackupRO);
    aBackupFI.Show(bBackupRO);

    aAutoSaveBtn.Check(aSaveOpt.IsAutoSave());
    aWarnAlienFormatBtn.Check(aSaveOpt.IsWarnAlienFormat());
    aWarnAlienFormatBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_WARNALIENFORMAT));
//    aAutoSaveBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVE));

    // the pretty printing
    aNoPrettyPrintingBtn.Check( !aSaveOpt.IsPrettyPrinting());
//    aNoPrettyPrintingBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_DOPRETTYPRINTING ));


    aAutoSaveEdit.SetValue( aSaveOpt.GetAutoSaveTime() );
//    aAutoSaveEdit.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_AUTOSAVETIME));

    // relativ speichern
    aRelFsysBtn.Check( aSaveOpt.IsSaveRelFSys() );
//    aRelFsysBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELFSYS));

    aRelInetBtn.Check( aSaveOpt.IsSaveRelINet() );
//    aRelInetBtn.Enable(!aSaveOpt.IsReadOnly(SvtSaveOptions::E_SAVERELINET));

    AutoClickHdl_Impl( &aAutoSaveBtn );

    aDocInfoBtn.SaveValue();
    aBackupBtn.SaveValue();
    aWarnAlienFormatBtn.SaveValue();
    aNoPrettyPrintingBtn.SaveValue();
    aAutoSaveBtn.SaveValue();
    aAutoSaveEdit.SaveValue();
//  aAutoSavePromptBtn.SaveValue();

    aRelFsysBtn.SaveValue();
    aRelInetBtn.SaveValue();
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxSaveTabPage, AutoClickHdl_Impl, CheckBox *, pBox )
{
    if ( pBox == &aAutoSaveBtn )
    {
        if ( aAutoSaveBtn.IsChecked() )
        {
            aAutoSaveEdit.Enable();
            aMinuteText.Enable();
//          aAutoSavePromptBtn.Enable();
            aAutoSaveEdit.GrabFocus();
        }
        else
        {
            aAutoSaveEdit.Disable();
            aMinuteText.Disable();
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
/* -----------------------------05.04.01 13:37--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SfxSaveTabPage, FilterHdl_Impl, ListBox *, pBox )
{
    if(&aApplicationLB == pBox)
    {
        USHORT nPos = pBox->GetSelectEntryPos();
        if(nPos < APP_COUNT)
        {
            aFiltersLB.Clear();
            const ::rtl::OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
            if(!pImpl->aUIFilterArr[nPos].getLength())
            {
                pImpl->aUIFilterArr[nPos].realloc(pImpl->aFilterArr[nPos].getLength());
                OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getArray();
                for(int nFilter = 0; nFilter < pImpl->aFilterArr[nPos].getLength(); nFilter++)
                {
                    Any aProps = pImpl->xFact->getByName(pFilters[nFilter]);
                    Sequence<PropertyValue> aProperties;
                    aProps >>= aProperties;
                    pUIFilters[nFilter] = lcl_ExtracUIName(aProperties);
                }
            }
            const OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getConstArray();
            OUString sSelect;
            for(int i = 0; i < pImpl->aUIFilterArr[nPos].getLength(); i++)
            {
                aFiltersLB.InsertEntry(pUIFilters[i]);
                if(pFilters[i] == pImpl->aDefaultArr[nPos])
                    sSelect = pUIFilters[i];
            }
            if(sSelect.getLength())
                aFiltersLB.SelectEntry(sSelect);
            aFiltersFI.Show(pImpl->aDefaultReadonlyArr[nPos]);
            aFiltersFT.Enable(!pImpl->aDefaultReadonlyArr[nPos]);
            aFiltersLB.Enable(!pImpl->aDefaultReadonlyArr[nPos]);
        }
    }
    else
    {
        OUString sSelect = pBox->GetSelectEntry();
        USHORT nPos = aApplicationLB.GetSelectEntryPos();
        const ::rtl::OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
        OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getArray();
        for(int i = 0; i < pImpl->aUIFilterArr[nPos].getLength(); i++)
            if(pUIFilters[i] == sSelect)
            {
                sSelect = pFilters[i];
                break;
            }

        pImpl->aDefaultArr[nPos] = sSelect;
    }
    return 0;
};

