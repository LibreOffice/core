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

#include <ctype.h>
#include <stdio.h>

//#define REMOTE_VERSION

#include "bf_so3/applet.hxx"
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include "insdlg.hxx"
#include <svuidlg.hrc>
#include <bf_so3/ipenv.hxx>
#include "bf_so3/ipclient.hxx"
#include <bf_so3/svstor.hxx>
#include "bf_so3/ipwin.hxx"
#include "bf_so3/soerr.hxx"
#include <vcl/syschild.hxx>
#include <sot/formats.hxx>
#include <comphelper/classids.hxx>

#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace binfilter {

#define SoResId( id )	ResId( id, *SOAPP->GetResMgr() )

// forward declarations
class SvAppletEnvironment;

//=========================================================================
//=========================================================================
//=========================================================================
struct SvAppletData_Impl
/*	[Beschreibung]

    In diesem struct werden sich Member von SvAppletData gemerkt, um
    nicht immer inkompatibel zu werden.
*/
{
    SvAppletEnvironment * 	pAppletEnv;
    SvCommandList			aCmdList;
    String					aClass;
    XubString				aName;
    XubString				aCodeBase;
    BOOL					bMayScript;
    INetURLObject *			pDocBase;	// Ist nicht persistent

    SvAppletData_Impl()
        : pAppletEnv( NULL )
        , bMayScript( FALSE )
        , pDocBase( NULL )
        {}
};

//=========================================================================
//============== SvAppletEnvironment ======================================
//=========================================================================
class SvAppletEnvironment : public SvInPlaceEnvironment
{
protected:
    virtual void 	RectsChangedPixel( const Rectangle & rObjRect,
                                     const Rectangle & rClip );
    virtual void	ShowIPObj( BOOL bShow );
public:
                    SvAppletEnvironment( SvContainerEnvironment *,
                                         SvAppletObject * );
                    ~SvAppletEnvironment();

#ifdef SOLAR_JAVA
    virtual void appletResize( const Size & );
    virtual void showDocument( const INetURLObject &, const XubString & );
    virtual void showStatus( const XubString & );
#endif
};

class NoCursorWindow : public Window
{
public:
    NoCursorWindow( Window* pParent, WinBits nStyle )
    : Window( pParent, nStyle ) {}

    virtual long		Notify( NotifyEvent& rNEvt );
};

long NoCursorWindow::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent rKEvt = *rNEvt.GetKeyEvent();

        if( !rKEvt.GetKeyCode().GetModifier() )
        {
            USHORT nCode = rKEvt.GetKeyCode().GetCode();
            switch( nCode )
            {
                case KEY_HOME:
                case KEY_END:
                case KEY_UP:
                case KEY_DOWN:
                case KEY_LEFT:
                case KEY_RIGHT:
                    return 1;
            }
        }
    }
    return Window::Notify( rNEvt );
}

//=========================================================================
SvAppletEnvironment::SvAppletEnvironment
(
    SvContainerEnvironment * pFrm,	/* Das Callback Gegenst"uck zum
                                       InPlace-Environment */
    SvAppletObject * pObj_			/* Das zum Environment geh"orende
                                       Objekt */
)
/*	[Beschreibung]

    Das Environment wird im <SvAppletObject::InPlaceActivate()> angelegt.
    Durch die Verbindung mit dem Container Environment kann "uber die
    UI-Tools und Fenster verhandelt werden.

    [Querverweise]

    <SvInPlaceEnvironment>, <SvContainerEnvironment>
*/
    : SvInPlaceEnvironment( pFrm, pObj_ )
{
    // Das Border- und Clip-Window werden erzeugt
      MakeWindows();
      SvInPlaceWindow * pBW = GetBorderWin();
      pBW->SetHatchBorderPixel( Size() );

      // Eigentliches Fenster erzeugen, es muss ein AppletWindow sein
      Window * pWin;
      pWin = new SystemChildWindow( pBW, WB_CLIPCHILDREN );
    pWin->SetBackground();
    SetEditWin( pWin );
    pWin->Show();

    // Fenster zuordnen (fuers Resize)
    pBW->SetObjWin( pWin );
}

//=========================================================================
SvAppletEnvironment::~SvAppletEnvironment()
/*	[Beschreibung]

    Die angelegten Fenster werden zerst"ort.
*/
{
    Window * pAppletWin = GetEditWin();
    // statt DeleteEditWin() auf NULL setzen und durch den Manager
    // zerst"oren
    SetEditWin( NULL );
    delete pAppletWin;
    DeleteWindows();
    DeleteObjMenu();
}

//=========================================================================
void SvAppletEnvironment::RectsChangedPixel
(
    const Rectangle & rObjRect, /* Position und Gr"osse relativ zum
                                   EditWin des Containers */
    const Rectangle & rClip		/* Clipping des Containers auf das Objekt */
)
/*	[Beschreibung]

    Der Container "andert die Gr"osse oder Position des Objektes. Dies
    wird an das Applet gemeldet.

    [Querverweise]

    <SvInPlaceEnvironment::RectsChangedPixel()>
*/
{
    SvInPlaceEnvironment::RectsChangedPixel( rObjRect, rClip );
}

//=========================================================================
void SvAppletEnvironment::ShowIPObj
(
    BOOL bShow	/* TRUE, IP-Window anzeigen.
                   FALSE, IP-Window nicht anzeigen */
)
/*	[Beschreibung]

    Anzeigen de IP-Windows. Es wird nur angezeigt, wenn es nicht im
    Hintergrund gestartet werden soll.
*/
{
    SvInPlaceEnvironment::ShowIPObj( bShow );
}

#ifdef SOLAR_JAVA
//=========================================================================
void SvAppletEnvironment::showStatus( const XubString & rStatus )
{
    GetContainerEnv()->SetStatusText( rStatus );
}

//=========================================================================
void SvAppletEnvironment::showDocument
(
    const INetURLObject & rURL,
    const XubString & rTarget
)
{
    GetContainerEnv()->ShowDocument( rURL, rTarget );
}

//=========================================================================
void SvAppletEnvironment::appletResize( const Size & )
{
}


#endif

//=========================================================================
//============== SvAppletObject ===========================================
//=========================================================================
SO2_IMPL_BASIC_CLASS1_DLL( SvAppletObject, SvFactory, SvInPlaceObject,
              SvGlobalName( SO3_APPLET_CLASSID ) )


//=========================================================================
SvAppletObject::SvAppletObject()
    : pImpl( new SvAppletData_Impl )
/*	[Beschreibung]

    Konstruktor der Klasse SvAppletObject. Die Verbliste und das
    Format werden global initialisiert.
*/
{
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pAppletVerbList )
    {
        pSoApp->pAppletVerbList = new SvVerbList();
        // Alle unterstuetzten Verben anlegen
        pSoApp->pAppletVerbList->Append(
                SvVerb( 0, String( SoResId( STR_VERB_OPEN ) ) ) );
        pSoApp->pAppletVerbList->Append(
                SvVerb( SVVERB_PROPS, String( SoResId( STR_VERB_PROPS ) ) ) );
        pSoApp->nAppletDocFormat = SOT_FORMATSTR_ID_APPLETOBJECT;
    }
    // Verben der Superklasse bekanntgeben, um GetVerbs nicht
    // ueberlagern zu muessen
    SetVerbList( pSoApp->pAppletVerbList );
}

//=========================================================================
SvAppletObject::~SvAppletObject()
/*	[Beschreibung]

    Destruktor der Klasse SvAppletObject.
*/
{
    DELETEZ( pImpl->pDocBase );
    delete pImpl;
}

//=========================================================================
BOOL SvAppletObject::StartApplet()
{
    return FALSE;
}

//=========================================================================
void SvAppletObject::DataChanged_Impl
(
    BOOL bOnlyEmbedSource	/* TRUE, es "andert sich nur die persistenten
                               Daten. FALSE, auch das MetaFile "andert
                               sich */
)
/*	[Beschreibung]

    Wenn die Daten sich "andern, muss das Modiy-Flag gesetzt werden.
    Ausserdem m"ussen alle angemeldeten Advises benachrichtigt werden.
    In Abh"angigkeit des Parameters wird angezeigt, dass sich auch
    die View und das Mtf ge"andert hat.
*/
{
    if( IsEnableSetModified() )
    { // nicht im init oder deinit
        SetModified( TRUE );
        if( !bOnlyEmbedSource )
            ViewChanged( ASPECT_CONTENT );
    }
}

//=========================================================================
void SvAppletObject::FillClass
(
    SvGlobalName * pClassName,	/* Der Typ der Klasse */
    ULONG * pFormat,			/* Das Dateiformat in dem geschrieben wird */
    String * pAppName,			/* Der Applikationsname */
    String * pFullTypeName,     /* Der genaue Name des Typs	*/
    String * pShortTypeName,	/* Der kurze Name des Typs	*/
    long /*nFileFormat*/		/* F"ur dieses Office-Format sollen die
                                   Parameter gef"ullt werden */
) const
/*	[Beschreibung]

    Mit dieser Methode werden Informationen "uber das Objekt angefordert.
    Wird das Objekt gesichert, dann werden diese Informationen in den
    Storage geschrieben.

    [Anmerkung]

    Da diese Information nicht vom Applet ermittelt werden kann, m"ussen
    Standardwerte angegeben werden. Dies bedeutet, von aussen gibt es
    nur den Typ Applet, in den man nicht hinein schauen kann.
*/
{
    *pClassName     = *GetSvFactory();
    *pFormat    	= SOAPP->nAppletDocFormat;
    *pAppName		= String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarDivision Applet 1.0" ) );
    *pFullTypeName  = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarDivision Applet 1.0" ) );
    *pShortTypeName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Applet" ) );
}


//=========================================================================
void SvAppletObject::Open
(
    BOOL bOpen	/* TRUE, es handelt sich um eine Open.
                   FALSE, es handelt sich um Close. */
)
/*	[Beschreibung]

    Die Verbindung zum Client wird ge"offnet oder geschlossen.

    [Querverweise]

    <SvEmbeddedObject::Open()>
*/
{
    // l"asst Environment los
    SvInPlaceObject::Open( bOpen );
}

//=========================================================================
static sal_Bool isAppletEnabled();
void SvAppletObject::InPlaceActivate
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
    sal_Bool bEnabled= isAppletEnabled();
    if( bActivate  && bEnabled)
    {
        SvContainerEnvironment * pEnv;
        pEnv = GetIPClient()->GetEnv();
        pImpl->pAppletEnv = new SvAppletEnvironment( pEnv, this );

        // wird in die Verwaltung eingesetzt
        SetIPEnv( pImpl->pAppletEnv );

        if( !StartApplet() )
        {
            DoClose();
            return;
        }
    }
    // SvInPlaceObject::InPlaceActivate must not be called if SetIPEnv has not been
    // called (see above). It is possibe that the status of the "Enable Applet" option
    // changes between the calls InPlaceActivate( true) and InPlaceActiveFalse.
    if( (bEnabled && pImpl->pAppletEnv) ||
        (!bActivate && pImpl->pAppletEnv) )
    {
        SvInPlaceObject::InPlaceActivate( bActivate );
    }
    if( !bActivate && pImpl->pAppletEnv )
        DELETEZ( pImpl->pAppletEnv );
}

// This function detects if Applets are enabled. This option can be selected in the
// options dialog (StarOffice node->security)
static sal_Bool isAppletEnabled()
{
    sal_Bool bValue= sal_False;

    Reference<XInterface> xConfRegistry = ::comphelper::getProcessServiceFactory()->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationRegistry")));
    if(!xConfRegistry.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());
    
    Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
    if(!xConfRegistry_simple.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());
    
    xConfRegistry_simple->open(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common")), sal_True, sal_False);
    Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();
    
    if (xRegistryRootKey.is())
    {
        Reference<XRegistryKey> key_Enable = xRegistryRootKey->openKey(OUString(
            RTL_CONSTASCII_USTRINGPARAM("Java/Applet/Enable")));
        if (key_Enable.is())
        {
            bValue= key_Enable->getLongValue() != 0;
        }
    }
    xConfRegistry_simple->close();
    return bValue;
}
//=========================================================================
ErrCode SvAppletObject::Verb
(
    long 				nVerb,		/* welche Art des Aktivierens ist
                                       gew"unscht */
    SvEmbeddedClient *	pCl,		/* Callback-Svhnittstelle des Aufruffers */
    Window * 			pWin,		/* Parent auf dem aktiviert werden soll */
    const Rectangle * pWorkRectPixel/* Position und Gr"osse, an der das Objekt
                                       aktiviert werden soll */
)
/*	[Beschreibung]

    Es wird Versucht ein Applet zu starten. Es gibt nur die M"oglichkeit
    InPlace zu aktivieren. Deshalb sind auch nur die Verben gestattet,
    die dies ausl"osen.

    [R"uckgabewert]

    ErrCode		ERRCODE_NONE, es wurde InPlace aktiviert.
                ERRCODE_SO_NOT_INPLACEACTIVE, es wurde nicht InPlace
                aktiviert.

    [Querverweise]

    <SvPseudoObject::Verb>
*/
{
    (void)pCl;
    (void)pWin;
    (void)pWorkRectPixel;

    ErrCode nRet = ERRCODE_SO_GENERALERROR;

/*
    nRet = SjWrapper::CheckJavaEnvironment();
    BOOL bJavaOk = TRUE;
    bJavaOk = SjWrapper::IsJavaRuntimeOk();
    if( !ERRCODE_TOERROR( nRet ) && bJavaOk )
*/
    {
        switch( nVerb )
        {
            case SVVERB_PROPS:
            {
                DBG_ERROR( "non-working code!" );
                // TODO: dead corpses
                nRet = 0;
                break;
            }
            case SVVERB_SHOW:
                break;

            case SVVERB_IPACTIVATE:
                break;

            case 0L:
                nRet = GetProtocol().IPProtocol();
                break;
            case SVVERB_HIDE:
                nRet = DoInPlaceActivate( FALSE );
                break;
            default:
                nRet = ERRCODE_SO_GENERALERROR;
                break;
        }
    }
    return nRet;
}

//=========================================================================
void SvAppletObject::SetVisArea
(
    const Rectangle & rVisArea	/* neue Position und Groesse des
                                   sichtbaren Ausschnitts */
)
/*	[Beschreibung]

    Der sichtbare Ausschnitt beginnt immer an der Position (0, 0).
*/
{
    Rectangle aR( rVisArea );
    aR.SetPos( Point() );
    SvInPlaceObject::SetVisArea( aR );

    DataChanged_Impl( TRUE );
}

//=========================================================================
// aus PlugIn
void SoPaintReplacement( const Rectangle &rRect, String &rText,
                         OutputDevice *pOut );

//=========================================================================
void SvAppletObject::Draw
(
    OutputDevice * 	pDev,	/* in dieses Device findet die Ausgabe statt */
    const JobSetup &,       /* fuer dieses Device soll formatiert werden */
    USHORT 			nAspect /* Darstellungsvariante des Objektes */
)
/*	[Beschreibung]

    Ein Ausgabe ist nicht m"oglich. Deswegen wird eine Bitmap
    und als Unterschrift der URL ausgegeben,

    [Querverweise]

    <SvEmbeddedObject::Draw>
*/
{
    Rectangle aVisArea_ = GetVisArea( nAspect );
    SoPaintReplacement( aVisArea_, pImpl->aClass, pDev );
}

//=========================================================================
BOOL SvAppletObject::InitNew
(
    SvStorage * pStor	/* Storage auf dem das Objekt arbeitet. Der kann
                           auch NULL sein, das Bedeutet, es wird auf einem
                           tempor"aren Storage gearbeitet */
)
/*	[Beschreibung]

    Nach dem Konstruktor muss diese Methode oder Load gerufen werden,
    um das Objekt zu initialisieren.
    <SvPersist::InitNew>

    [R"uckgabewert]

    BOOL			TRUE, Initialisierung hat geklappt.
                    FALSE, Initialisierung hat nicht geklappt, das Objekt
                    muss sofort freigegeben werden.

    [Querverweise]

*/
{
    if( SvInPlaceObject::InitNew( pStor ) )
    {
        // Standardgr"osse
        SetVisArea( Rectangle( Point(), Size( 5000, 5000 ) ) );
        return TRUE;
    }
    return FALSE;
}

//=========================================================================
#define DOCNAME "Applet"
#define APPLET_VERS	1

BOOL SvAppletObject::Load
(
    SvStorage * pStor	/* Storage aus dem das Objekt geladen wird. */
)
/*	[Beschreibung]

    Nach dem Konstruktor muss diese Methode oder InitNew gerufen werden,
    um das Objekt zu initialisieren.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geladen.
                    FALSE, das Objekt wurde nicht geladen, es
                    muss sofort freigegeben werden.

    [Querverweise]

    <SvPersist::Load>
*/
{
    if( SvInPlaceObject::Load( pStor ) )
    {
        SvStorageStreamRef xStm;
        xStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ), STREAM_STD_READ );
        xStm->SetVersion( pStor->GetVersion() );
        xStm->SetBufferSize( 8192 );

        // nicht vorhandener Stream ist kein Fehler
        if( xStm->GetError() == SVSTREAM_FILE_NOT_FOUND )
            return TRUE;

        BYTE nVer;
        *xStm >> nVer;
        if( nVer == APPLET_VERS )
        {
            *xStm >> pImpl->aCmdList;
            xStm->ReadByteString( pImpl->aClass, RTL_TEXTENCODING_ASCII_US );
            xStm->ReadByteString( pImpl->aName, RTL_TEXTENCODING_ASCII_US );
            xStm->ReadByteString( pImpl->aCodeBase, RTL_TEXTENCODING_ASCII_US );
            *xStm >> pImpl->bMayScript;
        }
        else
            xStm->SetError( ERRCODE_IO_WRONGVERSION );

        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
BOOL SvAppletObject::Save()
/*	[Beschreibung]

    Der Inhalt des Objektes wird in den, in <SvAppletObject::InitNew>
    oder <SvAppletObject::Load> "ubergebenen Storage, geschrieben.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geschreiben.
                    FALSE, das Objekt wurde nicht geschrieben. Es muss
                    die in der Klasse <SvPersist> beschrieben
                    Fehlerbehandlung erfolgen.

    [Querverweise]

    <SvPersist::Save>
*/
{
    if( SvInPlaceObject::Save() )
    {
        SvStorageStreamRef xStm;
        xStm = GetStorage()->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ),
                                            STREAM_STD_WRITE | STREAM_TRUNC );
        xStm->SetVersion( GetStorage()->GetVersion() );
        xStm->SetBufferSize( 8192 );

        *xStm << (BYTE)APPLET_VERS;
        *xStm << pImpl->aCmdList;
        xStm->WriteByteString( pImpl->aClass, RTL_TEXTENCODING_ASCII_US );
        xStm->WriteByteString( pImpl->aName, RTL_TEXTENCODING_ASCII_US );
        xStm->WriteByteString( pImpl->aCodeBase, RTL_TEXTENCODING_ASCII_US );
        *xStm << pImpl->bMayScript;
        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
BOOL SvAppletObject::SaveAs
(
    SvStorage *pStor	/* Storage, in den der Inhalt des Objekte
                           geschrieben wird */
)
/*	[Beschreibung]

    Der Inhalt des Objektes wird in pStor geschrieben.

    [Anmerkung]

    Der Storage wird nicht behalten.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geschreiben.
                    FALSE, das Objekt wurde nicht geschrieben. Es muss
                    die in der Klasse <SvPersist> beschrieben
                    Fehlerbehandlung erfolgen.

    [Querverweise]

    <SvPersist::SaveAs>
*/
{
    if( SvInPlaceObject::SaveAs( pStor ) )
    {
        SvStorageStreamRef xStm;
        xStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ), STREAM_STD_WRITE | STREAM_TRUNC );
        xStm->SetVersion( pStor->GetVersion() );
        xStm->SetBufferSize( 8192 );

        *xStm << (BYTE)APPLET_VERS;
        *xStm << pImpl->aCmdList;
        xStm->WriteByteString( pImpl->aClass, RTL_TEXTENCODING_ASCII_US );
        xStm->WriteByteString( pImpl->aName, RTL_TEXTENCODING_ASCII_US );
        xStm->WriteByteString( pImpl->aCodeBase, RTL_TEXTENCODING_ASCII_US );
        *xStm << pImpl->bMayScript;

        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
void SvAppletObject::HandsOff()
/*	[Beschreibung]

    Ab diesen Zeitpunkt, bis zum <SvAppletObject::SaveCompleted>,
    darf auf den Storage im Objekt nicht zugegriffen werden.

    [Querverweise]

    <SvPersist::HandsOff>
*/
{
    SvInPlaceObject::HandsOff();
}

//=========================================================================
BOOL SvAppletObject::SaveCompleted
(
    SvStorage * pStor	/* Storage auf dem das Objekt arbeitet. Der kann
                           auch NULL sein. Dies Bedeutet, es wird auf
                           dem alten Storage weiter gearbeitet */
)
/*	[Beschreibung]

    Nach dem Aufruf dieser Methode ist das Verhalten des Objektes
    wieder definiert.

    [R"uckgabewert]

    BOOL			TRUE, es kann auf dem neuen Storage gearbeitet werden.
                    FALSE, es kann nicht auf dem neuen Storage gearbeitet
                    werden

    [Querverweise]

    <SvPersist::SaveCompleted>
*/
{
    return SvInPlaceObject::SaveCompleted( pStor );
}

//=========================================================================
ULONG SvAppletObject::GetMiscStatus() const
/*	[Beschreibung]

    Da ein Applet immer aktiviert wird, es ein Link ist und er
    keine UI-Tools braucht, muss dieser Status zur"uckgegeben werden.

    [R"uckgabewert]

    ULONG           Es wird immer  SVOBJ_MISCSTATUS_LINK,
                    SVOBJ_MISCSTATUS_INSIDEOUT und
                    SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE zur"uckgegeben

    [Querverweise]

    <SvPseudoObject::GetMiscStatus()>
*/
{
    ULONG nMisc = 0;

    //nMisc = SVOBJ_MISCSTATUS_LINK;
    nMisc |= SVOBJ_MISCSTATUS_INSIDEOUT;
    nMisc |= SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE;
    nMisc |= SVOBJ_MISCSTATUS_ALWAYSACTIVATE;
    nMisc |= SVOBJ_MISCSTATUS_SPECIALOBJECT;
    return nMisc;
}

//=========================================================================
BOOL SvAppletObject::IsLink() const
/*	[Beschreibung]

    Dieser Typ von Objekt ist immer ein Link.

    [R"uckgabewert]

    BOOL            Immer TRUE.

    [Querverweise]

    <SvPseudoObject::IsLink()>
*/
{
    //return TRUE;
    return FALSE;
}

//=========================================================================
void SvAppletObject::SetCommandList
(
    const SvCommandList & rList	/* Die Liste der Kommnados */
)
/*	[Beschreibung]

    Die Liste der Kommandos, die "ubergeben werden, wenn das Applet
    gestartet wird.

    [Anmerkung]

    Ein bereits gestartetes Applet wird dadurch nicht beeinflusst.
    Die neuen Kommandos werden erst beim n"achsten starten ausgew"ahlt.
*/
{
    pImpl->aCmdList = rList;

    DataChanged_Impl( TRUE );
}

//=========================================================================
const SvCommandList & SvAppletObject::GetCommandList() const
/*	[Beschreibung]

    Die Liste der Kommandos, die in <SetCommandList> "ubergeben werden,
    werden zur"uckgegeben.
*/
{
    return pImpl->aCmdList;
}

//=========================================================================
void SvAppletObject::SetClass
(
    const String & rClass	/* Der Verweis auf die Appletklasse */
)
/*	[Beschreibung]

    Der Verweis auf die Appletklasse wird "uebergeben. Dies entspricht
    dem "code" Attribut des Applet-Tags in HTML.

    [Anmerkung]

    Ein bereits gestartetes Applet wird dadurch nicht beeinflusst.
    Die neue Quelle wird erst beim n"achsten starten ausgew"ahlt.
*/
{
    if( pImpl->aClass != rClass )
    {
        pImpl->aClass = rClass;
        DataChanged_Impl( FALSE );
    }
}

//=========================================================================
const String & SvAppletObject::GetClass() const
/*	[Beschreibung]

    Der Verweis auf die Quelle wird zur"uckgegeben

*/
{
    return pImpl->aClass;
}

//=========================================================================
void SvAppletObject::SetName
(
    const XubString & rName	/* Name des Applets */
)
/*	[Beschreibung]

    Der Verweis auf den name des Applet wird "uebergeben. Dies entspricht
    dem "namee" Attribut des Applet-Tags in HTML.

    [Anmerkung]

    Ein bereits gestartetes Applet wird dadurch nicht beeinflusst.
*/
{
    if( pImpl->aName != rName )
    {
        pImpl->aName = rName;
        DataChanged_Impl( FALSE );
    }
}

//=========================================================================
const XubString & SvAppletObject::GetName() const
/*	[Beschreibung]

    Der Name des Applets wird zur"uckgegeben

*/
{
    return pImpl->aName;
}

//=========================================================================
void SvAppletObject::SetCodeBase
(
    const XubString & rCodeBase	/* Der Verweis auf den Code
                                   Appletklasse */
)
/*	[Beschreibung]

    Der Verweis auf den Code der Appletklasse wird "uebergeben.
    Dies entspricht dem "codebase" Attribut des Applet-Tags in HTML.

    [Anmerkung]

    Ein bereits gestartetes Applet wird dadurch nicht beeinflusst.
*/
{
    if( pImpl->aCodeBase != rCodeBase )
    {
        pImpl->aCodeBase = rCodeBase;
        DataChanged_Impl( FALSE );
    }
}

//=========================================================================
const XubString & SvAppletObject::GetCodeBase() const
/*	[Beschreibung]

    Der Verweis auf die Quelle wird zur"uckgegeben

*/
{
    return pImpl->aCodeBase;
}

//=========================================================================
void SvAppletObject::SetMayScript( BOOL bMayScript )
{
    pImpl->bMayScript = bMayScript;
}

//=========================================================================
BOOL SvAppletObject::IsMayScript() const
{
    return pImpl->bMayScript;
}



}
