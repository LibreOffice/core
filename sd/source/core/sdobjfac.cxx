/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdobjfac.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:13:42 $
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

#pragma hdrstop

#include <basic/sbx.hxx>

#include <app.hxx>
#include "sdpage.hxx"
#include "sdobjfac.hxx"
#include "glob.hxx"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "drawdoc.hxx"


SdObjectFactory aSdObjectFactory;

/*************************************************************************
|*
|* void SdObjectFactory::MakeUserData()
|*
\************************************************************************/

IMPL_LINK( SdObjectFactory, MakeUserData, SdrObjFactory *, pObjFactory )
{
    if ( pObjFactory->nInventor == SdUDInventor )
    {
        SdrObject* pObj = pObjFactory->pObj;

        switch( pObjFactory->nIdentifier )
        {
            case( SD_ANIMATIONINFO_ID ):
            {
                SdDrawDocument* pDoc = NULL;

                if ( pObj )
                    pDoc = (SdDrawDocument*) pObj->GetModel();

                DBG_ASSERT(pDoc, "kein Model gefunden");

                pObjFactory->pNewData = new SdAnimationInfo(pDoc);
            }
            break;

            case( SD_IMAPINFO_ID ):
                pObjFactory->pNewData = new SdIMapInfo;
            break;

            default:
            break;
        }
    }

    if ( pObjFactory->pNewData )
        return 0;

    if( aOldMakeUserDataLink.IsSet() )
        aOldMakeUserDataLink.Call( this );

    return 0;
}


