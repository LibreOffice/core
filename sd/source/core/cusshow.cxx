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

#include <com/sun/star/lang/XComponent.hpp>

#include <createunocustomshow.hxx>
#include <cusshow.hxx>
#include <customshowlist.hxx>

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow()
  : maPages()
{
}

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow( const SdCustomShow& rShow )
    : maPages(rShow.maPages)
{
    aName = rShow.GetName();
}

SdCustomShow::SdCustomShow(css::uno::Reference< css::uno::XInterface > const & xShow )
  : maPages(),
  mxUnoCustomShow( xShow )
{
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdCustomShow::~SdCustomShow()
{
    uno::Reference< uno::XInterface > xShow( mxUnoCustomShow );
    uno::Reference< lang::XComponent > xComponent( xShow, uno::UNO_QUERY );
    if( xComponent.is() )
        xComponent->dispose();
}

uno::Reference< uno::XInterface > SdCustomShow::getUnoCustomShow()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShow( mxUnoCustomShow );

    if( !xShow.is() )
    {
        xShow = createUnoCustomShow( this );
    }

    return xShow;
}

void SdCustomShow::ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if( !pNewPage )
    {
        maPages.erase(::std::remove(maPages.begin(), maPages.end(), pOldPage), maPages.end());
    }
    else
    {
        ::std::replace(maPages.begin(), maPages.end(), pOldPage, pNewPage);
    }
}

void SdCustomShow::SetName(const OUString& rName)
{
    aName = rName;
}

void SdCustomShowList::erase(std::vector<std::unique_ptr<SdCustomShow>>::iterator it)
{
    mShows.erase(it);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
