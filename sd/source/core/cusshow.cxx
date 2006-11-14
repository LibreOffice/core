/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cusshow.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:21:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include "sdiocmpt.hxx"
#include "cusshow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

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
