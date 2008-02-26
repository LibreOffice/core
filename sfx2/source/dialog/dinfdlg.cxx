/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dinfdlg.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:07:36 $
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
#include "precompiled_sfx2.hxx"

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. RET_OK
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <svtools/urihelper.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/datetime.hxx>

#include <memory>

#include <comphelper/string.hxx>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SECURITY_DOCUMENTSIGNATURESINFORMATION_HPP_
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentInfo.hpp>

#include <sfx2/dinfdlg.hxx>
#include "sfxresid.hxx"
#include "dinfedt.hxx"
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
//#include "exptypes.hxx"
#include "helper.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>

#include <sfx2/sfx.hrc>
#include "dinfdlg.hrc"
#include "sfxlocal.hrc"

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// The number of user defined fields handled by the dialog
// There are exactly 4. No more, no less.
#define FOUR 4

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

//------------------------------------------------------------------------
String CreateSizeText( ULONG nSize, BOOL bExtraBytes = TRUE, BOOL bSmartExtraBytes = FALSE );
String CreateSizeText( ULONG nSize, BOOL bExtraBytes, BOOL bSmartExtraBytes )
{
    String aUnitStr = ' ';
    aUnitStr += String( SfxResId(STR_BYTES) );
    ULONG nSize1 = nSize;
    ULONG nSize2 = nSize1;
    ULONG nMega = 1024 * 1024;
    ULONG nGiga = nMega * 1024;
    double fSize = nSize;
    int nDec = 0;
    BOOL bGB = FALSE;

    if ( nSize1 >= 10000 && nSize1 < nMega )
    {
        nSize1 /= 1024;
        aUnitStr = ' ';
        aUnitStr += String( SfxResId(STR_KB) );
        fSize /= 1024;
        nDec = 0;
    }
    else if ( nSize1 >= nMega && nSize1 < nGiga )
    {
        nSize1 /= nMega;
        aUnitStr = ' ';
        aUnitStr += String( SfxResId(STR_MB) );
        fSize /= nMega;
        nDec = 2;
    }
    else if ( nSize1 >= nGiga )
    {
        nSize1 /= nGiga;
        aUnitStr = ' ';
        aUnitStr += String( SfxResId(STR_GB) );
        bGB = TRUE;
        fSize /= nGiga;
        nDec = 3;
    }
    const LocaleDataWrapper& rLocaleWrapper = SvtSysLocale().GetLocaleData();
    String aSizeStr( rLocaleWrapper.getNum( nSize1, 0 ) );
    aSizeStr += aUnitStr;
    if ( bExtraBytes && ( nSize1 < nSize2 ) )
    {
        aSizeStr = ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                rLocaleWrapper.getNumDecimalSep().GetChar(0) );
        aSizeStr += aUnitStr;

        aSizeStr += DEFINE_CONST_UNICODE(" (");
        aSizeStr += rLocaleWrapper.getNum( nSize2, 0 );
        aSizeStr += ' ';
        aSizeStr += String( SfxResId(STR_BYTES) );
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
     aStr += rWrapper.getTime( aT, TRUE, FALSE );
     String aAuthor = rName;
     aAuthor.EraseLeadingChars();
     if ( aAuthor.Len() )
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
    , bHasTemplate( sal_True )
    , bDeleteUserData( sal_False )
    , bIsUseUserData( sal_True )
{
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem( const String& rFile,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Reference<document::XDocumentInfo>& i_xDocInfo,
        sal_Bool bIs )
    : SfxStringItem( SID_DOCINFO, rFile )
    , m_AutoloadDelay( i_xDocProps->getAutoloadSecs() )
    , m_AutoloadURL( i_xDocProps->getAutoloadURL() )
    , m_isAutoloadEnabled( (m_AutoloadDelay > 0) || m_AutoloadURL.getLength() )
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
    , bHasTemplate( TRUE )
    , bDeleteUserData( sal_False )
    , bIsUseUserData( bIs )
{
    for (sal_Int16 i = 0; i < FOUR; ++i) {
        m_UserDefinedFieldTitles[i] = i_xDocInfo->getUserFieldName(i);
        m_UserDefinedFieldValues[i] = i_xDocInfo->getUserFieldValue(i);
    }
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
    , bHasTemplate( rItem.bHasTemplate )
    , bDeleteUserData( rItem.bDeleteUserData )
    , bIsUseUserData( rItem.bIsUseUserData )
{
    for (size_t i = 0; i < FOUR; ++i) {
        m_UserDefinedFieldTitles[i] = rItem.getUserDefinedFieldTitle(i);
        m_UserDefinedFieldValues[i] = rItem.getUserDefinedFieldValue(i);
    }
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::~SfxDocumentInfoItem()
{
}

//------------------------------------------------------------------------

SfxPoolItem* SfxDocumentInfoItem::Clone( SfxItemPool * ) const
{
    return new SfxDocumentInfoItem( *this );
}

//------------------------------------------------------------------------

int SfxDocumentInfoItem::operator==( const SfxPoolItem& rItem) const
{
    if (!(rItem.Type() == Type() && SfxStringItem::operator==(rItem))) {
        return false;
    }
    const SfxDocumentInfoItem& rInfoItem(
        static_cast<const SfxDocumentInfoItem&>(rItem));
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
         std::equal(m_UserDefinedFieldTitles, m_UserDefinedFieldTitles+FOUR,
            rInfoItem.m_UserDefinedFieldTitles) &&
         std::equal(m_UserDefinedFieldValues, m_UserDefinedFieldValues+FOUR,
            rInfoItem.m_UserDefinedFieldValues);
}

//------------------------------------------------------------------------

void SfxDocumentInfoItem::resetUserData(const ::rtl::OUString & i_rAuthor)
{
    setAuthor(i_rAuthor);
    DateTime now;
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

void SfxDocumentInfoItem::updateDocumentInfo(
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Reference<document::XDocumentInfo>& i_xDocInfo) const
{
    if (isAutoloadEnabled()) {
        i_xDocProps->setAutoloadSecs(getAutoloadDelay());
        i_xDocProps->setAutoloadURL(getAutoloadURL());
    } else {
        i_xDocProps->setAutoloadSecs(0);
        i_xDocProps->setAutoloadURL(::rtl::OUString());
    }
    i_xDocProps->setDefaultTarget(getDefaultTarget());
//    i_xDocProps->setTemplateName(getTemplateName());
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
    for (sal_Int16 i = 0; i < FOUR; ++i) {
        i_xDocInfo->setUserFieldName(i, getUserDefinedFieldTitle(i));
        i_xDocInfo->setUserFieldValue(i, getUserDefinedFieldValue(i));
    }

}

//------------------------------------------------------------------------

BOOL SfxDocumentInfoItem::IsDeleteUserData() const
{
     return bDeleteUserData;
}

void SfxDocumentInfoItem::SetDeleteUserData( BOOL bSet )
{
    bDeleteUserData = bSet;
}

BOOL SfxDocumentInfoItem::IsUseUserData() const
{
     return bIsUseUserData;
}

void SfxDocumentInfoItem::SetUseUserData( BOOL bSet )
{
    bIsUseUserData = bSet;
}


::rtl::OUString SfxDocumentInfoItem::getUserDefinedFieldTitle(size_t i_ix) const
{
    DBG_ASSERT(i_ix < FOUR, "SfxDocumentInfoItem: invalid index");
    return m_UserDefinedFieldTitles[i_ix];
}

::rtl::OUString SfxDocumentInfoItem::getUserDefinedFieldValue(size_t i_ix) const
{
    DBG_ASSERT(i_ix < FOUR, "SfxDocumentInfoItem: invalid index");
    return m_UserDefinedFieldValues[i_ix];
}

void SfxDocumentInfoItem::setUserDefinedFieldTitle(size_t i_ix,
        ::rtl::OUString i_val)
{
    DBG_ASSERT(i_ix < FOUR, "SfxDocumentInfoItem: invalid index");
    m_UserDefinedFieldTitles[i_ix] = i_val;
}

void SfxDocumentInfoItem::setUserDefinedFieldValue(size_t i_ix,
        ::rtl::OUString i_val)
{
    DBG_ASSERT(i_ix < FOUR, "SfxDocumentInfoItem: invalid index");
    m_UserDefinedFieldValues[i_ix] = i_val;
}

sal_Bool SfxDocumentInfoItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    String aValue;
    sal_Int32 nValue = 0;
    sal_Bool bValue = sal_False;
    BOOL bField = FALSE;
    BOOL bIsInt = FALSE;
    BOOL bIsString = FALSE;
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
            bIsInt = TRUE;
            nValue = getAutoloadDelay();
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bIsString = TRUE;
            aValue = getAutoloadURL();
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bIsString = TRUE;
            aValue = getDefaultTarget();
            break;
        case MID_DOCINFO_DESCRIPTION:
            bIsString = TRUE;
            aValue = getDescription();
            break;
        case MID_DOCINFO_KEYWORDS:
            bIsString = TRUE;
            aValue = getKeywords();
            break;
        case MID_DOCINFO_SUBJECT:
            bIsString = TRUE;
            aValue = getSubject();
            break;
        case MID_DOCINFO_TITLE:
            bIsString = TRUE;
            aValue = getTitle();
            break;
        case MID_DOCINFO_FIELD1:
        case MID_DOCINFO_FIELD2:
        case MID_DOCINFO_FIELD3:
        case MID_DOCINFO_FIELD4:
            bField = TRUE;
            // no break here
        case MID_DOCINFO_FIELD1TITLE:
        case MID_DOCINFO_FIELD2TITLE:
        case MID_DOCINFO_FIELD3TITLE:
        case MID_DOCINFO_FIELD4TITLE:
        {
            bIsString = TRUE;
            USHORT nSub = MID_DOCINFO_FIELD1TITLE;
            if ( bField )
            {
                nSub = MID_DOCINFO_FIELD1;
            }
            if ( bField )
            {
                DBG_ASSERT( nMemberId == MID_DOCINFO_FIELD1 ||
                            nMemberId == MID_DOCINFO_FIELD2 ||
                            nMemberId == MID_DOCINFO_FIELD3 ||
                            nMemberId == MID_DOCINFO_FIELD4,
                            "SfxDocumentInfoItem:Anpassungsfehler" );
                aValue = getUserDefinedFieldValue( nMemberId - nSub );
            }
            else
            {
                DBG_ASSERT( nMemberId == MID_DOCINFO_FIELD1TITLE ||
                            nMemberId == MID_DOCINFO_FIELD2TITLE ||
                            nMemberId == MID_DOCINFO_FIELD3TITLE ||
                            nMemberId == MID_DOCINFO_FIELD4TITLE,
                            "SfxDocumentInfoItem:Anpassungsfehler" );
                aValue = getUserDefinedFieldTitle( nMemberId - nSub );
            }
            break;
        }
        default:
            DBG_ERROR("Wrong MemberId!");
            return sal_False;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else if ( bIsInt )
        rVal <<= nValue;
    else
        rVal <<= bValue;
    return sal_True;
}

sal_Bool SfxDocumentInfoItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::rtl::OUString aValue;
    sal_Int32 nValue=0;
    sal_Bool bValue = sal_False;
    sal_Bool bRet = sal_False;
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
        case MID_DOCINFO_FIELD1TITLE:
        case MID_DOCINFO_FIELD2TITLE:
        case MID_DOCINFO_FIELD3TITLE:
        case MID_DOCINFO_FIELD4TITLE:
        {
            bRet = (rVal >>= aValue);
            if ( bRet )
                setUserDefinedFieldTitle(
                    nMemberId - MID_DOCINFO_FIELD1TITLE, String(aValue));
            break;
        }
        case MID_DOCINFO_FIELD1:
        case MID_DOCINFO_FIELD2:
        case MID_DOCINFO_FIELD3:
        case MID_DOCINFO_FIELD4:
        {
            bRet = (rVal >>= aValue);
            if ( bRet )
                setUserDefinedFieldValue(
                    nMemberId - MID_DOCINFO_FIELD1, String(aValue));
            break;
        }
        default:
            DBG_ERROR("Wrong MemberId!");
            return sal_False;
    }

    return bRet;
}

//------------------------------------------------------------------------

SfxDocumentDescPage::SfxDocumentDescPage( Window * pParent, const SfxItemSet& rItemSet )  :

    SfxTabPage( pParent, SfxResId( TP_DOCINFODESC ), rItemSet ),

    aTitleFt    ( this, SfxResId( FT_TITLE ) ),
    aTitleEd    ( this, SfxResId( ED_TITLE ) ),
    aThemaFt    ( this, SfxResId( FT_THEMA ) ),
    aThemaEd    ( this, SfxResId( ED_THEMA ) ),
    aKeywordsFt ( this, SfxResId( FT_KEYWORDS ) ),
    aKeywordsEd ( this, SfxResId( ED_KEYWORDS ) ),
    aCommentFt  ( this, SfxResId( FT_COMMENT ) ),
    aCommentEd  ( this, SfxResId( ED_COMMENT ) ),

    pInfoItem   ( NULL )

{
    FreeResource();
}

//------------------------------------------------------------------------

SfxTabPage *SfxDocumentDescPage::Create(Window *pParent, const SfxItemSet &rItemSet)
{
     return new SfxDocumentDescPage(pParent, rItemSet);
}

//------------------------------------------------------------------------

BOOL SfxDocumentDescPage::FillItemSet(SfxItemSet &rSet)
{
    // Pruefung, ob eine Aenderung vorliegt
    const BOOL bTitleMod = aTitleEd.IsModified();
    const BOOL bThemeMod = aThemaEd.IsModified();
    const BOOL bKeywordsMod = aKeywordsEd.IsModified();
    const BOOL bCommentMod = aCommentEd.IsModified();
    if( !( bTitleMod || bThemeMod || bKeywordsMod || bCommentMod ) )
    {
        return FALSE;
    }

    // Erzeugung der Ausgabedaten
    const SfxPoolItem *pItem = NULL;
    SfxDocumentInfoItem *pInfo;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
    {
        pInfo = pInfoItem;
    }
    else
    {
        pInfo = new SfxDocumentInfoItem( *( const SfxDocumentInfoItem *) pItem );
    }
    if( bTitleMod )
    {
        pInfo->setTitle( aTitleEd.GetText() );
    }
    if( bThemeMod )
    {
        pInfo->setSubject( aThemaEd.GetText() );
    }
    if( bKeywordsMod )
    {
        pInfo->setKeywords( aKeywordsEd.GetText() );
    }
    if( bCommentMod )
    {
        pInfo->setDescription( aCommentEd.GetText() );
    }
    rSet.Put( SfxDocumentInfoItem( *pInfo ) );
    if( pInfo != pInfoItem )
    {
        delete pInfo;
    }

    return TRUE;
}

//------------------------------------------------------------------------

void SfxDocumentDescPage::Reset(const SfxItemSet &rSet)
{
     pInfoItem = &(SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);

     aTitleEd.SetText( pInfoItem->getTitle() );
     aThemaEd.SetText( pInfoItem->getSubject() );
     aKeywordsEd.SetText( pInfoItem->getKeywords() );
     aCommentEd.SetText( pInfoItem->getDescription() );

     SFX_ITEMSET_ARG( &rSet, pROItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
     if ( pROItem && pROItem->GetValue() )
     {
        aTitleEd.SetReadOnly( TRUE );
        aThemaEd.SetReadOnly( TRUE );
        aKeywordsEd.SetReadOnly( TRUE );
        aCommentEd.SetReadOnly( TRUE );
    }
}

//------------------------------------------------------------------------

namespace
{
    String GetDateTimeString( sal_Int32 _nDate, sal_Int32 _nTime )
    {
        LocaleDataWrapper aWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );

        Date aDate( _nDate );
        Time aTime( _nTime );
        String aStr( aWrapper.getDate( aDate ) );
        aStr.AppendAscii( ", " );
        aStr += aWrapper.getTime( aTime );
        return aStr;
    }

    // copy from xmlsecurity/source/dialog/resourcemanager.cxx
    String GetContentPart( const String& _rRawString, const String& _rPartId )
    {
        String      s;

        xub_StrLen  nContStart = _rRawString.Search( _rPartId );
        if( nContStart != STRING_NOTFOUND )
        {
            nContStart = nContStart + _rPartId.Len();
            ++nContStart;                   // now it's start of content, directly after Id

            xub_StrLen  nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );

            s = String( _rRawString, nContStart, nContEnd - nContStart );
        }

        return s;
    }

}

SfxDocumentPage::SfxDocumentPage( Window* pParent, const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFODOC ), rItemSet ),

    aBmp1           ( this, SfxResId( BMP_FILE_1 ) ),
    aNameED         ( this, SfxResId( ED_FILE_NAME ) ),

    aLine1FL        ( this, SfxResId( FL_FILE_1 ) ),
    aTypeFT         ( this, SfxResId( FT_FILE_TYP ) ),
    aShowTypeFT     ( this, SfxResId( FT_FILE_SHOW_TYP ) ),
    aReadOnlyCB     ( this, SfxResId( CB_FILE_READONLY ) ),
    aFileFt         ( this, SfxResId( FT_FILE ) ),
    aFileValFt      ( this, SfxResId( FT_FILE_VAL ) ),
    aSizeFT         ( this, SfxResId( FT_FILE_SIZE ) ),
    aShowSizeFT     ( this, SfxResId( FT_FILE_SHOW_SIZE ) ),

    aLine2FL        ( this, SfxResId( FL_FILE_2 ) ),
    aCreateFt       ( this, SfxResId( FT_CREATE ) ),
    aCreateValFt    ( this, SfxResId( FT_CREATE_VAL ) ),
    aChangeFt       ( this, SfxResId( FT_CHANGE ) ),
    aChangeValFt    ( this, SfxResId( FT_CHANGE_VAL ) ),
    aSignedFt       ( this, SfxResId( FT_SIGNED ) ),
    aSignedValFt    ( this, SfxResId( FT_SIGNED_VAL ) ),
    aSignatureBtn   ( this, SfxResId( BTN_SIGNATURE ) ),
    aPrintFt        ( this, SfxResId( FT_PRINT ) ),
    aPrintValFt     ( this, SfxResId( FT_PRINT_VAL ) ),
    aTimeLogFt      ( this, SfxResId( FT_TIMELOG ) ),
    aTimeLogValFt   ( this, SfxResId( FT_TIMELOG_VAL ) ),
    aDocNoFt        ( this, SfxResId( FT_DOCNO ) ),
    aDocNoValFt     ( this, SfxResId( FT_DOCNO_VAL ) ),
    aUseUserDataCB  ( this, SfxResId( CB_USE_USERDATA ) ),
    aDeleteBtn      ( this, SfxResId( BTN_DELETE ) ),

    aLine3FL        ( this, SfxResId( FL_FILE_3 ) ),
    aTemplFt        ( this, SfxResId( FT_TEMPL ) ),
    aTemplValFt     ( this, SfxResId( FT_TEMPL_VAL ) ),

    aUnknownSize    ( SfxResId( STR_UNKNOWNSIZE ) ),
    aMultiSignedStr ( SfxResId( STR_MULTSIGNED ) ),

    bEnableUseUserData  ( FALSE ),
    bHandleDelete       ( FALSE )

{
    FreeResource();

    ImplUpdateSignatures();
    aDeleteBtn.SetClickHdl( LINK( this, SfxDocumentPage, DeleteHdl ) );
    aSignatureBtn.SetClickHdl( LINK( this, SfxDocumentPage, SignatureHdl ) );

    // if the button text is too wide, then broaden it
    const long nOffset = 12;
    String sText = aSignatureBtn.GetText();
    long nTxtW = aSignatureBtn.GetTextWidth( sText );
    if ( sText.Search( '~' ) == STRING_NOTFOUND )
        nTxtW += nOffset;
    long nBtnW = aSignatureBtn.GetSizePixel().Width();
    if ( nTxtW >= nBtnW )
    {
        long nDelta = Max( nTxtW - nBtnW, nOffset/3 );
        Size aNewSize = aSignatureBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        aSignatureBtn.SetSizePixel( aNewSize );
        aDeleteBtn.SetSizePixel( aNewSize );
        // and give them a new position
        Point aNewPos = aSignatureBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        aSignatureBtn.SetPosPixel( aNewPos );
        aNewPos = aDeleteBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        aDeleteBtn.SetPosPixel( aNewPos );

        aNewSize = aSignedValFt.GetSizePixel();
        aNewSize.Width() -= nDelta;
        aSignedValFt.SetSizePixel( aNewSize );
        aNewSize = aUseUserDataCB.GetSizePixel();
        aNewSize.Width() -= nDelta;
        aUseUserDataCB.SetSizePixel( aNewSize );
    }
}

//------------------------------------------------------------------------

IMPL_LINK( SfxDocumentPage, DeleteHdl, PushButton*, EMPTYARG )
{
    String aName;
    if ( bEnableUseUserData && aUseUserDataCB.IsChecked() )
        aName = SvtUserOptions().GetFullName();
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    DateTime now;
    util::DateTime uDT(
        now.Get100Sec(), now.GetSec(), now.GetMin(), now.GetHour(),
        now.GetDay(), now.GetMonth(), now.GetYear() );
    aCreateValFt.SetText( ConvertDateTime_Impl( aName, uDT, aLocaleWrapper ) );
    XubString aEmpty;
    aChangeValFt.SetText( aEmpty );
    aPrintValFt.SetText( aEmpty );
    const Time aTime( 0 );
    aTimeLogValFt.SetText( aLocaleWrapper.getDuration( aTime ) );
    aDocNoValFt.SetText( '1' );
    bHandleDelete = TRUE;
    return 0;
}

IMPL_LINK( SfxDocumentPage, SignatureHdl, PushButton*, EMPTYARG )
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if( pDoc )
    {
        pDoc->SignDocumentContent();

        ImplUpdateSignatures();
    }

    return 0;
}

void SfxDocumentPage::ImplUpdateSignatures()
{
    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if( pDoc )
    {
        SfxMedium* pMedium = pDoc->GetMedium();
        if ( pMedium && pMedium->GetName().Len() && pMedium->GetStorage().is() )
        {
            Reference< security::XDocumentDigitalSignatures > xD(
                comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), uno::UNO_QUERY );

            if( xD.is() )
            {
                String s;
                Sequence< security::DocumentSignatureInformation > aInfos;
                aInfos = xD->verifyDocumentContentSignatures( pMedium->GetLastCommitReadStorage_Impl(),
                                                                uno::Reference< io::XInputStream >() );
                if( aInfos.getLength() > 1 )
                {
                    s = aMultiSignedStr;
                }
                else if( aInfos.getLength() == 1 )
                {
                    String aCN_Id( String::CreateFromAscii( "CN" ) );
                    const security::DocumentSignatureInformation& rInfo = aInfos[ 0 ];
                    s = GetDateTimeString( rInfo.SignatureDate, rInfo.SignatureTime );
                    s.AppendAscii( ", " );
                    s += GetContentPart( rInfo.Signer->getSubjectName(), aCN_Id );
                }
                aSignedValFt.SetText( s );
            }
        }
    }
}

//------------------------------------------------------------------------

SfxTabPage* SfxDocumentPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
     return new SfxDocumentPage( pParent, rItemSet );
}

//------------------------------------------------------------------------

void SfxDocumentPage::EnableUseUserData()
{
    bEnableUseUserData = TRUE;
    aUseUserDataCB.Show();
    aDeleteBtn.Show();
}

//------------------------------------------------------------------------

BOOL SfxDocumentPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;

    if ( !bHandleDelete && bEnableUseUserData &&
         aUseUserDataCB.GetState() != aUseUserDataCB.GetSavedValue() &&
         GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            SfxDocumentInfoItem* pInfoItem = (SfxDocumentInfoItem*)pItem;
            BOOL bUseData = ( STATE_CHECK == aUseUserDataCB.GetState() );
            pInfoItem->SetUseUserData( bUseData );
/*
            if ( !bUseData )
            {
                // "Benutzerdaten verwenden" ausgeschaltet ->
                // den Benutzer aus den Stamps l"oschen
                String aEmptyUser;
                aInfo.SetCreated(
                    SfxStamp( aEmptyUser, aInfo.GetCreated().GetTime() ) );
                aInfo.SetChanged(
                    SfxStamp( aEmptyUser, aInfo.GetChanged().GetTime() ) );
                aInfo.SetPrinted(
                    SfxStamp( aEmptyUser, aInfo.GetPrinted().GetTime() ) );
            }
*/
            rSet.Put( SfxDocumentInfoItem( *pInfoItem ) );
            bRet = TRUE;
        }
    }

    if ( bHandleDelete )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            SfxDocumentInfoItem* pInfoItem = (SfxDocumentInfoItem*)pItem;
            BOOL bUseAuthor = bEnableUseUserData && aUseUserDataCB.IsChecked();
            SfxDocumentInfoItem newItem( *pInfoItem );
            newItem.resetUserData( bUseAuthor
                ? SvtUserOptions().GetFullName()
                : String() );
            pInfoItem->SetUseUserData( STATE_CHECK == aUseUserDataCB.GetState() );
            newItem.SetUseUserData( STATE_CHECK == aUseUserDataCB.GetState() );

            newItem.SetDeleteUserData( TRUE );
            rSet.Put( newItem );
            bRet = TRUE;
        }
    }

    if ( aNameED.IsModified() && aNameED.GetText().Len() )
    {
        rSet.Put( SfxStringItem( ID_FILETP_TITLE, aNameED.GetText() ) );
        bRet = TRUE;
    }

    if ( /* aReadOnlyCB.IsModified() */ TRUE )
    {
        rSet.Put( SfxBoolItem( ID_FILETP_READONLY, aReadOnlyCB.IsChecked() ) );
        bRet = TRUE;
    }

    return bRet;
}

//------------------------------------------------------------------------

void SfxDocumentPage::Reset( const SfxItemSet& rSet )
{
    // Bestimmung der Dokumentinformationen
    const SfxDocumentInfoItem *pInfoItem =
        &(const SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);

    // template data
    if ( pInfoItem->HasTemplate() )
    {
        aTemplValFt.SetText( pInfoItem->getTemplateName() );
    }
    else
    {
        aTemplFt.Hide();
        aTemplValFt.Hide();
    }

    // determine file name
    String aFile( pInfoItem->GetValue() );
    String aFactory( aFile );
    if ( aFile.Len() > 2 && aFile.GetChar(0) == '[' )
    {
        USHORT nPos = aFile.Search( ']' );
        aFactory = aFile.Copy( 1, nPos-1  );
        aFile = aFile.Copy( nPos+1 );
    }

    // determine name
    String aName;
    const SfxPoolItem* pItem = 0;
    if ( SFX_ITEM_SET != rSet.GetItemState( ID_FILETP_TITLE, FALSE, &pItem ) )
    {
        INetURLObject aURL(aFile);
        aName = aURL.GetName( INetURLObject::DECODE_WITH_CHARSET );
        if ( !aName.Len() || aURL.GetProtocol() == INET_PROT_PRIVATE )
            aName = String( SfxResId( STR_NONAME ) );
        aNameED.SetReadOnly( TRUE );
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ), "SfxDocumentPage:<SfxStringItem> erwartet" );
        aName = ( ( SfxStringItem* ) pItem )->GetValue();
    }
    aNameED.SetText( aName );
    aNameED.ClearModifyFlag();

    // determine RO-Flag
    if ( SFX_ITEM_UNKNOWN == rSet.GetItemState( ID_FILETP_READONLY, FALSE, &pItem )
         || !pItem )
        aReadOnlyCB.Hide();
    else
        aReadOnlyCB.Check( ( (SfxBoolItem*)pItem )->GetValue() );

    // determine context symbol
    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetSmartURL( aFactory);
    const String& rMainURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
    aBmp1.SetImage( SvFileInformationManager::GetImage( aURL, TRUE ) );

    // determine size and type
    String aSizeText( aUnknownSize );
    if ( aURL.GetProtocol() == INET_PROT_FILE )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
    aShowSizeFT.SetText( aSizeText );

    String aDescription = SvFileInformationManager::GetDescription( INetURLObject(rMainURL) );
    if ( aDescription.Len() == 0 )
        aDescription = String( SfxResId( STR_SFX_NEWOFFICEDOC ) );
    aShowTypeFT.SetText( aDescription );

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
        aFileValFt.SetText( aText );
    }
    else if ( aURL.GetProtocol() != INET_PROT_PRIVATE )
        aFileValFt.SetText( aURL.GetPartBeforeLastName() );

    // handle access data
    BOOL bIsUseUserData = pInfoItem->IsUseUserData();
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    aCreateValFt.SetText( ConvertDateTime_Impl( pInfoItem->getAuthor(),
        pInfoItem->getCreationDate(), aLocaleWrapper ) );
    util::DateTime aTime( pInfoItem->getModificationDate() );
//  if ( aTime.IsValid() )
    if ( aTime.Month > 0 )
        aChangeValFt.SetText( ConvertDateTime_Impl(
            pInfoItem->getModifiedBy(), aTime, aLocaleWrapper ) );
    aTime = pInfoItem->getPrintDate();
//  if ( aTime.IsValid())
    if ( aTime.Month > 0 )
        aPrintValFt.SetText( ConvertDateTime_Impl( pInfoItem->getPrintedBy(),
            aTime, aLocaleWrapper ) );
    const long nTime = pInfoItem->getEditingDuration();
    if( bIsUseUserData )
    {
        const Time aT( nTime/3600, (nTime%3600)/60, nTime%60 );
        aTimeLogValFt.SetText( aLocaleWrapper.getDuration( aT ) );
        aDocNoValFt.SetText( String::CreateFromInt32(
            pInfoItem->getEditingCycles() ) );
    }

    TriState eState = (TriState)bIsUseUserData;

    if ( STATE_DONTKNOW == eState )
        aUseUserDataCB.EnableTriState( TRUE );

    aUseUserDataCB.SetState( eState );
    aUseUserDataCB.SaveValue();
    aUseUserDataCB.Enable( bEnableUseUserData );
    bHandleDelete = FALSE;
    aDeleteBtn.Enable( bEnableUseUserData );
}

//------------------------------------------------------------------------

SfxInternetPage::SfxInternetPage( Window* pParent, const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFORELOAD ), rItemSet ),

    aRBNoAutoUpdate     ( this, SfxResId( RB_NOAUTOUPDATE       ) ),

    aRBReloadUpdate     ( this, SfxResId( RB_RELOADUPDATE       ) ),

    aRBForwardUpdate    ( this, SfxResId( RB_FORWARDUPDATE      ) ),
    aFTEvery            ( this, SfxResId( FT_EVERY              ) ),
    aNFReload           ( this, SfxResId( ED_RELOAD         ) ),
    aFTReloadSeconds    ( this, SfxResId( FT_RELOADSECS     ) ),
    aFTAfter            ( this, SfxResId( FT_AFTER              ) ),
    aNFAfter            ( this, SfxResId( ED_FORWARD            ) ),
    aFTAfterSeconds     ( this, SfxResId( FT_FORWARDSECS        ) ),
    aFTURL              ( this, SfxResId( FT_URL                ) ),
    aEDForwardURL       ( this, SfxResId( ED_URL                ) ),
    aPBBrowseURL        ( this, SfxResId( PB_BROWSEURL          ) ),
    aFTFrame            ( this, SfxResId( FT_FRAME              ) ),
    aCBFrame            ( this, SfxResId( CB_FRAME              ) ),

    aForwardErrorMessg  (       SfxResId( STR_FORWARD_ERRMSSG   ) ),
    pInfoItem           ( NULL ),
    pFileDlg            ( NULL ),
    eState              ( S_Init )

{
    FreeResource();
    pInfoItem = &( SfxDocumentInfoItem& ) rItemSet.Get( SID_DOCINFO );
    TargetList          aList;
    SfxViewFrame*       pFrame = SfxViewFrame::Current();
    if( pFrame )
    {
        pFrame = pFrame->GetTopViewFrame();
        if( pFrame )
        {
            pFrame->GetTargetList( aList );

            String*         pObj;
            for( USHORT nPos = ( USHORT ) aList.Count() ; nPos ; )
            {
                pObj = aList.GetObject( --nPos );
                aCBFrame.InsertEntry( *pObj );
                delete pObj;
            }
        }
    }

    aRBNoAutoUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlNoUpdate ) );
    aRBReloadUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlReload ) );
    aRBForwardUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlForward ) );
    aPBBrowseURL.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlBrowseURL ) );

    aForwardErrorMessg.SearchAndReplaceAscii( "%PLACEHOLDER%", aRBForwardUpdate.GetText() );

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

    if( eState == eNewState  )
        return;

    switch( eState )
    {
        case S_Init:
            EnableNoUpdate( TRUE );
            EnableReload( FALSE );
            EnableForward( FALSE );
            break;
        case S_NoUpdate:
            EnableNoUpdate( FALSE );
            if( eNewState == S_Reload )
                EnableReload( TRUE );
            else
                EnableForward( TRUE );
            break;
        case S_Reload:
            EnableReload( FALSE );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( TRUE );
            else
                EnableForward( TRUE );
            break;
        case S_Forward:
            EnableForward( FALSE );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( TRUE );
            else
                EnableReload( TRUE );
            break;
        default:
            DBG_ERROR( "*SfxInternetPage::SetState(): unhandled state!" );
    }

    eState = eNewState;
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableNoUpdate( BOOL bEnable )
{
    if( bEnable )
        aRBNoAutoUpdate.Check();
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableReload( BOOL bEnable )
{
    aFTEvery.Enable( bEnable );
    aNFReload.Enable( bEnable );
    aFTReloadSeconds.Enable( bEnable );

    if( bEnable )
        aRBReloadUpdate.Check();
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableForward( BOOL bEnable )
{
    aFTAfter.Enable( bEnable );
    aNFAfter.Enable( bEnable );
    aFTAfterSeconds.Enable( bEnable );
    aFTURL.Enable( bEnable );
    aEDForwardURL.Enable( bEnable );
    aPBBrowseURL.Enable( bEnable );
    aFTFrame.Enable( bEnable );
    aCBFrame.Enable( bEnable );

    if( bEnable )
        aRBForwardUpdate.Check();
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

IMPL_LINK( SfxInternetPage, ClickHdlBrowseURL, PushButton*, EMPTYARG )
{
    if ( !pFileDlg )
        pFileDlg = new sfx2::FileDialogHelper( TemplateDescription::FILEOPEN_SIMPLE, WB_OPEN );
    pFileDlg->SetDisplayDirectory( aEDForwardURL.GetText() );
    pFileDlg->StartExecuteModal( LINK( this, SfxInternetPage, DialogClosedHdl ) );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, DialogClosedHdl, sfx2::FileDialogHelper*, EMPTYARG )
{
    DBG_ASSERT( pFileDlg, "SfxInternetPage::DialogClosedHdl(): no file dialog" );

    if ( ERRCODE_NONE == pFileDlg->GetError() )
        aEDForwardURL.SetText( pFileDlg->GetPath() );

    return 0;
}

//------------------------------------------------------------------------

BOOL SfxInternetPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem*          pItem = NULL;
    SfxDocumentInfoItem*        pInfo = NULL;
    SfxTabDialog*               pDlg = GetTabDialog();
    const SfxItemSet*           pExSet = NULL;

    if( pDlg )
        pExSet = pDlg->GetExampleSet();

    if( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        pInfo = pInfoItem;
    else
        pInfo = new SfxDocumentInfoItem( *( const SfxDocumentInfoItem* ) pItem );

    DBG_ASSERT( eState != S_Init, "*SfxInternetPage::FillItemSet(): state init is not acceptable at this point!" );

    BOOL                        bEnableReload = FALSE;
    ::std::auto_ptr< String >   aURL( NULL );
    ::std::auto_ptr< String >   aFrame( NULL );
    ULONG                       nDelay = 0;

    switch( eState )
    {
        case S_NoUpdate:
            break;
        case S_Reload:
            bEnableReload = TRUE;
            aURL = ::std::auto_ptr< String >( new String() );
            aFrame = ::std::auto_ptr< String >( new String() );
            nDelay = static_cast<ULONG>(aNFReload.GetValue());
            break;
        case S_Forward:
            DBG_ASSERT( aEDForwardURL.GetText().Len(), "+SfxInternetPage::FillItemSet(): empty URL should be not possible for forward option!" );

            bEnableReload = TRUE;
            aURL = ::std::auto_ptr< String >( new String( URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), aEDForwardURL.GetText(), URIHelper::GetMaybeFileHdl(), true ) ) );
            aFrame = ::std::auto_ptr< String >( new String( aCBFrame.GetText() ) );
            nDelay = static_cast<ULONG>(aNFAfter.GetValue());
            break;
              default:
                  break;
    }

    pInfo->setAutoloadEnabled( bEnableReload );

    if( bEnableReload )
    {
        pInfo->setAutoloadURL( *aURL.get() );
        pInfo->setDefaultTarget( *aFrame.get() );
        pInfo->setAutoloadDelay( nDelay );
    }

    rSet.Put( *pInfo );
    if( pInfo != pInfoItem )
        delete pInfo;
    return TRUE;
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
    SFX_ITEMSET_ARG( &rSet, pURLItem, SfxStringItem, SID_BASEURL, FALSE );
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
    if ( pURLItem )
        aBaseURL = pURLItem->GetValue();

    STATE               eNewState = S_NoUpdate;

    if( pInfoItem->isAutoloadEnabled() )
    {
        const String&   rURL = pInfoItem->getAutoloadURL();

        if( rURL.Len() )
        {
            aNFAfter.SetValue( pInfoItem->getAutoloadDelay() );
            aEDForwardURL.SetText( rURL );
            aCBFrame.SetText( pInfoItem->getDefaultTarget() );
            eNewState = S_Forward;
        }
        else
        {
            aNFReload.SetValue( pInfoItem->getAutoloadDelay() );
            eNewState = S_Reload;
        }
    }

    ChangeState( eNewState );

    // #102907# ------------------------
    SFX_ITEMSET_ARG( &rSet, pROItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
    if ( pROItem && pROItem->GetValue() )
    {
        aRBNoAutoUpdate.Disable();
        aRBReloadUpdate.Disable();
        aRBForwardUpdate.Disable();
        aNFReload.Disable();
        aNFAfter.Disable();
        aEDForwardURL.Disable();
        aPBBrowseURL.Disable();
        aCBFrame.Disable();
        aFTEvery.Disable();
        aFTReloadSeconds.Disable();
        aFTAfter.Disable();
        aFTAfterSeconds.Disable();
        aFTURL.Disable();
        aFTFrame.Disable();
    }
}

//------------------------------------------------------------------------

int SfxInternetPage::DeactivatePage( SfxItemSet* /*pSet*/ )
{
    int             nRet = LEAVE_PAGE;

    if( eState == S_Forward && !aEDForwardURL.GetText().Len() )
    {
        ErrorBox    aErrBox( this, WB_OK, aForwardErrorMessg );
        aErrBox.Execute();

        nRet = KEEP_PAGE;
    }

    return nRet;
}

//------------------------------------------------------------------------

SfxDocumentUserPage::SfxDocumentUserPage( Window* pParent,
                                          const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFOUSER ), rItemSet ),

    bLabelModified  ( FALSE ),
    aInfo1Ft        ( this, SfxResId( FT_INFO1 ) ),
    aInfo1Ed        ( this, SfxResId( ED_INFO1 ) ),
    aInfo2Ft        ( this, SfxResId( FT_INFO2 ) ),
    aInfo2Ed        ( this, SfxResId( ED_INFO2 ) ),
    aInfo3Ft        ( this, SfxResId( FT_INFO3 ) ),
    aInfo3Ed        ( this, SfxResId( ED_INFO3 ) ),
    aInfo4Ft        ( this, SfxResId( FT_INFO4 ) ),
    aInfo4Ed        ( this, SfxResId( ED_INFO4 ) ),
    aEditLabelBtn   ( this, SfxResId( BTN_EDITLABEL ) ),

    pInfoItem       ( NULL )

{
    FreeResource();
    //increase button width in case of long labels
    Size aButtonSize = aEditLabelBtn.GetOutputSizePixel();
    sal_Int32 nTextWidth = aEditLabelBtn.GetTextWidth(aEditLabelBtn.GetText());
    //add some additional space
    sal_Int32 nDiff = nTextWidth + 4 - aButtonSize.Width();
    if( nDiff > 0)
    {
        Point aPos(aEditLabelBtn.GetPosPixel());
        aPos.X() -= nDiff;
        aButtonSize.Width() += nDiff;
        aEditLabelBtn.SetPosSizePixel(aPos, aButtonSize);
    }

    aEditLabelBtn.SetClickHdl( LINK( this, SfxDocumentUserPage, EditLabelHdl ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SfxDocumentUserPage, EditLabelHdl, PushButton *, pPushButton )
{
    (void)pPushButton; //unused
    SfxDocInfoEditDlg* pDlg = new SfxDocInfoEditDlg( this );
    pDlg->SetText1( GetLabelText_Impl( &aInfo1Ft ) );
    pDlg->SetText2( GetLabelText_Impl( &aInfo2Ft ) );
    pDlg->SetText3( GetLabelText_Impl( &aInfo3Ft ) );
    pDlg->SetText4( GetLabelText_Impl( &aInfo4Ft ) );

    if ( RET_OK == pDlg->Execute() )
    {
        SetLabelText_Impl( &aInfo1Ft, pDlg->GetText1() );
        SetLabelText_Impl( &aInfo2Ft, pDlg->GetText2() );
        SetLabelText_Impl( &aInfo3Ft, pDlg->GetText3() );
        SetLabelText_Impl( &aInfo4Ft, pDlg->GetText4() );
        bLabelModified = TRUE;
    }
    delete pDlg;
    return 0;
}

//------------------------------------------------------------------------

String SfxDocumentUserPage::GetLabelText_Impl( FixedText* pLabel )
{
    DBG_ASSERT( pLabel, "SfxDocumentUserPage::SetLabelText_Impl(): invalid label" );
    String aLabel = pLabel->GetText();
    aLabel.Erase( 0, aLabel.Search( ' ' ) + 1 );
    return aLabel;
}

//------------------------------------------------------------------------

void SfxDocumentUserPage::SetLabelText_Impl( FixedText* pLabel, const String& rNewLabel )
{
    String aLabel( '~' );
    sal_Int32 nNumber = 0;
    if ( &aInfo1Ft == pLabel )
        nNumber = 1;
    else if ( &aInfo2Ft == pLabel )
        nNumber = 2;
    else if ( &aInfo3Ft == pLabel )
        nNumber = 3;
    else if ( &aInfo4Ft == pLabel )
        nNumber = 4;
    DBG_ASSERT( nNumber > 0, "SfxDocumentUserPage::SetLabelText_Impl(): wrong label" );
    aLabel += String::CreateFromInt32( nNumber );
    aLabel += String( DEFINE_CONST_UNICODE(": ") );
    aLabel += rNewLabel;
    DBG_ASSERT( pLabel, "SfxDocumentUserPage::SetLabelText_Impl(): invalid label" );
    pLabel->SetText( aLabel );
}

//------------------------------------------------------------------------

SfxTabPage* SfxDocumentUserPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return new SfxDocumentUserPage(pParent, rItemSet);
}

//------------------------------------------------------------------------

BOOL SfxDocumentUserPage::FillItemSet( SfxItemSet& rSet )
{
    const BOOL bMod = bLabelModified ||
                      aInfo1Ed.IsModified() || aInfo2Ed.IsModified() ||
                      aInfo3Ed.IsModified() || aInfo4Ed.IsModified();
    if ( !bMod )
        return FALSE;

    const SfxPoolItem* pItem = 0;
    SfxDocumentInfoItem* pInfo = 0;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet &&
         SFX_ITEM_SET != pExSet->GetItemState(SID_DOCINFO, TRUE, &pItem) )
        pInfo = pInfoItem;
    else
        pInfo = new SfxDocumentInfoItem( *(const SfxDocumentInfoItem*)pItem );

    if ( bLabelModified || aInfo1Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo1Ft );
        pInfo->setUserDefinedFieldTitle( 0, aTitle );
        pInfo->setUserDefinedFieldValue( 0, aInfo1Ed.GetText() );
    }
    if ( bLabelModified || aInfo2Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo2Ft );
        pInfo->setUserDefinedFieldTitle( 1, aTitle );
        pInfo->setUserDefinedFieldValue( 1, aInfo2Ed.GetText() );
    }
    if ( bLabelModified || aInfo3Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo3Ft );
        pInfo->setUserDefinedFieldTitle( 2, aTitle );
        pInfo->setUserDefinedFieldValue( 2, aInfo3Ed.GetText() );
    }
    if ( bLabelModified || aInfo4Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo4Ft );
        pInfo->setUserDefinedFieldTitle( 3, aTitle );
        pInfo->setUserDefinedFieldValue( 3, aInfo4Ed.GetText() );
    }
    rSet.Put( *pInfo );
    if ( pInfo != pInfoItem )
        delete pInfo;
    return bMod;
}

//------------------------------------------------------------------------

void SfxDocumentUserPage::Reset(const SfxItemSet &rSet)
{
    pInfoItem = &(SfxDocumentInfoItem&)rSet.Get( SID_DOCINFO );

    SetLabelText_Impl( &aInfo1Ft, pInfoItem->getUserDefinedFieldTitle(0) );
    aInfo1Ed.SetText( pInfoItem->getUserDefinedFieldValue(0) );
    SetLabelText_Impl( &aInfo2Ft, pInfoItem->getUserDefinedFieldTitle(1) );
    aInfo2Ed.SetText( pInfoItem->getUserDefinedFieldValue(1) );
    SetLabelText_Impl( &aInfo3Ft, pInfoItem->getUserDefinedFieldTitle(2) );
    aInfo3Ed.SetText( pInfoItem->getUserDefinedFieldValue(2) );
    SetLabelText_Impl( &aInfo4Ft, pInfoItem->getUserDefinedFieldTitle(3) );
    aInfo4Ed.SetText( pInfoItem->getUserDefinedFieldValue(3) );
    bLabelModified = FALSE;

    SFX_ITEMSET_ARG( &rSet, pROItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
    if ( pROItem && pROItem->GetValue() )
    {
        aInfo1Ed.SetReadOnly( TRUE );
        aInfo2Ed.SetReadOnly( TRUE );
        aInfo3Ed.SetReadOnly( TRUE );
        aInfo4Ed.SetReadOnly( TRUE );
        aEditLabelBtn.Disable();
    }
}

//------------------------------------------------------------------------

SfxDocumentInfoDialog::SfxDocumentInfoDialog( Window* pParent,
                                              const SfxItemSet& rItemSet ) :

    SfxTabDialog( 0, pParent, SfxResId( SID_DOCINFO ), &rItemSet )

{
    FreeResource();

     const SfxDocumentInfoItem* pInfoItem =
        &(const SfxDocumentInfoItem &)rItemSet.Get( SID_DOCINFO );

#ifdef DBG_UTIL
    SFX_ITEMSET_ARG( &rItemSet, pURLItem, SfxStringItem, SID_BASEURL, FALSE );
    DBG_ASSERT( pURLItem, "No BaseURL provided for InternetTabPage!" );
#endif

     // Bestimmung des Titels
    const SfxPoolItem* pItem = 0;
    String aTitle( GetText() );
    if ( SFX_ITEM_SET !=
         rItemSet.GetItemState( SID_EXPLORER_PROPS_START, FALSE, &pItem ) )
    {
        // Dateiname
        String aFile( pInfoItem->GetValue() );
#ifdef WIN
        if ( aFile.Len() <= 8 )
        {
            String sTmp( SfxResId( STR_NONAME ) );
            USHORT nLen = Min( (USHORT)8, sTmp.Len() );

            if ( sTmp.Copy( 0, nLen ).Lower() ==
                 aFile.Copy( 0, nLen ).Lower() )
            {
                aFile = pInfoItem->GetValue();
            }
        }
#endif

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
            aTitle += String( SfxResId( STR_NONAME ) );
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ),
                    "SfxDocumentInfoDialog:<SfxStringItem> erwartet" );
        aTitle += ( ( SfxStringItem* ) pItem )->GetValue();
    }
    SetText( aTitle );

    // Eigenschaftenseiten
    AddTabPage(TP_DOCINFODESC, SfxDocumentDescPage::Create, 0);
    AddTabPage(TP_DOCINFODOC, SfxDocumentPage::Create, 0);
    AddTabPage(TP_DOCINFOUSER, SfxDocumentUserPage::Create, 0);
    AddTabPage(TP_DOCINFORELOAD, SfxInternetPage::Create, 0);
}

// -----------------------------------------------------------------------

void SfxDocumentInfoDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    if ( TP_DOCINFODOC == nId )
        ( (SfxDocumentPage&)rPage ).EnableUseUserData();
}
