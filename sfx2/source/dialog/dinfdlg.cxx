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
#include <tools/datetime.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <unotools/datetime.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/cmdoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <unotools/useroptions.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/imagemgr.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <memory>

#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTimeWithTimezone.hpp>
#include <com/sun/star/util/DateWithTimezone.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/CmisProperty.hpp>

#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <helper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <documentfontsdialog.hxx>
#include <dinfdlg.hrc>
#include <sfx2/strings.hrc>
#include <strings.hxx>
#include <tools/diagnose_ex.h>
#include "securitypage.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

struct CustomProperty
{
    OUString         m_sName;
    css::uno::Any    m_aValue;

    CustomProperty( const OUString& sName, const css::uno::Any& rValue ) :
        m_sName( sName ), m_aValue( rValue ) {}
};

SfxPoolItem* SfxDocumentInfoItem::CreateDefault() { return new SfxDocumentInfoItem; }

namespace {

OUString CreateSizeText( sal_Int64 nSize )
{
    OUString aUnitStr = " " + SfxResId(STR_BYTES);
    sal_Int64 nSize1 = nSize;
    sal_Int64 nSize2 = nSize1;
    sal_Int64 nMega = 1024 * 1024;
    sal_Int64 nGiga = nMega * 1024;
    double fSize = nSize;
    int nDec = 0;

    if ( nSize1 >= 10000 && nSize1 < nMega )
    {
        nSize1 /= 1024;
        aUnitStr = " " + SfxResId(STR_KB);
        fSize /= 1024;
        nDec = 0;
    }
    else if ( nSize1 >= nMega && nSize1 < nGiga )
    {
        nSize1 /= nMega;
        aUnitStr = " " + SfxResId(STR_MB);
        fSize /= nMega;
        nDec = 2;
    }
    else if ( nSize1 >= nGiga )
    {
        nSize1 /= nGiga;
        aUnitStr = " " + SfxResId(STR_GB);
        fSize /= nGiga;
        nDec = 3;
    }
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    OUString aSizeStr = rLocaleWrapper.getNum( nSize1, 0 ) + aUnitStr;
    if ( nSize1 < nSize2 )
    {
        aSizeStr = ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                rLocaleWrapper.getNumDecimalSep()[0] )
                 + aUnitStr
                 + " ("
                 + rLocaleWrapper.getNum( nSize2, 0 )
                 + " "
                 + SfxResId(STR_BYTES)
                 + ")";
    }
    return aSizeStr;
}

OUString ConvertDateTime_Impl( const OUString& rName,
    const util::DateTime& uDT, const LocaleDataWrapper& rWrapper )
{
     Date aD(uDT);
     tools::Time aT(uDT);
     const OUString aDelim( ", " );
     OUString aStr = rWrapper.getDate( aD )
                   + aDelim
                   + rWrapper.getTime( aT );
     OUString aAuthor = comphelper::string::stripStart(rName, ' ');
     if (!aAuthor.isEmpty())
     {
        aStr += aDelim + aAuthor;
     }
     return aStr;
}

}


SfxDocumentInfoItem::SfxDocumentInfoItem()
    : SfxStringItem()
    , m_AutoloadDelay(0)
    , m_AutoloadURL()
    , m_isAutoloadEnabled(false)
    , m_DefaultTarget()
    , m_TemplateName()
    , m_Author()
    , m_CreationDate()
    , m_ModifiedBy()
    , m_ModificationDate()
    , m_PrintedBy()
    , m_PrintDate()
    , m_EditingCycles(0)
    , m_EditingDuration(0)
    , m_Description()
    , m_Keywords()
    , m_Subject()
    , m_Title()
    , m_bHasTemplate( true )
    , m_bDeleteUserData( false )
    , m_bUseUserData( true )
    , m_bUseThumbnailSave( true )
{
}

SfxDocumentInfoItem::SfxDocumentInfoItem( const OUString& rFile,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Sequence<document::CmisProperty>& i_cmisProps,
        bool bIs, bool _bIs )
    : SfxStringItem( SID_DOCINFO, rFile )
    , m_AutoloadDelay( i_xDocProps->getAutoloadSecs() )
    , m_AutoloadURL( i_xDocProps->getAutoloadURL() )
    , m_isAutoloadEnabled( (m_AutoloadDelay > 0) || !m_AutoloadURL.isEmpty() )
    , m_DefaultTarget( i_xDocProps->getDefaultTarget() )
    , m_TemplateName( i_xDocProps->getTemplateName() )
    , m_Author( i_xDocProps->getAuthor() )
    , m_CreationDate( i_xDocProps->getCreationDate() )
    , m_ModifiedBy( i_xDocProps->getModifiedBy() )
    , m_ModificationDate( i_xDocProps->getModificationDate() )
    , m_PrintedBy( i_xDocProps->getPrintedBy() )
    , m_PrintDate( i_xDocProps->getPrintDate() )
    , m_EditingCycles( i_xDocProps->getEditingCycles() )
    , m_EditingDuration( i_xDocProps->getEditingDuration() )
    , m_Description( i_xDocProps->getDescription() )
    , m_Keywords( ::comphelper::string::convertCommaSeparated(
                    i_xDocProps->getKeywords()) )
    , m_Subject( i_xDocProps->getSubject() )
    , m_Title( i_xDocProps->getTitle() )
    , m_bHasTemplate( true )
    , m_bDeleteUserData( false )
    , m_bUseUserData( bIs )
    , m_bUseThumbnailSave( _bIs )
{
    try
    {
        Reference< beans::XPropertyContainer > xContainer = i_xDocProps->getUserDefinedProperties();
        if ( xContainer.is() )
        {
            Reference < beans::XPropertySet > xSet( xContainer, UNO_QUERY );
            const Sequence< beans::Property > lProps = xSet->getPropertySetInfo()->getProperties();
            for ( const beans::Property& rProp : lProps )
            {
                // "fix" property? => not a custom property => ignore it!
                if (!(rProp.Attributes & css::beans::PropertyAttribute::REMOVABLE))
                {
                    SAL_WARN( "sfx.dialog", "non-removable user-defined property?");
                    continue;
                }

                uno::Any aValue = xSet->getPropertyValue(rProp.Name);
                std::unique_ptr<CustomProperty> pProp(new CustomProperty( rProp.Name, aValue ));
                m_aCustomProperties.push_back( std::move(pProp) );
            }
        }

        // get CMIS properties
        m_aCmisProperties = i_cmisProps;
    }
    catch ( Exception& ) {}
}


SfxDocumentInfoItem::SfxDocumentInfoItem( const SfxDocumentInfoItem& rItem )
    : SfxStringItem( rItem )
    , m_AutoloadDelay( rItem.getAutoloadDelay() )
    , m_AutoloadURL( rItem.getAutoloadURL() )
    , m_isAutoloadEnabled( rItem.isAutoloadEnabled() )
    , m_DefaultTarget( rItem.getDefaultTarget() )
    , m_TemplateName( rItem.getTemplateName() )
    , m_Author( rItem.getAuthor() )
    , m_CreationDate( rItem.getCreationDate() )
    , m_ModifiedBy( rItem.getModifiedBy() )
    , m_ModificationDate( rItem.getModificationDate() )
    , m_PrintedBy( rItem.getPrintedBy() )
    , m_PrintDate( rItem.getPrintDate() )
    , m_EditingCycles( rItem.getEditingCycles() )
    , m_EditingDuration( rItem.getEditingDuration() )
    , m_Description( rItem.getDescription() )
    , m_Keywords( rItem.getKeywords() )
    , m_Subject( rItem.getSubject() )
    , m_Title( rItem.getTitle() )
    , m_bHasTemplate( rItem.m_bHasTemplate )
    , m_bDeleteUserData( rItem.m_bDeleteUserData )
    , m_bUseUserData( rItem.m_bUseUserData )
    , m_bUseThumbnailSave( rItem.m_bUseThumbnailSave )
{
    for (auto const & pOtherProp : rItem.m_aCustomProperties)
    {
        std::unique_ptr<CustomProperty> pProp(new CustomProperty( pOtherProp->m_sName,
                                                    pOtherProp->m_aValue ));
        m_aCustomProperties.push_back( std::move(pProp) );
    }

    m_aCmisProperties = rItem.m_aCmisProperties;
}

SfxDocumentInfoItem::~SfxDocumentInfoItem()
{
    ClearCustomProperties();
}

SfxDocumentInfoItem* SfxDocumentInfoItem::Clone( SfxItemPool * ) const
{
    return new SfxDocumentInfoItem( *this );
}

bool SfxDocumentInfoItem::operator==( const SfxPoolItem& rItem) const
{
    if (!SfxStringItem::operator==(rItem))
        return false;
    const SfxDocumentInfoItem& rInfoItem(static_cast<const SfxDocumentInfoItem&>(rItem));

    return
         m_AutoloadDelay        == rInfoItem.m_AutoloadDelay     &&
         m_AutoloadURL          == rInfoItem.m_AutoloadURL       &&
         m_isAutoloadEnabled    == rInfoItem.m_isAutoloadEnabled &&
         m_DefaultTarget        == rInfoItem.m_DefaultTarget     &&
         m_Author               == rInfoItem.m_Author            &&
         m_CreationDate         == rInfoItem.m_CreationDate      &&
         m_ModifiedBy           == rInfoItem.m_ModifiedBy        &&
         m_ModificationDate     == rInfoItem.m_ModificationDate  &&
         m_PrintedBy            == rInfoItem.m_PrintedBy         &&
         m_PrintDate            == rInfoItem.m_PrintDate         &&
         m_EditingCycles        == rInfoItem.m_EditingCycles     &&
         m_EditingDuration      == rInfoItem.m_EditingDuration   &&
         m_Description          == rInfoItem.m_Description       &&
         m_Keywords             == rInfoItem.m_Keywords          &&
         m_Subject              == rInfoItem.m_Subject           &&
         m_Title                == rInfoItem.m_Title             &&
         m_aCustomProperties.size() == rInfoItem.m_aCustomProperties.size() &&
         std::equal(m_aCustomProperties.begin(), m_aCustomProperties.end(),
            rInfoItem.m_aCustomProperties.begin()) &&
         m_aCmisProperties.getLength() == rInfoItem.m_aCmisProperties.getLength();
}


void SfxDocumentInfoItem::resetUserData(const OUString & i_rAuthor)
{
    m_Author = i_rAuthor;
    DateTime now( DateTime::SYSTEM );
    m_CreationDate = now.GetUNODateTime();
    m_ModifiedBy = OUString();
    m_PrintedBy = OUString();
    m_ModificationDate = util::DateTime();
    m_PrintDate = util::DateTime();
    m_EditingDuration = 0;
    m_EditingCycles = 1;
}


void SfxDocumentInfoItem::UpdateDocumentInfo(
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        bool i_bDoNotUpdateUserDefined) const
{
    if (isAutoloadEnabled()) {
        i_xDocProps->setAutoloadSecs(getAutoloadDelay());
        i_xDocProps->setAutoloadURL(getAutoloadURL());
    } else {
        i_xDocProps->setAutoloadSecs(0);
        i_xDocProps->setAutoloadURL(OUString());
    }
    i_xDocProps->setDefaultTarget(getDefaultTarget());
    i_xDocProps->setAuthor(getAuthor());
    i_xDocProps->setCreationDate(getCreationDate());
    i_xDocProps->setModifiedBy(getModifiedBy());
    i_xDocProps->setModificationDate(getModificationDate());
    i_xDocProps->setPrintedBy(getPrintedBy());
    i_xDocProps->setPrintDate(getPrintDate());
    i_xDocProps->setEditingCycles(getEditingCycles());
    i_xDocProps->setEditingDuration(getEditingDuration());
    i_xDocProps->setDescription(getDescription());
    i_xDocProps->setKeywords(
        ::comphelper::string::convertCommaSeparated(getKeywords()));
    i_xDocProps->setSubject(getSubject());
    i_xDocProps->setTitle(getTitle());

    // this is necessary in case of replaying a recorded macro:
    // in this case, the macro may contain the 4 old user-defined DocumentInfo
    // fields, but not any of the DocumentInfo properties;
    // as a consequence, most of the UserDefined properties of the
    // DocumentProperties would be summarily deleted here, which does not
    // seem like a good idea.
    if (i_bDoNotUpdateUserDefined)
        return;

    try
    {
        Reference< beans::XPropertyContainer > xContainer = i_xDocProps->getUserDefinedProperties();
        Reference < beans::XPropertySet > xSet( xContainer, UNO_QUERY );
        Reference< beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();
        const Sequence< beans::Property > lProps = xSetInfo->getProperties();
        for ( const beans::Property& rProp : lProps )
        {
            if (rProp.Attributes & css::beans::PropertyAttribute::REMOVABLE)
            {
                xContainer->removeProperty( rProp.Name );
            }
        }

        for (auto const & pProp : m_aCustomProperties)
        {
            try
            {
                xContainer->addProperty( pProp->m_sName,
                    beans::PropertyAttribute::REMOVABLE, pProp->m_aValue );
            }
            catch ( Exception const & )
            {
                TOOLS_WARN_EXCEPTION( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while adding custom properties" );
            }
        }
    }
    catch ( Exception const & )
    {
        TOOLS_WARN_EXCEPTION( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while removing custom properties" );
    }
}


void SfxDocumentInfoItem::SetDeleteUserData( bool bSet )
{
    m_bDeleteUserData = bSet;
}


void SfxDocumentInfoItem::SetUseUserData( bool bSet )
{
    m_bUseUserData = bSet;
}

void SfxDocumentInfoItem::SetUseThumbnailSave( bool bSet )
{
    m_bUseThumbnailSave = bSet;
}

std::vector< std::unique_ptr<CustomProperty> > SfxDocumentInfoItem::GetCustomProperties() const
{
    std::vector< std::unique_ptr<CustomProperty> > aRet;
    for (auto const & pOtherProp : m_aCustomProperties)
    {
        std::unique_ptr<CustomProperty> pProp(new CustomProperty( pOtherProp->m_sName,
                                                    pOtherProp->m_aValue ));
        aRet.push_back( std::move(pProp) );
    }

    return aRet;
}

void SfxDocumentInfoItem::ClearCustomProperties()
{
    m_aCustomProperties.clear();
}

void SfxDocumentInfoItem::AddCustomProperty( const OUString& sName, const Any& rValue )
{
    std::unique_ptr<CustomProperty> pProp(new CustomProperty( sName, rValue ));
    m_aCustomProperties.push_back( std::move(pProp) );
}


void SfxDocumentInfoItem::SetCmisProperties( const Sequence< document::CmisProperty >& cmisProps)
{
    m_aCmisProperties = cmisProps;
}

bool SfxDocumentInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    OUString aValue;
    sal_Int32 nValue = 0;
    bool bValue = false;
    bool bIsInt = false;
    bool bIsString = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bValue = IsUseUserData();
            break;
        case MID_DOCINFO_USETHUMBNAILSAVE:
            bValue = IsUseThumbnailSave();
            break;
        case MID_DOCINFO_DELETEUSERDATA:
            bValue = m_bDeleteUserData;
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            bValue = isAutoloadEnabled();
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bIsInt = true;
            nValue = getAutoloadDelay();
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bIsString = true;
            aValue = getAutoloadURL();
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bIsString = true;
            aValue = getDefaultTarget();
            break;
        case MID_DOCINFO_DESCRIPTION:
            bIsString = true;
            aValue = getDescription();
            break;
        case MID_DOCINFO_KEYWORDS:
            bIsString = true;
            aValue = getKeywords();
            break;
        case MID_DOCINFO_SUBJECT:
            bIsString = true;
            aValue = getSubject();
            break;
        case MID_DOCINFO_TITLE:
            bIsString = true;
            aValue = getTitle();
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
     }

    if ( bIsString )
        rVal <<= aValue;
    else if ( bIsInt )
        rVal <<= nValue;
    else
        rVal <<= bValue;
    return true;
}

bool SfxDocumentInfoItem::PutValue( const Any& rVal, sal_uInt8 nMemberId )
{
    OUString aValue;
    sal_Int32 nValue=0;
    bool bValue = false;
    bool bRet = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bRet = (rVal >>= bValue);
            if ( bRet )
                SetUseUserData( bValue );
            break;
        case MID_DOCINFO_USETHUMBNAILSAVE:
            bRet = (rVal >>=bValue);
            if ( bRet )
                SetUseThumbnailSave( bValue );
            break;
        case MID_DOCINFO_DELETEUSERDATA:
            // QUESTION: deleting user data was done here; seems to be superfluous!
            bRet = (rVal >>= bValue);
            if ( bRet )
                SetDeleteUserData( bValue );
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            bRet = (rVal >>= bValue);
            if ( bRet )
                m_isAutoloadEnabled = bValue;
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bRet = (rVal >>= nValue);
            if ( bRet )
                m_AutoloadDelay = nValue;
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bRet = (rVal >>= aValue);
            if ( bRet )
                m_AutoloadURL = aValue;
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bRet = (rVal >>= aValue);
            if ( bRet )
                m_DefaultTarget = aValue;
            break;
        case MID_DOCINFO_DESCRIPTION:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setDescription(aValue);
            break;
        case MID_DOCINFO_KEYWORDS:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setKeywords(aValue);
            break;
        case MID_DOCINFO_SUBJECT:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setSubject(aValue);
            break;
        case MID_DOCINFO_TITLE:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setTitle(aValue);
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    return bRet;
}

SfxDocumentDescPage::SfxDocumentDescPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rItemSet)
    : SfxTabPage(pPage, pController, "sfx/ui/descriptioninfopage.ui", "DescriptionInfoPage", &rItemSet)
    , m_pInfoItem(nullptr)
    , m_xTitleEd(m_xBuilder->weld_entry("title"))
    , m_xThemaEd(m_xBuilder->weld_entry("subject"))
    , m_xKeywordsEd(m_xBuilder->weld_entry("keywords"))
    , m_xCommentEd(m_xBuilder->weld_text_view("comments"))
{
    m_xCommentEd->set_size_request(m_xKeywordsEd->get_preferred_size().Width(),
                                   m_xCommentEd->get_height_rows(16));
}

SfxDocumentDescPage::~SfxDocumentDescPage()
{
}

std::unique_ptr<SfxTabPage> SfxDocumentDescPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rItemSet)
{
     return std::make_unique<SfxDocumentDescPage>(pPage, pController, *rItemSet);
}

bool SfxDocumentDescPage::FillItemSet(SfxItemSet *rSet)
{
    // Test whether a change is present
    const bool bTitleMod = m_xTitleEd->get_value_changed_from_saved();
    const bool bThemeMod = m_xThemaEd->get_value_changed_from_saved();
    const bool bKeywordsMod = m_xKeywordsEd->get_value_changed_from_saved();
    const bool bCommentMod = m_xCommentEd->get_value_changed_from_saved();
    if ( !( bTitleMod || bThemeMod || bKeywordsMod || bCommentMod ) )
    {
        return false;
    }

    // Generating the output data
    const SfxPoolItem* pItem = nullptr;
    SfxDocumentInfoItem* pInfo = nullptr;
    const SfxItemSet* pExSet = GetDialogExampleSet();

    if ( pExSet && SfxItemState::SET != pExSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        pInfo = m_pInfoItem;
    else if ( pItem )
        pInfo = new SfxDocumentInfoItem( *static_cast<const SfxDocumentInfoItem *>(pItem) );

    if ( !pInfo )
    {
        SAL_WARN( "sfx.dialog", "SfxDocumentDescPage::FillItemSet(): no item found" );
        return false;
    }

    if ( bTitleMod )
    {
        pInfo->setTitle( m_xTitleEd->get_text() );
    }
    if ( bThemeMod )
    {
        pInfo->setSubject( m_xThemaEd->get_text() );
    }
    if ( bKeywordsMod )
    {
        pInfo->setKeywords( m_xKeywordsEd->get_text() );
    }
    if ( bCommentMod )
    {
        pInfo->setDescription( m_xCommentEd->get_text() );
    }
    rSet->Put( *pInfo );
    if ( pInfo != m_pInfoItem )
    {
        delete pInfo;
    }

    return true;
}

void SfxDocumentDescPage::Reset(const SfxItemSet *rSet)
{
    m_pInfoItem = const_cast<SfxDocumentInfoItem*>(&rSet->Get(SID_DOCINFO));

    m_xTitleEd->set_text(m_pInfoItem->getTitle());
    m_xThemaEd->set_text(m_pInfoItem->getSubject());
    m_xKeywordsEd->set_text(m_pInfoItem->getKeywords());
    m_xCommentEd->set_text(m_pInfoItem->getDescription());

    m_xTitleEd->save_value();
    m_xThemaEd->save_value();
    m_xKeywordsEd->save_value();
    m_xCommentEd->save_value();

    const SfxBoolItem* pROItem = SfxItemSet::GetItem<SfxBoolItem>(rSet, SID_DOC_READONLY, false);
    if (pROItem && pROItem->GetValue())
    {
        m_xTitleEd->set_editable(false);
        m_xThemaEd->set_editable(false);
        m_xKeywordsEd->set_editable(false);
        m_xCommentEd->set_editable(false);
    }
}

SfxDocumentPage::SfxDocumentPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rItemSet)
    : SfxTabPage(pPage, pController, "sfx/ui/documentinfopage.ui", "DocumentInfoPage", &rItemSet)
    , bEnableUseUserData( false )
    , bHandleDelete( false )
    , m_xBmp(m_xBuilder->weld_image("icon"))
    , m_xNameED(m_xBuilder->weld_label("nameed"))
    , m_xChangePassBtn(m_xBuilder->weld_button("changepass"))
    , m_xShowTypeFT(m_xBuilder->weld_label("showtype"))
    , m_xFileValEd(m_xBuilder->weld_label("showlocation"))
    , m_xShowSizeFT(m_xBuilder->weld_label("showsize"))
    , m_xCreateValFt(m_xBuilder->weld_label("showcreate"))
    , m_xChangeValFt(m_xBuilder->weld_label("showmodify"))
    , m_xSignedValFt(m_xBuilder->weld_label("showsigned"))
    , m_xSignatureBtn(m_xBuilder->weld_button("signature"))
    , m_xPrintValFt(m_xBuilder->weld_label("showprint"))
    , m_xTimeLogValFt(m_xBuilder->weld_label("showedittime"))
    , m_xDocNoValFt(m_xBuilder->weld_label("showrevision"))
    , m_xUseUserDataCB(m_xBuilder->weld_check_button("userdatacb"))
    , m_xDeleteBtn(m_xBuilder->weld_button("reset"))
    , m_xUseThumbnailSaveCB(m_xBuilder->weld_check_button("thumbnailsavecb"))
    , m_xTemplFt(m_xBuilder->weld_label("templateft"))
    , m_xTemplValFt(m_xBuilder->weld_label("showtemplate"))
{
    m_aUnknownSize = m_xShowSizeFT->get_label();
    m_xShowSizeFT->set_label(OUString());

    m_aMultiSignedStr = m_xSignedValFt->get_label();
    m_xSignedValFt->set_label(OUString());

    ImplUpdateSignatures();
    ImplCheckPasswordState();
    m_xChangePassBtn->connect_clicked( LINK( this, SfxDocumentPage, ChangePassHdl ) );
    m_xSignatureBtn->connect_clicked( LINK( this, SfxDocumentPage, SignatureHdl ) );
    m_xDeleteBtn->connect_clicked( LINK( this, SfxDocumentPage, DeleteHdl ) );

    // [i96288] Check if the document signature command is enabled
    // on the main list enable/disable the pushbutton accordingly
    SvtCommandOptions aCmdOptions;
    if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, "Signature" ) )
        m_xSignatureBtn->set_sensitive(false);
}

SfxDocumentPage::~SfxDocumentPage()
{
}

IMPL_LINK_NOARG(SfxDocumentPage, DeleteHdl, weld::Button&, void)
{
    OUString aName;
    if (bEnableUseUserData && m_xUseUserDataCB->get_active())
        aName = SvtUserOptions().GetFullName();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    DateTime now( DateTime::SYSTEM );
    util::DateTime uDT( now.GetUNODateTime() );
    m_xCreateValFt->set_label( ConvertDateTime_Impl( aName, uDT, rLocaleWrapper ) );
    m_xChangeValFt->set_label( "" );
    m_xPrintValFt->set_label( "" );
    const tools::Time aTime( 0 );
    m_xTimeLogValFt->set_label( rLocaleWrapper.getDuration( aTime ) );
    m_xDocNoValFt->set_label(OUString('1'));
    bHandleDelete = true;
}

IMPL_LINK_NOARG(SfxDocumentPage, SignatureHdl, weld::Button&, void)
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if( pDoc )
    {
        pDoc->SignDocumentContent(GetFrameWeld());

        ImplUpdateSignatures();
    }
}

IMPL_LINK_NOARG(SfxDocumentPage, ChangePassHdl, weld::Button&, void)
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    do
    {
        if (!pShell)
            break;
        SfxItemSet* pMedSet = pShell->GetMedium()->GetItemSet();
        if (!pMedSet)
            break;
        std::shared_ptr<const SfxFilter> pFilter = pShell->GetMedium()->GetFilter();
        if (!pFilter)
            break;

        sfx2::RequestPassword(pFilter, OUString(), pMedSet, GetFrameWeld()->GetXWindow());
        pShell->SetModified();
    }
    while (false);
}

void SfxDocumentPage::ImplUpdateSignatures()
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if ( !pDoc )
        return;

    SfxMedium* pMedium = pDoc->GetMedium();
    if ( !pMedium || pMedium->GetName().isEmpty() || !pMedium->GetStorage().is() )
        return;

    Reference< security::XDocumentDigitalSignatures > xD;
    try
    {
        xD = security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext());
        xD->setParentWindow(GetDialogController()->getDialog()->GetXWindow());
    }
    catch ( const css::uno::DeploymentException& )
    {
    }
    OUString s;
    Sequence< security::DocumentSignatureInformation > aInfos;

    if ( xD.is() )
        aInfos = xD->verifyDocumentContentSignatures( pMedium->GetZipStorageToSign_Impl(),
                                                      uno::Reference< io::XInputStream >() );
    if ( aInfos.getLength() > 1 )
        s = m_aMultiSignedStr;
    else if ( aInfos.getLength() == 1 )
    {
        const security::DocumentSignatureInformation& rInfo = aInfos[ 0 ];
        s = utl::GetDateTimeString( rInfo.SignatureDate, rInfo.SignatureTime ) + ", " +
            comphelper::xmlsec::GetContentPart(rInfo.Signer->getSubjectName(), rInfo.Signer->getCertificateKind());
    }
    m_xSignedValFt->set_label(s);
}

void SfxDocumentPage::ImplCheckPasswordState()
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    do
    {
        if (!pShell)
            break;
        SfxItemSet* pMedSet = pShell->GetMedium()->GetItemSet();
        if (!pMedSet)
            break;
        const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pMedSet, SID_ENCRYPTIONDATA, false);
        uno::Sequence< beans::NamedValue > aEncryptionData;
        if (pEncryptionDataItem)
            pEncryptionDataItem->GetValue() >>= aEncryptionData;
        else
             break;

        if (!aEncryptionData.hasElements())
             break;
        m_xChangePassBtn->set_sensitive(true);
        return;
    }
    while (false);
    m_xChangePassBtn->set_sensitive(false);
}

std::unique_ptr<SfxTabPage> SfxDocumentPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rItemSet)
{
     return std::make_unique<SfxDocumentPage>(pPage, pController, *rItemSet);
}

void SfxDocumentPage::EnableUseUserData()
{
    bEnableUseUserData = true;
    m_xUseUserDataCB->show();
    m_xDeleteBtn->show();
}

bool SfxDocumentPage::FillItemSet( SfxItemSet* rSet )
{
    bool bRet = false;

    if ( !bHandleDelete && bEnableUseUserData &&
         m_xUseUserDataCB->get_state_changed_from_saved() )
    {
        const SfxItemSet* pExpSet = GetDialogExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* pInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseData = ( TRISTATE_TRUE == m_xUseUserDataCB->get_state() );
            const_cast<SfxDocumentInfoItem*>(pInfoItem)->SetUseUserData( bUseData );
            rSet->Put( *pInfoItem );
            bRet = true;
        }
    }

    if ( bHandleDelete )
    {
        const SfxItemSet* pExpSet = GetDialogExampleSet();
        const SfxPoolItem* pItem;
        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* pInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseAuthor = bEnableUseUserData && m_xUseUserDataCB->get_active();
            SfxDocumentInfoItem newItem( *pInfoItem );
            newItem.resetUserData( bUseAuthor
                ? SvtUserOptions().GetFullName()
                : OUString() );
            const_cast<SfxDocumentInfoItem*>(pInfoItem)->SetUseUserData( TRISTATE_TRUE == m_xUseUserDataCB->get_state() );
            newItem.SetUseUserData( TRISTATE_TRUE == m_xUseUserDataCB->get_state() );

            newItem.SetDeleteUserData( true );
            rSet->Put( newItem );
            bRet = true;
        }
    }

    if ( m_xUseThumbnailSaveCB->get_state_changed_from_saved() )
    {
        const SfxItemSet* pExpSet = GetDialogExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* pInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseThumbnail = ( TRISTATE_TRUE == m_xUseThumbnailSaveCB->get_state() );
            const_cast<SfxDocumentInfoItem*>(pInfoItem)->SetUseThumbnailSave( bUseThumbnail );
            rSet->Put( *pInfoItem );
            bRet = true;
        }
    }

    return bRet;
}

void SfxDocumentPage::Reset( const SfxItemSet* rSet )
{
    // Determine the document information
    const SfxDocumentInfoItem& rInfoItem = rSet->Get(SID_DOCINFO);

    // template data
    if (rInfoItem.HasTemplate())
    {
        const OUString& rName = rInfoItem.getTemplateName();
        if (rName.getLength() > SAL_MAX_INT16) // tdf#122780 pick some ~arbitrary max size
            m_xTemplValFt->set_label(rName.copy(0, SAL_MAX_INT16));
        else
            m_xTemplValFt->set_label(rName);
    }
    else
    {
        m_xTemplFt->hide();
        m_xTemplValFt->hide();
    }

    // determine file name
    OUString aFile( rInfoItem.GetValue() );
    OUString aFactory( aFile );
    if ( aFile.getLength() > 2 && aFile[0] == '[' )
    {
        sal_Int32 nPos = aFile.indexOf( ']' );
        aFactory = aFile.copy( 1, nPos-1  );
        aFile = aFile.copy( nPos+1 );
    }

    // determine name
    INetURLObject aURL(aFile);
    OUString aName = aURL.GetLastName(INetURLObject::DecodeMechanism::WithCharset);
    if ( aName.isEmpty() || aURL.GetProtocol() == INetProtocol::PrivSoffice )
        aName = SfxResId( STR_NONAME );
    m_xNameED->set_label( aName );

    // determine context symbol
    aURL.SetSmartProtocol( INetProtocol::File );
    aURL.SetSmartURL( aFactory);
    const OUString& rMainURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    OUString aImage = SvFileInformationManager::GetImageId( aURL, true );
    m_xBmp->set_from_icon_name(aImage);

    // determine size and type
    OUString aSizeText( m_aUnknownSize );
    if ( aURL.GetProtocol() == INetProtocol::File ||
         aURL.isAnyKnownWebDAVScheme() )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );
    m_xShowSizeFT->set_label( aSizeText );

    OUString aDescription = SvFileInformationManager::GetDescription( INetURLObject(rMainURL) );
    if ( aDescription.isEmpty() )
        aDescription = SfxResId( STR_SFX_NEWOFFICEDOC );
    m_xShowTypeFT->set_label( aDescription );

    // determine location
    aURL.SetSmartURL( aFile);
    if ( aURL.GetProtocol() == INetProtocol::File )
    {
        INetURLObject aPath( aURL );
        aPath.setFinalSlash();
        aPath.removeSegment();
        // we know it's a folder -> don't need the final slash, but it's better for WB_PATHELLIPSIS
        aPath.removeFinalSlash();
        OUString aText( aPath.PathToFileName() ); //! (pb) MaxLen?
        m_xFileValEd->set_label( aText );
    }
    else if ( aURL.GetProtocol() != INetProtocol::PrivSoffice )
        m_xFileValEd->set_label( aURL.GetPartBeforeLastName() );

    // handle access data
    bool bUseUserData = rInfoItem.IsUseUserData();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_xCreateValFt->set_label( ConvertDateTime_Impl( rInfoItem.getAuthor(),
        rInfoItem.getCreationDate(), rLocaleWrapper ) );
    util::DateTime aTime( rInfoItem.getModificationDate() );
    if ( aTime.Month > 0 )
        m_xChangeValFt->set_label( ConvertDateTime_Impl(
            rInfoItem.getModifiedBy(), aTime, rLocaleWrapper ) );
    aTime = rInfoItem.getPrintDate();
    if ( aTime.Month > 0 )
        m_xPrintValFt->set_label( ConvertDateTime_Impl( rInfoItem.getPrintedBy(),
            aTime, rLocaleWrapper ) );
    const tools::Long nTime = rInfoItem.getEditingDuration();
    if ( bUseUserData )
    {
        const tools::Time aT( nTime/3600, (nTime%3600)/60, nTime%60 );
        m_xTimeLogValFt->set_label( rLocaleWrapper.getDuration( aT ) );
        m_xDocNoValFt->set_label( OUString::number(
            rInfoItem.getEditingCycles() ) );
    }

    bool bUseThumbnailSave = rInfoItem.IsUseThumbnailSave();

    // Check for cmis properties where otherwise unavailable
    if ( rInfoItem.isCmisDocument( ) )
    {
        const uno::Sequence< document::CmisProperty > aCmisProps = rInfoItem.GetCmisProperties();
        for ( const auto& rCmisProp : aCmisProps )
        {
            if ( rCmisProp.Id == "cmis:contentStreamLength" &&
                 aSizeText == m_aUnknownSize )
            {
                Sequence< sal_Int64 > seqValue;
                rCmisProp.Value >>= seqValue;
                SvNumberFormatter aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() );
                sal_uInt32 nIndex = aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
                if ( seqValue.hasElements() )
                {
                    OUString sValue;
                    aNumberFormatter.GetInputLineString( seqValue[0], nIndex, sValue );
                    m_xShowSizeFT->set_label( CreateSizeText( sValue.toInt64( ) ) );
                }
            }

            util::DateTime uDT;
            OUString emptyDate = ConvertDateTime_Impl( "", uDT, rLocaleWrapper );
            if ( rCmisProp.Id == "cmis:creationDate" &&
                 (m_xCreateValFt->get_label() == emptyDate ||
                  m_xCreateValFt->get_label().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                rCmisProp.Value >>= seqValue;
                if ( seqValue.hasElements() )
                {
                    m_xCreateValFt->set_label( ConvertDateTime_Impl( "", seqValue[0], rLocaleWrapper ) );
                }
            }
            if ( rCmisProp.Id == "cmis:lastModificationDate" &&
                 (m_xChangeValFt->get_label() == emptyDate ||
                  m_xChangeValFt->get_label().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                rCmisProp.Value >>= seqValue;
                if ( seqValue.hasElements() )
                {
                    m_xChangeValFt->set_label( ConvertDateTime_Impl( "", seqValue[0], rLocaleWrapper ) );
                }
            }
        }
    }

    m_xUseUserDataCB->set_active(bUseUserData);
    m_xUseUserDataCB->save_state();
    m_xUseUserDataCB->set_sensitive( bEnableUseUserData );
    bHandleDelete = false;
    m_xDeleteBtn->set_sensitive( bEnableUseUserData );
    m_xUseThumbnailSaveCB->set_active(bUseThumbnailSave);
    m_xUseThumbnailSaveCB->save_state();
}

SfxDocumentInfoDialog::SfxDocumentInfoDialog(weld::Window* pParent, const SfxItemSet& rItemSet)
    : SfxTabDialogController(pParent, "sfx/ui/documentpropertiesdialog.ui",
                             "DocumentPropertiesDialog", &rItemSet)
{
    const SfxDocumentInfoItem& rInfoItem = rItemSet.Get( SID_DOCINFO );

#ifdef DBG_UTIL
    const SfxStringItem* pURLItem = rItemSet.GetItem<SfxStringItem>(SID_BASEURL, false);
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
#endif

     // Determine the Titles
    const SfxPoolItem* pItem = nullptr;
    OUString aTitle(m_xDialog->get_title());
    if ( SfxItemState::SET !=
         rItemSet.GetItemState( SID_EXPLORER_PROPS_START, false, &pItem ) )
    {
        // File name
        const OUString& aFile( rInfoItem.GetValue() );

        INetURLObject aURL;
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( aFile);
        if ( INetProtocol::PrivSoffice != aURL.GetProtocol() )
        {
            OUString aLastName( aURL.GetLastName() );
            if ( !aLastName.isEmpty() )
                aTitle = aTitle.replaceFirst("%1", aLastName);
            else
                aTitle = aTitle.replaceFirst("%1", aFile);
        }
        else
            aTitle = aTitle.replaceFirst("%1", SfxResId( STR_NONAME ));
    }
    else
    {
        DBG_ASSERT( dynamic_cast<const SfxStringItem *>(pItem) != nullptr,
                    "SfxDocumentInfoDialog:<SfxStringItem> expected" );
        aTitle = aTitle.replaceFirst("%1", static_cast<const SfxStringItem*>(pItem)->GetValue());
    }
    m_xDialog->set_title(aTitle);

    // Property Pages
    AddTabPage("general", SfxDocumentPage::Create, nullptr);
    AddTabPage("description", SfxDocumentDescPage::Create, nullptr);
    AddTabPage("customprops", SfxCustomPropertiesPage::Create, nullptr);
    if (rInfoItem.isCmisDocument())
        AddTabPage("cmisprops", SfxCmisPropertiesPage::Create, nullptr);
    else
        RemoveTabPage("cmisprops");
    AddTabPage("security", SfxSecurityPage::Create, nullptr);
}

void SfxDocumentInfoDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "general")
        static_cast<SfxDocumentPage&>(rPage).EnableUseUserData();
}

void SfxDocumentInfoDialog::AddFontTabPage()
{
    AddTabPage("font", SfxResId(STR_FONT_TABPAGE), SfxDocumentFontsPage::Create);
}

// class CustomPropertiesYesNoButton -------------------------------------

CustomPropertiesYesNoButton::CustomPropertiesYesNoButton(std::unique_ptr<weld::Widget> xTopLevel,
                                                         std::unique_ptr<weld::RadioButton> xYesButton,
                                                         std::unique_ptr<weld::RadioButton> xNoButton)
    : m_xTopLevel(std::move(xTopLevel))
    , m_xYesButton(std::move(xYesButton))
    , m_xNoButton(std::move(xNoButton))
{
    CheckNo();
}

CustomPropertiesYesNoButton::~CustomPropertiesYesNoButton()
{
}

namespace {

class DurationDialog_Impl : public weld::GenericDialogController
{
    std::unique_ptr<weld::CheckButton> m_xNegativeCB;
    std::unique_ptr<weld::SpinButton> m_xYearNF;
    std::unique_ptr<weld::SpinButton> m_xMonthNF;
    std::unique_ptr<weld::SpinButton> m_xDayNF;
    std::unique_ptr<weld::SpinButton> m_xHourNF;
    std::unique_ptr<weld::SpinButton> m_xMinuteNF;
    std::unique_ptr<weld::SpinButton> m_xSecondNF;
    std::unique_ptr<weld::SpinButton> m_xMSecondNF;

public:
    DurationDialog_Impl(weld::Widget* pParent, const util::Duration& rDuration);
    util::Duration  GetDuration() const;
};

}

DurationDialog_Impl::DurationDialog_Impl(weld::Widget* pParent, const util::Duration& rDuration)
    : GenericDialogController(pParent, "sfx/ui/editdurationdialog.ui", "EditDurationDialog")
    , m_xNegativeCB(m_xBuilder->weld_check_button("negative"))
    , m_xYearNF(m_xBuilder->weld_spin_button("years"))
    , m_xMonthNF(m_xBuilder->weld_spin_button("months"))
    , m_xDayNF(m_xBuilder->weld_spin_button("days"))
    , m_xHourNF(m_xBuilder->weld_spin_button("hours"))
    , m_xMinuteNF(m_xBuilder->weld_spin_button("minutes"))
    , m_xSecondNF(m_xBuilder->weld_spin_button("seconds"))
    , m_xMSecondNF(m_xBuilder->weld_spin_button("milliseconds"))
{
    m_xNegativeCB->set_active(rDuration.Negative);
    m_xYearNF->set_value(rDuration.Years);
    m_xMonthNF->set_value(rDuration.Months);
    m_xDayNF->set_value(rDuration.Days);
    m_xHourNF->set_value(rDuration.Hours);
    m_xMinuteNF->set_value(rDuration.Minutes);
    m_xSecondNF->set_value(rDuration.Seconds);
    m_xMSecondNF->set_value(rDuration.NanoSeconds);
}

util::Duration  DurationDialog_Impl::GetDuration() const
{
    util::Duration  aRet;
    aRet.Negative = m_xNegativeCB->get_active();
    aRet.Years = m_xYearNF->get_value();
    aRet.Months = m_xMonthNF->get_value();
    aRet.Days = m_xDayNF->get_value();
    aRet.Hours  = m_xHourNF->get_value();
    aRet.Minutes = m_xMinuteNF->get_value();
    aRet.Seconds = m_xSecondNF->get_value();
    aRet.NanoSeconds = m_xMSecondNF->get_value();
    return aRet;
}

CustomPropertiesDurationField::CustomPropertiesDurationField(std::unique_ptr<weld::Entry> xEntry,
                                                             std::unique_ptr<weld::Button> xEditButton)
    : m_xEntry(std::move(xEntry))
    , m_xEditButton(std::move(xEditButton))
{
    m_xEditButton->connect_clicked(LINK(this, CustomPropertiesDurationField, ClickHdl));
    SetDuration( util::Duration(false, 0, 0, 0, 0, 0, 0, 0) );
}

void CustomPropertiesDurationField::set_visible(bool bVisible)
{
    m_xEntry->set_visible(bVisible);
    m_xEditButton->set_visible(bVisible);
}

void CustomPropertiesDurationField::SetDuration( const util::Duration& rDuration )
{
    m_aDuration = rDuration;
    OUString sText = (rDuration.Negative ? OUString('-') : OUString('+')) +
        SfxResId(SFX_ST_DURATION_FORMAT);
    sText = sText.replaceFirst( "%1", OUString::number( rDuration.Years ) );
    sText = sText.replaceFirst( "%2", OUString::number( rDuration.Months ) );
    sText = sText.replaceFirst( "%3", OUString::number( rDuration.Days   ) );
    sText = sText.replaceFirst( "%4", OUString::number( rDuration.Hours  ) );
    sText = sText.replaceFirst( "%5", OUString::number( rDuration.Minutes) );
    sText = sText.replaceFirst( "%6", OUString::number( rDuration.Seconds) );
    m_xEntry->set_text(sText);
}

IMPL_LINK(CustomPropertiesDurationField, ClickHdl, weld::Button&, rButton, void)
{
    DurationDialog_Impl aDurationDlg(&rButton, GetDuration());
    if (aDurationDlg.run() == RET_OK)
        SetDuration(aDurationDlg.GetDuration());
}

namespace
{
    void fillNameBox(weld::ComboBox& rNameBox)
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(SFX_CB_PROPERTY_STRINGARRAY); ++i)
            rNameBox.append_text(SfxResId(SFX_CB_PROPERTY_STRINGARRAY[i]));
        Size aSize(rNameBox.get_preferred_size());
        rNameBox.set_size_request(aSize.Width(), aSize.Height());
    }

    void fillTypeBox(weld::ComboBox& rTypeBox)
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(SFX_LB_PROPERTY_STRINGARRAY); ++i)
        {
            OUString sId(OUString::number(SFX_LB_PROPERTY_STRINGARRAY[i].second));
            rTypeBox.append(sId, SfxResId(SFX_LB_PROPERTY_STRINGARRAY[i].first));
        }
        rTypeBox.set_active(0);
        Size aSize(rTypeBox.get_preferred_size());
        rTypeBox.set_size_request(aSize.Width(), aSize.Height());
    }
}

// struct CustomPropertyLine ---------------------------------------------
CustomPropertyLine::CustomPropertyLine(CustomPropertiesWindow* pParent, weld::Widget* pContainer)
    : m_pParent(pParent)
    , m_xBuilder(Application::CreateBuilder(pContainer, "sfx/ui/linefragment.ui"))
    , m_xLine(m_xBuilder->weld_container("lineentry"))
    , m_xNameBox(m_xBuilder->weld_combo_box("namebox"))
    , m_xTypeBox(m_xBuilder->weld_combo_box("typebox"))
    , m_xValueEdit(m_xBuilder->weld_entry("valueedit"))
    , m_xDateTimeBox(m_xBuilder->weld_widget("datetimebox"))
    , m_xDateField(new CustomPropertiesDateField(new SvtCalendarBox(m_xBuilder->weld_menu_button("date"))))
    , m_xTimeField(new CustomPropertiesTimeField(m_xBuilder->weld_formatted_spin_button("time")))
    , m_xDurationBox(m_xBuilder->weld_widget("durationbox"))
    , m_xDurationField(new CustomPropertiesDurationField(m_xBuilder->weld_entry("duration"),
                                                         m_xBuilder->weld_button("durationbutton")))
    , m_xYesNoButton(new CustomPropertiesYesNoButton(m_xBuilder->weld_widget("yesno"),
                                                     m_xBuilder->weld_radio_button("yes"),
                                                     m_xBuilder->weld_radio_button("no")))
    , m_xRemoveButton(m_xBuilder->weld_button("remove"))
    , m_bTypeLostFocus( false )
{
    fillNameBox(*m_xNameBox);
    fillTypeBox(*m_xTypeBox);

    m_xTypeBox->connect_changed(LINK(this, CustomPropertyLine, TypeHdl));
    m_xRemoveButton->connect_clicked(LINK(this, CustomPropertyLine, RemoveHdl));
    m_xValueEdit->connect_focus_out(LINK(this, CustomPropertyLine, EditLoseFocusHdl));
    //add lose focus handlers of date/time fields
    m_xTypeBox->connect_focus_out(LINK(this, CustomPropertyLine, BoxLoseFocusHdl));
}

void CustomPropertyLine::Clear()
{
    m_xNameBox->set_active(-1);
    m_xValueEdit->set_text(OUString());

}

void CustomPropertyLine::Hide()
{
    m_xLine->hide();
}

CustomPropertiesWindow::CustomPropertiesWindow(weld::Container& rParent, weld::Label& rHeaderAccName,
                                               weld::Label& rHeaderAccType, weld::Label& rHeaderAccValue)
    : m_nHeight(0)
    , m_nLineHeight(0)
    , m_nScrollPos(0)
    , m_pCurrentLine(nullptr)
    , m_aNumberFormatter(::comphelper::getProcessComponentContext(),
                         Application::GetSettings().GetLanguageTag().getLanguageType())
    , m_rBody(rParent)
    , m_rHeaderAccName(rHeaderAccName)
    , m_rHeaderAccType(rHeaderAccType)
    , m_rHeaderAccValue(rHeaderAccValue)
{
    m_aEditLoseFocusIdle.SetPriority( TaskPriority::LOWEST );
    m_aEditLoseFocusIdle.SetInvokeHandler( LINK( this, CustomPropertiesWindow, EditTimeoutHdl ) );
    m_aBoxLoseFocusIdle.SetPriority( TaskPriority::LOWEST );
    m_aBoxLoseFocusIdle.SetInvokeHandler( LINK( this, CustomPropertiesWindow, BoxTimeoutHdl ) );
}

CustomPropertiesWindow::~CustomPropertiesWindow()
{
    m_aEditLoseFocusIdle.Stop();
    m_aBoxLoseFocusIdle.Stop();

    m_pCurrentLine = nullptr;
}

void CustomPropertyLine::DoTypeHdl(const weld::ComboBox& rBox)
{
    auto nType = rBox.get_active_id().toInt32();
    m_xValueEdit->set_visible( (CUSTOM_TYPE_TEXT == nType) || (CUSTOM_TYPE_NUMBER  == nType) );
    m_xDateTimeBox->set_visible( (CUSTOM_TYPE_DATE == nType) || (CUSTOM_TYPE_DATETIME  == nType) );
    m_xDateField->set_visible( (CUSTOM_TYPE_DATE == nType) || (CUSTOM_TYPE_DATETIME  == nType) );
    m_xTimeField->set_visible( CUSTOM_TYPE_DATETIME  == nType );
    m_xDurationBox->set_visible( CUSTOM_TYPE_DURATION == nType );
    m_xDurationField->set_visible( CUSTOM_TYPE_DURATION == nType );
    m_xYesNoButton->set_visible( CUSTOM_TYPE_BOOLEAN == nType );
}

IMPL_LINK(CustomPropertyLine, TypeHdl, weld::ComboBox&, rBox, void)
{
    DoTypeHdl(rBox);
}

void CustomPropertiesWindow::Remove(const CustomPropertyLine* pLine)
{
    StoreCustomProperties();

    auto pFound = std::find_if( m_aCustomPropertiesLines.begin(), m_aCustomPropertiesLines.end(),
                    [&] (const std::unique_ptr<CustomPropertyLine>& p) { return p.get() == pLine; });
    if ( pFound != m_aCustomPropertiesLines.end() )
    {
        sal_uInt32 nLineNumber = pFound - m_aCustomPropertiesLines.begin();
        sal_uInt32 nDataModelIndex = GetCurrentDataModelPosition() + nLineNumber;
        m_aCustomProperties.erase(m_aCustomProperties.begin() + nDataModelIndex);

        ReloadLinesContent();
    }

    m_aRemovedHdl.Call(nullptr);
}

IMPL_LINK_NOARG(CustomPropertyLine, RemoveHdl, weld::Button&, void)
{
    m_pParent->Remove(this);
}

void CustomPropertiesWindow::EditLoseFocus(CustomPropertyLine* pLine)
{
    m_pCurrentLine = pLine;
    m_aEditLoseFocusIdle.Start();
}

IMPL_LINK_NOARG(CustomPropertyLine, EditLoseFocusHdl, weld::Widget&, void)
{
    if (!m_bTypeLostFocus)
        m_pParent->EditLoseFocus(this);
    else
        m_bTypeLostFocus = false;
}

void CustomPropertiesWindow::BoxLoseFocus(CustomPropertyLine* pLine)
{
    m_pCurrentLine = pLine;
    m_aBoxLoseFocusIdle.Start();
}

IMPL_LINK_NOARG(CustomPropertyLine, BoxLoseFocusHdl, weld::Widget&, void)
{
    m_pParent->BoxLoseFocus(this);
}

IMPL_LINK_NOARG(CustomPropertiesWindow, EditTimeoutHdl, Timer *, void)
{
    ValidateLine( m_pCurrentLine, false );
}

IMPL_LINK_NOARG(CustomPropertiesWindow, BoxTimeoutHdl, Timer *, void)
{
    ValidateLine( m_pCurrentLine, true );
}

bool CustomPropertiesWindow::IsLineValid( CustomPropertyLine* pLine ) const
{
    bool bIsValid = true;
    pLine->m_bTypeLostFocus = false;
    auto nType = pLine->m_xTypeBox->get_active_id().toInt32();
    OUString sValue = pLine->m_xValueEdit->get_text();
    if ( sValue.isEmpty() )
        return true;

    sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
    if ( CUSTOM_TYPE_NUMBER == nType )
        // tdf#116214 Scientific format allows to use also standard numbers
        nIndex = const_cast< SvNumberFormatter& >(
            m_aNumberFormatter ).GetFormatIndex( NF_SCIENTIFIC_000E00 );
    else if ( CUSTOM_TYPE_DATE == nType )
        nIndex = const_cast< SvNumberFormatter& >(
            m_aNumberFormatter).GetFormatIndex( NF_DATE_SYS_DDMMYYYY );

    if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        sal_uInt32 nTemp = nIndex;
        double fDummy = 0.0;
        bIsValid = const_cast< SvNumberFormatter& >(
            m_aNumberFormatter ).IsNumberFormat( sValue, nIndex, fDummy );
        if ( bIsValid && nTemp != nIndex )
            // sValue is a number but the format doesn't match the index
            bIsValid = false;
    }

    return bIsValid;
}

void CustomPropertiesWindow::ValidateLine( CustomPropertyLine* pLine, bool bIsFromTypeBox )
{
    if (pLine && !IsLineValid(pLine))
    {
        if ( bIsFromTypeBox ) // LoseFocus of TypeBox
            pLine->m_bTypeLostFocus = true;
        std::unique_ptr<weld::MessageDialog> xMessageBox(Application::CreateMessageDialog(&m_rBody,
                                                         VclMessageType::Question, VclButtonsType::OkCancel, SfxResId(STR_SFX_QUERY_WRONG_TYPE)));
        if (xMessageBox->run() == RET_OK)
            pLine->m_xTypeBox->set_active_id(OUString::number(CUSTOM_TYPE_TEXT));
        else
            pLine->m_xValueEdit->grab_focus();
    }
}

void CustomPropertiesWindow::SetVisibleLineCount(sal_uInt32 nCount)
{
    while (GetExistingLineCount() < nCount)
    {
        CreateNewLine();
    }
}

void CustomPropertiesWindow::AddLine(const OUString& sName, Any const & rAny)
{
    m_aCustomProperties.push_back(std::unique_ptr<CustomProperty>(new CustomProperty(sName, rAny)));
    ReloadLinesContent();
}

void CustomPropertiesWindow::CreateNewLine()
{
    CustomPropertyLine* pNewLine = new CustomPropertyLine(this, &m_rBody);
    pNewLine->m_xNameBox->set_accessible_relation_labeled_by(&m_rHeaderAccName);
    pNewLine->m_xNameBox->set_accessible_name(m_rHeaderAccName.get_label());
    pNewLine->m_xTypeBox->set_accessible_relation_labeled_by(&m_rHeaderAccType);
    pNewLine->m_xTypeBox->set_accessible_name(m_rHeaderAccType.get_label());
    pNewLine->m_xValueEdit->set_accessible_relation_labeled_by(&m_rHeaderAccValue);
    pNewLine->m_xValueEdit->set_accessible_name(m_rHeaderAccValue.get_label());

    m_aCustomPropertiesLines.emplace_back( pNewLine );

    pNewLine->DoTypeHdl(*pNewLine->m_xTypeBox);
    pNewLine->m_xNameBox->grab_focus();
}

bool CustomPropertiesWindow::AreAllLinesValid() const
{
    bool bRet = true;
    for ( std::unique_ptr<CustomPropertyLine> const & pLine : m_aCustomPropertiesLines )
    {
        if ( !IsLineValid( pLine.get() ) )
        {
            bRet = false;
            break;
        }
    }

    return bRet;
}

void CustomPropertiesWindow::ClearAllLines()
{
    for (auto& pLine : m_aCustomPropertiesLines)
    {
        pLine->Clear();
    }
    m_pCurrentLine = nullptr;
    m_aCustomProperties.clear();
    m_nScrollPos = 0;
}

void CustomPropertiesWindow::DoScroll( sal_Int32 nNewPos )
{
    StoreCustomProperties();
    m_nScrollPos += nNewPos;
    ReloadLinesContent();
}

Sequence< beans::PropertyValue > CustomPropertiesWindow::GetCustomProperties()
{
    StoreCustomProperties();

    Sequence< beans::PropertyValue > aPropertiesSeq(GetTotalLineCount());

    for (sal_uInt32 i = 0; i < GetTotalLineCount(); ++i)
    {
        aPropertiesSeq[i].Name = m_aCustomProperties[i]->m_sName;
        aPropertiesSeq[i].Value = m_aCustomProperties[i]->m_aValue;
    }

    return aPropertiesSeq;
}

CustomPropertiesTimeField::CustomPropertiesTimeField(std::unique_ptr<weld::FormattedSpinButton> xTimeField)
    : m_xTimeField(std::move(xTimeField))
    , m_xFormatter(new weld::TimeFormatter(*m_xTimeField))
    , m_isUTC(false)
{
    m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    m_xFormatter->EnableEmptyField(false);
}

tools::Time CustomPropertiesTimeField::get_value() const
{
    return m_xFormatter->GetTime();
}

void CustomPropertiesTimeField::set_value(const tools::Time& rTime)
{
    m_xFormatter->SetTime(rTime);
}

CustomPropertiesTimeField::~CustomPropertiesTimeField()
{
}

CustomPropertiesDateField::CustomPropertiesDateField(SvtCalendarBox* pDateField)
    : m_xDateField(pDateField)
{
    DateTime aDateTime(DateTime::SYSTEM);
    m_xDateField->set_date(aDateTime);
}

void CustomPropertiesDateField::set_visible(bool bVisible)
{
    m_xDateField->set_visible(bVisible);
}

Date CustomPropertiesDateField::get_date() const
{
    return m_xDateField->get_date();
}

void CustomPropertiesDateField::set_date(const Date& rDate)
{
    m_xDateField->set_date(rDate);
}

CustomPropertiesDateField::~CustomPropertiesDateField()
{
}

void CustomPropertiesWindow::StoreCustomProperties()
{
    sal_uInt32 nDataModelPos = GetCurrentDataModelPosition();

    for (sal_uInt32 i = 0; nDataModelPos + i < GetTotalLineCount() && i < GetExistingLineCount(); i++)
    {
        CustomPropertyLine* pLine = m_aCustomPropertiesLines[i].get();

        OUString sPropertyName = pLine->m_xNameBox->get_active_text();
        if (!sPropertyName.isEmpty())
        {
            m_aCustomProperties[nDataModelPos + i]->m_sName = sPropertyName;
            auto nType = pLine->m_xTypeBox->get_active_id().toInt32();
            if (CUSTOM_TYPE_NUMBER == nType)
            {
                double nValue = 0;
                sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex(NF_NUMBER_SYSTEM);
                bool bIsNum = m_aNumberFormatter.
                    IsNumberFormat(pLine->m_xValueEdit->get_text(), nIndex, nValue);
                if (bIsNum)
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= nValue;
            }
            else if (CUSTOM_TYPE_BOOLEAN == nType)
            {
                bool bValue = pLine->m_xYesNoButton->IsYesChecked();
                m_aCustomProperties[nDataModelPos + i]->m_aValue <<= bValue;
            }
            else if (CUSTOM_TYPE_DATETIME == nType)
            {
                Date aTmpDate = pLine->m_xDateField->get_date();
                tools::Time aTmpTime = pLine->m_xTimeField->get_value();
                util::DateTime const aDateTime(aTmpTime.GetNanoSec(),
                    aTmpTime.GetSec(), aTmpTime.GetMin(), aTmpTime.GetHour(),
                    aTmpDate.GetDay(), aTmpDate.GetMonth(), aTmpDate.GetYear(),
                    pLine->m_xTimeField->m_isUTC);
                if (pLine->m_xDateField->m_TZ)
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= util::DateTimeWithTimezone(
                        aDateTime, *pLine->m_xDateField->m_TZ);
                }
                else
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= aDateTime;
                }
            }
            else if (CUSTOM_TYPE_DATE == nType)
            {
                Date aTmpDate = pLine->m_xDateField->get_date();
                util::Date const aDate(aTmpDate.GetDay(), aTmpDate.GetMonth(),
                    aTmpDate.GetYear());
                if (pLine->m_xDateField->m_TZ)
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= util::DateWithTimezone(
                        aDate, *pLine->m_xDateField->m_TZ);
                }
                else
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= aDate;
                }
            }
            else if (CUSTOM_TYPE_DURATION == nType)
            {
                m_aCustomProperties[nDataModelPos + i]->m_aValue <<= pLine->m_xDurationField->GetDuration();
            }
            else
            {
                OUString sValue(pLine->m_xValueEdit->get_text());
                m_aCustomProperties[nDataModelPos + i]->m_aValue <<= sValue;
            }
        }
    }
}

void CustomPropertiesWindow::SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties)
{
    m_aCustomProperties = std::move(rProperties);
    ReloadLinesContent();
}

void CustomPropertiesWindow::ReloadLinesContent()
{
    double nTmpValue = 0;
    bool bTmpValue = false;
    OUString sTmpValue;
    util::DateTime aTmpDateTime;
    util::Date aTmpDate;
    util::DateTimeWithTimezone aTmpDateTimeTZ;
    util::DateWithTimezone aTmpDateTZ;
    util::Duration aTmpDuration;
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    sal_IntPtr nType = CUSTOM_TYPE_UNKNOWN;
    OUString sValue;

    sal_uInt32 nDataModelPos = GetCurrentDataModelPosition();
    sal_uInt32 i = 0;

    for (; nDataModelPos + i < GetTotalLineCount() && i < GetExistingLineCount(); i++)
    {
        const OUString& rName = m_aCustomProperties[nDataModelPos + i]->m_sName;
        const css::uno::Any& rAny = m_aCustomProperties[nDataModelPos + i]->m_aValue;

        CustomPropertyLine* pLine = m_aCustomPropertiesLines[i].get();
        pLine->Clear();

        pLine->m_xNameBox->set_entry_text(rName);
        pLine->m_xLine->show();

        if (!rAny.hasValue())
        {
            pLine->m_xValueEdit->set_text(OUString());
        }
        else if (rAny >>= nTmpValue)
        {
            sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex(NF_NUMBER_SYSTEM);
            m_aNumberFormatter.GetInputLineString(nTmpValue, nIndex, sValue);
            pLine->m_xValueEdit->set_text(sValue);
            nType = CUSTOM_TYPE_NUMBER;
        }
        else if (rAny >>= bTmpValue)
        {
            sValue = (bTmpValue ? rLocaleWrapper.getTrueWord() : rLocaleWrapper.getFalseWord());
            nType = CUSTOM_TYPE_BOOLEAN;
        }
        else if (rAny >>= sTmpValue)
        {
            pLine->m_xValueEdit->set_text(sTmpValue);
            nType = CUSTOM_TYPE_TEXT;
        }
        else if (rAny >>= aTmpDate)
        {
            pLine->m_xDateField->set_date(Date(aTmpDate));
            nType = CUSTOM_TYPE_DATE;
        }
        else if (rAny >>= aTmpDateTime)
        {
            pLine->m_xDateField->set_date(Date(aTmpDateTime));
            pLine->m_xTimeField->set_value(tools::Time(aTmpDateTime));
            pLine->m_xTimeField->m_isUTC = aTmpDateTime.IsUTC;
            nType = CUSTOM_TYPE_DATETIME;
        }
        else if (rAny >>= aTmpDateTZ)
        {
            pLine->m_xDateField->set_date(Date(aTmpDateTZ.DateInTZ.Day,
                aTmpDateTZ.DateInTZ.Month, aTmpDateTZ.DateInTZ.Year));
            pLine->m_xDateField->m_TZ = aTmpDateTZ.Timezone;
            nType = CUSTOM_TYPE_DATE;
        }
        else if (rAny >>= aTmpDateTimeTZ)
        {
            util::DateTime const& rDT(aTmpDateTimeTZ.DateTimeInTZ);
            pLine->m_xDateField->set_date(Date(rDT));
            pLine->m_xTimeField->set_value(tools::Time(rDT));
            pLine->m_xTimeField->m_isUTC = rDT.IsUTC;
            pLine->m_xDateField->m_TZ = aTmpDateTimeTZ.Timezone;
            nType = CUSTOM_TYPE_DATETIME;
        }
        else if (rAny >>= aTmpDuration)
        {
            nType = CUSTOM_TYPE_DURATION;
            pLine->m_xDurationField->SetDuration(aTmpDuration);
        }

        if (nType != CUSTOM_TYPE_UNKNOWN)
        {
            if (CUSTOM_TYPE_BOOLEAN == nType)
            {
                if (bTmpValue)
                    pLine->m_xYesNoButton->CheckYes();
                else
                    pLine->m_xYesNoButton->CheckNo();
            }
            pLine->m_xTypeBox->set_active_id(OUString::number(nType));
        }

        pLine->DoTypeHdl(*pLine->m_xTypeBox);
    }

    while (nDataModelPos + i >= GetTotalLineCount() && i < GetExistingLineCount())
    {
        CustomPropertyLine* pLine = m_aCustomPropertiesLines[i].get();
        pLine->Hide();
        i++;
    }
}

CustomPropertiesControl::CustomPropertiesControl()
    : m_nThumbPos(0)
{
}

void CustomPropertiesControl::Init(weld::Builder& rBuilder)
{
    m_xBox = rBuilder.weld_widget("box");
    m_xBody = rBuilder.weld_container("properties");

    m_xName = rBuilder.weld_label("name");
    m_xType = rBuilder.weld_label("type");
    m_xValue = rBuilder.weld_label("value");
    m_xVertScroll = rBuilder.weld_scrolled_window("scroll", true);
    m_xPropertiesWin.reset(new CustomPropertiesWindow(*m_xBody, *m_xName, *m_xType, *m_xValue));

    m_xBox->set_stack_background();
    m_xVertScroll->show();

    std::unique_ptr<CustomPropertyLine> xNewLine(new CustomPropertyLine(m_xPropertiesWin.get(), m_xBody.get()));
    Size aLineSize(xNewLine->m_xLine->get_preferred_size());
    m_xPropertiesWin->SetLineHeight(aLineSize.Height() + 6);
    m_xBody->set_size_request(aLineSize.Width() + 6, -1);
    auto nHeight = aLineSize.Height() * 8;
    m_xVertScroll->set_size_request(-1, nHeight + 6);

    m_xPropertiesWin->SetHeight(nHeight);
    m_xVertScroll->connect_size_allocate(LINK(this, CustomPropertiesControl, ResizeHdl));

    m_xName->set_size_request(xNewLine->m_xNameBox->get_preferred_size().Width(), -1);
    m_xType->set_size_request(xNewLine->m_xTypeBox->get_preferred_size().Width(), -1);
    m_xValue->set_size_request(xNewLine->m_xValueEdit->get_preferred_size().Width(), -1);

    m_xBody->move(xNewLine->m_xLine.get(), nullptr);
    xNewLine.reset();

    m_xPropertiesWin->SetRemovedHdl( LINK( this, CustomPropertiesControl, RemovedHdl ) );

    m_xVertScroll->vadjustment_set_lower(0);
    m_xVertScroll->vadjustment_set_upper(0);
    m_xVertScroll->vadjustment_set_page_size(0xFFFF);

    Link<weld::ScrolledWindow&,void> aScrollLink = LINK( this, CustomPropertiesControl, ScrollHdl );
    m_xVertScroll->connect_vadjustment_changed(aScrollLink);

    ResizeHdl(Size(-1, nHeight));
}

IMPL_LINK(CustomPropertiesControl, ResizeHdl, const Size&, rSize, void)
{
    int nHeight = rSize.Height() - 6;
    if (nHeight == m_xPropertiesWin->GetHeight())
        return;
    m_xPropertiesWin->SetHeight(nHeight);
    sal_Int32 nScrollOffset = m_xPropertiesWin->GetLineHeight();
    sal_Int32 nVisibleEntries = nHeight / nScrollOffset;
    m_xPropertiesWin->SetVisibleLineCount( nVisibleEntries );
    m_xVertScroll->vadjustment_set_page_increment( nVisibleEntries - 1 );
    m_xVertScroll->vadjustment_set_page_size( nVisibleEntries );
    m_xPropertiesWin->ReloadLinesContent();
}

CustomPropertiesControl::~CustomPropertiesControl()
{
}

IMPL_LINK( CustomPropertiesControl, ScrollHdl, weld::ScrolledWindow&, rScrollBar, void )
{
    sal_Int32 nOffset = m_xPropertiesWin->GetLineHeight();
    int nThumbPos = rScrollBar.vadjustment_get_value();
    nOffset *= ( m_nThumbPos - nThumbPos );
    m_nThumbPos = nThumbPos;
    m_xPropertiesWin->DoScroll( nOffset );
}

IMPL_LINK_NOARG(CustomPropertiesControl, RemovedHdl, void*, void)
{
    auto nLineCount = m_xPropertiesWin->GetTotalLineCount();
    m_xVertScroll->vadjustment_set_upper(nLineCount + 1);
    if (m_xPropertiesWin->GetTotalLineCount() > m_xPropertiesWin->GetExistingLineCount())
    {
        m_xVertScroll->vadjustment_set_value(nLineCount - 1);
        ScrollHdl(*m_xVertScroll);
    }
}

void CustomPropertiesControl::AddLine( Any const & rAny )
{
    m_xPropertiesWin->AddLine( OUString(), rAny );
    auto nLineCount = m_xPropertiesWin->GetTotalLineCount();
    m_xVertScroll->vadjustment_set_upper(nLineCount + 1);
    if (m_xPropertiesWin->GetHeight() < nLineCount * m_xPropertiesWin->GetLineHeight())
    {
        m_xVertScroll->vadjustment_set_value(nLineCount + 1);
        ScrollHdl(*m_xVertScroll);
    }
}

void CustomPropertiesControl::SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties)
{
    m_xPropertiesWin->SetCustomProperties(std::move(rProperties));
    auto nLineCount = m_xPropertiesWin->GetTotalLineCount();
    m_xVertScroll->vadjustment_set_upper(nLineCount + 1);
}

// class SfxCustomPropertiesPage -----------------------------------------
SfxCustomPropertiesPage::SfxCustomPropertiesPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rItemSet )
    : SfxTabPage(pPage, pController, "sfx/ui/custominfopage.ui", "CustomInfoPage", &rItemSet)
    , m_xPropertiesCtrl(new CustomPropertiesControl)
    , m_xAdd(m_xBuilder->weld_button("add"))
{
    m_xPropertiesCtrl->Init(*m_xBuilder);
    m_xAdd->connect_clicked(LINK(this, SfxCustomPropertiesPage, AddHdl));
}

SfxCustomPropertiesPage::~SfxCustomPropertiesPage()
{
    m_xPropertiesCtrl.reset();
}

IMPL_LINK_NOARG(SfxCustomPropertiesPage, AddHdl, weld::Button&, void)
{
    // tdf#115853: reload current lines before adding a brand new one
    // indeed the info are deleted by ClearCustomProperties
    // each time SfxDocumentInfoItem destructor is called
    SfxDocumentInfoItem pInfo;
    const Sequence< beans::PropertyValue > aPropertySeq = m_xPropertiesCtrl->GetCustomProperties();
    for ( const auto& rProperty : aPropertySeq )
    {
        if ( !rProperty.Name.isEmpty() )
        {
            pInfo.AddCustomProperty( rProperty.Name, rProperty.Value );
        }
    }

    Any aAny;
    m_xPropertiesCtrl->AddLine(aAny);
}

bool SfxCustomPropertiesPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxPoolItem* pItem = nullptr;
    SfxDocumentInfoItem* pInfo = nullptr;
    bool bMustDelete = false;

    if (const SfxItemSet* pItemSet = GetDialogExampleSet())
    {
        if (SfxItemState::SET != pItemSet->GetItemState(SID_DOCINFO, true, &pItem))
            pInfo = const_cast<SfxDocumentInfoItem*>(&rSet->Get( SID_DOCINFO ));
        else
        {
            bMustDelete = true;
            pInfo = new SfxDocumentInfoItem( *static_cast<const SfxDocumentInfoItem*>(pItem) );
        }
    }

    if ( pInfo )
    {
        // If it's a CMIS document, we can't save custom properties
        if ( pInfo->isCmisDocument( ) )
        {
            if ( bMustDelete )
                delete pInfo;
            return false;
        }

        pInfo->ClearCustomProperties();
        const Sequence< beans::PropertyValue > aPropertySeq = m_xPropertiesCtrl->GetCustomProperties();
        for ( const auto& rProperty : aPropertySeq )
        {
            if ( !rProperty.Name.isEmpty() )
                pInfo->AddCustomProperty( rProperty.Name, rProperty.Value );
        }
    }

    if (pInfo)
    {
        rSet->Put(*pInfo);
        if ( bMustDelete )
            delete pInfo;
    }
    return true;
}

void SfxCustomPropertiesPage::Reset( const SfxItemSet* rItemSet )
{
    m_xPropertiesCtrl->ClearAllLines();
    const SfxDocumentInfoItem& rInfoItem = rItemSet->Get(SID_DOCINFO);
    std::vector< std::unique_ptr<CustomProperty> > aCustomProps = rInfoItem.GetCustomProperties();
    m_xPropertiesCtrl->SetCustomProperties(std::move(aCustomProps));
}

DeactivateRC SfxCustomPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    DeactivateRC nRet = DeactivateRC::LeavePage;
    if ( !m_xPropertiesCtrl->AreAllLinesValid() )
        nRet = DeactivateRC::KeepPage;
    return nRet;
}

std::unique_ptr<SfxTabPage> SfxCustomPropertiesPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rItemSet)
{
    return std::make_unique<SfxCustomPropertiesPage>(pPage, pController, *rItemSet);
}

CmisValue::CmisValue(weld::Widget* pParent, const OUString& aStr)
    : m_xBuilder(Application::CreateBuilder(pParent, "sfx/ui/cmisline.ui"))
    , m_xFrame(m_xBuilder->weld_frame("CmisFrame"))
    , m_xValueEdit(m_xBuilder->weld_entry("value"))
{
    m_xValueEdit->show();
    m_xValueEdit->set_text(aStr);
}

CmisDateTime::CmisDateTime(weld::Widget* pParent, const util::DateTime& aDateTime)
    : m_xBuilder(Application::CreateBuilder(pParent, "sfx/ui/cmisline.ui"))
    , m_xFrame(m_xBuilder->weld_frame("CmisFrame"))
    , m_xDateField(new SvtCalendarBox(m_xBuilder->weld_menu_button("date")))
    , m_xTimeField(m_xBuilder->weld_formatted_spin_button("time"))
    , m_xFormatter(new weld::TimeFormatter(*m_xTimeField))
{
    m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    m_xFormatter->EnableEmptyField(false);

    m_xDateField->show();
    m_xTimeField->show();
    m_xDateField->set_date(Date(aDateTime));
    m_xFormatter->SetTime(tools::Time(aDateTime));
}

CmisYesNo::CmisYesNo(weld::Widget* pParent, bool bValue)
    : m_xBuilder(Application::CreateBuilder(pParent, "sfx/ui/cmisline.ui"))
    , m_xFrame(m_xBuilder->weld_frame("CmisFrame"))
    , m_xYesButton(m_xBuilder->weld_radio_button("yes"))
    , m_xNoButton(m_xBuilder->weld_radio_button("no"))
{
    m_xYesButton->show();
    m_xNoButton->show();
    if (bValue)
        m_xYesButton->set_active(true);
    else
        m_xNoButton->set_active(true);
}

// struct CmisPropertyLine ---------------------------------------------
CmisPropertyLine::CmisPropertyLine(weld::Widget* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, "sfx/ui/cmisline.ui"))
    , m_sType(CMIS_TYPE_STRING)
    , m_bUpdatable(false)
    , m_bRequired(false)
    , m_bMultiValued(false)
    , m_bOpenChoice(false)
    , m_xFrame(m_xBuilder->weld_frame("CmisFrame"))
    , m_xName(m_xBuilder->weld_label("name"))
    , m_xType(m_xBuilder->weld_label("type"))
{
    m_xFrame->set_sensitive(true);
}

CmisPropertyLine::~CmisPropertyLine( )
{
}

// class CmisPropertiesWindow -----------------------------------------

CmisPropertiesWindow::CmisPropertiesWindow(std::unique_ptr<weld::Container> xParent)
    : m_xBox(std::move(xParent))
    , m_aNumberFormatter(::comphelper::getProcessComponentContext(),
                         Application::GetSettings().GetLanguageTag().getLanguageType())
{
}

CmisPropertiesWindow::~CmisPropertiesWindow()
{
}

void CmisPropertiesWindow::ClearAllLines()
{
    m_aCmisPropertiesLines.clear();
}

void CmisPropertiesWindow::AddLine( const OUString& sId, const OUString& sName,
                                    const OUString& sType, const bool bUpdatable,
                                    const bool bRequired, const bool bMultiValued,
                                    const bool bOpenChoice, Any& /*aChoices*/, Any const & rAny )
{
    std::unique_ptr<CmisPropertyLine> pNewLine(new CmisPropertyLine(m_xBox.get()));

    pNewLine->m_sId = sId;
    pNewLine->m_sType = sType;
    pNewLine->m_bUpdatable = bUpdatable;
    pNewLine->m_bRequired = bRequired;
    pNewLine->m_bMultiValued = bMultiValued;
    pNewLine->m_bOpenChoice = bOpenChoice;

    if ( sType == CMIS_TYPE_INTEGER )
    {
        Sequence< sal_Int64 > seqValue;
        rAny >>= seqValue;
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        for ( const auto& rValue : std::as_const(seqValue) )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( rValue, nIndex, sValue );
            std::unique_ptr<CmisValue> pValue(new CmisValue(m_xBox.get(), sValue));
            pValue->m_xValueEdit->set_editable(bUpdatable);
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }
    }
    else if ( sType == CMIS_TYPE_DECIMAL )
    {
        Sequence< double > seqValue;
        rAny >>= seqValue;
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        for ( const auto& rValue : std::as_const(seqValue) )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( rValue, nIndex, sValue );
            std::unique_ptr<CmisValue> pValue(new CmisValue(m_xBox.get(), sValue));
            pValue->m_xValueEdit->set_editable(bUpdatable);
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }

    }
    else if ( sType == CMIS_TYPE_BOOL )
    {
        Sequence<sal_Bool> seqValue;
        rAny >>= seqValue;
        for ( const auto& rValue : std::as_const(seqValue) )
        {
            std::unique_ptr<CmisYesNo> pYesNo(new CmisYesNo(m_xBox.get(), rValue));
            pYesNo->m_xYesButton->set_sensitive( bUpdatable );
            pYesNo->m_xNoButton->set_sensitive( bUpdatable );
            pNewLine->m_aYesNos.push_back( std::move(pYesNo) );
        }
    }
    else if ( sType == CMIS_TYPE_STRING )
    {
        Sequence< OUString > seqValue;
        rAny >>= seqValue;
        for ( const auto& rValue : std::as_const(seqValue) )
        {
            std::unique_ptr<CmisValue> pValue(new CmisValue(m_xBox.get(), rValue));
            pValue->m_xValueEdit->set_editable(bUpdatable);
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }
    }
    else if ( sType == CMIS_TYPE_DATETIME )
    {
        Sequence< util::DateTime > seqValue;
        rAny >>= seqValue;
        for ( const auto& rValue : std::as_const(seqValue) )
        {
            std::unique_ptr<CmisDateTime> pDateTime(new CmisDateTime(m_xBox.get(), rValue));
            pDateTime->m_xDateField->set_sensitive(bUpdatable);
            pDateTime->m_xTimeField->set_sensitive(bUpdatable);
            pNewLine->m_aDateTimes.push_back( std::move(pDateTime) );
        }
    }
    pNewLine->m_xName->set_label( sName );
    pNewLine->m_xName->show();
    pNewLine->m_xType->set_label( sType );
    pNewLine->m_xType->show();

    m_aCmisPropertiesLines.push_back( std::move(pNewLine) );
}

Sequence< document::CmisProperty > CmisPropertiesWindow::GetCmisProperties() const
{
    Sequence< document::CmisProperty > aPropertiesSeq( m_aCmisPropertiesLines.size() );
    sal_Int32 i = 0;
    for ( auto& rxLine : m_aCmisPropertiesLines )
    {
        CmisPropertyLine* pLine = rxLine.get();

        aPropertiesSeq[i].Id = pLine->m_sId;
        aPropertiesSeq[i].Type = pLine->m_sType;
        aPropertiesSeq[i].Updatable = pLine->m_bUpdatable;
        aPropertiesSeq[i].Required = pLine->m_bRequired;
        aPropertiesSeq[i].OpenChoice = pLine->m_bOpenChoice;
        aPropertiesSeq[i].MultiValued = pLine->m_bMultiValued;

        OUString sPropertyName = pLine->m_xName->get_label();
        if ( !sPropertyName.isEmpty() )
        {
            aPropertiesSeq[i].Name = sPropertyName;
            OUString sType = pLine->m_xType->get_label();
            if ( CMIS_TYPE_DECIMAL == sType )
            {
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                Sequence< double > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( const auto& rxValue : pLine->m_aValues )
                {
                    double dValue = 0.0;
                    OUString sValue( rxValue->m_xValueEdit->get_text() );
                    bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( sValue, nIndex, dValue );
                    if ( bIsNum )
                        seqValue[k] = dValue;
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;
            }
            else if ( CMIS_TYPE_INTEGER == sType )
            {
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                Sequence< sal_Int64 > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( const auto& rxValue : pLine->m_aValues )
                {
                    double dValue = 0;
                    OUString sValue( rxValue->m_xValueEdit->get_text() );
                    bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( sValue, nIndex, dValue );
                    if ( bIsNum )
                        seqValue[k] = static_cast<sal_Int64>(dValue);
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;
            }
            else if ( CMIS_TYPE_BOOL == sType )
            {
                Sequence<sal_Bool> seqValue( pLine->m_aYesNos.size( ) );
                sal_Int32 k = 0;
                for ( const auto& rxYesNo : pLine->m_aYesNos )
                {
                    bool bValue = rxYesNo->m_xYesButton->get_active();
                    seqValue[k] = bValue;
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;

            }
            else if ( CMIS_TYPE_DATETIME == sType )
            {
                Sequence< util::DateTime > seqValue( pLine->m_aDateTimes.size( ) );
                sal_Int32 k = 0;
                for ( const auto& rxDateTime : pLine->m_aDateTimes )
                {
                    Date aTmpDate = rxDateTime->m_xDateField->get_date();
                    tools::Time aTmpTime = rxDateTime->m_xFormatter->GetTime();
                    util::DateTime aDateTime( aTmpTime.GetNanoSec(), aTmpTime.GetSec(),
                                              aTmpTime.GetMin(), aTmpTime.GetHour(),
                                              aTmpDate.GetDay(), aTmpDate.GetMonth(),
                                              aTmpDate.GetYear(), true );
                    seqValue[k] = aDateTime;
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;
            }
            else
            {
                Sequence< OUString > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( const auto& rxValue : pLine->m_aValues )
                {
                    OUString sValue( rxValue->m_xValueEdit->get_text() );
                    seqValue[k] = sValue;
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;
            }
        }
        ++i;
    }

    return aPropertiesSeq;
}

CmisPropertiesControl::CmisPropertiesControl(weld::Builder& rBuilder)
    : m_aPropertiesWin(rBuilder.weld_container("CmisWindow"))
    , m_xScrolledWindow(rBuilder.weld_scrolled_window("CmisScroll"))
{
    // set height to something small and force it to take the size
    // dictated by the other pages
    m_xScrolledWindow->set_size_request(-1, 42);
}

void CmisPropertiesControl::ClearAllLines()
{
   m_aPropertiesWin.ClearAllLines();
}

void CmisPropertiesControl::AddLine( const OUString& sId, const OUString& sName,
                                     const OUString& sType, const bool bUpdatable,
                                     const bool bRequired, const bool bMultiValued,
                                     const bool bOpenChoice, Any& aChoices, Any const & rAny
                                     )
{
    m_aPropertiesWin.AddLine( sId, sName, sType, bUpdatable, bRequired, bMultiValued,
                               bOpenChoice, aChoices, rAny );
}

// class SfxCmisPropertiesPage -----------------------------------------
SfxCmisPropertiesPage::SfxCmisPropertiesPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rItemSet)
    : SfxTabPage(pPage, pController, "sfx/ui/cmisinfopage.ui", "CmisInfoPage", &rItemSet)
    , m_xPropertiesCtrl(new CmisPropertiesControl(*m_xBuilder))
{
}

SfxCmisPropertiesPage::~SfxCmisPropertiesPage()
{
    m_xPropertiesCtrl.reset();
}

bool SfxCmisPropertiesPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxPoolItem* pItem = nullptr;
    SfxDocumentInfoItem* pInfo = nullptr;
    bool bMustDelete = false;

    if (const SfxItemSet* pItemSet = GetDialogExampleSet())
    {
        if (SfxItemState::SET != pItemSet->GetItemState(SID_DOCINFO, true, &pItem))
            pInfo = const_cast<SfxDocumentInfoItem*>(&rSet->Get( SID_DOCINFO ));
        else
        {
            bMustDelete = true;
            pInfo = new SfxDocumentInfoItem( *static_cast<const SfxDocumentInfoItem*>(pItem) );
        }
    }

    sal_Int32 modifiedNum = 0;
    if ( pInfo )
    {
        Sequence< document::CmisProperty > aOldProps = pInfo->GetCmisProperties( );
        Sequence< document::CmisProperty > aNewProps = m_xPropertiesCtrl->GetCmisProperties();

        std::vector< document::CmisProperty > changedProps;
        for ( sal_Int32 i = 0; i< aNewProps.getLength( ); ++i )
        {
            if ( aOldProps[i].Updatable && !aNewProps[i].Id.isEmpty( ) )
            {
                if ( aOldProps[i].Type == CMIS_TYPE_DATETIME )
                {
                    Sequence< util::DateTime > oldValue;
                    aOldProps[i].Value >>= oldValue;
                    // We only edit hours and minutes
                    // don't compare NanoSeconds and Seconds
                    for ( auto& rDateTime : oldValue )
                    {
                        rDateTime.NanoSeconds = 0;
                        rDateTime.Seconds = 0;
                    }
                    Sequence< util::DateTime > newValue;
                    aNewProps[i].Value >>= newValue;
                    if ( oldValue != newValue )
                    {
                        modifiedNum++;
                        changedProps.push_back( aNewProps[i] );
                    }
                }
                else if ( aOldProps[i].Value != aNewProps[i].Value )
                {
                    modifiedNum++;
                    changedProps.push_back( aNewProps[i] );
                }
            }
        }
        Sequence< document::CmisProperty> aModifiedProps( comphelper::containerToSequence(changedProps) );
        pInfo->SetCmisProperties( aModifiedProps );
        rSet->Put( *pInfo );
        if ( bMustDelete )
            delete pInfo;
    }

    return modifiedNum;
}

void SfxCmisPropertiesPage::Reset( const SfxItemSet* rItemSet )
{
    m_xPropertiesCtrl->ClearAllLines();
    const SfxDocumentInfoItem& rInfoItem = rItemSet->Get(SID_DOCINFO);
    uno::Sequence< document::CmisProperty > aCmisProps = rInfoItem.GetCmisProperties();
    for ( auto& rCmisProp : aCmisProps )
    {
        m_xPropertiesCtrl->AddLine(rCmisProp.Id,
                                   rCmisProp.Name,
                                   rCmisProp.Type,
                                   rCmisProp.Updatable,
                                   rCmisProp.Required,
                                   rCmisProp.MultiValued,
                                   rCmisProp.OpenChoice,
                                   rCmisProp.Choices,
                                   rCmisProp.Value);
    }
}

DeactivateRC SfxCmisPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SfxCmisPropertiesPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rItemSet)
{
    return std::make_unique<SfxCmisPropertiesPage>(pPage, pController, *rItemSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
