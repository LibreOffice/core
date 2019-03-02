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

#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <svl/itemprop.hxx>
#include <svl/listener.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/odffields.hxx>

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

using namespace ::sw::mark;
using namespace ::com::sun::star;

class SwXBookmark::Impl
    : public SvtListener
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    SwDoc* m_pDoc;
    ::sw::mark::IMark* m_pRegisteredBookmark;
    OUString m_sMarkName;
    bool m_bHidden;
    OUString m_HideCondition;

    Impl( SwDoc *const pDoc )
        : m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_pRegisteredBookmark(nullptr)
        , m_bHidden(false)
    {
        // DO NOT registerInMark here! (because SetXBookmark would delete rThis)
    }

    void registerInMark(SwXBookmark & rThis, ::sw::mark::IMark *const pBkmk);
protected:
    virtual void Notify(const SfxHint&) override;

};

void SwXBookmark::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
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
}

void SwXBookmark::Impl::registerInMark(SwXBookmark& rThis,
        ::sw::mark::IMark* const pBkmk)
{
    const uno::Reference<text::XTextContent> xBookmark(&rThis);
    if (pBkmk)
    {
        EndListeningAll();
        StartListening(pBkmk->GetNotifier());
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

        // the following applies only to bookmarks (not to fieldmarks)
        IBookmark* pBookmark = dynamic_cast<IBookmark*>(m_pRegisteredBookmark);
        if (pBookmark)
        {
            m_bHidden = pBookmark->IsHidden();
            m_HideCondition = pBookmark->GetHideCondition();
        }
        EndListeningAll();
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

IDocumentMarkAccess* SwXBookmark::GetIDocumentMarkAccess()
{
    return m_pImpl->m_pDoc->getIDocumentMarkAccess();
}

SwXBookmark::SwXBookmark(SwDoc *const pDoc)
    : m_pImpl( new SwXBookmark::Impl(pDoc) )
{
}

SwXBookmark::SwXBookmark()
    : m_pImpl( new SwXBookmark::Impl(nullptr) )
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
            pBookmark ? new SwXBookmark(&rDoc) : new SwXBookmark;
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
{
    return ::sw::UnoTunnelImpl<SwXBookmark>(rId, this);
}

void SwXBookmark::attachToRangeEx(
    const uno::Reference< text::XTextRange > & xTextRange,
    IDocumentMarkAccess::MarkType eType)
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
        pRange ? &pRange->GetDoc() : (pCursor ? pCursor->GetDoc() : nullptr);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    m_pImpl->m_pDoc = pDoc;
    SwUnoInternalPaM aPam(*m_pImpl->m_pDoc);
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aCont(m_pImpl->m_pDoc);
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
    }
    m_pImpl->registerInMark(*this,
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->makeMark(
            aPam, m_pImpl->m_sMarkName, eType, ::sw::mark::InsertMode::New));
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
{
    attachToRangeEx(xTextRange, IDocumentMarkAccess::MarkType::BOOKMARK);
}

void SAL_CALL SwXBookmark::attach( const uno::Reference< text::XTextRange > & xTextRange )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange > SAL_CALL SwXBookmark::getAnchor()
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
{
    SolarMutexGuard aGuard;
    if (m_pImpl->m_pRegisteredBookmark)
    {
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->deleteMark( m_pImpl->m_pRegisteredBookmark );
    }
}

void SAL_CALL SwXBookmark::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXBookmark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

OUString SAL_CALL SwXBookmark::getName()
{
    SolarMutexGuard aGuard;

    return (m_pImpl->m_pRegisteredBookmark)
        ?   m_pImpl->m_pRegisteredBookmark->GetName()
        :   m_pImpl->m_sMarkName;
}

void SAL_CALL SwXBookmark::setName(const OUString& rName)
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
SwXBookmark::getImplementationName()
{
    return OUString("SwXBookmark");
}

static char const*const g_ServicesBookmark[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.Bookmark",
    "com.sun.star.document.LinkTarget",
};
static const size_t g_nServicesBookmark(SAL_N_ELEMENTS(g_ServicesBookmark));

sal_Bool SAL_CALL SwXBookmark::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBookmark::getSupportedServiceNames()
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
        return pShell ? pShell->GetModel() : nullptr;
    }
    return nullptr;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXBookmark::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xRef(
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_BOOKMARK)
            ->getPropertySetInfo() );
    return xRef;
}

void SAL_CALL
SwXBookmark::setPropertyValue(const OUString& PropertyName,
        const uno::Any& rValue)
{
    if (PropertyName == UNO_NAME_BOOKMARK_HIDDEN)
    {
        bool bNewValue = false;
        if (!(rValue >>= bNewValue))
            throw lang::IllegalArgumentException("Property BookmarkHidden requires value of type boolean", nullptr, 0);

        IBookmark* pBookmark = dynamic_cast<IBookmark*>(m_pImpl->m_pRegisteredBookmark);
        if (pBookmark)
        {
            pBookmark->Hide(bNewValue);
        }
        else
        {
            m_pImpl->m_bHidden = bNewValue;
        }
        return;
    }
    else if (PropertyName == UNO_NAME_BOOKMARK_CONDITION)
    {
        OUString newValue;
        if (!(rValue >>= newValue))
            throw lang::IllegalArgumentException("Property BookmarkCondition requires value of type string", nullptr, 0);

        IBookmark* pBookmark = dynamic_cast<IBookmark*>(m_pImpl->m_pRegisteredBookmark);
        if (pBookmark)
        {
            pBookmark->SetHideCondition(newValue);
        }
        else
        {
            m_pImpl->m_HideCondition = newValue;
        }
        return;
    }

    // nothing to set here
    throw lang::IllegalArgumentException("Property is read-only: "
            + PropertyName, static_cast< cppu::OWeakObject * >(this), 0 );
}

uno::Any SAL_CALL SwXBookmark::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard g;

    uno::Any aRet;
    if (! ::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName == UNO_LINK_DISPLAY_NAME)
        {
            aRet <<= getName();
        }
        else if (rPropertyName == UNO_NAME_BOOKMARK_HIDDEN)
        {
            IBookmark* pBookmark = dynamic_cast<IBookmark*>(m_pImpl->m_pRegisteredBookmark);
            if (pBookmark)
            {
                aRet <<= pBookmark->IsHidden();
            }
            else
            {
                aRet <<= m_pImpl->m_bHidden;
            }
        }
        else if (rPropertyName == UNO_NAME_BOOKMARK_CONDITION)
        {
            IBookmark* pBookmark = dynamic_cast<IBookmark*>(m_pImpl->m_pRegisteredBookmark);
            if (pBookmark)
            {
                aRet <<= pBookmark->GetHideCondition();
            }
            else
            {
                aRet <<= m_pImpl->m_HideCondition;
            }
        }
    }
    return aRet;
}

void SAL_CALL
SwXBookmark::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXBookmark::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXBookmark::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXBookmark::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXBookmark::removeVetoableChangeListener(): not implemented");
}

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, SwDoc* pDc)
    : SwXFieldmark_Base(pDc)
    , isReplacementObject(_isReplacementObject)
{ }

void SwXFieldmarkParameters::insertByName(const OUString& aName, const uno::Any& aElement)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    if(pParameters->find(aName) != pParameters->end())
        throw container::ElementExistException();
    (*pParameters)[aName] = aElement;
}

void SwXFieldmarkParameters::removeByName(const OUString& aName)
{
    SolarMutexGuard aGuard;
    if(!getCoreParameters()->erase(aName))
        throw container::NoSuchElementException();
}

void SwXFieldmarkParameters::replaceByName(const OUString& aName, const uno::Any& aElement)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    pEntry->second = aElement;
}

uno::Any SwXFieldmarkParameters::getByName(const OUString& aName)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    return pEntry->second;
}

uno::Sequence<OUString> SwXFieldmarkParameters::getElementNames()
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return comphelper::mapKeysToSequence(*pParameters);
}

sal_Bool SwXFieldmarkParameters::hasByName(const OUString& aName)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return (pParameters->find(aName) != pParameters->end());
}

uno::Type SwXFieldmarkParameters::getElementType()
{
    return ::cppu::UnoType<void>::get();
}

sal_Bool SwXFieldmarkParameters::hasElements()
{
    SolarMutexGuard aGuard;
    return !getCoreParameters()->empty();
}

void SwXFieldmarkParameters::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        m_pFieldmark = nullptr;
}

IFieldmark::parameter_map_t* SwXFieldmarkParameters::getCoreParameters()
{
    if(!m_pFieldmark)
        throw uno::RuntimeException();
    return m_pFieldmark->GetParameters();
}

void SwXFieldmark::attachToRange( const uno::Reference < text::XTextRange >& xTextRange )
{

    attachToRangeEx( xTextRange,
                     ( isReplacementObject ? IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK : IDocumentMarkAccess::MarkType::TEXT_FIELDMARK ) );
}

OUString SwXFieldmark::getFieldType()
{
    SolarMutexGuard aGuard;
    const IFieldmark *pBkm = dynamic_cast<const IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw uno::RuntimeException();
    return pBkm->GetFieldname();
}

void SwXFieldmark::setFieldType(const OUString & fieldType)
{
    SolarMutexGuard aGuard;
    IFieldmark *pBkm = const_cast<IFieldmark*>(
        dynamic_cast<const IFieldmark*>(GetBookmark()));
    if(!pBkm)
        throw uno::RuntimeException();
    if(fieldType != getFieldType())
    {
        if(fieldType == ODF_FORMDROPDOWN || fieldType == ODF_FORMCHECKBOX)
        {
            ::sw::mark::IFieldmark* pNewFieldmark = GetIDocumentMarkAccess()->changeNonTextFieldmarkType(pBkm, fieldType);
            if (pNewFieldmark)
            {
                registerInMark(*this, pNewFieldmark);
                return;
            }
        }

        // We did not generate a new fieldmark, so set the type ID
        pBkm->SetFieldname(fieldType);
    }
}

uno::Reference<container::XNameContainer> SwXFieldmark::getParameters()
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
            pXBkmk = new SwXFieldmark(false, &rDoc);
        else if (dynamic_cast< ::sw::mark::CheckboxFieldmark* >(pMark))
            pXBkmk = new SwXFieldmark(true, &rDoc);
        else if (dynamic_cast< ::sw::mark::DropDownFieldmark* >(pMark))
            pXBkmk = new SwXFieldmark(true, &rDoc);
        else
            pXBkmk = new SwXFieldmark(isReplacementObject, &rDoc);

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
        assert( GetBookmark() == nullptr || pCheckboxFm != nullptr );
            // unclear to me whether GetBookmark() can be null here
    }
    return  pCheckboxFm;

}

// support 'hidden' "Checked" property ( note: this property is just for convenience to support
// docx import filter thus not published via PropertySet info )

void SAL_CALL
SwXFieldmark::setPropertyValue(const OUString& PropertyName,
        const uno::Any& rValue)
{
    SolarMutexGuard g;
    if ( PropertyName == "Checked" )
    {
        ::sw::mark::ICheckboxFieldmark* pCheckboxFm = getCheckboxFieldmark();
        bool bChecked( false );
        if ( !(pCheckboxFm && ( rValue >>= bChecked )) )
            throw uno::RuntimeException();

        pCheckboxFm->SetChecked( bChecked );
    }
    else
        SwXFieldmark_Base::setPropertyValue( PropertyName, rValue );
}

// support 'hidden' "Checked" property ( note: this property is just for convenience to support
// docx import filter thus not published via PropertySet info )

uno::Any SAL_CALL SwXFieldmark::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard g;
    if ( rPropertyName == "Checked" )
    {
        ::sw::mark::ICheckboxFieldmark* pCheckboxFm = getCheckboxFieldmark();
        if ( !pCheckboxFm )
            throw uno::RuntimeException();

        return uno::makeAny( pCheckboxFm->IsChecked() );
    }
    return SwXFieldmark_Base::getPropertyValue( rPropertyName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
