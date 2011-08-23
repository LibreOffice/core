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


#ifndef _TENANT_H_
#define _TENANT_H_

#include <tools/prewin.h>
#include "soole.h"
#include <tools/postwin.h>
#include "bf_so3/outplace.hxx"

#define HIMETRIC_PER_INCH           2540
#define LOMETRIC_PER_INCH           254

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif  //PPVOID


namespace binfilter {

//CHAPTER17MOD

class CImpIOleClientSite : public IOleClientSite
    {
    protected:
        ULONG               m_cRef;
        class CSO_Cont      *m_pTen;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleClientSite(class CSO_Cont *, LPUNKNOWN);
        virtual ~CImpIOleClientSite(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP SaveObject(void);
        STDMETHODIMP GetMoniker(DWORD, DWORD, LPMONIKER *);
        STDMETHODIMP GetContainer(LPOLECONTAINER *);
        STDMETHODIMP ShowObject(void);
        STDMETHODIMP OnShowWindow(WIN_BOOL);
        STDMETHODIMP RequestNewObjectLayout(void);
    };

typedef CImpIOleClientSite *PCImpIOleClientSite;



class CImpIAdviseSink : public IAdviseSink
    {
    protected:
        ULONG               m_cRef;
        class CSO_Cont      *m_pTen;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIAdviseSink(class CSO_Cont *, LPUNKNOWN);
        virtual ~CImpIAdviseSink(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
        STDMETHODIMP_(void)  OnRename(LPMONIKER);
        STDMETHODIMP_(void)  OnSave(void);
        STDMETHODIMP_(void)  OnClose(void);
    };


typedef CImpIAdviseSink *PCImpIAdviseSink;

//End CHAPTER17MOD


/*
 * SO_Cont class describing an individual piece of data in a page.
 * It knows where it sits, what object is inside of it, and what
 * its idenitifer is such that it can find it's storage within a
 * page.
 */

//Patron Objects clipboard format
typedef struct tagPATRONOBJECT
    {
    POINTL      ptl;        //Location of object
    POINTL      ptlPick;    //Pick point from drag-drop operation
    SIZEL       szl;        //Extents of object (absolute)
    FORMATETC   fe;         //Actual object format
    } PATRONOBJECT, *PPATRONOBJECT;



//Values for hit-testing, drawing, and resize-tracking tenants
#define CXYHANDLE       5

//SO_Cont types (not persistent, but determined at load time)
typedef enum
    {
    TENANTTYPE_NULL=0,
    TENANTTYPE_STATIC,
    //CHAPTER17MOD
    TENANTTYPE_EMBEDDEDOBJECT,
    TENANTTYPE_EMBEDDEDFILE,
    TENANTTYPE_EMBEDDEDOBJECTFROMDATA
    //End CHAPTER17MOD
    } TENANTTYPE, *PTENANTTYPE;


//State flags
#define TENANTSTATE_DEFAULT      0x00000000
#define TENANTSTATE_SELECTED     0x00000001

//CHAPTER17MOD
#define TENANTSTATE_OPEN         0x00000002
//End CHAPTER17MOD


/*
 * Persistent information we need to save for each tenant, which is
 * done in the tenant list instead of with each tenant.  Since this
 * is a small structure it's best not to blow another stream for it
 * (overhead).
 */
/*
#pragma pack( push, 8 )
typedef struct tagTENANTINFO
    {
    DWORD       dwID;
    RECTL       rcl;
    FORMATETC   fe;             //Excludes ptd
    short       fSetExtent;     //Call IOleObject::SetExtent on Run
    } TENANTINFO, *PTENANTINFO;
#pragma pack( pop, 8 )
*/

class CSO_Cont : public IUnknown
    {
    //CHAPTER17MOD
    friend class CImpIOleClientSite;
    friend class CImpIAdviseSink;
    //End CHAPTER17MOD

    private:
        HWND            m_hWnd;         //Pages window
        DWORD           m_dwID;         //Persistent DWORD ID
        DWORD           m_cOpens;       //Count calls to Open

        WIN_BOOL        m_fInitialized; //Something here?
        LPUNKNOWN       m_pObj;         //The object here
        LPSTORAGE       m_pIStorage;    //Sub-storage for tenant
        ILockBytes *	pTmpStorage;	// temporary storage

        DWORD			dwAspect;		// new MM
        //FORMATETC       m_fe;           //Used to create the object
        DWORD           m_dwState;      //State flags
        RECTL           m_rcl;          //Space of this object
        CLSID           m_clsID;        //Object class (for statics)
        WIN_BOOL        m_fSetExtent;   //Call SetExtent on next run

        class SvOutPlaceObject *m_pPG;          //Pages window
//        class CPages   *m_pPG;          //Pages window

        //CHAPTER17MOD
        TENANTTYPE      m_tType;            //Type identifier
        ULONG           m_cRef;             //We're an object now
        LPOLEOBJECT     m_pIOleObject;      //IOleObject on m_pObj
        LPVIEWOBJECT2   m_pIViewObject2;    //IViewObject2 on m_pObj
        ULONG           m_grfMisc;          //OLEMISC flags
        WIN_BOOL            m_fRepaintEnabled;  //No redundant paints

        //Our interfaces
        PCImpIOleClientSite m_pImpIOleClientSite;
        PCImpIAdviseSink    m_pImpIAdviseSink;
        //End CHAPTER17MOD

    protected:
        //CHAPTER17MOD
        WIN_BOOL    ObjectInitialize(LPUNKNOWN, DWORD, DWORD);
        //WIN_BOOL    ObjectInitialize(LPUNKNOWN, LPFORMATETC, DWORD);
        //End CHAPTER17MOD
        HRESULT CreateStatic(LPDATAOBJECT, LPFORMATETC
            , LPUNKNOWN *);

    public:
        CSO_Cont(DWORD, HWND, SvOutPlaceObject *);
//        CSO_Cont(DWORD, HWND, CPages *);
        virtual ~CSO_Cont(void);

        //CHAPTER17MOD
        //Gotta have an IUnknown for delegation
        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
        //End CHAPTER17MOD

        LPOLEOBJECT GetOleObj() const { return m_pIOleObject; }
        CLSID		GetCLSID();
        DWORD       GetID(void);
        String      GetStorageName();
        //UINT        GetStorageName(LPOLESTR);
        //CHAPTER17MOD
        //void        StorageGet(LPSTORAGE *);
        //End CHAPTER17MOD
        UINT        Create(TENANTTYPE, LPVOID, LPFORMATETC, PPOINTL
                        , LPSIZEL, SotStorage *, PPATRONOBJECT, DWORD);
//      UINT        Create(TENANTTYPE, LPVOID, LPFORMATETC, PPOINTL
//                      , LPSIZEL, LPSTORAGE, PPATRONOBJECT, DWORD);
        WIN_BOOL    Save(SotStorage * );
        WIN_BOOL    Load(SotStorage *, DWORD dwAspect, WIN_BOOL fSetExtend, RECTL & rcl );
//      WIN_BOOL    Load(LPSTORAGE, PTENANTINFO);
        void        GetInfo(DWORD& rAspect, WIN_BOOL & rSetExtend );
//        void        GetInfo(PTENANTINFO);
        WIN_BOOL    Open(SotStorage *);
//      WIN_BOOL    Open(LPSTORAGE);
        void        Close(WIN_BOOL);
        WIN_BOOL    Update(void);
        void        Destroy(SotStorage *);
//        void        Destroy(LPSTORAGE);

        void        Select(WIN_BOOL);
        //CHAPTER17MOD
        void        ShowAsOpen(WIN_BOOL);
        void        ShowYourself(void);
        void        AddVerbMenu(HMENU, UINT);
        TENANTTYPE  TypeGet(void);
        void        CopyEmbeddedObject(LPDATAOBJECT, LPFORMATETC
                        , PPOINTL);
        void        NotifyOfRename(LPTSTR, LPVOID);
        //End CHAPTER17MOD

        WIN_BOOL    Activate(LONG, HWND);
        void        Draw(HDC, DVTARGETDEVICE *, HDC, int, int
                        , WIN_BOOL, WIN_BOOL);
        void        Repaint(void);
        void        Invalidate(void);

        //CHAPTER17MOD
        void        ObjectClassFormatAndIcon(LPCLSID, LPWORD
                        , LPTSTR *, HGLOBAL *, LPTSTR *);
        WIN_BOOL        SwitchOrUpdateAspect(HGLOBAL, WIN_BOOL);
        void        EnableRepaint(WIN_BOOL);
        //End CHAPTER17MOD

        void        ObjectGet(LPUNKNOWN *);
        void        FormatEtcGet(LPFORMATETC, WIN_BOOL);
        WIN_BOOL    GetExtent(LPSIZEL);
        void        SizeGet(LPSIZEL, WIN_BOOL);
        //CHAPTER17MOD
        void        SizeSet(LPSIZEL, WIN_BOOL, WIN_BOOL);
        //End CHAPTER17MOD
        void        RectGet(LPRECTL, WIN_BOOL);
        //CHAPTER17MOD
        void        RectSet(LPRECTL, WIN_BOOL, WIN_BOOL);
        //End CHAPTER17MOD
        BOOL 		GetMetaFile( long& nMapMode, Size& rSize, HMETAFILE& hMet );
    };


typedef CSO_Cont *PCSO_Cont;

//Return codes for Create
#define CREATE_FAILED               0
#define CREATE_GRAPHICONLY          1
#define CREATE_PLACEDOBJECT         2

}


#endif //_TENANT_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
