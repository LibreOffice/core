/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swacorr.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 21:27:34 $
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
#include "precompiled_sw.hxx"



#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SWACORR_HXX
#include <swacorr.hxx>
#endif
#ifndef _SWBLOCKS_HXX
#include <swblocks.hxx>
#endif
#ifndef _SW_XMLTEXTBLOCKS_HXX
#include "SwXMLTextBlocks.hxx"
#endif
#ifndef _SWSERROR_H
#include <swerror.h>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#include <sot/storage.hxx>

TYPEINIT1( SwAutoCorrect, SvxAutoCorrect );


    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
    //      rShort ist der Stream-Name - gecryptet!

BOOL SwAutoCorrect::GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const String& rFileName, const String& rShort, String& rLong )
{
    ULONG nRet;
    if (rStg.is())
    {
        // mba: relative URLs don't make sense here
        SwXMLTextBlocks aBlk( rStg, rFileName );
        nRet = aBlk.GetText( rShort, rLong );
    }
    else
        ASSERT ( rStg.is(), "Someone passed SwAutoCorrect::GetLongText a dud storage!");
    return !IsError( nRet ) && rLong.Len();
}

    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    //      rShort ist der Stream-Name - gecryptet!
BOOL SwAutoCorrect::PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&  rStg, const String& rFileName, const String& rShort,
                            SfxObjectShell& rObjSh, String& rLong )
{
    if( !rObjSh.IsA( TYPE(SwDocShell) ) )
        return FALSE;

    SwDocShell& rDShell = (SwDocShell&)rObjSh;
    ULONG nRet = 0;

    // mba: relative URLs don't make sense here
    SwXMLTextBlocks aBlk( rStg, rFileName );
    SwDoc* pDoc = aBlk.GetDoc();

    nRet = aBlk.BeginPutDoc( rShort, rShort );
    if( !IsError( nRet ) )
    {
        ((SwEditShell*)rDShell.GetWrtShell())->_CopySelToDoc( pDoc );
        nRet = aBlk.PutDoc();
        aBlk.AddName ( rShort, rShort, FALSE );
        if( !IsError( nRet ) )
            nRet = aBlk.GetText( rShort, rLong );
    }
    return !IsError( nRet );
}


SwAutoCorrect::SwAutoCorrect( const SvxAutoCorrect& rACorr )
    : SvxAutoCorrect( rACorr )
{
    SwEditShell::SetAutoFmtFlags(&GetSwFlags());
}

SwAutoCorrect::~SwAutoCorrect()
{
}
