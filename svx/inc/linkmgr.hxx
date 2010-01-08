/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: linkmgr.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _SVXLINKMGR_HXX
#define _SVXLINKMGR_HXX


#include <sfx2/linkmgr.hxx>
#include "svx/svxdllapi.h"

class Graphic;
class Size;

// Damit der Link ueber den Status der zu ladenen Grafik informierten werden
// verschickt das FileObject ein SvData, mit der FormatId
// "RegisterStatusInfoId" und ein einem String als Datentraeger. Dieser
// enthaelt den folgenden enum.
enum LinkState
{
    STATE_LOAD_OK,
    STATE_LOAD_ERROR,
    STATE_LOAD_ABORT
};

class SVX_DLLPUBLIC SvxLinkManager : public ::sfx2::SvLinkManager
{
    SvxLinkManager( const SvxLinkManager& );
    SvxLinkManager& operator=( const SvxLinkManager& );

public:
    SvxLinkManager( SfxObjectShell * pCacheCont );

    // den Link mit einem PseudoObject verbinden und in die Liste eintragen
    BOOL InsertFileLink( sfx2::SvBaseLink&,
                        USHORT nFileType,
                        const String& rTxt,
                        const String* pFilterNm = 0,
                        const String* pRange = 0 );

            // falls am Link schon alles eingestellt ist !
    BOOL InsertFileLink( sfx2::SvBaseLink& );

        // erfrage die Strings fuer den Dialog
    virtual BOOL GetDisplayNames( const sfx2::SvBaseLink*,
                                    String* pType,
                                    String* pFile = 0,
                                    String* pLink = 0,
                                    String* pFilter = 0 ) const;

    virtual sfx2::SvLinkSourceRef CreateObj( sfx2::SvBaseLink * );

    // eine Uebertragung wird abgebrochen, also alle DownloadMedien canceln
    // (ist zur Zeit nur fuer die FileLinks interressant!)
    void CancelTransfers();

    // um Status Informationen aus dem FileObject an den BaseLink zu
    // senden, gibt es eine eigene ClipBoardId. Das SvData-Object hat
    // dann die entsprechenden Informationen als String.
    // Wird zur Zeit fuer FileObject in Verbindung mit JavaScript benoetigt
    // - das braucht Informationen ueber Load/Abort/Error
    static ULONG  RegisterStatusInfoId();

    // if the mimetype says graphic/bitmap/gdimetafile then get the
    // graphic from the Any. Return says no errors
    static BOOL GetGraphicFromAny( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf );

};


#endif

