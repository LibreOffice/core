/*************************************************************************
 *
 *  $RCSfile: sdmod.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SVX_PSZCTRL_HXX //autogen
#include <svx/pszctrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif

#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif

#define ITEMID_SEARCH           SID_SEARCH_ITEM
#include <svx/svxids.hrc>
#include <offmgr/ofaids.hrc>
#include <svx/srchitem.hxx>

#pragma hdrstop

#define _SD_DLL                 // fuer SD_MOD()
#include "sdmod.hxx"
#include "sddll.hxx"
#include "sdresid.hxx"
#include "docdlg.hxx"
#include "optsitem.hxx"
#include "docshell.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "cfgids.hxx"

TYPEINIT1( SdModuleDummy, SfxModule );
TYPEINIT1( SdModule, SdModuleDummy );

#define SdModule
#include "sdslots.hxx"


SFX_IMPL_INTERFACE(SdModule, SfxModule, SdResId(STR_APPLICATIONOBJECTBAR))
{
    SFX_STATUSBAR_REGISTRATION(RID_DRAW_STATUSBAR);
}

SFX_IMPL_MODULE_DLL(Sd)

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

SdModule::SdModule(SvFactory* pDrawObjFact, SvFactory* pGraphicObjFact)
:   SdModuleDummy(SFX_APP()->CreateResManager("sd"), FALSE,
                  pDrawObjFact, pGraphicObjFact),
    bWaterCan(FALSE),
    pDragData(NULL),
    pClipboardData(NULL),
    pImpressOptions(NULL),
    pDrawOptions(NULL),
    pSearchItem(NULL)
{
    SetName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarDraw" ) ) );  // Nicht uebersetzen!
    pSearchItem = new SvxSearchItem(ITEMID_SEARCH);
    pSearchItem->SetAppFlag(SVX_SEARCHAPP_DRAW);
}



/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdModule::~SdModule()
{
    delete pImpressOptions;
    delete pDrawOptions;
    delete pSearchItem;
}


/*************************************************************************
|*
|* DocFileDialog
|*
\************************************************************************/

SfxFileDialog* SdModule::CreateDocFileDialog( ULONG nBits,
                                              const SfxObjectFactory& rFact,
                                              const SfxItemSet* pSet )
{
    SfxFileDialog*   pDlg;
    SfxItemSet aSet( GetPool(),
                     SID_DOC_READONLY, SID_DOC_READONLY,
                     SID_PASSWORD, SID_PASSWORD,
                     SID_LOAD_LAYOUT, SID_LOAD_LAYOUT,
                     SID_FILE_FILTEROPTIONS, SID_FILE_FILTEROPTIONS,
                     SID_FILE_NAME, SID_FILE_NAME, 0 );

    if( pSet )
        aSet.Put( *pSet );

    if ( WB_SAVEAS == ( nBits & WB_SAVEAS ) )
        nBits |= SFXWB_PASSWORD;
    else
    {
        if ( SFXWB_INSERT != ( nBits & SFXWB_INSERT ) )
            nBits |= SFXWB_READONLY;
    }

    // bei Open und SaveAs einen eigenen Dialog mit Filter-Button
    // verwenden;
    // bei SaveAs eigenen Dialog mit Checkbox verwenden
    if ( WB_SAVEAS == ( nBits & WB_SAVEAS ) )
        pDlg = new SdSaveAsDialog( NULL, nBits, rFact, aSet );
    else
        pDlg = new SdOpenDialog( NULL, nBits, rFact, aSet );


    return pDlg;
}



/*************************************************************************
|*
|* Statusbar erzeugen
|*
\************************************************************************/

#define AUTOSIZE_WIDTH  180
#define TEXT_WIDTH(s)   rStatusBar.GetTextWidth((s))

void SdModule::FillStatusBar(StatusBar& rStatusBar)
{
    // Hinweis
    rStatusBar.InsertItem( SID_CONTEXT, TEXT_WIDTH( String().Fill( 30, 'x' ) ), // vorher 52
                            SIB_IN | SIB_LEFT | SIB_AUTOSIZE );

    // Groesse und Position
    rStatusBar.InsertItem( SID_ATTR_SIZE, SvxPosSizeStatusBarControl::GetDefItemWidth(rStatusBar), // vorher 42
                            SIB_IN | SIB_USERDRAW );
                            // SIB_AUTOSIZE | SIB_LEFT | SIB_OWNERDRAW );

    // Massstab
    rStatusBar.InsertItem( SID_ATTR_ZOOM, SvxZoomStatusBarControl::GetDefItemWidth(rStatusBar), SIB_IN | SIB_CENTER );
/*
    // Einfuege- / Uberschreibmodus
    rStatusBar.InsertItem( SID_ATTR_INSERT, TEXT_WIDTH( "EINFG" ),
                            SIB_IN | SIB_CENTER );

    // Selektionsmodus
    rStatusBar.InsertItem( SID_STATUS_SELMODE, TEXT_WIDTH( "ERG" ),
                            SIB_IN | SIB_CENTER );
*/
    // Dokument geaendert
    rStatusBar.InsertItem( SID_DOC_MODIFIED, SvxModifyControl::GetDefItemWidth(rStatusBar) );


    // Seite
    rStatusBar.InsertItem( SID_STATUS_PAGE, TEXT_WIDTH( String().Fill( 12, 'X' ) ),
                            SIB_IN | SIB_LEFT );

    // Praesentationslayout
    rStatusBar.InsertItem( SID_STATUS_LAYOUT, TEXT_WIDTH( String().Fill( 10, 'X' ) ),
                            SIB_IN | SIB_LEFT | SIB_AUTOSIZE );
}



/*************************************************************************
|*
|* Modul laden (nur Attrappe fuer das Linken der DLL)
|*
\************************************************************************/

SfxModule* SdModuleDummy::Load()
{
    return (NULL);
}



/*************************************************************************
|*
|* Modul laden
|*
\************************************************************************/

SfxModule* SdModule::Load()
{
    return (this);
}



/*************************************************************************
|*
|* Modul freigeben
|*
\************************************************************************/

void SdModule::Free()
{
}

/*************************************************************************
|*
|* Optionen zurueckgeben
|*
\************************************************************************/

SdOptions* SdModule::GetSdOptions(DocumentType eDocType)
{
    SdOptions* pOptions = NULL;

    if (eDocType == DOCUMENT_TYPE_DRAW)
    {
        if (!pDrawOptions)
        {
            pDrawOptions = new SdOptions(SDCFG_DRAW);
            pDrawOptions->SetDefault(FALSE);
            pDrawOptions->Initialize();
        }

        pOptions = pDrawOptions;
    }
    else if (eDocType == DOCUMENT_TYPE_IMPRESS)
    {
        if (!pImpressOptions)
        {
            pImpressOptions = new SdOptions(SDCFG_IMPRESS);
            pImpressOptions->SetDefault(FALSE);
            pImpressOptions->Initialize();
        }

        pOptions = pImpressOptions;
    }
    if( pOptions )
    {
        UINT16 nMetric = pOptions->GetMetric();

        SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
        SdDrawDocument* pDoc = NULL;
        if (pDocSh)
            pDoc = pDocSh->GetDoc();

        if( nMetric != 0xffff && pDoc && eDocType == pDoc->GetDocumentType() )
            PutItem( SfxUInt16Item( SID_ATTR_METRIC, nMetric ) );
    }

    return(pOptions);
}

/*************************************************************************
|*
|* Optionen-Stream fuer interne Options oeffnen und zurueckgeben;
|* falls der Stream zum Lesen geoeffnet wird, aber noch nicht
|* angelegt wurde, wird ein 'leeres' RefObject zurueckgegeben
|*
\************************************************************************/

SvStorageStreamRef SdModule::GetOptionStream( const String& rOptionName,
                                              SdOptionStreamMode eMode )
{
    SdDrawDocShell*     pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
    SvStorageStreamRef  xStm;

    if( pDocSh )
    {
        DocumentType    eType = pDocSh->GetDoc()->GetDocumentType();
        String          aStmName;

        if( !xOptionStorage.Is() )
        {
            INetURLObject aURL;

            aURL.SetSmartURL( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_USERCONFIG_PATH ) );
            aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "drawing.cfg" ) ) );
            xOptionStorage = new SvStorage( aURL.PathToFileName() );
        }

        if( DOCUMENT_TYPE_DRAW == eType )
            aStmName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Draw_" ) );
        else
            aStmName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Impress_" ) );

        aStmName += rOptionName;

        if( SD_OPTION_STORE == eMode || xOptionStorage->IsContained( aStmName ) )
            xStm = xOptionStorage->OpenStream( aStmName );
    }

    return xStm;
}



