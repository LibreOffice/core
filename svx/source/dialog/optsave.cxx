/*************************************************************************
 *
 *  $RCSfile: optsave.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 18:48:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#pragma hdrstop

#define _SVX_OPTSAVE_CXX

#include "optsave.hrc"
#include "dialogs.hrc"

#include "optsave.hxx"
#include "dialmgr.hxx"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
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

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)
// -------------------- --------------------------------------------------
class FilterWarningDialog_Impl : public ModalDialog
{
    OKButton        aOk;
    CancelButton    aCancel;
    FixedImage      aImage;
    FixedInfo       aFilterWarningFT;
//    CheckBox        aDontShowAgainCB;

    public:
        FilterWarningDialog_Impl(Window* pParent);

        void        SetFilterName(const String& rFilterUIName);
//        sal_Bool    IsCheckBoxSet() {return aDontShowAgainCB.IsChecked();}
};
// ----------------------------------------------------------------------
FilterWarningDialog_Impl::FilterWarningDialog_Impl(Window* pParent) :
    ModalDialog(pParent, SVX_RES( RID_SVXDLG_FILTER_WARNING ) ),
    aOk(                this, ResId(PB_OK               )),
    aCancel(            this, ResId(PB_CANCEL           )),
    aImage(             this, ResId(IMG_WARNING         )),
    aFilterWarningFT(   this, ResId(FT_FILTER_WARNING   ))
//    ,aDontShowAgainCB(   this, ResId(CB_DONT_SHOW_AGAIN  ))
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
    ConfigItem(C2U("Office.TypeDetection/Defaults"),
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
    Sequence<OUString>          aFilterArr[APP_COUNT];
    Sequence<OUString>          aUIFilterArr[APP_COUNT];
    CheckBox*                   m_pNoPrettyPrinting;
    int                         aDefaultIdxArr[APP_COUNT];
    sal_Bool                    bInitialized;

    SvxSaveTabPage_Impl();
    ~SvxSaveTabPage_Impl();
};

SvxSaveTabPage_Impl::SvxSaveTabPage_Impl()
    :m_pNoPrettyPrinting( NULL )
    ,bInitialized( sal_False )
{
    for (int i = 0; i < APP_COUNT; ++i)
        aDefaultIdxArr[i] = 0;
}

SvxSaveTabPage_Impl::~SvxSaveTabPage_Impl()
{
    delete m_pNoPrettyPrinting;
}

// class SvxSaveTabPage --------------------------------------------------

SfxSaveTabPage::SfxSaveTabPage( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SFXPAGE_SAVE ), rCoreSet ),
    aLoadFL(              this, ResId(  LB_LOAD         ) ),
    aLoadUserSettingsCB(  this, ResId(  CB_LOAD_SETTINGS) ),
    aSaveBox            ( this, ResId( GB_SAVE ) ),
    aDocInfoBtn         ( this, ResId( BTN_DOCINFO ) ),
    aBackupBtn          ( this, ResId( BTN_BACKUP ) ),
    aAutoSaveBtn        ( this, ResId( BTN_AUTOSAVE ) ),
    aAutoSaveEdit       ( this, ResId( ED_AUTOSAVE ) ),
    aMinuteText         ( this, ResId( FT_MINUTE ) ),
    aAutoSavePromptBtn  ( this, ResId( BTN_AUTOSAVEPROMPT ) ),
    aRelBox             ( this, ResId( GB_RELATIVE ) ),
    aRelFsysBtn         ( this, ResId( BTN_RELATIVE_FSYS ) ),
    aRelInetBtn         ( this, ResId( BTN_RELATIVE_INET ) ),
    aFilterFL           ( this, ResId( FL_FILTER ) ),
    aApplicationFT      ( this, ResId( FT_APP   ) ),
    aApplicationLB      ( this, ResId( LB_APP    ) ),
    aFiltersFT          ( this, ResId( FT_FILTER ) ),
    aFiltersLB          ( this, ResId( LB_FILTER ) ),
    pImpl(0)
{
    pImpl = new SvxSaveTabPage_Impl;
    pImpl->m_pNoPrettyPrinting = new CheckBox( this, ResId( BTN_NOPRETTYPRINTING ) );

    FreeResource();

    // correct the z-order for the no-pretty-printing checkbox
    Window* pNewPrev = &aAutoSavePromptBtn;
    Window* pNewNext = pNewPrev->GetWindow( WINDOW_NEXT );
    pImpl->m_pNoPrettyPrinting->SetZOrder( pNewNext, WINDOW_ZORDER_BEFOR );
    pNewPrev->SetZOrder( pImpl->m_pNoPrettyPrinting, WINDOW_ZORDER_BEFOR );

    Link aLink = LINK( this, SfxSaveTabPage, AutoClickHdl_Impl );
    aAutoSaveBtn.SetClickHdl( aLink );
    aAutoSaveEdit.SetMaxTextLen( 2 );

    SvtModuleOptions aModuleOpt;
    if ( !aModuleOpt.IsMath() )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_MATH ));
    if ( !aModuleOpt.IsDraw() )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_DRAW ));
    if ( !aModuleOpt.IsImpress() )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_IMPRESS ));
    if ( !aModuleOpt.IsCalc() )
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_CALC ));
    if ( !aModuleOpt.IsWriter() )
    {
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_WRITER ));
        aFiltersLB.RemoveEntry(aFiltersLB.GetEntryPos( (void*) APP_WRITER_WEB ));
    }

    Link aLk = LINK(this, SfxSaveTabPage, FilterHdl_Impl);
    aApplicationLB.SetSelectHdl(aLk);
    aFiltersLB.SetSelectHdl(aLk);
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
sal_Bool SfxSaveTabPage::SetDefaultFilter_Impl(Sequence<PropertyValue>& rProperties,
        sal_Bool bSet/*, sal_Bool& bShowWarning*/)
{
    BOOL bRet = TRUE;
    PropertyValue* pProperties = rProperties.getArray();
    for(int nProp = 0; nProp < rProperties.getLength(); nProp++)
    {
        if(!pProperties[nProp].Name.compareToAscii("Flags"))
        {
            sal_Int32 nFlags; pProperties[nProp].Value >>= nFlags;
            if(bSet)
                nFlags |= SFX_FILTER_DEFAULT;
            else
                nFlags &= ~SFX_FILTER_DEFAULT;
            if(/*bShowWarning & */bSet && (nFlags & SFX_FILTER_ALIEN))
            {
                FilterWarningDialog_Impl aDlg(this);
                aDlg.SetFilterName(lcl_ExtractUIName(rProperties));
                if(RET_OK != aDlg.Execute())
                    bRet = FALSE;
//                if(aDlg.IsCheckBoxSet())
//                    bShowWarning = sal_False;
            }
            if(bRet)
                pProperties[nProp].Value <<= nFlags;
            return bRet;
        }
    }
    DBG_ERROR("Property not found");
    return bRet;
}
// -----------------------------------------------------------------------
BOOL SfxSaveTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(aLoadUserSettingsCB.IsChecked() != aLoadUserSettingsCB.GetSavedValue())
    {
        SvtSaveOptions aSaveOpt;
        aSaveOpt.SetLoadUserSettings(aLoadUserSettingsCB.IsChecked());
    }

    BOOL bModified = FALSE;

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

    if ( pImpl->m_pNoPrettyPrinting->IsChecked() != pImpl->m_pNoPrettyPrinting->GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PRETTYPRINTING ), !pImpl->m_pNoPrettyPrinting->IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSaveBtn.IsChecked() != aAutoSaveBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVE ),
                               aAutoSaveBtn.IsChecked() ) );
        bModified |= TRUE;
    }

    if ( aAutoSavePromptBtn.IsChecked() != aAutoSavePromptBtn.GetSavedValue() )
    {
        rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_AUTOSAVEPROMPT ),
                               aAutoSavePromptBtn.IsChecked() ) );
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

    if(pImpl->xFact.is())
    {
        sal_Bool bFlush = sal_False;
//        SvxAlienFilterWarningConfig_Impl aWarningConfig;
        for(int n = 0; n < aApplicationLB.GetEntryCount() && n < APP_COUNT; n++)
        {
            if(pImpl->aDefaultIdxArr[n])
            {
                bFlush = sal_True;
                OUString sOldFilter = pImpl->aFilterArr[n].getConstArray()[0];
                OUString sNewFilter = pImpl->aFilterArr[n].getConstArray()[pImpl->aDefaultIdxArr[n]];

                Sequence<PropertyValue> aOldProperties;
                Sequence<PropertyValue> aNewProperties;
                Any aOldProps;
                aOldProps = pImpl->xFact->getByName(sOldFilter);
                aOldProps >>= aOldProperties;

                Any aNewProps;
                aNewProps = pImpl->xFact->getByName(sNewFilter);
                aNewProps >>= aNewProperties;
//                sal_Bool bWarning = aWarningConfig.IsWarning();
                SetDefaultFilter_Impl(aOldProperties, sal_False/*, bWarning*/);
                if(SetDefaultFilter_Impl(aNewProperties, sal_True/*, bWarning*/))
                {
                    aOldProps <<= aOldProperties;
                    pImpl->xFact->replaceByName(sOldFilter, aOldProps);
                    aNewProps <<= aNewProperties;
                    pImpl->xFact->replaceByName(sNewFilter, aNewProps);
                }
//                if(!bWarning)
//                    aWarningConfig.ResetWarning();
            }
        }
        Reference<XFlushable> xFlush(pImpl->xFact, UNO_QUERY);
        if(xFlush.is())
            xFlush->flush();
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SfxSaveTabPage::Reset( const SfxItemSet& rSet )
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
            if(pImpl->xFact.is())
            {
                for(int n = 0; n < aApplicationLB.GetEntryCount(); n++)
                {
                    long nData = (long) aApplicationLB.GetEntryData(n);
                    OUString sCommand;
                    sCommand = C2U("_query_%1:iflags=");
                    sCommand += String::CreateFromInt32(SFX_FILTER_IMPORT|SFX_FILTER_EXPORT);
                    sCommand += C2U(":eflags=");
                    sCommand += String::CreateFromInt32(SFX_FILTER_NOTINFILEDLG);
                    sCommand += C2U(":default_first");
                    String sReplace;
                    switch(nData)
                    {
                        case  APP_WRITER     : sReplace = C2U("writer");  break;
                        case  APP_WRITER_WEB : sReplace = C2U("web");   break;
                        case  APP_CALC       : sReplace = C2U("calc");break;
                        case  APP_IMPRESS    : sReplace = C2U("impress");break;
                        case  APP_DRAW       : sReplace = C2U("draw");break;
                        case  APP_MATH       : sReplace = C2U("math");break;
                        default: DBG_ERROR("illegal user data");
                    }
                    String sTmp(sCommand);
                    sTmp.SearchAndReplaceAscii("%1", sReplace);
                    sCommand = sTmp;
                    Any aVal = pImpl->xFact->getByName(sCommand);
                    aVal >>= pImpl->aFilterArr[nData];
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

    SfxItemState eItemState = SFX_ITEM_UNKNOWN;
//  const SfxItemSet& rSet = GetItemSet();
    const SfxBoolItem* pAttr = NULL;
    const SfxUInt16Item* pMinAttr = NULL;

    // if an item does not exist inside item set -> disable the corresponding control!
    aDocInfoBtn.Enable(FALSE);
    aBackupBtn.Enable(FALSE);
    aAutoSaveBtn.Enable(FALSE);
    aAutoSavePromptBtn.Enable(FALSE);

    for ( USHORT i = SID_ATTR_DOCINFO; i < SID_ATTR_AUTOSAVEMINUTE; ++i  )
    {
        pAttr = (SfxBoolItem*)GetItem( rSet, i );

        CheckBox* pBox      = NULL;
        sal_Bool bInverse   = sal_False;

        switch ( i )
        {
            case SID_ATTR_DOCINFO:
                pBox = &aDocInfoBtn;
                break;

            case SID_ATTR_BACKUP:
                pBox = &aBackupBtn;
                break;

            case SID_ATTR_AUTOSAVE:
                pBox = &aAutoSaveBtn;
                break;

            case SID_ATTR_AUTOSAVEPROMPT:
                pBox = &aAutoSavePromptBtn;
                break;

            case SID_ATTR_INDEP_METAFILE:
                // neue ID
                break;
        }

        if ( pBox )
        {
            pBox->Check( pAttr ? pAttr->GetValue() : FALSE );
            pBox->Enable( pAttr ? TRUE : FALSE );
        }
    }

    // the pretty printing
    {
        const SfxPoolItem* pPrettyPrinting = GetItem( rSet, SID_ATTR_PRETTYPRINTING );
        pAttr = PTR_CAST( SfxBoolItem, pPrettyPrinting );
        pImpl->m_pNoPrettyPrinting->Check( pAttr ? !pAttr->GetValue() : sal_True );
    }


    USHORT nWhich = GetWhich( SID_ATTR_AUTOSAVEMINUTE );
    eItemState =
        rSet.GetItemState( nWhich, FALSE, (const SfxPoolItem**)&pMinAttr );

    if ( eItemState <= SFX_ITEM_DEFAULT )
        pMinAttr = NULL;

    aAutoSaveEdit.SetValue( pMinAttr ? (long)pMinAttr->GetValue() : 15 );

    const SfxPoolItem* pItem = 0;

    // relativ speichern
    nWhich = GetWhich( SID_SAVEREL_FSYS );

    if ( SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem ) )
        aRelFsysBtn.Check( ( (const SfxBoolItem*)pItem )->GetValue() );

    nWhich = GetWhich( SID_SAVEREL_INET );

    if ( SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem ) )
        aRelInetBtn.Check( ( (const SfxBoolItem*)pItem )->GetValue() );

    AutoClickHdl_Impl( &aAutoSaveBtn );

    aDocInfoBtn.SaveValue();
    aBackupBtn.SaveValue();
    pImpl->m_pNoPrettyPrinting->SaveValue();
    aAutoSaveBtn.SaveValue();
    aAutoSaveEdit.SaveValue();
    aAutoSavePromptBtn.SaveValue();

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
            aAutoSavePromptBtn.Enable();
            aAutoSaveEdit.GrabFocus();
        }
        else
        {
            aAutoSaveEdit.Disable();
            aMinuteText.Disable();
            aAutoSavePromptBtn.Disable();
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
            OUString sSelect;
            if(!pImpl->aUIFilterArr[nPos].getLength())
            {
                pImpl->aUIFilterArr[nPos].realloc(pImpl->aFilterArr[nPos].getLength());
                const OUString* pFilters = pImpl->aFilterArr[nPos].getConstArray();
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
            for(int i = 0; i < pImpl->aUIFilterArr[nPos].getLength(); i++)
            {
                aFiltersLB.InsertEntry(pUIFilters[i]);
                if(i == pImpl->aDefaultIdxArr[nPos])
                    sSelect = pUIFilters[i];
            }
            aFiltersLB.SelectEntry(sSelect);
        }
    }
    else
    {
        OUString sSelect = pBox->GetSelectEntry();
        USHORT nPos = aApplicationLB.GetSelectEntryPos();
        const OUString* pUIFilters = pImpl->aUIFilterArr[nPos].getConstArray();
        for(int i = 0; i < pImpl->aUIFilterArr[nPos].getLength(); i++)
        {
            if(sSelect == pUIFilters[i])
                pImpl->aDefaultIdxArr[nPos] = i;
        }
    }
    return 0;
};

