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
#ifndef INCLUDED_SFX2_LINKMGR_HXX
#define INCLUDED_SFX2_LINKMGR_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/linksrc.hxx>
#include <sfx2/lnkbase.hxx>
#include <set>
#include <vector>

class SfxObjectShell;
class Graphic;
class Size;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XComponent;
    }
}}}

namespace weld { class Window; }

namespace sfx2
{
    // For the link to receive information about the status of graphics that
    // will be loaded the FileObject sends a SvData, which contains the
    // FormatID "RegisterStatusInfoId" and a string as the data container.
    // This contains the following enum.

typedef std::vector<tools::SvRef<SvBaseLink> > SvBaseLinks;

typedef std::set<SvLinkSource*> SvLinkSources;

class SFX2_DLLPUBLIC LinkManager
{
    typedef ::std::vector< css::uno::Reference< css::lang::XComponent > >
        CompVector;
    CompVector maCachedComps;

    SvBaseLinks     aLinkTbl;
    SvLinkSources aServerTbl;

    SfxObjectShell *pPersist; // LinkMgr must be release before SfxObjectShell
protected:
    bool        InsertLink( SvBaseLink* pLink, sal_uInt16 nObjType, SfxLinkUpdateMode nUpdateType,
                            const OUString* pName );
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
    void        InsertCachedComp(const css::uno::Reference< css::lang::XComponent >& xComp);

    void        CloseCachedComps();

    SfxObjectShell*    GetPersist() const              { return pPersist; }
    void        SetPersist( SfxObjectShell * p )   { pPersist = p; }

    void        Remove( SvBaseLink const *pLink );
    void        Remove( size_t nPos, size_t nCnt = 1 );
    bool        Insert( SvBaseLink* pLink );

    // the links connect to a SvLinkSource and adds to the list
    void        InsertDDELink( SvBaseLink*,
                        const OUString& rServer,
                        const OUString& rTopic,
                        const OUString& rItem );

    // if everything is already set at the link!
    void        InsertDDELink( SvBaseLink* );

    // Connect the links to a pseudo-object and add to the list
    void InsertFileLink( sfx2::SvBaseLink&,
                        sal_uInt16 nFileType,
                        const OUString& rFileNm,
                        const OUString* pFilterNm = nullptr,
                        const OUString* pRange = nullptr );

    void ReconnectDdeLink(SfxObjectShell& rServer);

    /**
     * Reconnect the server document shell to a DDE link object.
     *
     * @param rPath path to the server document
     * @param rServer server document shell instance
     * @param rLink link object of the client document
     */
    static void LinkServerShell(const OUString& rPath, SfxObjectShell& rServer, ::sfx2::SvBaseLink& rLink);

    // Obtain the string for the dialog
    static bool GetDisplayNames( const SvBaseLink *,
                                    OUString* pType,
                                    OUString* pFile = nullptr,
                                    OUString* pLink = nullptr,
                                    OUString* pFilter = nullptr );

    static SvLinkSourceRef CreateObj( SvBaseLink const * );

    void        UpdateAllLinks(bool bAskUpdate,
                               bool bUpdateGrfLinks,
                               weld::Window* pParentWin);

    // Call for list of links (eg for link-dialog)
    const       SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Server-side management --------------------

    // Call with list of links to server
    const SvLinkSources& GetServers() const { return aServerTbl; }
    // Link register/delete
    bool            InsertServer( SvLinkSource* rObj );
    void            RemoveServer( SvLinkSource* rObj );

    // A transfer is aborted, so cancel all download media
    // (for the time being this is only of interest for the FileLinks!)
    void CancelTransfers();

    // To send status information from the FileObject to the Baselink,
    // for this there exist a separate ClipBoardId. The SvData-object has
    // got the appropriate information as a string.
    // Is now required for FileObject in conjunction with JavaScript
    // this needs information about Load/Abort/Error
    static SotClipboardFormatId RegisterStatusInfoId();

    // if the mimetype says graphic/bitmap/gdimetafile then get the
    // graphic from the Any. Return says no errors
    static bool GetGraphicFromAny(const OUString& rMimeType,
                                  const css::uno::Any & rValue,
                                  const OUString& rReferer,
                                  Graphic& rGrf);

private:
                LinkManager( const LinkManager& ) = delete;
                LinkManager& operator=( const LinkManager& ) = delete;
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
                 const OUString* pFilter = nullptr );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
