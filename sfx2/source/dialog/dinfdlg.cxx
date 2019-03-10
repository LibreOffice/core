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
#include <svl/urihelper.hxx>
#include <tools/datetime.hxx>
#include <tools/urlobj.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/datetime.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/cmdoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <unotools/useroptions.hxx>
#include <svtools/controldims.hxx>
#include <svtools/imagemgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <memory>

#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTimeWithTimezone.hpp>
#include <com/sun/star/util/DateWithTimezone.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/CmisProperty.hpp>

#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <sfx2/securitypage.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/dinfdlg.hxx>
#include <helper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <documentfontsdialog.hxx>
#include <dinfdlg.hrc>
#include <sfx2/strings.hrc>
#include <strings.hxx>
#include <bitmaps.hlst>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

const sal_uInt16 FONT_PAGE_ID = 99;

struct CustomProperty
{
    OUString         m_sName;
    css::uno::Any    m_aValue;

    CustomProperty( const OUString& sName, const css::uno::Any& rValue ) :
        m_sName( sName ), m_aValue( rValue ) {}
};

SfxPoolItem* SfxDocumentInfoItem::CreateDefault() { return new SfxDocumentInfoItem; }

const sal_uInt16 HI_NAME = 1;
const sal_uInt16 HI_TYPE = 2;
const sal_uInt16 HI_VALUE = 3;
const sal_uInt16 HI_ACTION = 4;

namespace {

OUString CreateSizeText( sal_Int64 nSize )
{
    OUString aUnitStr(" ");
    aUnitStr += SfxResId(STR_BYTES);
    sal_Int64 nSize1 = nSize;
    sal_Int64 nSize2 = nSize1;
    sal_Int64 nMega = 1024 * 1024;
    sal_Int64 nGiga = nMega * 1024;
    double fSize = nSize;
    int nDec = 0;

    if ( nSize1 >= 10000 && nSize1 < nMega )
    {
        nSize1 /= 1024;
        aUnitStr = " ";
        aUnitStr += SfxResId(STR_KB);
        fSize /= 1024;
        nDec = 0;
    }
    else if ( nSize1 >= nMega && nSize1 < nGiga )
    {
        nSize1 /= nMega;
        aUnitStr = " ";
        aUnitStr += SfxResId(STR_MB);
        fSize /= nMega;
        nDec = 2;
    }
    else if ( nSize1 >= nGiga )
    {
        nSize1 /= nGiga;
        aUnitStr = " ";
        aUnitStr += SfxResId(STR_GB);
        fSize /= nGiga;
        nDec = 3;
    }
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    OUString aSizeStr( rLocaleWrapper.getNum( nSize1, 0 ) );
    aSizeStr += aUnitStr;
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
        aStr += aDelim;
        aStr += aAuthor;
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
            const beans::Property* pProps = lProps.getConstArray();
            sal_Int32 nCount = lProps.getLength();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                // "fix" property? => not a custom property => ignore it!
                if (!(pProps[i].Attributes & css::beans::PropertyAttribute::REMOVABLE))
                {
                    SAL_WARN( "sfx.dialog", "non-removable user-defined property?");
                    continue;
                }

                uno::Any aValue = xSet->getPropertyValue(pProps[i].Name);
                std::unique_ptr<CustomProperty> pProp(new CustomProperty( pProps[i].Name, aValue ));
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


SfxPoolItem* SfxDocumentInfoItem::Clone( SfxItemPool * ) const
{
    return new SfxDocumentInfoItem( *this );
}


bool SfxDocumentInfoItem::operator==( const SfxPoolItem& rItem) const
{
    if (!(typeid(rItem) == typeid(*this) && SfxStringItem::operator==(rItem)))
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
    setAuthor(i_rAuthor);
    DateTime now( DateTime::SYSTEM );
    m_CreationDate = now.GetUNODateTime();
    setModifiedBy(OUString());
    setPrintedBy(OUString());
    m_ModificationDate = util::DateTime();
    m_PrintDate = util::DateTime();
    setEditingDuration(0);
    setEditingCycles(1);
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
        const beans::Property* pProps = lProps.getConstArray();
        sal_Int32 nCount = lProps.getLength();
        for ( sal_Int32 j = 0; j < nCount; ++j )
        {
            if (pProps[j].Attributes & css::beans::PropertyAttribute::REMOVABLE)
            {
                xContainer->removeProperty( pProps[j].Name );
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while adding custom properties " << exceptionToString(ex) );
            }
        }
    }
    catch ( Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while removing custom properties " << exceptionToString(ex) );
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
            bValue = IsDeleteUserData();
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
                setAutoloadEnabled(bValue);
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bRet = (rVal >>= nValue);
            if ( bRet )
                setAutoloadDelay(nValue);
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setAutoloadURL(aValue);
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bRet = (rVal >>= aValue);
            if ( bRet )
                setDefaultTarget(aValue);
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

SfxDocumentDescPage::SfxDocumentDescPage(TabPageParent pParent, const SfxItemSet& rItemSet)
    : SfxTabPage(pParent, "sfx/ui/descriptioninfopage.ui", "DescriptionInfoPage", &rItemSet)
    , m_pInfoItem( nullptr)
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

VclPtr<SfxTabPage> SfxDocumentDescPage::Create(TabPageParent pParent, const SfxItemSet *rItemSet)
{
     return VclPtr<SfxDocumentDescPage>::Create(pParent, *rItemSet);
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

SfxDocumentPage::SfxDocumentPage(TabPageParent pParent, const SfxItemSet& rItemSet)
    : SfxTabPage(pParent, "sfx/ui/documentinfopage.ui", "DocumentInfoPage", &rItemSet)
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
    disposeOnce();
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

        sfx2::RequestPassword(pFilter, OUString(), pMedSet, VCLUnoHelper::GetInterface(GetParentDialog()));
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
        xD->setParentWindow(VCLUnoHelper::GetInterface(GetTabDialog()));
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
        s = utl::GetDateTimeString( rInfo.SignatureDate, rInfo.SignatureTime );
        s += ", ";
        s += comphelper::xmlsec::GetContentPart(rInfo.Signer->getSubjectName());
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

        if (!aEncryptionData.getLength())
             break;
        m_xChangePassBtn->set_sensitive(true);
        return;
    }
    while (false);
    m_xChangePassBtn->set_sensitive(false);
}

VclPtr<SfxTabPage> SfxDocumentPage::Create( TabPageParent pParent, const SfxItemSet* rItemSet )
{
     return VclPtr<SfxDocumentPage>::Create( pParent, *rItemSet );
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
         m_xUseUserDataCB->get_state_changed_from_saved() &&
         GetDialogExampleSet() )
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

    if ( m_xUseThumbnailSaveCB->get_state_changed_from_saved() &&
       GetDialogExampleSet() )
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
    if ( rInfoItem.HasTemplate() )
        m_xTemplValFt->set_label( rInfoItem.getTemplateName() );
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
    OUString aName = aURL.GetName( INetURLObject::DecodeMechanism::WithCharset );
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
    const long nTime = rInfoItem.getEditingDuration();
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
        uno::Sequence< document::CmisProperty > aCmisProps = rInfoItem.GetCmisProperties();
        for ( sal_Int32 i = 0; i < aCmisProps.getLength(); i++ )
        {
            if ( aCmisProps[i].Id == "cmis:contentStreamLength" &&
                 aSizeText == m_aUnknownSize )
            {
                Sequence< sal_Int64 > seqValue;
                aCmisProps[i].Value >>= seqValue;
                SvNumberFormatter aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() );
                sal_uInt32 nIndex = aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
                if ( seqValue.getLength( ) > 0 )
                {
                    OUString sValue;
                    aNumberFormatter.GetInputLineString( seqValue[0], nIndex, sValue );
                    m_xShowSizeFT->set_label( CreateSizeText( sValue.toInt64( ) ) );
                }
            }

            util::DateTime uDT;
            OUString emptyDate = ConvertDateTime_Impl( "", uDT, rLocaleWrapper );
            if ( aCmisProps[i].Id == "cmis:creationDate" &&
                 (m_xCreateValFt->get_label() == emptyDate ||
                  m_xCreateValFt->get_label().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                aCmisProps[i].Value >>= seqValue;
                if ( seqValue.getLength( ) > 0 )
                {
                    m_xCreateValFt->set_label( ConvertDateTime_Impl( "", seqValue[0], rLocaleWrapper ) );
                }
            }
            if ( aCmisProps[i].Id == "cmis:lastModificationDate" &&
                 (m_xChangeValFt->get_label() == emptyDate ||
                  m_xChangeValFt->get_label().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                aCmisProps[i].Value >>= seqValue;
                if ( seqValue.getLength( ) > 0 )
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

SfxDocumentInfoDialog::SfxDocumentInfoDialog( vcl::Window* pParent,
                                              const SfxItemSet& rItemSet )
    : SfxTabDialog(pParent, "DocumentPropertiesDialog",
        "sfx/ui/documentpropertiesdialog.ui", &rItemSet)
    , m_nDocInfoId(0)
{
    const SfxDocumentInfoItem& rInfoItem = rItemSet.Get( SID_DOCINFO );

#ifdef DBG_UTIL
    const SfxStringItem* pURLItem = rItemSet.GetItem<SfxStringItem>(SID_BASEURL, false);
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
#endif

     // Determine the Titles
    const SfxPoolItem* pItem = nullptr;
    OUString aTitle( GetText() );
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
    SetText( aTitle );

    // Property Pages
    m_nDocInfoId = AddTabPage("general", SfxDocumentPage::Create);
    AddTabPage("description", SfxDocumentDescPage::Create);
    AddTabPage("customprops", SfxCustomPropertiesPage::Create);
    AddTabPage("cmisprops", SfxCmisPropertiesPage::Create);
    AddTabPage("security", SfxSecurityPage::Create);
}


void SfxDocumentInfoDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if ( m_nDocInfoId == nId )
        static_cast<SfxDocumentPage&>(rPage).EnableUseUserData();
}

void SfxDocumentInfoDialog::AddFontTabPage()
{
    AddTabPage(FONT_PAGE_ID, SfxResId(STR_FONT_TABPAGE), SfxDocumentFontsPage::Create);
    SetPageName(FONT_PAGE_ID , "font");
}

// class CustomPropertiesYesNoButton -------------------------------------

CustomPropertiesYesNoButton::CustomPropertiesYesNoButton(vcl::Window* pParent)
    : Control(pParent, WB_DIALOGCONTROL | WB_BORDER)
    , m_aYesButton(VclPtr<RadioButton>::Create(this, WB_TABSTOP))
    , m_aNoButton(VclPtr<RadioButton>::Create(this, WB_TABSTOP))
{
    m_aYesButton->SetText(MnemonicGenerator::EraseAllMnemonicChars(Button::GetStandardText(StandardButtonType::Yes)));
    m_aYesButton->Show();
    m_aNoButton->SetText(MnemonicGenerator::EraseAllMnemonicChars(Button::GetStandardText(StandardButtonType::No)));
    m_aNoButton->Show();
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
    SetBorderStyle( WindowBorderStyle::MONO  );
    CheckNo();
    Wallpaper aWall( COL_TRANSPARENT );
    m_aYesButton->SetBackground( aWall );
    m_aNoButton->SetBackground( aWall );
}

CustomPropertiesYesNoButton::~CustomPropertiesYesNoButton()
{
    disposeOnce();
}

void CustomPropertiesYesNoButton::dispose()
{
    m_aYesButton.disposeAndClear();
    m_aNoButton.disposeAndClear();
    Control::dispose();
}

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
    DurationDialog_Impl(weld::Window* pParent, const util::Duration& rDuration);
    util::Duration  GetDuration() const;
};

DurationDialog_Impl::DurationDialog_Impl(weld::Window* pParent, const util::Duration& rDuration)
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

CustomPropertiesDurationField::CustomPropertiesDurationField(vcl::Window* pParent, WinBits nStyle,
                                                             CustomPropertyLine* pLine)
    : Edit(pParent, nStyle)
    , m_pLine(pLine)

{
    SetDuration( util::Duration(false, 0, 0, 0, 0, 0, 0, 0) );
}

void CustomPropertiesDurationField::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Size aSize( GetSizePixel() );
        tools::Rectangle aItemRect( rHEvt.GetMousePosPixel(), aSize );
        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aItemRect, GetText() );
        else
            Help::ShowQuickHelp( this, aItemRect, GetText(),
                QuickHelpFlags::Left|QuickHelpFlags::VCenter );
    }
}

void CustomPropertiesDurationField::SetDuration( const util::Duration& rDuration )
{
    m_aDuration = rDuration;
    OUString sText(rDuration.Negative ? OUString('-') : OUString('+'));
    sText += m_pLine->m_sDurationFormat;
    sText = sText.replaceFirst( "%1", OUString::number( rDuration.Years ) );
    sText = sText.replaceFirst( "%2", OUString::number( rDuration.Months ) );
    sText = sText.replaceFirst( "%3", OUString::number( rDuration.Days   ) );
    sText = sText.replaceFirst( "%4", OUString::number( rDuration.Hours  ) );
    sText = sText.replaceFirst( "%5", OUString::number( rDuration.Minutes) );
    sText = sText.replaceFirst( "%6", OUString::number( rDuration.Seconds) );
    SetText( sText );
}

CustomPropertiesEditButton::CustomPropertiesEditButton(vcl::Window* pParent, WinBits nStyle,
                                                       CustomPropertyLine* pLine)
    : PushButton(pParent, nStyle)
    , m_pLine(pLine)
{
    SetClickHdl( LINK( this, CustomPropertiesEditButton, ClickHdl ));
}

IMPL_LINK_NOARG(CustomPropertiesEditButton, ClickHdl, Button*, void)
{
    DurationDialog_Impl aDurationDlg(GetFrameWeld(), m_pLine->m_aDurationField->GetDuration());
    if (aDurationDlg.run() == RET_OK)
        m_pLine->m_aDurationField->SetDuration(aDurationDlg.GetDuration());
}

void CustomPropertiesYesNoButton::Resize()
{
    Size aParentSize(GetSizePixel());
    const long nWidth = aParentSize.Width();
    const long n1Width = LogicToPixel(Size(1, 1), MapMode(MapUnit::MapAppFont)).Width();
    const long n3Width = LogicToPixel(Size(3, 3), MapMode(MapUnit::MapAppFont)).Width();
    const long nNewWidth = (nWidth / 2) - n3Width - 2;
    Size aSize(nNewWidth, m_aYesButton->get_preferred_size().Height());
    Point aPos(n1Width, (aParentSize.Height() - aSize.Height()) / 2);
    m_aYesButton->SetPosSizePixel(aPos, aSize);
    aPos.AdjustX(aSize.Width() + n3Width );
    m_aNoButton->SetPosSizePixel(aPos, aSize);
}

namespace
{
    VclPtr<ComboBox> makeComboBox(vcl::Window *pParent)
    {
        VclPtr<ComboBox> aNameBox(VclPtr<ComboBox>::Create(pParent, WB_TABSTOP|WB_DROPDOWN|WB_AUTOHSCROLL));
        for (size_t i = 0; i < SAL_N_ELEMENTS(SFX_CB_PROPERTY_STRINGARRAY); ++i)
            aNameBox->InsertEntry(SfxResId(SFX_CB_PROPERTY_STRINGARRAY[i]));
        aNameBox->EnableAutoSize(true);
        return aNameBox;
    }
}

CustomPropertiesTypeBox::CustomPropertiesTypeBox(vcl::Window* pParent, CustomPropertyLine* pLine)
    : ListBox(pParent, WB_BORDER|WB_DROPDOWN)
    , m_pLine(pLine)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(SFX_LB_PROPERTY_STRINGARRAY); ++i)
    {
        InsertEntry(SfxResId(SFX_LB_PROPERTY_STRINGARRAY[i].first));
        SetEntryData(i, reinterpret_cast<void*>(SFX_LB_PROPERTY_STRINGARRAY[i].second));
    }
    SelectEntryPos(0);
    EnableAutoSize(true);
}

// struct CustomPropertyLine ---------------------------------------------
CustomPropertyLine::CustomPropertyLine( vcl::Window* pParent ) :
    m_aLine         ( VclPtr<VclGrid>::Create(pParent) ),
    m_aNameBox      ( makeComboBox(m_aLine) ),
    m_aTypeBox      ( VclPtr<CustomPropertiesTypeBox>::Create(m_aLine, this) ),
    m_aValueEdit    ( VclPtr<CustomPropertiesEdit>::Create(m_aLine, WB_BORDER|WB_TABSTOP|WB_LEFT, this ) ),
    m_aDateField    ( VclPtr<CustomPropertiesDateField>::Create(m_aLine, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_aTimeField    ( VclPtr<CustomPropertiesTimeField>::Create(m_aLine, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_sDurationFormat( SfxResId( SFX_ST_DURATION_FORMAT ) ),
    m_aDurationField( VclPtr<CustomPropertiesDurationField>::Create(m_aLine, WB_BORDER|WB_TABSTOP|WB_READONLY, this ) ),
    m_aEditButton   ( VclPtr<CustomPropertiesEditButton>::Create(m_aLine, WB_TABSTOP, this) ),
    m_aYesNoButton  ( VclPtr<CustomPropertiesYesNoButton>::Create(m_aLine) ),
    m_aRemoveButton ( VclPtr<CustomPropertiesRemoveButton>::Create(m_aLine, 0, this) ),
    m_bTypeLostFocus( false )
{
    m_aLine->set_column_spacing(4);

    m_aNameBox->set_grid_left_attach(0);
    m_aNameBox->set_grid_top_attach(0);
    m_aNameBox->set_margin_left(4);
    m_aNameBox->Show();

    m_aTypeBox->set_grid_left_attach(1);
    m_aTypeBox->set_grid_top_attach(0);
    m_aTypeBox->Show();

    m_aValueEdit->set_grid_left_attach(2);
    m_aValueEdit->set_grid_top_attach(0);
    m_aValueEdit->set_hexpand(true);
    m_aValueEdit->Show();

    m_aDateField->set_grid_left_attach(3);
    m_aDateField->set_grid_top_attach(0);
    m_aDateField->set_hexpand(true);
    m_aDateField->Show();

    m_aTimeField->set_grid_left_attach(4);
    m_aTimeField->set_grid_top_attach(0);
    m_aTimeField->set_hexpand(true);
    m_aTimeField->Show();

    m_aDurationField->set_grid_left_attach(5);
    m_aDurationField->set_grid_top_attach(0);
    m_aDurationField->set_hexpand(true);
    m_aDurationField->Show();

    m_aEditButton->set_grid_left_attach(6);
    m_aEditButton->set_grid_top_attach(0);
    m_aEditButton->Show();

    m_aYesNoButton->set_grid_left_attach(7);
    m_aYesNoButton->set_grid_top_attach(0);
    m_aYesNoButton->set_hexpand(true);
    m_aYesNoButton->Show();

    m_aRemoveButton->set_grid_left_attach(8);
    m_aRemoveButton->set_grid_top_attach(0);
    m_aRemoveButton->set_margin_right(4);
    m_aRemoveButton->Show();

    m_aTimeField->SetExtFormat( ExtTimeFieldFormat::Long24H );
    m_aDateField->SetExtDateFormat( ExtDateFieldFormat::SystemShortYYYY );

    m_aRemoveButton->SetModeImage(Image(StockImage::Yes, SFX_BMP_PROPERTY_REMOVE));
    m_aRemoveButton->SetQuickHelpText(SfxResId(STR_SFX_REMOVE_PROPERTY));

    m_aEditButton->SetText(SFX_ST_EDIT);
}

void CustomPropertyLine::Clear()
{
    m_aNameBox->SetNoSelection();
    m_aValueEdit->SetText(OUString());

}

void CustomPropertyLine::Hide()
{
    m_aLine->Hide();
}

CustomPropertiesWindow::CustomPropertiesWindow(vcl::Window* pParent,
    FixedText *pHeaderAccName,
    FixedText *pHeaderAccType,
    FixedText *pHeaderAccValue) :
    Window(pParent, WB_HIDE | WB_TABSTOP | WB_DIALOGCONTROL),
    m_pHeaderAccName(pHeaderAccName),
    m_pHeaderAccType(pHeaderAccType),
    m_pHeaderAccValue(pHeaderAccValue),
    m_nScrollPos (0),
    m_pCurrentLine (nullptr),
    m_aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() )
{
    m_nRemoveButtonWidth = ScopedVclPtrInstance<CustomPropertiesRemoveButton>(pParent, 0, nullptr)->get_preferred_size().Width();
    Size aSize = ScopedVclPtrInstance<CustomPropertiesTypeBox>(pParent, nullptr)->CalcMinimumSize();
    m_nTypeBoxWidth = aSize.Width();
    m_nWidgetHeight = aSize.Height();

    Point aPos(LogicToPixel(Point(0, 2), MapMode(MapUnit::MapAppFont)));

    m_aEditLoseFocusIdle.SetPriority( TaskPriority::LOWEST );
    m_aEditLoseFocusIdle.SetInvokeHandler( LINK( this, CustomPropertiesWindow, EditTimeoutHdl ) );
    m_aBoxLoseFocusIdle.SetPriority( TaskPriority::LOWEST );
    m_aBoxLoseFocusIdle.SetInvokeHandler( LINK( this, CustomPropertiesWindow, BoxTimeoutHdl ) );

    m_nLineHeight = (aPos.Y() * 2) + m_nWidgetHeight;
}

void CustomPropertiesWindow::Init(HeaderBar* pHeaderBar, ScrollBar* pScrollBar)
{
    m_pHeaderBar = pHeaderBar;
    m_pScrollBar = pScrollBar;
}

CustomPropertiesWindow::~CustomPropertiesWindow()
{
    disposeOnce();
}

void CustomPropertiesWindow::dispose()
{
    m_aEditLoseFocusIdle.Stop();
    m_aBoxLoseFocusIdle.Stop();

    m_aCustomPropertiesLines.clear();
    m_pCurrentLine = nullptr;

    m_pHeaderBar.clear();
    m_pScrollBar.clear();
    m_pHeaderAccName.clear();
    m_pHeaderAccType.clear();
    m_pHeaderAccValue.clear();
    vcl::Window::dispose();
}

IMPL_LINK(CustomPropertiesWindow, TypeHdl, ListBox&, rListBox, void)
{
    CustomPropertiesTypeBox* pBox = static_cast<CustomPropertiesTypeBox*>(&rListBox);
    long nType = reinterpret_cast<long>( pBox->GetSelectedEntryData() );
    CustomPropertyLine* pLine = pBox->GetLine();
    pLine->m_aValueEdit->Show( (CUSTOM_TYPE_TEXT == nType) || (CUSTOM_TYPE_NUMBER  == nType) );
    pLine->m_aDateField->Show( (CUSTOM_TYPE_DATE == nType) || (CUSTOM_TYPE_DATETIME  == nType) );
    pLine->m_aTimeField->Show( CUSTOM_TYPE_DATETIME  == nType );
    pLine->m_aDurationField->Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aEditButton->Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aYesNoButton->Show( CUSTOM_TYPE_BOOLEAN == nType );

    pLine->m_aLine->SetSizePixel(Size(GetSizePixel().Width(), m_nWidgetHeight));
}

IMPL_LINK( CustomPropertiesWindow, RemoveHdl, Button*, pBtn, void )
{
    StoreCustomProperties();

    CustomPropertiesRemoveButton* pButton = static_cast<CustomPropertiesRemoveButton*>(pBtn);
    CustomPropertyLine* pLine = pButton->GetLine();
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

IMPL_LINK( CustomPropertiesWindow, EditLoseFocusHdl, Control&, rControl, void )
{
    CustomPropertiesEdit* pEdit = static_cast<CustomPropertiesEdit*>(&rControl);
    CustomPropertyLine* pLine = pEdit->GetLine();
    if ( !pLine->m_bTypeLostFocus )
    {
        m_pCurrentLine = pLine;
        m_aEditLoseFocusIdle.Start();
    }
    else
        pLine->m_bTypeLostFocus = false;
}

IMPL_LINK( CustomPropertiesWindow, BoxLoseFocusHdl, Control&, rControl, void )
{
    m_pCurrentLine = static_cast<CustomPropertiesTypeBox*>(&rControl)->GetLine();
    m_aBoxLoseFocusIdle.Start();
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
    long nType = reinterpret_cast<long>(
                     pLine->m_aTypeBox->GetSelectedEntryData() );
    OUString sValue = pLine->m_aValueEdit->GetText();
    if ( sValue.isEmpty() )
        return true;

    sal_uInt32 nIndex = 0xFFFFFFFF;
    if ( CUSTOM_TYPE_NUMBER == nType )
        nIndex = const_cast< SvNumberFormatter& >(
            m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
    else if ( CUSTOM_TYPE_DATE == nType )
        nIndex = const_cast< SvNumberFormatter& >(
            m_aNumberFormatter).GetFormatIndex( NF_DATE_SYS_DDMMYYYY );

    if ( nIndex != 0xFFFFFFFF )
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
        vcl::Window* pParent = GetParent()->GetParent();
        std::unique_ptr<weld::MessageDialog> xMessageBox(Application::CreateMessageDialog(pParent ? pParent->GetFrameWeld() : nullptr,
                                                         VclMessageType::Question, VclButtonsType::OkCancel, SfxResId(STR_SFX_QUERY_WRONG_TYPE)));
        if (xMessageBox->run() == RET_OK)
            pLine->m_aTypeBox->SelectEntryPos(pLine->m_aTypeBox->GetEntryPos(reinterpret_cast<void*>(CUSTOM_TYPE_TEXT)));
        else
            pLine->m_aValueEdit->GrabFocus();
    }
}

void CustomPropertiesWindow::SetWidgetWidths(const CustomPropertyLine* pLine) const
{
    const long nOffset = 4;
    long nItemWidth = m_pHeaderBar->GetItemSize(HI_NAME);
    nItemWidth -= nOffset;

    pLine->m_aNameBox->set_width_request(nItemWidth);
    pLine->m_aTypeBox->set_width_request(m_nTypeBoxWidth);
    pLine->m_aValueEdit->set_width_request(nItemWidth);

    long nTimeWidth = nItemWidth;
    nTimeWidth /= 2;
    nTimeWidth -= 2;

    pLine->m_aDateField->set_width_request(nTimeWidth);
    pLine->m_aTimeField->set_width_request(nTimeWidth);

    pLine->m_aDurationField->set_width_request(nItemWidth - (pLine->m_aEditButton->get_preferred_size().Width() + nOffset));
    pLine->m_aYesNoButton->set_width_request(nItemWidth);
    pLine->m_aRemoveButton->set_width_request(m_nRemoveButtonWidth);

    pLine->m_aLine->SetSizePixel(Size(GetSizePixel().Width(), m_nWidgetHeight));
}

void CustomPropertiesWindow::Resize()
{
    const long nOffset = 4;
    const long nScrollBarWidth = m_pScrollBar->GetSizePixel().Width();
    long nButtonWidth = m_nRemoveButtonWidth + nScrollBarWidth + nOffset;
    long nTypeWidth = m_nTypeBoxWidth + (2 * nOffset);
    long nFullWidth = m_pHeaderBar->GetSizePixel().Width();
    long nItemWidth = ( nFullWidth - nTypeWidth - nButtonWidth ) / 2;
    m_pHeaderBar->SetItemSize( HI_NAME, nItemWidth );
    m_pHeaderBar->SetItemSize( HI_TYPE, nTypeWidth );
    m_pHeaderBar->SetItemSize( HI_VALUE, nItemWidth );
    m_pHeaderBar->SetItemSize( HI_ACTION, nButtonWidth );

    for (std::unique_ptr<CustomPropertyLine>& pLine : m_aCustomPropertiesLines)
        SetWidgetWidths(pLine.get());

    SetVisibleLineCount(GetVisibleLineCount());
    ReloadLinesContent();
}

sal_uInt16 CustomPropertiesWindow::GetVisibleLineCount() const
{
    sal_Int32 nScrollOffset = GetLineHeight();
    sal_uInt16 nCount = ceil(static_cast<double>(GetSizePixel().Height()) / nScrollOffset);
    return nCount;
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
    CustomPropertyLine* pNewLine = new CustomPropertyLine( this );
    pNewLine->m_aTypeBox->SetSelectHdl( LINK( this, CustomPropertiesWindow, TypeHdl ) );
    pNewLine->m_aRemoveButton->SetClickHdl( LINK( this, CustomPropertiesWindow, RemoveHdl ) );
    pNewLine->m_aValueEdit->SetLoseFocusHdl( LINK( this, CustomPropertiesWindow, EditLoseFocusHdl ) );
    //add lose focus handlers of date/time fields

    pNewLine->m_aTypeBox->SetLoseFocusHdl( LINK( this, CustomPropertiesWindow, BoxLoseFocusHdl ) );

    pNewLine->m_aNameBox->add_mnemonic_label(m_pHeaderAccName);
    pNewLine->m_aNameBox->SetAccessibleName(m_pHeaderAccName->GetText());
    pNewLine->m_aTypeBox->add_mnemonic_label(m_pHeaderAccType);
    pNewLine->m_aTypeBox->SetAccessibleName(m_pHeaderAccType->GetText());
    pNewLine->m_aValueEdit->add_mnemonic_label(m_pHeaderAccValue);
    pNewLine->m_aValueEdit->SetAccessibleName(m_pHeaderAccValue->GetText());

    sal_Int32 nPos = GetExistingLineCount() * GetLineHeight();
    nPos += LogicToPixel(Size(0, 2), MapMode(MapUnit::MapAppFont)).Height();
    m_aCustomPropertiesLines.emplace_back( pNewLine );

    SetWidgetWidths(pNewLine);
    pNewLine->m_aLine->SetPosSizePixel(Point(0, nPos + m_nScrollPos), Size(GetSizePixel().Width(), m_nWidgetHeight));
    pNewLine->m_aLine->Show();

    TypeHdl(*pNewLine->m_aTypeBox.get());
    pNewLine->m_aNameBox->GrabFocus();
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

void CustomPropertiesWindow::StoreCustomProperties()
{
    sal_uInt32 nDataModelPos = GetCurrentDataModelPosition();

    for (sal_uInt32 i = 0; nDataModelPos + i < GetTotalLineCount() && i < GetExistingLineCount(); i++)
    {
        CustomPropertyLine* pLine = m_aCustomPropertiesLines[i].get();

        OUString sPropertyName = pLine->m_aNameBox->GetText();
        if (!sPropertyName.isEmpty())
        {
            m_aCustomProperties[nDataModelPos + i]->m_sName = sPropertyName;
            long nType = reinterpret_cast<long>(
                pLine->m_aTypeBox->GetSelectedEntryData());
            if (CUSTOM_TYPE_NUMBER == nType)
            {
                double nValue = 0;
                sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex(NF_NUMBER_SYSTEM);
                bool bIsNum = m_aNumberFormatter.
                    IsNumberFormat(pLine->m_aValueEdit->GetText(), nIndex, nValue);
                if (bIsNum)
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= nValue;
            }
            else if (CUSTOM_TYPE_BOOLEAN == nType)
            {
                bool bValue = pLine->m_aYesNoButton->IsYesChecked();
                m_aCustomProperties[nDataModelPos + i]->m_aValue <<= bValue;
            }
            else if (CUSTOM_TYPE_DATETIME == nType)
            {
                Date aTmpDate = pLine->m_aDateField->GetDate();
                tools::Time aTmpTime = pLine->m_aTimeField->GetTime();
                util::DateTime const aDateTime(aTmpTime.GetNanoSec(),
                    aTmpTime.GetSec(), aTmpTime.GetMin(), aTmpTime.GetHour(),
                    aTmpDate.GetDay(), aTmpDate.GetMonth(), aTmpDate.GetYear(),
                    pLine->m_aTimeField->m_isUTC);
                if (pLine->m_aDateField->m_TZ.is_initialized())
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= util::DateTimeWithTimezone(
                        aDateTime, pLine->m_aDateField->m_TZ.get());
                }
                else
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= aDateTime;
                }
            }
            else if (CUSTOM_TYPE_DATE == nType)
            {
                Date aTmpDate = pLine->m_aDateField->GetDate();
                util::Date const aDate(aTmpDate.GetDay(), aTmpDate.GetMonth(),
                    aTmpDate.GetYear());
                if (pLine->m_aDateField->m_TZ.is_initialized())
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= util::DateWithTimezone(
                        aDate, pLine->m_aDateField->m_TZ.get());
                }
                else
                {
                    m_aCustomProperties[nDataModelPos + i]->m_aValue <<= aDate;
                }
            }
            else if (CUSTOM_TYPE_DURATION == nType)
            {
                m_aCustomProperties[nDataModelPos + i]->m_aValue <<= pLine->m_aDurationField->GetDuration();
            }
            else
            {
                OUString sValue(pLine->m_aValueEdit->GetText());
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

        pLine->m_aNameBox->SetText(rName);
        pLine->m_aLine->Show();

        if (!rAny.hasValue())
        {
            pLine->m_aValueEdit->SetText(OUString());
        }
        else if (rAny >>= nTmpValue)
        {
            sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex(NF_NUMBER_SYSTEM);
            m_aNumberFormatter.GetInputLineString(nTmpValue, nIndex, sValue);
            pLine->m_aValueEdit->SetText(sValue);
            nType = CUSTOM_TYPE_NUMBER;
        }
        else if (rAny >>= bTmpValue)
        {
            sValue = (bTmpValue ? rLocaleWrapper.getTrueWord() : rLocaleWrapper.getFalseWord());
            nType = CUSTOM_TYPE_BOOLEAN;
        }
        else if (rAny >>= sTmpValue)
        {
            pLine->m_aValueEdit->SetText(sTmpValue);
            nType = CUSTOM_TYPE_TEXT;
        }
        else if (rAny >>= aTmpDate)
        {
            pLine->m_aDateField->SetDate(Date(aTmpDate));
            nType = CUSTOM_TYPE_DATE;
        }
        else if (rAny >>= aTmpDateTime)
        {
            pLine->m_aDateField->SetDate(Date(aTmpDateTime));
            pLine->m_aTimeField->SetTime(tools::Time(aTmpDateTime));
            pLine->m_aTimeField->m_isUTC = aTmpDateTime.IsUTC;
            nType = CUSTOM_TYPE_DATETIME;
        }
        else if (rAny >>= aTmpDateTZ)
        {
            pLine->m_aDateField->SetDate(Date(aTmpDateTZ.DateInTZ.Day,
                aTmpDateTZ.DateInTZ.Month, aTmpDateTZ.DateInTZ.Year));
            pLine->m_aDateField->m_TZ = aTmpDateTZ.Timezone;
            nType = CUSTOM_TYPE_DATE;
        }
        else if (rAny >>= aTmpDateTimeTZ)
        {
            util::DateTime const& rDT(aTmpDateTimeTZ.DateTimeInTZ);
            pLine->m_aDateField->SetDate(Date(rDT));
            pLine->m_aTimeField->SetTime(tools::Time(rDT));
            pLine->m_aTimeField->m_isUTC = rDT.IsUTC;
            pLine->m_aDateField->m_TZ = aTmpDateTimeTZ.Timezone;
            nType = CUSTOM_TYPE_DATETIME;
        }
        else if (rAny >>= aTmpDuration)
        {
            nType = CUSTOM_TYPE_DURATION;
            pLine->m_aDurationField->SetDuration(aTmpDuration);
        }

        if (nType != CUSTOM_TYPE_UNKNOWN)
        {
            if (CUSTOM_TYPE_BOOLEAN == nType)
            {
                if (bTmpValue)
                    pLine->m_aYesNoButton->CheckYes();
                else
                    pLine->m_aYesNoButton->CheckNo();
            }
            pLine->m_aTypeBox->SelectEntryPos(pLine->m_aTypeBox->GetEntryPos(reinterpret_cast<void*>(nType)));
        }

        TypeHdl(*pLine->m_aTypeBox.get());
    }
    while (nDataModelPos + i >= GetTotalLineCount() && i < GetExistingLineCount())
    {
        CustomPropertyLine* pLine = m_aCustomPropertiesLines[i].get();
        pLine->Hide();
        i++;
    }
}

CustomPropertiesControl::CustomPropertiesControl(vcl::Window* pParent)
    : Window(pParent, WB_HIDE | WB_CLIPCHILDREN | WB_TABSTOP | WB_DIALOGCONTROL | WB_BORDER)
    , m_pVBox(nullptr)
    , m_pHeaderBar(nullptr)
    , m_pBody(nullptr)
    , m_pPropertiesWin(nullptr)
    , m_pVertScroll(nullptr)
    , m_nThumbPos(0)
{
    Size aRequest(LogicToPixel(Size(320, 141), MapMode(MapUnit::MapAppFont)));
    set_width_request(aRequest.Width());
}

void CustomPropertiesControl::Init(VclBuilderContainer& rBuilder)
{
    m_pVBox = VclPtr<VclVBox>::Create(this);
    m_pHeaderBar = VclPtr<HeaderBar>::Create(m_pVBox, WB_BUTTONSTYLE | WB_BOTTOMBORDER);
    m_pBody = VclPtr<VclHBox>::Create(m_pVBox);
    FixedText* pName = rBuilder.get<FixedText>("name");
    FixedText* pType = rBuilder.get<FixedText>("type");
    FixedText* pValue = rBuilder.get<FixedText>("value");
    OUString sName = pName->GetText();
    OUString sType = pType->GetText();
    OUString sValue = pValue->GetText();
    m_pPropertiesWin = VclPtr<CustomPropertiesWindow>::Create(m_pBody, pName, pType, pValue);
    m_pVertScroll = VclPtr<ScrollBar>::Create(m_pBody, WB_VERT);
    m_pPropertiesWin->Init(m_pHeaderBar, m_pVertScroll);

    set_hexpand(true);
    set_vexpand(true);
    set_expand(true);
    set_fill(true);

    m_pVBox->set_hexpand(true);
    m_pVBox->set_vexpand(true);
    m_pVBox->set_expand(true);
    m_pVBox->set_fill(true);
    m_pVBox->Show();

    m_pBody->set_hexpand(true);
    m_pBody->set_vexpand(true);
    m_pBody->set_expand(true);
    m_pBody->set_fill(true);
    m_pBody->Show();

    m_pPropertiesWin->set_hexpand(true);
    m_pPropertiesWin->set_vexpand(true);
    m_pPropertiesWin->set_expand(true);
    m_pPropertiesWin->set_fill(true);
    m_pPropertiesWin->Show();

    m_pPropertiesWin->SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
    m_pVertScroll->EnableDrag();
    m_pVertScroll->Show();

    m_pHeaderBar->set_height_request(GetTextHeight() + 6);

    const HeaderBarItemBits nHeadBits = HeaderBarItemBits::FIXED | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::LEFT;

    m_pHeaderBar->InsertItem( HI_NAME, sName, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_TYPE, sType, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_VALUE, sValue, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_ACTION, OUString(), 0, nHeadBits );
    m_pHeaderBar->Show();

    m_pPropertiesWin->SetRemovedHdl( LINK( this, CustomPropertiesControl, RemovedHdl ) );

    m_pVertScroll->SetRangeMin( 0 );
    m_pVertScroll->SetRangeMax( 0 );
    m_pVertScroll->SetVisibleSize( 0xFFFF );

    Link<ScrollBar*,void> aScrollLink = LINK( this, CustomPropertiesControl, ScrollHdl );
    m_pVertScroll->SetScrollHdl( aScrollLink );
}

void CustomPropertiesControl::Resize()
{
    if (m_pVBox)
    {
        m_pVBox->SetSizePixel(GetSizePixel());
        sal_Int32 nScrollOffset = m_pPropertiesWin->GetLineHeight();
        sal_Int32 nVisibleEntries = m_pPropertiesWin->GetSizePixel().Height() / nScrollOffset;
        m_pPropertiesWin->SetVisibleLineCount( nVisibleEntries );
        m_pVertScroll->SetPageSize( nVisibleEntries - 1 );
        m_pVertScroll->SetVisibleSize( nVisibleEntries );
    }
    Window::Resize();
}

VCL_BUILDER_FACTORY(CustomPropertiesControl)

CustomPropertiesControl::~CustomPropertiesControl()
{
    disposeOnce();
}

void CustomPropertiesControl::dispose()
{
    m_pVertScroll.disposeAndClear();
    m_pPropertiesWin.disposeAndClear();
    m_pBody.disposeAndClear();
    m_pHeaderBar.disposeAndClear();
    m_pVBox.disposeAndClear();
    vcl::Window::dispose();
}

IMPL_LINK( CustomPropertiesControl, ScrollHdl, ScrollBar*, pScrollBar, void )
{
    sal_Int32 nOffset = m_pPropertiesWin->GetLineHeight();
    nOffset *= ( m_nThumbPos - pScrollBar->GetThumbPos() );
    m_nThumbPos = pScrollBar->GetThumbPos();
    m_pPropertiesWin->DoScroll( nOffset );
}

IMPL_LINK_NOARG(CustomPropertiesControl, RemovedHdl, void*, void)
{
    long nLineCount = m_pPropertiesWin->GetTotalLineCount();
    m_pVertScroll->SetRangeMax(nLineCount + 1);
    if ( m_pPropertiesWin->GetTotalLineCount() > m_pPropertiesWin->GetExistingLineCount() )
        m_pVertScroll->DoScrollAction ( ScrollType::LineUp );
}

void CustomPropertiesControl::AddLine( Any const & rAny )
{
    m_pPropertiesWin->AddLine( OUString(), rAny );
    long nLineCount = m_pPropertiesWin->GetTotalLineCount();
    m_pVertScroll->SetRangeMax(nLineCount + 1);
    if ( m_pPropertiesWin->GetOutputSizePixel().Height() < nLineCount * m_pPropertiesWin->GetLineHeight() )
        m_pVertScroll->DoScroll(nLineCount + 1);
}

void CustomPropertiesControl::SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties)
{
    m_pPropertiesWin->SetCustomProperties(std::move(rProperties));
    long nLineCount = m_pPropertiesWin->GetTotalLineCount();
    m_pVertScroll->SetRangeMax(nLineCount + 1);
}

// class SfxCustomPropertiesPage -----------------------------------------
SfxCustomPropertiesPage::SfxCustomPropertiesPage( vcl::Window* pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "CustomInfoPage", "sfx/ui/custominfopage.ui", &rItemSet)
{
    get(m_pPropertiesCtrl, "properties");
    m_pPropertiesCtrl->Init(*this);
    get<PushButton>("add")->SetClickHdl(LINK(this, SfxCustomPropertiesPage, AddHdl));
}

SfxCustomPropertiesPage::~SfxCustomPropertiesPage()
{
    disposeOnce();
}

void SfxCustomPropertiesPage::dispose()
{
    m_pPropertiesCtrl.clear();
    SfxTabPage::dispose();
}

IMPL_LINK_NOARG(SfxCustomPropertiesPage, AddHdl, Button*, void)
{
    // tdf#115853: reload current lines before adding a brand new one
    // indeed the info are deleted by ClearCustomProperties
    // each time SfxDocumentInfoItem destructor is called
    SfxDocumentInfoItem pInfo;
    Sequence< beans::PropertyValue > aPropertySeq = m_pPropertiesCtrl->GetCustomProperties();
    sal_Int32 i = 0, nCount = aPropertySeq.getLength();
    for ( ; i < nCount; ++i )
    {
        if ( !aPropertySeq[i].Name.isEmpty() )
        {
            pInfo.AddCustomProperty( aPropertySeq[i].Name, aPropertySeq[i].Value );
        }
    }

    Any aAny;
    m_pPropertiesCtrl->AddLine(aAny);
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
        Sequence< beans::PropertyValue > aPropertySeq = m_pPropertiesCtrl->GetCustomProperties();
        sal_Int32 i = 0, nCount = aPropertySeq.getLength();
        for ( ; i < nCount; ++i )
        {
            if ( !aPropertySeq[i].Name.isEmpty() )
                pInfo->AddCustomProperty( aPropertySeq[i].Name, aPropertySeq[i].Value );
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
    m_pPropertiesCtrl->ClearAllLines();
    const SfxDocumentInfoItem& rInfoItem = rItemSet->Get(SID_DOCINFO);
    std::vector< std::unique_ptr<CustomProperty> > aCustomProps = rInfoItem.GetCustomProperties();
    m_pPropertiesCtrl->SetCustomProperties(std::move(aCustomProps));
}

DeactivateRC SfxCustomPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    DeactivateRC nRet = DeactivateRC::LeavePage;
    if ( !m_pPropertiesCtrl->AreAllLinesValid() )
        nRet = DeactivateRC::KeepPage;
    return nRet;
}

VclPtr<SfxTabPage> SfxCustomPropertiesPage::Create( TabPageParent pParent, const SfxItemSet* rItemSet )
{
    return VclPtr<SfxCustomPropertiesPage>::Create( pParent.pParent, *rItemSet );
}

CmisValue::CmisValue( vcl::Window* pParent, const OUString& aStr )
{
    m_pUIBuilder.reset(new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui"));
    get( m_aValueEdit, "value");
    m_aValueEdit->Show();
    m_aValueEdit->SetText( aStr );
}

CmisDateTime::CmisDateTime( vcl::Window* pParent, const util::DateTime& aDateTime )
{
    m_pUIBuilder.reset(new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui"));
    get( m_aDateField, "date");
    get( m_aTimeField, "time");
    m_aDateField->Show();
    m_aTimeField->Show();
    m_aDateField->SetDate( Date( aDateTime ) );
    m_aTimeField->SetTime( tools::Time( aDateTime ) );
}

CmisYesNo::CmisYesNo( vcl::Window* pParent, bool bValue )
{
    m_pUIBuilder.reset(new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui"));
    get( m_aYesButton, "yes");
    get( m_aNoButton, "no");
    m_aYesButton->Show();
    m_aNoButton->Show();
    if ( bValue )
        m_aYesButton->Check( );
    else
        m_aNoButton->Check( );
}

// struct CmisPropertyLine ---------------------------------------------
CmisPropertyLine::CmisPropertyLine(vcl::Window* pParent)
    : m_sType(CMIS_TYPE_STRING)
    , m_bUpdatable(false)
    , m_bRequired(false)
    , m_bMultiValued(false)
    , m_bOpenChoice(false)
{
    m_pUIBuilder.reset(new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui"));
    get( m_pFrame, "CmisFrame" );
    get( m_aName, "name" );
    get( m_aType, "type" );
    m_pFrame->Enable();
}

CmisPropertyLine::~CmisPropertyLine( )
{
    m_aValues.clear();
    m_aYesNos.clear();
    m_aDateTimes.clear();
}

long CmisPropertyLine::getItemHeight() const
{
    return VclContainer::getLayoutRequisition(*m_pFrame).Height();
}

// class CmisPropertiesWindow -----------------------------------------

CmisPropertiesWindow::CmisPropertiesWindow(SfxTabPage* pParent):
    m_aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() )
{
    pParent->get(m_pBox, "CmisWindow");
    CmisPropertyLine aTemp( m_pBox );
    m_nItemHeight = aTemp.getItemHeight();
}

CmisPropertiesWindow::~CmisPropertiesWindow()
{
    ClearAllLines();
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
    std::unique_ptr<CmisPropertyLine> pNewLine(new CmisPropertyLine( m_pBox ));

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
        sal_Int32 nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < nNumValue; ++i )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( seqValue[i], nIndex, sValue );
            std::unique_ptr<CmisValue> pValue(new CmisValue( m_pBox, sValue ));
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }
    }
    else if ( sType == CMIS_TYPE_DECIMAL )
    {
        Sequence< double > seqValue;
        rAny >>= seqValue;
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        sal_Int32 nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < nNumValue; ++i )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( seqValue[i], nIndex, sValue );
            std::unique_ptr<CmisValue> pValue(new CmisValue( m_pBox, sValue ));
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }

    }
    else if ( sType == CMIS_TYPE_BOOL )
    {
        Sequence<sal_Bool> seqValue;
        rAny >>= seqValue;
        sal_Int32 nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < nNumValue; ++i )
        {
            std::unique_ptr<CmisYesNo> pYesNo(new CmisYesNo( m_pBox, seqValue[i] ));
            pYesNo->m_aYesButton->Enable( bUpdatable );
            pYesNo->m_aNoButton->Enable( bUpdatable );
            pNewLine->m_aYesNos.push_back( std::move(pYesNo) );
        }
    }
    else if ( sType == CMIS_TYPE_STRING )
    {
        Sequence< OUString > seqValue;
        rAny >>= seqValue;
        sal_Int32 nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < nNumValue; ++i )
        {
            std::unique_ptr<CmisValue> pValue(new CmisValue( m_pBox, seqValue[i] ));
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( std::move(pValue) );
        }
    }
    else if ( sType == CMIS_TYPE_DATETIME )
    {
        Sequence< util::DateTime > seqValue;
        rAny >>= seqValue;
        sal_Int32 nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < nNumValue; ++i )
        {
            std::unique_ptr<CmisDateTime> pDateTime(new CmisDateTime( m_pBox, seqValue[i]));
            pDateTime->m_aDateField->SetReadOnly( !bUpdatable );
            pDateTime->m_aTimeField->SetReadOnly( !bUpdatable );
            pNewLine->m_aDateTimes.push_back( std::move(pDateTime) );
        }
    }
    pNewLine->m_aName->SetText( sName );
    pNewLine->m_aName->Show();
    pNewLine->m_aType->SetText( sType );
    pNewLine->m_aType->Show();

    m_aCmisPropertiesLines.push_back( std::move(pNewLine) );
}

void CmisPropertiesWindow::DoScroll( sal_Int32 nNewPos )
{
    m_pBox->SetPosPixel(Point(0, nNewPos));
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

        OUString sPropertyName = pLine->m_aName->GetText();
        if ( !sPropertyName.isEmpty() )
        {
            aPropertiesSeq[i].Name = sPropertyName;
            OUString sType = pLine->m_aType->GetText( );
            if ( CMIS_TYPE_DECIMAL == sType )
            {
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                Sequence< double > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( auto& rxValue : pLine->m_aValues )
                {
                    double dValue = 0.0;
                    OUString sValue( rxValue->m_aValueEdit->GetText() );
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
                for ( auto& rxValue : pLine->m_aValues )
                {
                    double dValue = 0;
                    OUString sValue( rxValue->m_aValueEdit->GetText() );
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
                for ( auto& rxYesNo : pLine->m_aYesNos )
                {
                    bool bValue = rxYesNo->m_aYesButton->IsChecked();
                    seqValue[k] = bValue;
                    ++k;
                }
                aPropertiesSeq[i].Value <<= seqValue;

            }
            else if ( CMIS_TYPE_DATETIME == sType )
            {
                Sequence< util::DateTime > seqValue( pLine->m_aDateTimes.size( ) );
                sal_Int32 k = 0;
                for ( auto& rxDateTime : pLine->m_aDateTimes )
                {
                    Date aTmpDate = rxDateTime->m_aDateField->GetDate();
                    tools::Time aTmpTime = rxDateTime->m_aTimeField->GetTime();
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
                for ( auto& rxValue : pLine->m_aValues )
                {
                    OUString sValue( rxValue->m_aValueEdit->GetText() );
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

CmisPropertiesControl::CmisPropertiesControl(SfxTabPage* pParent)
    : m_pPropertiesWin( pParent )
    , m_rScrolledWindow( *pParent->get<VclScrolledWindow>("CmisScroll"))
    , m_rVertScroll( m_rScrolledWindow.getVertScrollBar())
{
    m_rScrolledWindow.setUserManagedScrolling(true);
    m_rVertScroll.EnableDrag();
    m_rVertScroll.Show( m_rScrolledWindow.GetStyle() & WB_VSCROLL);
    m_rVertScroll.SetRangeMin(0);
    m_rVertScroll.SetVisibleSize( 0xFFFF );

    Link<ScrollBar*,void> aScrollLink = LINK( this, CmisPropertiesControl, ScrollHdl );
    m_rVertScroll.SetScrollHdl( aScrollLink );
}

void CmisPropertiesControl::ClearAllLines()
{
   m_pPropertiesWin.ClearAllLines();
}

IMPL_LINK( CmisPropertiesControl, ScrollHdl, ScrollBar*, pScrollBar, void )
{
    sal_Int32 nOffset = m_pPropertiesWin.GetItemHeight();
    nOffset *= ( pScrollBar->GetThumbPos() );
    m_pPropertiesWin.DoScroll( -nOffset );
}

void CmisPropertiesControl::checkAutoVScroll()
{
    WinBits nBits = m_rScrolledWindow.GetStyle();
    if (nBits & WB_VSCROLL)
        return;
    if (nBits & WB_AUTOVSCROLL)
    {
        bool bShow = m_rVertScroll.GetRangeMax() > m_rVertScroll.GetVisibleSize();
        if (bShow != m_rVertScroll.IsVisible())
            m_rVertScroll.Show(bShow);
    }
}

void CmisPropertiesControl::setScrollRange()
{
    sal_Int32 nScrollOffset = m_pPropertiesWin.GetItemHeight();
    sal_Int32 nVisibleItems = m_rScrolledWindow.getVisibleChildSize().Height() / nScrollOffset;
    m_rVertScroll.SetPageSize( nVisibleItems - 1 );
    m_rVertScroll.SetVisibleSize( nVisibleItems );
    m_rVertScroll.Scroll();
    checkAutoVScroll();
}

void CmisPropertiesControl::AddLine( const OUString& sId, const OUString& sName,
                                     const OUString& sType, const bool bUpdatable,
                                     const bool bRequired, const bool bMultiValued,
                                     const bool bOpenChoice, Any& aChoices, Any const & rAny
                                     )
{
    m_pPropertiesWin.AddLine( sId, sName, sType, bUpdatable, bRequired, bMultiValued,
                               bOpenChoice, aChoices, rAny );
    //compute logical elements
    sal_Int32 nLogicElements = ( m_pPropertiesWin.getBoxHeight()
                                 + m_pPropertiesWin.GetItemHeight() ) / m_pPropertiesWin.GetItemHeight();
    m_rVertScroll.SetRangeMax( nLogicElements );
    m_rVertScroll.DoScroll( nLogicElements );
    checkAutoVScroll();
}

// class SfxCmisPropertiesPage -----------------------------------------
SfxCmisPropertiesPage::SfxCmisPropertiesPage( vcl::Window* pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "CmisInfoPage", "sfx/ui/cmisinfopage.ui", &rItemSet)
    , m_pPropertiesCtrl( this )
{
}

SfxCmisPropertiesPage::~SfxCmisPropertiesPage()
{
    disposeOnce();
}

void SfxCmisPropertiesPage::dispose()
{
    m_pPropertiesCtrl.ClearAllLines();
    SfxTabPage::dispose();
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
        Sequence< document::CmisProperty > aNewProps = m_pPropertiesCtrl.GetCmisProperties();

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
                    for ( sal_Int32 ii = 0; ii < oldValue.getLength( ); ++ii )
                    {
                        oldValue[ii].NanoSeconds = 0;
                        oldValue[ii].Seconds = 0;
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
    m_pPropertiesCtrl.ClearAllLines();
    const SfxDocumentInfoItem& rInfoItem = rItemSet->Get(SID_DOCINFO);
    uno::Sequence< document::CmisProperty > aCmisProps = rInfoItem.GetCmisProperties();
    for ( sal_Int32 i = 0; i < aCmisProps.getLength(); i++ )
    {
        m_pPropertiesCtrl.AddLine( aCmisProps[i].Id,
                                   aCmisProps[i].Name,
                                   aCmisProps[i].Type,
                                   aCmisProps[i].Updatable,
                                   aCmisProps[i].Required,
                                   aCmisProps[i].MultiValued,
                                   aCmisProps[i].OpenChoice,
                                   aCmisProps[i].Choices,
                                   aCmisProps[i].Value );
    }
    m_pPropertiesCtrl.setScrollRange();
}

DeactivateRC SfxCmisPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SfxCmisPropertiesPage::Create( TabPageParent pParent, const SfxItemSet* rItemSet )
{
    return VclPtr<SfxCmisPropertiesPage>::Create( pParent.pParent, *rItemSet );
}

void SfxCmisPropertiesPage::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    SfxTabPage::SetPosSizePixel(rAllocPos, rAllocation);
    m_pPropertiesCtrl.setScrollRange();
}

void SfxCmisPropertiesPage::SetSizePixel(const Size& rAllocation)
{
    SfxTabPage::SetSizePixel(rAllocation);
    m_pPropertiesCtrl.setScrollRange();
}

void SfxCmisPropertiesPage::SetPosPixel(const Point& rAllocPos)
{
    SfxTabPage::SetPosPixel(rAllocPos);
    m_pPropertiesCtrl.setScrollRange();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
