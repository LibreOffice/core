/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srcview.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:10:01 $
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
#ifndef _SRCVIEW_HXX
#define _SRCIEW_HXX

#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#include "srcedtw.hxx"
#include "shellid.hxx"

class SwDocShell;
class SvxSearchItem;
class SfxMedium;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SwSrcView: public SfxViewShell
{
    SwSrcEditWindow     aEditWin;

    SvxSearchItem*      pSearchItem;

    BOOL                bSourceSaved    :1;
    rtl_TextEncoding    eLoadEncoding;
    void                Init();

    // fuer Readonly-Umschaltung
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );


protected:
    USHORT              StartSearchAndReplace(const SvxSearchItem& rItem,
                                                        BOOL bFromStart,
                                                        BOOL bApi,
                                                        BOOL bRecursive = FALSE);


public:

    SFX_DECL_VIEWFACTORY(SwSrcView);
    SFX_DECL_INTERFACE(SW_SRC_VIEWSHELL)
    TYPEINFO();


    SwSrcView(SfxViewFrame* pFrame, SfxViewShell*);

    ~SwSrcView();

    SwDocShell*         GetDocShell();
    SwSrcEditWindow&    GetEditWin() {return aEditWin;}
    void                SaveContent(const String& rTmpFile);
    void                SaveContentTo(SfxMedium& rMed);

    BOOL                IsModified() {return aEditWin.IsModified();}



    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);

    SvxSearchItem*  GetSearchItem();
    void            SetSearchItem( const SvxSearchItem& rItem );

    void            Load(SwDocShell* pDocShell);

    virtual USHORT  SetPrinter( SfxPrinter* pNew,
                                        USHORT nDiff = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual ErrCode DoPrint( SfxPrinter *pPrinter, PrintDialog *pPrintDialog, BOOL bSilent, BOOL bIsAPI );
    virtual         SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );

    void            SourceSaved() {bSourceSaved = TRUE;}
    BOOL            HasSourceSaved() const {return bSourceSaved;}

};


#endif





















