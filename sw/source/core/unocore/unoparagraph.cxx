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

#include <unoparagraph.hxx>
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


#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <comphelper/servicehelper.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;

class SwParaSelection
{
    SwCursor & m_rCursor;
public:
    SwParaSelection(SwCursor & rCursor);
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
        m_rCursor.MovePara(fnParaCurr, fnParaStart);
    }
    // or at the end already?
    if (m_rCursor.GetPoint()->nContent != m_rCursor.GetCntntNode()->Len())
    {
        m_rCursor.SetMark();
        m_rCursor.MovePara(fnParaCurr, fnParaEnd);
    }
}

SwParaSelection::~SwParaSelection()
{
    if (m_rCursor.GetPoint()->nContent != 0)
    {
        m_rCursor.DeleteMark();
        m_rCursor.MovePara(fnParaCurr, fnParaStart);
    }
}

/******************************************************************
 * forward declarations
 ******************************************************************/
beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            const SwTxtNode& rTxtNode,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertySimpleEntry& rEntry,
                            sal_Bool &rAttrSetFetched )
    throw (beans::UnknownPropertyException);

/******************************************************************
 * SwXParagraph
 ******************************************************************/
class SwXParagraph::Impl
    : public SwClient
{

public:
    SwXParagraph &              m_rThis;
    SwEventListenerContainer    m_ListenerContainer;
    SfxItemPropertySet const&   m_rPropSet;
    bool                        m_bIsDescriptor;
    sal_Int32                   m_nSelectionStartPos;
    sal_Int32                   m_nSelectionEndPos;
    ::rtl::OUString             m_sText;
    uno::Reference<text::XText> m_xParentText;

    Impl(   SwXParagraph & rThis,
            SwTxtNode *const pTxtNode = 0,
            uno::Reference< text::XText > const & xParent = 0,
            const sal_Int32 nSelStart = -1, const sal_Int32 nSelEnd = -1)
        : SwClient(pTxtNode)
        , m_rThis(rThis)
        , m_ListenerContainer(static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH))
        // #i111177# unxsols4 (Sun C++ 5.9 SunOS_sparc) may generate wrong code
        , m_bIsDescriptor((0 == pTxtNode) ? true : false)
        , m_nSelectionStartPos(nSelStart)
        , m_nSelectionEndPos(nSelEnd)
        , m_xParentText(xParent)
    {
    }

    const SwTxtNode * GetTxtNode() const {
        return static_cast<const SwTxtNode*>(GetRegisteredIn());
    }
          SwTxtNode * GetTxtNode()       {
        return static_cast<SwTxtNode*>(GetRegisteredInNonConst());
    }

    SwTxtNode & GetTxtNodeOrThrow() {
        SwTxtNode *const pTxtNode( GetTxtNode() );
        if (!pTxtNode) {
            throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "SwXParagraph: disposed or invalid")), 0);
        }
        return *pTxtNode;
    }

    bool IsDescriptor() const { return m_bIsDescriptor; }

    void SetPropertyValues_Impl(
            const uno::Sequence< ::rtl::OUString >& rPropertyNames,
            const uno::Sequence< uno::Any >& rValues)
        throw (beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException);

    uno::Sequence< uno::Any >
        GetPropertyValues_Impl(
            const uno::Sequence< ::rtl::OUString >& rPropertyNames)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                uno::RuntimeException);

    uno::Sequence< beans::GetDirectPropertyTolerantResult >
        GetPropertyValuesTolerant_Impl(
            const uno::Sequence< ::rtl::OUString >& rPropertyNames,
            bool bDirectValuesOnly)
        throw (uno::RuntimeException);
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXParagraph::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
{
    ClientModify(this, pOld, pNew);
    if (!GetRegisteredIn())
    {
        m_ListenerContainer.Disposing();
    }
}

SwXParagraph::SwXParagraph()
    : m_pImpl( new SwXParagraph::Impl(*this) )
{
}

SwXParagraph::SwXParagraph(
        uno::Reference< text::XText > const & xParent,
        SwTxtNode & rTxtNode,
        const sal_Int32 nSelStart, const sal_Int32 nSelEnd)
    : m_pImpl(
        new SwXParagraph::Impl(*this, &rTxtNode, xParent, nSelStart, nSelEnd))
{
}

SwXParagraph::~SwXParagraph()
{
}

const SwTxtNode * SwXParagraph::GetTxtNode() const
{
    return m_pImpl->GetTxtNode();
}

bool SwXParagraph::IsDescriptor() const
{
    return m_pImpl->IsDescriptor();
}

uno::Reference<text::XTextContent>
SwXParagraph::CreateXParagraph(SwDoc & rDoc, SwTxtNode& rTxtNode,
        uno::Reference< text::XText> const& i_xParent,
        const sal_Int32 nSelStart, const sal_Int32 nSelEnd)
{
    // re-use existing SwXParagraph
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference<text::XTextContent> xParagraph;
    if ((-1 == nSelStart) && (-1 == nSelEnd)) // only use cache if no selection!
    {
        xParagraph.set(rTxtNode.GetXParagraph());
    }
    if (xParagraph.is())
    {
        return xParagraph;
    }

    // create new SwXParagraph
    uno::Reference<text::XText> xParentText(i_xParent);
    if (!xParentText.is())
    {
        SwPosition Pos( rTxtNode );
        xParentText.set(::sw::CreateParentXText( rDoc, Pos ));
    }
    SwXParagraph *const pXPara(
            new SwXParagraph(xParentText, rTxtNode, nSelStart, nSelEnd) );
    // this is why the constructor is private: need to acquire pXPara here
    xParagraph.set(pXPara);
    // in order to initialize the weak pointer cache in the core object
    if ((-1 == nSelStart) && (-1 == nSelEnd))
    {
        rTxtNode.SetXParagraph(xParagraph);
    }
    return xParagraph;
}

bool SwXParagraph::SelectPaM(SwPaM & rPaM)
{
    SwTxtNode const*const pTxtNode( GetTxtNode() );

    if (!pTxtNode)
    {
        return false;
    }

    *rPaM.GetPoint() = SwPosition( *pTxtNode );
    // set selection to the whole paragraph
    rPaM.SetMark();
    rPaM.GetMark()->nContent = pTxtNode->GetTxt().Len();
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
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXParagraph>(rId, this);
}

OUString SAL_CALL
SwXParagraph::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXParagraph");
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

static const size_t g_nServicesParagraph(
    sizeof(g_ServicesParagraph)/sizeof(g_ServicesParagraph[0]));

sal_Bool SAL_CALL
SwXParagraph::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesParagraph, g_ServicesParagraph, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXParagraph::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParagraph, g_ServicesParagraph);
}

void
SwXParagraph::attachToText(SwXText & rParent, SwTxtNode & rTxtNode)
{
    OSL_ENSURE(m_pImpl->m_bIsDescriptor, "Paragraph is not a descriptor");
    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_bIsDescriptor = false;
        rTxtNode.Add(m_pImpl.get());
        rTxtNode.SetXParagraph(uno::Reference<text::XTextContent>(this));
        m_pImpl->m_xParentText = &rParent;
        if (!m_pImpl->m_sText.isEmpty())
        {
            try { setString(m_pImpl->m_sText); }
            catch(...){}
            m_pImpl->m_sText = OUString();
        }
    }
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXParagraph::getPropertySetInfo()
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXParagraph::setPropertyValue(const OUString& rPropertyName,
        const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    uno::Sequence<uno::Any> aValues(1);
    aValues.getArray()[0] = rValue;
    m_pImpl->SetPropertyValues_Impl( aPropertyNames, aValues );
}

uno::Any
SwXParagraph::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    const uno::Sequence< uno::Any > aRet =
        m_pImpl->GetPropertyValues_Impl(aPropertyNames);
    return aRet.getConstArray()[0];
}

void SwXParagraph::Impl::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SwTxtNode & rTxtNode(GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
    const OUString* pPropertyNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    const SfxItemPropertyMap &rMap = m_rPropSet.getPropertyMap();
    SwParaSelection aParaSel( aCursor );
    for (sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( pPropertyNames[nProp] );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            throw beans::PropertyVetoException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only: "))
                    + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        SwUnoCursorHelper::SetPropertyValue(aCursor, m_rPropSet,
                pPropertyNames[nProp], pValues[nProp]);
    }
}

void SAL_CALL SwXParagraph::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
    lang::WrappedTargetException, uno::RuntimeException)
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

uno::Sequence< uno::Any > SwXParagraph::Impl::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SwTxtNode & rTxtNode(GetTxtNodeOrThrow());

    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    SwPosition aPos( rTxtNode );
    SwPaM aPam( aPos );
    uno::Any* pValues = aValues.getArray();
    const OUString* pPropertyNames = rPropertyNames.getConstArray();
    const SfxItemPropertyMap &rMap = m_rPropSet.getPropertyMap();
    const SwAttrSet& rAttrSet( rTxtNode.GetSwAttrSet() );
    for (sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( pPropertyNames[nProp] );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + pPropertyNames[nProp],
                static_cast< cppu::OWeakObject * >(&m_rThis));
        }
        if (! ::sw::GetDefaultTextContentValue(
                pValues[nProp], pPropertyNames[nProp], pEntry->nWID))
        {
            beans::PropertyState eTemp;
            const bool bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                *pEntry, aPam, &(pValues[nProp]), eTemp, &rTxtNode );
            if (!bDone)
            {
                m_rPropSet.getPropertyValue(
                    *pEntry, rAttrSet, pValues[nProp]);
            }
        }
    }
    return aValues;
}

uno::Sequence< uno::Any > SAL_CALL
SwXParagraph::getPropertyValues(const uno::Sequence< OUString >& rPropertyNames)
throw (uno::RuntimeException)
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
        throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                "Unknown property exception caught")),
            static_cast<cppu::OWeakObject *>(this));
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                "WrappedTargetException caught")),
            static_cast<cppu::OWeakObject *>(this));
    }

    return aValues;
}

void SAL_CALL SwXParagraph::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
throw (uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::addPropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXParagraph::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
throw (uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::removePropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXParagraph::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::firePropertiesChangeEvent(): not implemented");
}

/* disabled for #i46921# */

uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL
SwXParagraph::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (rPropertyNames.getLength() != rValues.getLength())
    {
        throw lang::IllegalArgumentException();
    }

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    //SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
    //const SwAttrSet& rAttrSet = ((SwTxtNode&)rTxtNode).GetSwAttrSet();
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

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
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
throw (uno::RuntimeException)
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
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return m_pImpl->GetPropertyValuesTolerant_Impl( rPropertyNames, true );
}

uno::Sequence< beans::GetDirectPropertyTolerantResult >
SwXParagraph::Impl::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        bool bDirectValuesOnly )
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(GetTxtNodeOrThrow());

    // #i46786# Use SwAttrSet pointer for determining the state.
    //          Use the value SwAttrSet (from the paragraph OR the style)
    //          for determining the actual value(s).
    const SwAttrSet* pAttrSet = rTxtNode.GetpSwAttrSet();
    const SwAttrSet& rValueAttrSet = rTxtNode.GetSwAttrSet();

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
                sal_Bool bAttrSetFetched = sal_True;
                beans::PropertyState eState = lcl_SwXParagraph_getPropertyState(
                            rTxtNode, &pAttrSet, *pEntry, bAttrSetFetched );
                rResult.State  = eState;

//                if (bDirectValuesOnly  &&  PropertyState_DIRECT_VALUE != eState)
//                    rResult.Result = beans::TolerantPropertySetResultType::NO_DIRECT_VALUE;
//                else
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
                        SwPosition aPos( rTxtNode );
                        SwPaM aPam( aPos );
                        // handle properties that are not part of the attribute
                        // and thus only pretendend to be paragraph attributes
                        beans::PropertyState eTemp;
                        const bool bDone =
                            SwUnoCursorHelper::getCrsrPropertyValue(
                                    *pEntry, aPam, &aValue, eTemp, &rTxtNode );

                        // if not found try the real paragraph attributes...
                        if (!bDone)
                        {
                            m_rPropSet.getPropertyValue(
                                *pEntry, rValueAttrSet, aValue );
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
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)))
            nWID = FN_UNO_ANCHOR_TYPE;
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES)))
            nWID = FN_UNO_ANCHOR_TYPES;
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_TEXT_WRAP)))
            nWID = FN_UNO_TEXT_WRAP;
        else
            return sal_False;
    }

    switch(nWID)
    {
        case FN_UNO_TEXT_WRAP:  rAny <<= text::WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= text::TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {   uno::Sequence<text::TextContentAnchorType> aTypes(1);
            text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
            rAny.setValue(&aTypes, ::getCppuType((uno::Sequence<text::TextContentAnchorType>*)0));
        }
        break;
        default:
            return sal_False;
    }
    return sal_True;
}

void SAL_CALL
SwXParagraph::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXParagraph::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_FAIL("SwXParagraph::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState lcl_SwXParagraph_getPropertyState(
//                          SwUnoCrsr& rUnoCrsr,
                            const SwTxtNode& rTxtNode,
                            const SwAttrSet** ppSet,
                            const SfxItemPropertySimpleEntry& rEntry,
                            sal_Bool &rAttrSetFetched )
throw (beans::UnknownPropertyException)
{
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;

    if(!(*ppSet) && !rAttrSetFetched )
    {
        (*ppSet) = rTxtNode.GetpSwAttrSet();
        rAttrSetFetched = sal_True;
    }
    SwPosition aPos( rTxtNode );
    SwPaM aPam( aPos );
    switch( rEntry.nWID )
    {
    case FN_UNO_NUM_RULES:
        // if numbering is set, return it; else do nothing
        SwUnoCursorHelper::getNumberingProperty( aPam, eRet, NULL );
        break;
    case FN_UNO_ANCHOR_TYPES:
        break;
    case RES_ANCHOR:
        if ( MID_SURROUND_SURROUNDTYPE != rEntry.nMemberId )
            goto lcl_SwXParagraph_getPropertyStateDEFAULT;
        break;
    case RES_SURROUND:
        if ( MID_ANCHOR_ANCHORTYPE != rEntry.nMemberId )
            goto lcl_SwXParagraph_getPropertyStateDEFAULT;
        break;
    case FN_UNO_PARA_STYLE:
    case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        {
            SwFmtColl* pFmt = SwUnoCursorHelper::GetCurTxtFmtColl(
                aPam, rEntry.nWID == FN_UNO_PARA_CONDITIONAL_STYLE_NAME);
            eRet = pFmt ? beans::PropertyState_DIRECT_VALUE
                        : beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
    case FN_UNO_PAGE_STYLE:
        {
            String sVal;
            SwUnoCursorHelper::GetCurPageStyle( aPam, sVal );
            eRet = sVal.Len() ? beans::PropertyState_DIRECT_VALUE
                              : beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
    lcl_SwXParagraph_getPropertyStateDEFAULT:
    default:
        if((*ppSet) && SFX_ITEM_SET == (*ppSet)->GetItemState(rEntry.nWID, sal_False))
            eRet = beans::PropertyState_DIRECT_VALUE;
        break;
    }
    return eRet;
}

beans::PropertyState SAL_CALL
SwXParagraph::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    const SwAttrSet* pSet = 0;
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    sal_Bool bDummy = sal_False;
    const beans::PropertyState eRet =
        lcl_SwXParagraph_getPropertyState(rTxtNode, &pSet, *pEntry, bDummy);
    return eRet;
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXParagraph::getPropertyStates(
        const uno::Sequence< OUString >& PropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    const OUString* pNames = PropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const SfxItemPropertyMap &rMap = m_pImpl->m_rPropSet.getPropertyMap();
    const SwAttrSet* pSet = 0;
    sal_Bool bAttrSetFetched = sal_False;

    for (sal_Int32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd;
            ++i, ++pStates, ++pNames)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            rMap.getByName( *pNames );
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + *pNames,
                static_cast<cppu::OWeakObject *>(this));
        }

        if (bAttrSetFetched && !pSet && isATR(pEntry->nWID))
        {
            *pStates = beans::PropertyState_DEFAULT_VALUE;
        }
        else
        {
            *pStates = lcl_SwXParagraph_getPropertyState(
                rTxtNode, &pSet, *pEntry, bAttrSetFetched );
        }
    }

    return aRet;
}

void SAL_CALL
SwXParagraph::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
    if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE))  ||
        rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES)) ||
        rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_TEXT_WRAP)))
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
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    if (pEntry->nWID < RES_FRMATR_END)
    {
        std::set<sal_uInt16> aWhichIds;
        aWhichIds.insert( pEntry->nWID );
        if (pEntry->nWID < RES_PARATR_BEGIN)
        {
            aCursor.GetDoc()->ResetAttrs(aCursor, sal_True, aWhichIds);
        }
        else
        {
            // for paragraph attributes the selection must be extended
            // to paragraph boundaries
            SwPosition aStart( *aCursor.Start() );
            SwPosition aEnd  ( *aCursor.End()   );
            ::std::auto_ptr<SwUnoCrsr> pTemp(
                aCursor.GetDoc()->CreateUnoCrsr(aStart, false) );
            if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
            {
                pTemp->MovePara(fnParaCurr, fnParaStart);
            }
            pTemp->SetMark();
            *pTemp->GetPoint() = aEnd;
            //pTemp->Exchange();
            SwUnoCursorHelper::SelectPam(*pTemp, true);
            if (!SwUnoCursorHelper::IsEndOfPara(*pTemp))
            {
                pTemp->MovePara(fnParaCurr, fnParaEnd);
            }
            pTemp->GetDoc()->ResetAttrs(*pTemp, sal_True, aWhichIds);
        }
    }
    else
    {
        SwUnoCursorHelper::resetCrsrPropertyValue(*pEntry, aCursor);
    }
}

uno::Any SAL_CALL
SwXParagraph::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SolarMutexGuard g;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

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
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    if (pEntry->nWID < RES_FRMATR_END)
    {
        const SfxPoolItem& rDefItem =
            rTxtNode.GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
        rDefItem.QueryValue(aRet, pEntry->nMemberId);
    }

    return aRet;
}

void SAL_CALL
SwXParagraph::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    // SwXParagraph will only created in order to be inserted by
    // 'insertTextContentBefore' or 'insertTextContentAfter' therefore
    // they cannot be attached
    throw uno::RuntimeException();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getAnchor() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
    // select paragraph
    SwParaSelection aParaSel( aCursor );
    const uno::Reference< text::XTextRange >  xRet =
        new SwXTextRange(aCursor, m_pImpl->m_xParentText);
    return xRet;
}

void SAL_CALL SwXParagraph::dispose() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode *const pTxtNode( m_pImpl->GetTxtNode() );
    if (pTxtNode)
    {
        SwCursor aCursor( SwPosition( *pTxtNode ), 0, false );
        // select paragraph
        {
            SwParaSelection aParaSel( aCursor );
            pTxtNode->GetDoc()->DelFullPara(aCursor);
        }
        m_pImpl->m_ListenerContainer.Disposing();
    }
}

void SAL_CALL SwXParagraph::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetTxtNode())
    {
        throw uno::RuntimeException();
    }
    m_pImpl->m_ListenerContainer.AddListener(xListener);
}

void SAL_CALL SwXParagraph::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetTxtNode() ||
        !m_pImpl->m_ListenerContainer.RemoveListener(xListener))
    {
        throw uno::RuntimeException();
    }
}

uno::Reference< container::XEnumeration >  SAL_CALL
SwXParagraph::createEnumeration() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwPaM aPam ( aPos );
    const uno::Reference< container::XEnumeration > xRef =
        new SwXTextPortionEnumeration(aPam, m_pImpl->m_xParentText,
            m_pImpl->m_nSelectionStartPos, m_pImpl->m_nSelectionEndPos);
    return xRef;
}

uno::Type SAL_CALL SwXParagraph::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL SwXParagraph::hasElements() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return (GetTxtNode()) ? sal_True : sal_False;
}

uno::Reference< text::XText > SAL_CALL
SwXParagraph::getText() throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getStart() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
    SwParaSelection aParaSel( aCursor );
    SwPaM aPam( *aCursor.Start() );
    uno::Reference< text::XText >  xParent = getText();
    const uno::Reference< text::XTextRange > xRet =
        new SwXTextRange(aPam, xParent);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXParagraph::getEnd() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwCursor aCursor( aPos, 0, false );
    SwParaSelection aParaSel( aCursor );
    SwPaM aPam( *aCursor.End() );
    uno::Reference< text::XText >  xParent = getText();
    const uno::Reference< text::XTextRange > xRet =
        new SwXTextRange(aPam, xParent);
    return xRet;
}

OUString SAL_CALL SwXParagraph::getString() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwTxtNode const*const pTxtNode( GetTxtNode() );
    if (pTxtNode)
    {
        SwPosition aPos( *pTxtNode );
        SwCursor aCursor( aPos, 0, false );
        SwParaSelection aParaSel( aCursor );
        SwUnoCursorHelper::GetTextFromPam(aCursor, aRet);
    }
    else if (m_pImpl->IsDescriptor())
    {
        aRet = m_pImpl->m_sText;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return aRet;
}

void SAL_CALL SwXParagraph::setString(const OUString& aString)
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwTxtNode const*const pTxtNode( GetTxtNode() );
    if (pTxtNode)
    {
        SwPosition aPos( *pTxtNode );
        SwCursor aCursor( aPos, 0, false );
        if (!SwUnoCursorHelper::IsStartOfPara(aCursor)) {
            aCursor.MovePara(fnParaCurr, fnParaStart);
        }
        SwUnoCursorHelper::SelectPam(aCursor, true);
        if (pTxtNode->GetTxt().Len()) {
            aCursor.MovePara(fnParaCurr, fnParaEnd);
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
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if ( rServiceName != "com.sun.star.text.TextContent" )
    {
        throw uno::RuntimeException();
    }

    SwTxtNode & rTxtNode(m_pImpl->GetTxtNodeOrThrow());

    SwPosition aPos( rTxtNode );
    SwPaM aPam( aPos );
    uno::Reference< container::XEnumeration > xRet =
        new SwXParaFrameEnumeration(aPam, PARAFRAME_PORTION_PARAGRAPH);
    return xRet;
}

uno::Sequence< OUString > SAL_CALL
SwXParagraph::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

// MetadatableMixin
::sfx2::Metadatable* SwXParagraph::GetCoreObject()
{
    SwTxtNode *const pTxtNode( m_pImpl->GetTxtNode() );
    return pTxtNode;
}

uno::Reference<frame::XModel> SwXParagraph::GetModel()
{
    SwTxtNode *const pTxtNode( m_pImpl->GetTxtNode() );
    if (pTxtNode)
    {
        SwDocShell const*const pShell( pTxtNode->GetDoc()->GetDocShell() );
        return (pShell) ? pShell->GetModel() : 0;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
