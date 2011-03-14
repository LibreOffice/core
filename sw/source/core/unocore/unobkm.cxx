/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <TextCursorHelper.hxx>
#include <unotextrange.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoevtlstnr.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <comcore.hrc>
#include <SwRewriter.hxx>
#include <docsh.hxx>
#include <xmloff/odffields.hxx>

using namespace ::sw::mark;
using namespace ::com::sun::star;
using ::rtl::OUString;


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

void SwXBookmark::registerInMark(SwXBookmark & rThis,
        ::sw::mark::IMark *const pBkmk)
{
    m_pImpl->registerInMark( rThis, pBkmk );
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
throw (uno::RuntimeException)
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
        throw lang::IllegalArgumentException();
    }

    m_pImpl->m_pDoc = pDoc;
    SwUnoInternalPaM aPam(*m_pImpl->m_pDoc);
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aCont(m_pImpl->m_pDoc);
    if (!m_pImpl->m_sMarkName.getLength())
    {
         m_pImpl->m_sMarkName =  OUString(RTL_CONSTASCII_USTRINGPARAM("Bookmark"));
    }
    if ((eType == IDocumentMarkAccess::BOOKMARK) &&
        ::sw::mark::CrossRefNumItemBookmark::IsLegalName(m_pImpl->m_sMarkName))
    {
        eType = IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK;
    }
    else if ((eType == IDocumentMarkAccess::BOOKMARK) &&
        ::sw::mark::CrossRefHeadingBookmark::IsLegalName(m_pImpl->m_sMarkName))
    {
        eType = IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK;
    }
    m_pImpl->registerInMark(*this,
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->makeMark(
            aPam, m_pImpl->m_sMarkName, eType));
    // --> OD 2007-10-23 #i81002#
    // Check, if bookmark has been created.
    // E.g., the creation of a cross-reference bookmark is suppress,
    // if the PaM isn't a valid one for cross-reference bookmarks.
    if (!m_pImpl->m_pRegisteredBookmark)
    {
        OSL_ENSURE(false,
            "<SwXBookmark::attachToRange(..)>"
            " - could not create Mark.");
        throw lang::IllegalArgumentException();
    }
    // <--
}

void SwXBookmark::attachToRange(
        const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    attachToRangeEx(xTextRange, IDocumentMarkAccess::BOOKMARK);
}

void SAL_CALL
SwXBookmark::attach(const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange > SAL_CALL
SwXBookmark::getAnchor() throw (uno::RuntimeException)
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
                ? &m_pImpl->m_pRegisteredBookmark->GetOtherMarkPos() : NULL);
}

void SAL_CALL SwXBookmark::dispose() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (m_pImpl->m_pRegisteredBookmark)
    {
        m_pImpl->m_pDoc->getIDocumentMarkAccess()->deleteMark(
                m_pImpl->m_pRegisteredBookmark);
    }
}

void SAL_CALL SwXBookmark::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->m_pRegisteredBookmark)
    {
        throw uno::RuntimeException();
    }
    m_pImpl->m_ListenerContainer.AddListener(xListener);
}

void SAL_CALL SwXBookmark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->m_pRegisteredBookmark ||
        !m_pImpl->m_ListenerContainer.RemoveListener(xListener))
    {
        throw uno::RuntimeException();
    }
}

OUString SAL_CALL SwXBookmark::getName()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return (m_pImpl->m_pRegisteredBookmark)
        ?   m_pImpl->m_pRegisteredBookmark->GetName()
        :   m_pImpl->m_sMarkName;
}

void SAL_CALL SwXBookmark::setName(const OUString& rName)
throw (uno::RuntimeException)
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
    if(pMarkAccess->findMark(rName) != pMarkAccess->getMarksEnd())
    {
        throw uno::RuntimeException();
    }

    SwPaM aPam(m_pImpl->m_pRegisteredBookmark->GetMarkPos());
    if (m_pImpl->m_pRegisteredBookmark->IsExpanded())
    {
        aPam.SetMark();
        *aPam.GetMark() = m_pImpl->m_pRegisteredBookmark->GetOtherMarkPos();
    }

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, lcl_QuoteName(getName()));
    aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
    aRewriter.AddRule(UNDO_ARG3, lcl_QuoteName(rName));

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(
            UNDO_BOOKMARK_RENAME, &aRewriter);
    pMarkAccess->renameMark(m_pImpl->m_pRegisteredBookmark, rName);
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(
            UNDO_BOOKMARK_RENAME, &aRewriter);
}

OUString SAL_CALL
SwXBookmark::getImplementationName() throw (uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SwXBookmark"));
}

static char const*const g_ServicesBookmark[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.Bookmark",
    "com.sun.star.document.LinkTarget",
};
static const size_t g_nServicesBookmark(
    SAL_N_ELEMENTS(g_ServicesBookmark));

sal_Bool SAL_CALL SwXBookmark::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesBookmark, g_ServicesBookmark, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBookmark::getSupportedServiceNames() throw (uno::RuntimeException)
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
        return (pShell) ? pShell->GetModel() : 0;
    }
    return 0;
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXBookmark::getPropertySetInfo() throw (uno::RuntimeException)
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
    uno::RuntimeException)
{
    // nothing to set here
    throw lang::IllegalArgumentException( ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Property is read-only: "))
            + PropertyName, static_cast< cppu::OWeakObject * >(this), 0 );
}

uno::Any SAL_CALL SwXBookmark::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard g;

    uno::Any aRet;
    if (! ::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
        {
            aRet <<= getName();
        }
    }
    return aRet;
}

void SAL_CALL
SwXBookmark::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXBookmark::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXBookmark::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXBookmark::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXBookmark::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXBookmark::removeVetoableChangeListener(): not implemented");
}

/******************************************************************
 * SwXFieldmark
 ******************************************************************/

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, ::sw::mark::IMark* pBkm, SwDoc* pDc)
    : SwXFieldmark_Base(pBkm, pDc)
    , isReplacementObject(_isReplacementObject)
{ }

void SwXFieldmarkParameters::insertByName(const OUString& aName, const uno::Any& aElement)
    throw (lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    if(pParameters->find(aName) != pParameters->end())
        throw container::ElementExistException();
    (*pParameters)[aName] = aElement;
}

void SwXFieldmarkParameters::removeByName(const OUString& aName)
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!getCoreParameters()->erase(aName))
        throw container::NoSuchElementException();
}

void SwXFieldmarkParameters::replaceByName(const OUString& aName, const uno::Any& aElement)
    throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    pEntry->second = aElement;
}

uno::Any SwXFieldmarkParameters::getByName(const OUString& aName)
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    IFieldmark::parameter_map_t::iterator pEntry = pParameters->find(aName);
    if(pEntry == pParameters->end())
        throw container::NoSuchElementException();
    return pEntry->second;
}

uno::Sequence<OUString> SwXFieldmarkParameters::getElementNames()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    uno::Sequence<OUString> vResult(pParameters->size());
    OUString* pOutEntry = vResult.getArray();
    for(IFieldmark::parameter_map_t::iterator pEntry = pParameters->begin(); pEntry!=pParameters->end(); ++pEntry, ++pOutEntry)
        *pOutEntry = pEntry->first;
    return vResult;
}

::sal_Bool SwXFieldmarkParameters::hasByName(const OUString& aName)
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark::parameter_map_t* pParameters = getCoreParameters();
    return (pParameters->find(aName) != pParameters->end());
}

uno::Type SwXFieldmarkParameters::getElementType()
    throw (uno::RuntimeException)
{
    return ::cppu::UnoType< ::cppu::UnoVoidType>::get();
}

::sal_Bool SwXFieldmarkParameters::hasElements()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return !getCoreParameters()->empty();
}

void SwXFieldmarkParameters::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
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
                     ( isReplacementObject ? IDocumentMarkAccess::CHECKBOX_FIELDMARK : IDocumentMarkAccess::TEXT_FIELDMARK ) );
}

::rtl::OUString SwXFieldmark::getFieldType(void)
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const IFieldmark *pBkm = dynamic_cast<const IFieldmark*>(GetBookmark());
    if(!pBkm)
        throw uno::RuntimeException();
    return pBkm->GetFieldname();
}

void SwXFieldmark::setFieldType(const::rtl::OUString & fieldType)
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark *pBkm = const_cast<IFieldmark*>(
        dynamic_cast<const IFieldmark*>(GetBookmark()));
    if(!pBkm)
        throw uno::RuntimeException();
    pBkm->SetFieldname(fieldType);
}

uno::Reference<container::XNameContainer> SwXFieldmark::getParameters()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IFieldmark *pBkm = const_cast<IFieldmark*>(
        dynamic_cast<const IFieldmark*>(GetBookmark()));
    if(!pBkm)
        throw uno::RuntimeException();
    return uno::Reference<container::XNameContainer>(new SwXFieldmarkParameters(pBkm));
}

uno::Reference<text::XTextContent>
SwXFieldmark::CreateXFieldmark(SwDoc & rDoc, ::sw::mark::IMark & rMark)
{
    // #i105557#: do not iterate over the registered clients: race condition
    ::sw::mark::MarkBase *const pMarkBase(
        dynamic_cast< ::sw::mark::MarkBase * >(&rMark));
    OSL_ENSURE(pMarkBase, "CreateXBookmark: no MarkBase?");
    if (!pMarkBase) { return 0; }
    uno::Reference<text::XTextContent> xMark(pMarkBase->GetXBookmark());
    if (!xMark.is())
    {
        // FIXME: These belong in XTextFieldsSupplier
        SwXFieldmark* pXBkmk = NULL;
        if (dynamic_cast< ::sw::mark::TextFieldmark* >(&rMark))
            pXBkmk = new SwXFieldmark(false, &rMark, &rDoc);
        else if (dynamic_cast< ::sw::mark::CheckboxFieldmark* >(&rMark))
            pXBkmk = new SwXFieldmark(true, &rMark, &rDoc);

        xMark.set(pXBkmk);
        pXBkmk->registerInMark(*pXBkmk, pMarkBase);
    }
    return xMark;
}

::sw::mark::ICheckboxFieldmark*
SwXFieldmark::getCheckboxFieldmark()
{
    ::sw::mark::ICheckboxFieldmark* pCheckboxFm = NULL;
    if ( getFieldType() == rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(ODF_FORMCHECKBOX) ) )
        // evil #TODO #FIXME casting away the const-ness
        pCheckboxFm = const_cast<sw::mark::ICheckboxFieldmark*>(reinterpret_cast< const ::sw::mark::ICheckboxFieldmark* >( GetBookmark()));
    return  pCheckboxFm;

}

// support 'hidden' "Checked" property ( note: this property is just for convenience to support
// docx import filter thus not published via PropertySet info )

void SAL_CALL
SwXFieldmark::setPropertyValue(const OUString& PropertyName,
        const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SolarMutexGuard g;
    if ( PropertyName.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Checked") ) ) )
    {
        ::sw::mark::ICheckboxFieldmark* pCheckboxFm = getCheckboxFieldmark();
        sal_Bool bChecked( sal_False );
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
        uno::RuntimeException)
{
    SolarMutexGuard g;
    if ( rPropertyName.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Checked") ) ) )
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
