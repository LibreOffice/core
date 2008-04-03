/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawdoc_animations.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:23:35 $
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

#include "drawdoc.hxx"
#include "cusshow.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

/** replaces a slide from all custom shows with a new one or removes a slide from
    all custom shows if pNewPage is 0.
*/
void SdDrawDocument::ReplacePageInCustomShows( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if ( mpCustomShowList )
    {
        for (ULONG i = 0; i < mpCustomShowList->Count(); i++)
        {
            SdCustomShow* pCustomShow = (SdCustomShow*) mpCustomShowList->GetObject(i);
            if( pNewPage == 0 )
                pCustomShow->RemovePage(pOldPage);
            else
                pCustomShow->ReplacePage(pOldPage,pNewPage);
        }
    }
}

extern Reference< XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument );

const Reference< XPresentation2 >& SdDrawDocument::getPresentation() const
{
    if( !mxPresentation.is() )
    {
        const_cast< SdDrawDocument* >( this )->mxPresentation = CreatePresentation(*this);
    }
    return mxPresentation;
}
