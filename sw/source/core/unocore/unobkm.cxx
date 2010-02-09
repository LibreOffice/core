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

#include <unobookmark.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <TextCursorHelper.hxx>
#include <unotextrange.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoevtlstnr.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#include <undobj.hxx>
#include <docsh.hxx>


using namespace ::sw::mark;
using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace
{
    static OUString lcl_QuoteName(const OUString& rName)
    {
        static const OUString sStart = OUString(String(SW_RES(STR_START_QUOTE)));
        static const OUString sEnd = OUString(String(SW_RES(STR_END_QUOTE)));
        ::rtl::OUStringBuffer sBuf(64);
        return sBuf.append(sStart).append(rName).append(sEnd).makeStringAndClear();
    }
}

/******************************************************************
 * SwXBookmark
 ******************************************************************/

class SwXBookmark::Impl
    : public SwClient
{

public:
    SwEventListenerContainer    m_ListenerContainer;
    SwDoc *                     m_pDoc;
    ::sw::mark::IMark *         m_pRegisteredBookmark;
    ::rtl::OUString             m_sMarkName;


    Impl(   SwXBookmark & rThis,
            SwDoc *const pDoc, ::sw::mark::IMark *const /*pBookmark*/)
        : SwClient()
        , m_ListenerContainer(static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_pDoc(pDoc)
        , m_pRegisteredBookmark(0)
    {
        // DO NOT regiserInMark here! (because SetXBookmark would delete rThis)
    }

    void registerInMark(SwXBookmark & rThis, ::sw::mark::IMark *const pBkmk);

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

void SwXBookmark::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (!GetRegisteredIn())
    {
        m_pRegisteredBookmark = NULL;
        m_pDoc = NULL;
        m_ListenerContainer.Disposing();
    }
}

void SwXBookmark::Impl::registerInMark(SwXBookmark & rThis,
        ::sw::mark::IMark *const pBkmk)
{
    if (pBkmk)
    {
        pBkmk->Add(this);
        ::sw::mark::MarkBase *const pMarkBase(
            dynamic_cast< ::sw::mark::MarkBase * >(pBkmk));
        OSL_ENSURE(pMarkBase, "registerInMark: no MarkBase?");
        if (pMarkBase)
        {
            const uno::Reference<text::XTextContent> xBookmark(& rThis);
            pMarkBase->SetXBookmark(xBookmark);
        }
    }
    else if (m_pRegisteredBookmark)
    {
        m_sMarkName = m_pRegisteredBookmark->GetName();
        m_pRegisteredBookmark->Remove(this);
    }
    m_pRegisteredBookmark = pBkmk;
}


const ::sw::mark::IMark* SwXBookmark::GetBookmark() const
{
    return m_pImpl->m_pRegisteredBookmark;
}

SwXBookmark::SwXBookmark(::sw::mark::IMark *const pBkmk, SwDoc *const pDoc)
    : m_pImpl( new SwXBookmark::Impl(*this, pDoc, pBkmk) )
{
}

SwXBookmark::SwXBookmark()
    : m_pImpl( new SwXBookmark::Impl(*this, 0, 0) )
{
}

SwXBookmark::~SwXBookmark()
{
}

uno::Reference<text::XTextContent>
SwXBookmark::CreateXBookmark(SwDoc & rDoc, ::sw::mark::IMark & rBookmark)
{
    // #i105557#: do not iterate over the registered clients: race condition
    ::sw::mark::MarkBase *const pMarkBase(
        dynamic_cast< ::sw::mark::MarkBase * >(&rBookmark));
    OSL_ENSURE(pMarkBase, "CreateXBookmark: no MarkBase?");
    if (!pMarkBase) { return 0; }
    uno::Reference<text::XTextContent> xBookmark(pMarkBase->GetXBookmark());
    if (!xBookmark.is())
    {
        // FIXME: These belong in XTextFieldsSupplier
        //if (dynamic_cast< ::sw::mark::TextFieldmark* >(&rBkmk))
        //    pXBkmk = new SwXFieldmark(false, &rBkmk, pDoc);
        //else if (dynamic_cast< ::sw::mark::CheckboxFieldmark* >(&rBkmk))
        //    pXBkmk = new SwXFieldmark(true, &rBkmk, pDoc);
        //else
        OSL_ENSURE(
            dynamic_cast< ::sw::mark::IBookmark* >(&rBookmark),
            "<SwXBookmark::GetObject(..)>"
            "SwXBookmark requested for non-bookmark mark.");
        SwXBookmark *const pXBookmark = new SwXBookmark(&rBookmark, &rDoc);
        xBookmark.set(pXBookmark);
        pXBookmark->m_pImpl->registerInMark(*pXBookmark, pMarkBase);
    }
    return xBookmark;
}

::sw::mark::IMark const* SwXBookmark::GetBookmarkInDoc(SwDoc const*const pDoc,
        const uno::Reference< lang::XUnoTunnel> & xUT)
{
    SwXBookmark *const pXBkm(
            ::sw::UnoTunnelGetImplementation<SwXBookmark>(xUT));
    if (pXBkm && (pDoc == pXBkm->m_pImpl->m_pDoc))
    {
        return pXBkm->m_pImpl->m_pRegisteredBookmark;
    }
    return 0;
}

const uno::Sequence< sal_Int8 > & SwXBookmark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL
SwXBookmark::getSomething(const uno::Sequence< sal_Int8 >& rId)
    throw(uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXBookmark>(rId, this);
    }

void SwXBookmark::attachToRangeEx(
    const uno::Reference< text::XTextRange > & xTextRange,
    IDocumentMarkAccess::MarkType eType)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if (m_pImpl->m_pRegisteredBookmark)
    {
        throw uno::RuntimeException();
    }

    const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
            xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    SwDoc *const pDoc =
        (pRange) ? pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : 0);
    if (!pDoc)
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

// MetadatableMixin
::sfx2::Metadatable* SwXBookmark::GetCoreObject()
{
    return dynamic_cast< ::sfx2::Metadatable* >( GetBookmark() );
}

uno::Reference<frame::XModel> SwXBookmark::GetModel()
{
    if (GetDoc())
    {
        SwDocShell const * const pShell( GetDoc()->GetDocShell() );
        return (pShell) ? pShell->GetModel() : 0;
    }
    return 0;
}


uno::Reference< beans::XPropertySetInfo >  SwXBookmark::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef;
    if(!aRef.is())
    {
        aRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_BOOKMARK)->getPropertySetInfo();
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


void SwXFieldmarkParameters::insertByName(const OUString& aName, const Any& aElement)
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    if(pParameters->find(aName) != pParameters->end())
        throw ElementExistException();
    (*pParameters)[aName] = aElement;
}

void SwXFieldmarkParameters::removeByName(const OUString& aName)
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!getCoreParameters()->erase(aName))
        throw NoSuchElementException();
}

void SwXFieldmarkParameters::replaceByName(const OUString& aName, const Any& aElement)
    throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw NoSuchElementException();
    pEntry->second = aElement;
}

Any SwXFieldmarkParameters::getByName(const OUString& aName)
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw NoSuchElementException();
    return pEntry->second;
}

Sequence<OUString> SwXFieldmarkParameters::getElementNames()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    Sequence<OUString> vResult(pParameters->size());
    OUString* pOutEntry = vResult.getArray();
    for(IFieldmark::parameter_map_t::iterator pEntry = pParameters->begin(); pEntry!=pParameters->end(); ++pEntry, ++pOutEntry)
        *pOutEntry = pEntry->first;
    return vResult;
}

::sal_Bool SwXFieldmarkParameters::hasByName(const OUString& aName)
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return (pParameters->find(aName) != pParameters->end());
}

Type SwXFieldmarkParameters::getElementType()
    throw (RuntimeException)
{
    return ::cppu::UnoType< ::cppu::UnoVoidType>::get();
}

::sal_Bool SwXFieldmarkParameters::hasElements()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return !getCoreParameters()->empty();
}

void SwXFieldmarkParameters::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}


IFieldmark::parameter_map_t* SwXFieldmarkParameters::getCoreParameters()
    throw (RuntimeException)
{
    const IFieldmark* pFieldmark = dynamic_cast< const IFieldmark* >(GetRegisteredIn());
    if(!pFieldmark)
        throw RuntimeException();
    return const_cast< IFieldmark* >(pFieldmark)->GetParameters();
}

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, ::sw::mark::IMark* pBkm, SwDoc* pDc)
    : SwXFieldmark_Base(pBkm, pDc)
    , isReplacementObject(_isReplacementObject)
{ }

void SwXFieldmark::attachToRange( const uno::Reference < text::XTextRange >& xTextRange )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    attachToRangeEx( xTextRange,
                     ( isReplacementObject ? IDocumentMarkAccess::CHECKBOX_FIELDMARK : IDocumentMarkAccess::TEXT_FIELDMARK ) );
}

::rtl::OUString SwXFieldmark::getFieldType(void)
    throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const IFieldmark *pBkm = dynamic_cast<const IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw RuntimeException();
    return pBkm->GetFieldname();
}

void SwXFieldmark::setFieldType(const::rtl::OUString & fieldType)
    throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark *pBkm = dynamic_cast<IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw RuntimeException();
    pBkm->SetFieldname(fieldType);
}

Reference<XNameContainer> SwXFieldmark::getParameters()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IFieldmark *pBkm = dynamic_cast<IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw uno::RuntimeException();
    return Reference<XNameContainer>(new SwXFieldmarkParameters(pBkm));
}
