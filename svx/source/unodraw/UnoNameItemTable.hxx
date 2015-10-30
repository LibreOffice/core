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

#include <vector>
#include <svl/lstner.hxx>
#include <svx/xit.hxx>

#include <svx/xdef.hxx>

class SdrModel;
class SfxItemPool;
class SfxItemSet;

typedef std::vector< SfxItemSet* > ItemPoolVector;
class SvxUnoNameItemTable : public cppu::WeakImplHelper< css::container::XNameContainer, css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpModelPool;
    sal_uInt16          mnWhich;
    sal_uInt8           mnMemberId;

    ItemPoolVector maItemSetVector;

    void SAL_CALL ImplInsertByName( const OUString& aName, const css::uno::Any& aElement );

public:
    SvxUnoNameItemTable( SdrModel* pModel, sal_uInt16 nWhich, sal_uInt8 nMemberId ) throw();
    virtual ~SvxUnoNameItemTable() throw();

    virtual NameOrIndex* createItem() const throw() = 0;
    virtual bool isValid( const NameOrIndex* pItem ) const;

    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw () override;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  css::uno::Any& aElement ) throw( css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  css::uno::Any& aElement ) throw( css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const  OUString& aName ) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SVX_SOURCE_UNODRAW_UNONAMEITEMTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
