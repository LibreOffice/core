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


#ifndef _LINKMGR_HXX
#define _LINKMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sfx2/linksrc.hxx>
#include <tools/string.hxx>
#include <svl/svarray.hxx>

class SfxObjectShell;
class Graphic;
class Size;

namespace sfx2
{
    // Damit der Link ueber den Status der zu ladenen Grafik informierten werden
    // verschickt das FileObject ein SvData, mit der FormatId
    // "RegisterStatusInfoId" und ein einem String als Datentraeger. Dieser
    // enthaelt den folgenden enum.

class SvBaseLink;
class SvBaseLinkRef;

typedef SvBaseLinkRef* SvBaseLinkRefPtr;
SV_DECL_PTRARR( SvBaseLinks, SvBaseLinkRefPtr, 1, 1 )

typedef SvLinkSource* SvLinkSourcePtr;
SV_DECL_PTRARR( SvLinkSources, SvLinkSourcePtr, 1, 1 )

class SFX2_DLLPUBLIC LinkManager
{
    SvBaseLinks     aLinkTbl;
    SvLinkSources aServerTbl;

    SfxObjectShell *pPersist; // LinkMgr muss vor SfxObjectShell freigegeben werden
protected:
    sal_Bool        InsertLink( SvBaseLink* pLink, sal_uInt16 nObjType, sal_uInt16 nUpdateType,
                            const String* pName = 0 );
public:

    enum LinkState
    {
        STATE_LOAD_OK,
        STATE_LOAD_ERROR,
        STATE_LOAD_ABORT
    };

                LinkManager( SfxObjectShell * pCacheCont );
                ~LinkManager();

    SfxObjectShell*    GetPersist() const              { return pPersist; }
    void        SetPersist( SfxObjectShell * p )   { pPersist = p; }

    void        Remove( SvBaseLink *pLink );
    void        Remove( sal_uInt16 nPos, sal_uInt16 nCnt = 1 );
    sal_Bool        Insert( SvBaseLink* pLink );

                // den Link mit einem SvLinkSource verbinden und in die Liste eintragen
    sal_Bool        InsertDDELink( SvBaseLink*,
                        const String& rServer,
                        const String& rTopic,
                        const String& rItem );

                // falls am Link schon alles eingestellt ist !
    sal_Bool        InsertDDELink( SvBaseLink* );

    // den Link mit einem PseudoObject verbinden und in die Liste eintragen
    sal_Bool InsertFileLink( sfx2::SvBaseLink&,
                        sal_uInt16 nFileType,
                        const String& rTxt,
                        const String* pFilterNm = 0,
                        const String* pRange = 0 );

            // falls am Link schon alles eingestellt ist !
    sal_Bool InsertFileLink( sfx2::SvBaseLink& );

                // erfrage die Strings fuer den Dialog
    sal_Bool GetDisplayNames( const SvBaseLink *,
                                    String* pType,
                                    String* pFile = 0,
                                    String* pLink = 0,
                                    String* pFilter = 0 ) const;

    SvLinkSourceRef CreateObj( SvBaseLink* );

    void        UpdateAllLinks( sal_Bool bAskUpdate = sal_True,
                                sal_Bool bCallErrHdl = sal_True,
                                sal_Bool bUpdateGrfLinks = sal_False,
                                Window* pParentWin = 0 );

                // Liste aller Links erfragen (z.B. fuer Verknuepfungs-Dialog)
    const       SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Serverseitige Verwaltung --------------------

                // Liste der zu serviereden Links erfragen
    const SvLinkSources& GetServers() const { return aServerTbl; }
                // einen zu servierenden Link eintragen/loeschen
    sal_Bool        InsertServer( SvLinkSource* rObj );
    void        RemoveServer( SvLinkSource* rObj );
    void        RemoveServer( sal_uInt16 nPos, sal_uInt16 nCnt = 1 )
                {   aServerTbl.Remove( nPos, nCnt ); }

    // eine Uebertragung wird abgebrochen, also alle DownloadMedien canceln
    // (ist zur Zeit nur fuer die FileLinks interressant!)
    void CancelTransfers();

    // um Status Informationen aus dem FileObject an den BaseLink zu
    // senden, gibt es eine eigene ClipBoardId. Das SvData-Object hat
    // dann die entsprechenden Informationen als String.
    // Wird zur Zeit fuer FileObject in Verbindung mit JavaScript benoetigt
    // - das braucht Informationen ueber Load/Abort/Error
    static sal_uIntPtr  RegisterStatusInfoId();

    // if the mimetype says graphic/bitmap/gdimetafile then get the
    // graphic from the Any. Return says no errors
    static sal_Bool GetGraphicFromAny( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf );

private:
                LinkManager( const LinkManager& );
                LinkManager& operator=( const LinkManager& );
};

// Trenner im LinkName fuer die DDE-/File-/Grafik- Links
// (nur wer es braucht, um einen SvLinkName zusammenzubasteln)
const sal_Unicode cTokenSeperator = 0xFFFF;

// erzeuge einen String fuer den SvLinkName. Fuer
// - DDE die ersten 3 Strings, (Server, Topic, Item)
// - File-/Grf-LinkNms die letzen 3 Strings (FileName, Bereich, Filter)
SFX2_DLLPUBLIC void MakeLnkName( String& rName,
                 const String* pType,       // kann auch 0 sein !!
                 const String& rFile,
                 const String& rLink,
                 const String* pFilter = 0 );

}

#endif

