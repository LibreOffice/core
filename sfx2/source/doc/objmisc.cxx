/*************************************************************************
 *
 *  $RCSfile: objmisc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-09-28 16:29:22 $
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

#ifndef _INETMSG_HXX //autogen
#include <svtools/inetmsg.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#include <so3/inetbnd.hxx>
#include <vos/mutex.hxx>

#pragma hdrstop

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHENGINEEVENT_HPP_
#include <com/sun/star/script/FinishEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTREASON_HPP_
#include <com/sun/star/script/InterruptReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINELISTENER_HPP_
#include <com/sun/star/script/XEngineListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XDEBUGGING_HPP_
#include <com/sun/star/script/XDebugging.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOKATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_CONTEXTINFORMATION_HPP_
#include <com/sun/star/script/ContextInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHREASON_HPP_
#include <com/sun/star/script/FinishReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINE_HPP_
#include <com/sun/star/script/XEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTENGINEEVENT_HPP_
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYACCESS_HPP_
#include <com/sun/star/script/XLibraryAccess.hpp>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/ucb/XContent.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

#ifndef _SB_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _VCL_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include "appdata.hxx"
#include "request.hxx"
#include "bindings.hxx"
#include "sfxresid.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "docfilt.hxx"
#include "interno.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "event.hxx"
#include "fltfnc.hxx"
#include "sfx.hrc"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "ctrlitem.hxx"
#include "arrdecl.hxx"
#include "module.hxx"
#include "macrconf.hxx"
#include "docfac.hxx"
#include "inimgr.hxx"
#include "ucbhelp.hxx"
#include "helper.hxx"

// class SfxHeaderAttributes_Impl ----------------------------------------

class SfxHeaderAttributes_Impl : public SvKeyValueIterator
{
private:
    SfxObjectShell* pDoc;
    SvKeyValueIteratorRef xIter;
    sal_Bool bAlert;

public:
    SfxHeaderAttributes_Impl( SfxObjectShell* pSh ) :
        pDoc( pSh ), SvKeyValueIterator(),
        xIter( pSh->GetMedium()->GetHeaderAttributes_Impl() ),
        bAlert( sal_False ) {}

    virtual sal_Bool GetFirst( SvKeyValue& rKV ) { return xIter->GetFirst( rKV ); }
    virtual sal_Bool GetNext( SvKeyValue& rKV ) { return xIter->GetNext( rKV ); }
    virtual void Append( const SvKeyValue& rKV );

    void ClearForSourceView() { xIter = new SvKeyValueIterator; bAlert = sal_False; }
    void SetAttributes();
    void SetAttribute( const SvKeyValue& rKV );
};

//=========================================================================

sal_uInt16 __READONLY_DATA aTitleMap_Impl[3][2] =
{
                                //  local               remote
    /*  SFX_TITLE_CAPTION   */  {   SFX_TITLE_FILENAME, SFX_TITLE_TITLE },
#ifdef MAC
    /*  SFX_TITLE_PICKLIST  */  {   SFX_TITLE_FILENAME, SFX_TITLE_FULLNAME },
#else
    /*  SFX_TITLE_PICKLIST  */  {   32,                 SFX_TITLE_FULLNAME },
#endif
    /*  SFX_TITLE_HISTORY   */  {   32,                 SFX_TITLE_FULLNAME }
};

//=========================================================================

void SfxObjectShell::AbortImport()
{
    pImp->bIsAbortingImport = sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsAbortingImport() const
{
    return pImp->bIsAbortingImport;
}

//-------------------------------------------------------------------------

#if SUPD<604
void SfxObjectShell::NotifyReloadAvailable()
{
}
#endif

//-------------------------------------------------------------------------

SfxDocumentInfo& SfxObjectShell::GetDocInfo()
{
    if( !pImp->pDocInfo )
    {
        pImp->pDocInfo = new SfxDocumentInfo;
        pImp->pDocInfo->SetReadOnly( IsReadOnly() );
    }

    return *pImp->pDocInfo;
}

//-------------------------------------------------------------------------

void SfxObjectShell::FlushDocInfo()
{
    SetModified(sal_True);
    SfxDocumentInfo &rInfo = GetDocInfo();
    Broadcast( SfxDocumentInfoHint( &rInfo ) );
    SetAutoLoad( rInfo.GetReloadURL(), rInfo.GetReloadDelay() * 1000,
                 rInfo.IsReloadEnabled() );

    // bitte beachten:
    // 1. Titel in DocInfo aber nicht am Doc (nach HTML-Import)
    //  => auch am Doc setzen
    // 2. Titel in DocInfo leer (Briefumschlagsdruck)
    //  => nicht am Doc setzen, da sonst "unbenanntX" daraus wird
    String aDocInfoTitle = GetDocInfo().GetTitle();
    if ( aDocInfoTitle.Len() )
        SetTitle( aDocInfoTitle );
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetError(sal_uInt32 lErr)
{
    if(pImp->lErr==ERRCODE_NONE)
        pImp->lErr=lErr;
}

//-------------------------------------------------------------------------

sal_uInt32 SfxObjectShell::GetError() const
{
    return ERRCODE_TOERROR(GetErrorCode());
}

//-------------------------------------------------------------------------

sal_uInt32 SfxObjectShell::GetErrorCode() const
{
    sal_uInt32 lError=pImp->lErr;
    if(!lError && GetMedium())
        lError=GetMedium()->GetErrorCode();
    if(!lError && HasStorage())
        lError= GetStorage()->GetErrorCode();
    return lError;
}

//-------------------------------------------------------------------------

void SfxObjectShell::ResetError()
{
    pImp->lErr=0;
    SfxMedium * pMed = GetMedium();
    if( pMed )
        pMed->ResetError();
    SvStorage *pStor= HasStorage() ? GetStorage() : 0;
    if( pStor )
        pStor->ResetError();
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsTemplate() const
{
    return pImp->bIsTemplate;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetTemplate(sal_Bool bIs)
{
    pImp->bIsTemplate=bIs;
    SfxFilterMatcher aMatcher( GetFactory().GetFilterContainer() );
    SfxFilterMatcherIter aIter( &aMatcher, SFX_FILTER_TEMPLATEPATH );
    SfxMedium* pMed = GetMedium();
    if( pMed ) pMed->SetFilter( aIter.First() );
}

//-------------------------------------------------------------------------

void SfxObjectShell::ModifyChanged()

/*  [Beschreibung]

    Diese virtuelle Methode wird aus der virtuellen Basisklasse SvPersist
    gerufen, wenn sich das Modified-Flag ge"andert hat. Diese Querverbindung
    ist notwendig, da aus einem Zweig einer virtuellen Vererbung nicht
    quer un den anderen gerufen werden kann.
*/

{
    if ( pImp->bClosing )
        // SetModified aus dem dispose des Models!
        return;

    {DBG_CHKTHIS(SfxObjectShell, 0);}
    SfxObjectShell *pDoc;
    for ( pDoc = SfxObjectShell::GetFirst(); pDoc;
          pDoc = SfxObjectShell::GetNext(*pDoc) )
        if( pDoc->IsModified() )
            break;
    SfxApplication *pSfxApp = SFX_APP();
    Timer *pTimer = pSfxApp->GetAutoSaveTimer_Impl();
    if( pDoc )
    {
        if( !pTimer->IsActive() )
            pTimer->Start();
    }
    else
        pTimer->Stop();

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
        pViewFrame->GetBindings().Invalidate( SID_SAVEDOCS );

    pSfxApp->NotifyEvent( SfxEventHint( SFX_EVENT_MODIFYCHANGED, this ) );
}

//--------------------------------------------------------------------

SfxInPlaceObject* SfxObjectShell::GetInPlaceObject() const
{
    if( !pImp->pInPlaceObj && !pImp->bSetInPlaceObj )
    {
        // try to cast
        SvInPlaceObjectRef xSvIP( (SfxObjectShell *)this );
        if ( xSvIP.Is() )
            pImp->pInPlaceObj = (SfxInPlaceObject*) &xSvIP;
        pImp->bSetInPlaceObj = sal_True;
    }
    return pImp->pInPlaceObj;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnlyUI() const

/*  [Beschreibung]

    Liefert sal_True, wenn das Dokument fuer die UI wie r/o behandelt werden
    soll. Dieses ist unabhaengig vom tatsaechlichen r/o, welches per
    <IsReadOnly()> erfragbar ist.
*/

{
    return pImp->bReadOnlyUI;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnlyMedium() const

/*  [Beschreibung]

    Liefert sal_True, wenn das Medium r/o ist bzw. r/o geoeffnet wurde.
*/

{
    if ( !pMedium )
        return sal_True;
    return pMedium->IsReadOnly();
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetReadOnlyUI( sal_Bool bReadOnly )

/*  [Beschreibung]

    Schaltet das Dokument in einen r/o bzw. r/w Zustand ohne es neu
    zu laden und ohne die Open-Modi des Mediums zu aendern.
*/

{
    sal_Bool bWasRO = IsReadOnly();
    pImp->bReadOnlyUI = bReadOnly;
    if ( bWasRO != IsReadOnly() )
    {
        Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
        if ( pImp->pDocInfo )
            pImp->pDocInfo->SetReadOnly( IsReadOnly() );
    }
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnly() const
{
    return pImp->bReadOnlyUI || IsReadOnlyMedium();
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInModalMode() const

/*  [Beschreibung]

    Diese Methode liefert sal_True, falls in einer ::com::sun::star::sdbcx::View auf dieses Dokument
    ein modaler Dialog exitiert, sonst sal_False.
*/

{
    return pImp->bModalMode;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInAPICall() const

/*  [Beschreibung]

    Diese Methode liefert sal_True, falls z.B. ein zur Zeit durchzufuehrendes
    ConvertFrom (oder andere Callbacks) von der API gerufen werden. Dann
    duefen keine Dialoge gestartet werden.
*/

{
    return pImp->bSilent;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetModalMode_Impl( sal_Bool bModal )
{
    // nur Broadcasten wenn modifiziert, sonst ggf. Endlosrekursion
    if ( !pImp->bModalMode != !bModal )
    {
        // zentral mitz"ahlen
        sal_uInt16 &rDocModalCount = SFX_APP()->Get_Impl()->nDocModalMode;
        if ( bModal )
            ++rDocModalCount;
        else
            --rDocModalCount;

        // umschalten
        pImp->bModalMode = bModal;
        Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ) );
    }
}

//--------------------------------------------------------------------

Size SfxObjectShell::GetFirstPageSize()
{
    Size aRet;
    SfxInPlaceObject *pIpObj = GetInPlaceObject();
    if ( pIpObj )
        aRet = pIpObj->GetVisArea(ASPECT_THUMBNAIL).GetSize();
    return aRet;
}


//--------------------------------------------------------------------

IndexBitSet& SfxObjectShell::GetNoSet_Impl()
{
    return pImp->aBitSet;
}

//--------------------------------------------------------------------
// changes the title of the document

void SfxObjectShell::SetTitle
(
    const String& rTitle        // der neue Titel des Dokuments
)

/*  [Beschreibung]

    Mit dieser Methode kann der Titel des Dokuments gesetzt werden.
    Dieser entspricht initial dem kompletten Dateinamen. Ein Setzen
    des Titels wirkt jedoch nicht zu"uck auf den Dateinamen; er wird
    jedoch in den Caption-Bars der MDI-Fenster angezeigt.
*/

{
    DBG_CHKTHIS(SfxObjectShell, 0);

    // nix zu tun?
    if ( ( HasName() && pImp->aTitle == rTitle ) ||
         ( !HasName() && GetTitle() == rTitle ) )
        return;

    SfxApplication *pSfxApp = SFX_APP();
#if 0
    // wird 'unbenannt#' als Titel gesetzt
    String aNoName(SfxResId(STR_NONAME));
    if ( rTitle.Match(aNoName) <= aNoName.Len() )
    {
        // er ist es selbst => ignorieren
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);
        pImp->bIsNamedVisible=0;
    }
#endif

    // ggf. die unbenannt-Nummer freigeben
    if ( pImp->bIsNamedVisible && USHRT_MAX != pImp->nVisualDocumentNumber )
    {
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);
        pImp->bIsNamedVisible = 0;
    }

    // Title setzen
    pImp->aTitle = rTitle;
//  Wieso denn in der DocInfo?
//  GetDocInfo().SetTitle( rTitle );
//  FlushDocInfo();

    // Benachrichtigungen
    SfxShell::SetName( GetTitle(SFX_TITLE_APINAME) );
    Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
}

//--------------------------------------------------------------------

#ifdef DEBUG
String X(const String &rRet)
{
    if ( !rRet.Len() )
        return DEFINE_CONST_UNICODE( "-empty-" );
    return rRet;
}
#else
#define X(ret) ret
#endif

//--------------------------------------------------------------------
String SfxObjectShell::GetTitle
(
    sal_uInt16  nMaxLength      /*  0 (default)
                                der Titel selbst, so wie er ist

                                1 (==SFX_TITLE_FILENAME)
                                liefert den logischen Dateinamen ohne Pfad
                                (unter WNT je nach Systemeinstellung ohne
                                Extension)

                                2 (==SFX_TITLE_FULLNAME)
                                liefert den mit komplettem logischen Dateinamen
                                mit Pfad (remote => ::com::sun::star::util::URL)

                                3 (==SFX_TITLE_APINAME)
                                liefert den logischen Dateinamen ohne Pfad
                                und Extension

                                4 (==SFX_TITLE_DETECT)
                                liefert den kompletten Titel, falls noch
                                nicht gesetzt wird aber aus DocInfo oder
                                dem Namen des Medium erzeugt

                                5 (==SFX_TITLE_CAPTION)
                                liefert den Titel so, wie MB ihn heute in
                                der CaptionBar anzeigen m"ochte

                                6 (==SFX_TITLE_PICKLIST)
                                liefert den Titel so, wie MB ihn heute in
                                der PickList anzeigen m"ochte

                                7 (==SFX_TITLE_HISTORY)
                                liefert den Titel so, wie MB ihn heute in
                                der History anzeigen m"ochte

                                10 bis USHRT_MAX
                                liefert maximal 'nMaxLength' Zeichen vom logischen
                                Dateinamen inkl. Pfad (remote => ::com::sun::star::util::URL)
                                */
) const

/*  [Beschreibung]

    Liefert den Titel bzw. logischen Dateinamen des Dokuments, je nach
    'nMaxLength'.

    Falls der Dateiname mit Pfad verwendet wird, wird die Namensk"urzung durch
    Ersetzung eines oder mehrerer Directory-Namen durch "..." durchgef"uhrt,
    URLs werden z.Zt. immer komplett geliefert.
*/

{
//    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
//        return String();

    // Titel erzeugen?
    if ( SFX_TITLE_DETECT == nMaxLength && !pImp->aTitle.Len() )
    {
        static sal_Bool bRecur = sal_False;
        if ( bRecur )
            return DEFINE_CONST_UNICODE( "-not available-" );
        bRecur = sal_True;

        // evtl. ist Titel aus DocInfo verwendbar
        SfxObjectShell *pThis = (SfxObjectShell*) this;
        String aTitle = pThis->GetDocInfo().GetTitle();
        aTitle.EraseLeadingChars();
        aTitle.EraseTrailingChars();
        if ( !aTitle.Len() )
            // sonst wie SFX_TITLE_FILENAME
            aTitle = GetTitle( SFX_TITLE_FILENAME );
        pThis->SetTitle( aTitle );
        bRecur = sal_False;
        return X(aTitle);
    }
    else if (SFX_TITLE_APINAME == nMaxLength )
        return X(GetAPIName());

    // Sonderfall Vorlagen:
    if( IsTemplate() && pImp->aTitle.Len() &&
         ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
        return X(pImp->aTitle);

    // Picklist/Caption wird gemappt
    SfxMedium *pMed = GetMedium();
    if ( pMed && ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
    {
        // Wenn ein spezieller Titel beim "Offnen mitgegebent wurde;
        // wichtig bei URLs, die INET_PROT_FILE verwenden, denn bei denen
        // wird der gesetzte Titel nicht beachtet.
        // (s.u., Auswertung von aTitleMap_Impl)
        SFX_ITEMSET_ARG( pMed->GetItemSet(), pNameItem, SfxStringItem, SID_DOCINFO_TITLE, sal_False );

        // Demn"achst nur noch SID_FILE_LONGNAME statt SID_DOCINFO_TITLE
        if ( !pNameItem )
            SFX_ITEMSET_ARG( pMed->GetItemSet(), pNameItem, SfxStringItem, SID_FILE_LONGNAME, sal_False );

        if ( pNameItem )
        {
            return X( pNameItem->GetValue() );
        }
    }

    if ( nMaxLength >= SFX_TITLE_CAPTION && nMaxLength <= SFX_TITLE_HISTORY )
    {
        sal_uInt16 nRemote;
        if( !pMed || pMed->GetURLObject().GetProtocol() == INET_PROT_FILE )
            nRemote = 0;
        else nRemote = 1;
        nMaxLength = aTitleMap_Impl[nMaxLength-SFX_TITLE_CAPTION][nRemote];
    }

    // noch unbenannt?
    DBG_ASSERT( !HasName() || pMed, "HasName() aber kein Medium?!?" );
    if ( !HasName() || !pMed )
    {
        // schon Titel gesezt?
        if ( pImp->aTitle.Len() )
            return X(pImp->aTitle);

        // mu\s es durchnumeriert werden?
        String aNoName( SfxResId( STR_NONAME ) );
        if ( pImp->bIsNamedVisible )
            // Nummer hintenanh"angen
            aNoName += String::CreateFromInt32( pImp->nVisualDocumentNumber );

        // Dokument hei\st vorerst 'unbenannt#'
        return X(aNoName);
    }


    // lokale Datei?
    const INetURLObject& aURL = pMed->GetURLObject();
    if ( aURL.GetProtocol() == INET_PROT_FILE )
    {
        String aName( aURL.HasMark() ? INetURLObject( aURL.GetURLNoMark() ).PathToFileName() : aURL.PathToFileName() );

//        if ( nMaxLength > SFX_TITLE_MAXLEN )
//            return X( DirEntry( aName ).GetFull( FSYS_STYLE_HOST, sal_False, nMaxLength ) );
      /*  else*/ if ( nMaxLength == SFX_TITLE_FULLNAME )
            return X( aName );

        if ( !pImp->aTitle.Len() )
        {
            INetURLObject aPath( aName, INET_PROT_FILE );
            if ( nMaxLength == SFX_TITLE_FILENAME )
                return X( aPath.GetName() );

            // sonst Titel aus Dateiname generieren
            pImp->aTitle = aPath.GetBase();
        }
    }
    else
    {
        // ::com::sun::star::util::URL-Versionen
        if ( nMaxLength >= SFX_TITLE_MAXLEN )
        {
            String aComplete( pMed->GetName() );
            if( aComplete.Len() > nMaxLength )
            {
                String aRet( DEFINE_CONST_UNICODE( "..." ) );
                aRet += aComplete.Copy( aComplete.Len() - nMaxLength + 3, nMaxLength - 3 );
                return X( aRet );
            }
            else
                return X( pMed->GetName() );
        }
        else if ( nMaxLength == SFX_TITLE_FILENAME )
        {
            String aName( aURL.GetLastName() );
            aName = INetURLObject::decode( aName, INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET );
            if( !aName.Len() )
                aName = aURL.GetURLNoPass();
            return X(aName);
        }
        else if ( nMaxLength == SFX_TITLE_FULLNAME )
            return X(aURL.GetMainURL());

        // ggf. Titel aus Dateiname generieren
        if ( !pImp->aTitle.Len() )
            pImp->aTitle = aURL.GetBase();
    }

    // ganzer Titel
    return X(pImp->aTitle);
}

//--------------------------------------------------------------------

void SfxObjectShell::InvalidateName()

/*  [Beschreibung]

    Ermittelt den Titel des Dokuments neu aus 'unbenannt', DocInfo-Titel
    bzw. Dateinamen. Wird nach Laden aus Template oder SaveAs ben"otigt.
*/

{
    // Title neu erzeugen
    pImp->aTitle.Erase();
//  pImp->nVisualDocumentNumber = USHRT_MAX;
    GetTitle( SFX_TITLE_DETECT );
    SetName( GetTitle( SFX_TITLE_APINAME ) );

    // Benachrichtigungen
    Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
}

//--------------------------------------------------------------------

void SfxObjectShell::SetNamedVisibility_Impl()
{
    if ( !pImp->bIsNamedVisible )
    {
        // Nummer verpassen
        pImp->bIsNamedVisible = sal_True;
        // ggf. neue Nummer verpassen
        if ( !HasName() && USHRT_MAX == pImp->nVisualDocumentNumber && !pImp->aTitle.Len() )
        {
            pImp->nVisualDocumentNumber = SFX_APP()->GetFreeIndex();
            Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
        }
    }

    SetName( GetTitle(SFX_TITLE_APINAME) );
}

//--------------------------------------------------------------------

void SfxObjectShell::MemoryError()
{
}

//--------------------------------------------------------------------

SfxProgress* SfxObjectShell::GetProgress() const
{
    return pImp->pProgress;
}

//--------------------------------------------------------------------

void SfxObjectShell::SetProgress_Impl
(
    SfxProgress *pProgress  /*  zu startender <SfxProgress> oder 0, falls
                                der Progress zur"uckgesetzt werden soll */
)

/*  [Beschreibung]

    Interne Methode zum setzen oder zur"ucksetzen des Progress-Modes
    f"ur diese SfxObjectShell.
*/

{
    DBG_ASSERT( ( !pImp->pProgress && pProgress ) ||
                ( pImp->pProgress && !pProgress ),
                "Progress activation/deacitivation mismatch" );
    pImp->pProgress = pProgress;
}

//--------------------------------------------------------------------

void SfxObjectShell::PostActivateEvent_Impl()
{
    SfxApplication* pSfxApp = SFX_APP();
    if ( !pSfxApp->IsDowning() && !IsLoading() )
    {
        if (pImp->nEventId)
        {
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem,
                             SfxStringItem, SID_DOC_SALVAGE, sal_False );
            sal_uInt16 nId = pImp->nEventId;
            pImp->nEventId = 0;
            if ( !pSalvageItem )
                pSfxApp->NotifyEvent(SfxEventHint( nId, this, pImp->bAddToHistory), sal_False);
        }

        if ( GetFrame() )
            pSfxApp->NotifyEvent(SfxEventHint(SFX_EVENT_ACTIVATEDOC, this), sal_False);
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::SetActivateEvent_Impl(sal_uInt16 nId, sal_Bool bAddToHistory)
{
    if ( GetFactory().GetFlags() & SFXOBJECTSHELL_HASOPENDOC )
    {
        pImp->nEventId = nId;
        pImp->bAddToHistory = bAddToHistory;
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::RegisterTransfer( SfxMedium& rMedium )
/*  [Beschreibung ]
    Alle Medien, die aufgesetzt werden, um Teile eines Dokumentes zu
    laden, muessen an der zugehoerigen SfxObjectShell angemeldet
    werden. So kann dokumentweise abgebrochen werden.  */
{
    rMedium.SetCancelManager_Impl( GetMedium()->GetCancelManager_Impl() );
    if( IsReloading() )
        rMedium.SetUsesCache( sal_False );
    rMedium.SetReferer( GetMedium()->GetName() );
}

//-------------------------------------------------------------------------

void SfxObjectShell::PrepareReload( )
/*  [Beschreibung ]
    Wird vor dem Reload gerufen und gibt die Moeglichkeit,
    etwaige Caches zu leeren. */
{
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReloading() const
/*  [Beschreibung ]
    Wird beim Aufsetzen neuer Downloads abgefragt, um gegebenenfalls
    am SfxMedium SetUsesCache( sal_False ) zu rufen. */

{
    return !GetMedium()->UsesCache();
}

//-------------------------------------------------------------------------

void SfxObjectShell::LockAutoLoad( sal_Bool bLock )

/*  Verhindert ein evtl. eintreffendes AutoLoad. Wird auch vor AutoLoad
    eines umgebenden FrameSet beruecksichtigt.
*/

{
    if ( bLock )
        ++pImp->nAutoLoadLocks;
    else
        --pImp->nAutoLoadLocks;
}

//-------------------------------------------------------------------------

// kann nach frame.cxx gemoved werden, wenn 358+36x-Stand gemerged sind

sal_Bool SfxFrame::IsAutoLoadLocked_Impl() const
{
    // sein einges Doc gelockt?
    const SfxObjectShell* pObjSh = GetCurrentDocument();
    if ( !pObjSh || !pObjSh->IsAutoLoadLocked() )
        return sal_False;

    // seine Childs gelockt?
    for ( sal_uInt16 n = GetChildFrameCount(); n--; )
        if ( !GetChildFrame(n)->IsAutoLoadLocked_Impl() )
            return sal_False;

    // sonst ist AutoLoad erlaubt
    return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsAutoLoadLocked() const

/*  Liefert, ob ein eintreffendes AutoLoad ausgefuehrt werden darf. Wird auch
    vor AutoLoad eines umgebenden FrameSet beruecksichtigt.
*/

{
    return !IsReadOnly() || pImp->nAutoLoadLocks > 0;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetAutoLoad(
    const INetURLObject& rUrl, sal_uInt32 nTime, sal_Bool bReload )
/*  [Beschreibung ]
    Hiermit wird automatisches Laden der Url rUrl nTime
    Millisekunden nach Aufruf von FinishedLoading angefordert. bReload
    bestimmt, ob das Dokument aus dem Cache geladen werden soll oder
    nicht.  */
{
    if ( pImp->pReloadTimer )
        DELETEZ(pImp->pReloadTimer);
    if ( bReload )
    {
        pImp->pReloadTimer = new AutoReloadTimer_Impl(
                                rUrl.GetMainURL(), nTime, bReload, this );
        pImp->pReloadTimer->Start();
    }
}

void SfxObjectShell::FinishedLoading( sal_uInt16 nFlags )
{
    sal_Bool bSetModifiedTRUE = sal_False;
    if( ( nFlags & SFX_LOADED_MAINDOCUMENT ) &&
        !(pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ))
    {
        ((SfxHeaderAttributes_Impl*)GetHeaderAttributes())->SetAttributes();
        pImp->bImportDone = sal_True;
        const SfxFilter* pFilter = GetMedium()->GetFilter();
        if( !IsAbortingImport() )
        {
//            if( pFilter && !pFilter->UsesStorage() && !(GetMedium()->GetOpenMode() & STREAM_WRITE ) )
//                GetMedium()->Close();
            PositionView_Impl();
        }
        // Salvage
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem,
                         SfxStringItem, SID_DOC_SALVAGE, sal_False );
        if ( pSalvageItem )
            bSetModifiedTRUE = sal_True;
    }

    if( ( nFlags & SFX_LOADED_IMAGES ) &&
        !(pImp->nLoadedFlags & SFX_LOADED_IMAGES ) )
    {
        SfxDocumentInfo& rInfo = GetDocInfo();
        SetAutoLoad( rInfo.GetReloadURL(), rInfo.GetReloadDelay() * 1000,
                     rInfo.IsReloadEnabled() );
        if( !bSetModifiedTRUE && IsEnableSetModified() )
            SetModified( sal_False );
        Invalidate( SID_SAVEASDOC );
        SfxFrame* pFrame = GetMedium()->GetLoadTargetFrame();
        if( pFrame ) pFrame->SetLoadCancelable_Impl( 0 );
    }

    pImp->nLoadedFlags |= nFlags;

    if( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT &&
        pImp->nLoadedFlags & SFX_LOADED_IMAGES )
        GetMedium()->SetUsesCache( sal_True );

    if ( bSetModifiedTRUE )
        SetModified( sal_True );
    if ( pImp->nEventId )
    {
        // Falls noch kein OnLoad ausgel"ost wurde, weil beim Erzeugen der ::com::sun::star::sdbcx::View der Frame nicht aktiv war,
        // mu\s das jetzt nachgeholt werden, indem der Frame benachrichtigt wird.
        Broadcast( SfxEventHint( SFX_EVENT_LOADFINISHED, this, sal_False ) );

        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
        if ( pHiddenItem && pHiddenItem->GetValue() )
        {
            sal_uInt16 nId = pImp->nEventId;
            pImp->nEventId = 0;
            SFX_APP()->NotifyEvent(SfxEventHint( nId, this, pImp->bAddToHistory), sal_False);
        }
    }
}

//-------------------------------------------------------------------------

void SfxObjectShell::PositionView_Impl()
{
    MarkData_Impl *pMark = Get_Impl()->pMarkData;
    if( pMark )
    {
        SfxViewShell* pSh = pMark->pFrame->GetViewShell();
        if( pMark->aUserData.Len() )
            pSh->ReadUserData( pMark->aUserData, sal_True );
        else if( pMark->aMark.Len() )
            pSh->JumpToMark( pMark->aMark );
        DELETEZ( Get_Impl()->pMarkData );
    }
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsLoading() const
/*  [Beschreibung ]
    Wurde bereits FinishedLoading aufgerufeb? */
{
    return !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT );
}

//-------------------------------------------------------------------------

void SfxObjectShell::CancelTransfers()
/*  [Beschreibung ]
    Hier koennen Transfers gecanceled werden, die nicht mit
    RegisterTransfer registiert wurden */
{
    GetMedium()->CancelTransfers();
    if( ( pImp->nLoadedFlags & SFX_LOADED_ALL ) != SFX_LOADED_ALL )
    {
        AbortImport();
        if( IsLoading() )
            FinishedLoading( SFX_LOADED_ALL );

/*
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
        while( pFrame )
        {
            pFrame->CancelTransfers();
            pFrame = SfxViewFrame::GetNext( *pFrame, this );
        }*/
    }
}

//-------------------------------------------------------------------------

AutoReloadTimer_Impl::AutoReloadTimer_Impl(
    const String& rURL, sal_uInt32 nTime, sal_Bool bReloadP, SfxObjectShell* pSh )
    : aUrl( rURL ), bReload( bReloadP ), pObjSh( pSh )
{
    SetTimeout( nTime );
}

//-------------------------------------------------------------------------

void AutoReloadTimer_Impl::Timeout()
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pObjSh );

    if ( pFrame )
    {
        // momentan nicht m"oglich/sinnvoll?
        if ( !pObjSh->CanReload_Impl() || pObjSh->IsAutoLoadLocked() || DragManager::GetDragManager() )
        {
            // erneuten Versuch erlauben
            Start();
            return;
        }

        SfxAllItemSet aSet( SFX_APP()->GetPool() );
        aSet.Put( SfxBoolItem( SID_AUTOLOAD, sal_True ) );
        if ( aUrl.Len() )
            aSet.Put(  SfxStringItem( SID_FILE_NAME, aUrl ) );
        SfxRequest aReq( SID_RELOAD, 0, aSet );
        pObjSh->Get_Impl()->pReloadTimer = 0;
        delete this;
        pFrame->ExecReload_Impl( aReq );
        return;
    }

    pObjSh->Get_Impl()->pReloadTimer = 0;
    delete this;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetActualSize( const Size &rSize )
{
    pImp->aViewSize = rSize;
}

//-------------------------------------------------------------------------

Size SfxObjectShell::GetActualSize() const
{
    return pImp->aViewSize;
}

sal_Bool SfxObjectShell::IsInFrame() const
{
    return pImp->bInFrame;
}

void SfxObjectShell::SetInFrame( sal_Bool bOn )
{
    pImp->bInFrame = bOn;
}

SfxModule* SfxObjectShell::GetModule() const
{
    return GetFactory().GetModule();
}

sal_Bool SfxObjectShell::IsBasic(
    const String & rCode, SbxObject * pVCtrl )
{
    if( !rCode.Len() ) return sal_False;
    if( !pImp->bIsBasicDefault )
        return sal_False;
    return SfxMacroConfig::IsBasic( pVCtrl, rCode, GetBasicManager() );
}

ErrCode SfxObjectShell::CallBasic( const String& rMacro,
    const String& rBasic, SbxObject* pVCtrl, SbxArray* pArgs,
    SbxValue* pRet )
{
    if ( !IsSecure() )
        return ERRCODE_IO_ACCESSDENIED;

    SfxApplication* pApp = SFX_APP();
    pApp->EnterBasicCall();
    BasicManager *pMgr = GetBasicManager();
    if( pApp->GetName() == rBasic )
        pMgr = pApp->GetBasicManager();
    ErrCode nRet = SfxMacroConfig::Call( pVCtrl, rMacro, pMgr, pArgs, pRet );
    pApp->LeaveBasicCall();
    return nRet;
}

ErrCode SfxObjectShell::Call( const String & rCode, sal_Bool bIsBasicReturn, SbxObject * pVCtrl )
{
    ErrCode nErr = ERRCODE_NONE;
    if ( bIsBasicReturn )
        CallBasic( rCode, String(), pVCtrl );
    return nErr;
}

extern ::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );

ErrCode SfxObjectShell::CallScript(
    const String & rScriptType,
    const String & rCode,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  & rSource,
    void *pArgs,
    void *pRet
)
{
    ::vos::OClearableGuard aGuard( Application::GetSolarMutex() );
    ErrCode nErr = ERRCODE_NONE;
    if( rScriptType.EqualsAscii( "StarBasic" ) )
    {

        SbxArrayRef xArray;
        String aTmp;
        if( pArgs )
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > * pTmpArgs = (::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > *)pArgs;
            sal_Int32 nCnt = pTmpArgs->getLength();

            if( nCnt )
            {
                xArray = new SbxArray;
                const ::com::sun::star::uno::Any *pArgs = pTmpArgs->getConstArray();
                for( sal_Int32 i = 0; i < nCnt; i++ )
                    xArray->Put( GetSbUnoObject( aTmp, pArgs[i] ), i+1 );
            }
        }

        SbxVariableRef xValue = pRet ? new SbxVariable : 0;
        nErr = CallBasic( rCode, aTmp, NULL, xArray, xValue );
        if ( pRet )
            *((::com::sun::star::uno::Any*)pRet) = sbxToUnoValue( xValue );

        if ( nErr == ERRCODE_BASIC_PROC_UNDEFINED )
        {
            aTmp = SFX_APP()->GetName();
            nErr = CallBasic( rCode, aTmp, 0, xArray );
        }
    }
    else if( rScriptType.EqualsAscii( "JavaScript" ) )
    {
        DBG_ERROR( "JavaScript not allowed" );
        return 0;
    }
    else
    {
        DBG_ERROR( "StarScript not allowed" );
    }
    return nErr;
}

SfxFrame* SfxObjectShell::GetSmartSelf( SfxFrame* pSelf, SfxMedium& rMedium )
{
    return pSelf;
}

SfxObjectShellFlags SfxObjectShell::GetFlags() const
{
    if( pImp->eFlags == SFXOBJECTSHELL_UNDEFINED )
        pImp->eFlags = GetFactory().GetFlags();
    return pImp->eFlags;
}

void SfxObjectShell::SetFlags( SfxObjectShellFlags eFlags )
{
    pImp->eFlags = eFlags;
}

void SfxObjectShell::SetBaseURL( const String& rURL )
{
    pImp->aBaseURL = rURL;
}

const String& SfxObjectShell::GetBaseURL() const
{
    if ( pImp->aBaseURL.Len() )
        return pImp->aBaseURL;
    else if ( pMedium->GetFilter() && ( pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_PACKED ) )
        return pMedium->GetPhysicalName();
    else
        return pMedium->GetName();
}

String SfxObjectShell::QueryTitle( SfxTitleQuery eType ) const
{
    String aRet;

    switch( eType )
    {
        case SFX_TITLE_QUERY_SAVE_NAME_PROPOSAL:
        {
            SfxMedium* pMed = GetMedium();
            const INetURLObject& rObj = pMed->GetURLObject();
            aRet = rObj.GetMainURL();
            if ( !aRet.Len() )
                aRet = GetTitle( SFX_TITLE_CAPTION );
            break;
        }
    }
    return aRet;
}

void SfxHeaderAttributes_Impl::SetAttributes()
{
    bAlert = sal_True;
    SvKeyValue aPair;
    for( sal_Bool bCont = xIter->GetFirst( aPair ); bCont;
         bCont = xIter->GetNext( aPair ) )
        SetAttribute( aPair );
}

void SfxHeaderAttributes_Impl::SetAttribute( const SvKeyValue& rKV )
{
    String aValue = rKV.GetValue();
    if( rKV.GetKey().CompareIgnoreCaseToAscii( "refresh" ) == COMPARE_EQUAL && rKV.GetValue().Len() )
    {
        sal_uInt32 nTime = aValue.GetToken(  0, ';' ).ToInt32() ;
        String aURL = aValue.GetToken( 1, ';' );
        aURL.EraseTrailingChars().EraseLeadingChars();
        SfxDocumentInfo& rInfo = pDoc->GetDocInfo();
        if( aURL.Copy(0, 4).CompareIgnoreCaseToAscii( "url=" ) == COMPARE_EQUAL )
        {
            INetURLObject aObj;
            pDoc->GetMedium()->GetURLObject().GetNewAbsURL(
                aURL.Copy( 4 ), &aObj );
            rInfo.SetReloadURL( aObj.GetMainURL() );
        }
        rInfo.EnableReload( sal_True );
        rInfo.SetReloadDelay( nTime );
        pDoc->FlushDocInfo();
    }
    else if( rKV.GetKey().CompareIgnoreCaseToAscii( "expires" ) == COMPARE_EQUAL )
    {
        DateTime aDateTime;
        if( INetRFC822Message::ParseDateField( rKV.GetValue(), aDateTime ) )
        {
            aDateTime.ConvertToLocalTime();
            pDoc->GetMedium()->SetExpired_Impl( aDateTime );
        }
        else
        {
//          DBG_ERROR( "Schlechtes ::com::sun::star::util::DateTime fuer Expired" );
            pDoc->GetMedium()->SetExpired_Impl( Date( 1, 1, 1970 ) );
        }
    }
}

void SfxHeaderAttributes_Impl::Append( const SvKeyValue& rKV )
{
    xIter->Append( rKV );
    if( bAlert ) SetAttribute( rKV );
}

SvKeyValueIterator* SfxObjectShell::GetHeaderAttributes()
{
    if( !pImp->xHeaderAttributes.Is() )
    {
        DBG_ASSERT( pMedium, "Kein Medium" );
        pImp->xHeaderAttributes = new SfxHeaderAttributes_Impl( this );
    }
    return ( SvKeyValueIterator*) &pImp->xHeaderAttributes;
}

void SfxObjectShell::ClearHeaderAttributesForSourceViewHack()
{
    ((SfxHeaderAttributes_Impl*)GetHeaderAttributes())
        ->ClearForSourceView();
}


void SfxObjectShell::SetHeaderAttributesForSourceViewHack()
{
    ((SfxHeaderAttributes_Impl*)GetHeaderAttributes())
        ->SetAttributes();
}

void SfxObjectShell::StartLoading_Impl()
{
    pImp->nLoadedFlags = 0;
}

sal_Bool SfxObjectShell::IsPreview() const
{
    if ( !pMedium )
        return sal_False;

    sal_Bool bPreview = sal_False;
    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
    if ( pFlags )
    {
        // Werte auf einzelne Items verteilen
        String aFileFlags = pFlags->GetValue();
        aFileFlags.ToUpperAscii();
        if ( STRING_NOTFOUND != aFileFlags.Search( 'B' ) )
            bPreview = sal_True;
    }

    if ( !bPreview )
    {
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pItem, SfxBoolItem, SID_PREVIEW, sal_False);
        if ( pItem )
            bPreview = pItem->GetValue();
    }

    return bPreview;
}

sal_Bool SfxObjectShell::IsSecure()
{
    // Wenn globale Warnung an ist, nach Secure-Referer-Liste gehen
    String aReferer = GetMedium()->GetName();
    if ( !aReferer.Len() )
    {
        // bei neuen Dokumenten das Template als Referer nehmen
        String aTempl( GetDocInfo().GetTemplateFileName() );
        if ( aTempl.Len() )
            aReferer = INetURLObject( aTempl, INET_PROT_FILE ).GetMainURL();
        else
            // Da leider ein noch nicht gespeichertes Dokument zwar als "lokal"
            // zu gelten hat, aber kein ::com::sun::star::util::URL hat: ( !!HACK!! )
            aReferer = DEFINE_CONST_UNICODE( "file://" );
    }

    INetURLObject aURL( "macro:" );
    if ( SFX_APP()->IsSecureURL( aURL, &aReferer ) )
    {
        if ( GetMedium()->GetContent().is() )
        {
            Any aAny( UCB_Helper::GetProperty( GetMedium()->GetContent(), WID_IS_PROTECTED ) );
            sal_Bool bIsProtected = FALSE;
            if ( ( aAny >>= bIsProtected ) && bIsProtected )
                return sal_False;
            else
                return sal_True;
        }
        else
            return sal_True;
    }
    else
        return sal_False;
}

void SfxObjectShell::SetWaitCursor( BOOL bSet ) const
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
    {
        if ( bSet )
            pFrame->GetFrame()->GetWindow().EnterWait();
        else
            pFrame->GetFrame()->GetWindow().LeaveWait();
    }
}

String SfxObjectShell::GetAPIName() const
{
    INetURLObject aURL( GetMedium()->GetName() );
    String aName( aURL.GetBase() );
    if( !aName.Len() )
        aName = aURL.GetURLNoPass();
    if ( !aName.Len() )
        aName = GetTitle( SFX_TITLE_DETECT );
    return aName;
}

void SfxObjectShell::Invalidate( USHORT nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

