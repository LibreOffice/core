/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVXLINK_HXX
#include <sfx2/linkmgr.hxx>
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
    ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, FORMAT_FILE),
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


SdPageLink::~SdPageLink()
{
}

/*************************************************************************
|*
|* Daten haben sich geaendert
|*
\************************************************************************/

void SdPageLink::DataChanged( const String& ,
                                       const ::com::sun::star::uno::Any& )
{
    SdDrawDocument* pDoc = (SdDrawDocument*) pPage->GetModel();
    sfx2::LinkManager* pLinkManager = pDoc!=NULL ? pDoc->GetLinkManager() : NULL;

    if (pLinkManager)
    {
        /**********************************************************************
        * Nur Standardseiten duerfen gelinkt sein
        * Die entsprechenden Notizseiten werden automatisch aktualisiert
        **********************************************************************/
        String aFileName;
        String aBookmarkName;
        String aFilterName;
        pLinkManager->GetDisplayNames( this,0, &aFileName, &aBookmarkName,
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
            sal_uInt16 nInsertPos = pPage->GetPageNum();
            sal_Bool bLink = sal_True;
            sal_Bool bReplace = sal_True;
            sal_Bool bNoDialogs = sal_False;
            sal_Bool bCopy = sal_False;

            if( pDoc->pDocLockedInsertingLinks )
            {
                // resolving links while loading pDoc
                bNoDialogs = sal_True;
                bCopy = sal_True;
            }

            pDoc->InsertBookmarkAsPage(&aBookmarkList, NULL, bLink, bReplace,
                                       nInsertPos, bNoDialogs, NULL, bCopy, sal_True, sal_True);

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

void SdPageLink::Closed()
{
    // Die Verbindung wird aufgehoben
    pPage->SetFileName(String());
    pPage->SetBookmarkName(String());

    SvBaseLink::Closed();
}



