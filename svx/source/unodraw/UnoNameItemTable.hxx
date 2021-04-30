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

#ifndef INCLUDED_SVX_SOURCE_UNODRAW_UNONAMEITEMTABLE_HXX
#define INCLUDED_SVX_SOURCE_UNODRAW_UNONAMEITEMTABLE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>
#include <vector>
#include <svl/lstner.hxx>
#include <svx/xit.hxx>

class SdrModel;
class SfxItemPool;
class SfxItemSet;

typedef std::vector< std::unique_ptr< SfxItemSet > > ItemPoolVector;
class SvxUnoNameItemTable : public cppu::WeakImplHelper< css::container::XNameContainer, css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpModelPool;
    sal_uInt16          mnWhich;
    sal_uInt8           mnMemberId;

    ItemPoolVector maItemSetVector;

    void ImplInsertByName( const OUString& aName, const css::uno::Any& aElement );

public:
    SvxUnoNameItemTable( SdrModel* pModel, sal_uInt16 nWhich, sal_uInt8 nMemberId ) noexcept;
    virtual ~SvxUnoNameItemTable() noexcept override;

    virtual NameOrIndex* createItem() const = 0;
    virtual bool isValid( const NameOrIndex* pItem ) const;

    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) noexcept override;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  css::uno::Any& aElement ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const  OUString& aName ) override;
    virtual css::uno::Sequence<  OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) override;
};

#endif // INCLUDED_SVX_SOURCE_UNODRAW_UNONAMEITEMTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
