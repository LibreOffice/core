/*************************************************************************
 *
 *  $RCSfile: pglink.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
#include <so3/iface.hxx>
#ifndef _SVXLINK_HXX
#include <svx/linkmgr.hxx>
#endif

#include "pglink.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"


/*************************************************************************
|*
|*      Ctor
|*
\************************************************************************/

SdPageLink::SdPageLink(SdPage* pPg, const String& rFileName,
                       const String& rBookmarkName) :
    SvBaseLink(LINKUPDATE_ONCALL, FORMAT_FILE),
    pPage(pPg)
{
    pPage->SetFileName(rFileName);
    pPage->SetBookmarkName(rBookmarkName);
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/


__EXPORT SdPageLink::~SdPageLink()
{
}

/*************************************************************************
|*
|* Daten haben sich geaendert
|*
\************************************************************************/

void __EXPORT SdPageLink::DataChanged(SvData& rData)
{
    SdDrawDocument* pDoc = (SdDrawDocument*) pPage->GetModel();
    SvxLinkManager* pLinkManager = pDoc!=NULL ? pDoc->GetLinkManager() : NULL;

    if (pLinkManager)
    {
        /**********************************************************************
        * Nur Standardseiten duerfen gelinkt sein
        * Die entsprechenden Notizseiten werden automatisch aktualisiert
        **********************************************************************/
        String aFileName;
        String aBookmarkName;
        String aFilterName;
        pLinkManager->GetDisplayNames(*this,0, &aFileName, &aBookmarkName,
                                      &aFilterName);
        pPage->SetFileName(aFileName);
        pPage->SetBookmarkName(aBookmarkName);

        SdDrawDocument* pBookmarkDoc = pDoc->OpenBookmarkDoc(aFileName);

        if (pBookmarkDoc)
        {
            /******************************************************************
            * Die gelinkte Seite wird im Model replaced
            ******************************************************************/
            if (aBookmarkName.Len() == 0)
            {
                // Kein Seitenname angegeben: es wird die erste Seite genommen
                aBookmarkName = pBookmarkDoc->GetSdPage(0, PK_STANDARD)->GetName();
                pPage->SetBookmarkName(aBookmarkName);
            }

            List aBookmarkList;
            aBookmarkList.Insert(&aBookmarkName);
            USHORT nInsertPos = pPage->GetPageNum();
            BOOL bLink = TRUE;
            BOOL bReplace = TRUE;
            BOOL bNoDialogs = FALSE;
            BOOL bCopy = FALSE;

            if( pDoc->pDocLockedInsertingLinks )
            {
                // resolving links while loading pDoc
                bNoDialogs = TRUE;
                bCopy = TRUE;
            }

            pDoc->InsertBookmarkAsPage(&aBookmarkList, NULL, bLink, bReplace,
                                       nInsertPos, bNoDialogs, NULL, bCopy, TRUE);

            if( !pDoc->pDocLockedInsertingLinks )
                pDoc->CloseBookmarkDoc();
        }
    }
}

/*************************************************************************
|*
|* Link an oder abmelden
|*
\************************************************************************/

void __EXPORT SdPageLink::Closed()
{
    // Die Verbindung wird aufgehoben
    pPage->SetFileName(String());
    pPage->SetBookmarkName(String());

    SvBaseLink::Closed();
}



