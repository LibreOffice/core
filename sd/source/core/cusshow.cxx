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

#include "sdiocmpt.hxx"
#include "cusshow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

#include <tools/tenccvt.hxx>

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc)
  : maPages(),
  pDoc(pDrawDoc)
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
    pDoc = rShow.GetDoc();
}

SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow )
  : maPages(),
  pDoc(pDrawDoc),
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

extern uno::Reference< uno::XInterface > createUnoCustomShow( SdCustomShow* pShow );

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

SdCustomShow::PageVec& SdCustomShow::PagesVector()
{
    return maPages;
}

void SdCustomShow::ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if( !pNewPage )
    {
        RemovePage(pOldPage);
    }
    else
    {
        ::std::replace(maPages.begin(), maPages.end(), pOldPage, pNewPage);
    }
}

void SdCustomShow::RemovePage( const SdPage* pPage )
{
    maPages.erase(::std::remove(maPages.begin(), maPages.end(), pPage), maPages.end());
}

void   SdCustomShow::SetName(const OUString& rName)
{
    aName = rName;
}

OUString SdCustomShow::GetName() const
{
    return aName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
