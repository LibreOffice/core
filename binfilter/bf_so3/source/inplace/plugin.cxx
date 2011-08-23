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

#include <vcl/bitmap.hxx>
#include "bf_so3/plugin.hxx"
#include <comphelper/classids.hxx>
#include <sot/exchange.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <tools/debug.hxx>
#include <unotools/processfactory.hxx>

#include "bf_so3/ipclient.hxx"
#include <bf_so3/svstor.hxx>
#include "bf_so3/ipwin.hxx"
#include <ipmenu.hxx>
#include <svuidlg.hrc>
#include <tools/urlobj.hxx>
#include "bf_so3/soerr.hxx"
#include "bf_so3/staticbaseurl.hxx"

#define SoResId( id )	ResId( id, *SOAPP->GetResMgr() )

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/plugin/PluginMode.hpp>
#include <com/sun/star/plugin/XPlugin.hpp>
#include <com/sun/star/plugin/XPluginManager.hpp>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::plugin;

namespace binfilter {

class SvPlugInEnvironment_Impl
{
public:
    Reference< XPlugin >	_xPlugin;
    String					_aMimeType;
};

//=========================================================================
SvPlugInEnvironment::SvPlugInEnvironment
(
    SvContainerEnvironment * pFrm,	/* Das Callback Gegenst"uck zum
                                       InPlace-Environment */
    SvPlugInObject * pObjP			/* Das zum Environment geh"orende
                                       Objekt */
)
    : SvInPlaceEnvironment( pFrm, pObjP )
    , pObj( pObjP )
    , pImpl( new SvPlugInEnvironment_Impl )
/*	[Beschreibung]

    Das Environment wird im <SvPlugInObject::InPlaceActivate()> angelegt.
    Durch die Verbindung mit dem Container Environment kann "uber die
    UI-Tools und Fenster verhandelt werden.

    [Querverweise]

    <SvInPlaceEnvironment>, <SvContainerEnvironment>
*/
{
    // Das Border- und Clip-Window werden erzeugt
    MakeWindows();
    SvInPlaceWindow * pBW = GetBorderWin();
    pBW->SetHatchBorderPixel( Size() );
}


BOOL SvPlugInEnvironment::MIMETypeDetected( const String& rMIME )
{
    (void)rMIME;

    // obsolete -> should be removed
    return TRUE;
}


//=========================================================================
SvPlugInEnvironment::~SvPlugInEnvironment()
/*	[Beschreibung]

    Die angelegten Fenster werden zerst"ort.
*/
{
    Reference< XComponent > xComp( pImpl->_xPlugin, UNO_QUERY );
    if (xComp.is())
        xComp->dispose();

    // statt DeleteEditWin() auf NULL setzen und durch den Manager
    // zerst"oren
    SetEditWin( NULL );
    DeleteObjMenu();
    DeleteWindows();
    delete pImpl;
}

//=========================================================================
void * SvPlugInEnvironment::GetJavaPeer
(
    Window * //pPlugWin Plugin, zu dem die JavaKlasse besorgt wird
)
{
    return NULL;
}

//=========================================================================
void SvPlugInEnvironment::DocWinResize()
/*	[Beschreibung]

    Ist es ein Full-PlugIn, dann wird das IP-Fenster entsprechend
    der Gr"osse des DocWindows angepasst.

    [Querverweise]

    <SvPlugInEnvironment::RectsChangedPixel()>
*/
{
}

//=========================================================================
void SvPlugInEnvironment::RectsChangedPixel
(
    const Rectangle & rObjRect, /* Position und Gr"osse relativ zum
                                   EditWin des Containers */
    const Rectangle & rClip		/* Clipping des Containers auf das Objekt */
)
/*	[Beschreibung]

    Der Container "andert die Gr"osse oder Position des Objektes. Dies
    wird an das PlugIn gemeldet.

    [Querverweise]

    <SvInPlaceEnvironment::RectsChangedPixel()>
*/
{
    Reference< awt::XWindow > xWindow( pImpl->_xPlugin, UNO_QUERY );
    if (xWindow.is())
        xWindow->setPosSize( 0, 0, rObjRect.getWidth(), rObjRect.getHeight(), WINDOW_POSSIZE_SIZE );
    SvInPlaceEnvironment::RectsChangedPixel( rObjRect, rClip );
}

//=========================================================================
struct SvPlugInData_Impl
/*	[Beschreibung]

    In diesem struct werden sich Member von SvPlugInData gemerkt, um
    nicht immer inkompatibel zu werden.
*/
{
    BOOL					bRegisterFailed;	/* TRUE, wenn das registrieren
                                                    eines PlugIns nicht klappt */
                SvPlugInData_Impl()
                    : bRegisterFailed( FALSE )
                {}
};

//=========================================================================
//============== SvPlugInObject ===========================================
//=========================================================================
SO2_IMPL_BASIC_CLASS1_DLL( SvPlugInObject, SvFactory, SvInPlaceObject,
              SvGlobalName( SO3_PLUGIN_CLASSID ) )
//  			  SvGlobalName( 0xc1b5d281, 0x4870, 0x11d0,
//  							0x89, 0xca, 0x0, 0x80, 0x29, 0xe4, 0xb0, 0xb1) )


//=========================================================================
SvPlugInObject::SvPlugInObject()
    : pPlugInEnv( NULL )
    , pImpl( new SvPlugInData_Impl )
    , pURL( NULL )
    , nPlugInMode( PLUGIN_EMBEDED ) // muss noch ge"ndert werden
/*	[Beschreibung]

    Konstruktor der Klasse SvPlugInObject. Die Verbliste und das
    Format werden global initialisiert.
*/
{
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pPlugInVerbList )
    {
        pSoApp->pPlugInVerbList = new SvVerbList();
        // Alle unterstuetzten Verben anlegen
        pSoApp->pPlugInVerbList->Append(
                SvVerb( 0, String( SoResId( STR_VERB_OPEN ) ) ) );
        pSoApp->nPlugInDocFormat =
                SotExchange::RegisterFormatName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PlugIn Object" ) ) );
    }
    // Verben der Superklasse bekanntgeben, um GetVerbs nicht
    // ueberlagern zu muessen
    SetVerbList( pSoApp->pPlugInVerbList );
}

//=========================================================================
SvPlugInObject::~SvPlugInObject()
/*	[Beschreibung]

    Destruktor der Klasse SvPlugInObject.
*/
{
    delete pURL;
    delete pImpl;
}

BOOL SvPlugInObject::StartPlugIn( )
/*	[Beschreibung]

    Die Methode startet das PlugIn. Es ist notwendig, dass das Objekt
    im InPlace-Active ist.
*/
{
    SvPlugInEnvironment* pEnv = (SvPlugInEnvironment*)GetIPEnv();
    if( !pEnv )
        return FALSE;

    ULONG nCount = aCmdList.Count();
    Sequence< OUString > aCmds( nCount ), aArgs( nCount );
    OUString *pCmds = aCmds.getArray(), *pArgs = aArgs.getArray();
    for( ULONG i = 0; i < nCount; i++ )
    {
        SvCommand & rCmd = aCmdList.GetObject( i );
        pCmds[i] = rCmd.GetCommand();
        pArgs[i] = rCmd.GetArgument();
    }

    Reference< XMultiServiceFactory > xFac( ::utl::getProcessServiceFactory() );
    Reference< XPluginManager > xPMgr( xFac->createInstance( OUString::createFromAscii("com.sun.star.plugin.PluginManager") ), UNO_QUERY );
    if (! xPMgr.is() )
        ShowServiceNotAvailableError( NULL, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PluginManager" ) ), TRUE );

    SvInPlaceWindow * pBW = pEnv->GetBorderWin();
    DBG_ASSERT( pBW, "### no border window!" );

    INT16 nMode = (PlugInMode)nPlugInMode == PLUGIN_EMBEDED	? PluginMode::EMBED : PluginMode::FULL;
    if (xPMgr.is() && pBW)
    {
        Reference< XPlugin > xPlugin = xPMgr->createPluginFromURL(
            xPMgr->createPluginContext(), nMode, aCmds, aArgs, Reference< awt::XToolkit >(),
            Reference< awt::XWindowPeer >( pBW->GetComponentInterface() ),
            pURL ? pURL->GetMainURL( INetURLObject::NO_DECODE ) : rtl::OUString() );

        if( ! GetIPEnv() )
            return FALSE;
        // Environment can be discarded already by Application::Reschedule

        pEnv->pImpl->_xPlugin = xPlugin;

        if (pEnv->pImpl->_xPlugin.is())
        {
            pEnv->SetEditWin( pBW );
            // Fenster zuordnen (fuers Resize)
            pBW->SetObjWin( pBW );

            Reference< awt::XWindow > xWindow( pEnv->pImpl->_xPlugin, UNO_QUERY );
            if (xWindow.is())
            {
                Size aSize( pBW->GetSizePixel() );
                xWindow->setPosSize( 0, 0, aSize.getWidth(), aSize.getHeight(), WINDOW_POSSIZE_SIZE );
                xWindow->setVisible( TRUE );
            }
            if( ! pURL )
            {
                try
                {
                    Reference< awt::XControl > xControl( pEnv->pImpl->_xPlugin, UNO_QUERY );
                    if( xControl.is() )
                    {
                        Reference< awt::XControlModel > xModel = xControl->getModel();
                        Reference< XPropertySet > xProp( xModel, UNO_QUERY );
                        if( xProp.is() )
                        {
                            Any aValue = xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) );
                            OUString aVal;
                            aValue >>= aVal;
                            pURL = new INetURLObject( String( aVal ) );
                        }
                    }
                }
                catch(...)
                {
                }
            }
        }
    }

    return !pImpl->bRegisterFailed;
}


//=========================================================================
void SvPlugInObject::DataChanged_Impl
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
void SvPlugInObject::FillClass
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

    Da diese Information nicht vom PlugIn ermittelt werden kann, m"ussen
    Standardwerte angegeben werden. Dies bedeutet, von aussen gibt es
    nur den Typ PlugIn, in den man nicht hinein schauen kann.
*/
{
    *pClassName     = *GetSvFactory();
    *pFormat    	= SOAPP->nPlugInDocFormat;
    *pAppName		= String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PlugIn" ) );
    *pFullTypeName  = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PlugIn" ) );
    *pShortTypeName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PlugIn" ) );
}


//=========================================================================
void SvPlugInObject::Open
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
void SvPlugInObject::Embed
(
    BOOL bEmbed	/* TRUE, OutPlace Aktivierung beginnt.
                   FALSE, OutPlace Aktivierung endet. */
)
/*	[Beschreibung]

    Das Objekt wird OutPlace aktiviert oder deaktiviert. Es soll nicht
    aktiviert werden k"onnen. Die Aktivierung wird abgebrochen.

    [Querverweise]

    <SvEmbeddedObject::Embed()>
*/
{
    if( bEmbed )
        DoClose();
}

//=========================================================================
void SvPlugInObject::InPlaceActivate
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
        SvContainerEnvironment * pEnv;
        pEnv = GetIPClient()->GetEnv();
        // Wenn schon eines drin sitzt, ist es das des Sfx
        pPlugInEnv = (SvPlugInEnvironment*) GetIPEnv();
        if( !pPlugInEnv )
        {
            pPlugInEnv = new SvPlugInEnvironment( pEnv, this );
            // wird in die Verwaltung eingesetzt
            SetIPEnv( pPlugInEnv );
        }

        StartPlugIn();
    }

    SvInPlaceObject::InPlaceActivate( bActivate );

    if( !bActivate )
    {
        DELETEZ( pPlugInEnv );
        SetIPEnv( pPlugInEnv );
    }
}

//=========================================================================
ErrCode SvPlugInObject::Verb
(
    long 				nVerb,		/* welche Art des Aktivierens ist
                                       gew"unscht */
    SvEmbeddedClient *	pCl,		/* Callback-Svhnittstelle des Aufruffers */
    Window * 			pWin,		/* Parent auf dem aktiviert werden soll */
    const Rectangle * pWorkRectPixel/* Position und Gr"osse, an der das Objekt
                                       aktiviert werden soll */
)
/*	[Beschreibung]

    Es wird Versucht ein PlugIn zu starten. Es gibt nur die M"oglichkeit
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

    ErrCode nRet = ERRCODE_SO_NOT_INPLACEACTIVE;
    Reference< XMultiServiceFactory > xFac( ::utl::getProcessServiceFactory() );
    Sequence< OUString > aNames( xFac->getAvailableServiceNames() );
    const OUString * pNames = aNames.getConstArray();
    INT32 nPos;
    for ( nPos = aNames.getLength(); nPos--; )
    {
        if (pNames[nPos].compareToAscii("com.sun.star.plugin.PluginManager") == 0)
            break;
    }
    if (nPos < 0 || pImpl->bRegisterFailed)
        return nRet;

    switch( nVerb )
    {
        case SVVERB_SHOW:
        case SVVERB_IPACTIVATE:
        case 0L:
            if( PLUGIN_EMBEDED == ((PlugInMode)nPlugInMode) )
                nRet = GetProtocol().IPProtocol();
            else
                nRet = GetProtocol().UIProtocol();
            break;
        case SVVERB_HIDE:
            nRet = DoInPlaceActivate( FALSE );
            break;
    }
    return nRet;
}

//=========================================================================
void SvPlugInObject::SetVisArea
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
void SoPaintReplacement( const Rectangle &rRect, String &rText,
                         OutputDevice *pOut )
{
    MapMode aMM( MAP_APPFONT );
    Size aAppFontSz = pOut->LogicToLogic( Size( 0, 8 ), &aMM, NULL );
    Font aFnt( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica" ) ), aAppFontSz );
    aFnt.SetTransparent( TRUE );
    aFnt.SetColor( Color( COL_LIGHTRED ) );
    aFnt.SetWeight( WEIGHT_BOLD );
    aFnt.SetFamily( FAMILY_SWISS );

    pOut->Push();
    pOut->SetBackground();
    pOut->SetFont( aFnt );

    Point aPt;
    // Nun den Text so skalieren, dass er in das Rect passt.
    // Wir fangen mit der Defaultsize an und gehen 1-AppFont runter
    for( USHORT i = 8; i > 2; i-- )
    {
        aPt.X() = (rRect.GetWidth()  - pOut->GetTextWidth( rText )) / 2;
        aPt.Y() = (rRect.GetHeight() - pOut->GetTextHeight()) / 2;

        BOOL bTiny = FALSE;
        if( aPt.X() < 0 ) bTiny = TRUE, aPt.X() = 0;
        if( aPt.Y() < 0 ) bTiny = TRUE, aPt.Y() = 0;
        if( bTiny )
        {
            // heruntergehen bei kleinen Bildern
            aFnt.SetSize( Size( 0, aAppFontSz.Height() * i / 8 ) );
            pOut->SetFont( aFnt );
        }
        else
            break;
    }

    Bitmap aBmp( SoResId( BMP_PLUGIN ) );
    long nHeight = rRect.GetHeight() - pOut->GetTextHeight();
    long nWidth = rRect.GetWidth();
    if( nHeight > 0 )
    {
        aPt.Y() = nHeight;
        Point	aP = rRect.TopLeft();
        Size	aBmpSize = aBmp.GetSizePixel();
        // Bitmap einpassen
        if( nHeight * 10 / nWidth
          > aBmpSize.Height() * 10 / aBmpSize.Width() )
        {
            // nach der Breite ausrichten
            // Proportion beibehalten
            long nH = nWidth * aBmpSize.Height() / aBmpSize.Width();
            // zentrieren
            aP.Y() += (nHeight - nH) / 2;
            nHeight = nH;
        }
        else
        {
            // nach der H"ohe ausrichten
            // Proportion beibehalten
            long nW = nHeight * aBmpSize.Width() / aBmpSize.Height();
            // zentrieren
            aP.X() += (nWidth - nW) / 2;
            nWidth = nW;
        }
        pOut->DrawBitmap( aP, Size( nWidth, nHeight ), aBmp );
    }

    pOut->IntersectClipRegion( rRect );
    aPt += rRect.TopLeft();
    pOut->DrawText( aPt, rText );
    pOut->Pop();
}

//=========================================================================
void SvPlugInObject::Draw
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
    String aOut;
    if( pURL)
        aOut = pURL->GetMainURL(INetURLObject::DECODE_TO_IURI);
    SoPaintReplacement( aVisArea_, aOut, pDev );
}

//=========================================================================
BOOL SvPlugInObject::InitNew
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
        SetVisArea( Rectangle( Point(), Size( 10000, 10000 ) ) );
        return TRUE;
    }
    return FALSE;
}

//=========================================================================
#define DOCNAME "plugin"
#define PLUGIN_VERS	2

BOOL SvPlugInObject::Load
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
        if( nVer == 1 || nVer == PLUGIN_VERS )
        {
            *xStm >> nPlugInMode;
            // we only support embedding
            nPlugInMode = (USHORT)PLUGIN_EMBEDED;

            *xStm >> aCmdList;
            DBG_ASSERT( !pURL, "pURL exists in load" );
            BYTE bURLExist;
            *xStm >> bURLExist;
            if( bURLExist )
            {
                if( nVer == 1 )
                {
                    String aURL;
                    xStm->ReadByteString( aURL, RTL_TEXTENCODING_ASCII_US );
                    pURL = new INetURLObject( aURL );
                    // Ignore, not necessary
                    BOOL bStrict;
                    *xStm >> bStrict;
                }
                else
                {
                    String aURL;
                    xStm->ReadByteString( aURL, RTL_TEXTENCODING_ASCII_US );
                    pURL = new INetURLObject(
                        ::binfilter::StaticBaseUrl::RelToAbs( aURL ));
                }
            }

            String aMimeType;
            xStm->ReadByteString( aMimeType, RTL_TEXTENCODING_ASCII_US );
        }
        else
            xStm->SetError( ERRCODE_IO_WRONGVERSION );

        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
BOOL SvPlugInObject::Save()
/*	[Beschreibung]

    Der Inhalt des Objektes wird in den, in <SvPlugInObject::InitNew>
    oder <SvPlugInObject::Load> "ubergebenen Storage, geschrieben.

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

        *xStm << (BYTE)PLUGIN_VERS;
        *xStm << nPlugInMode;
        *xStm << aCmdList;
        if( pURL )
        {
            *xStm << (BYTE)TRUE;
            String aURL = pURL->GetMainURL( INetURLObject::NO_DECODE );
            if( aURL.Len() )
                aURL = ::binfilter::StaticBaseUrl::AbsToRel( aURL );
            xStm->WriteByteString( aURL, RTL_TEXTENCODING_ASCII_US );
        }
        else
            *xStm << (BYTE)FALSE;
        xStm->WriteByteString( GetMimeType(), RTL_TEXTENCODING_ASCII_US );
        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
BOOL SvPlugInObject::SaveAs
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
        xStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ),
                                STREAM_STD_WRITE | STREAM_TRUNC );
        xStm->SetVersion( pStor->GetVersion() );
        xStm->SetBufferSize( 8192 );

        *xStm << (BYTE)PLUGIN_VERS;
        *xStm << nPlugInMode;
        *xStm << aCmdList;
        if( pURL )
        {
            *xStm << (BYTE)TRUE;
            String aURL = pURL->GetMainURL( INetURLObject::NO_DECODE );
            if( aURL.Len() )
                aURL = ::binfilter::StaticBaseUrl::AbsToRel( aURL );
            xStm->WriteByteString( aURL, RTL_TEXTENCODING_ASCII_US );
        }
        else
            *xStm << (BYTE)FALSE;
        xStm->WriteByteString( GetMimeType(), RTL_TEXTENCODING_ASCII_US );

        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
void SvPlugInObject::HandsOff()
/*	[Beschreibung]

    Ab diesen Zeitpunkt, bis zum <SvPlugInObject::SaveCompleted>,
    darf auf den Storage im Objekt nicht zugegriffen werden.

    [Querverweise]

    <SvPersist::HandsOff>
*/
{
    SvInPlaceObject::HandsOff();
}

//=========================================================================
BOOL SvPlugInObject::SaveCompleted
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
ULONG SvPlugInObject::GetMiscStatus() const
/*	[Beschreibung]

    Da ein PlugIn immer aktiviert wird, es ein Link ist und er
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
    if( PLUGIN_EMBEDED == ((PlugInMode)nPlugInMode) )
    {
        nMisc |= SVOBJ_MISCSTATUS_INSIDEOUT;
        if( !pImpl->bRegisterFailed )
            nMisc |= SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE;
    }
    else if( PLUGIN_FULL == ((PlugInMode)nPlugInMode) )
    {
        nMisc |= SVOBJ_MISCSTATUS_INSIDEOUT;
    }

    nMisc |= SVOBJ_MISCSTATUS_SPECIALOBJECT;
    return nMisc;
}

//=========================================================================
BOOL SvPlugInObject::IsLink() const
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
void SvPlugInObject::SetCommandList
(
    const SvCommandList & rList	/* Die Liste der Kommnados */
)
/*	[Beschreibung]

    Die Liste der Kommandos, die "ubergeben werden, wenn das PlugIn
    gestartet wird.

    [Anmerkung]

    Ein bereits gestartetes PlugIn wird dadurch nicht beeinflusst.
    Die neuen Kommandos werden erst beim n"achsten starten ausgew"ahlt.
*/
{
    aCmdList = rList;

    DataChanged_Impl( TRUE );
}

//=========================================================================
void SvPlugInObject::SetURL
(
    const INetURLObject & rURL	/* Der Verweis auf die Quelle */
)
/*	[Beschreibung]

    Der Verweis auf die Quelle wird "uebergeben.

    [Anmerkung]

    Ein bereits gestartetes PlugIn wird dadurch nicht beeinflusst.
    Die neue Quelle wird erst beim n"achsten starten ausgew"ahlt.
*/
{
    if( !pURL )
    {
        pURL = new INetURLObject( rURL );
    }
    else if( *pURL != rURL )
    {
        *pURL = rURL;
        DataChanged_Impl( FALSE );
    }
}

//=========================================================================
void SvPlugInObject::SetMimeType
(
    const String & rMimeType
)
{
    (void)rMimeType;
}

//=========================================================================
const String & SvPlugInObject::GetMimeType() const
{
    static String aEmpty;
    SvPlugInEnvironment* pEnv = (SvPlugInEnvironment*)GetIPEnv();
    if( pEnv )
    {
        try
        {
            Reference< awt::XControl > xControl( pEnv->pImpl->_xPlugin, UNO_QUERY );
            if( xControl.is() )
            {
                Reference< awt::XControlModel > xModel = xControl->getModel();
                Reference< XPropertySet > xProp( xModel, UNO_QUERY );
                if( xProp.is() )
                {
                    Any aValue = xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TYPE" ) ) );
                    OUString aVal;
                    aValue >>= aVal;
                    pEnv->pImpl->_aMimeType = aVal;
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "got mimetype %s from plugin\n", OUStringToOString( pEnv->pImpl->_aMimeType, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                }
            }
        }
        catch(...)
        {
        }

        return pEnv->pImpl->_aMimeType;
    }
    return aEmpty;
}

//=========================================================================
void SvPlugInObject::SetPlugInMode
(
    USHORT nPlugIn	/* Der Modus in dem das PlugIn gestartet werden soll.
                       Um nicht inet.hxx zu includen. */
)
{
    (void)nPlugIn;
}


}
