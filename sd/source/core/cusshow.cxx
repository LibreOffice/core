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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <com/sun/star/lang/XComponent.hpp>

#include "sdiocmpt.hxx"
#include "cusshow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

// #90477#
#include <tools/tenccvt.hxx>

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc)
  : List(),
  pDoc(pDrawDoc)
{
}

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow( const SdCustomShow& rShow )
    : List( rShow )
{
    aName = rShow.GetName();
    pDoc = rShow.GetDoc();
}

SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow )
  : List(),
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

void SdCustomShow::ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if( !pNewPage )
    {
        RemovePage( pOldPage );
    }
    else
    {
        sal_uLong nPos;
        while( (nPos = GetPos( (void*)pOldPage )) != CONTAINER_ENTRY_NOTFOUND  )
        {
            Replace( (void*)pNewPage, nPos );
        }
    }
}

void SdCustomShow::RemovePage( const SdPage* pPage )
{
    sal_uLong nPos;
    while( (nPos = GetPos( (void*)pPage )) != CONTAINER_ENTRY_NOTFOUND  )
    {
        Remove( nPos );
    }
}

void   SdCustomShow::SetName(const String& rName)
{
    aName = rName;
}

String SdCustomShow::GetName() const
{
    return aName;
}

