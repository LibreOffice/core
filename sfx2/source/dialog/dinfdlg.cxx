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
#include <vcl/msgbox.hxx>
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
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <vcl/timer.hxx>
#include "sfx2/dinfdlg.hxx"
#include "sfx2/securitypage.hxx"
#include "sfxresid.hxx"
#include "dinfedt.hxx"
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include "helper.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>

#include <sfx2/sfx.hrc>
#include "dinfdlg.hrc"
#include "../appl/app.hrc"
#include "sfxlocal.hrc"
#include <dialog.hrc>
#include <vcl/help.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

struct CustomProperty
{
    ::rtl::OUString             m_sName;
    com::sun::star::uno::Any    m_aValue;

    CustomProperty( const ::rtl::OUString& sName,
            const com::sun::star::uno::Any& rValue ) :
        m_sName( sName ), m_aValue( rValue ) {}

    inline bool operator==( const CustomProperty& rProp )
    { return m_sName.equals( rProp.m_sName ) && m_aValue == rProp.m_aValue; }
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
        && i_rLeft.HundredthSeconds == i_rRight.HundredthSeconds;
}

// STATIC DATA -----------------------------------------------------------
TYPEINIT1_AUTOFACTORY(SfxDocumentInfoItem, SfxStringItem);

const sal_uInt16 HI_NAME = 1;
const sal_uInt16 HI_TYPE = 2;
const sal_uInt16 HI_VALUE = 3;
const sal_uInt16 HI_ACTION = 4;

static const char DOCUMENT_SIGNATURE_MENU_CMD[] = "Signature";

//------------------------------------------------------------------------
String CreateSizeText( sal_uIntPtr nSize, sal_Bool bExtraBytes = sal_True, sal_Bool bSmartExtraBytes = sal_False );
String CreateSizeText( sal_uIntPtr nSize, sal_Bool bExtraBytes, sal_Bool bSmartExtraBytes )
{
    String aUnitStr = rtl::OUString(' ');
    aUnitStr += SfxResId(STR_BYTES).toString();
    sal_uIntPtr nSize1 = nSize;
    sal_uIntPtr nSize2 = nSize1;
    sal_uIntPtr nMega = 1024 * 1024;
    sal_uIntPtr nGiga = nMega * 1024;
    double fSize = nSize;
    int nDec = 0;
    sal_Bool bGB = sal_False;

    if ( nSize1 >= 10000 && nSize1 < nMega )
    {
        nSize1 /= 1024;
        aUnitStr = ' ';
        aUnitStr += SfxResId(STR_KB).toString();
        fSize /= 1024;
        nDec = 0;
    }
    else if ( nSize1 >= nMega && nSize1 < nGiga )
    {
        nSize1 /= nMega;
        aUnitStr = ' ';
        aUnitStr += SfxResId(STR_MB).toString();
        fSize /= nMega;
        nDec = 2;
    }
    else if ( nSize1 >= nGiga )
    {
        nSize1 /= nGiga;
        aUnitStr = ' ';
        aUnitStr += SfxResId(STR_GB).toString();
        bGB = sal_True;
        fSize /= nGiga;
        nDec = 3;
    }
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    String aSizeStr( rLocaleWrapper.getNum( nSize1, 0 ) );
    aSizeStr += aUnitStr;
    if ( bExtraBytes && ( nSize1 < nSize2 ) )
    {
        aSizeStr = ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                rLocaleWrapper.getNumDecimalSep()[0] );
        aSizeStr += aUnitStr;

        aSizeStr += DEFINE_CONST_UNICODE(" (");
        aSizeStr += rLocaleWrapper.getNum( nSize2, 0 );
        aSizeStr += ' ';
        aSizeStr += SfxResId(STR_BYTES).toString();
        aSizeStr += ')';
    }
    else if ( bGB && bSmartExtraBytes )
    {
        nSize1 = nSize / nMega;
        aSizeStr = DEFINE_CONST_UNICODE(" (");
        aSizeStr += rLocaleWrapper.getNum( nSize1, 0 );
        aSizeStr += aUnitStr;
        aSizeStr += ')';
    }
    return aSizeStr;
}

String ConvertDateTime_Impl( const String& rName,
    const util::DateTime& uDT, const LocaleDataWrapper& rWrapper )
{
    Date aD(uDT.Day, uDT.Month, uDT.Year);
    Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     String aStr( rWrapper.getDate( aD ) );
     aStr += pDelim;
     aStr += rWrapper.getTime( aT, sal_True, sal_False );
     rtl::OUString aAuthor = comphelper::string::stripStart(rName, ' ');
     if (!aAuthor.isEmpty())
     {
        aStr += pDelim;
        aStr += aAuthor;
     }
     return aStr;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem()
    : SfxStringItem()
    , m_AutoloadDelay(0)
    , m_AutoloadURL()
    , m_isAutoloadEnabled(sal_False)
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
    , m_bHasTemplate( sal_True )
    , m_bDeleteUserData( sal_False )
    , m_bUseUserData( sal_True )
{
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem( const String& rFile,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        sal_Bool bIs )
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
    , m_bHasTemplate( sal_True )
    , m_bDeleteUserData( sal_False )
    , m_bUseUserData( bIs )
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
                if (!(pProps[i].Attributes &
                        ::com::sun::star::beans::PropertyAttribute::REMOVABLE))
                {
                    DBG_ASSERT(false, "non-removable user-defined property?");
                    continue;
                }

                uno::Any aValue = xSet->getPropertyValue(pProps[i].Name);
                CustomProperty* pProp = new CustomProperty( pProps[i].Name, aValue );
                m_aCustomProperties.push_back( pProp );
            }
        }
    }
    catch ( Exception& ) {}
}

//------------------------------------------------------------------------

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
{
    for ( sal_uInt32 i = 0; i < rItem.m_aCustomProperties.size(); i++ )
    {
        CustomProperty* pProp = new CustomProperty( rItem.m_aCustomProperties[i]->m_sName,
                                                    rItem.m_aCustomProperties[i]->m_aValue );
        m_aCustomProperties.push_back( pProp );
    }
}

//------------------------------------------------------------------------
SfxDocumentInfoItem::~SfxDocumentInfoItem()
{
    ClearCustomProperties();
}

//------------------------------------------------------------------------
SfxPoolItem* SfxDocumentInfoItem::Clone( SfxItemPool * ) const
{
    return new SfxDocumentInfoItem( *this );
}

//------------------------------------------------------------------------
int SfxDocumentInfoItem::operator==( const SfxPoolItem& rItem) const
{
    if (!(rItem.Type() == Type() && SfxStringItem::operator==(rItem)))
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
            rInfoItem.m_aCustomProperties.begin());
}

//------------------------------------------------------------------------
void SfxDocumentInfoItem::resetUserData(const ::rtl::OUString & i_rAuthor)
{
    setAuthor(i_rAuthor);
    DateTime now( DateTime::SYSTEM );
    setCreationDate( util::DateTime(
        now.Get100Sec(), now.GetSec(), now.GetMin(), now.GetHour(),
        now.GetDay(), now.GetMonth(), now.GetYear() ) );
    setModifiedBy(::rtl::OUString());
    setPrintedBy(::rtl::OUString());
    setModificationDate(util::DateTime());
    setPrintDate(util::DateTime());
    setEditingDuration(0);
    setEditingCycles(1);
}

//------------------------------------------------------------------------
void SfxDocumentInfoItem::UpdateDocumentInfo(
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        bool i_bDoNotUpdateUserDefined) const
{
    if (isAutoloadEnabled()) {
        i_xDocProps->setAutoloadSecs(getAutoloadDelay());
        i_xDocProps->setAutoloadURL(getAutoloadURL());
    } else {
        i_xDocProps->setAutoloadSecs(0);
        i_xDocProps->setAutoloadURL(::rtl::OUString());
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
            if ((pProps[j].Attributes &
                    ::com::sun::star::beans::PropertyAttribute::REMOVABLE))
            {
                xContainer->removeProperty( pProps[j].Name );
            }
        }

        for ( sal_uInt32 k = 0; k < m_aCustomProperties.size(); ++k )
        {
            try
            {
                xContainer->addProperty( m_aCustomProperties[k]->m_sName,
                    beans::PropertyAttribute::REMOVABLE, m_aCustomProperties[k]->m_aValue );
            }
            catch ( Exception& )
            {
                SAL_WARN( "sfx2.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while adding custom properties" );
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "sfx2.dialog", "SfxDocumentInfoItem::updateDocumentInfo(): exception while removing custom properties" );
    }
}

//------------------------------------------------------------------------
sal_Bool SfxDocumentInfoItem::IsDeleteUserData() const
{
    return m_bDeleteUserData;
}

void SfxDocumentInfoItem::SetDeleteUserData( sal_Bool bSet )
{
    m_bDeleteUserData = bSet;
}

sal_Bool SfxDocumentInfoItem::IsUseUserData() const
{
    return m_bUseUserData;
}

void SfxDocumentInfoItem::SetUseUserData( sal_Bool bSet )
{
    m_bUseUserData = bSet;
}

std::vector< CustomProperty* > SfxDocumentInfoItem::GetCustomProperties() const
{
    std::vector< CustomProperty* > aRet;
    for ( sal_uInt32 i = 0; i < m_aCustomProperties.size(); i++ )
    {
        CustomProperty* pProp = new CustomProperty( m_aCustomProperties[i]->m_sName,
                                                    m_aCustomProperties[i]->m_aValue );
        aRet.push_back( pProp );
    }

    return aRet;
}

void SfxDocumentInfoItem::ClearCustomProperties()
{
    for ( sal_uInt32 i = 0; i < m_aCustomProperties.size(); i++ )
        delete m_aCustomProperties[i];
    m_aCustomProperties.clear();
}

void SfxDocumentInfoItem::AddCustomProperty( const ::rtl::OUString& sName, const Any& rValue )
{
    CustomProperty* pProp = new CustomProperty( sName, rValue );
    m_aCustomProperties.push_back( pProp );
}

bool SfxDocumentInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    String aValue;
    sal_Int32 nValue = 0;
    sal_Bool bValue = sal_False;
    sal_Bool bIsInt = sal_False;
    sal_Bool bIsString = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bValue = IsUseUserData();
            break;
        case MID_DOCINFO_DELETEUSERDATA:
            bValue = IsDeleteUserData();
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            bValue = isAutoloadEnabled();
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bIsInt = sal_True;
            nValue = getAutoloadDelay();
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bIsString = sal_True;
            aValue = getAutoloadURL();
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bIsString = sal_True;
            aValue = getDefaultTarget();
            break;
        case MID_DOCINFO_DESCRIPTION:
            bIsString = sal_True;
            aValue = getDescription();
            break;
        case MID_DOCINFO_KEYWORDS:
            bIsString = sal_True;
            aValue = getKeywords();
            break;
        case MID_DOCINFO_SUBJECT:
            bIsString = sal_True;
            aValue = getSubject();
            break;
        case MID_DOCINFO_TITLE:
            bIsString = sal_True;
            aValue = getTitle();
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return sal_False;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else if ( bIsInt )
        rVal <<= nValue;
    else
        rVal <<= bValue;
    return true;
}

bool SfxDocumentInfoItem::PutValue( const Any& rVal, sal_uInt8 nMemberId )
{
    ::rtl::OUString aValue;
    sal_Int32 nValue=0;
    sal_Bool bValue = sal_False;
    bool bRet = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bRet = (rVal >>= bValue);
            if ( bRet )
                SetUseUserData( bValue );
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

//------------------------------------------------------------------------
SfxDocumentDescPage::SfxDocumentDescPage( Window * pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "DescriptionInfoPage", "sfx/ui/descriptioninfopage.ui", rItemSet)
    , m_pInfoItem   ( NULL )

{
    get(m_pTitleEd, "title");
    get(m_pThemaEd, "subject");
    get(m_pKeywordsEd, "keywords");
    get(m_pCommentEd, "comments");
}

//------------------------------------------------------------------------
SfxTabPage *SfxDocumentDescPage::Create(Window *pParent, const SfxItemSet &rItemSet)
{
     return new SfxDocumentDescPage(pParent, rItemSet);
}

//------------------------------------------------------------------------
sal_Bool SfxDocumentDescPage::FillItemSet(SfxItemSet &rSet)
{
    // Test whether a change is present
    const sal_Bool bTitleMod = m_pTitleEd->IsModified();
    const sal_Bool bThemeMod = m_pThemaEd->IsModified();
    const sal_Bool bKeywordsMod = m_pKeywordsEd->IsModified();
    const sal_Bool bCommentMod = m_pCommentEd->IsModified();
    if ( !( bTitleMod || bThemeMod || bKeywordsMod || bCommentMod ) )
    {
        return sal_False;
    }

    // Generating the output data
    const SfxPoolItem* pItem = NULL;
    SfxDocumentInfoItem* pInfo = NULL;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, sal_True, &pItem ) )
        pInfo = m_pInfoItem;
    else if ( pItem )
        pInfo = new SfxDocumentInfoItem( *(const SfxDocumentInfoItem *)pItem );

    if ( !pInfo )
    {
        SAL_WARN( "sfx2.dialog", "SfxDocumentDescPage::FillItemSet(): no item found" );
        return sal_False;
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
    rSet.Put( SfxDocumentInfoItem( *pInfo ) );
    if ( pInfo != m_pInfoItem )
    {
        delete pInfo;
    }

    return sal_True;
}

//------------------------------------------------------------------------
void SfxDocumentDescPage::Reset(const SfxItemSet &rSet)
{
    m_pInfoItem = &(SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);

    m_pTitleEd->SetText( m_pInfoItem->getTitle() );
    m_pThemaEd->SetText( m_pInfoItem->getSubject() );
    m_pKeywordsEd->SetText( m_pInfoItem->getKeywords() );
    m_pCommentEd->SetText( m_pInfoItem->getDescription() );

    SFX_ITEMSET_ARG( &rSet, pROItem, SfxBoolItem, SID_DOC_READONLY, sal_False );
    if ( pROItem && pROItem->GetValue() )
    {
        m_pTitleEd->SetReadOnly( sal_True );
        m_pThemaEd->SetReadOnly( sal_True );
        m_pKeywordsEd->SetReadOnly( sal_True );
        m_pCommentEd->SetReadOnly( sal_True );
    }
}

//------------------------------------------------------------------------
namespace
{
    String GetDateTimeString( sal_Int32 _nDate, sal_Int32 _nTime )
    {
        const LocaleDataWrapper& rWrapper( Application::GetSettings().GetLocaleDataWrapper() );

        Date aDate( _nDate );
        Time aTime( _nTime );
        String aStr( rWrapper.getDate( aDate ) );
        aStr.AppendAscii( ", " );
        aStr += rWrapper.getTime( aTime );
        return aStr;
    }

    // copy from xmlsecurity/source/dialog/resourcemanager.cxx
    String GetContentPart( const String& _rRawString, const String& _rPartId )
    {
        String s;

        xub_StrLen  nContStart = _rRawString.Search( _rPartId );
        if ( nContStart != STRING_NOTFOUND )
        {
            nContStart = nContStart + _rPartId.Len();
            ++nContStart; // now it's start of content, directly after Id

            xub_StrLen  nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );

            s = String( _rRawString, nContStart, nContEnd - nContStart );
        }

        return s;
    }
}

SfxDocumentPage::SfxDocumentPage(Window* pParent, const SfxItemSet& rItemSet)
    : SfxTabPage(pParent, "DocumentInfoPage", "sfx/ui/documentinfopage.ui", rItemSet)
    , bEnableUseUserData( sal_False )
    , bHandleDelete( sal_False )
{
    get(m_pBmp, "icon");
    get(m_pNameED, "nameed");
//FIXME    m_pNameED->SetAccessibleName( SfxResId( EDIT_FILE_NAME ).toString() );
    get(m_pChangePassBtn, "changepass");

    get(m_pShowTypeFT, "showtype");
    get(m_pReadOnlyCB, "readonlycb");
    get(m_pFileValFt, "showlocation");
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
                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( DOCUMENT_SIGNATURE_MENU_CMD ) ) ) )
        m_pSignatureBtn->Disable();
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxDocumentPage, DeleteHdl)
{
    String aName;
    if ( bEnableUseUserData && m_pUseUserDataCB->IsChecked() )
        aName = SvtUserOptions().GetFullName();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    DateTime now( DateTime::SYSTEM );
    util::DateTime uDT(
        now.Get100Sec(), now.GetSec(), now.GetMin(), now.GetHour(),
        now.GetDay(), now.GetMonth(), now.GetYear() );
    m_pCreateValFt->SetText( ConvertDateTime_Impl( aName, uDT, rLocaleWrapper ) );
    OUString aEmpty;
    m_pChangeValFt->SetText( aEmpty );
    m_pPrintValFt->SetText( aEmpty );
    const Time aTime( 0 );
    m_pTimeLogValFt->SetText( rLocaleWrapper.getDuration( aTime ) );
    m_pDocNoValFt->SetText(rtl::OUString('1'));
    bHandleDelete = sal_True;
    return 0;
}

IMPL_LINK_NOARG(SfxDocumentPage, SignatureHdl)
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if( pDoc )
    {
        pDoc->SignDocumentContent();

        ImplUpdateSignatures();
    }

    return 0;
}

IMPL_LINK_NOARG(SfxDocumentPage, ChangePassHdl)
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

        rtl::OUString aDocName;
        sfx2::RequestPassword(pFilter, aDocName, pMedSet);
        pShell->SetModified(true);
    }
    while (false);
    return 0;
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

            String s;
            Sequence< security::DocumentSignatureInformation > aInfos;
            aInfos = xD->verifyDocumentContentSignatures( pMedium->GetZipStorageToSign_Impl(),
                                                            uno::Reference< io::XInputStream >() );
            if ( aInfos.getLength() > 1 )
                s = m_aMultiSignedStr;
            else if ( aInfos.getLength() == 1 )
            {
                rtl::OUString aCN_Id("CN");
                const security::DocumentSignatureInformation& rInfo = aInfos[ 0 ];
                s = GetDateTimeString( rInfo.SignatureDate, rInfo.SignatureTime );
                s.AppendAscii( ", " );
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
        SFX_ITEMSET_ARG( pMedSet, pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False);
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

//------------------------------------------------------------------------

SfxTabPage* SfxDocumentPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
     return new SfxDocumentPage( pParent, rItemSet );
}

//------------------------------------------------------------------------

void SfxDocumentPage::EnableUseUserData()
{
    bEnableUseUserData = sal_True;
    m_pUseUserDataCB->Show();
    m_pDeleteBtn->Show();
}

//------------------------------------------------------------------------

sal_Bool SfxDocumentPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;

    if ( !bHandleDelete && bEnableUseUserData &&
         m_pUseUserDataCB->GetState() != m_pUseUserDataCB->GetSavedValue() &&
         GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, sal_True, &pItem ) )
        {
            SfxDocumentInfoItem* m_pInfoItem = (SfxDocumentInfoItem*)pItem;
            sal_Bool bUseData = ( STATE_CHECK == m_pUseUserDataCB->GetState() );
            m_pInfoItem->SetUseUserData( bUseData );
            rSet.Put( SfxDocumentInfoItem( *m_pInfoItem ) );
            bRet = sal_True;
        }
    }

    if ( bHandleDelete )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, sal_True, &pItem ) )
        {
            SfxDocumentInfoItem* m_pInfoItem = (SfxDocumentInfoItem*)pItem;
            sal_Bool bUseAuthor = bEnableUseUserData && m_pUseUserDataCB->IsChecked();
            SfxDocumentInfoItem newItem( *m_pInfoItem );
            newItem.resetUserData( bUseAuthor
                ? SvtUserOptions().GetFullName()
                : ::rtl::OUString() );
            m_pInfoItem->SetUseUserData( STATE_CHECK == m_pUseUserDataCB->GetState() );
            newItem.SetUseUserData( STATE_CHECK == m_pUseUserDataCB->GetState() );

            newItem.SetDeleteUserData( sal_True );
            rSet.Put( newItem );
            bRet = sal_True;
        }
    }

    if ( m_pNameED->IsModified() && !m_pNameED->GetText().isEmpty() )
    {
        rSet.Put( SfxStringItem( ID_FILETP_TITLE, m_pNameED->GetText() ) );
        bRet = sal_True;
    }

    if ( /* m_pReadOnlyCB->IsModified() */ sal_True )
    {
        rSet.Put( SfxBoolItem( ID_FILETP_READONLY, m_pReadOnlyCB->IsChecked() ) );
        bRet = sal_True;
    }

    return bRet;
}

//------------------------------------------------------------------------

void SfxDocumentPage::Reset( const SfxItemSet& rSet )
{
    // Determine the document information
    const SfxDocumentInfoItem *m_pInfoItem =
        &(const SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);

    // template data
    if ( m_pInfoItem->HasTemplate() )
        m_pTemplValFt->SetText( m_pInfoItem->getTemplateName() );
    else
    {
        m_pTemplFt->Hide();
        m_pTemplValFt->Hide();
    }

    // determine file name
    String aFile( m_pInfoItem->GetValue() );
    String aFactory( aFile );
    if ( aFile.Len() > 2 && aFile.GetChar(0) == '[' )
    {
        sal_uInt16 nPos = aFile.Search( ']' );
        aFactory = aFile.Copy( 1, nPos-1  );
        aFile = aFile.Copy( nPos+1 );
    }

    // determine name
    String aName;
    const SfxPoolItem* pItem = 0;
    if ( SFX_ITEM_SET != rSet.GetItemState( ID_FILETP_TITLE, sal_False, &pItem ) )
    {
        INetURLObject aURL(aFile);
        aName = aURL.GetName( INetURLObject::DECODE_WITH_CHARSET );
        if ( !aName.Len() || aURL.GetProtocol() == INET_PROT_PRIVATE )
            aName = SfxResId( STR_NONAME ).toString();
        m_pNameED->SetReadOnly( sal_True );
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ), "SfxDocumentPage:<SfxStringItem> expected" );
        aName = ( ( SfxStringItem* ) pItem )->GetValue();
    }
    m_pNameED->SetText( aName );
    m_pNameED->ClearModifyFlag();

    // determine RO-Flag
    if ( SFX_ITEM_UNKNOWN == rSet.GetItemState( ID_FILETP_READONLY, sal_False, &pItem )
         || !pItem )
        m_pReadOnlyCB->Hide();
    else
        m_pReadOnlyCB->Check( ( (SfxBoolItem*)pItem )->GetValue() );

    // determine context symbol
    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetSmartURL( aFactory);
    const String& rMainURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
    m_pBmp->SetImage( SvFileInformationManager::GetImage( aURL, sal_True ) );

    // determine size and type
    String aSizeText( m_aUnknownSize );
    if ( aURL.GetProtocol() == INET_PROT_FILE )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
    m_pShowSizeFT->SetText( aSizeText );

    String aDescription = SvFileInformationManager::GetDescription( INetURLObject(rMainURL) );
    if ( aDescription.Len() == 0 )
        aDescription = SfxResId( STR_SFX_NEWOFFICEDOC ).toString();
    m_pShowTypeFT->SetText( aDescription );

    // determine location
    aURL.SetSmartURL( aFile);
    if ( aURL.GetProtocol() == INET_PROT_FILE )
    {
        INetURLObject aPath( aURL );
        aPath.setFinalSlash();
        aPath.removeSegment();
        // we know it's a folder -> don't need the final slash, but it's better for WB_PATHELLIPSIS
        aPath.removeFinalSlash();
        String aText( aPath.PathToFileName() ); //! (pb) MaxLen?
        m_pFileValFt->SetText( aText );
    }
    else if ( aURL.GetProtocol() != INET_PROT_PRIVATE )
        m_pFileValFt->SetText( aURL.GetPartBeforeLastName() );

    // handle access data
    sal_Bool m_bUseUserData = m_pInfoItem->IsUseUserData();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_pCreateValFt->SetText( ConvertDateTime_Impl( m_pInfoItem->getAuthor(),
        m_pInfoItem->getCreationDate(), rLocaleWrapper ) );
    util::DateTime aTime( m_pInfoItem->getModificationDate() );
    if ( aTime.Month > 0 )
        m_pChangeValFt->SetText( ConvertDateTime_Impl(
            m_pInfoItem->getModifiedBy(), aTime, rLocaleWrapper ) );
    aTime = m_pInfoItem->getPrintDate();
    if ( aTime.Month > 0 )
        m_pPrintValFt->SetText( ConvertDateTime_Impl( m_pInfoItem->getPrintedBy(),
            aTime, rLocaleWrapper ) );
    const long nTime = m_pInfoItem->getEditingDuration();
    if ( m_bUseUserData )
    {
        const Time aT( nTime/3600, (nTime%3600)/60, nTime%60 );
        m_pTimeLogValFt->SetText( rLocaleWrapper.getDuration( aT ) );
        m_pDocNoValFt->SetText( String::CreateFromInt32(
            m_pInfoItem->getEditingCycles() ) );
    }

    TriState eState = (TriState)m_bUseUserData;

    if ( STATE_DONTKNOW == eState )
        m_pUseUserDataCB->EnableTriState( sal_True );

    m_pUseUserDataCB->SetState( eState );
    m_pUseUserDataCB->SaveValue();
    m_pUseUserDataCB->Enable( bEnableUseUserData );
    bHandleDelete = sal_False;
    m_pDeleteBtn->Enable( bEnableUseUserData );
}

//------------------------------------------------------------------------
SfxInternetPage::SfxInternetPage( Window* pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "InternetInfoPage", "sfx/ui/internetinfopage.ui", rItemSet)
    , aForwardErrorMessg(SFX2_RESSTR(RID_SVXSTR_FORWARD_ERRMSSG))
    , pInfoItem(NULL)
    , pFileDlg(NULL)
    , eState(S_Init)
{
    get(m_pContainer, "InternetInfoPage");
    get(m_pRBNoAutoUpdate, "noautoupdate");
    get(m_pRBReloadUpdate, "reloadupdate");
    get(m_pRBForwardUpdate, "forwardupdate");
    get(m_pFTEvery, "everyft");
    get(m_pNFReload, "reloadnf");
    get(m_pFTReloadSeconds, "seconds1ft");
    get(m_pFTAfter, "afterft");
    get(m_pNFAfter, "afternf");
    get(m_pFTAfterSeconds, "seconds1ft");
    get(m_pFTURL, "urlft");
    get(m_pEDForwardURL, "forwarded");
    get(m_pPBBrowseURL, "browse");
    get(m_pFTFrame, "frameft");
    get(m_pCBFrame, "framecb");

    pInfoItem = &( SfxDocumentInfoItem& ) rItemSet.Get( SID_DOCINFO );
    TargetList          aList;
    SfxViewFrame*       pFrame = SfxViewFrame::Current();
    if ( pFrame )
    {
        pFrame = pFrame->GetTopViewFrame();
        if ( pFrame )
        {
            pFrame->GetTargetList( aList );

            String* pObj;
            for ( size_t nPos = aList.size() ; nPos ; )
            {
                pObj = aList[ --nPos ];
                m_pCBFrame->InsertEntry( *pObj );
                delete pObj;
            }
        }
    }

    m_pRBNoAutoUpdate->SetClickHdl( LINK( this, SfxInternetPage, ClickHdlNoUpdate ) );
    m_pRBReloadUpdate->SetClickHdl( LINK( this, SfxInternetPage, ClickHdlReload ) );
    m_pRBForwardUpdate->SetClickHdl( LINK( this, SfxInternetPage, ClickHdlForward ) );
    m_pPBBrowseURL->SetClickHdl( LINK( this, SfxInternetPage, ClickHdlBrowseURL ) );

    aForwardErrorMessg.SearchAndReplaceAscii("%PLACEHOLDER%", m_pRBForwardUpdate->GetText());

    ChangeState( S_NoUpdate );
}

//------------------------------------------------------------------------
SfxInternetPage::~SfxInternetPage()
{
    delete pFileDlg;
}

//------------------------------------------------------------------------
void SfxInternetPage::ChangeState( STATE eNewState )
{
    DBG_ASSERT( eNewState != S_Init, "*SfxInternetPage::ChangeState(): new state init is supposed to not work here!" );

    if ( eState == eNewState  )
        return;

    switch ( eState )
    {
        case S_Init:
            EnableNoUpdate( sal_True );
            EnableReload( sal_False );
            EnableForward( sal_False );
            break;
        case S_NoUpdate:
            EnableNoUpdate( sal_False );
            if( eNewState == S_Reload )
                EnableReload( sal_True );
            else
                EnableForward( sal_True );
            break;
        case S_Reload:
            EnableReload( sal_False );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( sal_True );
            else
                EnableForward( sal_True );
            break;
        case S_Forward:
            EnableForward( sal_False );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( sal_True );
            else
                EnableReload( sal_True );
            break;
        default:
            OSL_FAIL( "*SfxInternetPage::SetState(): unhandled state!" );
    }

    eState = eNewState;
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableNoUpdate( sal_Bool bEnable )
{
    if ( bEnable )
        m_pRBNoAutoUpdate->Check();
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableReload( sal_Bool bEnable )
{
    m_pFTEvery->Enable( bEnable );
    m_pNFReload->Enable( bEnable );
    m_pFTReloadSeconds->Enable( bEnable );

    if ( bEnable )
        m_pRBReloadUpdate->Check();
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableForward( sal_Bool bEnable )
{
    m_pFTAfter->Enable( bEnable );
    m_pNFAfter->Enable( bEnable );
    m_pFTAfterSeconds->Enable( bEnable );
    m_pFTURL->Enable( bEnable );
    m_pEDForwardURL->Enable( bEnable );
    m_pPBBrowseURL->Enable( bEnable );
    m_pFTFrame->Enable( bEnable );
    m_pCBFrame->Enable( bEnable );

    if ( bEnable )
        m_pRBForwardUpdate->Check();
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlNoUpdate, Control*, pCtrl )
{
    (void)pCtrl; //unused
    ChangeState( S_NoUpdate );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlReload, Control*, pCtrl )
{
    (void)pCtrl; //unused
    ChangeState( S_Reload );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlForward, Control*, pCtrl )
{
    (void)pCtrl; //unused
    ChangeState( S_Forward );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxInternetPage, ClickHdlBrowseURL)
{
    if ( !pFileDlg )
        pFileDlg = new sfx2::FileDialogHelper(
                        TemplateDescription::FILEOPEN_SIMPLE, 0 );
    pFileDlg->SetDisplayDirectory( m_pEDForwardURL->GetText() );
    pFileDlg->StartExecuteModal( LINK( this, SfxInternetPage, DialogClosedHdl ) );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxInternetPage, DialogClosedHdl)
{
    DBG_ASSERT( pFileDlg, "SfxInternetPage::DialogClosedHdl(): no file dialog" );

    if ( ERRCODE_NONE == pFileDlg->GetError() )
        m_pEDForwardURL->SetText( pFileDlg->GetPath() );

    return 0;
}

//------------------------------------------------------------------------

sal_Bool SfxInternetPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem*          pItem = NULL;
    SfxDocumentInfoItem*        pInfo = NULL;
    SfxTabDialog*               pDlg = GetTabDialog();
    const SfxItemSet*           pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, sal_True, &pItem ) )
        pInfo = pInfoItem;
    else if ( pItem )
        pInfo = new SfxDocumentInfoItem( *(const SfxDocumentInfoItem*)pItem );

    if ( !pInfo )
    {
        SAL_WARN( "sfx2.dialog", "SfxInternetPage::FillItemSet(): no item found" );
        return sal_False;
    }

    DBG_ASSERT( eState != S_Init, "*SfxInternetPage::FillItemSet(): state init is not acceptable at this point!" );

    sal_Bool                        bEnableReload = sal_False;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< String >   aURL( NULL );
    ::std::auto_ptr< String >   aFrame( NULL );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_uIntPtr                     nDelay = 0;

    switch ( eState )
    {
        case S_NoUpdate:
            break;
        case S_Reload:
            bEnableReload = sal_True;
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            aURL = ::std::auto_ptr< String >( new String() );
            aFrame = ::std::auto_ptr< String >( new String() );
            SAL_WNODEPRECATED_DECLARATIONS_POP
            nDelay = static_cast<sal_uIntPtr>(m_pNFReload->GetValue());
            break;
        case S_Forward:
            DBG_ASSERT( m_pEDForwardURL->GetText().Len(), "+SfxInternetPage::FillItemSet(): empty URL should be not possible for forward option!" );

            bEnableReload = sal_True;
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            aURL = ::std::auto_ptr< String >( new String( URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), m_pEDForwardURL->GetText(), URIHelper::GetMaybeFileHdl(), true ) ) );
            aFrame = ::std::auto_ptr< String >( new String( m_pCBFrame->GetText() ) );
            SAL_WNODEPRECATED_DECLARATIONS_POP
            nDelay = static_cast<sal_uIntPtr>(m_pNFAfter->GetValue());
            break;
        default:
            break;
    }

    pInfo->setAutoloadEnabled( bEnableReload );

    if ( bEnableReload )
    {
        pInfo->setAutoloadURL( *aURL.get() );
        pInfo->setDefaultTarget( *aFrame.get() );
        pInfo->setAutoloadDelay( nDelay );
    }

    rSet.Put( *pInfo );
    if ( pInfo != pInfoItem )
        delete pInfo;
    return sal_True;
}

//------------------------------------------------------------------------
SfxTabPage *SfxInternetPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return new SfxInternetPage(pParent, rItemSet);
}

//------------------------------------------------------------------------
void SfxInternetPage::Reset( const SfxItemSet& rSet )
{
    pInfoItem = &( SfxDocumentInfoItem& ) rSet.Get( SID_DOCINFO );
    SFX_ITEMSET_ARG( &rSet, pURLItem, SfxStringItem, SID_BASEURL, sal_False );
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
    if ( pURLItem )
        aBaseURL = pURLItem->GetValue();

    STATE eNewState = S_NoUpdate;

    if ( pInfoItem->isAutoloadEnabled() )
    {
        const String&   rURL = pInfoItem->getAutoloadURL();

        if( rURL.Len() )
        {
            m_pNFAfter->SetValue( pInfoItem->getAutoloadDelay() );
            m_pEDForwardURL->SetText( rURL );
            m_pCBFrame->SetText( pInfoItem->getDefaultTarget() );
            eNewState = S_Forward;
        }
        else
        {
            m_pNFReload->SetValue( pInfoItem->getAutoloadDelay() );
            eNewState = S_Reload;
        }
    }

    ChangeState( eNewState );

    SFX_ITEMSET_ARG( &rSet, pROItem, SfxBoolItem, SID_DOC_READONLY, sal_False );
    if ( pROItem && pROItem->GetValue() )
        m_pContainer->Disable();
}

//------------------------------------------------------------------------
int SfxInternetPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    int nRet = LEAVE_PAGE;

    if ( eState == S_Forward && m_pEDForwardURL->GetText().isEmpty() )
    {
        ErrorBox aErrBox( this, WB_OK, aForwardErrorMessg );
        aErrBox.Execute();

        nRet = KEEP_PAGE;
    }

    return nRet;
}

//------------------------------------------------------------------------
SfxDocumentInfoDialog::SfxDocumentInfoDialog( Window* pParent,
                                              const SfxItemSet& rItemSet )
    : SfxTabDialog(0, pParent, "DocumentPropertiesDialog",
        "sfx/ui/documentpropertiesdialog.ui", &rItemSet)
    , m_nDocInfoId(0)
{
    const SfxDocumentInfoItem* m_pInfoItem =
        &(const SfxDocumentInfoItem &)rItemSet.Get( SID_DOCINFO );

#ifdef DBG_UTIL
    SFX_ITEMSET_ARG( &rItemSet, pURLItem, SfxStringItem, SID_BASEURL, sal_False );
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
#endif

     // Determine the Titels
    const SfxPoolItem* pItem = 0;
    String aTitle( GetText() );
    if ( SFX_ITEM_SET !=
         rItemSet.GetItemState( SID_EXPLORER_PROPS_START, sal_False, &pItem ) )
    {
        // File name
        String aFile( m_pInfoItem->GetValue() );

        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( aFile);
        if ( INET_PROT_PRIV_SOFFICE != aURL.GetProtocol() )
        {
            String aLastName( aURL.GetLastName() );
            if ( aLastName.Len() )
                aTitle += aLastName;
            else
                aTitle += aFile;
        }
        else
            aTitle += SfxResId( STR_NONAME ).toString();
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ),
                    "SfxDocumentInfoDialog:<SfxStringItem> expected" );
        aTitle += ( ( SfxStringItem* ) pItem )->GetValue();
    }
    SetText( aTitle );

    // Property Pages
    m_nDocInfoId = AddTabPage("general", SfxDocumentPage::Create, 0);
    AddTabPage("description", SfxDocumentDescPage::Create, 0);
    AddTabPage("customprops", SfxCustomPropertiesPage::Create, 0);
    AddTabPage("internet", SfxInternetPage::Create, 0);
    AddTabPage("security", SfxSecurityPage::Create, 0);
}

// -----------------------------------------------------------------------

void SfxDocumentInfoDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if ( m_nDocInfoId == nId )
        ( (SfxDocumentPage&)rPage ).EnableUseUserData();
}

// class CustomPropertiesYesNoButton -------------------------------------

CustomPropertiesYesNoButton::CustomPropertiesYesNoButton( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    m_aYesButton( this, ResId( RB_PROPERTY_YES, *rResId.GetResMgr() ) ),
    m_aNoButton ( this, ResId( RB_PROPERTY_NO, *rResId.GetResMgr() ) )
{
    FreeResource();
    Wallpaper aWall( Color( COL_TRANSPARENT ) );
    SetBackground( aWall );
    SetBorderStyle( WINDOW_BORDER_MONO  );
    CheckNo();
    m_aYesButton.SetBackground( aWall );
    m_aNoButton.SetBackground( aWall );
}
class DurationDialog_Impl : public ModalDialog
{
    FixedLine       aDurationFL;

    OKButton        aOKPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;

    CheckBox        aNegativeCB;
    FixedText       aYearFT;
    NumericField    aYearNF;
    FixedText       aMonthFT;
    NumericField    aMonthNF;
    FixedText       aDayFT;
    NumericField    aDayNF;
    FixedText       aHourFT;
    NumericField    aHourNF;
    FixedText       aMinuteFT;
    NumericField    aMinuteNF;
    FixedText       aSecondFT;
    NumericField    aSecondNF;
    FixedText       aMSecondFT;
    NumericField    aMSecondNF;

public:

    DurationDialog_Impl( Window* pParent, const util::Duration& rDuration );
    ~DurationDialog_Impl();

    util::Duration  GetDuration() const;
};

DurationDialog_Impl::DurationDialog_Impl(
    Window* pParent, const util::Duration& rDuration)
        :   ModalDialog( pParent, SfxResId( RID_EDIT_DURATIONS ) ),
            aDurationFL(this, SfxResId( FL_DURATION       )),
            aOKPB(      this, SfxResId( PB_OK       )),
            aCancelPB(  this, SfxResId( PB_CANCEL   )),
            aHelpPB(    this, SfxResId( PB_HELP     )),
            aNegativeCB(this, SfxResId( CB_NEGATIVE     )),
            aYearFT(    this, SfxResId( FT_YEAR         )),
            aYearNF(    this, SfxResId( ED_YEAR         )),
            aMonthFT(   this, SfxResId( FT_MONTH        )),
            aMonthNF(   this, SfxResId( ED_MONTH        )),
            aDayFT(     this, SfxResId( FT_DAY          )),
            aDayNF(     this, SfxResId( ED_DAY          )),
            aHourFT(    this, SfxResId( FT_HOUR         )),
            aHourNF(    this, SfxResId( ED_HOUR         )),
            aMinuteFT(  this, SfxResId( FT_MINUTE       )),
            aMinuteNF(  this, SfxResId( ED_MINUTE       )),
            aSecondFT(  this, SfxResId( FT_SECOND       )),
            aSecondNF(  this, SfxResId( ED_SECOND       )),
            aMSecondFT( this, SfxResId( FT_MSECOND      )),
            aMSecondNF( this, SfxResId( ED_MSECOND      ))
{
    FreeResource();
    aNegativeCB.Check(rDuration.Negative);
    aYearNF.SetValue(rDuration.Years);
    aMonthNF.SetValue(rDuration.Months );
    aDayNF.SetValue(rDuration.Days   );
    aHourNF.SetValue(rDuration.Hours  );
    aMinuteNF.SetValue(rDuration.Minutes);
    aSecondNF.SetValue(rDuration.Seconds);
    aMSecondNF.SetValue(rDuration.MilliSeconds);
}

DurationDialog_Impl::~DurationDialog_Impl()
{
}

util::Duration  DurationDialog_Impl::GetDuration() const
{
    util::Duration  aRet;
    aRet.Negative = aNegativeCB.IsChecked();
    aRet.Years = aYearNF.GetValue();
    aRet.Months = aMonthNF.GetValue( );
    aRet.Days = aDayNF.GetValue(   );
    aRet.Hours  = aHourNF.GetValue( );
    aRet.Minutes = aMinuteNF.GetValue();
    aRet.Seconds = aSecondNF.GetValue();
    aRet.MilliSeconds = aMSecondNF.GetValue();
    return aRet;
}

CustomPropertiesDurationField::CustomPropertiesDurationField( Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
        Edit( pParent, rResId ), m_pLine( pLine )

{
    SetDuration( util::Duration(false, 0, 0, 0, 0, 0, 0, 0) );
}

CustomPropertiesDurationField::~CustomPropertiesDurationField()
{
}

void CustomPropertiesDurationField::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Size aSize( GetSizePixel() );
        Rectangle aItemRect( rHEvt.GetMousePosPixel(), aSize );
        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), GetText() );
        else
            Help::ShowQuickHelp( this, aItemRect, GetText(),
                QUICKHELP_LEFT|QUICKHELP_VCENTER );
    }
}

void CustomPropertiesDurationField::SetDuration( const util::Duration& rDuration )
{
    m_aDuration = rDuration;
    String sText(rDuration.Negative ? rtl::OUString('-') : rtl::OUString('+'));
    sText += m_pLine->m_sDurationFormat;
    sText.SearchAndReplace(rtl::OUString("%1"), String::CreateFromInt32( rDuration.Years ) );
    sText.SearchAndReplace(rtl::OUString("%2"), String::CreateFromInt32( rDuration.Months ) );
    sText.SearchAndReplace(rtl::OUString("%3"), String::CreateFromInt32( rDuration.Days   ) );
    sText.SearchAndReplace(rtl::OUString("%4"), String::CreateFromInt32( rDuration.Hours  ) );
    sText.SearchAndReplace(rtl::OUString("%5"), String::CreateFromInt32( rDuration.Minutes) );
    sText.SearchAndReplace(rtl::OUString("%6"), String::CreateFromInt32( rDuration.Seconds) );
    SetText( sText );
}

CustomPropertiesEditButton::CustomPropertiesEditButton( Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
        PushButton( pParent, rResId ), m_pLine( pLine )
{
    SetClickHdl( LINK( this, CustomPropertiesEditButton, ClickHdl ));
}

CustomPropertiesEditButton::~CustomPropertiesEditButton()
{
}

IMPL_LINK_NOARG(CustomPropertiesEditButton, ClickHdl)
{
    DurationDialog_Impl* pDurationDlg = new DurationDialog_Impl( this, m_pLine->m_aDurationField.GetDuration() );
    if ( RET_OK == pDurationDlg->Execute() )
        m_pLine->m_aDurationField.SetDuration( pDurationDlg->GetDuration() );
    delete pDurationDlg;
    return 1;
}
//--------------------------------------------------------------------------
void CustomPropertiesYesNoButton::Resize()
{
    const long nWidth = GetSizePixel().Width();
    const long n3Width = LogicToPixel( Size( 3, 3 ), MAP_APPFONT ).Width();
    const long nNewWidth = ( nWidth / 2 ) - n3Width - 2;
    Size aSize = m_aYesButton.GetSizePixel();
    const long nDelta = aSize.Width() - nNewWidth;
    aSize.Width() = nNewWidth;
    m_aYesButton.SetSizePixel( aSize );
    Point aPos = m_aNoButton.GetPosPixel();
    aPos.X() -= nDelta;
    m_aNoButton.SetPosSizePixel( aPos, aSize );
}

// struct CustomPropertyLine ---------------------------------------------
CustomPropertyLine::CustomPropertyLine( Window* pParent ) :
    m_aNameBox      ( pParent, SfxResId( SFX_CB_PROPERTY_NAME ) ),
    m_aTypeBox      ( pParent, SfxResId( SFX_LB_PROPERTY_TYPE ), this ),
    m_aValueEdit    ( pParent, SfxResId( SFX_ED_PROPERTY_VALUE ), this ),
    m_aDateField    ( pParent, SfxResId( SFX_FLD_DATE), this),
    m_aTimeField    ( pParent, SfxResId( SFX_FLD_TIME), this),
    m_sDurationFormat( SfxResId( SFX_ST_DURATION_FORMAT ).toString() ),
    m_aDurationField( pParent, SfxResId( SFX_FLD_DURATION), this),
    m_aEditButton(    pParent, SfxResId( SFX_PB_EDIT ), this),
    m_aYesNoButton  ( pParent, SfxResId( SFX_WIN_PROPERTY_YESNO ) ),
    m_aRemoveButton ( pParent, SfxResId( SFX_PB_PROPERTY_REMOVE ), this ),
    m_bIsRemoved    ( false ),
    m_bTypeLostFocus( false )

{
    m_aTimeField.SetExtFormat( EXTTIMEF_24H_LONG );
    m_aDateField.SetExtDateFormat( XTDATEF_SYSTEM_SHORT_YYYY );

    m_aRemoveButton.SetModeImage( SfxResId( SFX_IMG_PROPERTY_REMOVE ) );
}

void CustomPropertyLine::SetRemoved()
{
    DBG_ASSERT( !m_bIsRemoved, "CustomPropertyLine::SetRemoved(): line already removed" );
    m_bIsRemoved = true;
    m_aNameBox.Hide();
    m_aTypeBox.Hide();
    m_aValueEdit.Hide();
    m_aDateField.Hide();
    m_aTimeField.Hide();
    m_aDurationField.Hide();
    m_aEditButton.Hide();
    m_aYesNoButton.Hide();
    m_aRemoveButton.Hide();
}

CustomPropertiesWindow::CustomPropertiesWindow(Window* pParent,
    const OUString &rHeaderAccName,
    const OUString &rHeaderAccType,
    const OUString &rHeaderAccValue) :
    Window(pParent),
    m_aNameBox      ( this, SfxResId( SFX_CB_PROPERTY_NAME ) ),
    m_aTypeBox      ( this, SfxResId( SFX_LB_PROPERTY_TYPE ) ),
    m_aValueEdit    ( this, SfxResId( SFX_ED_PROPERTY_VALUE ) ),
    m_aDateField    ( this, SfxResId( SFX_FLD_DATE) ),
    m_aTimeField    ( this, SfxResId( SFX_FLD_TIME) ),
    m_aDurationField( this, SfxResId( SFX_FLD_DURATION) ),
    m_aEditButton(    this, SfxResId( SFX_PB_EDIT )),
    m_aYesNoButton  ( this, SfxResId( SFX_WIN_PROPERTY_YESNO ) ),
    m_aRemoveButton ( this, SfxResId( SFX_PB_PROPERTY_REMOVE ) ),
    m_nScrollPos (0),
    m_aNumberFormatter( ::comphelper::getProcessServiceFactory(),
                        Application::GetSettings().GetLanguageTag().getLanguageType() )

{
    m_aEditLoseFocusTimer.SetTimeout( 300 );
    m_aEditLoseFocusTimer.SetTimeoutHdl( LINK( this, CustomPropertiesWindow, EditTimeoutHdl ) );
    m_aBoxLoseFocusTimer.SetTimeout( 300 );
    m_aBoxLoseFocusTimer.SetTimeoutHdl( LINK( this, CustomPropertiesWindow, BoxTimeoutHdl ) );

    m_aNameBox.SetAccessibleName(rHeaderAccName);
    m_aTypeBox.SetAccessibleName(rHeaderAccType);
    m_aValueEdit.SetAccessibleName(rHeaderAccValue);
}

CustomPropertiesWindow::~CustomPropertiesWindow()
{
    m_aEditLoseFocusTimer.Stop();
    m_aBoxLoseFocusTimer.Stop();
    ClearAllLines();
}

IMPL_LINK( CustomPropertiesWindow, TypeHdl, CustomPropertiesTypeBox*, pBox )
{
    sal_Int64 nType = sal_Int64( (long)pBox->GetEntryData( pBox->GetSelectEntryPos() ) );
    CustomPropertyLine* pLine = pBox->GetLine();
    pLine->m_aValueEdit.Show( (CUSTOM_TYPE_TEXT == nType) || (CUSTOM_TYPE_NUMBER  == nType) );
    pLine->m_aDateField.Show( (CUSTOM_TYPE_DATE == nType) || (CUSTOM_TYPE_DATETIME  == nType) );
    pLine->m_aTimeField.Show( CUSTOM_TYPE_DATETIME  == nType );
    pLine->m_aDurationField.Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aEditButton.Show( CUSTOM_TYPE_DURATION == nType );
    pLine->m_aYesNoButton.Show( CUSTOM_TYPE_BOOLEAN == nType );
    //adjust positions of date and time controls
    if ( nType == CUSTOM_TYPE_DATE )
        pLine->m_aDateField.SetPosSizePixel(pLine->m_aValueEdit.GetPosPixel(), pLine->m_aValueEdit.GetSizePixel());
    else if ( nType == CUSTOM_TYPE_DATETIME)
    {
        pLine->m_aDateField.SetPosSizePixel( pLine->m_aDatePos, pLine->m_aDateTimeSize );
        pLine->m_aTimeField.SetPosSizePixel(pLine->m_aTimePos, pLine->m_aDateTimeSize );
    }

    return 0;
}

IMPL_LINK( CustomPropertiesWindow, RemoveHdl, CustomPropertiesRemoveButton*, pButton )
{
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

            Window* pWindows[] = {  &pLine->m_aNameBox, &pLine->m_aTypeBox, &pLine->m_aValueEdit,
                                    &pLine->m_aDateField, &pLine->m_aTimeField,
                                    &pLine->m_aDurationField, &pLine->m_aEditButton,
                                    &pLine->m_aYesNoButton, &pLine->m_aRemoveButton, NULL };
            Window** pCurrent = pWindows;
            while ( *pCurrent )
            {
                Point aPos = (*pCurrent)->GetPosPixel();
                aPos.Y() -= nDelta;
                (*pCurrent)->SetPosPixel( aPos );
                pCurrent++;
            }
        }
    }

    m_aRemovedHdl.Call(0);
    return 0;
}

IMPL_LINK( CustomPropertiesWindow, EditLoseFocusHdl, CustomPropertiesEdit*, pEdit )
{
    if ( pEdit )
    {
        CustomPropertyLine* pLine = pEdit->GetLine();
        if ( !pLine->m_bTypeLostFocus )
        {
            m_pCurrentLine = pLine;
            m_aEditLoseFocusTimer.Start();
        }
        else
            pLine->m_bTypeLostFocus = false;
    }
    return 0;
}

IMPL_LINK( CustomPropertiesWindow, BoxLoseFocusHdl, CustomPropertiesTypeBox*, pBox )
{
    if ( pBox )
    {
        m_pCurrentLine = pBox->GetLine();
        m_aBoxLoseFocusTimer.Start();
    }

    return 0;
}

IMPL_LINK_NOARG(CustomPropertiesWindow, EditTimeoutHdl)
{
    ValidateLine( m_pCurrentLine, false );
    return 0;
}

IMPL_LINK_NOARG(CustomPropertiesWindow, BoxTimeoutHdl)
{
    ValidateLine( m_pCurrentLine, true );
    return 0;
}

bool CustomPropertiesWindow::IsLineValid( CustomPropertyLine* pLine ) const
{
    bool bIsValid = true;
    pLine->m_bTypeLostFocus = false;
    sal_Int64 nType = sal_Int64(
        (long)pLine->m_aTypeBox.GetEntryData( pLine->m_aTypeBox.GetSelectEntryPos() ) );
    String sValue = pLine->m_aValueEdit.GetText();
    if ( sValue.Len() == 0 )
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
            m_aNumberFormatter ).IsNumberFormat( sValue, nIndex, fDummy ) != sal_False;
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
        Window* pParent = GetParent()->GetParent();
        if ( QueryBox( pParent, SfxResId( SFX_QB_WRONG_TYPE ) ).Execute() == RET_OK )
            pLine->m_aTypeBox.SelectEntryPos( m_aTypeBox.GetEntryPos( (void*)CUSTOM_TYPE_TEXT ) );
        else
            pLine->m_aValueEdit.GrabFocus();
    }
}

void CustomPropertiesWindow::InitControls( HeaderBar* pHeaderBar, const ScrollBar* pScrollBar )
{
    DBG_ASSERT( pHeaderBar, "CustomPropertiesWindow::InitControls(): invalid headerbar" );
    DBG_ASSERT( pScrollBar, "CustomPropertiesWindow::InitControls(): invalid scrollbar" );

    m_aNameBox.Hide();
    m_aTypeBox.Hide();
    m_aValueEdit.Hide();
    m_aDateField.Hide();
    m_aTimeField.Hide();
    m_aDurationField.Hide();
    m_aEditButton.Hide();
    m_aYesNoButton.Hide();
    m_aRemoveButton.Hide();

    const long nOffset = 4;
    const long nScrollBarWidth = pScrollBar->GetSizePixel().Width();
    const long nButtonWidth = m_aRemoveButton.GetSizePixel().Width() + nScrollBarWidth + nOffset;
    long nTypeWidth = m_aTypeBox.CalcMinimumSize().Width() + ( 2 * nOffset );
    long nFullWidth = pHeaderBar->GetSizePixel().Width();
    long nItemWidth = ( nFullWidth - nTypeWidth - nButtonWidth ) / 2;
    pHeaderBar->SetItemSize( HI_NAME, nItemWidth );
    pHeaderBar->SetItemSize( HI_TYPE, nTypeWidth );
    pHeaderBar->SetItemSize( HI_VALUE, nItemWidth );
    pHeaderBar->SetItemSize( HI_ACTION, nButtonWidth );

    Window* pWindows[] = { &m_aNameBox, &m_aTypeBox, &m_aValueEdit, &m_aRemoveButton, NULL };
    Window** pCurrent = pWindows;
    sal_uInt16 nPos = 0;
    while ( *pCurrent )
    {
        Rectangle aRect = pHeaderBar->GetItemRect( pHeaderBar->GetItemId( nPos++ ) );
        Size aSize = (*pCurrent)->GetSizePixel();
        Point aPos = (*pCurrent)->GetPosPixel();
        long nWidth = aRect.GetWidth() - nOffset;
        if ( *pCurrent == &m_aRemoveButton )
            nWidth -= pScrollBar->GetSizePixel().Width();
        aSize.Width() = nWidth;
        aPos.X() = aRect.getX() + ( nOffset / 2 );
        (*pCurrent)->SetPosSizePixel( aPos, aSize );

        if ( *pCurrent == &m_aValueEdit )
        {
            Point aDurationPos( aPos );
            m_aDurationField.SetPosPixel( aDurationPos );
            Size aDurationSize(aSize);
            aDurationSize.Width() -= (m_aEditButton.GetSizePixel().Width() + 3 );
            m_aDurationField.SetSizePixel(aDurationSize);
            aDurationPos.X() = aPos.X() - m_aEditButton.GetSizePixel().Width() + aSize.Width();
            m_aEditButton.SetPosPixel(aDurationPos);
            aSize = m_aYesNoButton.GetSizePixel();
            aPos = m_aYesNoButton.GetPosPixel();
            aSize.Width() = nWidth;
            aPos.X() = aRect.getX() + ( nOffset / 2 );
            m_aYesNoButton.SetPosSizePixel( aPos, aSize );
            aSize.Width() /= 2;
            aSize.Width() -= 2;
            m_aDateField.SetPosSizePixel( aPos, aSize );
            aPos.X() += aSize.Width() + 4;
            m_aTimeField.SetPosSizePixel( aPos, aSize );
        }

        pCurrent++;
    }

    m_nLineHeight =
        ( m_aRemoveButton.GetPosPixel().Y() * 2 ) + m_aRemoveButton.GetSizePixel().Height();
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
    Window* pWindows[] = {  &m_aNameBox, &m_aTypeBox, &m_aValueEdit,
                            &m_aDateField, &m_aTimeField,
                            &m_aDurationField, &m_aEditButton,
                            &m_aYesNoButton, &m_aRemoveButton, NULL };

    for (std::vector< CustomPropertyLine* >::iterator aI =
        m_aCustomPropertiesLines.begin(), aEnd = m_aCustomPropertiesLines.end();
        aI != aEnd; ++aI)
    {
        CustomPropertyLine* pNewLine = *aI;

        Window* pNewWindows[] =
            {   &pNewLine->m_aNameBox, &pNewLine->m_aTypeBox, &pNewLine->m_aValueEdit,
                &pNewLine->m_aDateField, &pNewLine->m_aTimeField,
                &pNewLine->m_aDurationField, &pNewLine->m_aEditButton,
                &pNewLine->m_aYesNoButton, &pNewLine->m_aRemoveButton, NULL };

        Window** pCurrent = pWindows;
        Window** pNewCurrent = pNewWindows;
        while ( *pCurrent )
        {
            Size aSize = (*pCurrent)->GetSizePixel();
            Point aPos = (*pCurrent)->GetPosPixel();
            aPos.Y() = (*pNewCurrent)->GetPosPixel().Y();
            (*pNewCurrent)->SetPosSizePixel( aPos, aSize );
            pCurrent++;
            pNewCurrent++;
        }
    }
}

void CustomPropertiesWindow::AddLine( const ::rtl::OUString& sName, Any& rAny )
{
    CustomPropertyLine* pNewLine = new CustomPropertyLine( this );
    pNewLine->m_aTypeBox.SetSelectHdl( LINK( this, CustomPropertiesWindow, TypeHdl ) );
    pNewLine->m_aRemoveButton.SetClickHdl( LINK( this, CustomPropertiesWindow, RemoveHdl ) );
    pNewLine->m_aValueEdit.SetLoseFocusHdl( LINK( this, CustomPropertiesWindow, EditLoseFocusHdl ) );
    //add lose focus handlers of date/time fields

    pNewLine->m_aTypeBox.SetLoseFocusHdl( LINK( this, CustomPropertiesWindow, BoxLoseFocusHdl ) );

    pNewLine->m_aNameBox.SetAccessibleName(m_aNameBox.GetAccessibleName());
    pNewLine->m_aTypeBox.SetAccessibleName(m_aTypeBox.GetAccessibleName());
    pNewLine->m_aValueEdit.SetAccessibleName(m_aValueEdit.GetAccessibleName());

    long nPos = GetVisibleLineCount() * GetLineHeight();
    m_aCustomPropertiesLines.push_back( pNewLine );
    Window* pWindows[] = {  &m_aNameBox, &m_aTypeBox, &m_aValueEdit,
                            &m_aDateField, &m_aTimeField,
                            &m_aDurationField, &m_aEditButton,
                            &m_aYesNoButton, &m_aRemoveButton, NULL };
    Window* pNewWindows[] =
        {   &pNewLine->m_aNameBox, &pNewLine->m_aTypeBox, &pNewLine->m_aValueEdit,
            &pNewLine->m_aDateField, &pNewLine->m_aTimeField,
            &pNewLine->m_aDurationField, &pNewLine->m_aEditButton,
            &pNewLine->m_aYesNoButton, &pNewLine->m_aRemoveButton, NULL };
    Window** pCurrent = pWindows;
    Window** pNewCurrent = pNewWindows;
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

    pNewLine->m_aDatePos = pNewLine->m_aDateField.GetPosPixel();
    pNewLine->m_aTimePos = pNewLine->m_aTimeField.GetPosPixel();
    pNewLine->m_aDateTimeSize = pNewLine->m_aDateField.GetSizePixel();

    double nTmpValue = 0;
    bool bTmpValue = false;
    ::rtl::OUString sTmpValue;
    util::DateTime aTmpDateTime;
    util::Date aTmpDate;
    util::Duration aTmpDuration;
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    pNewLine->m_aNameBox.SetText( sName );
    sal_IntPtr nType = CUSTOM_TYPE_UNKNOWN;
    String sValue;

    if ( rAny >>= nTmpValue )
    {
        sal_uInt32 nIndex = m_aNumberFormatter.GetFormatIndex( NF_NUMBER_SYSTEM );
        m_aNumberFormatter.GetInputLineString( nTmpValue, nIndex, sValue );
        pNewLine->m_aValueEdit.SetText( sValue );
        nType = CUSTOM_TYPE_NUMBER;
    }
    else if ( rAny >>= bTmpValue )
    {
        sValue = ( bTmpValue ? rLocaleWrapper.getTrueWord() : rLocaleWrapper.getFalseWord() );
        nType = CUSTOM_TYPE_BOOLEAN;
    }
    else if ( rAny >>= sTmpValue )
    {
        pNewLine->m_aValueEdit.SetText( sTmpValue );
        nType = CUSTOM_TYPE_TEXT;
    }
    else if ( rAny >>= aTmpDate )
    {
        nType = CUSTOM_TYPE_DATE;
        pNewLine->m_aDateField.SetDate( Date( aTmpDate.Day, aTmpDate.Month, aTmpDate.Year ) );

    }
    else if ( rAny >>= aTmpDuration )
    {
        nType = CUSTOM_TYPE_DURATION;
        pNewLine->m_aDurationField.SetDuration( aTmpDuration );
    }
    else if ( rAny >>= aTmpDateTime )
    {
        pNewLine->m_aDateField.SetDate( Date( aTmpDateTime.Day, aTmpDateTime.Month, aTmpDateTime.Year ) );
        pNewLine->m_aTimeField.SetTime( Time( aTmpDateTime.Hours, aTmpDateTime.Minutes, aTmpDateTime.Seconds, aTmpDateTime.HundredthSeconds ) );

        nType = CUSTOM_TYPE_DATETIME;
    }

    if ( nType != CUSTOM_TYPE_UNKNOWN )
    {
        if ( CUSTOM_TYPE_BOOLEAN == nType )
        {
            if ( bTmpValue )
                pNewLine->m_aYesNoButton.CheckYes();
            else
                pNewLine->m_aYesNoButton.CheckNo();
        }
        pNewLine->m_aTypeBox.SelectEntryPos( m_aTypeBox.GetEntryPos( (void*)nType ) );
    }

    TypeHdl( &pNewLine->m_aTypeBox );
    pNewLine->m_aNameBox.GrabFocus();
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

        Window* pWindows[] = {  &pLine->m_aNameBox, &pLine->m_aTypeBox, &pLine->m_aValueEdit, &pLine->m_aDateField, &pLine->m_aTimeField,
                                &pLine->m_aDurationField, &pLine->m_aEditButton, &pLine->m_aYesNoButton, &pLine->m_aRemoveButton, NULL };
        Window** pCurrent = pWindows;
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

        String sPropertyName = pLine->m_aNameBox.GetText();
        if ( sPropertyName.Len() > 0 )
        {
            aPropertiesSeq[i].Name = sPropertyName;
            sal_Int64 nType = sal_Int64(
                (long)pLine->m_aTypeBox.GetEntryData( pLine->m_aTypeBox.GetSelectEntryPos() ) );
            if ( CUSTOM_TYPE_NUMBER == nType )
            {
                double nValue = 0;
                sal_uInt32 nIndex = const_cast< SvNumberFormatter& >(
                    m_aNumberFormatter ).GetFormatIndex( NF_NUMBER_SYSTEM );
                sal_Bool bIsNum = const_cast< SvNumberFormatter& >( m_aNumberFormatter ).
                    IsNumberFormat( pLine->m_aValueEdit.GetText(), nIndex, nValue );
                if ( bIsNum )
                    aPropertiesSeq[i].Value <<= makeAny( nValue );
            }
            else if ( CUSTOM_TYPE_BOOLEAN == nType )
            {
                bool bValue = pLine->m_aYesNoButton.IsYesChecked();
                aPropertiesSeq[i].Value <<= makeAny( bValue );
            }
            else if ( CUSTOM_TYPE_DATETIME == nType )
            {
                Date aTmpDate = pLine->m_aDateField.GetDate();
                Time aTmpTime = pLine->m_aTimeField.GetTime();
                util::DateTime aDateTime(aTmpTime.Get100Sec(), aTmpTime.GetSec(), aTmpTime.GetMin(), aTmpTime.GetHour(),
                        aTmpDate.GetDay(), aTmpDate.GetMonth(), aTmpDate.GetYear() );
                aPropertiesSeq[i].Value <<= aDateTime;
            }
            else if ( CUSTOM_TYPE_DURATION == nType )
            {
                aPropertiesSeq[i].Value <<= pLine->m_aDurationField.GetDuration();
            }
            else if ( CUSTOM_TYPE_DATE == nType )
            {
                Date aTmpDate = pLine->m_aDateField.GetDate();
                util::Date aDate(aTmpDate.GetDay(), aTmpDate.GetMonth(), aTmpDate.GetYear());
                aPropertiesSeq[i].Value <<= aDate;

            }
            else
            {
                ::rtl::OUString sValue( pLine->m_aValueEdit.GetText() );
                aPropertiesSeq[i].Value <<= makeAny( sValue );
            }
        }
    }

    return aPropertiesSeq;
}

CustomPropertiesControl::CustomPropertiesControl(Window* pParent)
    : VclVBox(pParent)
    , m_nThumbPos(0)
{
}

void CustomPropertiesControl::Init(VclBuilderContainer& rBuilder)
{
    m_pHeaderBar = new HeaderBar(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER);
    m_pBody = new VclHBox(this);
    OUString sName = rBuilder.get<FixedText>("name")->GetText();
    OUString sType = rBuilder.get<FixedText>("type")->GetText();
    OUString sValue = rBuilder.get<FixedText>("value")->GetText();
    m_pPropertiesWin = new CustomPropertiesWindow(m_pBody, sName, sType, sValue);
    m_pVertScroll = new ScrollBar(m_pBody, WB_VERT);

    set_hexpand(true);
    set_vexpand(true);
    set_expand(true);
    set_fill(true);

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

    const HeaderBarItemBits nHeadBits = HIB_VCENTER | HIB_FIXED | HIB_FIXEDPOS | HIB_LEFT;

    m_pHeaderBar->InsertItem( HI_NAME, sName, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_TYPE, sType, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_VALUE, sValue, 0, nHeadBits );
    m_pHeaderBar->InsertItem( HI_ACTION, OUString(), 0, nHeadBits );
    m_pHeaderBar->Show();

    m_pPropertiesWin->SetRemovedHdl( LINK( this, CustomPropertiesControl, RemovedHdl ) );

    m_pVertScroll->SetRangeMin( 0 );
    m_pVertScroll->SetRangeMax( 0 );
    m_pVertScroll->SetVisibleSize( 0xFFFF );

    Link aScrollLink = LINK( this, CustomPropertiesControl, ScrollHdl );
    m_pVertScroll->SetScrollHdl( aScrollLink );
}

void CustomPropertiesControl::setAllocation(const Size &rAllocation)
{
    VclVBox::setAllocation(rAllocation);

    m_pPropertiesWin->InitControls( m_pHeaderBar, m_pVertScroll );
    sal_Int32 nScrollOffset = m_pPropertiesWin->GetLineHeight();
    sal_Int32 nVisibleEntries = m_pPropertiesWin->GetSizePixel().Height() / nScrollOffset;
    m_pVertScroll->SetPageSize( nVisibleEntries - 1 );
    m_pVertScroll->SetVisibleSize( nVisibleEntries );
    m_pPropertiesWin->updateLineWidth();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeCustomPropertiesControl(Window *pParent,
    VclBuilder::stringmap &)
{
    return new CustomPropertiesControl(pParent);
}

CustomPropertiesControl::~CustomPropertiesControl()
{
    delete m_pVertScroll;
    delete m_pPropertiesWin;
    delete m_pBody;
    delete m_pHeaderBar;
}

IMPL_LINK( CustomPropertiesControl, ScrollHdl, ScrollBar*, pScrollBar )
{
    sal_Int32 nOffset = m_pPropertiesWin->GetLineHeight();
    nOffset *= ( m_nThumbPos - pScrollBar->GetThumbPos() );
    m_nThumbPos = pScrollBar->GetThumbPos();
    m_pPropertiesWin->DoScroll( nOffset );
    return 0;
}

IMPL_LINK_NOARG(CustomPropertiesControl, RemovedHdl)
{
    m_pVertScroll->SetRangeMax( m_pPropertiesWin->GetVisibleLineCount() + 1 );
    if ( m_pPropertiesWin->GetOutputSizePixel().Height() < m_pPropertiesWin->GetVisibleLineCount() * m_pPropertiesWin->GetLineHeight() )
        m_pVertScroll->DoScrollAction ( SCROLL_LINEUP );
    return 0;
}

void CustomPropertiesControl::AddLine( const ::rtl::OUString& sName, Any& rAny, bool bInteractive )
{
    m_pPropertiesWin->AddLine( sName, rAny );
    m_pVertScroll->SetRangeMax( m_pPropertiesWin->GetVisibleLineCount() + 1 );
    if ( bInteractive && m_pPropertiesWin->GetOutputSizePixel().Height() < m_pPropertiesWin->GetVisibleLineCount() * m_pPropertiesWin->GetLineHeight() )
        m_pVertScroll->DoScroll( m_pPropertiesWin->GetVisibleLineCount() + 1 );
}

// class SfxCustomPropertiesPage -----------------------------------------
SfxCustomPropertiesPage::SfxCustomPropertiesPage( Window* pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "CustomInfoPage", "sfx/ui/custominfopage.ui", rItemSet)
{
    get(m_pPropertiesCtrl, "properties");
    m_pPropertiesCtrl->Init(*this);
    get<PushButton>("add")->SetClickHdl(LINK(this, SfxCustomPropertiesPage, AddHdl));
}

IMPL_LINK_NOARG(SfxCustomPropertiesPage, AddHdl)
{
    Any aAny;
    m_pPropertiesCtrl->AddLine( ::rtl::OUString(), aAny, true );
    return 0;
}

sal_Bool SfxCustomPropertiesPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;
    const SfxPoolItem* pItem = NULL;
    SfxDocumentInfoItem* pInfo = NULL;
    bool bMustDelete = false;

    if ( GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        if ( SFX_ITEM_SET !=
                GetTabDialog()->GetExampleSet()->GetItemState( SID_DOCINFO, sal_True, &pItem ) )
            pInfo = &( SfxDocumentInfoItem& )rSet.Get( SID_DOCINFO );
        else
        {
            bMustDelete = true;
            pInfo = new SfxDocumentInfoItem( *( const SfxDocumentInfoItem* ) pItem );
        }
    }

    if ( pInfo )
    {
        pInfo->ClearCustomProperties();
        Sequence< beans::PropertyValue > aPropertySeq = m_pPropertiesCtrl->GetCustomProperties();
        sal_Int32 i = 0, nCount = aPropertySeq.getLength();
        for ( ; i < nCount; ++i )
        {
            if ( !aPropertySeq[i].Name.isEmpty() )
                pInfo->AddCustomProperty( aPropertySeq[i].Name, aPropertySeq[i].Value );
        }
    }

    bModified = sal_True; //!!!
    if ( bModified )
        rSet.Put( *pInfo );
    if ( bMustDelete )
        delete pInfo;
    return bModified;
}

void SfxCustomPropertiesPage::Reset( const SfxItemSet& rItemSet )
{
    m_pPropertiesCtrl->ClearAllLines();
    const SfxDocumentInfoItem* m_pInfoItem = &(const SfxDocumentInfoItem &)rItemSet.Get(SID_DOCINFO);
    std::vector< CustomProperty* > aCustomProps = m_pInfoItem->GetCustomProperties();
    for ( sal_uInt32 i = 0; i < aCustomProps.size(); i++ )
    {
        m_pPropertiesCtrl->AddLine( aCustomProps[i]->m_sName, aCustomProps[i]->m_aValue, false );
    }
}

int SfxCustomPropertiesPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    int nRet = LEAVE_PAGE;
    if ( !m_pPropertiesCtrl->AreAllLinesValid() )
        nRet = KEEP_PAGE;
    return nRet;
}

SfxTabPage* SfxCustomPropertiesPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return new SfxCustomPropertiesPage( pParent, rItemSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
