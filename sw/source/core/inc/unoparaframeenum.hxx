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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX

#include <deque>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <cppuhelper/implbase.hxx>

#include <unobaseclass.hxx>

class SwDepend;
class SwNodeIndex;
class SwPaM;
class SwFrameFormat;

struct FrameDependSortListEntry
{
    sal_Int32 nIndex;
    sal_uInt32 nOrder;
    ::boost::shared_ptr<SwDepend> pFrameDepend;

    FrameDependSortListEntry (sal_Int32 const i_nIndex,
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

typedef ::cppu::WeakImplHelper
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
        const enum ParaFrameMode eParaFrameMode, SwFrameFormat *const pFormat = 0);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
