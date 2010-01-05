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

#ifndef SW_UNOBOOKMARK_HXX
#define SW_UNOBOOKMARK_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XFormField.hpp>

#include <cppuhelper/implbase5.hxx>

#include <tools/string.hxx>
#include <sfx2/Metadatable.hxx>

#include <IDocumentMarkAccess.hxx>
#include <calbck.hxx>
#include <unoevtlstnr.hxx>


class SwDoc;


typedef ::cppu::ImplInheritanceHelper5
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::container::XNamed
,   ::com::sun::star::text::XTextContent
> SwXBookmark_Base;

class SwXBookmark
    : public SwXBookmark_Base
    , private SwClient
{

private:

    SwEventListenerContainer m_aLstnrCntnr;
    SwDoc* m_pDoc;
    String m_aName;
    ::sw::mark::IMark* m_pRegisteredBookmark;

    void registerInMark(::sw::mark::IMark* const pBkmk);

protected:

    virtual ~SwXBookmark();

public:

    SwXBookmark(::sw::mark::IMark* pMark = 0, SwDoc* pDoc = 0);

    const ::sw::mark::IMark* GetBookmark() const
        { return m_pRegisteredBookmark; }
          ::sw::mark::IMark* GetBookmark()
        { return m_pRegisteredBookmark; }
    SwDoc* GetDoc()
        { return m_pDoc; }

    void attachToRangeEx(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange,
            IDocumentMarkAccess::MarkType eType)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException );
    virtual void attachToRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    TYPEINFO();

    // SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject();
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > GetModel();

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

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName(const ::rtl::OUString& rName)
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

};

typedef cppu::ImplInheritanceHelper1< SwXBookmark,
    ::com::sun::star::text::XFormField > SwXFieldmark_Base;

class SwXFieldmark
    : public SwXFieldmark_Base
{

private:

    bool isReplacementObject;

public:

    SwXFieldmark(bool isReplacementObject,
            ::sw::mark::IMark* pBkm = 0, SwDoc* pDoc = 0);

    virtual void attachToRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getDescription()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getRes()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setDescription(const ::rtl::OUString& rDescription)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setType(::sal_Int16 fieldType)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRes(::sal_Int16 res)
        throw (::com::sun::star::uno::RuntimeException);

};

#endif // SW_UNOBOOKMARK_HXX

