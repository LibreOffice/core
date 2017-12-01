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
    // The FileObject sends a SvData with the FormatId to notify the link about
    // the status of the to be loaded graphic "RegisterStatusInfoId" and a string
    // as data carrier. This contains the following enum.

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

    SfxObjectShell *pPersist; // LinkMgr has to be deallocated before SfxObjectShell

    sal_Bool mAutoAskUpdateAllLinks;
    sal_Bool mUpdateAsked;
    sal_Bool mAllowUpdate;

    sal_Bool GetUserAllowsLinkUpdate(Window *pParent);
    void SetUserAllowsLinkUpdate(SvBaseLink *pLink, sal_Bool allows);
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

                // connect the link with a SvLinkSource and insert into the list
    sal_Bool        InsertDDELink( SvBaseLink*,
                        const String& rServer,
                        const String& rTopic,
                        const String& rItem );

                // in case that all was already adjusted at the link!
    sal_Bool        InsertDDELink( SvBaseLink* );

    // connect the link with a PseudoObject and insert into the list
    sal_Bool InsertFileLink( sfx2::SvBaseLink&,
                        sal_uInt16 nFileType,
                        const String& rTxt,
                        const String* pFilterNm = 0,
                        const String* pRange = 0 );

            // in case that all was already adjusted at the link!
    sal_Bool InsertFileLink( sfx2::SvBaseLink& );

                // ask for the strings to be used in the dialog
    sal_Bool GetDisplayNames( const SvBaseLink *,
                                    String* pType,
                                    String* pFile = 0,
                                    String* pLink = 0,
                                    String* pFilter = 0 ) const;

    SvLinkSourceRef CreateObj( SvBaseLink* );


    // Automatically ask user about update all links, on first insert
    void SetAutoAskUpdateAllLinks();

    void        UpdateAllLinks( sal_Bool bAskUpdate = sal_True,
                                sal_Bool bCallErrHdl = sal_True,
                                sal_Bool bUpdateGrfLinks = sal_False,
                                Window* pParentWin = 0 );

                // ask for the list of all links (e.g, for the link dialog)
    const       SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Server-side Management --------------------

                // ask for the list of all links that get served
    const SvLinkSources& GetServers() const { return aServerTbl; }
                // insert/delete a served link
    sal_Bool        InsertServer( SvLinkSource* rObj );
    void        RemoveServer( SvLinkSource* rObj );
    void        RemoveServer( sal_uInt16 nPos, sal_uInt16 nCnt = 1 )
                {   aServerTbl.Remove( nPos, nCnt ); }

    // a transfer will be discontinued, therefore cancel all DownloadMedia
    // (at the moment only interesting for the FileLinks!)

    void CancelTransfers();

    // to send status information from the FileObject to the BaseLink, there is an own ClipboardId.
    // The SvData object has then the respective information as string.
    // Currently this will be used for FileObject in connection with JavaScript
    // - that needs information about Load/Abort/Error

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

// separator in the LinkName for the DDE/File/Graphic links
// (only who needs it, in order to assemble a SvLinkName)
const sal_Unicode cTokenSeperator = 0xFFFF;

// erzeuge einen String fuer den SvLinkName. Fuer
// - DDE die ersten 3 Strings, (Server, Topic, Item)
// - File-/Grf-LinkNms die letzen 3 Strings (FileName, Bereich, Filter)
// create a string for the SvLinkName. For:
// - DDE the first 3 strings, (Server, Topic, Item)
// - File/Graphic LinkNames the last 3 strings (FileName, Area, Filter)
SFX2_DLLPUBLIC void MakeLnkName( String& rName,
                 const String* pType,       // can also be 0!
                 const String& rFile,
                 const String& rLink,
                 const String* pFilter = 0 );

}

#endif

