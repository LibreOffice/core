/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unobkm.cxx,v $
 * $Revision: 1.17 $
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


#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#include <undobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace
{
    static OUString lcl_QuoteName(const OUString& rName)
    {
        static const OUString sStart = OUString(String(SW_RES(STR_START_QUOTE)));
        static const OUString sEnd = OUString(String(SW_RES(STR_END_QUOTE)));
        OUStringBuffer sBuf(64);
        return sBuf.append(sStart).append(rName).append(sEnd).makeStringAndClear();
    }
}

/******************************************************************
 * SwXBookmark
 ******************************************************************/
TYPEINIT1(SwXBookmark, SwClient)

const uno::Sequence< sal_Int8 > & SwXBookmark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXBookmark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

SwXBookmark::SwXBookmark(::sw::mark::IMark* pBkmk, SwDoc* pDoc)
    : m_aLstnrCntnr((text::XTextContent*)this)
    , m_pDoc(pDoc)
    , m_pRegisteredBookmark(NULL)
{
    registerInMark(pBkmk);
}

SwXBookmark::~SwXBookmark()
{ }

void SwXBookmark::attachToRangeEx(
    const uno::Reference< text::XTextRange > & xTextRange,
    IDocumentMarkAccess::MarkType eType)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if(m_pRegisteredBookmark)
        throw uno::RuntimeException();

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {

        pRange = reinterpret_cast< SwXTextRange * >(
            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId() )));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    }

    SwDoc* pDc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ?
        (SwDoc*)pCursor->GetDoc() : 0;
    if(pDc)
    {
        m_pDoc = pDc;
        SwUnoInternalPaM aPam(*m_pDoc);
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        UnoActionContext aCont(m_pDoc);
        if(!m_aName.Len())
             m_aName =  OUString::createFromAscii("Bookmark");
        if(eType == IDocumentMarkAccess::BOOKMARK && ::sw::mark::CrossRefNumItemBookmark::IsLegalName(m_aName))
            eType = IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK;
        else if(eType == IDocumentMarkAccess::BOOKMARK && ::sw::mark::CrossRefHeadingBookmark::IsLegalName(m_aName))
            eType = IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK;
        registerInMark(m_pDoc->getIDocumentMarkAccess()->makeMark(aPam, m_aName, eType));
        // --> OD 2007-10-23 #i81002#
        // Check, if bookmark has been created.
        // E.g., the creation of a cross-reference bookmark is suppress,
        // if the PaM isn't a valid one for cross-reference bookmarks.
        if(!m_pRegisteredBookmark)
        {
            OSL_ENSURE(false,
                "<SwXBookmark::attachToRange(..)>"
                " - could not create Mark.");
            throw lang::IllegalArgumentException();
        }
        // <--
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXBookmark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    attachToRangeEx(xTextRange, IDocumentMarkAccess::BOOKMARK);
}

void SwXBookmark::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange > SwXBookmark::getAnchor(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange > aRet;
    if(m_pRegisteredBookmark)
        aRet = SwXTextRange::CreateTextRangeFromPosition(
            m_pDoc,
            m_pRegisteredBookmark->GetMarkPos(),
            m_pRegisteredBookmark->IsExpanded() ? &m_pRegisteredBookmark->GetOtherMarkPos() : NULL);
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXBookmark::dispose(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(m_pRegisteredBookmark)
        GetDoc()->getIDocumentMarkAccess()->deleteMark(m_pRegisteredBookmark);
    else
        throw uno::RuntimeException();
}

void SwXBookmark::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!m_pRegisteredBookmark)
        throw uno::RuntimeException();
    m_aLstnrCntnr.AddListener(aListener);
}

void SwXBookmark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!m_pRegisteredBookmark || !m_aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

OUString SwXBookmark::getName(void)
    throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sRet;
    if(m_pRegisteredBookmark)
        sRet = m_pRegisteredBookmark->GetName();
    else
        sRet = m_aName;
    return sRet;
}

void SwXBookmark::setName(const OUString& rName)
    throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!m_pRegisteredBookmark)
        m_aName = rName;
    if(!m_pRegisteredBookmark || getName() == rName)
        return;
    IDocumentMarkAccess* const pMarkAccess = m_pDoc->getIDocumentMarkAccess();
    if(pMarkAccess->findMark(rName) != pMarkAccess->getMarksEnd())
        throw uno::RuntimeException();

    SwPaM aPam(m_pRegisteredBookmark->GetMarkPos());
    if(m_pRegisteredBookmark->IsExpanded())
    {
        aPam.SetMark();
        *aPam.GetMark() = m_pRegisteredBookmark->GetOtherMarkPos();
    }

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, lcl_QuoteName(getName()));
    aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
    aRewriter.AddRule(UNDO_ARG3, lcl_QuoteName(rName));

    m_pDoc->StartUndo(UNDO_BOOKMARK_RENAME, &aRewriter);
    pMarkAccess->renameMark(m_pRegisteredBookmark, rName);
    m_pDoc->EndUndo(UNDO_BOOKMARK_RENAME, NULL);
}

OUString SwXBookmark::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString::createFromAscii("SwXBookmark");
}

sal_Bool SwXBookmark::supportsService(const OUString& rServiceName)
    throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.Bookmark") ||
                !rServiceName.compareToAscii("com.sun.star.document.LinkTarget") ||
                    !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}

uno::Sequence< OUString > SwXBookmark::getSupportedServiceNames(void)
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    aRet[0] = OUString::createFromAscii("com.sun.star.text.Bookmark");
    aRet[1] = OUString::createFromAscii("com.sun.star.document.LinkTarget");
    aRet[2] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}

void SwXBookmark::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pRegisteredBookmark = NULL;
        m_pDoc = NULL;
        m_aLstnrCntnr.Disposing();
    }
}

uno::Reference< beans::XPropertySetInfo >  SwXBookmark::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef;
    if(!aRef.is())
    {
        const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_BOOKMARK);
        uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(pMap);
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        aRef = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return aRef;
}

void SwXBookmark::setPropertyValue(const OUString& PropertyName, const uno::Any& /*aValue*/)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
        lang::WrappedTargetException,
        uno::RuntimeException )
{
    // nothing to set here
    throw IllegalArgumentException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + PropertyName, static_cast < cppu::OWeakObject * > ( this ), 0 );
}

uno::Any SwXBookmark::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
            aRet <<= getName();
    }
    return aRet;
}

void SwXBookmark::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{ }

void SwXBookmark::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
            throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{ }

void SwXBookmark::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{ }

void SwXBookmark::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{ }

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, ::sw::mark::IMark* pBkm, SwDoc* pDc)
    : SwXFieldmark_BASE(pBkm, pDc)
    , isReplacementObject(_isReplacementObject)
{ }

void SwXFieldmark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    attachToRangeEx(xTextRange, (isReplacementObject?IDocumentMarkAccess::CHECKBOX_FIELDMARK:IDocumentMarkAccess::TEXT_FIELDMARK));
}

::rtl::OUString SwXFieldmark::getDescription(void) throw( ::com::sun::star::uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
//        TODO implement...
//        if(!GetBookmark())
    ::sw::mark::IFieldmark const * const pMark =
        dynamic_cast< ::sw::mark::IFieldmark const * const>(GetBookmark());
    if(!pMark)
        throw uno::RuntimeException();
    return pMark->GetFieldHelptext();
}

::sal_Int16 SAL_CALL SwXFieldmark::getType() throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::sw::mark::ICheckboxFieldmark const * const pAsCheckbox =
        dynamic_cast< ::sw::mark::ICheckboxFieldmark const * const>(GetBookmark());
    if(pAsCheckbox)
        return 1;
    return 0;
}

::sal_Int16 SAL_CALL SwXFieldmark::getRes() throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::sw::mark::ICheckboxFieldmark const * const pAsCheckbox =
        dynamic_cast< ::sw::mark::ICheckboxFieldmark const * const>(GetBookmark());
    if(pAsCheckbox && pAsCheckbox->IsChecked())
        return 1;
    return 0;
}

//FIXME Remove Method
void SAL_CALL SwXFieldmark::setType( ::sal_Int16 ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    throw uno::RuntimeException();
}

//FIXME Remove Method
void SAL_CALL SwXFieldmark::setRes( ::sal_Int16  ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    throw uno::RuntimeException();
}

void SAL_CALL SwXFieldmark::setDescription( const ::rtl::OUString& description )
    throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const ::sw::mark::IFieldmark* pMark =
        dynamic_cast<const ::sw::mark::IFieldmark*>(GetBookmark());
    if(pMark)
        const_cast< ::sw::mark::IFieldmark*>(pMark)->SetFieldHelptext(description);
    else
        throw uno::RuntimeException();
}
