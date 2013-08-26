/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _LINKMGR_HXX
#define _LINKMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sfx2/linksrc.hxx>
#include <tools/string.hxx>

#include <vector>
#include <set>

class SfxObjectShell;
class Graphic;
class Size;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XComponent;
    }
}}}

namespace sfx2
{
    // For the link to receive information about the status of graphics that
    // will be loaded the FileObject sends a SvData, which contains the
    // FormatID "RegisterStatusInfoId" and a string as the data container.
    // This contains the following enum.

class SvBaseLink;
class SvBaseLinkRef;

typedef std::vector<SvBaseLinkRef*> SvBaseLinks;

typedef std::set<SvLinkSource*> SvLinkSources;

class SFX2_DLLPUBLIC LinkManager
{
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > >
        CompVector;
    CompVector maCachedComps;

    SvBaseLinks     aLinkTbl;
    SvLinkSources aServerTbl;

    SfxObjectShell *pPersist; // LinkMgr must be release before SfxObjectShell
protected:
    sal_Bool        InsertLink( SvBaseLink* pLink, sal_uInt16 nObjType, sal_uInt16 nUpdateType,
                            const OUString* pName = 0 );
public:

    enum LinkState
    {
        STATE_LOAD_OK,
        STATE_LOAD_ERROR,
        STATE_LOAD_ABORT
    };

                LinkManager( SfxObjectShell * pCacheCont );
                ~LinkManager();

    /**
     * Insert a component loaded during link update, which needs to be closed
     * when the update is complete.
     *
     * @param xComp component loaded during link update.
     */
    void        InsertCachedComp(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xComp);

    void        CloseCachedComps();

    SfxObjectShell*    GetPersist() const              { return pPersist; }
    void        SetPersist( SfxObjectShell * p )   { pPersist = p; }

    void        Remove( SvBaseLink *pLink );
    void        Remove( size_t nPos, size_t nCnt = 1 );
    sal_Bool        Insert( SvBaseLink* pLink );

    // the links connect to a SvLinkSource and adds to the list
    sal_Bool        InsertDDELink( SvBaseLink*,
                        const OUString& rServer,
                        const OUString& rTopic,
                        const OUString& rItem );

    // if everything is already set at the link!
    sal_Bool        InsertDDELink( SvBaseLink* );

    // Connect the links to a pseudo-object and add to the list
    bool InsertFileLink( sfx2::SvBaseLink&,
                        sal_uInt16 nFileType,
                        const OUString& rFileNm,
                        const OUString* pFilterNm = NULL,
                        const OUString* pRange = NULL );

    void ReconnectDdeLink(SfxObjectShell& rServer);

    /**
     * Reconnect the server document shell to a DDE link object.
     *
     * @param rPath path to the server document
     * @param rServer server document shell instance
     * @param rLink link object of the client document
     */
    void LinkServerShell(const OUString& rPath, SfxObjectShell& rServer, ::sfx2::SvBaseLink& rLink) const;

    // Obtain the string for the dialog
    bool GetDisplayNames( const SvBaseLink *,
                                    OUString* pType,
                                    OUString* pFile = NULL,
                                    OUString* pLink = NULL,
                                    OUString* pFilter = NULL ) const;

    SvLinkSourceRef CreateObj( SvBaseLink* );

    void        UpdateAllLinks( bool bAskUpdate = true,
                                bool bCallErrHdl = true,
                                bool bUpdateGrfLinks = false,
                                Window* pParentWin = 0 );

    // Call for list of links (eg for link-dialog)
    const       SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Server-side management --------------------

    // Call with list of links to server
    const SvLinkSources& GetServers() const { return aServerTbl; }
    // Link register/delete
    sal_Bool        InsertServer( SvLinkSource* rObj );
    void            RemoveServer( SvLinkSource* rObj );

    // A transfer is aborted, so cancel all download media
    // (for the time beeing this is only of interest for the FileLinks!)
    void CancelTransfers();

    // To send status information from the FileObject to the Baselink,
    // for this there exist a separate ClipBoardId. The SvData-object has
    // got the appropriate information as a string.
    // Is now required for FileObject in conjunction with JavaScript
    // this needs information about Load/Abort/Error
    static sal_uIntPtr RegisterStatusInfoId();

    // if the mimetype says graphic/bitmap/gdimetafile then get the
    // graphic from the Any. Return says no errors
    static sal_Bool GetGraphicFromAny( const OUString& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf );

private:
                LinkManager( const LinkManager& );
                LinkManager& operator=( const LinkManager& );
};

// Separator in the link name for the DDE-/File-/Graphics- links
// (only those who need to construct a SvLinkName)
const sal_Unicode cTokenSeparator = 0xFFFF;

// create a string for the SvLinkName. For:
// - DDE the first 3 Strings, (Server, Topic, Item)
// - File-/Graphics-LinkNames the last 3 Strings (FileName, Region, Filter)
SFX2_DLLPUBLIC void MakeLnkName( OUString& rName,
                 const OUString* pType,       // Can also be null!!
                 const OUString& rFile,
                 const OUString& rLink,
                 const OUString* pFilter = 0 );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
