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

#include <bf_so3/ipwin.hxx>

#include <bf_svtools/plugpool.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "plugobj.hxx"


#include "frmenv.hxx"
#include "frmdescr.hxx"
#include "urlframe.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "clientsh.hxx"

#include <appdata.hxx>
namespace binfilter {

class SfxPluginObjectEnv_Impl : public SvPlugInEnvironment,
                                public SfxReleasableFrame
{
    SfxPluginObject*	pPluginObject;
public:
                        SfxPluginObjectEnv_Impl( SvContainerEnvironment*, SfxPluginObject*);
                        ~SfxPluginObjectEnv_Impl();

    virtual BOOL        MIMETypeDetected( const String& rMIME );
    virtual void		ReleaseFrame();
};

struct SfxPluginObject_Impl
/*	[Beschreibung]

    Impl-Struktur des SfxPluginObjects.
*/
{
    SfxPluginObjectEnv_Impl*	pEnv;
    SfxURLFrame*				pFrame;
    SfxFrameSetDescriptor*		pSet;

                                SfxPluginObject_Impl()
                                    : pFrame( NULL )
                                    , pEnv( NULL )
                                {}
};


class SfxPluginObjectWindow_Impl : public Window
{
    SfxPluginObject*	pObj;

protected:
    virtual void		Resize();

public:
                        SfxPluginObjectWindow_Impl( SfxPluginObject *p,
                                        Window *pParent, USHORT nBits )
                            : Window( pParent, nBits )
                            , pObj( p )
                        {}
};


void SfxPluginObjectWindow_Impl::Resize()
{
    // Die neue Fenstergr"o\se an den Frame weitergeben
    if ( pObj->GetFrame() )
        pObj->GetFrame()->GetWindow().SetOutputSizePixel( GetOutputSizePixel() );
}

//=========================================================================
SfxPluginObjectEnv_Impl::SfxPluginObjectEnv_Impl
(
    SvContainerEnvironment * pFrm,	/* Das Callback Gegenst"uck zum
                                       InPlace-Environment */
    SfxPluginObject * pObj			/* Das zum Environment geh"orende
                                       Objekt */
)
/*	[Beschreibung]

    Das Environment wird im <SfxPluginObject::InPlaceActivate()> angelegt.
    Durch die Verbindung mit dem Container Environment kann "uber die
    UI-Tools und Fenster verhandelt werden.

    [Querverweise]

    <SvInPlaceEnvironment>, <SvContainerEnvironment>
*/
    : SvPlugInEnvironment( pFrm, pObj )
    , pPluginObject( pObj )
{
}

//=========================================================================
SfxPluginObjectEnv_Impl::~SfxPluginObjectEnv_Impl()
/*	[Beschreibung]

    Die angelegten Fenster werden zerst"ort.
*/
{
    if( IsStarPlug() )
    {
        Window *pWin = GetEditWin();
        if( pWin )
        {
            pWin->Hide();
            SetEditWin( NULL );
            delete pWin;
            DeleteWindows();
        }
    }
}

void SfxPluginObjectEnv_Impl::ReleaseFrame()
{
    pPluginObject->ReleaseFrame_Impl();
}

//=========================================================================
//============== SfxPluginObject ===========================================
//=========================================================================
SO2_IMPL_BASIC_CLASS1_DLL( SfxPluginObject, SvFactory, SvPlugInObject,
              SvGlobalName( 0x4caa7761, 0x6b8b, 0x11cf,
                            0x89,0xca,0x0,0x80,0x29,0xe4,0xb0,0xb1 ) )

//=========================================================================
SfxPluginObject::SfxPluginObject()
    : pImpl( new SfxPluginObject_Impl )
/*	[Beschreibung]

    Konstruktor der Klasse SfxPluginObject. Die Verbliste und das
    Format werden global initialisiert.
*/
{
    pImpl->pSet = new SfxFrameSetDescriptor;

    // Verben der Superklasse bekanntgeben, um GetVerbs nicht
    // ueberlagern zu muessen
    SvVerbList *pVerbs = new SvVerbList;
    SetVerbList( pVerbs );
}

//=========================================================================
SfxPluginObject::~SfxPluginObject()
/*	[Beschreibung]

    Destruktor der Klasse SfxPluginObject.
*/
{
    delete pImpl;
}


BOOL SfxPluginObjectEnv_Impl::MIMETypeDetected( const String& rMIME )
{
    BOOL bRet;
    PlugInManager * pPlugInMgr = SvFactory::GetPlugInManager();
    PlugData* pPD =	pPlugInMgr->GetPlugForMime( rMIME );
    if( !pPD ) pPD = pPlugInMgr->GetPlugForExtension(
        pPluginObject->GetURL()->GetFileExtension() );
    if( pPD && !pPD->IsStarPlug() )
    {
        bRet = SvPlugInEnvironment::MIMETypeDetected( rMIME );
        if( bRet )
        {
            SvFactory::GetPlugInManager()->SetURLLoadHdl(
                GetEditWin(),
                STATIC_LINK( pPluginObject->GetFrame(), SfxFrame,
                            UrlLoadHdl ) );
        }
    }
    else
    {
        SvInPlaceWindow * pBW = GetBorderWin();
        // Eigentliches Fenster erzeugen
        Window *pWin = new SfxPluginObjectWindow_Impl(
            pPluginObject, pBW, WB_CLIPCHILDREN );
        SetEditWin( pWin );
        pWin->Show();

        // Fenster zuordnen (fuers Resize)
        pBW->SetObjWin( pWin );
        bRet = pPluginObject->ConstructFrame();
    }

    return bRet;
}

//=========================================================================
BOOL SfxPluginObject::ConstructFrame()
/*	[Beschreibung]

    Die Methode erzeugt den Frame. Es ist notwendig, da\s das Objekt
    InPlace-Active ist.
*/
{
    // Nur in SfxApps kann ein Frame aufgebaut werden
    SfxInPlaceClientRef aClient = GetIPClient();
    if ( !aClient.Is() )
        return FALSE;

    // Der Frame erh"alt als Parent den ViewFrame seines Clients
    SfxFrame *pParent = aClient->GetViewShell()->GetViewFrame()->GetFrame();
    SfxFrameDescriptor *pFD = new SfxFrameDescriptor( pImpl->pSet );
    pFD->SetURL( *GetURL() );

    // Den URLFrame erzeugen, der l"adt dann asynchron das Dokument
    pImpl->pFrame = new SfxURLFrame( pFD, pImpl->pEnv, pImpl->pEnv, pParent );

    // Dieser Frame wird nicht "uber eine Id, sondern "uber seinen
    // Objektnamen identifiziert; daher auf keinen Fall die Id setzen,
    // wohl aber den IdName.
    pImpl->pFrame->SetFrameIdName_Impl( GetParent()->Find(this)->GetObjName() );
    return TRUE;
}

//=========================================================================
void SfxPluginObject::FillClass
(
    SvGlobalName * pClassName,	/* Der Typ der Klasse */
    ULONG * pFormat,			/* Das Dateiformat in dem geschrieben wird */
    String * pAppName,			/* Der Applikationsname */
    String * pFullTypeName,     /* Der genaue Name des Typs	*/
    String * pShortTypeName		/* Der kurze Name des Typs	*/
) const
/*	[Beschreibung]

    Mit dieser Methode werden Informationen "uber das Objekt angefordert.
    Wird das Objekt gesichert, dann werden diese Informationen in den
    Storage geschrieben.

    [Anmerkung]
*/
{
    *pClassName     = *GetSvFactory();
    *pFormat    	= 0;
    *pAppName		= "PlugIn";
    *pFullTypeName  = "PlugIn";
    *pShortTypeName = "PlugIn";
}

//=========================================================================
void SfxPluginObject::FillRegInfo
(
    SvEmbeddedRegistryInfo * pInfo	/* In diese Objekt werden alle
                                       relevaten Informationen eingetragen */
)
/*	[Beschreibung]

    Mit dieser Methode werden Informationen "uber das Objekt angefordert.
    Diese Informationen werden dazu benutzt, um das Objekt in der
    Registration DataBase von Windows anzumelden.

    [Anmerkung]
*/
{
    SvInPlaceObject::FillRegInfo( pInfo ); // setzt alle anderen Werte
    pInfo->aObjName 			= "PlugInObject";
    pInfo->nMajorVers			= 1;
    pInfo->nMinorVers 			= 0;
    pInfo->aHumanAppName 		= "PlugIn";
    pInfo->aHumanShortTypeName	= "PlugIn";
}



//=========================================================================
void SfxPluginObject::InPlaceActivate
(
    BOOL bActivate	/* TRUE, InPlace Aktivierung beginnt.
                       FALSE, InPlace Aktivierung endet. */
)
/*	[Beschreibung]

    Das Objekt wird InPlace aktiviert oder deaktiviert.

    [Querverweise]

    <SvInPlaceObject::InPlaceActivate()>
*/
{
    if( bActivate )
    {
        SvContainerEnvironment* pEnv = GetIPClient()->GetEnv();
        pImpl->pEnv = new SfxPluginObjectEnv_Impl( pEnv, this );

        // wird in die Verwaltung eingesetzt
        SetIPEnv( pImpl->pEnv );
        SvPlugInObject::InPlaceActivate( bActivate );
    }
    else
    {
        // Wichtig: zuerst den Frame l"oschen, denn der benachrichtigt in
        // seinem dtor das Environment, das aber im InPlaceDeactivate zerst"ort
        // wird. Au\serdem wird dabei das parent window des URLFrames abger"aumt!
        if ( pImpl->pFrame )
        {
            // Frame hat sich noch nicht abgemeldet
            pImpl->pFrame->DoClose();
            pImpl->pFrame = NULL;
        }

        SvPlugInObject::InPlaceActivate( bActivate );
    }
}

SfxFrame* SfxPluginObject::GetFrame() const
{
    return pImpl->pFrame;
}

void SfxPluginObject::ReleaseFrame_Impl()
{
    pImpl->pFrame = NULL;
}

SfxPluginObjectFactoryPtr* SfxPluginObject::GetFactoryPtr()
{
    return SFX_APP()->Get_Impl()->pSfxPluginObjectFactoryPtr;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
