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

#ifndef _IPENV_HXX
#define _IPENV_HXX

#include <bf_so3/client.hxx>
#ifndef _TOOLS_VCLRSC_HXX
#include <vcl/mapunit.hxx>
#endif

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

/*************************************************************************
*************************************************************************/
class  MenuBar;
class  Menu;
class  Accelerator;
class  KeyCode;
class  Window;
class  KeyEvent;
class  INetURLObject;
struct IOleInPlaceFrame;
struct IOleInPlaceUIWindow;

namespace binfilter {

class  SvInPlaceClient;
class  SvInPlaceObject;
class  SvContainerEnvironment;
class  SvInPlaceClipWindow;
class  SvInPlaceWindow;
class  SvContainerEnvironmentList;
//=========================================================================
class SO3_DLLPUBLIC SvAppFrame : public SvObject
{
friend class ImpInPlaceFrame;
friend class SvContainerEnvironment;
friend class SvInPlaceClient;
private:
    IOleInPlaceFrame *      	pObjI;
    SvContainerEnvironment *	pEnv;
protected:
    SO3_DLLPRIVATE 		~SvAppFrame();
public:
                        SO2_DECL_INTERFACE()
    IOleInPlaceFrame *	GetInPlaceFrame() const;
};
SV_DECL_IMPL_REF(SvAppFrame)

//=========================================================================
class SO3_DLLPUBLIC SvDocFrame : public SvObject
{
friend class ImpInPlaceUIWindow;
friend class SvContainerEnvironment;
friend class SvInPlaceClient;
private:
    IOleInPlaceUIWindow *   	pObjI;
    SvContainerEnvironment *	pEnv;
protected:
    SO3_DLLPRIVATE 		~SvDocFrame();
public:
                        SO2_DECL_INTERFACE()
    IOleInPlaceUIWindow * GetOleInPlaceUIWindow() const;
};
SV_DECL_IMPL_REF(SvDocFrame)

//=========================================================================
struct tagOIFI;
typedef struct tagOIFI OLEINPLACEFRAMEINFO;

class SO3_DLLPUBLIC SvContainerEnvironment : public SvClientData
{
friend class ImpInPlaceSite;
friend class ImpInPlaceFrame;
friend class ImpInPlaceUIWindow;
friend class SvInPlaceEnvironment;
friend class SvInPlaceClient;
friend class SvInPlaceObject;

private:
    SvInPlaceEnvironment *		pIPEnv; // IP-Env des Objektes
    SvInPlaceClient *			pObj;	// kann auch NULL sein
    SvContainerEnvironment *	pParent;// fuer IP in IP
    SvContainerEnvironmentList * pChildList; // fuer IP in IP
    WorkWindow *            	pTopWin;// Application window
    WorkWindow *            	pDocWin;// doc window != pTopWin
    SvAppFrameRef				xAppFrame;
    SvDocFrameRef				xDocFrame;
    OLEINPLACEFRAMEINFO *   	pOleInfo;

    // Variablen fuer die Menubearbeitung
    long                    hOleMenuDesc; // ist ein HOLEMENU
    USHORT                  nCount1, nCount2, nCount3;
    // Protokoll Stati
    USHORT            		nMenuUseCount;  //Menu in Benutzung
    Accelerator *           pAccel;     // Cont Accelerator
    Rectangle               aClipAreaPixel;

    // Variablen fuer Toolgroessen
    SvBorder        		aTopBorder;	// In Pixel
    SvBorder        		aDocBorder;	// In Pixel
    BOOL					bDfltUIAction:1,
                            bDeleteTopWin:1,
                            bDeleteDocWin:1,
                            bDeleteEditWin:1,
                            bDummy1;

    SO3_DLLPRIVATE void			SetIPEnv( SvInPlaceEnvironment * pEnv )
                    { pIPEnv = pEnv; }
    SO3_DLLPRIVATE void			ResetIPEnv()
                    { pIPEnv = NULL; }
    SO3_DLLPRIVATE void			MakeWinContext_Impl();
    //void			DeleteWindows_Impl();
protected:
    virtual void	ShowUIByChildDeactivate();

    SO3_DLLPRIVATE virtual MenuBar * QueryMenu( USHORT *, USHORT *, USHORT * );
    SO3_DLLPRIVATE virtual void    SetInPlaceMenu( MenuBar * pIPMenu, BOOL bSet );
    SO3_DLLPRIVATE virtual void    MenuReleased();
    SO3_DLLPRIVATE virtual void 	UIToolsShown( BOOL bShow );
    SO3_DLLPRIVATE void			RemoveDocWin() { pDocWin = NULL; }
    SO3_DLLPRIVATE void			RemoveEditWin() { SetEditWin( NULL ); }
    SO3_DLLPRIVATE BOOL			IsDefaultUIAction() const
                    { return bDfltUIAction; }
    SO3_DLLPRIVATE const SvBorder& GetTopBorder() const
                    { return aTopBorder; }

public:
                    TYPEINFO();
                    SvContainerEnvironment( SvInPlaceClient *,
                                            WorkWindow * pTopWin = NULL,
                                            WorkWindow * pDocWin = NULL,
                                            Window * pObjWin = NULL );
    virtual         ~SvContainerEnvironment();

    IOleInPlaceFrame *		GetInPlaceFrame() const;
    IOleInPlaceUIWindow *	GetOleInPlaceUIWindow() const;
    OLEINPLACEFRAMEINFO * 	GetOleInfo_Impl();

    SvContainerEnvironment* GetParent() const { return pParent; }
    SvContainerEnvironment* GetChild( ULONG n ) const;
    void 					ResetChilds();
    void					ResetChilds2IPActive();
    BOOL					IsChild( SvContainerEnvironment * pEnv ) const;
    SvInPlaceEnvironment * 	GetIPEnv() const { return pIPEnv; }

    BOOL			IsStub() const;
    SvInPlaceClient	* GetIPClient() const { return pObj; }
    void			SetDefaultUIAction( BOOL bOn )
                    { bDfltUIAction = bOn; }

    WorkWindow *    GetTopWin() const;
    WorkWindow *    GetDocWin() const;
    virtual Window* GetEditWin() const;

    // Sichtbarkeit
    virtual void	Scroll( const Size & rSize );


    // Accelerator Verwaltung
    virtual                 BOOL DispatchAccel( const KeyCode & );
    virtual Accelerator * 	GetAccel();

    // Groesse fuer Object-Window
    virtual void	SetSizeScale( const Fraction & rScaleWidth,
                                  const Fraction & rScaleHeight );
    void			OutDevScaleChanged();
    const Rectangle & GetClipAreaPixel() const
                    { return aClipAreaPixel; }
    virtual BOOL    SetObjArea( const Rectangle & );
    virtual Rectangle GetObjArea() const;
                    // Der Return ist in logischen Koordinaten des Objektes
    Rectangle		PixelObjVisAreaToLogic( const Rectangle & rObjRect ) const;
    virtual void    RequestObjAreaPixel( const Rectangle & rObjRect );

    // Groessen und Positionsverhandlungen fuer Tools
    void            SetTopToolFramePixel( const SvBorder & rBorder );
    Rectangle 		GetTopOuterRectPixel() const;

    void            SetDocToolFramePixel( const SvBorder & rBorder );
    Rectangle 		GetDocOuterRectPixel() const;

    // Das rInnerRect bezieht sich auf rOuterRect
    virtual BOOL    RequestTopToolSpacePixel( const SvBorder & );
    virtual BOOL    RequestDocToolSpacePixel( const SvBorder & );
    virtual BOOL    SetTopToolSpacePixel( const SvBorder & );
    virtual BOOL    SetDocToolSpacePixel( const SvBorder & );

    virtual BOOL    SetStatusText( const String & rText );
    virtual void 	ShowDocument( const INetURLObject &,
                                  const XubString & );
};
DECLARE_LIST(SvContainerEnvironmentList,SvContainerEnvironment*)

/*************************************************************************/
class SO3_DLLPUBLIC SvInPlaceEnvironment
{
friend class SvInPlaceObject;
friend class ImpInPlaceObject;
friend class SvInPlaceClient;
friend class SvContainerEnvironment;
friend class ImpOleInPlaceActiveObject;
friend class ImplSvEditObjectProtocol;

private:
    USHORT                  nChangeRectsLockCount;
    BOOL					bShowUITools:1,
                            bTopWinResize:1,
                            bDocWinResize:1,
                            bDeleteEditWin:1;
    // Controller
    MenuBar *      			pUIMenu;
    MenuBar *				pClientMenu;

    // View
    SvInPlaceClipWindow *   pClipWin;
    SvInPlaceWindow *       pBorderWin;
    Window *				pEditWin;
    Rectangle               aOldClipAreaPixel;
    Rectangle               aOldObjAreaPixel;

    SvContainerEnvironment *pContEnv;
    SvInPlaceObject *		pObj;

    SO3_DLLPRIVATE void 			MergeMenus();
    SO3_DLLPRIVATE void 			ReleaseClientMenu();
protected:
    SO3_DLLPRIVATE virtual void	TopWinResize();
    SO3_DLLPRIVATE virtual void	DocWinResize();

    SO3_DLLPRIVATE virtual MenuBar * QueryMenu( USHORT * pCount0, USHORT * pCount1,
                                 USHORT * pCount2 );
    SO3_DLLPRIVATE virtual void	ShowIPObj( BOOL bShow );
    SO3_DLLPRIVATE virtual void	ShowUITools( BOOL bShow );

    SO3_DLLPRIVATE virtual void 	RectsChangedPixel( const Rectangle & rObjRect,
                                        const Rectangle & rClip );

    SO3_DLLPRIVATE virtual 		BOOL DispatchAccel( const KeyCode & );

    SO3_DLLPRIVATE void			DeleteObjMenu();

    SO3_DLLPRIVATE void 			MakeWindows();
    SO3_DLLPRIVATE void			DeleteWindows();
    SO3_DLLPRIVATE void			SetClipWin( SvInPlaceClipWindow * pWin )
                    { pClipWin = pWin; }
    SO3_DLLPRIVATE void			SetBorderWin( SvInPlaceWindow * pWin )
                    { pBorderWin = pWin; }

public:
                    SvInPlaceEnvironment( SvContainerEnvironment *, SvInPlaceObject * );
    virtual			~SvInPlaceEnvironment();

    SvInPlaceObject *	GetIPObj() const { return pObj; }
    void			DoTopWinResize();
    void			DoDocWinResize();
    void			DoShowIPObj( BOOL bShow );
    void			DoShowUITools( BOOL bShow );
    BOOL			IsShowUITools() const { return bShowUITools; }

    const Rectangle & GetOldObjAreaPixel() const
                    { return aOldObjAreaPixel; }
    virtual void	QueryObjAreaPixel( Rectangle & rRect ) const;

    SvInPlaceClipWindow *	GetClipWin() const { return pClipWin; }
    SvInPlaceWindow * 		GetBorderWin() const { return pBorderWin; }
    void					SetEditWin( Window * pWin )
                            { pEditWin = pWin; }
    Window *         		GetEditWin();
    SvContainerEnvironment* GetContainerEnv() const { return pContEnv; }

    void            DoRectsChanged( BOOL bIfEqual = FALSE );
    USHORT          GetRectsChangedLockCount() const
                    {
                        return nChangeRectsLockCount;
                    }
    void            LockRectsChanged();
    void            UnlockRectsChanged();
};

//=========================================================================
//============== SvPlugInEnvironment ======================================
//=========================================================================
// since there is no more plugin manager from inet library
// we need PluginMode and PluginJavaRequester here
// #59963#
enum PlugInMode
{
    PLUGIN_EMBEDED		= 1,
    PLUGIN_FULL			= 2,
    PLUGIN_BACKGROUND	= 3
};

struct PlugJavaRequester
{
    virtual	void *	GetJavaPeer( Window * pPlugWin ) = 0;
};

class SvPlugInObject;
class SvPlugInEnvironment_Impl;
class SO3_DLLPUBLIC SvPlugInEnvironment : public SvInPlaceEnvironment
#ifdef SOLAR_JAVA
, PlugJavaRequester
#endif
{
friend class SvPlugInObject;

private:

    DECL_STATIC_LINK( SvPlugInEnvironment, MenuSelectHdl, Menu * );
    BOOL        bIsStarPlugIn;
    SvPlugInObject* pObj;
    SvPlugInEnvironment_Impl* pImpl;

protected:
    SO3_DLLPRIVATE virtual void	DocWinResize();
    SO3_DLLPRIVATE virtual void 	RectsChangedPixel( const Rectangle & rObjRect,
                                     const Rectangle & rClip );
public:
        SvPlugInEnvironment( SvContainerEnvironment *, SvPlugInObject * );
        ~SvPlugInEnvironment();

    BOOL            IsStarPlug() const { return bIsStarPlugIn; }

    virtual BOOL    MIMETypeDetected( const String& rMIME );
    virtual	void *	GetJavaPeer( Window * pPlugWin );
};

}

#endif // _IPENV_HXX
