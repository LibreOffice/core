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

#ifndef _CLIENT_HXX
#define _CLIENT_HXX

#include <bf_so3/protocol.hxx>
#include <bf_so3/iface.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

/*************************************************************************
*************************************************************************/
class Window;
class OutputDevice;
class ImpClientSite;
class ImpAdviseSink;
class ImpOleObject;
struct IOleClientSite;
struct IAdviseSink;

class INetURLObject;

namespace binfilter {

class SvEmbeddedObject;
class SvEmbeddedClient;

#ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
#define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
SO2_DECL_REF(SvEmbeddedObject)
#endif

class SO3_DLLPUBLIC SvClientData
{
friend class SvEmbeddedClient;
private:
    SvEmbeddedClient * pClient;
    Rectangle		aObjRect;	// Groesse des Objektes in logischen
                                // Koordinaten
    Fraction		aScaleWidth;// extra Skalierung fuer die Breite
    Fraction		aScaleHeight;// extra Skalierung fuer die Hoehe
    Window *        pEditWin;   // Ausgabefenster
    BOOL			bInvalidate;// nur wenn Window Initial uebergeben wird

protected:
    SO3_DLLPRIVATE void			SetEditWin( Window * pWin ) { pEditWin = pWin; }
public:
                    TYPEINFO();
                    SvClientData( SvEmbeddedClient *, Window * pWin = NULL );
    virtual 		~SvClientData();
    SvEmbeddedClient * GetClient() const { return pClient; }

    void			SetInvalidate( BOOL b ) { bInvalidate = b; }
    BOOL			IsInvalidate() const { return bInvalidate; }

    virtual void    Invalidate();
    virtual Window* GetEditWin() const;

    virtual void	SetSizeScale( const Fraction & rScaleWidth,
                                  const Fraction & rScaleHeight );
    const Fraction & GetScaleWidth() const { return aScaleWidth; }
    const Fraction & GetScaleHeight() const { return aScaleHeight; }

    virtual BOOL	SetObjArea( const Rectangle & );
    virtual Rectangle GetObjArea() const;
    Rectangle 		PixelObjAreaToLogic( const Rectangle & rRectPixel ) const;
    Rectangle 		LogicObjAreaToPixel( const Rectangle & rRect ) const;
    void            SetObjAreaPixel( const Rectangle & rRect )
                    {
                        SetObjArea( PixelObjAreaToLogic( rRect ) );
                    }
    Rectangle       GetObjAreaPixel() const
                    {
                        return LogicObjAreaToPixel( GetObjArea() );
                    }
};

class SO3_DLLPUBLIC SvAreaConstrain
{
private:
    Size	aMaxSize; //maximale Groesse des Gebietes
    Size	aMinSize; //minimale Groesse des Gebietes
    Size	aStepSize;//Schrittweite der Veraenderung
public:
            SvAreaConstrain() {};
            SvAreaConstrain( const Size & rMax,
                            const Size & rMin, const Size & rStep )
                : aMaxSize( rMax )
                , aMinSize( rMin )
                , aStepSize( rStep ) {};

    void	SetMaxSize( const Size & r ) { aMaxSize = r; }
    void	SetMinSize( const Size & r ) { aMinSize = r; }
    void	SetStepSize( const Size & r ) { aStepSize = r; }
    const Size & GetMaxSize() const { return aMaxSize; }
    const Size & GetMinSize() const { return aMinSize; }
    const Size & GetStepSize() const { return aStepSize; }
};

class SO3_DLLPUBLIC SvEmbeddedClient : public SvObject
{
friend class ImpClientSite;
friend class ImpAdviseSink;
friend class SvEmbeddedObject;
friend class SvInPlaceClient;
friend class SvEditObjectProtocol;
friend class ImplSvEditObjectProtocol;

private:
    SvClientData *  	pData;		// Wird auch in SvInPlaceClient modifiziert
    SvEmbeddedClient *	pParent;
    USHORT          	nAspect;
    BOOL            	bAutoForce:1,
                        bDeleteData:1,
                        bCanPlugIn;

    SO3_DLLPRIVATE void            SetStandardAdvises( SvEmbeddedObject * );
protected:
    SvEditObjectProtocol aProt;
                     ~SvEmbeddedClient();

    SO3_DLLPRIVATE void			SetViewData( SvClientData * pD )
                    {
                        pData = pD;
                    }
    SO3_DLLPRIVATE BOOL			HasViewData() const { return pData != NULL; }
    virtual void	MakeViewData();
    virtual void	FreeViewData( SvClientData * );

    SO3_DLLPRIVATE void			SetCanPlugIn( BOOL b )
                    { bCanPlugIn = b; }

                    // Benachrichtigungen
    virtual void    Connected( BOOL bConnect );
    virtual void    Closed();
    virtual void    Opened( BOOL bOpen );
    virtual void    Embedded( BOOL bEmbedding );
    virtual void    PlugIn( BOOL bPlugIn );
    virtual void    ViewChanged( USHORT nAspect );
public:
    Rectangle      	aDoVerbRectPixel;// Hack um Ole2 zu bezwingen

                    SvEmbeddedClient();
                    SO2_DECL_STANDARD_CLASS_DLL(SvEmbeddedClient,SOAPP)

    SvClientData *	GetClientData();

    void			SetParent( SvEmbeddedClient * pPar )
                    { pParent = pPar; }
    SvEmbeddedClient * GetParent() const { return pParent; }

    SvEmbeddedObject * GetEmbedObj() const
                    { return aProt.GetObj(); }
    void            DoDisconnect() { aProt.Reset(); }
    BOOL            IsEmbed() const { return aProt.IsEmbed(); }

                    // Befehle
    virtual BOOL    SaveObject();  // Objekt speichern
    virtual SvEmbeddedObjectRef  GetContainer();
    virtual void    MakeVisible();

    const SvEditObjectProtocol &  GetProtocol() const { return aProt; }
    SvEditObjectProtocol &  GetProtocol() { return aProt; }

    BOOL			CanPlugIn();
    BOOL            IsAutoForceMoniker() const { return bAutoForce; }
    void            SetAutoForceMoniker( BOOL bSet )
                    { bAutoForce = bSet; }
    virtual ErrCode	GetContURL( INetURLObject & );

    USHORT          GetAspect() const { return nAspect; }
};
#ifndef SO2_DECL_SVEMBEDDEDCLIENT_DEFINED
#define SO2_DECL_SVEMBEDDEDCLIENT_DEFINED
SO2_DECL_REF(SvEmbeddedClient)
#endif
SO2_IMPL_REF(SvEmbeddedClient)

}

#endif // _CLIENT_HXX
