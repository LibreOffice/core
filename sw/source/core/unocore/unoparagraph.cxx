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

#include <unoparagraph.hxx>

#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

#include <cmdid.h>
#include <unomid.h>
#include <unoparaframeenum.hxx>
#include <unotext.hxx>
#include <unotextrange.hxx>
#include <unoport.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <unoprnms.hxx>
#include <unocrsrhelper.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
#include <swunohelper.hxx>

#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <comphelper/servicehelper.hxx>
#include <editeng/unoipset.hxx>
#include <svl/listener.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>

using namespace ::com::sun::star;


class SwParaSelection
{
    SwCursor & m_rCursor;
public:
    explicit SwParaSelection(SwCursor & rCursor);
    ~SwParaSelection();
};

SwParaSelection::SwParaSelection(SwCursor & rCursor)
    : m_rCursor(rCursor)
{
    if (m_rCursor.HasMark())
    {
        m_rCursor.DeleteMark();
    }
    // is it at the start?
    if (m_rCursor.GetPoint()->nContent != 0)
    {
        m_rCursor.MovePara(GoCurrPara, fnParaStart);
    }
    // or at the end already?
    if (m_rCursor.GetPoint()->nContent != m_rCursor.GetContentNode()->Len())
    {
        m_rCursor.SetMark();
        m_rCursor.MovePara(GoCurrPara, fnParaEnd);
    }
}

SwParaSelection::~SwParaSelection()
{
    if (m_rCursor.GetPoint()->nContent != 0)
    {
        m_rCursor.DeleteMark();
        m_rCursor.MovePara(GoCurrPara, fnParaStart);
    }
}

/// @throws beans::UnknownPropertyException
/// @throws uno::RuntimeException
static beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            const SwTextNode& rTextNode,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertySimpleEntry& rEntry,
                            bool &rAttrSetFetched );

class SwXParagraph::Impl
    : public SvtListener
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    SwXParagraph& m_rThis;
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    SfxItemPropertySet const& m_rPropSet;
    bool m_bIsDescriptor;
    sal_Int32 const m_nSelectionStartPos;
    sal_Int32 const m_nSelectionEndPos;
    OUString m_sText;
    uno::Reference<text::XText> m_xParentText;
    SwTextNode* m_pTextNode;

    Impl(SwXParagraph& rThis,
            SwTextNode* const pTextNode = nullptr, uno::Reference<text::XText> const& xParent = nullptr,
            const sal_Int32 nSelStart = -1, const sal_Int32 nSelEnd = -1)
        : m_rThis(rThis)
        , m_EventListeners(m_Mutex)
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH))
        , m_bIsDescriptor(nullptr == pTextNode)
        , m_nSelectionStartPos(nSelStart)
        , m_nSelectionEndPos(nSelEnd)
        , m_xParentText(xParent)
        , m_pTextNode(pTextNode)
    {
        m_pTextNode && StartListening(m_pTextNode->GetNotifier());
    }

    SwTextNode* GetTextNode() {
        return m_pTextNode;
    }

    SwTextNode& GetTextNodeOrThrow() {
        if (!m_pTextNode) {
            throw uno::RuntimeException("SwXParagraph: disposed or invalid", nullptr);
        }
        return *m_pTextNode;
    }

    bool IsDescriptor() const { return m_bIsDescriptor; }

    /// @throws beans::UnknownPropertyException
    /// @throws beans::PropertyVetoException
    /// @throws lang::IllegalArgumentException
    /// @throws lang::WrappedTargetException
    /// @throws uno::RuntimeException
    void SetPropertyValues_Impl(
            const uno::Sequence< OUString >& rPropertyNames,
            const uno::Sequence< uno::Any >& rValues);

    /// @throws beans::UnknownPropertyException
    /// @throws lang::WrappedTargetException
    /// @throws uno::RuntimeException
    uno::Sequence< uno::Any >
        GetPropertyValues_Impl(
            const uno::Sequence< OUString >& rPropertyNames);

    /// @throws uno::RuntimeException
    void GetSinglePropertyValue_Impl(
        const SfxItemPropertySimpleEntry& rEntry,
        const SfxItemSet& rSet,
        uno::Any& rAny ) const;

    /// @throws uno::RuntimeException
    uno::Sequence< beans::GetDirectPropertyTolerantResult >
        GetPropertyValuesTolerant_Impl(
            const uno::Sequence< OUString >& rPropertyNames,
            bool bDirectValuesOnly);
protected:
    virtual void Notify(const SfxHint& rHint) override;

};

void SwXParagraph::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pTextNode = nullptr;
        uno::Reference<uno::XInterface> const xThis(m_wThis);
        if (!xThis.is())
        {   // fdo#72695: if UNO object is already dead, don't revive it with event
            return;
        }
        lang::EventObject const ev(xThis);
        m_EventListeners.disposeAndClear(ev);
    }
}

SwXParagraph::SwXParagraph()
    : m_pImpl( new SwXParagraph::Impl(*this) )
{
}

SwXParagraph::SwXParagraph(
        uno::Reference< text::XText > const & xParent,
        SwTextNode & rTextNode,
        const sal_Int32 nSelStart, const sal_Int32 nSelEnd)
    : m_pImpl(
        new SwXParagraph::Impl(*this, &rTextNode, xParent, nSelStart, nSelEnd))
{
}

SwXParagraph::~SwXParagraph()
{
}

const SwTextNode * SwXParagraph::GetTextNode() const
{
    return m_pImpl->GetTextNode();
}

bool SwXParagraph::IsDescriptor() const
{
    return m_pImpl->IsDescriptor();
}

uno::Reference<text::XTextContent>
SwXParagraph::CreateXParagraph(SwDoc & rDoc, SwTextNode *const pTextNode,
        uno::Reference< text::XText> const& i_xParent,
        const sal_Int32 nSelStart, const sal_Int32 nSelEnd)
{
    // re-use existing SwXParagraph
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference<text::XTextContent> xParagraph;
    if (pTextNode && (-1 == nSelStart) && (-1 == nSelEnd))
    {   // only use cache if no selection!
        xParagraph.set(pTextNode->GetXParagraph());
    }
    if (xParagraph.is())
    {
        return xParagraph;
    }

    // create new SwXParagraph
    uno::Reference<text::XText> xParentText(i_xParent);
    if (!xParentText.is() && pTextNode)
    {
        SwPosition Pos(*pTextNode);
        xParentText.set(::sw::CreateParentXText( rDoc, Pos ));
    }
    SwXParagraph *const pXPara( pTextNode
            ? new SwXParagraph(xParentText, *pTextNode, nSelStart, nSelEnd)
            : new SwXParagraph);
    // this is why the constructor is private: need to acquire pXPara here
    xParagraph.set(pXPara);
    // in order to initialize the weak pointer cache in the core object
    if (pTextNode && (-1 == nSelStart) && (-1 == nSelEnd))
    {
        pTextNode->SetXParagraph(xParagraph);
    }
    // need a permanent Reference to initialize m_wThis
    pXPara->m_pImpl->m_wThis = xParagraph;
    return xParagraph;
}

bool SwXParagraph::SelectPaM(SwPaM & rPaM)
{
    SwTextNode const*const pTextNode( GetTextNode() );

    if (!pTextNode)
    {
        return false;
    }

    *rPaM.GetPoint() = SwPosition( *pTextNode );
    // set selection to the whole paragraph
    rPaM.SetMark();
    rPaM.GetMark()->nContent = pTextNode->GetText().getLength();
    return true;
}

namespace
{
    class theSwXParagraphUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXParagraphUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXParagraph::getUnoTunnelId()
{
    return theSwXParagraphUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXParagraph::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return ::sw::UnoTunnelImpl<SwXParagraph>(rId, this);
}

OUString SAL_CALL
SwXParagraph::getImplementationName()
{
    return OUString("SwXParagraph");
}

static char const*const g_ServicesParagraph[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.Paragraph",
    "com.sun.star.style.CharacterProperties",
    "com.sun.star.style.CharacterPropertiesAsian",
    "com.sun.star.style.CharacterPropertiesComplex",
    "com.sun.star.style.ParagraphProperties",
    "com.sun.star.style.ParagraphPropertiesAsian",
    "com.sun.star.style.ParagraphPropertiesComplex",
};

static const size_t g_nServicesParagraph(SAL_N_ELEMENTS(g_ServicesParagraph));

sal_Bool SAL_CALL
SwXParagraph::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXParagraph::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParagraph, g_ServicesParagraph);
}

void
SwXParagraph::attachToText(SwXText & rParent, SwTextNode & rTextNode)
{
    OSL_ENSURE(m_pImpl->m_bIsDescriptor, "Paragraph is not a descriptor");
    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_bIsDescriptor = false;
        m_pImpl->EndListeningAll();
        m_pImpl->StartListening(rTextNode.GetNotifier());
        rTextNode.SetXParagraph(uno::Reference<text::XTextContent>(this));
        m_pImpl->m_xParentText = &rParent;
        if (!m_pImpl->m_sText.isEmpty())
        {
            try { setString(m_pImpl->m_sText); }
            catch(...){}
            m_pImpl->m_sText.clear();
        }
    }
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXParagraph::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXParagraph::setPropertyValue(const OUString& rPropertyName,
        const uno::Any& rValue)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aPropertyNames { rPropertyName };
    uno::Sequence<uno::Any> aValues(1);
    aValues.getArray()[0] = rValue;
    m_pImpl->SetPropertyValues_Impl( aPropertyNames, aValues );
}

uno::Any
SwXParagraph::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aPropertyNames { rPropertyName };
    const uno::Sequence< uno::Any > aRet =
        m_pImpl->GetPropertyValues_Impl(aPropertyNames);
    return aRet.getConstArray()[0];
}

void SwXParagraph::Impl::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
{
    SwTextNode & rTextNode(GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    const OUString* pPropertyNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    const SfxItemPropertyMap &rMap = m_rPropSet.getPropertyMap();
    SwParaSelection aParaSel( aCursor );

    uno::Sequence< beans::PropertyValue > aValues( rPropertyNames.getLength() );
    for (sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( pPropertyNames[nProp] );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            throw beans::PropertyVetoException(
                "Property is read-only: " + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        aValues[nProp].Name = pPropertyNames[nProp];
        aValues[nProp].Value = pValues[nProp];
    }
    SwUnoCursorHelper::SetPropertyValues(aCursor, m_rPropSet, aValues);
}

void SAL_CALL SwXParagraph::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        m_pImpl->SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

// Support for DrawingLayer FillStyles for GetPropertyValue() usages
void SwXParagraph::Impl::GetSinglePropertyValue_Impl(
    const SfxItemPropertySimpleEntry& rEntry,
    const SfxItemSet& rSet,
    uno::Any& rAny ) const
{
    bool bDone(false);

    switch(rEntry.nWID)
    {
        case RES_BACKGROUND:
        {
            const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));

            if(!aOriginalBrushItem.QueryValue(rAny, rEntry.nMemberId))
            {
                OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
            }

            bDone = true;
            break;
        }
        case OWN_ATTR_FILLBMP_MODE:
        {
            if (rSet.Get(XATTR_FILLBMP_TILE).GetValue())
            {
                rAny <<= drawing::BitmapMode_REPEAT;
            }
            else if (rSet.Get(XATTR_FILLBMP_STRETCH).GetValue())
            {
                rAny <<= drawing::BitmapMode_STRETCH;
            }
            else
            {
                rAny <<= drawing::BitmapMode_NO_REPEAT;
            }

            bDone = true;
            break;
        }
        default: break;
    }

    if(!bDone)
    {
        // fallback to standard get value implementation used before this helper was created
        m_rPropSet.getPropertyValue(rEntry, rSet, rAny);

        if(rEntry.aType == cppu::UnoType<sal_Int16>::get() && rEntry.aType != rAny.getValueType())
        {
            // since the sfx uInt16 item now exports a sal_Int32, we may have to fix this here
            sal_Int32 nValue(0);

            if (rAny >>= nValue)
            {
                rAny <<= static_cast<sal_Int16>(nValue);
            }
        }

        // check for needed metric translation
        if(rEntry.nMoreFlags & PropertyMoreFlags::METRIC_ITEM)
        {
            bool bDoIt(true);

            if(XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
            {
                // exception: If these ItemTypes are used, do not convert when these are negative
                // since this means they are intended as percent values
                sal_Int32 nValue = 0;

                if(rAny >>= nValue)
                {
                    bDoIt = nValue > 0;
                }
            }

            if(bDoIt)
            {
                const MapUnit eMapUnit(rSet.GetPool()->GetMetric(rEntry.nWID));

                if(eMapUnit != MapUnit::Map100thMM)
                {
                    SvxUnoConvertToMM(eMapUnit, rAny);
                }
            }
        }
    }
}

uno::Sequence< uno::Any > SwXParagraph::Impl::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
{
    SwTextNode & rTextNode(GetTextNodeOrThrow());

    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    SwPosition aPos( rTextNode );
    SwPaM aPam( aPos );
    uno::Any* pValues = aValues.getArray();
    const OUString* pPropertyNames = rPropertyNames.getConstArray();
    const SfxItemPropertyMap &rMap = m_rPropSet.getPropertyMap();
    const SwAttrSet& rAttrSet( rTextNode.GetSwAttrSet() );
    for (sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( pPropertyNames[nProp] );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        if (! ::sw::GetDefaultTextContentValue(
                pValues[nProp], pPropertyNames[nProp], pEntry->nWID))
        {
            beans::PropertyState eTemp;
            const bool bDone = SwUnoCursorHelper::getCursorPropertyValue(
                *pEntry, aPam, &(pValues[nProp]), eTemp, &rTextNode );
            if (!bDone)
            {
                GetSinglePropertyValue_Impl(*pEntry, rAttrSet, pValues[nProp]);
            }
        }
    }
    return aValues;
}

uno::Sequence< uno::Any > SAL_CALL
SwXParagraph::getPropertyValues(const uno::Sequence< OUString >& rPropertyNames)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = m_pImpl->GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException("Unknown property exception caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }
    catch (lang::WrappedTargetException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException("WrappedTargetException caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }

    return aValues;
}

void SAL_CALL SwXParagraph::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXParagraph::addPropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXParagraph::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXParagraph::removePropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXParagraph::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXParagraph::firePropertiesChangeEvent(): not implemented");
}

/* disabled for #i46921# */

uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL
SwXParagraph::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
{
    SolarMutexGuard aGuard;

    if (rPropertyNames.getLength() != rValues.getLength())
    {
        throw lang::IllegalArgumentException();
    }

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    //SwNode& rTextNode = pUnoCursor->GetPoint()->nNode.GetNode();
    //const SwAttrSet& rAttrSet = static_cast<SwTextNode&>(rTextNode).GetSwAttrSet();
    //sal_uInt16 nAttrCount = rAttrSet.Count();

    const sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    //sal_Int32 nVals = rValues.getLength();
    const uno::Any *pValue = rValues.getConstArray();

    sal_Int32 nFailed = 0;
    uno::Sequence< beans::SetPropertyTolerantFailed > aFailed( nProps );
    beans::SetPropertyTolerantFailed *pFailed = aFailed.getArray();

    // get entry to start with
    const SfxItemPropertyMap &rPropMap =
        m_pImpl->m_rPropSet.getPropertyMap();

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    SwParaSelection aParaSel( aCursor );
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        try
        {
            pFailed[ nFailed ].Name = pProp[i];

            SfxItemPropertySimpleEntry const*const pEntry =
                rPropMap.getByName( pProp[i] );
            if (!pEntry)
            {
                pFailed[ nFailed++ ].Result  =
                    beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            }
            else
            {
                // set property value
                // (compare to SwXParagraph::setPropertyValues)
                if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
                {
                    pFailed[ nFailed++ ].Result  =
                        beans::TolerantPropertySetResultType::PROPERTY_VETO;
                }
                else
                {
                    SwUnoCursorHelper::SetPropertyValue(
                        aCursor, m_pImpl->m_rPropSet, pProp[i], pValue[i]);
                }
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            OSL_FAIL( "unexpected exception caught" );
            pFailed[ nFailed++ ].Result =
                beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            pFailed[ nFailed++ ].Result =
                beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            pFailed[ nFailed++ ].Result =
                beans::TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            pFailed[ nFailed++ ].Result =
                beans::TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    aFailed.realloc( nFailed );
    return aFailed;
}

uno::Sequence< beans::GetPropertyTolerantResult > SAL_CALL
SwXParagraph::getPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
{
    SolarMutexGuard aGuard;

    uno::Sequence< beans::GetDirectPropertyTolerantResult > aTmpRes(
        m_pImpl->GetPropertyValuesTolerant_Impl( rPropertyNames, false ) );
    const beans::GetDirectPropertyTolerantResult *pTmpRes =
        aTmpRes.getConstArray();

    // copy temporary result to final result type
    const sal_Int32 nLen = aTmpRes.getLength();
    uno::Sequence< beans::GetPropertyTolerantResult > aRes( nLen );
    beans::GetPropertyTolerantResult *pRes = aRes.getArray();
    for (sal_Int32 i = 0;  i < nLen;  i++)
    {
        *pRes++ = *pTmpRes++;
    }
    return aRes;
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL
SwXParagraph::getDirectPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
{
    SolarMutexGuard aGuard;

    return m_pImpl->GetPropertyValuesTolerant_Impl( rPropertyNames, true );
}

uno::Sequence< beans::GetDirectPropertyTolerantResult >
SwXParagraph::Impl::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        bool bDirectValuesOnly )
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(GetTextNodeOrThrow());

    // #i46786# Use SwAttrSet pointer for determining the state.
    //          Use the value SwAttrSet (from the paragraph OR the style)
    //          for determining the actual value(s).
    const SwAttrSet* pAttrSet = rTextNode.GetpSwAttrSet();
    const SwAttrSet& rValueAttrSet = rTextNode.GetSwAttrSet();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    uno::Sequence< beans::GetDirectPropertyTolerantResult > aResult( nProps );
    beans::GetDirectPropertyTolerantResult *pResult = aResult.getArray();
    sal_Int32 nIdx = 0;

    // get entry to start with
    const SfxItemPropertyMap &rPropMap = m_rPropSet.getPropertyMap();

    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        OSL_ENSURE( nIdx < nProps, "index out ouf bounds" );
        beans::GetDirectPropertyTolerantResult &rResult = pResult[nIdx];

        try
        {
            rResult.Name = pProp[i];

            SfxItemPropertySimpleEntry const*const pEntry =
                rPropMap.getByName( pProp[i] );
            if (!pEntry)  // property available?
            {
                rResult.Result =
                    beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            }
            else
            {
                // get property state
                // (compare to SwXParagraph::getPropertyState)
                bool bAttrSetFetched = true;
                beans::PropertyState eState = lcl_SwXParagraph_getPropertyState(
                            rTextNode, &pAttrSet, *pEntry, bAttrSetFetched );
                rResult.State  = eState;

                rResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_FAILURE;
                if (!bDirectValuesOnly ||
                    (beans::PropertyState_DIRECT_VALUE == eState))
                {
                    // get property value
                    // (compare to SwXParagraph::getPropertyValue(s))
                    uno::Any aValue;
                    if (! ::sw::GetDefaultTextContentValue(
                                aValue, pProp[i], pEntry->nWID ) )
                    {
                        SwPosition aPos( rTextNode );
                        SwPaM aPam( aPos );
                        // handle properties that are not part of the attribute
                        // and thus only pretended to be paragraph attributes
                        beans::PropertyState eTemp;
                        const bool bDone =
                            SwUnoCursorHelper::getCursorPropertyValue(
                                    *pEntry, aPam, &aValue, eTemp, &rTextNode );

                        // if not found try the real paragraph attributes...
                        if (!bDone)
                        {
                            GetSinglePropertyValue_Impl(*pEntry, rValueAttrSet, aValue);
                        }
                    }

                    rResult.Value  = aValue;
                    rResult.Result = beans::TolerantPropertySetResultType::SUCCESS;

                    nIdx++;
                }
                // this assertion should never occur!
                OSL_ENSURE( nIdx < 1  ||  pResult[nIdx - 1].Result != beans::TolerantPropertySetResultType::UNKNOWN_FAILURE,
                        "unknown failure while retrieving property" );

            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            OSL_FAIL( "unexpected exception caught" );
            rResult.Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            rResult.Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            rResult.Result = beans::TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            rResult.Result = beans::TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    // resize to actually used size
    aResult.realloc( nIdx );

    return aResult;
}

bool ::sw::GetDefaultTextContentValue(
        uno::Any& rAny, const OUString& rPropertyName, sal_uInt16 nWID)
{
    if(!nWID)
    {
        if(rPropertyName == UNO_NAME_ANCHOR_TYPE)
            nWID = FN_UNO_ANCHOR_TYPE;
        else if(rPropertyName == UNO_NAME_ANCHOR_TYPES)
            nWID = FN_UNO_ANCHOR_TYPES;
        else if(rPropertyName == UNO_NAME_TEXT_WRAP)
            nWID = FN_UNO_TEXT_WRAP;
        else
            return false;
    }

    switch(nWID)
    {
        case FN_UNO_TEXT_WRAP:  rAny <<= text::WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= text::TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {   uno::Sequence<text::TextContentAnchorType> aTypes(1);
            text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
            rAny <<= aTypes;
        }
        break;
        default:
            return false;
    }
    return true;
}

void SAL_CALL
SwXParagraph::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXParagraph::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXParagraph::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXParagraph::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXParagraph::removeVetoableChangeListener(): not implemented");
}

static beans::PropertyState lcl_SwXParagraph_getPropertyState(
    const SwTextNode& rTextNode,
    const SwAttrSet** ppSet,
    const SfxItemPropertySimpleEntry& rEntry,
    bool &rAttrSetFetched)
{
    beans::PropertyState eRet(beans::PropertyState_DEFAULT_VALUE);

    if(!(*ppSet) && !rAttrSetFetched)
    {
        (*ppSet) = rTextNode.GetpSwAttrSet();
        rAttrSetFetched = true;
    }

    SwPosition aPos(rTextNode);
    SwPaM aPam(aPos);
    bool bDone(false);

    switch(rEntry.nWID)
    {
        case FN_UNO_NUM_RULES:
        {
            // if numbering is set, return it; else do nothing
            SwUnoCursorHelper::getNumberingProperty(aPam,eRet,nullptr);
            bDone = true;
            break;
        }
        case FN_UNO_ANCHOR_TYPES:
        {
            bDone = true;
            break;
        }
        case RES_ANCHOR:
        {
            bDone = (MID_SURROUND_SURROUNDTYPE == rEntry.nMemberId);
            break;
        }
        case RES_SURROUND:
        {
            bDone = (MID_ANCHOR_ANCHORTYPE == rEntry.nMemberId);
            break;
        }
        case FN_UNO_PARA_STYLE:
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        {
            SwFormatColl* pFormat = SwUnoCursorHelper::GetCurTextFormatColl(aPam,rEntry.nWID == FN_UNO_PARA_CONDITIONAL_STYLE_NAME);
            eRet = pFormat ? beans::PropertyState_DIRECT_VALUE : beans::PropertyState_AMBIGUOUS_VALUE;
            bDone = true;
            break;
        }
        case FN_UNO_PAGE_STYLE:
        {
            OUString sVal;
            SwUnoCursorHelper::GetCurPageStyle( aPam, sVal );
            eRet = !sVal.isEmpty() ? beans::PropertyState_DIRECT_VALUE
                              : beans::PropertyState_AMBIGUOUS_VALUE;
            bDone = true;
            break;
        }

        // DrawingLayer PropertyStyle support
        case OWN_ATTR_FILLBMP_MODE:
        {
            if(*ppSet)
            {
                if(SfxItemState::SET == (*ppSet)->GetItemState(XATTR_FILLBMP_STRETCH, false)
                    || SfxItemState::SET == (*ppSet)->GetItemState(XATTR_FILLBMP_TILE, false))
                {
                    eRet = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    eRet = beans::PropertyState_AMBIGUOUS_VALUE;
                }

                bDone = true;
            }
            break;
        }
        case RES_BACKGROUND:
        {
            if(*ppSet)
            {
                if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(**ppSet,
                        rEntry.nMemberId))
                {
                    eRet = beans::PropertyState_DIRECT_VALUE;
                }
                bDone = true;
            }
            break;
        }
    }

    if(!bDone)
    {
        if((*ppSet) && SfxItemState::SET == (*ppSet)->GetItemState(rEntry.nWID, false))
        {
            eRet = beans::PropertyState_DIRECT_VALUE;
        }
    }

    return eRet;
}

beans::PropertyState SAL_CALL
SwXParagraph::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    const SwAttrSet* pSet = nullptr;
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    bool bDummy = false;
    const beans::PropertyState eRet =
        lcl_SwXParagraph_getPropertyState(rTextNode, &pSet, *pEntry, bDummy);
    return eRet;
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXParagraph::getPropertyStates(
        const uno::Sequence< OUString >& PropertyNames)
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    const OUString* pNames = PropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const SfxItemPropertyMap &rMap = m_pImpl->m_rPropSet.getPropertyMap();
    const SwAttrSet* pSet = nullptr;
    bool bAttrSetFetched = false;

    for (sal_Int32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd;
            ++i, ++pStates, ++pNames)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( *pNames );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + *pNames,
                static_cast<cppu::OWeakObject *>(this));
        }

        if (bAttrSetFetched && !pSet && isATR(pEntry->nWID))
        {
            *pStates = beans::PropertyState_DEFAULT_VALUE;
        }
        else
        {
            *pStates = lcl_SwXParagraph_getPropertyState(
                rTextNode, &pSet, *pEntry, bAttrSetFetched );
        }
    }

    return aRet;
}

void SAL_CALL
SwXParagraph::setPropertyToDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    if (rPropertyName == UNO_NAME_ANCHOR_TYPE  ||
        rPropertyName == UNO_NAME_ANCHOR_TYPES ||
        rPropertyName == UNO_NAME_TEXT_WRAP)
    {
        return;
    }

    // select paragraph
    SwParaSelection aParaSel( aCursor );
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(
            "Property is read-only: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    const bool bBelowFrameAtrEnd(pEntry->nWID < RES_FRMATR_END);
    const bool bDrawingLayerRange(XATTR_FILL_FIRST <= pEntry->nWID && XATTR_FILL_LAST >= pEntry->nWID);

    if(bBelowFrameAtrEnd || bDrawingLayerRange)
    {
        std::set<sal_uInt16> aWhichIds;

        // For FillBitmapMode two IDs have to be reset (!)
        if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
        {
            aWhichIds.insert(XATTR_FILLBMP_STRETCH);
            aWhichIds.insert(XATTR_FILLBMP_TILE);
        }
        else
        {
            aWhichIds.insert(pEntry->nWID);
        }

        if (pEntry->nWID < RES_PARATR_BEGIN)
        {
            aCursor.GetDoc()->ResetAttrs(aCursor, true, aWhichIds);
        }
        else
        {
            // for paragraph attributes the selection must be extended
            // to paragraph boundaries
            SwPosition aStart( *aCursor.Start() );
            SwPosition aEnd  ( *aCursor.End()   );
            auto pTemp( aCursor.GetDoc()->CreateUnoCursor(aStart) );
            if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
            {
                pTemp->MovePara(GoCurrPara, fnParaStart);
            }

            pTemp->SetMark();
            *pTemp->GetPoint() = aEnd;

            SwUnoCursorHelper::SelectPam(*pTemp, true);

            if (!SwUnoCursorHelper::IsEndOfPara(*pTemp))
            {
                pTemp->MovePara(GoCurrPara, fnParaEnd);
            }


            pTemp->GetDoc()->ResetAttrs(*pTemp, true, aWhichIds);
        }
    }
    else
    {
        SwUnoCursorHelper::resetCursorPropertyValue(*pEntry, aCursor);
    }
}

uno::Any SAL_CALL
SwXParagraph::getPropertyDefault(const OUString& rPropertyName)
{
    SolarMutexGuard g;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    uno::Any aRet;
    if (::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        return aRet;
    }

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    const bool bBelowFrameAtrEnd(pEntry->nWID < RES_FRMATR_END);
    const bool bDrawingLayerRange(XATTR_FILL_FIRST <= pEntry->nWID && XATTR_FILL_LAST >= pEntry->nWID);

    if(bBelowFrameAtrEnd || bDrawingLayerRange)
    {
        const SfxPoolItem& rDefItem = rTextNode.GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);

        rDefItem.QueryValue(aRet, pEntry->nMemberId);
    }

    return aRet;
}

void SAL_CALL
SwXParagraph::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
{
    SolarMutexGuard aGuard;
    // SwXParagraph will only created in order to be inserted by
    // 'insertTextContentBefore' or 'insertTextContentAfter' therefore
    // they cannot be attached
    throw uno::RuntimeException();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getAnchor()
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    // select paragraph
    SwParaSelection aParaSel( aCursor );
    const uno::Reference< text::XTextRange >  xRet =
        new SwXTextRange(aCursor, m_pImpl->m_xParentText);
    return xRet;
}

void SAL_CALL SwXParagraph::dispose()
{
    SolarMutexGuard aGuard;

    SwTextNode *const pTextNode( m_pImpl->GetTextNode() );
    if (pTextNode)
    {
        SwCursor aCursor( SwPosition( *pTextNode ), nullptr );
        pTextNode->GetDoc()->getIDocumentContentOperations().DelFullPara(aCursor);
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_pImpl->m_EventListeners.disposeAndClear(ev);
    }
}

void SAL_CALL SwXParagraph::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXParagraph::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< container::XEnumeration >  SAL_CALL
SwXParagraph::createEnumeration()
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwPaM aPam ( aPos );
    const uno::Reference< container::XEnumeration > xRef =
        new SwXTextPortionEnumeration(aPam, m_pImpl->m_xParentText,
            m_pImpl->m_nSelectionStartPos, m_pImpl->m_nSelectionEndPos);
    return xRef;
}

uno::Type SAL_CALL SwXParagraph::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXParagraph::hasElements()
{
    SolarMutexGuard aGuard;
    return GetTextNode() != nullptr;
}

uno::Reference< text::XText > SAL_CALL
SwXParagraph::getText()
{
    SolarMutexGuard g;

    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getStart()
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    SwParaSelection aParaSel( aCursor );
    SwPaM aPam( *aCursor.Start() );
    uno::Reference< text::XText >  xParent = getText();
    const uno::Reference< text::XTextRange > xRet =
        new SwXTextRange(aPam, xParent);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getEnd()
{
    SolarMutexGuard aGuard;

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwCursor aCursor( aPos, nullptr );
    SwParaSelection aParaSel( aCursor );
    SwPaM aPam( *aCursor.End() );
    uno::Reference< text::XText >  xParent = getText();
    const uno::Reference< text::XTextRange > xRet =
        new SwXTextRange(aPam, xParent);
    return xRet;
}

OUString SAL_CALL SwXParagraph::getString()
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwTextNode const*const pTextNode( GetTextNode() );
    if (pTextNode)
    {
        SwPosition aPos( *pTextNode );
        SwCursor aCursor( aPos, nullptr );
        SwParaSelection aParaSel( aCursor );
        SwUnoCursorHelper::GetTextFromPam(aCursor, aRet);
    }
    else if (m_pImpl->IsDescriptor())
    {
        aRet = m_pImpl->m_sText;
    }
    else
    {
        // Seems object is being disposed or some other problem occurs.
        // Anyway from user point of view object still exist, so on that level this is not an error
        SAL_WARN("sw.uno", "getString() for invalid paragraph called. Returning empty string.");
    }
    return aRet;
}

void SAL_CALL SwXParagraph::setString(const OUString& aString)
{
    SolarMutexGuard aGuard;

    SwTextNode const*const pTextNode( GetTextNode() );
    if (pTextNode)
    {
        SwPosition aPos( *pTextNode );
        SwCursor aCursor( aPos, nullptr );
        if (!SwUnoCursorHelper::IsStartOfPara(aCursor)) {
            aCursor.MovePara(GoCurrPara, fnParaStart);
        }
        SwUnoCursorHelper::SelectPam(aCursor, true);
        if (pTextNode->GetText().getLength()) {
            aCursor.MovePara(GoCurrPara, fnParaEnd);
        }
        SwUnoCursorHelper::SetString(aCursor, aString);
        SwUnoCursorHelper::SelectPam(aCursor, false);
    }
    else if (m_pImpl->IsDescriptor())
    {
        m_pImpl->m_sText = aString;
    }
    else
    {
        throw uno::RuntimeException();
    }
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXParagraph::createContentEnumeration(const OUString& rServiceName)
{
    SolarMutexGuard g;

    if ( rServiceName != "com.sun.star.text.TextContent" )
    {
        throw uno::RuntimeException();
    }

    SwTextNode & rTextNode(m_pImpl->GetTextNodeOrThrow());

    SwPosition aPos( rTextNode );
    SwPaM aPam( aPos );
    uno::Reference< container::XEnumeration > xRet =
        SwXParaFrameEnumeration::Create(aPam, PARAFRAME_PORTION_PARAGRAPH);
    return xRet;
}

uno::Sequence< OUString > SAL_CALL
SwXParagraph::getAvailableServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.TextContent" };
    return aRet;
}

// MetadatableMixin
::sfx2::Metadatable* SwXParagraph::GetCoreObject()
{
    SwTextNode *const pTextNode( m_pImpl->GetTextNode() );
    return pTextNode;
}

uno::Reference<frame::XModel> SwXParagraph::GetModel()
{
    SwTextNode *const pTextNode( m_pImpl->GetTextNode() );
    if (pTextNode)
    {
        SwDocShell const*const pShell( pTextNode->GetDoc()->GetDocShell() );
        return pShell ? pShell->GetModel() : nullptr;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
