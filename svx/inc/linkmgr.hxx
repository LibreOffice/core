/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linkmgr.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:01:27 $
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
#ifndef _SVXLINKMGR_HXX
#define _SVXLINKMGR_HXX


#ifndef _LINKMGR_HXX
#include <sfx2/linkmgr.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

    static void SetTransferPriority( sfx2::SvBaseLink& rLink, USHORT nPrio );

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

