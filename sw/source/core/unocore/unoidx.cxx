/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoidx.cxx,v $
 * $Revision: 1.68 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/unolingu.hxx>

#include <hints.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <shellres.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#include <pagedesc.hxx>
#include <fmtcntnt.hxx>
#include <unomap.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unosection.hxx>
#include <doctxm.hxx>
#include <txttxmrk.hxx>
#include <unocrsr.hxx>
#include <unostyle.hxx>
#include <ndtxt.hxx>
#include <unoidx.hxx>
#include <docsh.hxx>
#include <chpfld.hxx>
#include <SwStyleNameMapper.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;

//-----------------------------------------------------------------------------
static String
lcl_AnyToString(uno::Any const& rVal) throw (lang::IllegalArgumentException)
{
    OUString sRet;
    if(!(rVal >>= sRet))
    {
        throw lang::IllegalArgumentException();
    }
    return sRet;
}
//-----------------------------------------------------------------------------
static sal_Int16
lcl_AnyToInt16(uno::Any const& rVal) throw (lang::IllegalArgumentException)
{
    sal_Int16 nRet = 0;
    if(!(rVal >>= nRet))
    {
        throw lang::IllegalArgumentException();
    }
    return nRet;
}
//-----------------------------------------------------------------------------
static sal_Bool
lcl_AnyToBool(uno::Any const& rVal) throw (lang::IllegalArgumentException)
{
    sal_Bool bRet = sal_False;
    if(!(rVal >>= bRet))
    {
        throw lang::IllegalArgumentException();
    }
    return bRet;
}

static void
lcl_AnyToBitMask(uno::Any const& rValue,
        sal_uInt16 & rBitMask, const sal_uInt16 nBit)
throw (lang::IllegalArgumentException)
{
    rBitMask = lcl_AnyToBool(rValue)
        ? (rBitMask |  nBit)
        : (rBitMask & ~nBit);
}
static void
lcl_BitMaskToAny(uno::Any & o_rValue,
        const sal_uInt16 nBitMask, const sal_uInt16 nBit)
{
    const sal_Bool bRet = 0 != (nBitMask & nBit);
    o_rValue <<= bRet;
}

//-----------------------------------------------------------------------------
static void
lcl_ReAssignTOXType(SwDoc* pDoc, SwTOXBase& rTOXBase, const OUString& rNewName)
{
    const sal_uInt16 nUserCount = pDoc->GetTOXTypeCount( TOX_USER );
    const SwTOXType* pNewType = 0;
    for(sal_uInt16 nUser = 0; nUser < nUserCount; nUser++)
    {
        const SwTOXType* pType = pDoc->GetTOXType( TOX_USER, nUser );
        if(pType->GetTypeName().Equals((String)rNewName))
        {
            pNewType = pType;
            break;
        }
    }
    if(!pNewType)
    {
        SwTOXType aNewType(TOX_USER, rNewName);
        pNewType = pDoc->InsertTOXType( aNewType );
    }
    //has to be non-const-casted
    ((SwTOXType*)pNewType)->Add(&rTOXBase);
}
//-----------------------------------------------------------------------------
static const char cUserDefined[] = "User-Defined";
static const char cUserSuffix[] = " (user)";
#define USER_LEN 12
#define USER_AND_SUFFIXLEN 19

void lcl_ConvertTOUNameToProgrammaticName(OUString& rTmp)
{
    ShellResource* pShellRes = ViewShell::GetShellRes();

    if(rTmp.equals(pShellRes->aTOXUserName))
    {
        rTmp = OUString(C2U(cUserDefined));
    }
    // if the version is not English but the alternative index's name is
    // "User-Defined" a " (user)" is appended
    else if(rTmp.equalsAscii(cUserDefined))
    {
        rTmp += C2U(cUserSuffix);
    }
}
//-----------------------------------------------------------------------------
static void
lcl_ConvertTOUNameToUserName(OUString& rTmp)
{
    ShellResource* pShellRes = ViewShell::GetShellRes();
    if(rTmp.equalsAscii(cUserDefined))
    {
        rTmp = pShellRes->aTOXUserName;
    }
    else if(!pShellRes->aTOXUserName.EqualsAscii(cUserDefined) &&
        USER_AND_SUFFIXLEN == rTmp.getLength())
    {
        //make sure that in non-English versions the " (user)" suffix is removed
        if (rTmp.matchAsciiL(cUserDefined, sizeof(cUserDefined)) &&
            rTmp.matchAsciiL(cUserSuffix, sizeof(cUserSuffix), USER_LEN))
        {
            rTmp = C2U(cUserDefined);
        }
    }
}

/******************************************************************
 * SwXDocumentIndex
 ******************************************************************/

/* -----------------20.06.98 11:06-------------------
 *
 * --------------------------------------------------*/
class SwDocIndexDescriptorProperties_Impl
{
private:
    ::std::auto_ptr<SwTOXBase> m_pTOXBase;
    OUString m_sUserTOXTypeName;

public:
    SwDocIndexDescriptorProperties_Impl(SwTOXType const*const pType);

    SwTOXBase &     GetTOXBase() { return *m_pTOXBase; }
    const OUString& GetTypeName() const { return m_sUserTOXTypeName; }
    void  SetTypeName(const OUString& rSet) { m_sUserTOXTypeName = rSet; }
};
/* -----------------20.06.98 11:41-------------------
 *
 * --------------------------------------------------*/
SwDocIndexDescriptorProperties_Impl::SwDocIndexDescriptorProperties_Impl(
        SwTOXType const*const pType)
{
    SwForm aForm(pType->GetType());
    m_pTOXBase.reset(new SwTOXBase(pType, aForm,
                             nsSwTOXElement::TOX_MARK, pType->GetTypeName()));
    if(pType->GetType() == TOX_CONTENT || pType->GetType() == TOX_USER)
    {
        m_pTOXBase->SetLevel(MAXLEVEL);
    }
    m_sUserTOXTypeName = pType->GetTypeName();
}

static sal_uInt16
lcl_TypeToPropertyMap(const TOXTypes eType)
{
    switch (eType)
    {
        case TOX_INDEX:         return PROPERTY_MAP_INDEX_IDX;
        case TOX_CONTENT:       return PROPERTY_MAP_INDEX_CNTNT;
        case TOX_TABLES:        return PROPERTY_MAP_INDEX_TABLES;
        case TOX_ILLUSTRATIONS: return PROPERTY_MAP_INDEX_ILLUSTRATIONS;
        case TOX_OBJECTS:       return PROPERTY_MAP_INDEX_OBJECTS;
        case TOX_AUTHORITIES:   return PROPERTY_MAP_BIBLIOGRAPHY;
        //case TOX_USER:
        default:
            return PROPERTY_MAP_INDEX_USER;
    }
}

class SwXDocumentIndex::Impl
    : public SwClient
{

public:

    SfxItemPropertySet const&   m_rPropSet;
    const TOXTypes              m_eTOXType;
    SwEventListenerContainer    m_ListenerContainer;
    bool                        m_bIsDescriptor;
    SwDoc *                     m_pDoc;
    ::std::auto_ptr<SwDocIndexDescriptorProperties_Impl> m_pProps;
    uno::WeakReference<container::XIndexReplace> m_wStyleAccess;
    uno::WeakReference<container::XIndexReplace> m_wTokenAccess;

    Impl(   SwXDocumentIndex & rThis,
            SwDoc & rDoc,
            const TOXTypes eType,
            SwTOXBaseSection const*const pBaseSection)
        : SwClient((pBaseSection) ? pBaseSection->GetFmt() : 0)
        , m_rPropSet(
            *aSwMapProvider.GetPropertySet(lcl_TypeToPropertyMap(eType)))
        , m_eTOXType(eType)
        , m_ListenerContainer(static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_bIsDescriptor(0 == pBaseSection)
        , m_pDoc(&rDoc)
        , m_pProps((m_bIsDescriptor)
            ? new SwDocIndexDescriptorProperties_Impl(rDoc.GetTOXType(eType, 0))
            : 0)
    {
    }

    SwSectionFmt * GetSectionFmt() const {
        return static_cast<SwSectionFmt *>(
                const_cast<SwModify *>(GetRegisteredIn()));
    }

    SwTOXBase & GetTOXSectionOrThrow() const
    {
        SwSectionFmt *const pSectionFmt(GetSectionFmt());
        SwTOXBase *const pTOXSection( (m_bIsDescriptor)
            ?  &m_pProps->GetTOXBase()
            : ((pSectionFmt)
                ? static_cast<SwTOXBaseSection*>(pSectionFmt->GetSection())
                : 0));
        if (!pTOXSection)
        {
            throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "SwXDocumentIndex: disposed or invalid")), 0);
        }
        return *pTOXSection;
    }

    sal_Int32 GetFormMax() const
    {
        SwTOXBase & rSection( GetTOXSectionOrThrow() );
        return (m_bIsDescriptor)
            ? SwForm::GetFormMaxLevel(m_eTOXType)
            : rSection.GetTOXForm().GetFormMax();
    }

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

/*-- 14.12.98 09:35:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    if (!GetRegisteredIn())
    {
        m_ListenerContainer.Disposing();
    }
}

/*-- 14.12.98 09:35:03---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndex::SwXDocumentIndex(
        SwTOXBaseSection const& rBaseSection, SwDoc & rDoc)
    : m_pImpl( new SwXDocumentIndex::Impl( *this,
                rDoc, rBaseSection.SwTOXBase::GetType(), & rBaseSection) )
{
}
/* -----------------15.01.99 14:59-------------------
 *
 * --------------------------------------------------*/
SwXDocumentIndex::SwXDocumentIndex(const TOXTypes eType, SwDoc& rDoc)
    : m_pImpl( new SwXDocumentIndex::Impl( *this, rDoc, eType, 0) )
{
}

/*-- 14.12.98 09:35:04---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndex::~SwXDocumentIndex()
{
}

uno::Reference<text::XDocumentIndex>
SwXDocumentIndex::CreateXDocumentIndex(
        SwDoc & rDoc, SwTOXBaseSection const& rSection)
{
    // re-use existing SwXDocumentIndex
    // #i105557#: do not iterate over the registered clients: race condition
    SwSectionFmt *const pFmt = rSection.GetFmt();
    uno::Reference<text::XDocumentIndex> xIndex(pFmt->GetXObject(),
            uno::UNO_QUERY);
    if (!xIndex.is())
    {
        SwXDocumentIndex *const pIndex(new SwXDocumentIndex(rSection, rDoc));
        xIndex.set(pIndex);
        pFmt->SetXObject(uno::Reference<uno::XInterface>(xIndex));
    }
    return xIndex;
}

/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXDocumentIndex::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL
SwXDocumentIndex::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXDocumentIndex>(rId, this);
}

/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL
SwXDocumentIndex::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXDocumentIndex");
}
/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXDocumentIndex::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    return C2U("com.sun.star.text.BaseIndex") == rServiceName
        || ((TOX_INDEX == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.DocumentIndex"))
        || ((TOX_CONTENT == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.ContentIndex"))
        || ((TOX_USER == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.UserDefinedIndex"))
        || ((TOX_ILLUSTRATIONS == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.IllustrationsIndex"))
        || ((TOX_TABLES == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.TableIndex"))
        || ((TOX_OBJECTS == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.ObjectIndex"))
        || ((TOX_AUTHORITIES == m_pImpl->m_eTOXType) &&
            rServiceName.equalsAscii("com.sun.star.text.Bibliography"));
}
/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXDocumentIndex::getSupportedServiceNames() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseIndex");
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_INDEX:
            pArray[1] = C2U("com.sun.star.text.DocumentIndex");
        break;
        case TOX_CONTENT:
            pArray[1] = C2U("com.sun.star.text.ContentIndex");
        break;
        case TOX_TABLES:
            pArray[1] = C2U("com.sun.star.text.TableIndex");
        break;
        case TOX_ILLUSTRATIONS:
            pArray[1] = C2U("com.sun.star.text.IllustrationsIndex");
        break;
        case TOX_OBJECTS:
            pArray[1] = C2U("com.sun.star.text.ObjectIndex");
        break;
        case TOX_AUTHORITIES:
            pArray[1] = C2U("com.sun.star.text.Bibliography");
        break;
        //case TOX_USER:
        default:
            pArray[1] = C2U("com.sun.star.text.UserDefinedIndex");
    }
    return aRet;
}

/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SAL_CALL SwXDocumentIndex::getServiceName()
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    USHORT nObjectType = SW_SERVICE_TYPE_INDEX;
    switch (m_pImpl->m_eTOXType)
    {
//      case TOX_INDEX:             break;
        case TOX_USER:          nObjectType = SW_SERVICE_USER_INDEX;
        break;
        case TOX_CONTENT:       nObjectType = SW_SERVICE_CONTENT_INDEX;
        break;
        case TOX_ILLUSTRATIONS: nObjectType = SW_SERVICE_INDEX_ILLUSTRATIONS;
        break;
        case TOX_OBJECTS:       nObjectType = SW_SERVICE_INDEX_OBJECTS;
        break;
        case TOX_TABLES:        nObjectType = SW_SERVICE_INDEX_TABLES;
        break;
        case TOX_AUTHORITIES:   nObjectType = SW_SERVICE_INDEX_BIBLIOGRAPHY;
        break;
        default:
        break;
    }
    return SwXServiceProvider::GetProviderName(nObjectType);
}

/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXDocumentIndex::update() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwSectionFmt *const pFmt = m_pImpl->GetSectionFmt();
    SwTOXBaseSection *const pTOXBase = (pFmt) ?
        static_cast<SwTOXBaseSection*>(pFmt->GetSection()) : 0;
    if(!pTOXBase)
    {
        throw uno::RuntimeException();
    }
    pTOXBase->Update();
    // page numbers
    pTOXBase->UpdatePageNum();
}

/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXDocumentIndex::getPropertySetInfo() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    const uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw beans::PropertyVetoException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only: " ))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    SwSectionFmt *const pSectionFmt(m_pImpl->GetSectionFmt());
    SwTOXBase & rTOXBase( m_pImpl->GetTOXSectionOrThrow() );

    sal_uInt16 nCreate = rTOXBase.GetCreateType();
    sal_uInt16 nTOIOptions = 0;
    sal_uInt16 nOLEOptions = rTOXBase.GetOLEOptions();
    const TOXTypes eTxBaseType = rTOXBase.GetTOXType()->GetType();
    if (eTxBaseType == TOX_INDEX)
    {
        nTOIOptions = rTOXBase.GetOptions();
    }
    SwForm  aForm(rTOXBase.GetTOXForm());
    sal_Bool bForm = sal_False;
    switch (pEntry->nWID)
    {
        case WID_IDX_TITLE:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetTitle(sNewName);
        }
        break;
        case WID_IDX_NAME:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetTOXName(sNewName);
        }
        break;
        case WID_USER_IDX_NAME:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            lcl_ConvertTOUNameToUserName(sNewName);
            DBG_ASSERT(TOX_USER == eTxBaseType,
                    "tox type name can only be changed for user indexes");
            if (pSectionFmt)
            {
                OUString sTmp = rTOXBase.GetTOXType()->GetTypeName();
                if (sTmp != sNewName)
                {
                    lcl_ReAssignTOXType(pSectionFmt->GetDoc(),
                            rTOXBase, sNewName);
                }
            }
            else
            {
                m_pImpl->m_pProps->SetTypeName(sNewName);
            }
        }
        break;
        case WID_IDX_LOCALE:
        {
            lang::Locale aLocale;
            if (!(rValue>>= aLocale))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetLanguage(SvxLocaleToLanguage(aLocale));
        }
        break;
        case WID_IDX_SORT_ALGORITHM:
        {
            OUString sTmp;
            if (!(rValue >>= sTmp))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetSortAlgorithm(sTmp);
        }
        break;
        case WID_LEVEL:
        {
            rTOXBase.SetLevel(lcl_AnyToInt16(rValue));
        }
        break;
        case WID_CREATE_FROM_MARKS:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_MARK);
        break;
        case WID_CREATE_FROM_OUTLINE:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_OUTLINELEVEL);
        break;
//          case WID_PARAGRAPH_STYLE_NAMES             :DBG_ERROR("not implemented")
//          break;
        case WID_CREATE_FROM_CHAPTER:
            rTOXBase.SetFromChapter(lcl_AnyToBool(rValue));
        break;
        case WID_CREATE_FROM_LABELS:
            rTOXBase.SetFromObjectNames(! lcl_AnyToBool(rValue));
        break;
        case WID_PROTECTED:
        {
            sal_Bool bSet = lcl_AnyToBool(rValue);
            rTOXBase.SetProtected(bSet);
            if (pSectionFmt)
            {
                static_cast<SwTOXBaseSection &>(rTOXBase).SetProtect(bSet);
            }
        }
        break;
        case WID_USE_ALPHABETICAL_SEPARATORS:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    nsSwTOIOptions::TOI_ALPHA_DELIMITTER);
        break;
        case WID_USE_KEY_AS_ENTRY:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    nsSwTOIOptions::TOI_KEY_AS_ENTRY);
        break;
        case WID_USE_COMBINED_ENTRIES:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    nsSwTOIOptions::TOI_SAME_ENTRY);
        break;
        case WID_IS_CASE_SENSITIVE:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    nsSwTOIOptions::TOI_CASE_SENSITIVE);
        break;
        case WID_USE_P_P:
            lcl_AnyToBitMask(rValue, nTOIOptions, nsSwTOIOptions::TOI_FF);
        break;
        case WID_USE_DASH:
            lcl_AnyToBitMask(rValue, nTOIOptions, nsSwTOIOptions::TOI_DASH);
        break;
        case WID_USE_UPPER_CASE:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    nsSwTOIOptions::TOI_INITIAL_CAPS);
        break;
        case WID_IS_COMMA_SEPARATED:
            bForm = sal_True;
            aForm.SetCommaSeparated(lcl_AnyToBool(rValue));
        break;
        case WID_LABEL_CATEGORY:
        {
            // convert file-format/API/external programmatic english name
            // to internal UI name before usage
            String aName( SwStyleNameMapper::GetSpecialExtraUIName(
                                lcl_AnyToString(rValue) ) );
            rTOXBase.SetSequenceName( aName );
        }
        break;
        case WID_LABEL_DISPLAY_TYPE:
        {
            const sal_Int16 nVal = lcl_AnyToInt16(rValue);
            sal_uInt16 nSet = CAPTION_COMPLETE;
            switch (nVal)
            {
                case text::ReferenceFieldPart::TEXT:
                    nSet = CAPTION_COMPLETE;
                break;
                case text::ReferenceFieldPart::CATEGORY_AND_NUMBER:
                    nSet = CAPTION_NUMBER;
                break;
                case text::ReferenceFieldPart::ONLY_CAPTION:
                    nSet = CAPTION_TEXT;
                break;
                default:
                    throw lang::IllegalArgumentException();
            }
            rTOXBase.SetCaptionDisplay(static_cast<SwCaptionDisplay>(nSet));
        }
        break;
        case WID_USE_LEVEL_FROM_SOURCE:
            rTOXBase.SetLevelFromChapter(lcl_AnyToBool(rValue));
        break;
        case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME:
        {
            String aString;
            SwStyleNameMapper::FillUIName(lcl_AnyToString(rValue),
                aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True);
            rTOXBase.SetMainEntryCharStyle( aString );
        }
        break;
        case WID_CREATE_FROM_TABLES:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_TABLE);
        break;
        case WID_CREATE_FROM_TEXT_FRAMES:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_FRAME);
        break;
        case WID_CREATE_FROM_GRAPHIC_OBJECTS:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_GRAPHIC);
        break;
        case WID_CREATE_FROM_EMBEDDED_OBJECTS:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_OLE);
        break;
        case WID_CREATE_FROM_STAR_MATH:
            lcl_AnyToBitMask(rValue, nOLEOptions, nsSwTOOElements::TOO_MATH);
        break;
        case WID_CREATE_FROM_STAR_CHART:
            lcl_AnyToBitMask(rValue, nOLEOptions, nsSwTOOElements::TOO_CHART);
        break;
        case WID_CREATE_FROM_STAR_CALC:
            lcl_AnyToBitMask(rValue, nOLEOptions, nsSwTOOElements::TOO_CALC);
        break;
        case WID_CREATE_FROM_STAR_DRAW:
            lcl_AnyToBitMask(rValue, nOLEOptions,
                    nsSwTOOElements::TOO_DRAW_IMPRESS);
        break;
        case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
            lcl_AnyToBitMask(rValue, nOLEOptions, nsSwTOOElements::TOO_OTHER);
        break;
        case WID_PARA_HEAD:
        {
            String aString;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
            bForm = sal_True;
            // Header is on Pos 0
            aForm.SetTemplate( 0, aString );
        }
        break;
        case WID_IS_RELATIVE_TABSTOPS:
            bForm = sal_True;
            aForm.SetRelTabPos(lcl_AnyToBool(rValue));
        break;
        case WID_PARA_SEP:
        {
            String aString;
            bForm = sal_True;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
            aForm.SetTemplate( 1, aString );
        }
        break;
        case WID_CREATE_FROM_PARAGRAPH_STYLES:
            lcl_AnyToBitMask(rValue, nCreate, nsSwTOXElement::TOX_TEMPLATE);
        break;

        case WID_PARA_LEV1:
        case WID_PARA_LEV2:
        case WID_PARA_LEV3:
        case WID_PARA_LEV4:
        case WID_PARA_LEV5:
        case WID_PARA_LEV6:
        case WID_PARA_LEV7:
        case WID_PARA_LEV8:
        case WID_PARA_LEV9:
        case WID_PARA_LEV10:
        {
            bForm = sal_True;
            // in sdbcx::Index Label 1 begins at Pos 2 otherwise at Pos 1
            const sal_uInt16 nLPos = rTOXBase.GetType() == TOX_INDEX ? 2 : 1;
            String aString;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
            aForm.SetTemplate(nLPos + pEntry->nWID - WID_PARA_LEV1, aString );
        }
        break;
        default:
            //this is for items only
            if (WID_PRIMARY_KEY > pEntry->nWID)
            {
                const SwAttrSet& rSet =
                    m_pImpl->m_pDoc->GetTOXBaseAttrSet(rTOXBase);
                SfxItemSet aAttrSet(rSet);
                m_pImpl->m_rPropSet.setPropertyValue(
                        rPropertyName, rValue, aAttrSet);

                const SwSectionFmts& rSects = m_pImpl->m_pDoc->GetSections();
                for (sal_uInt16 i = 0; i < rSects.Count(); i++)
                {
                    const SwSectionFmt* pTmpFmt = rSects[ i ];
                    if (pTmpFmt == pSectionFmt)
                    {
                        m_pImpl->m_pDoc->ChgSection( i,
                            static_cast<SwTOXBaseSection&>(rTOXBase),
                            & aAttrSet);
                        break;
                    }
                }
            }
    }
    rTOXBase.SetCreate(nCreate);
    rTOXBase.SetOLEOptions(nOLEOptions);
    if (rTOXBase.GetTOXType()->GetType() == TOX_INDEX)
    {
        rTOXBase.SetOptions(nTOIOptions);
    }
    if (bForm)
    {
        rTOXBase.SetTOXForm(aForm);
    }
}

/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL
SwXDocumentIndex::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Any aRet;
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }

    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    SwTOXBase* pTOXBase = 0;
    if (pSectionFmt)
    {
        pTOXBase = static_cast<SwTOXBaseSection*>(pSectionFmt->GetSection());
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        pTOXBase = &m_pImpl->m_pProps->GetTOXBase();
    }
    if(pTOXBase)
    {
        const sal_uInt16 nCreate = pTOXBase->GetCreateType();
        const sal_uInt16 nOLEOptions = pTOXBase->GetOLEOptions();
        const sal_uInt16 nTOIOptions =
            (pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            ? pTOXBase->GetOptions()
            : 0U;
        const SwForm& rForm = pTOXBase->GetTOXForm();
        switch(pEntry->nWID)
        {
            case WID_IDX_CONTENT_SECTION:
            case WID_IDX_HEADER_SECTION :
                if(WID_IDX_CONTENT_SECTION == pEntry->nWID)
                {
                    const uno::Reference <text::XTextSection> xContentSect =
                        SwXTextSection::CreateXTextSection( pSectionFmt );
                    aRet <<= xContentSect;
                }
                else
                {
                    SwSections aSectArr;
                    pSectionFmt->GetChildSections(aSectArr,
                            SORTSECT_NOT, FALSE);
                    for(USHORT i = 0; i < aSectArr.Count(); i++)
                    {
                        SwSection* pSect = aSectArr[i];
                        if(pSect->GetType() == TOX_HEADER_SECTION)
                        {
                            const uno::Reference <text::XTextSection> xHeader =
                                SwXTextSection::CreateXTextSection(
                                    pSect->GetFmt() );
                            aRet <<= xHeader;
                            break;
                        }
                    }
                }
            break;
            case WID_IDX_TITLE  :
            {
                OUString uRet(pTOXBase->GetTitle());
                aRet <<= uRet;
                break;
            }
            case WID_IDX_NAME:
                aRet <<= OUString(pTOXBase->GetTOXName());
            break;
            case WID_USER_IDX_NAME:
            {
                OUString sTmp;
                if (!m_pImpl->m_bIsDescriptor)
                {
                    sTmp = pTOXBase->GetTOXType()->GetTypeName();
                }
                else
                {
                    sTmp = m_pImpl->m_pProps->GetTypeName();
                }
                //I18N
                lcl_ConvertTOUNameToProgrammaticName(sTmp);
                aRet <<= sTmp;
            }
            break;
            case WID_IDX_LOCALE:
                aRet <<= SvxCreateLocale(pTOXBase->GetLanguage());
            break;
            case WID_IDX_SORT_ALGORITHM:
                aRet <<= OUString(pTOXBase->GetSortAlgorithm());
            break;
            case WID_LEVEL      :
                aRet <<= static_cast<sal_Int16>(pTOXBase->GetLevel());
            break;
            case WID_CREATE_FROM_MARKS:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_MARK);
            break;
            case WID_CREATE_FROM_OUTLINE:
                lcl_BitMaskToAny(aRet, nCreate,
                        nsSwTOXElement::TOX_OUTLINELEVEL);
            break;
            case WID_CREATE_FROM_CHAPTER:
            {
                const sal_Bool bRet = pTOXBase->IsFromChapter();
                aRet <<= bRet;
            }
            break;
            case WID_CREATE_FROM_LABELS:
            {
                const sal_Bool bRet = ! pTOXBase->IsFromObjectNames();
                aRet <<= bRet;
            }
            break;
            case WID_PROTECTED:
            {
                const sal_Bool bRet = pTOXBase->IsProtected();
                aRet <<= bRet;
            }
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        nsSwTOIOptions::TOI_ALPHA_DELIMITTER);
            break;
            case WID_USE_KEY_AS_ENTRY:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        nsSwTOIOptions::TOI_KEY_AS_ENTRY);
            break;
            case WID_USE_COMBINED_ENTRIES:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        nsSwTOIOptions::TOI_SAME_ENTRY);
            break;
            case WID_IS_CASE_SENSITIVE:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        nsSwTOIOptions::TOI_CASE_SENSITIVE);
            break;
            case WID_USE_P_P:
                lcl_BitMaskToAny(aRet, nTOIOptions, nsSwTOIOptions::TOI_FF);
            break;
            case WID_USE_DASH:
                lcl_BitMaskToAny(aRet, nTOIOptions, nsSwTOIOptions::TOI_DASH);
            break;
            case WID_USE_UPPER_CASE:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        nsSwTOIOptions::TOI_INITIAL_CAPS);
            break;
            case WID_IS_COMMA_SEPARATED:
            {
                const sal_Bool bRet = rForm.IsCommaSeparated();
                aRet <<= bRet;
            }
            break;
            case WID_LABEL_CATEGORY:
            {
                // convert internal UI name to
                // file-format/API/external programmatic english name
                // before usage
                String aName( SwStyleNameMapper::GetSpecialExtraProgName(
                                    pTOXBase->GetSequenceName() ) );
                aRet <<= OUString( aName );
            }
            break;
            case WID_LABEL_DISPLAY_TYPE:
            {
                sal_Int16 nSet = text::ReferenceFieldPart::TEXT;
                switch (pTOXBase->GetCaptionDisplay())
                {
                    case CAPTION_COMPLETE:
                        nSet = text::ReferenceFieldPart::TEXT;
                    break;
                    case CAPTION_NUMBER:
                        nSet = text::ReferenceFieldPart::CATEGORY_AND_NUMBER;
                    break;
                    case CAPTION_TEXT:
                        nSet = text::ReferenceFieldPart::ONLY_CAPTION;
                    break;
                }
                aRet <<= nSet;
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE:
            {
                const sal_Bool bRet = pTOXBase->IsLevelFromChapter();
                aRet <<= bRet;
            }
            break;
            case WID_LEVEL_FORMAT:
            {
                uno::Reference< container::XIndexReplace > xTokenAccess(
                    m_pImpl->m_wTokenAccess);
                if (!xTokenAccess.is())
                {
                    xTokenAccess = new SwXIndexTokenAccess_Impl(*this);
                    m_pImpl->m_wTokenAccess = xTokenAccess;
                }
                aRet <<= xTokenAccess;
            }
            break;
            case WID_LEVEL_PARAGRAPH_STYLES:
            {
                uno::Reference< container::XIndexReplace > xStyleAccess(
                    m_pImpl->m_wStyleAccess);
                if (!xStyleAccess.is())
                {
                    xStyleAccess = new SwXIndexStyleAccess_Impl(*this);
                    m_pImpl->m_wStyleAccess = xStyleAccess;
                }
                aRet <<= xStyleAccess;
            }
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME:
            {
                String aString;
                SwStyleNameMapper::FillProgName(
                        pTOXBase->GetMainEntryCharStyle(),
                        aString,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                        sal_True);
                aRet <<= OUString( aString );
            }
            break;
            case WID_CREATE_FROM_TABLES:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_TABLE);
            break;
            case WID_CREATE_FROM_TEXT_FRAMES:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_FRAME);
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_GRAPHIC);
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_OLE);
            break;
            case WID_CREATE_FROM_STAR_MATH:
                lcl_BitMaskToAny(aRet, nOLEOptions, nsSwTOOElements::TOO_MATH);
            break;
            case WID_CREATE_FROM_STAR_CHART:
                lcl_BitMaskToAny(aRet, nOLEOptions, nsSwTOOElements::TOO_CHART);
            break;
            case WID_CREATE_FROM_STAR_CALC:
                lcl_BitMaskToAny(aRet, nOLEOptions, nsSwTOOElements::TOO_CALC);
            break;
            case WID_CREATE_FROM_STAR_DRAW:
                lcl_BitMaskToAny(aRet, nOLEOptions,
                        nsSwTOOElements::TOO_DRAW_IMPRESS);
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                lcl_BitMaskToAny(aRet, nOLEOptions, nsSwTOOElements::TOO_OTHER);
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                lcl_BitMaskToAny(aRet, nCreate, nsSwTOXElement::TOX_TEMPLATE);
            break;
            case WID_PARA_HEAD:
            {
                //Header steht an Pos 0
                String aString;
                SwStyleNameMapper::FillProgName(rForm.GetTemplate( 0 ), aString,
                        nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
                aRet <<= OUString( aString );
            }
            break;
            case WID_PARA_SEP:
            {
                String aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate( 1 ),
                        aString,
                        nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                        sal_True);
                aRet <<= OUString( aString );
            }
            break;
            case WID_PARA_LEV1:
            case WID_PARA_LEV2:
            case WID_PARA_LEV3:
            case WID_PARA_LEV4:
            case WID_PARA_LEV5:
            case WID_PARA_LEV6:
            case WID_PARA_LEV7:
            case WID_PARA_LEV8:
            case WID_PARA_LEV9:
            case WID_PARA_LEV10:
            {
                // in sdbcx::Index Label 1 begins at Pos 2 otherwise at Pos 1
                sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                String aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate(nLPos + pEntry->nWID - WID_PARA_LEV1),
                        aString,
                        nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                        sal_True);
                aRet <<= OUString( aString );
            }
            break;
            case WID_IS_RELATIVE_TABSTOPS:
            {
                const sal_Bool bRet = rForm.IsRelTabPos();
                aRet <<= bRet;
            }
            break;
            case WID_INDEX_MARKS:
            {
                SwTOXMarks aMarks;
                SwTOXType const*const pType = pTOXBase->GetTOXType();
                SwClientIter aIter(*pType);
                SwTOXMark * pMark =
                    static_cast<SwTOXMark*>(aIter.First(TYPE(SwTOXMark)));
                while( pMark )
                {
                    if(pMark->GetTxtTOXMark())
                    {
                        aMarks.C40_INSERT(SwTOXMark, pMark, aMarks.Count());
                    }
                    pMark = static_cast<SwTOXMark*>(aIter.Next());
                }
                uno::Sequence< uno::Reference<text::XDocumentIndexMark> >
                    aXMarks(aMarks.Count());
                uno::Reference<text::XDocumentIndexMark>* pxMarks =
                    aXMarks.getArray();
                for(USHORT i = 0; i < aMarks.Count(); i++)
                {
                    pMark = aMarks.GetObject(i);
                    pxMarks[i] = SwXDocumentIndexMark::GetObject(
                        const_cast<SwTOXType*>(pType), pMark, m_pImpl->m_pDoc);
                }
                aRet <<= aXMarks;
            }
            break;
            default:
                //this is for items only
                if(WID_PRIMARY_KEY > pEntry->nWID)
                {
                    const SwAttrSet& rSet =
                        m_pImpl->m_pDoc->GetTOXBaseAttrSet(*pTOXBase);
                    aRet = m_pImpl->m_rPropSet.getPropertyValue(
                            rPropertyName, rSet);
                }
        }
    }
    return aRet;
}

/*-- 14.12.98 09:35:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXDocumentIndex::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXDocumentIndex::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXDocumentIndex::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXDocumentIndex::removeVetoableChangeListener(): not implemented");
}

/* -----------------18.02.99 13:39-------------------
 *
 * --------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::attach(const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }
    const uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange *const pRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);

    SwDoc *const pDoc =
        (pRange) ? pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : 0);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDoc);
    //das muss jetzt sal_True liefern
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);

    const SwTOXBase* pOld = pDoc->GetCurTOX( *aPam.Start() );
    if (pOld)
    {
        throw lang::IllegalArgumentException();
    }

    UnoActionContext aAction(pDoc);
    if (aPam.HasMark())
    {
        pDoc->DeleteAndJoin(aPam);
    }

    SwTOXBase & rTOXBase = m_pImpl->m_pProps->GetTOXBase();
    SwTOXType const*const pTOXType = rTOXBase.GetTOXType();
    if ((TOX_USER == pTOXType->GetType()) &&
        !m_pImpl->m_pProps->GetTypeName().equals(pTOXType->GetTypeName()))
    {
        lcl_ReAssignTOXType(pDoc, rTOXBase, m_pImpl->m_pProps->GetTypeName());
    }
    //TODO: apply Section attributes (columns and background)
    SwTOXBaseSection const*const pTOX =
        pDoc->InsertTableOf( *aPam.GetPoint(), rTOXBase, 0, sal_False );

    pDoc->SetTOXBaseName(*pTOX, m_pImpl->m_pProps->GetTOXBase().GetTOXName());

    // update page numbers
    pTOX->GetFmt()->Add(m_pImpl.get());
    pTOX->GetFmt()->SetXObject(static_cast< ::cppu::OWeakObject*>(this));
    const_cast<SwTOXBaseSection*>(pTOX)->UpdatePageNum();

    m_pImpl->m_pProps.reset();
    m_pImpl->m_pDoc = pDoc;
    m_pImpl->m_bIsDescriptor = sal_False;
}

/*-- 15.01.99 14:23:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SAL_CALL
SwXDocumentIndex::getAnchor() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    if (!pSectionFmt)
    {
        throw uno::RuntimeException();
    }

    uno::Reference< text::XTextRange > xRet;
    SwNodeIndex const*const pIdx( pSectionFmt->GetCntnt().GetCntntIdx() );
    if (pIdx && pIdx->GetNode().GetNodes().IsDocNodes())
    {
        SwPaM aPaM(*pIdx);
        aPaM.Move( fnMoveForward, fnGoCntnt );
        aPaM.SetMark();
        aPaM.GetPoint()->nNode = *pIdx->GetNode().EndOfSectionNode();
        aPaM.Move( fnMoveBackward, fnGoCntnt );
        xRet = SwXTextRange::CreateXTextRange(*pSectionFmt->GetDoc(),
            *aPaM.GetMark(), aPaM.GetPoint());
    }
    return xRet;
}

/*-- 15.01.99 15:46:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_RemoveChildSections(SwSectionFmt& rParentFmt)
{
    SwSections aTmpArr;
    SwDoc *const pDoc = rParentFmt.GetDoc();
    const sal_uInt16 nCnt = rParentFmt.GetChildSections(aTmpArr, SORTSECT_POS);
    if( nCnt )
    {
        for( sal_uInt16 n = 0; n < nCnt; ++n )
        {
            if( aTmpArr[n]->GetFmt()->IsInNodesArr() )
            {
                SwSectionFmt* pFmt = aTmpArr[n]->GetFmt();
                lcl_RemoveChildSections(*pFmt);
                pDoc->DelSectionFmt( pFmt );
            }
        }
    }
}

void SAL_CALL SwXDocumentIndex::dispose() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    if (pSectionFmt)
    {
        pSectionFmt->GetDoc()->DeleteTOX(
            *static_cast<SwTOXBaseSection*>(pSectionFmt->GetSection()),
            sal_True);
    }
}

/*-- 15.01.99 15:46:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!m_pImpl->GetRegisteredIn())
    {
        throw uno::RuntimeException();
    }
    m_pImpl->m_ListenerContainer.AddListener(xListener);
}
/*-- 15.01.99 15:46:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!m_pImpl->GetRegisteredIn() ||
        !m_pImpl->m_ListenerContainer.RemoveListener(xListener))
    {
        throw uno::RuntimeException();
    }
}

/* -----------------30.07.99 11:28-------------------

 --------------------------------------------------*/
OUString SAL_CALL SwXDocumentIndex::getName() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    OUString uRet;
    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    if (m_pImpl->m_bIsDescriptor)
    {
        uRet = OUString(m_pImpl->m_pProps->GetTOXBase().GetTOXName());
    }
    else if(pSectionFmt)
    {
        uRet = OUString(pSectionFmt->GetSection()->GetName());
    }
    else
    {
        throw uno::RuntimeException();
    }
    return uRet;
}
/* -----------------30.07.99 11:28-------------------

 --------------------------------------------------*/
void SAL_CALL
SwXDocumentIndex::setName(const OUString& rName) throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!rName.getLength())
    {
        throw uno::RuntimeException();
    }

    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pProps->GetTOXBase().SetTOXName(rName);
    }
    else if (pSectionFmt)
    {
        const bool bSuccess = pSectionFmt->GetDoc()->SetTOXBaseName(
            *static_cast<SwTOXBaseSection*>(pSectionFmt->GetSection()), rName);
        if (!bSuccess)
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}

/******************************************************************
 * SwXDocumentIndexMark
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXDocumentIndexMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXDocumentIndexMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

TYPEINIT1(SwXDocumentIndexMark, SwClient)
const sal_Char cBaseMark[]      = "com.sun.star.text.BaseIndexMark";
const sal_Char cContentMark[]   = "com.sun.star.text.ContentIndexMark";
const sal_Char cIdxMark[]       = "com.sun.star.text.DocumentIndexMark";
const sal_Char cIdxMarkAsian[]  = "com.sun.star.text.DocumentIndexMarkAsian";
const sal_Char cUserMark[]      = "com.sun.star.text.UserIndexMark";
const sal_Char cTextContent[]   = "com.sun.star.text.TextContent";
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDocumentIndexMark::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXDocumentIndexMark");
}
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDocumentIndexMark::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii(cBaseMark)||
        !rServiceName.compareToAscii(cTextContent) ||
        (eType == TOX_USER && !rServiceName.compareToAscii(cUserMark)) ||
        (eType == TOX_CONTENT && !rServiceName.compareToAscii(cContentMark)) ||
        (eType == TOX_INDEX && !rServiceName.compareToAscii(cIdxMark)) ||
        (eType == TOX_INDEX && !rServiceName.compareToAscii(cIdxMarkAsian));
}
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXDocumentIndexMark::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    INT32 nCnt = (eType == TOX_INDEX) ? 4 : 3;
    uno::Sequence< OUString > aRet(nCnt);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U(cBaseMark);
    pArray[1] = C2U(cTextContent);
    switch(eType)
    {
        case TOX_USER:
            pArray[2] = C2U(cUserMark);
        break;
        case TOX_CONTENT:
            pArray[2] = C2U(cContentMark);
        break;
        case TOX_INDEX:
            pArray[2] = C2U(cIdxMark);
            pArray[3] = C2U(cIdxMarkAsian);
        break;

        default:
            ;
    }
    return aRet;
}
/*-- 14.12.98 10:25:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::SwXDocumentIndexMark(TOXTypes eToxType) :
    aLstnrCntnr( (text::XTextContent*)this),
    aTypeDepend(this, 0),
    m_pDoc(0),
    m_pTOXMark(0),
    bIsDescriptor(sal_True),
    bMainEntry(sal_False),
    eType(eToxType),
    nLevel(0)
{
    InitMap(eToxType);
}
/*-- 14.12.98 10:25:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::SwXDocumentIndexMark(const SwTOXType* pType,
                                    const SwTOXMark* pMark,
                                    SwDoc* pDc) :
    aLstnrCntnr( (text::XTextContent*)this),
    aTypeDepend(this, (SwTOXType*)pType),
    m_pDoc(pDc),
    m_pTOXMark(pMark),
    bIsDescriptor(sal_False),
    bMainEntry(sal_False),
    eType(pType->GetType()),
    nLevel(0)
{
    m_pDoc->GetUnoCallBack()->Add(this);
    InitMap(eType);
}
/*-- 14.12.98 10:25:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::~SwXDocumentIndexMark()
{

}
/* -----------------21.04.99 09:36-------------------
 *
 * --------------------------------------------------*/
void SwXDocumentIndexMark::InitMap(TOXTypes eToxType)
{
    sal_uInt16 nMapId = PROPERTY_MAP_USER_MARK; //case TOX_USER:
    switch( eToxType )
    {
        case TOX_INDEX:
            nMapId = PROPERTY_MAP_INDEX_MARK ;
        break;
        case TOX_CONTENT:
            nMapId = PROPERTY_MAP_CNTIDX_MARK;
        break;
        //case TOX_USER:

        default:
            ;
    }
    m_pPropSet = aSwMapProvider.GetPropertySet(nMapId);
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXDocumentIndexMark::getMarkEntry(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    OUString sRet;
    if(pType && m_pTOXMark)
    {
        sRet = OUString(m_pTOXMark->GetAlternativeText());
    }
    else if(bIsDescriptor)
         sRet = sAltText;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::setMarkEntry(const OUString& rIndexEntry) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType && m_pTOXMark)
    {
        SwTOXMark aMark(*m_pTOXMark);
        aMark.SetAlternativeText(rIndexEntry);
        const SwTxtTOXMark* pTxtMark = m_pTOXMark->GetTxtTOXMark();
        SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
        aPam.SetMark();
        if(pTxtMark->GetEnd())
        {
            aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
        }
        else
            aPam.GetPoint()->nContent++;

        //die alte Marke loeschen
        m_pDoc->DeleteTOXMark(m_pTOXMark);
        m_pTOXMark = 0;

        SwTxtAttr* pTxtAttr = 0;
        sal_Bool bInsAtPos = aMark.IsAlternativeText();
        const SwPosition *pStt = aPam.Start(),
                            *pEnd = aPam.End();
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            m_pDoc->InsertPoolItem( aTmp, aMark, 0 );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttrForCharAt(
                        pStt->nContent.GetIndex()-1, RES_TXTATR_TOXMARK);
        }
        else if( *pEnd != *pStt )
        {
            m_pDoc->InsertPoolItem( aPam, aMark,
                        nsSetAttrMode::SETATTR_DONTEXPAND );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                                pStt->nContent, RES_TXTATR_TOXMARK);
        }
        //und sonst - Marke geloescht?
        if(pTxtAttr)
            m_pTOXMark = &pTxtAttr->GetTOXMark();
    }
    else if(bIsDescriptor)
    {
        sAltText = rIndexEntry;
    }
    else
        throw uno::RuntimeException();
}
/* -----------------18.02.99 13:40-------------------
 *
 * --------------------------------------------------*/
void SwXDocumentIndexMark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!bIsDescriptor)
        throw uno::RuntimeException();

    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;

    if(pDoc )
    {
        const SwTOXType* pTOXType = 0;
        switch(eType)
        {
            case TOX_INDEX:
            case TOX_CONTENT:
                pTOXType = pDoc->GetTOXType( eType, 0 );
            break;
            case TOX_USER:
            {
                if(!sUserIndexName.Len())
                    pTOXType = pDoc->GetTOXType( eType, 0 );
                else
                {
                    sal_uInt16 nCount = pDoc->GetTOXTypeCount( eType);
                    for(sal_uInt16 i = 0; i < nCount; i++)
                    {
                        const SwTOXType* pTemp = pDoc->GetTOXType( eType, i );
                        if(sUserIndexName == pTemp->GetTypeName())
                        {
                            pTOXType = pTemp;
                            break;
                        }
                    }
                    if(!pTOXType)
                    {
                        SwTOXType aUserType(TOX_USER, sUserIndexName);
                        pTOXType = pDoc->InsertTOXType(aUserType);
                    }
                }
            }
            break;

            default:
                ;
        }
        if(!pTOXType)
            throw lang::IllegalArgumentException();
        pDoc->GetUnoCallBack()->Add(this);
        ((SwTOXType*)pTOXType)->Add(&aTypeDepend);

        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        ::sw::XTextRangeToSwPaM(aPam, xTextRange);
        SwTOXMark aMark (pTOXType);
        if(sAltText.Len())
            aMark.SetAlternativeText(sAltText);
        switch(eType)
        {
            case TOX_INDEX:
                if(sPrimaryKey.Len())
                    aMark.SetPrimaryKey(sPrimaryKey);
                if(sSecondaryKey.Len())
                    aMark.SetSecondaryKey(sSecondaryKey);
                if(sTextReading.Len())
                    aMark.SetTextReading(sTextReading);
                if(sPrimaryKeyReading.Len())
                    aMark.SetPrimaryKeyReading(sPrimaryKeyReading);
                if(sSecondaryKeyReading.Len())
                    aMark.SetSecondaryKeyReading(sSecondaryKeyReading);
                aMark.SetMainEntry(bMainEntry);
            break;
            case TOX_USER:
            case TOX_CONTENT:
                if(USHRT_MAX != nLevel)
                    aMark.SetLevel(nLevel+1);
            break;

            default:
                ;
        }
        UnoActionContext aAction(pDoc);
        sal_Bool bMark = *aPam.GetPoint() != *aPam.GetMark();
        // Marks ohne Alternativtext ohne selektierten Text koennen nicht eingefuegt werden,
        // deshalb hier ein Leerzeichen - ob das die ideale Loesung ist?
        if(!bMark && !aMark.GetAlternativeText().Len())
            aMark.SetAlternativeText( String(' ') );

        SwXTextCursor const*const pTextCursor(
                dynamic_cast<SwXTextCursor*>(pCursor));
        const bool bForceExpandHints( (!bMark && pTextCursor)
                ? pTextCursor->IsAtEndOfMeta() : false );
        const SetAttrMode nInsertFlags = (bForceExpandHints)
            ?   ( nsSetAttrMode::SETATTR_FORCEHINTEXPAND
                | nsSetAttrMode::SETATTR_DONTEXPAND)
            : nsSetAttrMode::SETATTR_DONTEXPAND;

        pDoc->InsertPoolItem(aPam, aMark, nInsertFlags);
        if( bMark && *aPam.GetPoint() > *aPam.GetMark())
            aPam.Exchange();

        SwTxtAttr* pTxtAttr = 0;
        if( bMark )
            pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttr(
                            aPam.GetPoint()->nContent, RES_TXTATR_TOXMARK );
        else
        {
            pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
                aPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_TOXMARK );
        }

        if(pTxtAttr)
        {
            m_pTOXMark = &pTxtAttr->GetTOXMark();
            m_pDoc = pDoc;
            bIsDescriptor = sal_False;
        }
        else
            throw uno::RuntimeException();
    }
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXDocumentIndexMark::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType && m_pTOXMark)
    {
        if( m_pTOXMark->GetTxtTOXMark() )
        {
            const SwTxtTOXMark* pTxtMark = m_pTOXMark->GetTxtTOXMark();
            SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
            aPam.SetMark();
            if(pTxtMark->GetEnd())
            {
                aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
            }
            else
                aPam.GetPoint()->nContent++;
            uno::Reference< frame::XModel >  xModel = m_pDoc->GetDocShell()->GetBaseModel();
            uno::Reference< text::XTextDocument > xTDoc(xModel, uno::UNO_QUERY);
            aRet = new SwXTextRange(aPam, xTDoc->getText());
        }
    }
    if(!aRet.is())
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType && m_pTOXMark)
    {
        m_pDoc->DeleteTOXMark(m_pTOXMark);
    }
    else
        throw uno::RuntimeException();
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXDocumentIndexMark::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xInfos[3];
    int nPos = 0;
    switch(eType)
    {
        case TOX_INDEX: nPos = 0; break;
        case TOX_CONTENT: nPos = 1; break;
        case TOX_USER:  nPos = 2; break;
        default:
            ;
    }
    if(!xInfos[nPos].is())
    {
        uno::Reference< beans::XPropertySetInfo >  xInfo = m_pPropSet->getPropertySetInfo();
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xInfos[nPos] = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMapEntries(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return xInfos[nPos];
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::setPropertyValue(const OUString& rPropertyName,
                                            const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
        throw beans::PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if(pType && m_pTOXMark)
    {
        SwDoc* pLocalDoc = m_pDoc;

        SwTOXMark aMark(*m_pTOXMark);
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aMark.SetAlternativeText(lcl_AnyToString(aValue));
            break;
            case WID_LEVEL:
                aMark.SetLevel(Min( (sal_Int8) ( MAXLEVEL ),
                                    (sal_Int8)(lcl_AnyToInt16(aValue)+1)));
            break;
            case WID_PRIMARY_KEY  :
                aMark.SetPrimaryKey(lcl_AnyToString(aValue));
            break;
            case WID_SECONDARY_KEY:
                aMark.SetSecondaryKey(lcl_AnyToString(aValue));
            break;
            case WID_MAIN_ENTRY:
                aMark.SetMainEntry(lcl_AnyToBool(aValue));
            break;
            case WID_TEXT_READING:
                aMark.SetTextReading(lcl_AnyToString(aValue));
            break;
            case WID_PRIMARY_KEY_READING:
                aMark.SetPrimaryKeyReading(lcl_AnyToString(aValue));
            break;
            case WID_SECONDARY_KEY_READING:
                aMark.SetSecondaryKeyReading(lcl_AnyToString(aValue));
            break;
        }
        const SwTxtTOXMark* pTxtMark = m_pTOXMark->GetTxtTOXMark();
        SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
        aPam.SetMark();
        if(pTxtMark->GetEnd())
        {
            aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
        }
        else
            aPam.GetPoint()->nContent++;

        //delete the old mark
        pLocalDoc->DeleteTOXMark(m_pTOXMark);
        m_pTOXMark = 0;

        sal_Bool bInsAtPos = aMark.IsAlternativeText();
        const SwPosition *pStt = aPam.Start(),
                            *pEnd = aPam.End();

        SwTxtAttr* pTxtAttr = 0;
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            pLocalDoc->InsertPoolItem( aTmp, aMark, 0 );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttrForCharAt(
                    pStt->nContent.GetIndex()-1, RES_TXTATR_TOXMARK );
        }
        else if( *pEnd != *pStt )
        {
            pLocalDoc->InsertPoolItem( aPam, aMark,
                    nsSetAttrMode::SETATTR_DONTEXPAND );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                            pStt->nContent, RES_TXTATR_TOXMARK );
        }
        m_pDoc = pLocalDoc;

        if(pTxtAttr)
        {
            m_pTOXMark = &pTxtAttr->GetTOXMark();
            m_pDoc->GetUnoCallBack()->Add(this);
            pType->Add(&aTypeDepend);
        }
    }
    else if(bIsDescriptor)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                sAltText = lcl_AnyToString(aValue);
            break;
            case WID_LEVEL:
            {
                sal_Int16 nVal = lcl_AnyToInt16(aValue);
                if(nVal >= 0 && nVal < MAXLEVEL)
                    nLevel = nVal;
                else
                    throw lang::IllegalArgumentException();
            }
            break;
            case WID_PRIMARY_KEY  :
                sPrimaryKey = lcl_AnyToString(aValue);
            break;
            case WID_SECONDARY_KEY:
                sSecondaryKey = lcl_AnyToString(aValue);
            break;
            case WID_TEXT_READING:
                sTextReading = lcl_AnyToString(aValue);
            break;
            case WID_PRIMARY_KEY_READING:
                sPrimaryKeyReading = lcl_AnyToString(aValue);
            break;
            case WID_SECONDARY_KEY_READING:
                sSecondaryKeyReading = lcl_AnyToString(aValue);
            break;
            case WID_USER_IDX_NAME :
            {
                OUString sTmp(lcl_AnyToString(aValue));
                lcl_ConvertTOUNameToUserName(sTmp);
                sUserIndexName = sTmp;
            }
            break;
            case WID_MAIN_ENTRY:
                bMainEntry = lcl_AnyToBool(aValue);
            break;
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexMark::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if (::sw::GetDefaultTextContentValue(aRet, rPropertyName, pEntry->nWID))
    {
        return aRet;
    }
    if(pType && m_pTOXMark)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= OUString(m_pTOXMark->GetAlternativeText());
            break;
            case WID_LEVEL:
                aRet <<= (sal_Int16)(m_pTOXMark->GetLevel() - 1);
            break;
            case WID_PRIMARY_KEY  :
                aRet <<= OUString(m_pTOXMark->GetPrimaryKey());
            break;
            case WID_SECONDARY_KEY:
                aRet <<= OUString(m_pTOXMark->GetSecondaryKey());
            break;
            case WID_TEXT_READING:
                aRet <<= OUString(m_pTOXMark->GetTextReading());
            break;
            case WID_PRIMARY_KEY_READING:
                aRet <<= OUString(m_pTOXMark->GetPrimaryKeyReading());
            break;
            case WID_SECONDARY_KEY_READING:
                aRet <<= OUString(m_pTOXMark->GetSecondaryKeyReading());
            break;
            case WID_USER_IDX_NAME :
            {
                OUString sTmp(pType->GetTypeName());
                lcl_ConvertTOUNameToProgrammaticName(sTmp);
                aRet <<= sTmp;
            }
            break;
            case WID_MAIN_ENTRY:
            {
                sal_Bool bTemp = m_pTOXMark->IsMainEntry();
                aRet.setValue(&bTemp, ::getBooleanCppuType());
            }
            break;
        }
    }
    else if(bIsDescriptor)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= OUString(sAltText);
            break;
            case WID_LEVEL:
                aRet <<= (sal_Int16)nLevel;
            break;
            case WID_PRIMARY_KEY  :
                aRet <<= OUString(sPrimaryKey);
            break;
            case WID_SECONDARY_KEY:
                aRet <<= OUString(sSecondaryKey);
            break;
            case WID_TEXT_READING:
                aRet <<= OUString(sTextReading);
            break;
            case WID_PRIMARY_KEY_READING:
                aRet <<= OUString(sPrimaryKeyReading);
            break;
            case WID_SECONDARY_KEY_READING:
                aRet <<= OUString(sSecondaryKeyReading);
            break;
            case WID_USER_IDX_NAME :
                aRet <<= OUString(sUserIndexName);
            break;
            case WID_MAIN_ENTRY:
            {
                aRet.setValue(&bMainEntry, ::getBooleanCppuType());
            }
            break;
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark*   SwXDocumentIndexMark::GetObject(SwTOXType* pType,
                                    const SwTOXMark* pMark, SwDoc* pDoc)
{
    SwClientIter aIter(*pType);
    SwXDocumentIndexMark* pxMark = (SwXDocumentIndexMark*)
                                            aIter.First(TYPE(SwXDocumentIndexMark));
    while( pxMark )
    {
        if(pxMark->m_pTOXMark == pMark)
            return pxMark;
        pxMark = (SwXDocumentIndexMark*)aIter.Next();
    }
    return new SwXDocumentIndexMark(pType, pMark, pDoc);
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            Invalidate();
        break;
    case  RES_TOXMARK_DELETED:
        if( (void*)m_pTOXMark == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;
    }
}
/* -----------------------------16.10.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDocumentIndexMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        if(aTypeDepend.GetRegisteredIn())
            ((SwModify*)aTypeDepend.GetRegisteredIn())->Remove(&aTypeDepend);
        aLstnrCntnr.Disposing();
        m_pTOXMark = 0;
        m_pDoc = 0;
    }
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDocumentIndexes::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXDocumentIndexes");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDocumentIndexes::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexes") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXDocumentIndexes::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexes");
    return aRet;
}
/*-- 05.05.99 13:14:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexes::SwXDocumentIndexes(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}
/*-- 05.05.99 13:15:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexes::~SwXDocumentIndexes()
{
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXDocumentIndexes::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();

    sal_uInt32 nRet = 0;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexes::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();

    sal_Int32 nIdx = 0;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
            nIdx++ == nIndex )
            {
               const uno::Reference< text::XDocumentIndex > xTmp =
                   SwXDocumentIndex::CreateXDocumentIndex(
                       *GetDoc(), static_cast<SwTOXBaseSection const&>(*pSect));
               uno::Any aRet;
               aRet <<= xTmp;
               return aRet;
            }
    }

    throw lang::IndexOutOfBoundsException();
}

/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexes::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();


    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
                ((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
            {
               const uno::Reference< text::XDocumentIndex > xTmp =
                   SwXDocumentIndex::CreateXDocumentIndex(
                       *GetDoc(), static_cast<SwTOXBaseSection const&>(*pSect));
               uno::Any aRet;
               aRet <<= xTmp;
               return aRet;
            }
    }
    throw container::NoSuchElementException();
}
/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXDocumentIndexes::getElementNames(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    sal_Int32 nCount = 0;
    sal_uInt16 n;
    for( n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nCount;
    }

    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    sal_uInt16 nCnt;
    for( n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            pArray[nCnt++] = OUString(((SwTOXBaseSection*)pSect)->GetTOXName());
        }
    }
    return aRet;
}
/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXDocumentIndexes::hasByName(const OUString& rName)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();

    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            if(((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
                return sal_True;
        }
    }
    return sal_False;
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXDocumentIndexes::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference< text::XDocumentIndex> *)0);
}
/*-- 05.05.99 13:15:02---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXDocumentIndexes::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != getCount();
}

/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXIndexStyleAccess_Impl::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXIndexStyleAccess_Impl");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXIndexStyleAccess_Impl::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexParagraphStyles") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXIndexStyleAccess_Impl::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexParagraphStyles");
    return aRet;
}
/*-- 13.09.99 16:52:28---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexStyleAccess_Impl::SwXIndexStyleAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx)
{
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexStyleAccess_Impl::~SwXIndexStyleAccess_Impl()
{
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXIndexStyleAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if(nIndex < 0 || nIndex > MAXLEVEL)
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwTOXBase * pTOXBase( &rParent.m_pImpl->GetTOXSectionOrThrow() );

    uno::Sequence<OUString> aSeq;
    if(!(rElement >>= aSeq))
        throw lang::IllegalArgumentException();

    sal_Int32 nStyles = aSeq.getLength();
    const OUString* pStyles = aSeq.getConstArray();
    String sSetStyles;
    String aString;
    for(sal_Int32 i = 0; i < nStyles; i++)
    {
        if(i)
            sSetStyles += TOX_STYLE_DELIMITER;
        SwStyleNameMapper::FillUIName(pStyles[i], aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
        sSetStyles +=  aString;
    }
    pTOXBase->SetStyleNames(sSetStyles, (sal_uInt16) nIndex);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXIndexStyleAccess_Impl::getCount(void) throw( uno::RuntimeException )
{
    return MAXLEVEL;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXIndexStyleAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
                 uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if(nIndex < 0 || nIndex > MAXLEVEL)
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwTOXBase * pTOXBase( &rParent.m_pImpl->GetTOXSectionOrThrow() );

    const String& rStyles = pTOXBase->GetStyleNames((sal_uInt16) nIndex);
    sal_uInt16 nStyles = rStyles.GetTokenCount(TOX_STYLE_DELIMITER);
    uno::Sequence<OUString> aStyles(nStyles);
    OUString* pStyles = aStyles.getArray();
    String aString;
    for(sal_uInt16 i = 0; i < nStyles; i++)
    {
        SwStyleNameMapper::FillProgName(
            rStyles.GetToken(i, TOX_STYLE_DELIMITER),
            aString,
            nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
            sal_True);
        pStyles[i] = OUString( aString );
    }
    uno::Any aRet(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
    return aRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXIndexStyleAccess_Impl::getElementType(void)
    throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Sequence<OUString>*)0);
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXIndexStyleAccess_Impl::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

/* -----------------13.09.99 16:51-------------------

 --------------------------------------------------*/
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXIndexTokenAccess_Impl::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXIndexTokenAccess_Impl");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXIndexTokenAccess_Impl::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexLevelFormat") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXIndexTokenAccess_Impl::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexLevelFormat");
    return aRet;
}
/*-- 13.09.99 16:52:28---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexTokenAccess_Impl::SwXIndexTokenAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx),
    nCount(0)
{
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexTokenAccess_Impl::~SwXIndexTokenAccess_Impl()
{
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXIndexTokenAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwTOXBase * pTOXBase( &rParent.m_pImpl->GetTOXSectionOrThrow() );

    if(nIndex < 0 ||
        (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
            throw lang::IndexOutOfBoundsException();

    uno::Sequence<beans::PropertyValues> aSeq;
    if(!(rElement >>= aSeq))
        throw lang::IllegalArgumentException();


    String sPattern;
    sal_Int32 nTokens = aSeq.getLength();
    const beans::PropertyValues* pTokens = aSeq.getConstArray();
    for(sal_Int32 i = 0; i < nTokens; i++)
    {
        const beans::PropertyValue* pProperties = pTokens[i].getConstArray();
        sal_Int32 nProperties = pTokens[i].getLength();
        //create an invalid token
        SwFormToken aToken(TOKEN_END);
        for(sal_Int32 j = 0; j < nProperties; j++)
        {
            if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TokenType"))
            {
                const String sTokenType =
                        lcl_AnyToString(pProperties[j].Value);
                if(sTokenType.EqualsAscii("TokenEntryNumber"))
                    aToken.eTokenType = TOKEN_ENTRY_NO;
                else if(sTokenType.EqualsAscii("TokenEntryText" ))
                    aToken.eTokenType = TOKEN_ENTRY_TEXT;
                else if(sTokenType.EqualsAscii("TokenTabStop"   ))
                    aToken.eTokenType = TOKEN_TAB_STOP;
                else if(sTokenType.EqualsAscii("TokenText"      ))
                    aToken.eTokenType = TOKEN_TEXT;
                else if(sTokenType.EqualsAscii("TokenPageNumber"))
                    aToken.eTokenType = TOKEN_PAGE_NUMS;
                else if(sTokenType.EqualsAscii("TokenChapterInfo"      ))
                    aToken.eTokenType = TOKEN_CHAPTER_INFO;
                else if(sTokenType.EqualsAscii("TokenHyperlinkStart" ))
                    aToken.eTokenType = TOKEN_LINK_START;
                else if(sTokenType.EqualsAscii("TokenHyperlinkEnd"))
                    aToken.eTokenType = TOKEN_LINK_END;
                else if(sTokenType.EqualsAscii("TokenBibliographyDataField" ))
                    aToken.eTokenType = TOKEN_AUTHORITY;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("CharacterStyleName"  )  ))
            {
                String sCharStyleName;
                SwStyleNameMapper::FillUIName(
                        lcl_AnyToString(pProperties[j].Value),
                        sCharStyleName,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                        sal_True);
                aToken.sCharStyleName = sCharStyleName;
                aToken.nPoolId = SwStyleNameMapper::GetPoolIdFromUIName (
                            sCharStyleName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopRightAligned") ))
            {
                sal_Bool bRight = lcl_AnyToBool(pProperties[j].Value);
                aToken.eTabAlign = bRight ?
                                    SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopPosition"  )))
            {
                sal_Int32 nPosition = 0;
                if(pProperties[j].Value.getValueType() != ::getCppuType((sal_Int32*)0))
                    throw lang::IllegalArgumentException();
                pProperties[j].Value >>= nPosition;
                nPosition = MM100_TO_TWIP(nPosition);
                if(nPosition < 0)
                    throw lang::IllegalArgumentException();
                aToken.nTabStopPosition = nPosition;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopFillCharacter" )))
            {
                const String sFillChar =
                    lcl_AnyToString(pProperties[j].Value);
                if(sFillChar.Len() > 1)
                    throw lang::IllegalArgumentException();
                aToken.cTabFillChar = sFillChar.Len() ?
                                sFillChar.GetChar(0) : ' ';
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Text" )))
               {
                const String sText =
                    lcl_AnyToString(pProperties[j].Value);
                aToken.sText = sText;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ChapterFormat"    )))
            {
                sal_Int16 nFormat = lcl_AnyToInt16(pProperties[j].Value);
                switch(nFormat)
                {
                    case text::ChapterFormat::NUMBER:           nFormat = CF_NUMBER;
                    break;
                    case text::ChapterFormat::NAME:             nFormat = CF_TITLE;
                    break;
                    case text::ChapterFormat::NAME_NUMBER:      nFormat = CF_NUM_TITLE;
                    break;
                    case text::ChapterFormat::NO_PREFIX_SUFFIX:nFormat = CF_NUMBER_NOPREPST;
                    break;
                    case text::ChapterFormat::DIGIT:           nFormat = CF_NUM_NOPREPST_TITLE;
                    break;
                    default:
                        throw lang::IllegalArgumentException();
                }
                aToken.nChapterFormat = nFormat;
            }
//--->i53420
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ChapterLevel")) )
            {
                const sal_Int16 nLevel = lcl_AnyToInt16(pProperties[j].Value);
                if( nLevel < 1 || nLevel > MAXLEVEL )
                    throw lang::IllegalArgumentException();
                aToken.nOutlineLevel = nLevel;
            }
//<---
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("BibliographyDataField")))
            {
                sal_Int16 nType = 0;
                pProperties[j].Value >>= nType;
                if(nType < 0 || nType > text::BibliographyDataField::ISBN)
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = C2U("BibliographyDataField - wrong value");
                    aExcept.ArgumentPosition = static_cast< sal_Int16 >(j);
                    throw aExcept;
                }
                aToken.nAuthorityField = nType;
            }
            // #i21237#
            else if ( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("WithTab")))
            {
                aToken.bWithTab = lcl_AnyToBool(pProperties[j].Value);
            }

        }
        //exception if wrong TokenType
        if(TOKEN_END <= aToken.eTokenType )
            throw lang::IllegalArgumentException();
        // set TokenType from TOKEN_ENTRY_TEXT to TOKEN_ENTRY if it is
        // not a content index
        if(TOKEN_ENTRY_TEXT == aToken.eTokenType &&
                                TOX_CONTENT != pTOXBase->GetType())
            aToken.eTokenType = TOKEN_ENTRY;
//---> i53420
// check for chapter format allowed values if it was TOKEN_ENTRY_NO type
// only allowed value are CF_NUMBER and CF_NUM_NOPREPST_TITLE
// reading from file
        if( TOKEN_ENTRY_NO == aToken.eTokenType )
            switch(aToken.nChapterFormat)
            {
            case CF_NUMBER:
            case CF_NUM_NOPREPST_TITLE:
                break;
            default:
                throw lang::IllegalArgumentException();
            }
//<---
        sPattern += aToken.GetString();
    }
    SwForm aForm(pTOXBase->GetTOXForm());
    aForm.SetPattern((sal_uInt16) nIndex, sPattern);
    pTOXBase->SetTOXForm(aForm);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXIndexTokenAccess_Impl::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    const sal_Int32 nRet = rParent.m_pImpl->GetFormMax();
    return nRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXIndexTokenAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
         uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwTOXBase * pTOXBase( &rParent.m_pImpl->GetTOXSectionOrThrow() );

    if(nIndex < 0 ||
    (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
        throw lang::IndexOutOfBoundsException();

    // #i21237#
    SwFormTokens aPattern = pTOXBase->GetTOXForm().
        GetPattern((sal_uInt16) nIndex);
    SwFormTokens::iterator aIt = aPattern.begin();

    sal_uInt16 nTokenCount = 0;
    uno::Sequence< beans::PropertyValues > aRetSeq;
    String aString;
    while(aIt != aPattern.end()) // #i21237#
    {
        nTokenCount++;
        aRetSeq.realloc(nTokenCount);
        beans::PropertyValues* pTokenProps = aRetSeq.getArray();
        SwFormToken  aToken = *aIt; // #i21237#

        uno::Sequence< beans::PropertyValue >& rCurTokenSeq = pTokenProps[nTokenCount-1];
        SwStyleNameMapper::FillProgName(
                        aToken.sCharStyleName,
                        aString,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                        sal_True );
        const OUString aProgCharStyle( aString );
        switch(aToken.eTokenType)
        {
            case TOKEN_ENTRY_NO     :
            {
//--->i53420
// writing to file (from doc to properties)
                sal_Int32 nElements = 2;
                sal_Int32 nCurrentElement = 0;

                if( aToken.nChapterFormat != CF_NUMBER )//check for default value
                    nElements++;//we need the element
                if( aToken.nOutlineLevel != MAXLEVEL )
                    nElements++;

                rCurTokenSeq.realloc( nElements );

                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[nCurrentElement].Name = C2U("TokenType");
                pArr[nCurrentElement++].Value <<= OUString::createFromAscii("TokenEntryNumber");
//              pArr[0].Value <<= C2U("TokenEntryNumber");

                pArr[nCurrentElement].Name = C2U("CharacterStyleName");
                pArr[nCurrentElement++].Value <<= aProgCharStyle;
                if( aToken.nChapterFormat != CF_NUMBER )
                {
                    pArr[nCurrentElement].Name = C2U("ChapterFormat");
                    sal_Int16 nVal;
//! the allowed values for chapter format, when used as entry number, are CF_NUMBER and CF_NUM_NOPREPST_TITLE only, all else forced to
//CF_NUMBER
                    switch(aToken.nChapterFormat)
                    {
                    default:
                    case CF_NUMBER:             nVal = text::ChapterFormat::NUMBER; break;
                    case CF_NUM_NOPREPST_TITLE: nVal = text::ChapterFormat::DIGIT; break;
                    }
                    pArr[nCurrentElement++].Value <<= (sal_Int16)nVal;
                }

                if( aToken.nOutlineLevel != MAXLEVEL ) //only  a ChapterLevel != MAXLEVEL is registered
                {
                    pArr[nCurrentElement].Name = C2U("ChapterLevel");
                    pArr[nCurrentElement].Value <<= aToken.nOutlineLevel;
                }
//<---
            }
            break;
            case TOKEN_ENTRY        :   // no difference between Entry and Entry Text
            case TOKEN_ENTRY_TEXT   :
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenEntryText");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_TAB_STOP     :
            {
                rCurTokenSeq.realloc(5); // #i21237#
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenTabStop");


                if(SVX_TAB_ADJUST_END == aToken.eTabAlign)
                {
                    pArr[1].Name = C2U("TabStopRightAligned");
                    BOOL bTemp = sal_True;
                    pArr[1].Value.setValue(&bTemp, ::getCppuBooleanType());
                }
                else
                {
                    pArr[1].Name = C2U("TabStopPosition");
                    sal_Int32 nPos = (TWIP_TO_MM100(aToken.nTabStopPosition));
                    if(nPos < 0)
                        nPos = 0;
                    pArr[1].Value <<= (sal_Int32)nPos;
                }
                pArr[2].Name = C2U("TabStopFillCharacter");
                pArr[2].Value <<= OUString(aToken.cTabFillChar);
                pArr[3].Name = C2U("CharacterStyleName");
                pArr[3].Value <<= aProgCharStyle;
                // #i21237#
                pArr[4].Name = C2U("WithTab");
                pArr[4].Value.setValue(&aToken.bWithTab, ::getCppuBooleanType());
            }
            break;
            case TOKEN_TEXT         :
            {
                rCurTokenSeq.realloc( 3 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenText");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("Text");
                pArr[2].Value <<= OUString(aToken.sText);
            }
            break;
            case TOKEN_PAGE_NUMS    :
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenPageNumber");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_CHAPTER_INFO :
            {
                rCurTokenSeq.realloc( 4 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenChapterInfo");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("ChapterFormat");
                sal_Int16 nVal = text::ChapterFormat::NUMBER;
                switch(aToken.nChapterFormat)
                {
                    case CF_NUMBER:             nVal = text::ChapterFormat::NUMBER; break;
                    case CF_TITLE:              nVal = text::ChapterFormat::NAME; break;
                    case CF_NUM_TITLE:          nVal = text::ChapterFormat::NAME_NUMBER; break;
                    case CF_NUMBER_NOPREPST:    nVal = text::ChapterFormat::NO_PREFIX_SUFFIX; break;
                    case CF_NUM_NOPREPST_TITLE: nVal = text::ChapterFormat::DIGIT; break;
                }
                pArr[2].Value <<= (sal_Int16)nVal;
//--->i53420
                pArr[3].Name = C2U("ChapterLevel");
                //
                pArr[3].Value <<= aToken.nOutlineLevel;
//<---
            }
            break;
            case TOKEN_LINK_START   :
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenHyperlinkStart");
                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_LINK_END     :
            {
                rCurTokenSeq.realloc( 1 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenHyperlinkEnd");
            }
            break;
            case TOKEN_AUTHORITY :
            {
                rCurTokenSeq.realloc( 3 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenBibliographyDataField");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("BibliographyDataField");
                pArr[2].Value <<= sal_Int16(aToken.nAuthorityField);
            }
            break;

            default:
                ;
        }

        aIt++; // #i21237#
    }

    uno::Any aRet(&aRetSeq, ::getCppuType((uno::Sequence< beans::PropertyValues >*)0));

    return aRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXIndexTokenAccess_Impl::getElementType(void)
    throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Sequence< beans::PropertyValues >*)0);
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXIndexTokenAccess_Impl::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

