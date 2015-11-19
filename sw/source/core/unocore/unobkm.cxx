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

#include <unobookmark.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include <TextCursorHelper.hxx>
#include <unotextrange.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <doc.hxx>
#include <IDocumentState.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <docsh.hxx>
#include <xmloff/odffields.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/sequence.hxx>

using namespace ::sw::mark;
using namespace ::com::sun::star;

class SwXBookmark::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OInterfaceContainerHelper m_EventListeners;
    SwDoc *                     m_pDoc;
    ::sw::mark::IMark *         m_pRegisteredBookmark;
    OUString             m_sMarkName;

    Impl(   SwDoc *const pDoc, ::sw::mark::IMark *const /*pBookmark*/)
        : SwClient()
        , m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_pRegisteredBookmark(nullptr)
    {
        // DO NOT registerInMark here! (because SetXBookmark would delete rThis)
    }

    void registerInMark(SwXBookmark & rThis, ::sw::mark::IMark *const pBkmk);
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;

};

void SwXBookmark::Impl::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (GetRegisteredIn())
    {
        return; // core object still alive
    }

    m_pRegisteredBookmark = nullptr;
    m_pDoc = nullptr;
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

void SwXBookmark::Impl::registerInMark(SwXBookmark & rThis,
        ::sw::mark::IMark *const pBkmk)
{
    const uno::Reference<text::XTextContent> xBookmark(& rThis);
    if (pBkmk)
    {
        pBkmk->Add(this);
        ::sw::mark::MarkBase *const pMarkBase(dynamic_cast< ::sw::mark::MarkBase * >(pBkmk));
        OSL_ENSURE(pMarkBase, "registerInMark: no MarkBase?");
        if (pMarkBase)
        {
            pMarkBase->SetXBookmark(xBookmark);
        }
    }
    else if (m_pRegisteredBookmark)
    {
        m_sMarkName = m_pRegisteredBookmark->GetName();
        m_pRegisteredBookmark->Remove(this);
    }
    m_pRegisteredBookmark = pBkmk;
    // need a permanent Reference to initialize m_wThis
    m_wThis = xBookmark;
}

void SwXBookmark::registerInMark(SwXBookmark & rThis,
        ::sw::mark::IMark *const pBkmk)
{
    m_pImpl->registerInMark( rThis, pBkmk );
}

const ::sw::mark::IMark* SwXBookmark::GetBookmark() const
{
    return m_pImpl->m_pRegisteredBookmark;
}

SwXBookmark::SwXBookmark(
    ::sw::mark::IMark *const pBkmk,
    SwDoc *const pDoc)
    : m_pImpl( new SwXBookmark::Impl(pDoc, pBkmk) )
{
}

SwXBookmark::SwXBookmark()
    : m_pImpl( new SwXBookmark::Impl(nullptr, nullptr) )
{
}

SwXBookmark::~SwXBookmark()
{
}

uno::Reference<text::XTextContent> SwXBookmark::CreateXBookmark(
    SwDoc & rDoc,
    ::sw::mark::IMark *const pBookmark)
{
    // #i105557#: do not iterate over the registered clients: race condition
    ::sw::mark::MarkBase *const pMarkBase(dynamic_cast< ::sw::mark::MarkBase * >(pBookmark));
    OSL_ENSURE(!pBookmark || pMarkBase, "CreateXBookmark: no MarkBase?");
    uno::Reference<text::XTextContent> xBookmark;
    if (pMarkBase)
    {
        xBookmark = pMarkBase->GetXBookmark();
    }
    if (!xBookmark.is())
    {
        OSL_ENSURE(!pBookmark ||
            dynamic_cast< ::sw::mark::IBookmark* >(pBookmark) ||
            IDocumentMarkAccess::GetType(*pBookmark) == IDocumentMarkAccess::MarkType::ANNOTATIONMARK,
            "<SwXBookmark::GetObject(..)>"
            "SwXBookmark requested for non-bookmark mark and non-annotation mark.");
        SwXBookmark *const pXBookmark =
            (pBookmark) ? new SwXBookmark(pBookmark, &rDoc) : new SwXBookmark;
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
    return nullptr;
}

namespace
{
    class theSwXBookmarkUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXBookmarkUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXBookmark::getUnoTunnelId()
{
    return theSwXBookmarkUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXBookmark::getSomething( const uno::Sequence< sal_Int8 >& rId )
throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXBookmark>(rId, this);
}

void SwXBookmark::attachToRangeEx(
    const uno::Reference< text::XTextRange > & xTextRange,
    IDocumentMarkAccess::MarkType eType)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if (m_pImpl->m_pRegisteredBookmark)
    {
        throw uno::RuntimeException();
    }

    const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
            xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    if(xRangeTunnel.is())
    {
        pRange = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    SwDoc *const pDoc =
        (pRange) ? &pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : nullptr);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    m_pImpl->m_pDoc = pDoc;
    SwUnoInternalPaM aPam(*m_pImpl->m_pDoc);
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aCont(m_pImpl->m_pDoc);
    bool isHorribleHackIgnoreDuplicates(false);
    if (m_pImpl->m_sMarkName.isEmpty())
    {
         m_pImpl->m_sMarkName = "Bookmark";
    }
    if ((eType == IDocumentMarkAccess::MarkType::BOOKMARK) &&
        ::sw::mark::CrossRefNumItemBookmark::IsLegalName(m_pImpl->m_sMarkName))
    {
        eType = IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK;
    }
    else if ((eType == IDocumentMarkAccess::MarkType::BOOKMARK) &&
        ::sw::mark::CrossRefHeadingBookmark::IsLegalName(m_pImpl->m_sMarkName) &&
        IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( aPam ) )
    {
        eType = IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK;
        // tdf#94804 LO 4.2-5.0 create invalid duplicates that must be preserved
        // note: do not check meta:generator, may be preserved by other versions
        if (m_pImpl->m_pDoc->IsInXMLImport())
        {
            isHorribleHackIgnoreDuplicates = true;
        }
    }
    m_pImpl->registerInMark(*this,
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->makeMark(
            aPam, m_pImpl->m_sMarkName, eType, isHorribleHackIgnoreDuplicates));
    // #i81002#
    // Check, if bookmark has been created.
    // E.g., the creation of a cross-reference bookmark is suppress,
    // if the PaM isn't a valid one for cross-reference bookmarks.
    if (!m_pImpl->m_pRegisteredBookmark)
    {
        OSL_FAIL("<SwXBookmark::attachToRange(..)>"
            " - could not create Mark.");
        throw lang::IllegalArgumentException();
    }
}

void SwXBookmark::attachToRange( const uno::Reference< text::XTextRange > & xTextRange )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    attachToRangeEx(xTextRange, IDocumentMarkAccess::MarkType::BOOKMARK);
}

void SAL_CALL SwXBookmark::attach( const uno::Reference< text::XTextRange > & xTextRange )
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange > SAL_CALL SwXBookmark::getAnchor()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_pRegisteredBookmark)
    {
        throw uno::RuntimeException();
    }
    return SwXTextRange::CreateXTextRange(
            *m_pImpl->m_pDoc,
            m_pImpl->m_pRegisteredBookmark->GetMarkPos(),
            (m_pImpl->m_pRegisteredBookmark->IsExpanded())
                ? &m_pImpl->m_pRegisteredBookmark->GetOtherMarkPos() : nullptr);
}

void SAL_CALL SwXBookmark::dispose()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_pImpl->m_pRegisteredBookmark)
    {
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->deleteMark( m_pImpl->m_pRegisteredBookmark );
    }
}

void SAL_CALL SwXBookmark::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXBookmark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

OUString SAL_CALL SwXBookmark::getName()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return (m_pImpl->m_pRegisteredBookmark)
        ?   m_pImpl->m_pRegisteredBookmark->GetName()
        :   m_pImpl->m_sMarkName;
}

void SAL_CALL SwXBookmark::setName(const OUString& rName)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_pRegisteredBookmark)
    {
        m_pImpl->m_sMarkName = rName;
    }
    if (!m_pImpl->m_pRegisteredBookmark || (getName() == rName))
    {
        return;
    }
    IDocumentMarkAccess *const pMarkAccess =
        m_pImpl->m_pDoc->getIDocumentMarkAccess();
    if(pMarkAccess->findMark(rName) != pMarkAccess->getAllMarksEnd())
    {
        throw uno::RuntimeException();
    }

    SwPaM aPam(m_pImpl->m_pRegisteredBookmark->GetMarkPos());
    if (m_pImpl->m_pRegisteredBookmark->IsExpanded())
    {
        aPam.SetMark();
        *aPam.GetMark() = m_pImpl->m_pRegisteredBookmark->GetOtherMarkPos();
    }

    pMarkAccess->renameMark(m_pImpl->m_pRegisteredBookmark, rName);
}

OUString SAL_CALL
SwXBookmark::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXBookmark");
}

static char const*const g_ServicesBookmark[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.Bookmark",
    "com.sun.star.document.LinkTarget",
};
static const size_t g_nServicesBookmark(
    sizeof(g_ServicesBookmark)/sizeof(g_ServicesBookmark[0]));

sal_Bool SAL_CALL SwXBookmark::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBookmark::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesBookmark, g_ServicesBookmark);
}

// MetadatableMixin
::sfx2::Metadatable* SwXBookmark::GetCoreObject()
{
    return dynamic_cast< ::sfx2::Metadatable* >(m_pImpl->m_pRegisteredBookmark);
}

uno::Reference<frame::XModel> SwXBookmark::GetModel()
{
    if (m_pImpl->m_pDoc)
    {
        SwDocShell const * const pShell( m_pImpl->m_pDoc->GetDocShell() );
        return (pShell) ? pShell->GetModel() : nullptr;
    }
    return nullptr;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXBookmark::getPropertySetInfo() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xRef(
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_BOOKMARK)
            ->getPropertySetInfo() );
    return xRef;
}

void SAL_CALL
SwXBookmark::setPropertyValue(const OUString& PropertyName,
        const uno::Any& /*rValue*/)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    // nothing to set here
    throw lang::IllegalArgumentException("Property is read-only: "
            + PropertyName, static_cast< cppu::OWeakObject * >(this), 0 );
}

uno::Any SAL_CALL SwXBookmark::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    uno::Any aRet;
    if (! ::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName == UNO_LINK_DISPLAY_NAME)
        {
            aRet <<= getName();
        }
    }
    return aRet;
}

void SAL_CALL
SwXBookmark::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXBookmark::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXBookmark::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXBookmark::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXBookmark::removeVetoableChangeListener(): not implemented");
}

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, ::sw::mark::IMark* pBkm, SwDoc* pDc)
    : SwXFieldmark_Base(pBkm, pDc)
    , isReplacementObject(_isReplacementObject)
{ }

void SwXFieldmarkParameters::insertByName(const OUString& aName, const uno::Any& aElement)
    throw (lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    if(pParameters->find(aName) != pParameters->end())
        throw container::ElementExistException();
    (*pParameters)[aName] = aElement;
}

void SwXFieldmarkParameters::removeByName(const OUString& aName)
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!getCoreParameters()->erase(aName))
        throw container::NoSuchElementException();
}

void SwXFieldmarkParameters::replaceByName(const OUString& aName, const uno::Any& aElement)
    throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    pEntry->second = aElement;
}

uno::Any SwXFieldmarkParameters::getByName(const OUString& aName)
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    return pEntry->second;
}

uno::Sequence<OUString> SwXFieldmarkParameters::getElementNames()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return comphelper::mapKeysToSequence(*pParameters);
}

sal_Bool SwXFieldmarkParameters::hasByName(const OUString& aName)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return (pParameters->find(aName) != pParameters->end());
}

uno::Type SwXFieldmarkParameters::getElementType()
    throw (uno::RuntimeException, std::exception)
{
    return ::cppu::UnoType<void>::get();
}

sal_Bool SwXFieldmarkParameters::hasElements()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return !getCoreParameters()->empty();
}

void SwXFieldmarkParameters::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

IFieldmark::parameter_map_t* SwXFieldmarkParameters::getCoreParameters()
    throw (uno::RuntimeException)
{
    const IFieldmark* pFieldmark = dynamic_cast< const IFieldmark* >(GetRegisteredIn());
    if(!pFieldmark)
        throw uno::RuntimeException();
    return const_cast< IFieldmark* >(pFieldmark)->GetParameters();
}

void SwXFieldmark::attachToRange( const uno::Reference < text::XTextRange >& xTextRange )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{

    attachToRangeEx( xTextRange,
                     ( isReplacementObject ? IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK : IDocumentMarkAccess::MarkType::TEXT_FIELDMARK ) );
}

OUString SwXFieldmark::getFieldType()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const IFieldmark *pBkm = dynamic_cast<const IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw uno::RuntimeException();
    return pBkm->GetFieldname();
}

void SwXFieldmark::setFieldType(const OUString & fieldType)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark *pBkm = const_cast<IFieldmark*>(
        dynamic_cast<const IFieldmark*>(GetBookmark()));
    if(!pBkm)
        throw uno::RuntimeException();
    pBkm->SetFieldname(fieldType);
}

uno::Reference<container::XNameContainer> SwXFieldmark::getParameters()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IFieldmark *pBkm = const_cast<IFieldmark*>(
        dynamic_cast<const IFieldmark*>(GetBookmark()));
    if(!pBkm)
        throw uno::RuntimeException();
    return uno::Reference<container::XNameContainer>(new SwXFieldmarkParameters(pBkm));
}

uno::Reference<text::XTextContent>
SwXFieldmark::CreateXFieldmark(SwDoc & rDoc, ::sw::mark::IMark *const pMark,
        bool const isReplacementObject)
{
    // #i105557#: do not iterate over the registered clients: race condition
    ::sw::mark::MarkBase *const pMarkBase(
        dynamic_cast< ::sw::mark::MarkBase * >(pMark));
    assert(!pMark || pMarkBase);
    uno::Reference<text::XTextContent> xMark;
    if (pMarkBase)
    {
        xMark = pMarkBase->GetXBookmark();
    }
    if (!xMark.is())
    {
        // FIXME: These belong in XTextFieldsSupplier
        SwXFieldmark* pXBkmk = nullptr;
        if (dynamic_cast< ::sw::mark::TextFieldmark* >(pMark))
            pXBkmk = new SwXFieldmark(false, pMark, &rDoc);
        else if (dynamic_cast< ::sw::mark::CheckboxFieldmark* >(pMark))
            pXBkmk = new SwXFieldmark(true, pMark, &rDoc);
        else
            pXBkmk = new SwXFieldmark(isReplacementObject, nullptr, &rDoc);

        xMark.set(pXBkmk);
        pXBkmk->registerInMark(*pXBkmk, pMarkBase);
    }
    return xMark;
}

::sw::mark::ICheckboxFieldmark*
SwXFieldmark::getCheckboxFieldmark()
{
    ::sw::mark::ICheckboxFieldmark* pCheckboxFm = nullptr;
    if ( getFieldType() == ODF_FORMCHECKBOX )
    {
        // evil #TODO #FIXME casting away the const-ness
        pCheckboxFm = const_cast<sw::mark::ICheckboxFieldmark*>(dynamic_cast< const ::sw::mark::ICheckboxFieldmark* >( GetBookmark()));
        OSL_ASSERT( GetBookmark() == nullptr || pCheckboxFm != nullptr );
            // unclear to me whether GetBookmark() can be null here
    }
    return  pCheckboxFm;

}

// support 'hidden' "Checked" property ( note: this property is just for convenience to support
// docx import filter thus not published via PropertySet info )

void SAL_CALL
SwXFieldmark::setPropertyValue(const OUString& PropertyName,
        const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    if ( PropertyName == "Checked" )
    {
        ::sw::mark::ICheckboxFieldmark* pCheckboxFm = getCheckboxFieldmark();
        bool bChecked( false );
        if ( pCheckboxFm && ( rValue >>= bChecked ) )
            pCheckboxFm->SetChecked( bChecked );
        else
            throw uno::RuntimeException();

    }
    else
        SwXFieldmark_Base::setPropertyValue( PropertyName, rValue );
}

// support 'hidden' "Checked" property ( note: this property is just for convenience to support
// docx import filter thus not published via PropertySet info )

uno::Any SAL_CALL SwXFieldmark::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    if ( rPropertyName == "Checked" )
    {
        ::sw::mark::ICheckboxFieldmark* pCheckboxFm = getCheckboxFieldmark();
        if ( pCheckboxFm )
            return uno::makeAny( pCheckboxFm->IsChecked() );
        else
            throw uno::RuntimeException();
    }
    return SwXFieldmark_Base::getPropertyValue( rPropertyName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
