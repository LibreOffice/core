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

#ifndef SW_UNOPARAFRAMEENUM_HXX
#define SW_UNOPARAFRAMEENUM_HXX

#include <deque>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <cppuhelper/implbase2.hxx>

#include <unobaseclass.hxx>


class SwDepend;
class SwNodeIndex;
class SwPaM;
class SwFrmFmt;


struct FrameDependSortListEntry
{
    xub_StrLen nIndex;
    sal_uInt32 nOrder;
    ::boost::shared_ptr<SwDepend> pFrameDepend;

    FrameDependSortListEntry (xub_StrLen const i_nIndex,
                sal_uInt32 const i_nOrder, SwDepend * const i_pDepend)
        : nIndex(i_nIndex), nOrder(i_nOrder), pFrameDepend(i_pDepend) { }
};

typedef ::std::deque< FrameDependSortListEntry >
    FrameDependSortList_t;

typedef ::std::deque< ::boost::shared_ptr<SwDepend> >
    FrameDependList_t;


// #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         FrameDependSortList_t & rFrames,
                         const bool _bAtCharAnchoredObjs );

enum ParaFrameMode
{
    PARAFRAME_PORTION_PARAGRAPH,
    PARAFRAME_PORTION_CHAR,
    PARAFRAME_PORTION_TEXTRANGE,
};

typedef ::cppu::WeakImplHelper2
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumeration
> SwXParaFrameEnumeration_Base;

class SwXParaFrameEnumeration
    : public SwXParaFrameEnumeration_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXParaFrameEnumeration();

public:

    SwXParaFrameEnumeration(const SwPaM& rPaM,
        const enum ParaFrameMode eParaFrameMode, SwFrmFmt *const pFmt = 0);

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

#endif // SW_UNOPARAFRAMEENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
