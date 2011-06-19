/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _LINKMGR_HXX
#define _LINKMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sfx2/linksrc.hxx>
#include <tools/string.hxx>
#include <svl/svarray.hxx>

#include <vector>

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

typedef SvBaseLinkRef* SvBaseLinkRefPtr;
SV_DECL_PTRARR( SvBaseLinks, SvBaseLinkRefPtr, 1, 1 )

typedef SvLinkSource* SvLinkSourcePtr;
SV_DECL_PTRARR( SvLinkSources, SvLinkSourcePtr, 1, 1 )

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
    void        Remove( sal_uInt16 nPos, sal_uInt16 nCnt = 1 );
    sal_Bool        Insert( SvBaseLink* pLink );

    // the links connect to a SvLinkSource and adds to the list
    sal_Bool        InsertDDELink( SvBaseLink*,
                        const String& rServer,
                        const String& rTopic,
                        const String& rItem );

    // if everything is already set at the link!
    sal_Bool        InsertDDELink( SvBaseLink* );

    // Connect the links to a pseudo-object and add to the list
    sal_Bool InsertFileLink( sfx2::SvBaseLink&,
                        sal_uInt16 nFileType,
                        const String& rTxt,
                        const String* pFilterNm = 0,
                        const String* pRange = 0 );

    // if everything is already set for the link!
    sal_Bool InsertFileLink( sfx2::SvBaseLink& );

    void ReconnectDdeLink(SfxObjectShell& rServer);

    /**
     * Reconnect the server document shell to a DDE link object.
     *
     * @param rPath path to the server document
     * @param rServer server document shell instance
     * @param rLink link object of the client document
     */
    void LinkServerShell(const ::rtl::OUString& rPath, SfxObjectShell& rServer, ::sfx2::SvBaseLink& rLink) const;

    // Obtain the string for the dialog
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

    // Call for list of links (eg for link-dialog)
    const       SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Server-side management --------------------

    // Call with list of links to server
    const SvLinkSources& GetServers() const { return aServerTbl; }
    // Link register/delete
    sal_Bool        InsertServer( SvLinkSource* rObj );
    void        RemoveServer( SvLinkSource* rObj );
    void        RemoveServer( sal_uInt16 nPos, sal_uInt16 nCnt = 1 )
                {   aServerTbl.Remove( nPos, nCnt ); }

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
    static sal_Bool GetGraphicFromAny( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf );

private:
                LinkManager( const LinkManager& );
                LinkManager& operator=( const LinkManager& );
};

// Separator in the link name for the DDE-/File-/Graphics- links
// (only those who need to construct a SvLinkName)
const sal_Unicode cTokenSeperator = 0xFFFF;

// create a string for the SvLinkName. For:
// - DDE the first 3 Strings, (Server, Topic, Item)
// - File-/Graphics-LinkNames the last 3 Strings (FileName, Region, Filter)
SFX2_DLLPUBLIC void MakeLnkName( String& rName,
                 const String* pType,       // Can also be null!!
                 const String& rFile,
                 const String& rLink,
                 const String* pFilter = 0 );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
