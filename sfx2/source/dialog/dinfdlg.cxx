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

#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/cmdoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/urihelper.hxx>
#include <unotools/useroptions.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/datetime.hxx>

#include <memory>

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
#include <sfx2/dinfdlg.hxx>
#include <sfx2/securitypage.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include "helper.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>

#include "documentfontsdialog.hxx"
#include <sfx2/sfx.hrc>
#include "dinfdlg.hrc"
#include "../appl/app.hrc"
#include "sfxlocal.hrc"
#include <dialog.hrc>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>

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

static
bool operator==(const util::DateTime &i_rLeft, const util::DateTime &i_rRight)
{
    return i_rLeft.Year             == i_rRight.Year
        && i_rLeft.Month            == i_rRight.Month
        && i_rLeft.Day              == i_rRight.Day
        && i_rLeft.Hours            == i_rRight.Hours
        && i_rLeft.Minutes          == i_rRight.Minutes
        && i_rLeft.Seconds          == i_rRight.Seconds
        && i_rLeft.NanoSeconds      == i_rRight.NanoSeconds
        && i_rLeft.IsUTC            == i_rRight.IsUTC;
}

SfxPoolItem* SfxDocumentInfoItem::CreateDefault() { return new SfxDocumentInfoItem; }

const sal_uInt16 HI_NAME = 1;
const sal_uInt16 HI_TYPE = 2;
const sal_uInt16 HI_VALUE = 3;
const sal_uInt16 HI_ACTION = 4;

static const char DOCUMENT_SIGNATURE_MENU_CMD[] = "Signature";


namespace {

OUString CreateSizeText( sal_Int64 nSize )
{
    OUString aUnitStr(" ");
    aUnitStr += SfxResId(STR_BYTES).toString();
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
        aUnitStr += SfxResId(STR_KB).toString();
        fSize /= 1024;
        nDec = 0;
    }
    else if ( nSize1 >= nMega && nSize1 < nGiga )
    {
        nSize1 /= nMega;
        aUnitStr = " ";
        aUnitStr += SfxResId(STR_MB).toString();
        fSize /= nMega;
        nDec = 2;
    }
    else if ( nSize1 >= nGiga )
    {
        nSize1 /= nGiga;
        aUnitStr = " ";
        aUnitStr += SfxResId(STR_GB).toString();
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
                rLocaleWrapper.getNumDecimalSep()[0] );
        aSizeStr += aUnitStr;

        aSizeStr += " (";
        aSizeStr += rLocaleWrapper.getNum( nSize2, 0 );
        aSizeStr += " ";
        aSizeStr += SfxResId(STR_BYTES).toString();
        aSizeStr += ")";
    }
    return aSizeStr;
}

OUString ConvertDateTime_Impl( const OUString& rName,
    const util::DateTime& uDT, const LocaleDataWrapper& rWrapper )
{
     Date aD(uDT);
     tools::Time aT(uDT);
     const OUString aDelim( ", " );
     OUString aStr( rWrapper.getDate( aD ) );
     aStr += aDelim;
     aStr += rWrapper.getTime( aT );
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
                    SAL_WARN( "sfx2", "non-removable user-defined property?");
                    continue;
                }

                uno::Any aValue = xSet->getPropertyValue(pProps[i].Name);
                CustomProperty* pProp = new CustomProperty( pProps[i].Name, aValue );
                m_aCustomProperties.push_back( pProp );
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
    for ( size_t i = 0; i < rItem.m_aCustomProperties.size(); i++ )
    {
        CustomProperty* pProp = new CustomProperty( rItem.m_aCustomProperties[i]->m_sName,
                                                    rItem.m_aCustomProperties[i]->m_aValue );
        m_aCustomProperties.push_back( pProp );
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
    setCreationDate( now.GetUNODateTime() );
    setModifiedBy(OUString());
    setPrintedBy(OUString());
    setModificationDate(util::DateTime());
    setPrintDate(util::DateTime());
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

        for ( size_t k = 0; k < m_aCustomProperties.size(); ++k )
        {
            try
            {
                xContainer->addProperty( m_aCustomProperties[k]->m_sName,
                    beans::PropertyAttribute::REMOVABLE, m_aCustomProperties[k]->m_aValue );
            }
            catch ( Exception& )
            {
                SAL_WARN( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while adding custom properties" );
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "sfx.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while removing custom properties" );
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

std::vector< CustomProperty* > SfxDocumentInfoItem::GetCustomProperties() const
{
    std::vector< CustomProperty* > aRet;
    for ( size_t i = 0; i < m_aCustomProperties.size(); i++ )
    {
        CustomProperty* pProp = new CustomProperty( m_aCustomProperties[i]->m_sName,
                                                    m_aCustomProperties[i]->m_aValue );
        aRet.push_back( pProp );
    }

    return aRet;
}

void SfxDocumentInfoItem::ClearCustomProperties()
{
    for ( size_t i = 0; i < m_aCustomProperties.size(); i++ )
        delete m_aCustomProperties[i];
    m_aCustomProperties.clear();
}

void SfxDocumentInfoItem::AddCustomProperty( const OUString& sName, const Any& rValue )
{
    CustomProperty* pProp = new CustomProperty( sName, rValue );
    m_aCustomProperties.push_back( pProp );
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
        rVal <<= OUString( aValue );
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

SfxDocumentDescPage::SfxDocumentDescPage( vcl::Window * pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "DescriptionInfoPage", "sfx/ui/descriptioninfopage.ui", &rItemSet)
    , m_pInfoItem   ( nullptr )

{
    get(m_pTitleEd, "title");
    get(m_pThemaEd, "subject");
    get(m_pKeywordsEd, "keywords");
    get(m_pCommentEd, "comments");
    m_pCommentEd->set_width_request(m_pKeywordsEd->get_preferred_size().Width());
    m_pCommentEd->set_height_request(m_pCommentEd->GetTextHeight() * 16);
}

SfxDocumentDescPage::~SfxDocumentDescPage()
{
    disposeOnce();
}

void SfxDocumentDescPage::dispose()
{
    m_pTitleEd.clear();
    m_pThemaEd.clear();
    m_pKeywordsEd.clear();
    m_pCommentEd.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SfxDocumentDescPage::Create(vcl::Window *pParent, const SfxItemSet *rItemSet)
{
     return VclPtr<SfxDocumentDescPage>::Create(pParent, *rItemSet);
}

bool SfxDocumentDescPage::FillItemSet(SfxItemSet *rSet)
{
    // Test whether a change is present
    const bool bTitleMod = m_pTitleEd->IsModified();
    const bool bThemeMod = m_pThemaEd->IsModified();
    const bool bKeywordsMod = m_pKeywordsEd->IsModified();
    const bool bCommentMod = m_pCommentEd->IsModified();
    if ( !( bTitleMod || bThemeMod || bKeywordsMod || bCommentMod ) )
    {
        return false;
    }

    // Generating the output data
    const SfxPoolItem* pItem = nullptr;
    SfxDocumentInfoItem* pInfo = nullptr;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = nullptr;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

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
        pInfo->setTitle( m_pTitleEd->GetText() );
    }
    if ( bThemeMod )
    {
        pInfo->setSubject( m_pThemaEd->GetText() );
    }
    if ( bKeywordsMod )
    {
        pInfo->setKeywords( m_pKeywordsEd->GetText() );
    }
    if ( bCommentMod )
    {
        pInfo->setDescription( m_pCommentEd->GetText() );
    }
    rSet->Put( SfxDocumentInfoItem( *pInfo ) );
    if ( pInfo != m_pInfoItem )
    {
        delete pInfo;
    }

    return true;
}


void SfxDocumentDescPage::Reset(const SfxItemSet *rSet)
{
    m_pInfoItem = const_cast<SfxDocumentInfoItem*>(&static_cast<const SfxDocumentInfoItem &>(rSet->Get(SID_DOCINFO)));

    m_pTitleEd->SetText( m_pInfoItem->getTitle() );
    m_pThemaEd->SetText( m_pInfoItem->getSubject() );
    m_pKeywordsEd->SetText( m_pInfoItem->getKeywords() );
    m_pCommentEd->SetText( m_pInfoItem->getDescription() );

    const SfxBoolItem* pROItem = SfxItemSet::GetItem<SfxBoolItem>(rSet, SID_DOC_READONLY, false);
    if ( pROItem && pROItem->GetValue() )
    {
        m_pTitleEd->SetReadOnly();
        m_pThemaEd->SetReadOnly();
        m_pKeywordsEd->SetReadOnly();
        m_pCommentEd->SetReadOnly();
    }
}


namespace
{
    OUString GetDateTimeString( sal_Int32 _nDate, sal_Int32 _nTime )
    {
        const LocaleDataWrapper& rWrapper( Application::GetSettings().GetLocaleDataWrapper() );

        Date aDate( _nDate );
        tools::Time aTime( _nTime );
        OUString aStr( rWrapper.getDate( aDate ) );
        aStr += ", ";
        aStr += rWrapper.getTime( aTime );
        return aStr;
    }

    // copy from xmlsecurity/source/dialog/resourcemanager.cxx
    OUString GetContentPart( const OUString& _rRawString, const OUString& _rPartId )
    {
        OUString s;

        sal_Int32  nContStart = _rRawString.indexOf( _rPartId );
        if ( nContStart != -1 )
        {
            nContStart = nContStart + _rPartId.getLength();
            ++nContStart; // now its start of content, directly after Id

            sal_Int32  nContEnd = _rRawString.indexOf( ',', nContStart );
            if (nContEnd != -1)
                s = _rRawString.copy( nContStart, nContEnd - nContStart );
            else
                s = _rRawString.copy(nContStart);
        }

        return s;
    }
}

SfxDocumentPage::SfxDocumentPage(vcl::Window* pParent, const SfxItemSet& rItemSet)
    : SfxTabPage(pParent, "DocumentInfoPage", "sfx/ui/documentinfopage.ui", &rItemSet)
    , bEnableUseUserData( false )
    , bHandleDelete( false )
{
    get(m_pBmp, "icon");
    get(m_pNameED, "nameed");
    get(m_pChangePassBtn, "changepass");

    get(m_pShowTypeFT, "showtype");
    get(m_pFileValEd, "showlocation");
    get(m_pShowSizeFT, "showsize");
    m_aUnknownSize = m_pShowSizeFT->GetText();
    m_pShowSizeFT->SetText(OUString());

    get(m_pCreateValFt, "showcreate");
    get(m_pChangeValFt, "showmodify");
    get(m_pSignedValFt, "showsigned");
    m_aMultiSignedStr = m_pSignedValFt->GetText();
    m_pSignedValFt->SetText(OUString());
    get(m_pSignatureBtn, "signature");
    get(m_pPrintValFt, "showprint");
    get(m_pTimeLogValFt, "showedittime");
    get(m_pDocNoValFt, "showrevision");

    get(m_pUseUserDataCB, "userdatacb");
    get(m_pUseThumbnailSaveCB, "thumbnailsavecb");
    get(m_pDeleteBtn, "reset");

    get(m_pTemplFt, "templateft");
    get(m_pTemplValFt, "showtemplate");

    ImplUpdateSignatures();
    ImplCheckPasswordState();
    m_pChangePassBtn->SetClickHdl( LINK( this, SfxDocumentPage, ChangePassHdl ) );
    m_pSignatureBtn->SetClickHdl( LINK( this, SfxDocumentPage, SignatureHdl ) );
    m_pDeleteBtn->SetClickHdl( LINK( this, SfxDocumentPage, DeleteHdl ) );

    // [i96288] Check if the document signature command is enabled
    // on the main list enable/disable the pushbutton accordingly
    SvtCommandOptions aCmdOptions;
    if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED,
                             DOCUMENT_SIGNATURE_MENU_CMD ) )
        m_pSignatureBtn->Disable();
}

SfxDocumentPage::~SfxDocumentPage()
{
    disposeOnce();
}

void SfxDocumentPage::dispose()
{
    m_pBmp.clear();
    m_pNameED.clear();
    m_pChangePassBtn.clear();
    m_pShowTypeFT.clear();
    m_pFileValEd.clear();
    m_pShowSizeFT.clear();
    m_pCreateValFt.clear();
    m_pChangeValFt.clear();
    m_pSignedValFt.clear();
    m_pSignatureBtn.clear();
    m_pPrintValFt.clear();
    m_pTimeLogValFt.clear();
    m_pDocNoValFt.clear();
    m_pUseUserDataCB.clear();
    m_pDeleteBtn.clear();
    m_pTemplFt.clear();
    m_pTemplValFt.clear();
    m_pUseThumbnailSaveCB.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_NOARG_TYPED(SfxDocumentPage, DeleteHdl, Button*, void)
{
    OUString aName;
    if ( bEnableUseUserData && m_pUseUserDataCB->IsChecked() )
        aName = SvtUserOptions().GetFullName();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    DateTime now( DateTime::SYSTEM );
    util::DateTime uDT( now.GetUNODateTime() );
    m_pCreateValFt->SetText( ConvertDateTime_Impl( aName, uDT, rLocaleWrapper ) );
    m_pChangeValFt->SetText( "" );
    m_pPrintValFt->SetText( "" );
    const tools::Time aTime( 0 );
    m_pTimeLogValFt->SetText( rLocaleWrapper.getDuration( aTime ) );
    m_pDocNoValFt->SetText(OUString('1'));
    bHandleDelete = true;
}

IMPL_LINK_NOARG_TYPED(SfxDocumentPage, SignatureHdl, Button*, void)
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if( pDoc )
    {
        pDoc->SignDocumentContent();

        ImplUpdateSignatures();
    }
}

IMPL_STATIC_LINK_NOARG_TYPED(SfxDocumentPage, ChangePassHdl, Button*, void)
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    do
    {
        if (!pShell)
            break;
        SfxItemSet* pMedSet = pShell->GetMedium()->GetItemSet();
        if (!pMedSet)
            break;
        const SfxFilter* pFilter = pShell->GetMedium()->GetFilter();
        if (!pFilter)
            break;

        OUString aDocName;
        sfx2::RequestPassword(pFilter, aDocName, pMedSet);
        pShell->SetModified();
    }
    while (false);
}

void SfxDocumentPage::ImplUpdateSignatures()
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if ( pDoc )
    {
        SfxMedium* pMedium = pDoc->GetMedium();
        if ( pMedium && !pMedium->GetName().isEmpty() && pMedium->GetStorage().is() )
        {
            Reference< security::XDocumentDigitalSignatures > xD(
                security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext()) );

            OUString s;
            Sequence< security::DocumentSignatureInformation > aInfos;
            aInfos = xD->verifyDocumentContentSignatures( pMedium->GetZipStorageToSign_Impl(),
                                                            uno::Reference< io::XInputStream >() );
            if ( aInfos.getLength() > 1 )
                s = m_aMultiSignedStr;
            else if ( aInfos.getLength() == 1 )
            {
                OUString aCN_Id("CN");
                const security::DocumentSignatureInformation& rInfo = aInfos[ 0 ];
                s = GetDateTimeString( rInfo.SignatureDate, rInfo.SignatureTime );
                s += ", ";
                s += GetContentPart( rInfo.Signer->getSubjectName(), aCN_Id );
            }
            m_pSignedValFt->SetText( s );
        }
    }
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
        m_pChangePassBtn->Enable();
        return;
    }
    while (false);
    m_pChangePassBtn->Disable();
}

VclPtr<SfxTabPage> SfxDocumentPage::Create( vcl::Window* pParent, const SfxItemSet* rItemSet )
{
     return VclPtr<SfxDocumentPage>::Create( pParent, *rItemSet );
}

void SfxDocumentPage::EnableUseUserData()
{
    bEnableUseUserData = true;
    m_pUseUserDataCB->Show();
    m_pDeleteBtn->Show();
}

bool SfxDocumentPage::FillItemSet( SfxItemSet* rSet )
{
    bool bRet = false;

    if ( !bHandleDelete && bEnableUseUserData &&
         m_pUseUserDataCB->IsValueChangedFromSaved() &&
         GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* m_pInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseData = ( TRISTATE_TRUE == m_pUseUserDataCB->GetState() );
            const_cast<SfxDocumentInfoItem*>(m_pInfoItem)->SetUseUserData( bUseData );
            rSet->Put( SfxDocumentInfoItem( *m_pInfoItem ) );
            bRet = true;
        }
    }

    if ( bHandleDelete )
    {
        SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* pInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseAuthor = bEnableUseUserData && m_pUseUserDataCB->IsChecked();
            SfxDocumentInfoItem newItem( *pInfoItem );
            newItem.resetUserData( bUseAuthor
                ? SvtUserOptions().GetFullName()
                : OUString() );
            const_cast<SfxDocumentInfoItem*>(pInfoItem)->SetUseUserData( TRISTATE_TRUE == m_pUseUserDataCB->GetState() );
            newItem.SetUseUserData( TRISTATE_TRUE == m_pUseUserDataCB->GetState() );

            newItem.SetDeleteUserData( true );
            rSet->Put( newItem );
            bRet = true;
        }
    }

    if ( m_pUseThumbnailSaveCB->IsValueChangedFromSaved() &&
       GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SfxItemState::SET == pExpSet->GetItemState( SID_DOCINFO, true, &pItem ) )
        {
            const SfxDocumentInfoItem* m_pxInfoItem = static_cast<const SfxDocumentInfoItem*>(pItem);
            bool bUseThumbnail = ( TRISTATE_TRUE == m_pUseThumbnailSaveCB->GetState() );
            const_cast<SfxDocumentInfoItem*>(m_pxInfoItem)->SetUseThumbnailSave( bUseThumbnail );
            rSet->Put( SfxDocumentInfoItem( *m_pxInfoItem ) );
            bRet = true;
        }
    }

    return bRet;
}

void SfxDocumentPage::Reset( const SfxItemSet* rSet )
{
    // Determine the document information
    const SfxDocumentInfoItem& rInfoItem =
        static_cast<const SfxDocumentInfoItem &>(rSet->Get(SID_DOCINFO));

    // template data
    if ( rInfoItem.HasTemplate() )
        m_pTemplValFt->SetText( rInfoItem.getTemplateName() );
    else
    {
        m_pTemplFt->Hide();
        m_pTemplValFt->Hide();
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
    OUString aName = aURL.GetName( INetURLObject::DECODE_WITH_CHARSET );
    if ( aName.isEmpty() || aURL.GetProtocol() == INetProtocol::PrivSoffice )
        aName = SfxResId( STR_NONAME ).toString();
    m_pNameED->SetText( aName );

    // determine context symbol
    aURL.SetSmartProtocol( INetProtocol::File );
    aURL.SetSmartURL( aFactory);
    const OUString& rMainURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
    Image aImage = SvFileInformationManager::GetImage( aURL, true );

    if ( GetDPIScaleFactor() > 1)
    {
        BitmapEx b = aImage.GetBitmapEx();
        b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());
        aImage = Image(b);
    }

    m_pBmp->SetImage( aImage );

    // determine size and type
    OUString aSizeText( m_aUnknownSize );
    if ( aURL.GetProtocol() == INetProtocol::File ||
         aURL.isAnyKnownWebDAVScheme() )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
    m_pShowSizeFT->SetText( aSizeText );

    OUString aDescription = SvFileInformationManager::GetDescription( INetURLObject(rMainURL) );
    if ( aDescription.isEmpty() )
        aDescription = SfxResId( STR_SFX_NEWOFFICEDOC ).toString();
    m_pShowTypeFT->SetText( aDescription );

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
        m_pFileValEd->SetText( aText );
    }
    else if ( aURL.GetProtocol() != INetProtocol::PrivSoffice )
        m_pFileValEd->SetText( aURL.GetPartBeforeLastName() );

    // handle access data
    bool m_bUseUserData = rInfoItem.IsUseUserData();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_pCreateValFt->SetText( ConvertDateTime_Impl( rInfoItem.getAuthor(),
        rInfoItem.getCreationDate(), rLocaleWrapper ) );
    util::DateTime aTime( rInfoItem.getModificationDate() );
    if ( aTime.Month > 0 )
        m_pChangeValFt->SetText( ConvertDateTime_Impl(
            rInfoItem.getModifiedBy(), aTime, rLocaleWrapper ) );
    aTime = rInfoItem.getPrintDate();
    if ( aTime.Month > 0 )
        m_pPrintValFt->SetText( ConvertDateTime_Impl( rInfoItem.getPrintedBy(),
            aTime, rLocaleWrapper ) );
    const long nTime = rInfoItem.getEditingDuration();
    if ( m_bUseUserData )
    {
        const tools::Time aT( nTime/3600, (nTime%3600)/60, nTime%60 );
        m_pTimeLogValFt->SetText( rLocaleWrapper.getDuration( aT ) );
        m_pDocNoValFt->SetText( OUString::number(
            rInfoItem.getEditingCycles() ) );
    }

    bool m_bUseThumbnailSave = rInfoItem.IsUseThumbnailSave();

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
                SvNumberFormatter m_aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() );
                sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
                if ( seqValue.getLength( ) > 0 )
                {
                    OUString sValue;
                    m_aNumberFormatter.GetInputLineString( seqValue[0], nIndex, sValue );
                    m_pShowSizeFT->SetText( CreateSizeText( sValue.toInt64( ) ) );
                }
            }

            util::DateTime uDT;
            OUString emptyDate = ConvertDateTime_Impl( "", uDT, rLocaleWrapper );
            if ( aCmisProps[i].Id == "cmis:creationDate" &&
                 (m_pCreateValFt->GetText() == emptyDate ||
                  m_pCreateValFt->GetText().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                aCmisProps[i].Value >>= seqValue;
                if ( seqValue.getLength( ) > 0 )
                {
                    m_pCreateValFt->SetText( ConvertDateTime_Impl( "", seqValue[0], rLocaleWrapper ) );
                }
            }
            if ( aCmisProps[i].Id == "cmis:lastModificationDate" &&
                 (m_pChangeValFt->GetText() == emptyDate ||
                  m_pChangeValFt->GetText().isEmpty()))
            {
                Sequence< util::DateTime > seqValue;
                aCmisProps[i].Value >>= seqValue;
                if ( seqValue.getLength( ) > 0 )
                {
                    m_pChangeValFt->SetText( ConvertDateTime_Impl( "", seqValue[0], rLocaleWrapper ) );
                }
            }
        }
    }

    m_pUseUserDataCB->SetState( static_cast<TriState>(m_bUseUserData) );
    m_pUseUserDataCB->SaveValue();
    m_pUseUserDataCB->Enable( bEnableUseUserData );
    bHandleDelete = false;
    m_pDeleteBtn->Enable( bEnableUseUserData );
    m_pUseThumbnailSaveCB->SetState( static_cast<TriState>(m_bUseThumbnailSave) );
    m_pUseThumbnailSaveCB->SaveValue();
}


SfxDocumentInfoDialog::SfxDocumentInfoDialog( vcl::Window* pParent,
                                              const SfxItemSet& rItemSet )
    : SfxTabDialog(pParent, "DocumentPropertiesDialog",
        "sfx/ui/documentpropertiesdialog.ui", &rItemSet)
    , m_nDocInfoId(0)
{
    const SfxDocumentInfoItem& rInfoItem =
        static_cast<const SfxDocumentInfoItem &>(rItemSet.Get( SID_DOCINFO ));

#ifdef DBG_UTIL
    const SfxStringItem* pURLItem = rItemSet.GetItem<SfxStringItem>(SID_BASEURL, false);
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
#endif

     // Determine the Titels
    const SfxPoolItem* pItem = nullptr;
    OUString aTitle( GetText() );
    if ( SfxItemState::SET !=
         rItemSet.GetItemState( SID_EXPLORER_PROPS_START, false, &pItem ) )
    {
        // File name
        OUString aFile( rInfoItem.GetValue() );

        INetURLObject aURL;
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( aFile);
        if ( INetProtocol::PrivSoffice != aURL.GetProtocol() )
        {
            OUString aLastName( aURL.GetLastName() );
            if ( !aLastName.isEmpty() )
                aTitle += aLastName;
            else
                aTitle += aFile;
        }
        else
            aTitle += SfxResId( STR_NONAME ).toString();
    }
    else
    {
        DBG_ASSERT( dynamic_cast<const SfxStringItem *>(pItem) != nullptr,
                    "SfxDocumentInfoDialog:<SfxStringItem> expected" );
        aTitle += static_cast<const SfxStringItem*>(pItem)->GetValue();
    }
    SetText( aTitle );

    // Property Pages
    m_nDocInfoId = AddTabPage("general", SfxDocumentPage::Create, nullptr);
    AddTabPage("description", SfxDocumentDescPage::Create, nullptr);
    AddTabPage("customprops", SfxCustomPropertiesPage::Create, nullptr);
    AddTabPage("cmisprops", SfxCmisPropertiesPage::Create, nullptr);
    AddTabPage("security", SfxSecurityPage::Create, nullptr);
}


void SfxDocumentInfoDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if ( m_nDocInfoId == nId )
        static_cast<SfxDocumentPage&>(rPage).EnableUseUserData();
}

void SfxDocumentInfoDialog::AddFontTabPage()
{
    AddTabPage( FONT_PAGE_ID, SfxResId( STR_FONT_TABPAGE ).toString(), SfxDocumentFontsPage::Create, nullptr);
}

// class CustomPropertiesYesNoButton -------------------------------------

CustomPropertiesYesNoButton::CustomPropertiesYesNoButton( vcl::Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    m_aYesButton( VclPtr<RadioButton>::Create(this, ResId( RB_PROPERTY_YES, *rResId.GetResMgr() )) ),
    m_aNoButton ( VclPtr<RadioButton>::Create(this, ResId( RB_PROPERTY_NO, *rResId.GetResMgr() )) )
{
    FreeResource();
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
    SetBorderStyle( WindowBorderStyle::MONO  );
    CheckNo();
    Wallpaper aWall( Color( COL_TRANSPARENT ) );
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

class DurationDialog_Impl : public ModalDialog
{
    VclPtr<CheckBox>       m_pNegativeCB;
    VclPtr<NumericField>   m_pYearNF;
    VclPtr<NumericField>   m_pMonthNF;
    VclPtr<NumericField>   m_pDayNF;
    VclPtr<NumericField>   m_pHourNF;
    VclPtr<NumericField>   m_pMinuteNF;
    VclPtr<NumericField>   m_pSecondNF;
    VclPtr<NumericField>   m_pMSecondNF;

public:

    DurationDialog_Impl( vcl::Window* pParent, const util::Duration& rDuration );
    virtual ~DurationDialog_Impl();
    virtual void dispose() override;
    util::Duration  GetDuration() const;
};

DurationDialog_Impl::DurationDialog_Impl(vcl::Window* pParent,
    const util::Duration& rDuration)
    : ModalDialog(pParent, "EditDurationDialog",
        "sfx/ui/editdurationdialog.ui")
{
    get(m_pNegativeCB, "negative");
    get(m_pYearNF, "years");
    get(m_pMonthNF, "months");
    get(m_pDayNF, "days");
    get(m_pHourNF, "hours");
    get(m_pMinuteNF, "minutes");
    get(m_pSecondNF, "seconds");
    get(m_pMSecondNF, "milliseconds");

    m_pNegativeCB->Check(rDuration.Negative);
    m_pYearNF->SetValue(rDuration.Years);
    m_pMonthNF->SetValue(rDuration.Months);
    m_pDayNF->SetValue(rDuration.Days);
    m_pHourNF->SetValue(rDuration.Hours);
    m_pMinuteNF->SetValue(rDuration.Minutes);
    m_pSecondNF->SetValue(rDuration.Seconds);
    m_pMSecondNF->SetValue(rDuration.NanoSeconds);
}

DurationDialog_Impl::~DurationDialog_Impl()
{
    disposeOnce();
}

void DurationDialog_Impl::dispose()
{
    m_pNegativeCB.clear();
    m_pYearNF.clear();
    m_pMonthNF.clear();
    m_pDayNF.clear();
    m_pHourNF.clear();
    m_pMinuteNF.clear();
    m_pSecondNF.clear();
    m_pMSecondNF.clear();
    ModalDialog::dispose();
}

util::Duration  DurationDialog_Impl::GetDuration() const
{
    util::Duration  aRet;
    aRet.Negative = m_pNegativeCB->IsChecked();
    aRet.Years = m_pYearNF->GetValue();
    aRet.Months = m_pMonthNF->GetValue( );
    aRet.Days = m_pDayNF->GetValue(   );
    aRet.Hours  = m_pHourNF->GetValue( );
    aRet.Minutes = m_pMinuteNF->GetValue();
    aRet.Seconds = m_pSecondNF->GetValue();
    aRet.NanoSeconds = m_pMSecondNF->GetValue();
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
        Rectangle aItemRect( rHEvt.GetMousePosPixel(), aSize );
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

IMPL_LINK_NOARG_TYPED(CustomPropertiesEditButton, ClickHdl, Button*, void)
{
    VclPtrInstance< DurationDialog_Impl > pDurationDlg( this, m_pLine->m_aDurationField->GetDuration() );
    if ( RET_OK == pDurationDlg->Execute() )
        m_pLine->m_aDurationField->SetDuration( pDurationDlg->GetDuration() );
}

void CustomPropertiesYesNoButton::Resize()
{
    const long nWidth = GetSizePixel().Width();
    const long n3Width = LogicToPixel( Size( 3, 3 ), MAP_APPFONT ).Width();
    const long nNewWidth = ( nWidth / 2 ) - n3Width - 2;
    Size aSize = m_aYesButton->GetSizePixel();
    const long nDelta = aSize.Width() - nNewWidth;
    aSize.Width() = nNewWidth;
    m_aYesButton->SetSizePixel( aSize );
    Point aPos = m_aNoButton->GetPosPixel();
    aPos.X() -= nDelta;
    m_aNoButton->SetPosSizePixel( aPos, aSize );
}

// struct CustomPropertyLine ---------------------------------------------
CustomPropertyLine::CustomPropertyLine( vcl::Window* pParent ) :
    m_aNameBox      ( VclPtr<ComboBox>::Create(pParent, SfxResId( SFX_CB_PROPERTY_NAME )) ),
    m_aTypeBox      ( VclPtr<CustomPropertiesTypeBox>::Create(pParent, SfxResId( SFX_LB_PROPERTY_TYPE ), this) ),
    m_aValueEdit    ( VclPtr<CustomPropertiesEdit>::Create(pParent, WB_BORDER|WB_TABSTOP|WB_LEFT, this ) ),
    m_aDateField    ( VclPtr<CustomPropertiesDateField>::Create(pParent, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_aTimeField    ( VclPtr<CustomPropertiesTimeField>::Create(pParent, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_sDurationFormat( SfxResId( SFX_ST_DURATION_FORMAT ).toString() ),
    m_aDurationField( VclPtr<CustomPropertiesDurationField>::Create(pParent, WB_BORDER|WB_TABSTOP|WB_READONLY, this ) ),
    m_aEditButton   ( VclPtr<CustomPropertiesEditButton>::Create(pParent, WB_TABSTOP, this) ),
    m_aYesNoButton  ( VclPtr<CustomPropertiesYesNoButton>::Create(pParent, SfxResId( SFX_WIN_PROPERTY_YESNO )) ),
    m_aRemoveButton ( VclPtr<CustomPropertiesRemoveButton>::Create(pParent, 0, this) ),
    m_bIsDate       ( false ),
    m_bIsRemoved    ( false ),
    m_bTypeLostFocus( false )

{
    m_aTimeField->SetExtFormat( EXTTIMEF_24H_LONG );
    m_aDateField->SetExtDateFormat( XTDATEF_SYSTEM_SHORT_YYYY );

    m_aRemoveButton->SetModeImage(Image(SfxResId(SFX_IMG_PROPERTY_REMOVE)));
    m_aRemoveButton->SetQuickHelpText(SfxResId(STR_SFX_REMOVE_PROPERTY).toString());

    m_aEditButton->SetText(SfxResId(SFX_ST_EDIT).toString());
}

void CustomPropertyLine::SetRemoved()
{
    DBG_ASSERT( !m_bIsRemoved, "CustomPropertyLine::SetRemoved(): line already removed" );
    m_bIsRemoved = true;
    m_aNameBox->Hide();
    m_aTypeBox->Hide();
    m_aValueEdit->Hide();
    m_aDateField->Hide();
    m_aTimeField->Hide();
    m_aDurationField->Hide();
    m_aEditButton->Hide();
    m_aYesNoButton->Hide();
    m_aRemoveButton->Hide();
}

CustomPropertiesWindow::CustomPropertiesWindow(vcl::Window* pParent,
    FixedText *pHeaderAccName,
    FixedText *pHeaderAccType,
    FixedText *pHeaderAccValue) :
    Window(pParent, WB_HIDE | WB_CLIPCHILDREN | WB_TABSTOP | WB_DIALOGCONTROL),
    m_pHeaderAccName(pHeaderAccName),
    m_pHeaderAccType(pHeaderAccType),
    m_pHeaderAccValue(pHeaderAccValue),
    m_aNameBox      ( VclPtr<ComboBox>::Create( this, SfxResId( SFX_CB_PROPERTY_NAME ) ) ),
    m_aTypeBox      ( VclPtr<ListBox>::Create( this, SfxResId( SFX_LB_PROPERTY_TYPE ) ) ),
    m_aValueEdit    ( VclPtr<Edit>::Create( this, WB_BORDER|WB_TABSTOP|WB_LEFT ) ),
    m_aDateField    ( VclPtr<DateField>::Create( this, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_aTimeField    ( VclPtr<TimeField>::Create( this, WB_BORDER|WB_TABSTOP|WB_SPIN|WB_LEFT ) ),
    m_aDurationField( VclPtr<Edit>::Create( this, WB_BORDER|WB_TABSTOP|WB_READONLY ) ),
    m_aEditButton   ( VclPtr<PushButton>::Create( this, WB_TABSTOP ) ),
    m_aYesNoButton  ( VclPtr<CustomPropertiesYesNoButton>::Create( this, SfxResId( SFX_WIN_PROPERTY_YESNO )) ),
    m_aRemoveButton ( VclPtr<ImageButton>::Create( this, 0 ) ),
    m_nScrollPos (0),
    m_pCurrentLine (nullptr),
    m_aNumberFormatter( ::comphelper::getProcessComponentContext(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() )
{
    Point aPos(LogicToPixel(Point(159, 2), MAP_APPFONT));

    m_aEditButton->SetPosSizePixel(aPos,
        LogicToPixel(Size(RSC_CD_TEXTBOX_HEIGHT, RSC_CD_TEXTBOX_HEIGHT), MAP_APPFONT));

    m_aRemoveButton->SetSizePixel(LogicToPixel(Size(RSC_CD_PUSHBUTTON_HEIGHT, RSC_CD_PUSHBUTTON_HEIGHT), MAP_APPFONT));

    Size aSize(LogicToPixel(Size(61, RSC_CD_TEXTBOX_HEIGHT), MAP_APPFONT));
    m_aValueEdit->SetPosSizePixel(aPos, aSize);
    m_aDurationField->SetPosSizePixel(aPos, aSize);
    m_aDateField->SetPosSizePixel(aPos, aSize);
    m_aTimeField->SetPosSizePixel(aPos, aSize);

    m_aEditLoseFocusIdle.SetPriority( SchedulerPriority::LOWEST );
    m_aEditLoseFocusIdle.SetIdleHdl( LINK( this, CustomPropertiesWindow, EditTimeoutHdl ) );
    m_aBoxLoseFocusIdle.SetPriority( SchedulerPriority::LOWEST );
    m_aBoxLoseFocusIdle.SetIdleHdl( LINK( this, CustomPropertiesWindow, BoxTimeoutHdl ) );

    m_aNameBox->add_mnemonic_label(m_pHeaderAccName);
    m_aNameBox->SetAccessibleName(m_pHeaderAccName->GetText());
    m_aTypeBox->add_mnemonic_label(m_pHeaderAccType);
    m_aTypeBox->SetAccessibleName(m_pHeaderAccType->GetText());
    m_aValueEdit->add_mnemonic_label(m_pHeaderAccValue);
    m_aValueEdit->SetAccessibleName(m_pHeaderAccValue->GetText());

    m_aNameBox->Hide();
    m_aTypeBox->Hide();
    m_aValueEdit->Hide();
    m_aDateField->Hide();
    m_aTimeField->Hide();
    m_aDurationField->Hide();
    m_aEditButton->Hide();
    m_aYesNoButton->Hide();
    m_aRemoveButton->Hide();

    m_nLineHeight =
        ( m_aRemoveButton->GetPosPixel().Y() * 2 ) + m_aRemoveButton->GetSizePixel().Height();
}

CustomPropertiesWindow::~CustomPropertiesWindow()
{
    disposeOnce();
}

void CustomPropertiesWindow::dispose()
{
    m_aEditLoseFocusIdle.Stop();
    m_aBoxLoseFocusIdle.Stop();
    ClearAllLines();
    m_aNameBox.disposeAndClear();
    m_aTypeBox.disposeAndClear();
    m_aValueEdit.disposeAndClear();
    m_aDateField.disposeAndClear();
    m_aTimeField.disposeAndClear();
    m_aDurationField.disposeAndClear();
    m_aEditButton.disposeAndClear();
    m_aYesNoButton.disposeAndClear();
    m_aRemoveButton.disposeAndClear();
    m_pHeaderAccName.clear();
    m_pHeaderAccType.clear();
    m_pHeaderAccValue.clear();
    vcl::Window::dispose();
}

IMPL_STATIC_LINK_TYPED(
    CustomPropertiesWindow, TypeHdl, ListBox&, rListBox, void )
{
    CustomPropertiesTypeBox* pBox = static_cast<CustomPropertiesTypeBox*>(&rListBox);
    long nType = reinterpret_cast<long>( pBox->GetSelectEntryData() );
    CustomPropertyLine* pLine = pBox->GetLine();
    pLine->m_aValueEdit->Show( (CUSTOM_TYPE_TEXT == nType) || (CUSTOM_TYPE_NUMBER  == nType) );
    pLine->m_aDateField->Show( (CUSTOM_TYPE_DATE == nType) || (CUSTOM_TYPE_DATETIME  == nType) );
    pLine->m_aTimeField->Show( CUSTOM_TYPE_DATETIME  == nType );
    pLine->m_aDurationField->Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aEditButton->Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aYesNoButton->Show( CUSTOM_TYPE_BOOLEAN == nType );

    //adjust positions of date and time controls
    if ( nType == CUSTOM_TYPE_DATE )
    {
        pLine->m_bIsDate = true;
        pLine->m_aDateField->SetSizePixel( pLine->m_aValueEdit->GetSizePixel() );
    }
    else if ( nType == CUSTOM_TYPE_DATETIME)
    {
        // because m_aDateField and m_aTimeField have the same size for type "DateTime",
        // we just rely on m_aTimeField here.
        pLine->m_bIsDate = false;
        pLine->m_aDateField->SetSizePixel( pLine->m_aTimeField->GetSizePixel() );
    }
}

IMPL_LINK_TYPED( CustomPropertiesWindow, RemoveHdl, Button*, pBtn, void )
{
    CustomPropertiesRemoveButton* pButton = static_cast<CustomPropertiesRemoveButton*>(pBtn);
    CustomPropertyLine* pLine = pButton->GetLine();
    std::vector< CustomPropertyLine* >::iterator pFound =
        std::find( m_aCustomPropertiesLines.begin(), m_aCustomPropertiesLines.end(), pLine );
    if ( pFound != m_aCustomPropertiesLines.end() )
    {
        pLine = *pFound;
        pLine->SetRemoved();
        std::vector< CustomPropertyLine* >::iterator pIter = pFound + 1;
        const long nDelta = GetLineHeight();
        for ( ; pIter != m_aCustomPropertiesLines.end(); ++pIter )
        {
            pLine = *pIter;
            if ( pLine->m_bIsRemoved )
                continue;

            vcl::Window* pWindows[] = {  pLine->m_aNameBox.get(), pLine->m_aTypeBox.get(), pLine->m_aValueEdit.get(),
                                    pLine->m_aDateField.get(), pLine->m_aTimeField.get(),
                                    pLine->m_aDurationField.get(), pLine->m_aEditButton.get(),
                                    pLine->m_aYesNoButton.get(), pLine->m_aRemoveButton.get(), nullptr };
            vcl::Window** pCurrent = pWindows;
            while ( *pCurrent )
            {
                Point aPos = (*pCurrent)->GetPosPixel();
                aPos.Y() -= nDelta;
                (*pCurrent)->SetPosPixel( aPos );
                pCurrent++;
            }
        }
    }

    m_aRemovedHdl.Call(nullptr);
}

IMPL_LINK_TYPED( CustomPropertiesWindow, EditLoseFocusHdl, Control&, rControl, void )
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

IMPL_LINK_TYPED( CustomPropertiesWindow, BoxLoseFocusHdl, Control&, rControl, void )
{
    m_pCurrentLine = static_cast<CustomPropertiesTypeBox*>(&rControl)->GetLine();
    m_aBoxLoseFocusIdle.Start();
}

IMPL_LINK_NOARG_TYPED(CustomPropertiesWindow, EditTimeoutHdl, Idle *, void)
{
    ValidateLine( m_pCurrentLine, false );
}

IMPL_LINK_NOARG_TYPED(CustomPropertiesWindow, BoxTimeoutHdl, Idle *, void)
{
    ValidateLine( m_pCurrentLine, true );
}

bool CustomPropertiesWindow::IsLineValid( CustomPropertyLine* pLine ) const
{
    bool bIsValid = true;
    pLine->m_bTypeLostFocus = false;
    long nType = reinterpret_cast<long>(
                     pLine->m_aTypeBox->GetSelectEntryData() );
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
    if ( !IsLineValid( pLine ) )
    {
        if ( bIsFromTypeBox ) // LoseFocus of TypeBox
            pLine->m_bTypeLostFocus = true;
        vcl::Window* pParent = GetParent()->GetParent();
        if (ScopedVclPtrInstance<MessageDialog>::Create(pParent, SfxResId(STR_SFX_QUERY_WRONG_TYPE), VCL_MESSAGE_QUESTION, VCL_BUTTONS_OK_CANCEL)->Execute() == RET_OK)
            pLine->m_aTypeBox->SelectEntryPos( m_aTypeBox->GetEntryPos( reinterpret_cast<void*>(CUSTOM_TYPE_TEXT) ) );
        else
            pLine->m_aValueEdit->GrabFocus();
    }
}

bool CustomPropertiesWindow::InitControls( HeaderBar* pHeaderBar, const ScrollBar* pScrollBar )
{
    bool bChanged = false;

    DBG_ASSERT( pHeaderBar, "CustomPropertiesWindow::InitControls(): invalid headerbar" );
    DBG_ASSERT( pScrollBar, "CustomPropertiesWindow::InitControls(): invalid scrollbar" );

    const long nOffset = 4;
    const long nScrollBarWidth = pScrollBar->GetSizePixel().Width();
    const long nButtonWidth = m_aRemoveButton->GetSizePixel().Width() + nScrollBarWidth + nOffset;
    long nTypeWidth = m_aTypeBox->CalcMinimumSize().Width() + ( 2 * nOffset );
    long nFullWidth = pHeaderBar->GetSizePixel().Width();
    long nItemWidth = ( nFullWidth - nTypeWidth - nButtonWidth ) / 2;
    pHeaderBar->SetItemSize( HI_NAME, nItemWidth );
    pHeaderBar->SetItemSize( HI_TYPE, nTypeWidth );
    pHeaderBar->SetItemSize( HI_VALUE, nItemWidth );
    pHeaderBar->SetItemSize( HI_ACTION, nButtonWidth );

    vcl::Window* pWindows[] = { m_aNameBox.get(), m_aTypeBox.get(), m_aValueEdit.get(), m_aRemoveButton.get(), nullptr };
    vcl::Window** pCurrent = pWindows;
    sal_uInt16 nPos = 0;
    while ( *pCurrent )
    {
        Rectangle aRect = pHeaderBar->GetItemRect( pHeaderBar->GetItemId( nPos++ ) );
        Size aOrigSize = (*pCurrent)->GetSizePixel();
        Point aOrigPos = (*pCurrent)->GetPosPixel();
        Size aSize(aOrigSize);
        Point aPos(aOrigPos);
        long nWidth = aRect.GetWidth() - nOffset;
        if ( *pCurrent == m_aRemoveButton.get() )
            nWidth -= pScrollBar->GetSizePixel().Width();
        aSize.Width() = nWidth;
        aPos.X() = aRect.getX() + ( nOffset / 2 );

        if (aOrigSize != aSize || aOrigPos != aPos)
        {
            (*pCurrent)->SetPosSizePixel(aPos, aSize);
            bChanged = true;
        }

        if ( *pCurrent == m_aValueEdit.get() )
        {
            Point aDurationPos( aPos );
            m_aDurationField->SetPosPixel( aDurationPos );
            Size aDurationSize(aSize);
            aDurationSize.Width() -= (m_aEditButton->GetSizePixel().Width() + 3 );
            m_aDurationField->SetSizePixel(aDurationSize);
            aDurationPos.X() = aPos.X() - m_aEditButton->GetSizePixel().Width() + aSize.Width();
            m_aEditButton->SetPosPixel(aDurationPos);

            m_aYesNoButton->SetPosSizePixel( aPos, aSize );

            aSize.Width() /= 2;
            aSize.Width() -= 2;
            m_aDateField->SetPosSizePixel( aPos, aSize );
            aPos.X() += aSize.Width() + 4;
            m_aTimeField->SetPosSizePixel( aPos, aSize );
        }

        pCurrent++;
    }
    return bChanged;
}

sal_uInt16 CustomPropertiesWindow::GetVisibleLineCount() const
{
    sal_uInt16 nCount = 0;
    std::vector< CustomPropertyLine* >::const_iterator pIter;
    for ( pIter = m_aCustomPropertiesLines.begin();
            pIter != m_aCustomPropertiesLines.end(); ++pIter )
    {
        CustomPropertyLine* pLine = *pIter;
        if ( !pLine->m_bIsRemoved )
            nCount++;
    }
    return nCount;
}

void CustomPropertiesWindow::updateLineWidth()
{
    vcl::Window* pWindows[] = {  m_aNameBox.get(), m_aTypeBox.get(), m_aValueEdit.get(),
                            m_aDateField.get(), m_aTimeField.get(),
                            m_aDurationField.get(), m_aEditButton.get(),
                            m_aYesNoButton.get(), m_aRemoveButton.get(), nullptr };

    for (std::vector< CustomPropertyLine* >::iterator aI =
        m_aCustomPropertiesLines.begin(), aEnd = m_aCustomPropertiesLines.end();
        aI != aEnd; ++aI)
    {
        CustomPropertyLine* pNewLine = *aI;

        vcl::Window* pNewWindows[] =
            {   pNewLine->m_aNameBox.get(), pNewLine->m_aTypeBox.get(), pNewLine->m_aValueEdit.get(),
                pNewLine->m_aDateField.get(), pNewLine->m_aTimeField.get(),
                pNewLine->m_aDurationField.get(), pNewLine->m_aEditButton.get(),
                pNewLine->m_aYesNoButton.get(), pNewLine->m_aRemoveButton.get(), nullptr };

        vcl::Window** pCurrent = pWindows;
        vcl::Window** pNewCurrent = pNewWindows;
        while ( *pCurrent )
        {
            Size aSize = (*pCurrent)->GetSizePixel();
            Point aPos = (*pCurrent)->GetPosPixel();
            aPos.Y() = (*pNewCurrent)->GetPosPixel().Y();
            (*pNewCurrent)->SetPosSizePixel( aPos, aSize );
            pCurrent++;
            pNewCurrent++;
        }

        // if we have type "Date", we use the full width, not only the half
        if (pNewLine->m_bIsDate)
            pNewLine->m_aDateField->SetSizePixel( pNewLine->m_aValueEdit->GetSizePixel() );
    }
}

void CustomPropertiesWindow::AddLine( const OUString& sName, Any& rAny )
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

    sal_Int32 nPos = GetVisibleLineCount() * GetLineHeight();
    m_aCustomPropertiesLines.push_back( pNewLine );
    vcl::Window* pWindows[] = {  m_aNameBox.get(), m_aTypeBox.get(), m_aValueEdit.get(),
                            m_aDateField.get(), m_aTimeField.get(),
                            m_aDurationField.get(), m_aEditButton.get(),
                            m_aYesNoButton.get(), m_aRemoveButton.get(), nullptr };
    vcl::Window* pNewWindows[] =
        {   pNewLine->m_aNameBox.get(), pNewLine->m_aTypeBox.get(), pNewLine->m_aValueEdit.get(),
            pNewLine->m_aDateField.get(), pNewLine->m_aTimeField.get(),
            pNewLine->m_aDurationField.get(), pNewLine->m_aEditButton.get(),
            pNewLine->m_aYesNoButton.get(), pNewLine->m_aRemoveButton.get(), nullptr };
    vcl::Window** pCurrent = pWindows;
    vcl::Window** pNewCurrent = pNewWindows;
    while ( *pCurrent )
    {
        Size aSize = (*pCurrent)->GetSizePixel();
        Point aPos = (*pCurrent)->GetPosPixel();
        aPos.Y() += nPos;
        aPos.Y() += m_nScrollPos;
        (*pNewCurrent)->SetPosSizePixel( aPos, aSize );
        (*pNewCurrent)->Show();
        pCurrent++;
        pNewCurrent++;
    }

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
    pNewLine->m_aNameBox->SetText( sName );
    sal_IntPtr nType = CUSTOM_TYPE_UNKNOWN;
    OUString sValue;

    if ( rAny >>= nTmpValue )
    {
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        m_aNumberFormatter.GetInputLineString( nTmpValue, nIndex, sValue );
        pNewLine->m_aValueEdit->SetText( sValue );
        nType = CUSTOM_TYPE_NUMBER;
    }
    else if ( rAny >>= bTmpValue )
    {
        sValue = ( bTmpValue ? rLocaleWrapper.getTrueWord() : rLocaleWrapper.getFalseWord() );
        nType = CUSTOM_TYPE_BOOLEAN;
    }
    else if ( rAny >>= sTmpValue )
    {
        pNewLine->m_aValueEdit->SetText( sTmpValue );
        nType = CUSTOM_TYPE_TEXT;
    }
    else if ( rAny >>= aTmpDate )
    {
        pNewLine->m_aDateField->SetDate( Date( aTmpDate ) );
        nType = CUSTOM_TYPE_DATE;
    }
    else if ( rAny >>= aTmpDateTime )
    {
        pNewLine->m_aDateField->SetDate( Date( aTmpDateTime ) );
        pNewLine->m_aTimeField->SetTime( tools::Time( aTmpDateTime ) );
        pNewLine->m_aTimeField->m_isUTC = aTmpDateTime.IsUTC;
        nType = CUSTOM_TYPE_DATETIME;
    }
    else if ( rAny >>= aTmpDateTZ )
    {
        pNewLine->m_aDateField->SetDate( Date( aTmpDateTZ.DateInTZ.Day,
                    aTmpDateTZ.DateInTZ.Month, aTmpDateTZ.DateInTZ.Year ) );
        pNewLine->m_aDateField->m_TZ = aTmpDateTZ.Timezone;
        nType = CUSTOM_TYPE_DATE;
    }
    else if ( rAny >>= aTmpDateTimeTZ )
    {
        util::DateTime const& rDT(aTmpDateTimeTZ.DateTimeInTZ);
        pNewLine->m_aDateField->SetDate( Date( rDT ) );
        pNewLine->m_aTimeField->SetTime( tools::Time( rDT ) );
        pNewLine->m_aTimeField->m_isUTC = rDT.IsUTC;
        pNewLine->m_aDateField->m_TZ = aTmpDateTimeTZ.Timezone;
        nType = CUSTOM_TYPE_DATETIME;
    }
    else if ( rAny >>= aTmpDuration )
    {
        nType = CUSTOM_TYPE_DURATION;
        pNewLine->m_aDurationField->SetDuration( aTmpDuration );
    }

    if ( nType != CUSTOM_TYPE_UNKNOWN )
    {
        if ( CUSTOM_TYPE_BOOLEAN == nType )
        {
            if ( bTmpValue )
                pNewLine->m_aYesNoButton->CheckYes();
            else
                pNewLine->m_aYesNoButton->CheckNo();
        }
        pNewLine->m_aTypeBox->SelectEntryPos( m_aTypeBox->GetEntryPos( reinterpret_cast<void*>(nType) ) );
    }

    TypeHdl( nullptr, *pNewLine->m_aTypeBox.get() );
    pNewLine->m_aNameBox->GrabFocus();
}

bool CustomPropertiesWindow::AreAllLinesValid() const
{
    bool bRet = true;
    std::vector< CustomPropertyLine* >::const_iterator pIter;
    for ( pIter = m_aCustomPropertiesLines.begin();
            pIter != m_aCustomPropertiesLines.end(); ++pIter )
    {
        CustomPropertyLine* pLine = *pIter;
        if ( !IsLineValid( pLine ) )
        {
            bRet = false;
            break;
        }
    }

    return bRet;
}

void CustomPropertiesWindow::ClearAllLines()
{
    std::vector< CustomPropertyLine* >::iterator pIter;
    for ( pIter = m_aCustomPropertiesLines.begin();
          pIter != m_aCustomPropertiesLines.end(); ++pIter )
    {
        CustomPropertyLine* pLine = *pIter;
        pLine->SetRemoved();
        delete pLine;
    }
    m_aCustomPropertiesLines.clear();
    m_nScrollPos = 0;
}

void CustomPropertiesWindow::DoScroll( sal_Int32 nNewPos )
{
    m_nScrollPos += nNewPos;
    std::vector< CustomPropertyLine* >::iterator pIter;
    for ( pIter = m_aCustomPropertiesLines.begin();
            pIter != m_aCustomPropertiesLines.end(); ++pIter )
    {
        CustomPropertyLine* pLine = *pIter;
        if ( pLine->m_bIsRemoved )
            continue;

        vcl::Window* pWindows[] = {  pLine->m_aNameBox.get(), pLine->m_aTypeBox.get(), pLine->m_aValueEdit.get(), pLine->m_aDateField.get(), pLine->m_aTimeField.get(),
                                pLine->m_aDurationField.get(), pLine->m_aEditButton.get(), pLine->m_aYesNoButton.get(), pLine->m_aRemoveButton.get(), nullptr };
        vcl::Window** pCurrent = pWindows;
        while ( *pCurrent )
        {
            Point aPos = (*pCurrent)->GetPosPixel();
            aPos.Y() += nNewPos;
            (*pCurrent)->SetPosPixel( aPos );
            pCurrent++;
        }
    }
}

Sequence< beans::PropertyValue > CustomPropertiesWindow::GetCustomProperties() const
{
    Sequence< beans::PropertyValue > aPropertiesSeq( m_aCustomPropertiesLines.size() );
    sal_Int32 i = 0;
    std::vector< CustomPropertyLine* >::const_iterator pIter;
    for ( pIter = m_aCustomPropertiesLines.begin();
            pIter != m_aCustomPropertiesLines.end(); ++pIter, ++i )
    {
        CustomPropertyLine* pLine = *pIter;
        if ( pLine->m_bIsRemoved )
            continue;

        OUString sPropertyName = pLine->m_aNameBox->GetText();
        if ( !sPropertyName.isEmpty() )
        {
            aPropertiesSeq[i].Name = sPropertyName;
            long nType = reinterpret_cast<long>(
                            pLine->m_aTypeBox->GetSelectEntryData() );
            if ( CUSTOM_TYPE_NUMBER == nType )
            {
                double nValue = 0;
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( pLine->m_aValueEdit->GetText(), nIndex, nValue );
                if ( bIsNum )
                    aPropertiesSeq[i].Value <<= makeAny( nValue );
            }
            else if ( CUSTOM_TYPE_BOOLEAN == nType )
            {
                bool bValue = pLine->m_aYesNoButton->IsYesChecked();
                aPropertiesSeq[i].Value <<= makeAny( bValue );
            }
            else if ( CUSTOM_TYPE_DATETIME == nType )
            {
                Date aTmpDate = pLine->m_aDateField->GetDate();
                tools::Time aTmpTime = pLine->m_aTimeField->GetTime();
                util::DateTime const aDateTime(aTmpTime.GetNanoSec(),
                    aTmpTime.GetSec(), aTmpTime.GetMin(), aTmpTime.GetHour(),
                    aTmpDate.GetDay(), aTmpDate.GetMonth(), aTmpDate.GetYear(),
                    pLine->m_aTimeField->m_isUTC);
                if (pLine->m_aDateField->m_TZ.is_initialized())
                {
                    aPropertiesSeq[i].Value <<= util::DateTimeWithTimezone(
                            aDateTime, pLine->m_aDateField->m_TZ.get());
                }
                else
                {
                    aPropertiesSeq[i].Value <<= aDateTime;
                }
            }
            else if ( CUSTOM_TYPE_DATE == nType )
            {
                Date aTmpDate = pLine->m_aDateField->GetDate();
                util::Date const aDate(aTmpDate.GetDay(), aTmpDate.GetMonth(),
                        aTmpDate.GetYear());
                if (pLine->m_aDateField->m_TZ.is_initialized())
                {
                    aPropertiesSeq[i].Value <<= util::DateWithTimezone(
                            aDate, pLine->m_aDateField->m_TZ.get());
                }
                else
                {
                    aPropertiesSeq[i].Value <<= aDate;
                }
            }
            else if ( CUSTOM_TYPE_DURATION == nType )
            {
                aPropertiesSeq[i].Value <<= pLine->m_aDurationField->GetDuration();
            }
            else
            {
                OUString sValue( pLine->m_aValueEdit->GetText() );
                aPropertiesSeq[i].Value <<= makeAny( sValue );
            }
        }
    }

    return aPropertiesSeq;
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

    const HeaderBarItemBits nHeadBits = HeaderBarItemBits::VCENTER | HeaderBarItemBits::FIXED | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::LEFT;

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
    Window::Resize();

    if (!m_pVBox)
        return;

    m_pVBox->SetSizePixel(GetSizePixel());

    bool bWidgetsResized = m_pPropertiesWin->InitControls( m_pHeaderBar, m_pVertScroll );
    sal_Int32 nScrollOffset = m_pPropertiesWin->GetLineHeight();
    sal_Int32 nVisibleEntries = m_pPropertiesWin->GetSizePixel().Height() / nScrollOffset;
    m_pVertScroll->SetPageSize( nVisibleEntries - 1 );
    m_pVertScroll->SetVisibleSize( nVisibleEntries );
    if (bWidgetsResized)
    {
        m_pPropertiesWin->updateLineWidth();
    }
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

IMPL_LINK_TYPED( CustomPropertiesControl, ScrollHdl, ScrollBar*, pScrollBar, void )
{
    sal_Int32 nOffset = m_pPropertiesWin->GetLineHeight();
    nOffset *= ( m_nThumbPos - pScrollBar->GetThumbPos() );
    m_nThumbPos = pScrollBar->GetThumbPos();
    m_pPropertiesWin->DoScroll( nOffset );
}

IMPL_LINK_NOARG_TYPED(CustomPropertiesControl, RemovedHdl, void*, void)
{
    long nLineCount = m_pPropertiesWin->GetVisibleLineCount();
    m_pVertScroll->SetRangeMax(nLineCount + 1);
    if ( m_pPropertiesWin->GetOutputSizePixel().Height() < nLineCount * m_pPropertiesWin->GetLineHeight() )
        m_pVertScroll->DoScrollAction ( SCROLL_LINEUP );
}

void CustomPropertiesControl::AddLine( const OUString& sName, Any& rAny, bool bInteractive )
{
    m_pPropertiesWin->AddLine( sName, rAny );
    long nLineCount = m_pPropertiesWin->GetVisibleLineCount();
    m_pVertScroll->SetRangeMax(nLineCount + 1);
    if ( bInteractive && m_pPropertiesWin->GetOutputSizePixel().Height() < nLineCount * m_pPropertiesWin->GetLineHeight() )
        m_pVertScroll->DoScroll(nLineCount + 1);
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

IMPL_LINK_NOARG_TYPED(SfxCustomPropertiesPage, AddHdl, Button*, void)
{
    Any aAny;
    m_pPropertiesCtrl->AddLine( OUString(), aAny, true );
}

bool SfxCustomPropertiesPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    const SfxPoolItem* pItem = nullptr;
    SfxDocumentInfoItem* pInfo = nullptr;
    bool bMustDelete = false;

    if ( GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        if ( SfxItemState::SET !=
                GetTabDialog()->GetExampleSet()->GetItemState( SID_DOCINFO, true, &pItem ) )
            pInfo = const_cast<SfxDocumentInfoItem*>(&static_cast<const SfxDocumentInfoItem& >(rSet->Get( SID_DOCINFO )));
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

    bModified = true; //!!!

    if (pInfo)
    {
        if ( bModified )
            rSet->Put( *pInfo );
        if ( bMustDelete )
            delete pInfo;
    }
    return bModified;
}

void SfxCustomPropertiesPage::Reset( const SfxItemSet* rItemSet )
{
    m_pPropertiesCtrl->ClearAllLines();
    const SfxDocumentInfoItem& rInfoItem = static_cast<const SfxDocumentInfoItem &>(rItemSet->Get(SID_DOCINFO));
    std::vector< CustomProperty* > aCustomProps = rInfoItem.GetCustomProperties();
    for ( size_t i = 0; i < aCustomProps.size(); i++ )
    {
        m_pPropertiesCtrl->AddLine( aCustomProps[i]->m_sName, aCustomProps[i]->m_aValue, false );
    }
}

SfxTabPage::sfxpg SfxCustomPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    sfxpg nRet = LEAVE_PAGE;
    if ( !m_pPropertiesCtrl->AreAllLinesValid() )
        nRet = KEEP_PAGE;
    return nRet;
}

VclPtr<SfxTabPage> SfxCustomPropertiesPage::Create( vcl::Window* pParent, const SfxItemSet* rItemSet )
{
    return VclPtr<SfxCustomPropertiesPage>::Create( pParent, *rItemSet );
}

CmisValue::CmisValue( vcl::Window* pParent, const OUString& aStr )
{
    m_pUIBuilder = new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui");
    get( m_aValueEdit, "value");
    m_aValueEdit->Show();
    m_aValueEdit->SetText( aStr );
}

CmisDateTime::CmisDateTime( vcl::Window* pParent, const util::DateTime& aDateTime )
{
    m_pUIBuilder = new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui");
    get( m_aDateField, "date");
    get( m_aTimeField, "time");
    m_aDateField->Show();
    m_aTimeField->Show();
    m_aDateField->SetDate( Date( aDateTime ) );
    m_aTimeField->SetTime( tools::Time( aDateTime ) );
}

CmisYesNo::CmisYesNo( vcl::Window* pParent, bool bValue )
{
    m_pUIBuilder = new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui");
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
    , m_nNumValue(1)
{
    m_pUIBuilder = new VclBuilder( pParent, getUIRootDir(), "sfx/ui/cmisline.ui");
    get( m_pFrame, "CmisFrame" );
    get( m_aName, "name" );
    get( m_aType, "type" );
    m_pFrame->Enable();
}

CmisPropertyLine::~CmisPropertyLine( )
{
    std::vector< CmisValue* >::iterator pIter;
    for ( pIter = m_aValues.begin();
          pIter != m_aValues.end(); ++pIter )
    {
        CmisValue* pValue = *pIter;
        delete pValue;
    }
    m_aValues.clear();

    std::vector< CmisYesNo* >::iterator pIterYesNo;
    for ( pIterYesNo = m_aYesNos.begin();
          pIterYesNo != m_aYesNos.end(); ++pIterYesNo )
    {
        CmisYesNo* pYesNo = *pIterYesNo;
        delete pYesNo;
    }
    m_aYesNos.clear();

    std::vector< CmisDateTime* >::iterator pIterDateTime;
    for ( pIterDateTime = m_aDateTimes.begin();
          pIterDateTime != m_aDateTimes.end(); ++pIterDateTime )
    {
        CmisDateTime* pDateTime = *pIterDateTime;
        delete pDateTime;
    }
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
    std::vector< CmisPropertyLine* >::iterator pIter;
    for ( pIter = m_aCmisPropertiesLines.begin();
          pIter != m_aCmisPropertiesLines.end(); ++pIter )
    {
        CmisPropertyLine* pLine = *pIter;
        delete pLine;
    }
    m_aCmisPropertiesLines.clear();
}

sal_uInt16 CmisPropertiesWindow::GetLineCount() const
{
    sal_uInt16 nCount = 0;
    std::vector< CmisPropertyLine* >::const_iterator pIter;
    for ( pIter = m_aCmisPropertiesLines.begin();
          pIter != m_aCmisPropertiesLines.end(); ++pIter )
        nCount += ( (*pIter)->m_nNumValue + 1 );
    return nCount;
}

void CmisPropertiesWindow::AddLine( const OUString& sId, const OUString& sName,
                                    const OUString& sType, const bool bUpdatable,
                                    const bool bRequired, const bool bMultiValued,
                                    const bool bOpenChoice, Any& /*aChoices*/, Any& rAny )
{
    CmisPropertyLine* pNewLine = new CmisPropertyLine( m_pBox );

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
        sal_Int32 m_nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < m_nNumValue; ++i )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( seqValue[i], nIndex, sValue );
            CmisValue* pValue = new CmisValue( m_pBox, sValue );
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( pValue );
        }
    }
    else if ( sType == CMIS_TYPE_DECIMAL )
    {
        Sequence< double > seqValue;
        rAny >>= seqValue;
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        sal_Int32 m_nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < m_nNumValue; ++i )
        {
            OUString sValue;
            m_aNumberFormatter.GetInputLineString( seqValue[i], nIndex, sValue );
            CmisValue* pValue = new CmisValue( m_pBox, sValue );
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( pValue );
        }

    }
    else if ( sType == CMIS_TYPE_BOOL )
    {
        Sequence<sal_Bool> seqValue;
        rAny >>= seqValue;
        sal_Int32 m_nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < m_nNumValue; ++i )
        {
            CmisYesNo* pYesNo = new CmisYesNo( m_pBox, seqValue[i] );
            pYesNo->m_aYesButton->Enable( bUpdatable );
            pYesNo->m_aNoButton->Enable( bUpdatable );
            pNewLine->m_aYesNos.push_back( pYesNo );
        }
    }
    else if ( sType == CMIS_TYPE_STRING )
    {
        Sequence< OUString > seqValue;
        rAny >>= seqValue;
        sal_Int32 m_nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < m_nNumValue; ++i )
        {
            CmisValue* pValue = new CmisValue( m_pBox, seqValue[i] );
            pValue->m_aValueEdit->SetReadOnly( !bUpdatable );
            pNewLine->m_aValues.push_back( pValue );
        }
    }
    else if ( sType == CMIS_TYPE_DATETIME )
    {
        Sequence< util::DateTime > seqValue;
        rAny >>= seqValue;
        sal_Int32 m_nNumValue = seqValue.getLength( );
        for ( sal_Int32 i = 0; i < m_nNumValue; ++i )
        {
            CmisDateTime* pDateTime = new CmisDateTime( m_pBox, seqValue[i]);
            pDateTime->m_aDateField->SetReadOnly( !bUpdatable );
            pDateTime->m_aTimeField->SetReadOnly( !bUpdatable );
            pNewLine->m_aDateTimes.push_back( pDateTime );
        }

    }
    pNewLine->m_aName->SetText( sName );
    pNewLine->m_aName->Show();
    pNewLine->m_aType->SetText( sType );
    pNewLine->m_aType->Show();

    m_aCmisPropertiesLines.push_back( pNewLine );
}

void CmisPropertiesWindow::DoScroll( sal_Int32 nNewPos )
{
    m_pBox->SetPosPixel(Point(0, nNewPos));
}

Sequence< document::CmisProperty > CmisPropertiesWindow::GetCmisProperties() const
{
    Sequence< document::CmisProperty > aPropertiesSeq( m_aCmisPropertiesLines.size() );
    sal_Int32 i = 0;
    std::vector< CmisPropertyLine* >::const_iterator pIter;
    for ( pIter = m_aCmisPropertiesLines.begin();
            pIter != m_aCmisPropertiesLines.end(); ++pIter, ++i )
    {
        CmisPropertyLine* pLine = *pIter;

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
                for ( std::vector< CmisValue*>::const_iterator it = pLine->m_aValues.begin();
                    it != pLine->m_aValues.end(); ++it, ++k)
                {
                    double dValue = 0.0;
                    OUString sValue( (*it)->m_aValueEdit->GetText() );
                    bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( sValue, nIndex, dValue );
                    if ( bIsNum )
                        seqValue[k] = dValue;
                }
                aPropertiesSeq[i].Value <<= makeAny( seqValue );
            }
            else if ( CMIS_TYPE_INTEGER == sType )
            {
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                Sequence< sal_Int64 > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( std::vector< CmisValue*>::const_iterator it = pLine->m_aValues.begin();
                    it != pLine->m_aValues.end(); ++it, ++k)
                {
                    double dValue = 0;
                    OUString sValue( (*it)->m_aValueEdit->GetText() );
                    bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( sValue, nIndex, dValue );
                    if ( bIsNum )
                        seqValue[k] = (sal_Int64) dValue;
                }
                aPropertiesSeq[i].Value <<= makeAny( seqValue );
            }
            else if ( CMIS_TYPE_BOOL == sType )
            {
                Sequence<sal_Bool> seqValue( pLine->m_aYesNos.size( ) );
                sal_Int32 k = 0;
                for ( std::vector< CmisYesNo*>::const_iterator it = pLine->m_aYesNos.begin();
                    it != pLine->m_aYesNos.end(); ++it, ++k)
                {
                    bool bValue = (*it)->m_aYesButton->IsChecked();
                    seqValue[k] = bValue;
                }
                aPropertiesSeq[i].Value <<= makeAny( seqValue );

            }
            else if ( CMIS_TYPE_DATETIME == sType )
            {
                Sequence< util::DateTime > seqValue( pLine->m_aDateTimes.size( ) );
                sal_Int32 k = 0;
                for ( std::vector< CmisDateTime*>::const_iterator it = pLine->m_aDateTimes.begin();
                    it != pLine->m_aDateTimes.end(); ++it, ++k)
                {
                    Date aTmpDate = (*it)->m_aDateField->GetDate();
                    tools::Time aTmpTime = (*it)->m_aTimeField->GetTime();
                    util::DateTime aDateTime( aTmpTime.GetNanoSec(), aTmpTime.GetSec(),
                                              aTmpTime.GetMin(), aTmpTime.GetHour(),
                                              aTmpDate.GetDay(), aTmpDate.GetMonth(),
                                              aTmpDate.GetYear(), true );
                    seqValue[k] = aDateTime;
                }
                aPropertiesSeq[i].Value <<= makeAny( seqValue );
            }
            else
            {
                Sequence< OUString > seqValue( pLine->m_aValues.size( ) );
                sal_Int32 k = 0;
                for ( std::vector< CmisValue*>::const_iterator it = pLine->m_aValues.begin();
                    it != pLine->m_aValues.end(); ++it, ++k)
                {
                    OUString sValue( (*it)->m_aValueEdit->GetText() );
                    seqValue[k] = sValue;
                }
                aPropertiesSeq[i].Value <<= makeAny( seqValue );
            }
        }
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

IMPL_LINK_TYPED( CmisPropertiesControl, ScrollHdl, ScrollBar*, pScrollBar, void )
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
                                     const bool bOpenChoice, Any& aChoices, Any& rAny
                                     )
{
    m_rVertScroll.SetRangeMax( m_pPropertiesWin.GetLineCount() + 1 );
    m_rVertScroll.DoScroll( m_pPropertiesWin.GetLineCount() + 1 );
    m_pPropertiesWin.AddLine( sId, sName, sType, bUpdatable, bRequired, bMultiValued,
                               bOpenChoice, aChoices, rAny );
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

    if ( GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        if ( SfxItemState::SET !=
                GetTabDialog()->GetExampleSet()->GetItemState( SID_DOCINFO, true, &pItem ) )
            pInfo = const_cast<SfxDocumentInfoItem*>(&static_cast<const SfxDocumentInfoItem& >(rSet->Get( SID_DOCINFO )));
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
        Sequence< document::CmisProperty> aModifiedProps( modifiedNum );
        sal_Int32 nCount = 0;
        for( std::vector< document::CmisProperty>::const_iterator pIter = changedProps.begin();
            pIter != changedProps.end( ); ++pIter )
                aModifiedProps[ nCount++ ] = *pIter;
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
    const SfxDocumentInfoItem& rInfoItem = static_cast<const SfxDocumentInfoItem& >(rItemSet->Get(SID_DOCINFO));
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

SfxTabPage::sfxpg SfxCmisPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    return LEAVE_PAGE;
}

VclPtr<SfxTabPage> SfxCmisPropertiesPage::Create( vcl::Window* pParent, const SfxItemSet* rItemSet )
{
    return VclPtr<SfxCmisPropertiesPage>::Create( pParent, *rItemSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
