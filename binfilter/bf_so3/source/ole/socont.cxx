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

#include "socont.h"
#include <bf_so3/svstor.hxx>


#define LOMETRIC_PER_INCH   254

#define HIMETRIC_PER_INCH   2540    //Number HIMETRIC units per inch
#define PTS_PER_INCH        72      //Number points (font size) per inch

#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

namespace binfilter {

void SO_MetricRectInPixelsToHimetric( LPRECT prcPix, LPRECT prcHiMetric)
    {
    int     iXppli;     //Pixels per logical inch along width
    int     iYppli;     //Pixels per logical inch along height
    int     iXextInPix=(prcPix->right-prcPix->left);
    int     iYextInPix=(prcPix->bottom-prcPix->top);
    BOOL    fSystemDC=FALSE;

    HDC hDC = GetDC(NULL);
    fSystemDC=TRUE;

    iXppli=GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli=GetDeviceCaps (hDC, LOGPIXELSY);

    //We got pixel units, convert them to logical HIMETRIC along the display
    prcHiMetric->right=MAP_PIX_TO_LOGHIM(iXextInPix, iXppli);
    prcHiMetric->top  =MAP_PIX_TO_LOGHIM(iYextInPix, iYppli);

    prcHiMetric->left   =0;
    prcHiMetric->bottom =0;

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }

void SO_MetricSizeInPixelsToHimetric( LPSIZEL pSizeInPix, LPSIZEL pSizeInHiMetric)
    {
    int     iXppli;     //Pixels per logical inch along width
    int     iYppli;     //Pixels per logical inch along height
    BOOL    fSystemDC=FALSE;

    HDC hDC=GetDC(NULL);
    fSystemDC=TRUE;

    iXppli=GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli=GetDeviceCaps (hDC, LOGPIXELSY);

    //We got pixel units, convert them to logical HIMETRIC along the display
    pSizeInHiMetric->cx=(long)MAP_PIX_TO_LOGHIM((int)pSizeInPix->cx, iXppli);
    pSizeInHiMetric->cy=(long)MAP_PIX_TO_LOGHIM((int)pSizeInPix->cy, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }

/*
 * RectConvertMappings
 *
 * Purpose:
 *  Converts the contents of a rectangle from device to logical
 *  coordinates where the hDC defines the logical coordinates.
 *
 * Parameters:
 *  pRect           LPRECT containing the rectangle to convert.
 *  hDC             HDC describing the logical coordinate system.
 *                  if NULL, uses a screen DC in MM_LOMETRIC.
 *  fToDevice       WIN_BOOL TRUE to convert from uConv to device,
 *                  FALSE to convert device to uConv.
 *
 * Return Value:
 *  None
 */

void RectConvertMappings(LPRECT pRect, HDC hDC, WIN_BOOL fToDevice)
    {
    POINT   rgpt[2];
    WIN_BOOL    fSysDC=FALSE;

    if (NULL==pRect)
        return;

    rgpt[0].x=pRect->left;
    rgpt[0].y=pRect->top;
    rgpt[1].x=pRect->right;
    rgpt[1].y=pRect->bottom;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        SetMapMode(hDC, MM_HIMETRIC);
        //SetMapMode(hDC, MM_LOMETRIC);
        fSysDC=TRUE;
        }

    if (fToDevice)
        LPtoDP(hDC, rgpt, 2);
    else
        DPtoLP(hDC, rgpt, 2);

    if (fSysDC)
        ReleaseDC(NULL, hDC);

    pRect->left=rgpt[0].x;
    pRect->top=rgpt[0].y;
    pRect->right=rgpt[1].x;
    pRect->bottom=rgpt[1].y;

    return;
    }

/*
 * CSO_Cont::CSO_Cont
 * CSO_Cont::~CSO_Cont
 *
 * Constructor Parameters:
 *  dwID            DWORD identifier for this page.
 *  hWnd            HWND of the pages window.
 *  pPG             PCPages to the parent structure.
 */

CSO_Cont::CSO_Cont(DWORD dwID, HWND hWnd, SvOutPlaceObject * pPG)
//CSO_Cont::CSO_Cont(DWORD dwID, HWND hWnd, PCPages pPG)
    {
    m_hWnd=hWnd;
    m_dwID=dwID;

    m_fInitialized=0;
    m_pIStorage=NULL;
    pTmpStorage=NULL;
    m_cOpens=0;

    m_pObj=NULL;
    m_pPG =pPG;
    m_clsID=CLSID_NULL;
    m_fSetExtent=FALSE;

    //CHAPTER17MOD
    m_cRef=0;
    m_pIOleObject=NULL;
    m_pIViewObject2=NULL;
    m_grfMisc=0;

    m_pImpIOleClientSite=NULL;
    m_pImpIAdviseSink=NULL;

    m_fRepaintEnabled=TRUE;
    //End CHAPTER17MOD
    return;
    }


CSO_Cont::~CSO_Cont(void)
    {
    //CHAPTER17MOD
    //Object pointers cleaned up in Close.

    DeleteInterfaceImp(m_pImpIAdviseSink);
    DeleteInterfaceImp(m_pImpIOleClientSite);
    //End CHAPTER17MOD

    return;
    }




//CHAPTER17MOD
/*
 * CSO_Cont::QueryInterface
 * CSO_Cont::AddRef
 * CSO_Cont::Release
 *
 * Purpose:
 *  IUnknown members for CSO_Cont object.
 */

STDMETHODIMP CSO_Cont::QueryInterface(REFIID riid, PPVOID ppv)
    {
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv=this;

    if (IID_IOleClientSite==riid)
        *ppv=m_pImpIOleClientSite;

    if (IID_IAdviseSink==riid)
        *ppv=m_pImpIAdviseSink;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CSO_Cont::AddRef(void)
    {
    return ++m_cRef;
    }

STDMETHODIMP_(ULONG) CSO_Cont::Release(void)
    {
    if (0!=--m_cRef)
        return m_cRef;

    delete this;
    return 0;
    }

//End CHAPTER17MOD





/*
 * CSO_Cont::GetID
 *
 * Return Value:
 *  DWORD           dwID field in this tenant.
 */

DWORD CSO_Cont::GetID(void)
    {
    return m_dwID;
    }



/*
 * CSO_Cont::GetStorageName
 *
 * Parameters:
 *  pszName         LPOLESTR to a buffer in which to store the storage
 *                  name for this tenant.
 *
 * Return Value:
 *  UINT            Number of characters stored.
 */

//UINT CSO_Cont::GetStorageName(LPOLESTR pszName)
String CSO_Cont::GetStorageName()
{
    // used in SvOutPlaceObject too, don't change name
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) );
}



//CHAPTER17MOD
/*
 * CSO_Cont::StorageGet
 *
 * Purpose:
 *  Returns the IStorage pointer maintained by this tenant,
 *  AddRef'd of course.
 *
 * Parameters:
 *  ppStg           LPSTORAGE * in which to return the pointer.
 *
 * Return Value:
 *  None
 */

/* MM ???
void CSO_Cont::StorageGet(LPSTORAGE *ppStg)
    {
    if (NULL==ppStg)
        return;

    *ppStg=m_pIStorage;

    if (NULL!=*ppStg)
        (*ppStg)->AddRef();

    return;
    }
*/
//End CHAPTER17MOD




/*
 * CSO_Cont::Create
 *
 * Purpose:
 *  Creates a new tenant of the given CLSID, which can be either a
 *  static bitmap or metafile or any compound document object.
 *
 * Parameters:
 *  tType           TENANTTYPE to create, either a static metafile,
 *                  bitmap, or some kind of compound document object
 *                  This determines which OleCreate* call we use.
 *  pvType          LPVOID providing the relevant pointer from which
 *                  to create the tenant, depending on iType.
 *  pFE             LPFORMATETC specifying the type of renderings
 *                  to use.
 *  pptl            PPOINTL in which we store offset coordinates.
 *  pszl            LPSIZEL where this object should store its
 *                  lometric extents.
 *  pIStorage       LPSTORAGE of the page we live in.  We have to
 *                  create another storage in this for the tenant.
 *  ppo             PPATRONOBJECT containing placement data.
 *  dwData          DWORD with extra data, sensitive to iType.
 *
 * Return Value:
 *  UINT            A CREATE_* value depending on what we
 *                  actually do.
 */

UINT CSO_Cont::Create(TENANTTYPE tType, LPVOID pvType
    , LPFORMATETC pFE, PPOINTL pptl, LPSIZEL pszl
    , SotStorage * pIStorage, PPATRONOBJECT ppo, DWORD dwData)
//    , LPSTORAGE pIStorage, PPATRONOBJECT ppo, DWORD dwData)
    {
    HRESULT             hr;
    LPUNKNOWN           pObj = NULL;
    UINT                uRet=CREATE_GRAPHICONLY;
    //CHAPTER17MOD
    //Some things moves to ObjectInitialize
    //End CHAPTER17MOD

    if (NULL==pvType || NULL==pIStorage)
        return CREATE_FAILED;

    //Fail if this is called for an already living tenant.
    if (m_fInitialized)
        return CREATE_FAILED;

    m_fInitialized=TRUE;

    //Create a new storage for this tenant.
    if (!Open(pIStorage))
        return CREATE_FAILED;

    /*
     * Get the placement info if it's here.  We either have a non-
     * NULL PPATRONOBJECT in ppo or we have to use default
     * placement and retrieve the size from the object itself.
     */
    pszl->cx=0;
    pszl->cy=0;

    if (NULL!=ppo)
        {
        *pFE=ppo->fe;
        *pptl=ppo->ptl;
        *pszl=ppo->szl;     //Could be 0,0 , so we ask object

        uRet=CREATE_PLACEDOBJECT;
        }

    hr=ResultFromScode(E_FAIL);

    //Now create an object based specifically for the type.
    switch (tType)
        {
        case TENANTTYPE_NULL:
            break;

        case TENANTTYPE_STATIC:
            /*
             * We could use OleCreateStaticFromData here which does
             * pretty much what we're doing below.  However, it does
             * not allow us to control whether we paste a bitmap or
             * a metafile--it uses metafile first, bitmap second.
             * For this reason we'll use code developed in Chapter
             * 11's FreeLoader to affect the paste.
             */
            hr=CreateStatic((LPDATAOBJECT)pvType, pFE, &pObj);
            break;

        //CHAPTER17MOD
        case TENANTTYPE_EMBEDDEDOBJECT:
            hr=OleCreate(*((LPCLSID)pvType), IID_IUnknown
                , OLERENDER_DRAW, NULL, NULL, m_pIStorage
                , (PPVOID)&pObj);
            break;

        case TENANTTYPE_EMBEDDEDFILE:
            hr=StarObject_OleCreateFromFile(CLSID_NULL, (LPTSTR)pvType
                , IID_IUnknown, OLERENDER_DRAW, NULL, NULL
                , m_pIStorage, (PPVOID)&pObj);
            break;

        case TENANTTYPE_EMBEDDEDOBJECTFROMDATA:
            hr=OleCreateFromData((LPDATAOBJECT)pvType, IID_IUnknown
                , OLERENDER_DRAW, NULL, NULL, m_pIStorage
                , (PPVOID)&pObj);
            break;
        //End CHAPTER17MOD

        default:
            break;
        }

    //If creation didn't work, get rid of the element Open created.
    if (FAILED(hr))
        {
        Destroy(pIStorage);
        return CREATE_FAILED;
        }

    //CHAPTER17MOD
    //We don't get the size if PatronObject data was seen already.
    if (!ObjectInitialize(pObj, pFE->dwAspect, dwData))
    //if (!ObjectInitialize(pObj, pFE, dwData))
        {
        Destroy(pIStorage);
        return CREATE_FAILED;
        }

    if (0==pszl->cx && 0==pszl->cy)
        {
        GetExtent( pszl );
        }
    //End CHAPTER17MOD

    return uRet;
    }



/*
 * CSO_Cont::Save
 *
 * Purpose:
 *
 * Parameters:
 *  pIStorage       LPSTORAGE of the page we live in.
 *
 * Return Value:
 *  WIN_BOOL            TRUE if successful, FALSE otherwise.
 */

WIN_BOOL CSO_Cont::Save(SotStorage *pIStorage)
    {
    LPPERSISTSTORAGE    pIPS;

    if (NULL!=m_pIStorage)
        {
        /*
         * We need to OleSave again because we might have changed
         * the size or position of this tenant.  We also need to
         * save the rectangle on the page, since that's not known
         * to OLE.
         */
        m_pObj->QueryInterface(IID_IPersistStorage, (PPVOID)&pIPS);

        //This fails for static objects...so we improvise if so
        if (FAILED(OleSave(pIPS, m_pIStorage, TRUE)))
            {
            //This is essentially what OleSave does.
            WriteClassStg(m_pIStorage, m_clsID);
            pIPS->Save(m_pIStorage, TRUE);
            }

        pIPS->SaveCompleted(NULL);
        pIPS->Release();
        m_pIStorage->Commit(STGC_DEFAULT);

        // copy to SotStream
        // Create the stream
        String szTemp = GetStorageName();
        SotStorageStreamRef xStm = pIStorage->OpenSotStream( szTemp, STREAM_STD_READWRITE );
        if( xStm->GetError() )
            return FALSE;
        //We know we only hold one ref from Create or Load
        HGLOBAL hGlobal = NULL;
        GetHGlobalFromILockBytes(
            pTmpStorage, 	//Pointer to the byte array object
            &hGlobal );//Pointer to the current memory handle for the specified byte array
        if( !hGlobal )
            return FALSE;
        void * pMem = GlobalLock( hGlobal );
        if( !pMem )
            return FALSE;
        xStm->SetSize( 0 );
        ULONG nMaxLen = GlobalSize( hGlobal );
        xStm->Write( pMem, nMaxLen );
//SvFileStream aFS( "d:\\temp\\test.ttt", STREAM_STD_READWRITE );
//aFS.Write( pMem, nMaxLen );
        GlobalUnlock( hGlobal );

        return TRUE;
        }

    return FALSE;
    }



/*
 * CSO_Cont::Load
 *
 * Purpose:
 *  Recreates the object living in this tenant in place of calling
 *  FCreate.  This is used in loading as opposed to new creation.
 *
 * Parameters:
 *  pIStorage       LPSTORAGE of the page we live in.
 *  pti             PTENTANTINFO containing persistent information.
 *                  The ID value in this structure is ignored.
 *
 * Return Value:
 *  WIN_BOOL            TRUE if successful, FALSE otherwise.
 */

//WIN_BOOL CSO_Cont::Load(LPSTORAGE pIStorage, PTENANTINFO pti)
WIN_BOOL CSO_Cont::Load(SotStorage *pIStorage, DWORD dwAspect, WIN_BOOL fSetExtent, RECTL & rcl )
    {
    HRESULT         hr;
    LPUNKNOWN       pObj;
    //CHAPTER17MOD
    DWORD           dwState=TENANTSTATE_DEFAULT;
    //End CHAPTER17MOD

    /*
    if (NULL==pIStorage || NULL==pti)
        return FALSE;
    */

    //CHAPTER17MOD
    /*
     * If we already initialized once, clean up, releasing
     * everything before we attempt to reload.  This happens
     * when using the Convert Dialog.
     */
    if (m_fInitialized)
        {
        //Preserve all states except open
        dwState=(m_dwState & ~TENANTSTATE_OPEN);
        m_cRef++;   //Prevent accidental closure

        //This should release all holds on our IStorage as well.
        if (NULL!=m_pIViewObject2)
            {
            m_pIViewObject2->SetAdvise(dwAspect, 0, NULL);
//            m_pIViewObject2->SetAdvise(m_fe.dwAspect, 0, NULL);
            ReleaseInterface(m_pIViewObject2);
            }

        ReleaseInterface(m_pIOleObject);
        ReleaseInterface(m_pObj);

        m_pIStorage=NULL;   //We'll have already released this.
        pTmpStorage=NULL;   //We'll have already released this.
        m_cRef--;           //Match safety increment above.
        }
    //End CHAPTER14MOD

    m_fInitialized=TRUE;

    //Open the storage for this tenant.
    if (!Open(pIStorage))
        return FALSE;

    hr=OleLoad(m_pIStorage, IID_IUnknown, NULL, (PPVOID)&pObj);

    if (FAILED(hr))
        {
        Destroy(pIStorage);
        return FALSE;
        }

    //CHAPTER17MOD
    m_fSetExtent=fSetExtent;
    //m_fSetExtent=pti->fSetExtent;
    ObjectInitialize(pObj, dwAspect, NULL);
    //ObjectInitialize(pObj, &pti->fe, NULL);

    //Restore the original state before reloading.
    m_dwState=dwState;
    //End CHAPTER17MOD

    RectSet(&rcl, FALSE, FALSE);
    //RectSet(&pti->rcl, FALSE, FALSE);
    return TRUE;
    }




/*
 * CSO_Cont::GetInfo
 *
 * Purpose:
 *  Retrieved a TENANTINFO structure for this tenant.
 *
 * Parameters:
 *  pti             PTENANTINFO structure to fill
 *
 * Return Value:
 *  None
 */

//void CSO_Cont::GetInfo(PTENANTINFO pti)
void CSO_Cont::GetInfo(DWORD& rAspect, WIN_BOOL & rSetExtend )
{
    rAspect = dwAspect;
    rSetExtend = m_fSetExtent;
    /*
    if (NULL!=pti)
        {
        pti->dwID=m_dwID;
        pti->rcl=m_rcl;
        pti->fe=m_fe;
        pti->fSetExtent=m_fSetExtent;
        }

    return;
    */
}




//CHAPTER17MOD
/*
 * CSO_Cont::ObjectInitialize
 * (Protected)
 *
 * Purpose:
 *  Performs operations necessary after creating an object or
 *  reloading one from storage.
 *
 * Parameters:
 *  pObj            LPUNKNOWN of the object in this tenant.
 *  pFE             LPFORMATETC describing the graphic here.
 *  dwData          DWORD extra data.  If pFE->dwAspect==
 *                  DVASPECT_ICON then this is the iconic metafile.
 *
 * Return Value:
 *  WIN_BOOL            TRUE if the function succeeded, FALSE otherwise.
 */

//WIN_BOOL CSO_Cont::ObjectInitialize(LPUNKNOWN pObj, LPFORMATETC pFE, DWORD dwData)
WIN_BOOL CSO_Cont::ObjectInitialize(LPUNKNOWN pObj, DWORD dwAspect_, DWORD dwData)
    {
    HRESULT         hr;
    LPPERSIST       pIPersist=NULL;
    DWORD           dw;
    //PCDocument      pDoc;
    TCHAR           szFile[CCHPATHMAX];

    //if (NULL==pObj || NULL==pFE)
    //    return FALSE;

    m_pObj=pObj;
    dwAspect = dwAspect_;
    //m_fe=*pFE;
    //m_fe.ptd=NULL;
    m_dwState=TENANTSTATE_DEFAULT;

    /*
     * Determine the type:  Static or Embedded.  If Static,
     * this will have CLSID_Picture_Metafile or CLSID_Picture_Dib.
     * Otherwise it's embedded.  Later we'll add a case for links.
     */
    m_tType=TENANTTYPE_EMBEDDEDOBJECT;

    if (SUCCEEDED(pObj->QueryInterface(IID_IPersist
        , (PPVOID)&pIPersist)))
        {
        CLSID   clsid=CLSID_NULL;

        hr=pIPersist->GetClassID(&clsid);

        //If we don't have a CLSID, default to static
        if (FAILED(hr) || CLSID_Picture_Metafile==clsid
            || CLSID_Picture_Dib==clsid)
            m_tType=TENANTTYPE_STATIC;

        pIPersist->Release();
        }


    m_pIViewObject2=NULL;
    hr=pObj->QueryInterface(IID_IViewObject2
        , (PPVOID)&m_pIViewObject2);

    if (FAILED(hr))
        return FALSE;

    m_pIViewObject2->SetAdvise(dwAspect, 0, m_pImpIAdviseSink);
    //m_pIViewObject2->SetAdvise(m_fe.dwAspect, 0, m_pImpIAdviseSink);

    IOleCache* pIOleCache = NULL;
    if ( SUCCEEDED(m_pIViewObject2->QueryInterface(IID_IOleCache, (PPVOID)&pIOleCache)) && pIOleCache )
    {
        IEnumSTATDATA* pEnumSD = NULL;
        HRESULT hr = pIOleCache->EnumCache( &pEnumSD );

        sal_Bool bRegister = sal_True;
        if ( SUCCEEDED( hr ) && pEnumSD )
        {
            pEnumSD->Reset();
            STATDATA aSD;
            DWORD nNum;
            while( SUCCEEDED( pEnumSD->Next( 1, &aSD, &nNum ) ) && nNum == 1 )
            {
                if ( aSD.formatetc.cfFormat == 0 )
                {
                    bRegister = sal_False;
                    break;
                }
            }
        }

        if ( bRegister )
        {
            DWORD nConn;
            FORMATETC aFormat = { 0, 0, ASPECT_CONTENT, -1, TYMED_MFPICT };
            hr = pIOleCache->Cache( &aFormat, /*ADVFCACHE_FORCEBUILTIN*/ /*ADVF_PRIMEFIRST*/ ADVFCACHE_ONSAVE, &nConn );
/*
            IDataObject* pIDataObject = NULL;
            if ( SUCCEEDED(pObj->QueryInterface(IID_IDataObject, (PPVOID)&pIDataObject)) && pIDataObject )
            {
                pIOleCache->InitCache( pIDataObject );
                pIDataObject->Release();
                pIDataObject = NULL;
            }
*/
        }

        pIOleCache->Release();
        pIOleCache = NULL;
    }


    //We need an IOleObject most of the time, so get one here.
    m_pIOleObject=NULL;
    hr=pObj->QueryInterface(IID_IOleObject
         , (PPVOID)&m_pIOleObject);

    /*
     * Follow up object creation with advises and so forth.  If
     * we cannot get IOleObject here, then we know we can't do
     * any IOleObject actions from here on--object is static.
     */
    if (FAILED(hr))
        return TRUE;

    /*
     * Get the MiscStatus bits and check for OLEMISC_ONLYICONIC.
     * If set, force dwAspect in m_fe to DVASPECT_ICON so we
     * remember to draw it properly and do extents right.
     */
    m_pIOleObject->GetMiscStatus(dwAspect, &m_grfMisc);
    //m_pIOleObject->GetMiscStatus(m_fe.dwAspect, &m_grfMisc);

    if (OLEMISC_ONLYICONIC & m_grfMisc)
        dwAspect=DVASPECT_ICON;
        //m_fe.dwAspect=DVASPECT_ICON;

    /*
     * We could pass m_pImpIOleClientSite in an OleCreate* call, but
     * since this function could be called after OleLoad, we still
     * need to do this here, so it's always done here...
     */
    m_pIOleObject->SetClientSite(m_pImpIOleClientSite);
    m_pIOleObject->Advise(m_pImpIAdviseSink, &dw);

    OleSetContainedObject(m_pIOleObject, TRUE);

    /*
     * For IOleObject::SetHostNames we need the application name
     * and the document name (which is passed in the object
     * parameter).  The design of Patron doesn't give us nice
     * structured access to the name of the document we're in, so
     * I grab the parent of the Pages window (the document) and
     * send it DOCM_PDOCUMENT which returns us the pointer.
     * Roundabout, but it works.
     */

    /* MM ???
    pDoc=(PCDocument)SendMessage(GetParent(m_hWnd), DOCM_PDOCUMENT
        , 0, 0L);

    if (NULL!=pDoc)
        pDoc->FilenameGet(szFile, CCHPATHMAX);
    else
    */
        szFile[0]=0;

    NotifyOfRename(szFile, NULL);


    /*
     * If we're creating an iconic aspect object and we have
     * an object from the Insert Object dialog, then we need to
     * store that iconic presentation in the cache, handled
     * with the utility function StarObject_SwitchDisplayAspect.  In
     * this case dwData is a handle to the metafile containing
     * the icon.  If dwData is NULL then we depend on the
     * server to provide the aspect, in which case we need
     * a view advise.
     */

    //if (DVASPECT_ICON & m_fe.dwAspect)
    if (DVASPECT_ICON & dwAspect)
        {
        DWORD           dw=DVASPECT_CONTENT;
        IAdviseSink    *pSink;

        pSink=(NULL==dwData) ? NULL : m_pImpIAdviseSink;

        StarObject_SwitchDisplayAspect(m_pIOleObject, &dw
            , DVASPECT_ICON, (HGLOBAL)(UINT)dwData, FALSE
            , (NULL!=dwData), pSink, NULL);
        }

    return TRUE;
    }
//End CHAPTER17MOD




/*
 * CSO_Cont::Open
 *
 * Purpose:
 *  Retrieves the IStorage associated with this tenant.  The
 *  IStorage is owned by the tenant and thus the tenant always
 *  holds a reference count.
 *
 *  If the storage is already open for this tenant, then this
 *  function will AddRef it; therefore the caller must always
 *  match an Open with a Close.
 *
 * Parameters:
 *  pIStorage       LPSTORAGE above this tenant (which has its
 *                  own storage).
 *
 * Return Value:
 *  WIN_BOOL            TRUE if opening succeeds, FALSE otherwise.
 */

//WIN_BOOL CSO_Cont::Open(LPSTORAGE pIStorage)
WIN_BOOL CSO_Cont::Open(SotStorage * pIStorage)
    {
    /*
    HRESULT     hr=NOERROR;
    DWORD       dwMode=STGM_TRANSACTED | STGM_READWRITE
                    | STGM_SHARE_EXCLUSIVE;
    OLECHAR     szTemp[32];
    */

    if (NULL==m_pIStorage)
        {
        if (NULL==pIStorage)
            return FALSE;

        /*
         * Attempt to open the storage under this ID.  If there is
         * none, then create it.  In either case we end up with an
         * IStorage that we either save in pPage or release.
         */

//		StgCreateDocfile( "d:\\temp\\test.ttt", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, &m_pIStorage );

        SotStorageStreamRef xStm;
        String szTemp = GetStorageName();
        if( pIStorage->IsContained( szTemp ) )
        {
            xStm = pIStorage->OpenSotStream( szTemp, STREAM_STD_READ );
            if( xStm->GetError() )
                return FALSE;
            xStm->Seek( STREAM_SEEK_TO_END );
            UINT32 nLen = xStm->Tell();
            xStm->Seek( 0 );
            HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_SHARE, nLen );
            void * pMem = GlobalLock( hMem );
            if( pMem )
                xStm->Read( pMem, nLen );
            else
                return FALSE;
            GlobalUnlock( hMem );
            xStm = NULL;

            CreateILockBytesOnHGlobal(
                hMem, 			//Memory handle for the byte array object
                TRUE, 			//Whether to free memory when the object is released
                &pTmpStorage	//Indirect pointer to the new byte array object
            );
            if( !pTmpStorage )
                return FALSE;

            HRESULT hr = StgOpenStorageOnILockBytes(
                pTmpStorage, 	//Pointer to the byte array object
                NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 	//Access mode
                NULL,
                0, 	//Reserved; must be zero
                &m_pIStorage	//Indirect pointer to the new storage object
            );
            if( FAILED( hr ) )
                return FALSE;
        }
        else
        {
            CreateILockBytesOnHGlobal(
                NULL, 			//Memory handle for the byte array object
                TRUE, 			//Whether to free memory when the object is released
                &pTmpStorage	//Indirect pointer to the new byte array object
            );
            if( !pTmpStorage )
                return FALSE;

            StgCreateDocfileOnILockBytes(
                pTmpStorage, 	//Pointer to the byte array object
                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 	//Access mode
                0, 	//Reserved; must be zero
                &m_pIStorage	//Indirect pointer to the new storage object
            );
        }

        /* MM ??? StgOpenStorage
        GetStorageName(szTemp);
        hr=pIStorage->OpenStorage(szTemp, NULL, dwMode, NULL, 0
            , &m_pIStorage);

        if (FAILED(hr))
            {
            hr=pIStorage->CreateStorage(szTemp, dwMode, 0, 0
                , &m_pIStorage);
            }
        if (FAILED(hr))
            return FALSE;
        */
        }
    else
    {
        m_pIStorage->AddRef();
        pTmpStorage->AddRef();
    }


    m_cOpens++;

    //CHAPTER17MOD
    //Create these if we don't have them already.
    if (NULL==m_pImpIOleClientSite && NULL==m_pImpIAdviseSink)
        {
        m_pImpIOleClientSite=new CImpIOleClientSite(this, this);
        m_pImpIAdviseSink=new CImpIAdviseSink(this, this);

        if (NULL==m_pImpIOleClientSite || NULL==m_pImpIAdviseSink)
            return FALSE;
        }
    //End CHAPTER17MOD

    return TRUE;
    }




/*
 * CSO_Cont::Close
 *
 * Purpose:
 *  Possibly commits the storage, then releases it reversing the
 *  reference count from Open.  If the reference on the storage
 *  goes to zero, the storage is forgotten.  However, the object we
 *  contain is still held and as long as it's active the storage
 *  remains alive.
 *
 * Parameters:
 *  fCommit         WIN_BOOL indicating if we're to commit.
 *
 * Return Value:
 *  None
 */

//void CSO_Cont::Close(WIN_BOOL fCommit)
void CSO_Cont::Close(WIN_BOOL fCommit)
    {
    if (fCommit)
        Update();

    if (NULL!=m_pIStorage)
        {
        m_pIStorage->Release();
        pTmpStorage->Release();

        /*
         * We can't use a zero reference count to know when to NULL
         * this since other things might have AddRef'd the storage.
         */
        //CHAPTER17MOD
        if (0==--m_cOpens)
            {
            m_pIStorage=NULL;
            pTmpStorage=NULL;

            //Close the object saving if necessary
            if (NULL!=m_pIOleObject)
                {
                m_pIOleObject->Close( fCommit ? OLECLOSE_SAVEIFDIRTY : OLECLOSE_NOSAVE );
                ReleaseInterface(m_pIOleObject);
                }

            //Release all other held pointers
            if (NULL!=m_pIViewObject2)
                {
                m_pIViewObject2->SetAdvise(dwAspect, 0, NULL);
                //m_pIViewObject2->SetAdvise(m_fe.dwAspect, 0, NULL);
                ReleaseInterface(m_pIViewObject2);
                }

            ReleaseInterface(m_pObj);
            }
        //End CHAPTER17MOD
        }

    return;
    }




/*
 * CSO_Cont::Update
 *
 * Purpose:
 *  Forces a common on the page if it's open.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  WIN_BOOL            TRUE if the object is open, FALSE otherwise.
 */

//WIN_BOOL CSO_Cont::Update(void)
WIN_BOOL CSO_Cont::Update()
    {
    Save( m_pPG->GetWorkingStorage());
    m_pPG->ClearCache();
    m_pPG->ViewChanged( (UINT16)dwAspect );
    m_pPG->SendViewChanged();

    return FALSE;
    }





/*
 * CSO_Cont::Destroy
 *
 * Purpose:
 *  Removes this page from the given storage.  The caller should
 *  eventually delete this CSO_Cont object to free the object herein.
 *  Nothing is committed when being destroyed.
 *
 * Parameters:
 *  pIStorage       LPSTORAGE contianing this page on which to call
 *                  DestroyElement
 *
 * Return Value:
 *  None
 */

//void CSO_Cont::Destroy(LPSTORAGE pIStorage)
void CSO_Cont::Destroy(SotStorage * pIStorage)
    {
    if (NULL!=pIStorage)
        {
        //CHAPTER17MOD
        if (NULL!=m_pIOleObject)
            m_pIOleObject->Close(OLECLOSE_NOSAVE);
        //End CHAPTER17MOD

        if (NULL!=m_pIStorage)
            {
            //Remove all reference/open counts on this storage.
            while (0!=m_cOpens)
                {
                m_pIStorage->Release();
                pTmpStorage->Release();
                m_cOpens--;
                }
            }

        String szTemp = GetStorageName();
        pIStorage->Remove(szTemp);
        //GetStorageName(szTemp);
        //pIStorage->DestroyElement(szTemp);

        m_pIStorage=NULL;
        pTmpStorage = NULL;
        }

    return;
    }




/*
 * CSO_Cont::Select
 *
 * Purpose:
 *  Selects or deselects the tenant.
 *
 * Parameters:
 *  fSelect         WIN_BOOL indicating the new state of the tenant.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::Select(WIN_BOOL fSelect)
    {
    WIN_BOOL        fWasSelected;
    DWORD       dwState;
    RECT        rc;

    fWasSelected=(WIN_BOOL)(TENANTSTATE_SELECTED & m_dwState);

    //Nothing to do when there's no change.
    if (fWasSelected==fSelect)
        return;

    dwState=m_dwState & ~TENANTSTATE_SELECTED;
    m_dwState=dwState | ((fSelect) ? TENANTSTATE_SELECTED : 0);

    /*
     * Draw sizing handles to show the selection state.  We convert
     * things to MM_TEXT since that's what this function expects.
     */

    RECTFROMRECTL(rc, m_rcl);
    RectConvertMappings(&rc, NULL, TRUE);

    /* MM ???
    OffsetRect(&rc, -(int)m_pPG->m_xPos, -(int)m_pPG->m_yPos);

    hDC=GetDC(m_hWnd);

    UIDrawHandles(&rc, hDC, UI_HANDLES_INSIDE
        | UI_HANDLES_NOBORDER | UI_HANDLES_USEINVERSE
        , CXYHANDLE, !fWasSelected);

    ReleaseDC(m_hWnd, hDC);
    */

    if (fSelect)
        m_pPG->SetModified();
        //m_pPG->m_fDirty=TRUE;

    return;
    }




//CHAPTER17MOD
/*
 * CSO_Cont::ShowAsOpen
 *
 * Purpose:
 *  Draws or removes the hatch pattern over an object.
 *
 * Parameters:
 *  fOpen           WIN_BOOL indicating the open state of this tenant.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::ShowAsOpen(WIN_BOOL fOpen)
    {
    WIN_BOOL        fWasOpen;
    DWORD       dwState;
    RECT        rc;

    fWasOpen=(WIN_BOOL)(TENANTSTATE_OPEN & m_dwState);

    dwState=m_dwState & ~TENANTSTATE_OPEN;
    m_dwState=dwState | ((fOpen) ? TENANTSTATE_OPEN : 0);

    //If this was not open, then just hatch, otherwise repaint.
    if (!fWasOpen && fOpen)
        {
        RECTFROMRECTL(rc, m_rcl);
        RectConvertMappings(&rc, NULL, TRUE);
        /* MM ???
        OffsetRect(&rc, -(int)m_pPG->m_xPos, -(int)m_pPG->m_yPos);

        hDC=GetDC(m_hWnd);
        UIDrawShading(&rc, hDC, UI_SHADE_FULLRECT, 0);
        ReleaseDC(m_hWnd, hDC);
        */
        }

    if (fWasOpen && !fOpen)
    {
        m_pPG->GetProtocol().Reset2Open();

        Repaint();
    }
    else if (!fWasOpen && fOpen)
    {
        m_pPG->GetProtocol().EmbedProtocol();
    }

    return;
    }





/*
 * CSO_Cont::ShowYourself
 *
 * Purpose:
 *  Function that really just implements IOleClientSite::ShowObject.
 *  Here we first check if the tenant is fully visible, and if so,
 *  then nothing needs to happen.  Otherwise, if the upper left
 *  corner of the tenant is in the upper left visible quadrant of
 *  the window, we'll also consider ourselves done.  Otherwise
 *  we'll put the upper left corner of the object at the upper left
 *  corner of the window.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CSO_Cont::ShowYourself(void)
    {
    RECTL       rcl;
    RECT        rc;

    //Scrolling deals in device units; get our rectangle in those.
    RectGet(&rcl, TRUE);

    //Get the window rectangle offset for the current scroll pos.
    GetClientRect(m_hWnd, &rc);
    /* MM ???
    OffsetRect(&rc, m_pPG->m_xPos, m_pPG->m_yPos);

    //Check if the object is already visible. (macro in bookguid.h)
    SETPOINT(pt1, (int)rcl.left,  (int)rcl.top);
    SETPOINT(pt2, (int)rcl.right, (int)rcl.bottom);

    if (PtInRect(&rc, pt1) && PtInRect(&rc, pt2))
        return;

    //Check if the upper left is within the upper left quadrant
    if (((int)rcl.left > rc.left
        && (int)rcl.left < ((rc.right+rc.left)/2))
        && ((int)rcl.top > rc.top
        && (int)rcl.top < ((rc.bottom+rc.top)/2)))
        return;

    //These are macros in INC\BOOK1632.H
    SendScrollPosition(m_hWnd, WM_HSCROLL, rcl.left-8);
    SendScrollPosition(m_hWnd, WM_VSCROLL, rcl.top-8);
    */
    return;
    }



/*
 * CSO_Cont::AddVerbMenu
 *
 * Purpose:
 *  Creates the variable verb menu item for the object in this
 *  tenant.
 *
 * Parmeters:
 *  hMenu           HMENU on which to add items.
 *  iPos            UINT position on that menu to add items.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::AddVerbMenu(HMENU, UINT)
    {
    /* MM ???
    HMENU       hMenuTemp;
    LPOLEOBJECT pObj=m_pIOleObject;

    //If we're static, say we have no object.
    if (TENANTTYPE_STATIC==m_tType)
        pObj=NULL;

    OleUIAddVerbMenu(pObj, NULL, hMenu, iPos, IDM_VERBMIN
        , IDM_VERBMAX, TRUE, IDM_EDITCONVERT, &hMenuTemp);

    return;
    */
    }




/*
 * CSO_Cont::TypeGet
 *
 * Purpose:
 *  Returns the type of this tenant
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  TENANTTYPE      Type of the tenant.
 */

TENANTTYPE CSO_Cont::TypeGet(void)
    {
    return m_tType;
    }






/*
 * CSO_Cont::CopyEmbeddedObject
 *
 * Purpose:
 *  Copies an embedded object to the given data object (via SetData,
 *  assuming this is a data transfer object for clipboard/drag-drop)
 *  if that's what we're holding.
 *
 * Parameters:
 *  pIDataObject    LPDATAOBJECT in which to store the copy.
 *  pFE             LPFORMATETC into which to copy CFSTR_EMBEDDEDOBJECT
 *                  if we put that in the data object.
 *  pptl            PPOINTL to the pick point (NULL outside of
 *                  drag-drop);
 *
 * Return Value:
 *  None
 */

void CSO_Cont::CopyEmbeddedObject(LPDATAOBJECT pIDataObject
    , LPFORMATETC pFE, PPOINTL pptl)
    {
    LPPERSISTSTORAGE    pIPS;
    STGMEDIUM           stm;
    FORMATETC           fe;
    HRESULT             hr;
    UINT                cf;
    POINTL              ptl;
    SIZEL               szl;

    //Can only copy embeddings.
    if (TENANTTYPE_EMBEDDEDOBJECT!=m_tType || NULL==m_pIOleObject)
        return;

    if (NULL==pptl)
        {
        SETPOINTL(ptl, 0, 0);
        pptl=&ptl;
        }

    /*
     * Create CFSTR_EMBEDDEDOBJECT.  This is simply an IStorage with
     * a copy of the embedded object in it.  The not-so-simple part
     * is getting an IStorage to stuff it in.  For this operation
     * we'll use a temporary compound file.
     */

    stm.pUnkForRelease=NULL;
    stm.tymed=TYMED_ISTORAGE;
    hr=StgCreateDocfile(NULL, STGM_TRANSACTED | STGM_READWRITE
        | STGM_CREATE| STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE
        , 0, &stm.pstg);

    if (FAILED(hr))
        return;

    m_pObj->QueryInterface(IID_IPersistStorage, (PPVOID)&pIPS);

    if (NOERROR==pIPS->IsDirty())
        {
        OleSave(pIPS, stm.pstg, FALSE);
        pIPS->SaveCompleted(NULL);
        }
    else
        m_pIStorage->CopyTo(0, NULL, NULL, stm.pstg);

    pIPS->Release();

    //stm.pstg now has a copy, so stuff it away.
    cf=RegisterClipboardFormat(CFSTR_EMBEDDEDOBJECT);
    SETDefFormatEtc(fe, sal::static_int_cast< CLIPFORMAT >(cf), TYMED_ISTORAGE);

    if (SUCCEEDED(pIDataObject->SetData(&fe, &stm, TRUE)))
        *pFE=fe;
    else
        ReleaseStgMedium(&stm);

    stm.tymed=TYMED_HGLOBAL;

    /*
     * You want to make sure that if this object is iconic, that you
     * create the object descriptor with DVASPECT_ICON instead of
     * the more typical DVASPECT_CONTENT.  Also remember that
     * the pick point is in HIMETRIC.
     */
    SO_MetricSizeInPixelsToHimetric( (LPSIZEL)pptl, (LPSIZEL)&ptl);

    //SETSIZEL(szl, (10*(m_rcl.right-m_rcl.left))
    //    , (10 * (m_rcl.bottom-m_rcl.top)));
    SETSIZEL(szl, m_rcl.right-m_rcl.left, m_rcl.bottom-m_rcl.top );

    stm.hGlobal=StarObject_ObjectDescriptorFromOleObject(m_pIOleObject, dwAspect, ptl, &szl);
    //stm.hGlobal=StarObject_ObjectDescriptorFromOleObject(m_pIOleObject, m_fe.dwAspect, ptl, &szl);

    cf=RegisterClipboardFormat(CFSTR_OBJECTDESCRIPTOR);
    SETDefFormatEtc(fe, sal::static_int_cast< CLIPFORMAT >(cf), TYMED_HGLOBAL);

    if (FAILED(pIDataObject->SetData(&fe, &stm, TRUE)))
        ReleaseStgMedium(&stm);

    return;
    }





/*
 * CSO_Cont::NotifyOfRename
 *
 * Purpose:
 *  Instructs the tenant that the document was saved under a
 *  different name.  In order to keep the right compound document
 *  user interface, this tenant needs to tell its object through
 *  IOleObject::SetHostNames.
 *
 * Parameters:
 *  pszFile         LPTSTR of filename.
 *  pvReserved      LPVOID reserved for future use.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::NotifyOfRename(LPTSTR pszFile, LPVOID)
    {
    TCHAR       szObj[40];
    TCHAR       szApp[40];

    if (NULL==m_pIOleObject)
        return;

    if (TEXT('\0')==*pszFile)
        {
        // MM ???
        lstrcpyn( szObj, "untitled", ARRAYSIZE(szObj) );
        //LoadString(m_pPG->m_hInst, IDS_UNTITLED, szObj, sizeof(szObj));
        }
    else
        {
        // MM ???
        lstrcpyn( szObj, "untitled", ARRAYSIZE(szObj) );
        //GetFileTitle(pszFile, szObj, sizeof(szObj));

       #ifndef WIN32
        //Force filenames to uppercase in DOS versions.
        AnsiUpper(szObj);
       #endif
        }

    // MM ???
    lstrcpyn( szApp, "app name", ARRAYSIZE(szApp) );
    //LoadString(m_pPG->m_hInst, IDS_CAPTION, szApp, sizeof(szApp));
   #ifdef WIN32ANSI
    OLECHAR     szObjW[40], szAppW[40];

    MultiByteToWideChar(CP_ACP, 0, szObj, -1, szObjW, 40);
    MultiByteToWideChar(CP_ACP, 0, szApp, -1, szAppW, 40);
    m_pIOleObject->SetHostNames(szAppW, szObjW);
   #else
    m_pIOleObject->SetHostNames(szApp, szObj);
   #endif
    return;
    }

//End CHAPTER17MOD





/*
 * CSO_Cont::Activate
 *
 * Purpose:
 *  Activates a verb on the object living in the tenant.  Does
 *  nothing for static objects.
 *
 * Parameters:
 *  iVerb           LONG of the verb to execute.
 *
 * Return Value:
 *  WIN_BOOL            TRUE if the object changed due to this verb
 *                  execution.
 */

WIN_BOOL CSO_Cont::Activate(LONG iVerb, HWND hParent )
    {
    //CHAPTER17MOD
    RECT        rc, rcH;
    //CHourglass *pHour;
    SIZEL       szl;

    //Can't activate statics.
    if (TENANTTYPE_STATIC==m_tType || NULL==m_pIOleObject)
        {
        MessageBeep(0);
        return FALSE;
        }

    RECTFROMRECTL(rc, m_rcl);
    RectConvertMappings(&rc, NULL, TRUE);
    SO_MetricRectInPixelsToHimetric( &rc, &rcH);

    //pHour=new CHourglass;

    //Get the server running first, then do a SetExtent, then show it
    OleRun(m_pIOleObject);

    if (m_fSetExtent)
        {
        SETSIZEL(szl, rcH.right-rcH.left, rcH.top-rcH.bottom);
        m_pIOleObject->SetExtent(dwAspect, &szl);
        //m_pIOleObject->SetExtent(m_fe.dwAspect, &szl);
        m_fSetExtent=FALSE;
        }

    if( !hParent )
        hParent = m_hWnd;
    HRESULT hr = m_pIOleObject->DoVerb(iVerb, NULL, m_pImpIOleClientSite, 0
        , hParent, &rcH);

    //delete pHour;

    //If object changes, IAdviseSink::OnViewChange will see it.
    return SUCCEEDED( hr );
    //End CHAPTER17MOD
    }


BOOL CSO_Cont::GetMetaFile( long& nMapMode, Size& rSize, HMETAFILE& hMet )
{
    if ( !m_pObj )
        return FALSE;

    HRESULT         hr;
    LPDATAOBJECT    pIDataObject;
    FORMATETC       fe;
    STGMEDIUM       stm;
    hr=m_pObj->QueryInterface(IID_IDataObject, (PPVOID)&pIDataObject);
    if ( SUCCEEDED(hr) )
    {
        if ( !OleIsRunning( m_pIOleObject ) )
            OleRun( m_pObj );

        SETFormatEtc(fe, CF_METAFILEPICT, DVASPECT_CONTENT, NULL, TYMED_MFPICT, -1);
        hr=pIDataObject->GetData(&fe, &stm);
        pIDataObject->Release();
        METAFILEPICT* pMem = (METAFILEPICT*) GlobalLock( stm.hGlobal );
        if( !pMem )
            return FALSE;
        nMapMode = pMem->mm;
        rSize.Width() = pMem->xExt;
        rSize.Height() = pMem->yExt;
        hMet = pMem->hMF;
        GlobalUnlock( stm.hGlobal );
    }

    return TRUE;
}

/*
 * CSO_Cont::Draw
 *
 * Purpose:
 *  Draws the tenant in its rectangle on the given hDC.  We assume
 *  the DC is already set up for the mapping mode in which our
 *  rectangle is expressed, since the Page we're in tells us both
 *  the rect and the hDC.
 *
 * Parameters:
 *  hDC             HDC in which to draw.  Could be a metafile,
 *                  memory DC, screen, or printer.
 *  ptd             DVTARGETDEVICE * describing the device.
 *  hIC             HDC holding an information context (printing).
 *  xOff, yOff      int offsets for the page in lometric
 *  fNoColor        WIN_BOOL indicating if we should do B & W
 *  fPrinter        WIN_BOOL indicating if we should render for a
 *                  printer.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::Draw(HDC hDC, DVTARGETDEVICE *ptd, HDC hIC
    , int xOff, int yOff, WIN_BOOL fNoColor, WIN_BOOL fPrinter)
    {
    HRESULT         hr;
    RECT            rc = { 0, 0, xOff, -yOff };
    RECTL           rcl = { 0, 0, xOff, -yOff };
    UINT            uMM;

    //CHAPTER17MOD
    //We hold IViewObject2 all the time now, so no QueryInterface
    //End CHAPTER17MOD

    //RECTFROMRECTL(rc, m_rcl);
    //OffsetRect(&rc, -xOff, -yOff);
    //RECTLFROMRECT(rcl, rc);

    //CHAPTER17MOD
    //Repaint erases the rectangle to insure full object cleanup

    if (!fNoColor && !fPrinter)
        {
        COLORREF    cr;
        cr=SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        ExtTextOut(hDC, rc.left, rc.top, ETO_OPAQUE, &rc, NULL
            , 0, NULL);
        SetBkColor(hDC, cr);
        }

    //We have to use Draw since we have a target device and IC.
    hr=m_pIViewObject2->Draw(dwAspect, -1, NULL, ptd, hIC, hDC, NULL, &rcl, NULL, 0);
    //hr=m_pIViewObject2->Draw(dwAspect, -1, NULL, ptd, hIC, hDC, &rcl, NULL, NULL, 0);

    //End CHAPTER17MOD

    /*
     * If Draw failed, then perhaps it couldn't work for the device,
     * so try good old OleDraw as a last resort.  The code will
     * generally be OLE_E_BLANK.
     */
    if (FAILED(hr))
        OleDraw(m_pObj, dwAspect, hDC, &rc);
        //OleDraw(m_pObj, m_fe.dwAspect, hDC, &rc);

    //CHAPTER17MOD
    if (!fPrinter
        && (TENANTSTATE_SELECTED | TENANTSTATE_OPEN) & m_dwState)
        {
        /*
         * Draw sizing handles to show the selection state.  We
         * convert things to MM_TEXT since that's what this
         * function expects.
         */
        RectConvertMappings(&rc, NULL, TRUE);
        uMM=SetMapMode(hDC, MM_TEXT);

        if (TENANTSTATE_SELECTED & m_dwState)
            {
            UIDrawHandles(&rc, hDC, UI_HANDLES_INSIDE
                | UI_HANDLES_NOBORDER | UI_HANDLES_USEINVERSE
                , CXYHANDLE, TRUE);
            }

        if (TENANTSTATE_OPEN & m_dwState)
            UIDrawShading(&rc, hDC, UI_SHADE_FULLRECT, 0);

        uMM=SetMapMode(hDC, uMM);
        }
    //End CHAPTER17MOD

    return;
    }





/*
 * CSO_Cont::Repaint
 * CSO_Cont::Invalidate
 *
 * Purpose:
 *  Repaints the tenant where it lies or invalidates its area
 *  for later repainting.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CSO_Cont::Repaint(void)
    {

    //CHAPTER17MOD
    /*
     * We might be asked to repaint from
     * IOleClientSite::OnShowWindow after we've switched pages if
     * our server was running. This check on m_cOpens prevents that.
     */
    if (0==m_cOpens || !m_fRepaintEnabled)
        return;
    //End CHAPTER17MOD

    /* MM ???
    hDC=GetDC(m_hWnd);
    SetRect(&rc, m_pPG->m_xPos, m_pPG->m_yPos, 0, 0);
    RectConvertMappings(&rc, NULL, FALSE);

    SetMapMode(hDC, MM_LOMETRIC);
    Draw(hDC, NULL, NULL, rc.left, rc.top, FALSE, FALSE);

    ReleaseDC(m_hWnd, hDC);
    */
    return;
    }


void CSO_Cont::Invalidate(void)
    {
    RECTL       rcl;
    RECT        rc;

    RectGet(&rcl, TRUE);
    RECTFROMRECTL(rc, rcl);

    /* MMM ???
    OffsetRect(&rc, -(int)m_pPG->m_xPos, -(int)m_pPG->m_yPos);
    InvalidateRect(m_hWnd, &rc, FALSE);
    */

    return;
    }




//CHAPTER17MOD
/*
 * CSO_Cont::ObjectClassFormatAndIcon
 *
 * Purpose:
 *  Helper function for CPage::ConvertObject to retrieve necessary
 *  information about the object.
 *
 * Parameters:
 *  pClsID          LPCLSID in which to store the CLSID.
 *  pwFormat        LPWORD in which to store the clipboard format
 *                  used.
 *  ppszType        LPTSTR * in which to return a pointer to a
 *                  type string.
 *  phMetaIcon      HGLOBAL * in which to return the metafile
 *                  icon currently in use.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::ObjectClassFormatAndIcon(LPCLSID pClsID
    , LPWORD pwFormat, LPTSTR *ppszType, HGLOBAL *phMetaIcon
    , LPTSTR *ppszLabel)
    {
    HRESULT         hr;
    TCHAR           szType[128];
    LPDATAOBJECT    pIDataObject;
    FORMATETC       fe;
    STGMEDIUM       stm;

    if (TENANTTYPE_EMBEDDEDOBJECT!=m_tType || NULL==m_pIOleObject)
        return;

    if (NULL==pClsID || NULL==pwFormat || NULL==ppszType
        || NULL==phMetaIcon)
        return;


    /*
     * For embedded objects get the real CLSID of the object and
     * its format string.  If this fails then we can try to ask
     * the object, or we can look in the registry.
     */

    hr=ReadClassStg(m_pIStorage, pClsID);

    if (FAILED(hr))
        {
        hr=m_pIOleObject->GetUserClassID(pClsID);

        if (FAILED(hr))
            *pClsID=CLSID_NULL;
        }


    hr=StarObject_ReadFmtUserTypeStg(m_pIStorage, pwFormat, ppszType);

    if (FAILED(hr))
        {
        *pwFormat=0;
        *ppszType=NULL;

        if (StarObject_GetUserTypeOfClass(*pClsID, 0, szType
            , sizeof(szType)))
            {
            *ppszType=StarObject_CopyString(szType);
            }
        }

    /*
     * Try to get the AuxUserType from the registry, using
     * the short version (registered under AuxUserType\2).
     * If that fails, just copy *ppszType.
     */
    *ppszLabel=NULL;

    if (StarObject_GetUserTypeOfClass(*pClsID, 2, szType
        , sizeof(szType)))
        {
        *ppszLabel=StarObject_CopyString(szType);
        }
    else
        *ppszLabel=StarObject_CopyString(*ppszType);

    //Get the icon for this thing, if we're iconic.
    *phMetaIcon=NULL;

    hr=m_pObj->QueryInterface(IID_IDataObject
        , (PPVOID)&pIDataObject);

    if (SUCCEEDED(hr))
        {
        SETFormatEtc(fe, CF_METAFILEPICT, DVASPECT_ICON, NULL
            , TYMED_MFPICT, -1);
        hr=pIDataObject->GetData(&fe, &stm);
        pIDataObject->Release();

        if (SUCCEEDED(hr))
            *phMetaIcon=stm.hGlobal;
        else
            *phMetaIcon=OleGetIconOfClass(*pClsID, NULL, TRUE);
        }

    return;
    }




/*
 * CSO_Cont::SwitchOrUpdateAspect
 *
 * Purpose:
 *  Switches between DVASPECT_CONTENT and DVASPECT_ICON
 *
 * Parameters:
 *  hMetaIcon       HGLOBAL to the new icon if we're changing the
 *                  icon or switching to DVASPECT_ICON.  NULL to
 *                  change back to content.
 *  fPreserve       WIN_BOOL indicating if we're to preserve the old
 *                  aspect after changing.
 *
 * Return Value:
 *  WIN_BOOL            TRUE if anything changed, FALSE otherwise.
 */

WIN_BOOL CSO_Cont::SwitchOrUpdateAspect(HGLOBAL hMetaIcon
    , WIN_BOOL fPreserve)
    {
    HRESULT			hr;
    WIN_BOOL        fUpdate=FALSE;

    //Nothing to do if we're content already and there's no icon.
    //if (NULL==hMetaIcon && DVASPECT_CONTENT==m_fe.dwAspect)
    if (NULL==hMetaIcon && DVASPECT_CONTENT==dwAspect)
        return FALSE;

    //If we're iconic already, just cache the new icon
    //if (NULL!=hMetaIcon && DVASPECT_ICON==m_fe.dwAspect)
    if (NULL!=hMetaIcon && DVASPECT_ICON==dwAspect)
        hr=StarObject_SetIconInCache(m_pIOleObject, hMetaIcon);
    else
        {
        //Otherwise, switch between iconic and content.
        dwAspect=(NULL==hMetaIcon) ? DVASPECT_CONTENT : DVASPECT_ICON;

        /*
         * Switch between aspects, where dwAspect has the new one
         * and m_fe.dwAspect will be changed in the process.
         */
        hr=StarObject_SwitchDisplayAspect(m_pIOleObject
            , &dwAspect, dwAspect, hMetaIcon, !fPreserve
            //, &m_fe.dwAspect, dwAspect, hMetaIcon, !fPreserve
            , TRUE, m_pImpIAdviseSink, &fUpdate);

        if (SUCCEEDED(hr))
            {
            //Update MiscStatus for the new aspect
            m_pIOleObject->GetMiscStatus(dwAspect, &m_grfMisc);
            //m_pIOleObject->GetMiscStatus(m_fe.dwAspect, &m_grfMisc);

            if (fUpdate)
                m_pIOleObject->Update();    //This repaints.
            }
        }

    //If we switched, update our extents.
    if (SUCCEEDED(hr))
        {
        SIZEL       szl;

        m_pIOleObject->GetExtent(dwAspect, &szl);
        //m_pIOleObject->GetExtent(m_fe.dwAspect, &szl);

        if (0 > szl.cy)
            szl.cy=-szl.cy;

        //Convert HIMETRIC absolute units to our LOMETRIC mapping
        //if (0!=szl.cx && 0!=szl.cy)
        //    SETSIZEL(szl, szl.cx/10, -szl.cy/10);

        Invalidate();                   //Remove old aspect
        SizeSet(&szl, FALSE, FALSE);    //Change size
        Repaint();                      //Paint the new one
        }

    return SUCCEEDED(hr);
    }



/*
 * CSO_Cont::EnableRepaint
 *
 * Purpose:
 *  Toggles whether the Repaint function does anything.  This
 *  is used during conversion/emulation of an object to disable
 *  repaints until the new object can be given the proper extents.
 *
 * Parameters:
 *  fEnable         TRUE to enable repaints, FALSE to disable.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::EnableRepaint(WIN_BOOL fEnable)
    {
    m_fRepaintEnabled=fEnable;
    return;
    }
//End CHAPTER17MOD








/*
 * CSO_Cont::ObjectGet
 *
 * Purpose:
 *  Retrieves the LPUNKNOWN of the object in use by this tenant
 *
 * Parameters:
 *  ppUnk           LPUNKNOWN * in which to return the object
 *                  pointer.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::ObjectGet(LPUNKNOWN *ppUnk)
    {
    if (NULL!=ppUnk)
        {
        *ppUnk=m_pObj;
        m_pObj->AddRef();
        }

    return;
    }





/*
 * CSO_Cont::FormatEtcGet
 *
 * Purpose:
 *  Retrieves the FORMATETC in use by this tenant
 *
 * Parameters:
 *  pFE             LPFORMATETC in which to store the information.
 *  fPresentation   WIN_BOOL indicating if we want the real format or
 *                  that of the presentation.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::FormatEtcGet(LPFORMATETC pFE, WIN_BOOL fPresentation)
    {
    if (NULL!=pFE)
        {
        SETDefFormatEtc(*pFE, 0, TYMED_NULL);
        //*pFE=m_fe;

        //CHAPTER17MOD
        //If there is no format, use metafile (for embedded objects)
        if (fPresentation || 0==pFE->cfFormat)
            {
            //Don't mess with dwAspect; might be icon or content.
            pFE->cfFormat=CF_METAFILEPICT;
            pFE->tymed=TYMED_MFPICT;
            }
        //End CHAPTER17MOD
        }

    return;
    }




WIN_BOOL CSO_Cont::GetExtent(LPSIZEL pszl)
{
    //SETSIZEL((*pszl), 2*LOMETRIC_PER_INCH, 2*LOMETRIC_PER_INCH);
    SETSIZEL((*pszl), 2*HIMETRIC_PER_INCH, 2*HIMETRIC_PER_INCH);
    SIZEL szl = SIZEL();
    HRESULT hr=ResultFromScode(E_FAIL);

    //Try IViewObject2 first, then IOleObject as a backup.
    if (NULL!=m_pIViewObject2)
        {
        hr = m_pIViewObject2->GetExtent(dwAspect, -1, NULL, &szl);
        //hr=m_pIViewObject2->GetExtent(m_fe.dwAspect, -1, NULL, &szl);
        }
    else
        {
        if (NULL!=m_pIOleObject)
            hr = m_pIOleObject->GetExtent(dwAspect, &szl);
            //hr=m_pIOleObject->GetExtent(m_fe.dwAspect, &szl);
        }
    if (SUCCEEDED(hr))
        {
        *pszl = szl;
        //Convert HIMETRIC to our LOMETRIC mapping
        //SETSIZEL((*pszl), szl.cx/10, szl.cy/10);
        }

    return SUCCEEDED( hr );
}

/*
 * CSO_Cont::SizeGet
 * CSO_Cont::SizeSet
 * CSO_Cont::RectGet
 * CSO_Cont::RectSet
 *
 * Purpose:
 *  Returns or sets the size/position of the object contained here.
 *
 * Parameters:
 *  pszl/prcl       LPSIZEL (Size) or LPRECTL (Rect) with the
 *                  extents of interest.  In Get situations,
 *                  this will receive the extents; in Set it
 *                  contains the extents.
 *  fDevice         WIN_BOOL indicating that pszl/prcl is expressed
 *                  in device units.  Otherwise it's LOMETRIC.
 *  fInformObj      (Set Only) WIN_BOOL indicating if we need to inform
 *                  the object all.
 *
 * Return Value:
 *  None
 */

void CSO_Cont::SizeGet(LPSIZEL pszl, WIN_BOOL fDevice)
    {
    if (!fDevice)
        {
        pszl->cx=m_rcl.right-m_rcl.left;
        pszl->cy=m_rcl.bottom-m_rcl.top;
        }
    else
        {
        RECT        rc;

        SetRect(&rc, (int)(m_rcl.right-m_rcl.left)
            , (int)(m_rcl.bottom-m_rcl.top), 0, 0);

        RectConvertMappings(&rc, NULL, TRUE);

        pszl->cx=(long)rc.left;
        pszl->cy=(long)rc.top;
        }

    return;
    }

//CHAPTER17MOD
void CSO_Cont::SizeSet(LPSIZEL pszl, WIN_BOOL fDevice, WIN_BOOL fInformObj)
//End CHAPTER17MOD
    {
    SIZEL           szl;

    if (!fDevice)
        {
        szl=*pszl;
        m_rcl.right =pszl->cx+m_rcl.left;
        m_rcl.bottom=pszl->cy+m_rcl.top;
        }
    else
        {
        RECT        rc;

        SetRect(&rc, (int)pszl->cx, (int)pszl->cy, 0, 0);
        RectConvertMappings(&rc, NULL, FALSE);

        m_rcl.right =(long)rc.left+m_rcl.left;
        m_rcl.bottom=(long)rc.top+m_rcl.top;

        SETSIZEL(szl, (long)rc.left, (long)rc.top);
        }


    //Tell OLE that this object was resized.
    //CHAPTER17MOD
    if (NULL!=m_pIOleObject && fInformObj)
        {
        HRESULT     hr;
        WIN_BOOL        fRun=FALSE;

        //Convert our LOMETRIC into HIMETRIC by *=10
        //szl.cx*=10;
        //szl.cy*=-10;    //Our size is stored negative.
        if( szl.cy < 0 )
            szl.cy *= -1;

        /*
         * If the MiscStatus bit of OLEMISC_RECOMPOSEONRESIZE
         * is set, then we need to run the object before calling
         * SetExtent to make sure it has a real chance to
         * re-render the object.  We have to update and close
         * the object as well after this happens.
         */

        if (OLEMISC_RECOMPOSEONRESIZE & m_grfMisc)
            {
            if (!OleIsRunning(m_pIOleObject))
                {
                OleRun(m_pIOleObject);
                fRun=TRUE;
                }
            }

        hr=m_pIOleObject->SetExtent(dwAspect, &szl);
        //hr=m_pIOleObject->SetExtent(m_fe.dwAspect, &szl);

        /*
         * If the object is not running and it does not have
         * RECOMPOSEONRESIZE, then SetExtent fails.  Make
         * sure that we call SetExtent again (by just calling
         * SizeSet here again) when we next run the object.
         */
        if (SUCCEEDED(hr))
            {
            m_fSetExtent=FALSE;

            if (fRun)
                {
                m_pIOleObject->Update();
                m_pIOleObject->Close(OLECLOSE_SAVEIFDIRTY);
                }
            }
        else
            {
            if (OLE_E_NOTRUNNING==GetScode(hr))
                m_fSetExtent=TRUE;
            }
        }
    //End CHAPTER17MOD

    return;
    }


void CSO_Cont::RectGet(LPRECTL prcl, WIN_BOOL fDevice)
    {
    if (!fDevice)
        *prcl=m_rcl;
    else
        {
        RECT        rc;

        RECTFROMRECTL(rc, m_rcl);
        RectConvertMappings(&rc, NULL, TRUE);
        RECTLFROMRECT(*prcl, rc);
        }

    return;
    }


//CHAPTER17MOD
void CSO_Cont::RectSet(LPRECTL prcl, WIN_BOOL fDevice, WIN_BOOL fInformObj)
//End CHAPTER17MOD
    {
    SIZEL   szl;
    LONG    cx, cy;

    cx=m_rcl.right-m_rcl.left;
    cy=m_rcl.bottom-m_rcl.top;

    if (!fDevice)
        m_rcl=*prcl;
    else
        {
        RECT        rc;

        RECTFROMRECTL(rc, *prcl);
        RectConvertMappings(&rc, NULL, FALSE);
        RECTLFROMRECT(m_rcl, rc);
        }

    /*
     * Tell ourselves that the size changed, if it did.  SizeSet
     * will call IOleObject::SetExtent for us.
     */
    if ((m_rcl.right-m_rcl.left)!=cx || (m_rcl.bottom-m_rcl.top)!=cy)
        {
        SETSIZEL(szl, m_rcl.right-m_rcl.left, m_rcl.bottom-m_rcl.top);
        //CHAPTER17MOD
        SizeSet(&szl, FALSE, fInformObj);
        //End CHAPTER17MOD
        }

    return;
    }







/*
 * CSO_Cont::CreateStatic
 * (Protected)
 *
 * Purpose:
 *  Creates a new static bitmap or metafile object for this tenant
 *  using a freeloading method allowing us to specify exactly which
 *  type of data we want to paste since OleCreateStaticFromData
 *  doesn't.
 *
 * Parameters:
 *  pIDataObject    LPDATAOBJECT from which to paste.
 *  pFE             LPFORMATETC describing the format to paste.
 *  ppObj           LPUNKNOWN * into which we store the
 *                  object pointer.
 *
 * Return Value:
 *  HRESULT         NOERROR on success, error code otherwise.
 */

HRESULT CSO_Cont::CreateStatic(LPDATAOBJECT pIDataObject
    , LPFORMATETC pFE, LPUNKNOWN *ppObj)
    {
    HRESULT             hr;
    STGMEDIUM           stm;
    LPUNKNOWN           pIUnknown;
    LPOLECACHE          pIOleCache;
    LPPERSISTSTORAGE    pIPersistStorage;
    CLSID               clsID;

    *ppObj=NULL;

    //Try to get the data desired as specified in pFE->cfFormat
    hr=pIDataObject->GetData(pFE, &stm);

    if (FAILED(hr))
        return hr;

    //Create the object to handle this data.
    if (CF_METAFILEPICT==pFE->cfFormat)
        clsID=CLSID_Picture_Metafile;
    else
        clsID=CLSID_Picture_Dib;

    hr=CreateDataCache(NULL, clsID, IID_IUnknown
        , (PPVOID)&pIUnknown);

    if (FAILED(hr))
        {
        ReleaseStgMedium(&stm);
        return hr;
        }

    m_clsID=clsID;

    //Stuff the data into the object
    pIUnknown->QueryInterface(IID_IPersistStorage
        , (PPVOID)&pIPersistStorage);
    pIPersistStorage->InitNew(m_pIStorage);

    //Now that we have the cache object, shove the data into it.
    pIUnknown->QueryInterface(IID_IOleCache, (PPVOID)&pIOleCache);
    pIOleCache->Cache(pFE, ADVF_PRIMEFIRST, NULL);

    hr=pIOleCache->SetData(pFE, &stm, TRUE);
    pIOleCache->Release();

    //Insure there is a persistent copy on the disk
    WriteClassStg(m_pIStorage, m_clsID);
    pIPersistStorage->Save(m_pIStorage, TRUE);
    pIPersistStorage->SaveCompleted(NULL);
    pIPersistStorage->Release();

    //The cache owns this now.
    ReleaseStgMedium(&stm);

    if (FAILED(hr))
        pIUnknown->Release();
    else
        *ppObj=pIUnknown;

    return hr;
    }

CLSID CSO_Cont::GetCLSID()
{
    if ( m_clsID == GUID_NULL )
    {
         LPPERSIST pIPS;
        m_pObj->QueryInterface(IID_IPersist, (PPVOID)&pIPS);
        pIPS->GetClassID( &m_clsID );
    }

    return m_clsID;
}
}
