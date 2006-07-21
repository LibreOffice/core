/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lnktrans.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:09:29 $
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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------



#include <svtools/urlbmk.hxx>

#include "lnktrans.hxx"
#include "scmod.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

ScLinkTransferObj::ScLinkTransferObj()
{
}

ScLinkTransferObj::~ScLinkTransferObj()
{
}

void ScLinkTransferObj::SetLinkURL( const String& rURL, const String& rText )
{
    aLinkURL = rURL;
    aLinkText = rText;
}

void ScLinkTransferObj::AddSupportedFormats()
{
    if ( aLinkURL.Len() )
    {
        //  TransferableHelper::SetINetBookmark formats

        AddFormat( SOT_FORMATSTR_ID_SOLK );
        AddFormat( SOT_FORMAT_STRING );
        AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
        AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
        AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
        AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
    }
}

sal_Bool ScLinkTransferObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_Bool bOK = sal_False;
    if ( aLinkURL.Len() )
    {
        INetBookmark aBmk( aLinkURL, aLinkText );
        bOK = SetINetBookmark( aBmk, rFlavor );
    }
    return bOK;
}

void ScLinkTransferObj::ObjectReleased()
{
    TransferableHelper::ObjectReleased();
}

void ScLinkTransferObj::DragFinished( sal_Int8 nDropAction )
{
    ScModule* pScMod = SC_MOD();
    pScMod->ResetDragObject();

    TransferableHelper::DragFinished( nDropAction );
}


