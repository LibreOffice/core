/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: forbiuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:39:40 $
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



#include <svx/forbiddencharacterstable.hxx>

#include "forbiuno.hxx"
#include "docsh.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

vos::ORef<SvxForbiddenCharactersTable> lcl_GetForbidden( ScDocShell* pDocSh )
{
    vos::ORef<SvxForbiddenCharactersTable> xRet;
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        xRet = pDoc->GetForbiddenCharacters();
        if ( !xRet.isValid() )
        {
            //  create an empty SvxForbiddenCharactersTable for SvxUnoForbiddenCharsTable,
            //  so changes can be stored.

            xRet = new SvxForbiddenCharactersTable( pDoc->GetServiceManager() );
            pDoc->SetForbiddenCharacters( xRet );
        }
    }
    return xRet;
}

ScForbiddenCharsObj::ScForbiddenCharsObj( ScDocShell* pDocSh ) :
    SvxUnoForbiddenCharsTable( lcl_GetForbidden( pDocSh ) ),
    pDocShell( pDocSh )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScForbiddenCharsObj::~ScForbiddenCharsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScForbiddenCharsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // document gone
    }
}

void ScForbiddenCharsObj::onChange()
{
    if (pDocShell)
    {
        pDocShell->GetDocument()->SetForbiddenCharacters( mxForbiddenChars );
        pDocShell->PostPaintGridAll();
        pDocShell->SetDocumentModified();
    }
}

