/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoobj.hxx,v $
 *
 * $Revision: 1.49 $
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

#ifndef SW_UNOPARAGRAPH_HXX
#define SW_UNOPARAGRAPH_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <cppuhelper/implbase10.hxx>

#include <sfx2/Metadatable.hxx>

#include <calbck.hxx>
#include <unoevtlstnr.hxx>
#include <unobaseclass.hxx>


class SfxItemPropertySet;
struct SwPosition;
class SwUnoCrsr;
class SwStartNode;
class SwTxtNode;
class SwTable;
class SwXText;


/*-----------------07.04.98 08:15-------------------

--------------------------------------------------*/
typedef ::cppu::ImplInheritanceHelper10
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::beans::XPropertyState
,   ::com::sun::star::beans::XMultiPropertySet
,   ::com::sun::star::beans::XTolerantMultiPropertySet
,   ::com::sun::star::container::XEnumerationAccess
,   ::com::sun::star::container::XContentEnumerationAccess
,   ::com::sun::star::text::XTextContent
,   ::com::sun::star::text::XTextRange
> SwXParagraph_Base;

class SwXParagraph
    : public SwXParagraph_Base
    , public SwClient
{

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    SwEventListenerContainer    aLstnrCntnr;
    const SfxItemPropertySet*   m_pPropSet;
    ::rtl::OUString             m_sText;
    sal_Int32                   nSelectionStartPos;
    sal_Int32                   nSelectionEndPos;
    BOOL                        m_bIsDescriptor;

protected:

    void SAL_CALL SetPropertyValues_Impl(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
        GetPropertyValues_Impl(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL
        GetPropertyValuesTolerant_Impl(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            sal_Bool bDirectValuesOnly)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ~SwXParagraph();

public:

    SwXParagraph();
    SwXParagraph(::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XText > const & i_xParent,
            SwTxtNode * i_pTxtNode,
            sal_Int32 nSelStart = -1, sal_Int32 nSelEnd = - 1);

    BOOL            IsDescriptor() const {return m_bIsDescriptor;}

    const SwTxtNode * GetTxtNode() const;
          SwTxtNode * GetTxtNode();

    static BOOL getDefaultTextContentValue(::com::sun::star::uno::Any& rAny,
        const ::rtl::OUString& rPropertyName, USHORT nWID = 0);
    static SwXParagraph* GetImplementation(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface> xRef);
    void attachToText(SwXText & rParent, SwTxtNode & rTxtNode);

    TYPEINFO();

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel();

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL
        getPropertyState(const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const ::com::sun::star::uno::Sequence<
                ::rtl::OUString >& rPropertyNames)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault(
            const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
            const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getPropertyValues(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);

    // XTolerantMultiPropertySet
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException);

    // XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XEnumeration > SAL_CALL
        createContentEnumeration(const ::rtl::OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAvailableServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException);

    // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getStart()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getEnd()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString(const ::rtl::OUString& rString)
        throw (::com::sun::star::uno::RuntimeException);

};


/*-----------------07.04.98 08:10-------------------

--------------------------------------------------*/
class SwXParagraphEnumeration
    : public SwSimpleEnumerationBaseClass
    , public SwClient
{

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        xParentText;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >
        xNextPara;

    const SwTable *     pOwnTable;
    /// Start node of the cell the enumeration belongs to.
    /// Used to restrict the movement of the UNO cursor to the cell and its
    /// embedded tables.
    const SwStartNode * pOwnStartNode;
    sal_Int32           nFirstParaStart;
    sal_Int32           nLastParaEnd;
    ULONG               nEndIndex;
    CursorType          eCursorType;
    BOOL                bFirstParagraph;

    SwUnoCrsr*          GetCrsr() {return (SwUnoCrsr*)GetRegisteredIn();}

protected:

    virtual ~SwXParagraphEnumeration();

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > SAL_CALL
        NextElement_Impl()
        throw (::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

public:

    SwXParagraphEnumeration(
            SwXText* pParent, SwPosition& rPos, CursorType eType);
    SwXParagraphEnumeration(
            SwXText* pParent, SwUnoCrsr* pCrsr, CursorType eType);

    // non-Uno functions

    void                SetOwnTable(const SwTable* pTable)
        { pOwnTable = pTable; }
    const SwTable*      GetOwnTable() const
        { return pOwnTable; }
    void                SetOwnStartNode(const SwStartNode* pNode)
        { pOwnStartNode = pNode; }
    const SwStartNode*  GetOwnStartNode() const
        { return pOwnStartNode; }

    // SwClient
    virtual void        Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw (::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

};

#endif // SW_UNOPARAGRAPH_HXX

