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


#include "soole.h"

#include <systools/win32/snprintf.h>

#include <sal/macros.h>

namespace binfilter {
static BOOL GetFileTimes(LPTSTR pszFileName, FILETIME *pft);

/*
 * StarObject_MetafilePictIconFree
 *
 * Purpose:
 *  Deletes the metafile contained in a METAFILEPICT structure and
 *  frees the memory for the structure itself.
 *
 * Parameters:
 *  hMetaPict       HGLOBAL metafilepict structure created in
 *                  OleMetafilePictFromIconAndLabel
 *
 * Return Value:
 *  None
 */

STDAPI_(void) StarObject_MetafilePictIconFree(HGLOBAL hMetaPict)
   {
   LPMETAFILEPICT      pMF;

   if (NULL==hMetaPict)
      return;

   pMF=(LPMETAFILEPICT)GlobalLock(hMetaPict);

   if (NULL!=pMF)
      {
      if (NULL!=pMF->hMF)
         DeleteMetaFile(pMF->hMF);
      }

   GlobalUnlock(hMetaPict);
   GlobalFree(hMetaPict);
   return;
   }





/*
 * StarObject_SwitchDisplayAspect
 *
 * Purpose:
 *  Switch the currently cached display aspect between DVASPECT_ICON
 *  and DVASPECT_CONTENT.  When setting up icon aspect, any currently
 *  cached content cache is discarded and any advise connections for
 *  content aspect are broken.
 *
 * Parameters:
 *  pObj            IUnknown * to the object in question
 *  pdwCurAspect    DWORD * containing the current aspect which
 *                  will contain the new aspect on output.
 *  dwNewAspect     DWORD with the aspect to switch to.
 *  hMetaPict       HGLOBAL containing the CF_METAFILEPICT with
 *                  the icon.
 *  fDeleteOld      BOOL indicating if we're to delete the old
 *                  aspect from the cache.
 *  fViewAdvise     BOOL indicating if we're to establish an
 *                  advise with the object for this new aspect.
 *  pSink           IAdviseSink * to the notification sink.
 *  pfMustUpdate    BOOL * in which to return whether or not
 *                  an update from a running server is necessary.
 *
 * Return Value:
 *  HRESULT         NOERROR or an error code in which case the cache
 *                  remains unchanged.
 */

STDAPI StarObject_SwitchDisplayAspect(IUnknown *pObj, LPDWORD pdwCurAspect
    , DWORD dwNewAspect, HGLOBAL hMetaPict, BOOL fDeleteOld
    , BOOL fViewAdvise, IAdviseSink *pSink, BOOL *pfMustUpdate)
    {
    IOleCache      *pCache=NULL;
    FORMATETC       fe;
    STGMEDIUM       stm;
    DWORD           dwAdvf;
    DWORD           dwNewConnection;
    DWORD           dwOldAspect=*pdwCurAspect;
    HRESULT         hr;

    if (pfMustUpdate)
       *pfMustUpdate=FALSE;

    hr=pObj->QueryInterface(IID_IOleCache, (void **)&pCache);

    if (FAILED(hr))
       return ResultFromScode(E_FAIL);

    //Establish new cache with the new aspect
    SETFormatEtc(fe, 0, dwNewAspect, NULL, TYMED_NULL, -1);

    /*
     * If we are using a custom icon for the display aspect then
     * we won't want to establish notifications with the data
     * source, that is, we don't want to change the icon.  Otherwise
     * we link up the given advise sink.
     */
    if (DVASPECT_ICON==dwNewAspect && NULL!=hMetaPict)
       dwAdvf=ADVF_NODATA;
    else
       dwAdvf=ADVF_PRIMEFIRST;

    hr=pCache->Cache(&fe, dwAdvf, &dwNewConnection);

    if (FAILED(hr))
        {
        pCache->Release();
        return hr;
        }

    *pdwCurAspect=dwNewAspect;

    /*
     * Stuff the custom icon into the cache, or force an update
     * from the server.
     */
    if (DVASPECT_ICON==dwNewAspect && NULL!=hMetaPict)
        {
        SETFormatEtc(fe, CF_METAFILEPICT, DVASPECT_ICON, NULL
            , TYMED_MFPICT, -1);

        stm.tymed=TYMED_MFPICT;
        stm.hGlobal=hMetaPict;
        stm.pUnkForRelease=NULL;

        hr=pCache->SetData(&fe, &stm, FALSE);
        }
    else
        {
        if (pfMustUpdate)
           *pfMustUpdate=TRUE;
        }

    if (fViewAdvise && pSink)
        {
        IViewObject    *pView=NULL;

        hr=pObj->QueryInterface(IID_IViewObject, (void **)&pView);

        if (SUCCEEDED(hr))
            {
            pView->SetAdvise(dwNewAspect, 0, pSink);
            pView->Release();
            }
        }

    /*
     * Remove existing caches for the old display aspect to cut
     * down on needless storage overhead.  If you want to switch
     * frequently between icon and content aspects, then it is
     * best to actually keep both presentations in the cache.
     */
    if (fDeleteOld)
        {
        IEnumSTATDATA  *pEnum=NULL;
        STATDATA        sd;

        hr=pCache->EnumCache(&pEnum);

        while(NOERROR==hr)
            {
            hr=pEnum->Next(1, &sd, NULL);

            if (NOERROR==hr)
                {
                //Remove old aspect caches
                if (sd.formatetc.dwAspect==dwOldAspect)
                    pCache->Uncache(sd.dwConnection);
                }
            }

        if (NULL!=pEnum)
            pEnum->Release();
        }

    pCache->Release();
    return NOERROR;
    }



/*
 * StarObject_SetIconInCache
 *
 * Purpose:
 *  Stores an iconic presentation metafile in the cache.
 *
 * Parameters:
 *  pObj            IUnknown * of the object.
 *  hMetaPict       HGLOBAL containing the presentation.
 *
 * Return Value:
 *  HRESULT         From IOleCache::SetData.
 */

STDAPI StarObject_SetIconInCache(IUnknown *pObj, HGLOBAL hMetaPict)
    {
    IOleCache      *pCache;
    FORMATETC       fe;
    STGMEDIUM       stm;
    HRESULT         hr;

    if (NULL==hMetaPict)
        return ResultFromScode(E_INVALIDARG);

    if (FAILED(pObj->QueryInterface(IID_IOleCache, (void **)&pCache)))
        return ResultFromScode(E_NOINTERFACE);

    SETFormatEtc(fe, CF_METAFILEPICT, DVASPECT_ICON, NULL
        , TYMED_MFPICT, -1);

    stm.tymed=TYMED_MFPICT;
    stm.hGlobal=hMetaPict;
    stm.pUnkForRelease=NULL;

    hr=pCache->SetData(&fe, &stm, FALSE);
    pCache->Release();

    return hr;
    }





/*
 * StarObject_GetUserTypeOfClass
 *
 * Purpose:
 *  Returns the user type (human readable class name) of the
 *  specified class as stored in the registry.
 *
 * Parameters:
 *  clsID           CLSID in question
 *  iName           UINT index to the name to retrieve, where
 *                  zero is the name found as the value of
 *                  of the CLSID, anything else tries
 *                  AuxUserType\iName.
 *  pszUserType     LPTSTR in which to return the type
 *  cch             UINT length of pszUserType
 *
 * Return Value:
 *  UINT            Number of characters in returned string.
 *                  0 on error.
 */

STDAPI_(UINT) StarObject_GetUserTypeOfClass(REFCLSID clsID, UINT iName
    , LPTSTR pszUserType, UINT cch)
    {
    LPTSTR      pszCLSID;
    LPTSTR      pszProgID;
    TCHAR       szKey[300];
    LONG        dw;
    LONG        lRet;

    if (!pszUserType)
        return 0;

    *pszUserType='\0';

    //Get a string containing the class name
    StarObject_StringFromCLSID(clsID, &pszCLSID);

    if (0==iName)
        sntprintf(szKey, SAL_N_ELEMENTS(szKey), TEXT("CLSID\\%s"), pszCLSID);
    else
        {
        sntprintf(szKey, SAL_N_ELEMENTS(szKey), TEXT("CLSID\\%s\\AuxUserType\\%u")
            , pszCLSID, iName);
        }

    CoTaskMemFree(pszCLSID);

    dw=cch;
    lRet=RegQueryValue(HKEY_CLASSES_ROOT, szKey, pszUserType, &dw);

    if (ERROR_SUCCESS!=lRet)
        {
        lstrcpyn(pszUserType, TEXT("Unknown"), cch);

        if (CoIsOle1Class(clsID))
            {
            //Try to get ProgID value for OLE 1 class
            StarObject_ProgIDFromCLSID(clsID, &pszProgID);

            dw=cch;
            lRet=RegQueryValue(HKEY_CLASSES_ROOT, pszProgID
                , pszUserType, &dw);

            CoTaskMemFree(pszProgID);

            if (ERROR_SUCCESS!=lRet)
                dw=0;
            }
        }

    return (UINT)dw;
    }





/*
 * StarObject_DoConvert
 *
 * Purpose:
 *  Convert an embedded or linked object to another type, working
 *  in conjunection with OleUIConvert.
 *
 * Parameters:
 *  pIStorage       IStorage * to the object's data.
 *  clsID           CLSID to which we convert the object.
 *
 * Return Value:
 *  HRESULT         The usual.
 */

STDAPI StarObject_DoConvert(IStorage *pIStorage, REFCLSID clsID)
    {
    HRESULT     hr;
    CLSID       clsIDOrg;
    CLIPFORMAT  cfOrg;
    LPTSTR      pszOrg=NULL;
    TCHAR       szNew[256];

    if (FAILED(ReadClassStg(pIStorage, &clsIDOrg)))
        return ResultFromScode(E_FAIL);

    //Read original format/user type
    hr=StarObject_ReadFmtUserTypeStg(pIStorage, &cfOrg, &pszOrg);

    //Get new user type
    if (0==StarObject_GetUserTypeOfClass(clsID, 0, szNew, 256))
        lstrcpyn(szNew, TEXT(""), SAL_N_ELEMENTS(szNew));

    //Write new class into the storage
    if (SUCCEEDED(WriteClassStg(pIStorage, clsID)))
        {
        if (SUCCEEDED(StarObject_WriteFmtUserTypeStg(pIStorage, cfOrg
            , szNew)))
            {
            SetConvertStg(pIStorage, TRUE);
            CoTaskMemFree((void *)pszOrg);
            return NOERROR;
            }

        //Failed to write new type, restore the old class
        WriteClassStg(pIStorage, clsIDOrg);
        }

    CoTaskMemFree((void *)pszOrg);
    return ResultFromScode(E_FAIL);
    }




/*
 * StarObject_CopyString
 *
 * Purpose:
 *  Copies a string allocated with CoTaskMemAlloc.
 *
 * Parameters:
 *  pszSrc          LPTSTR to the string to copy.
 *
 * Return Value:
 *  LPTSTR          New string or a NULL.
 */

STDAPI_(LPTSTR) StarObject_CopyString(LPTSTR pszSrc)
    {
    IMalloc    *pIMalloc;
    LPTSTR      pszDst;
    UINT        cch;

    cch=lstrlen(pszSrc);

    if (FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc)))
        return NULL;

    pszDst=(LPTSTR)pIMalloc->Alloc((cch+1)*sizeof(TCHAR));

    if (NULL!=pszDst)
       lstrcpyn(pszDst, pszSrc, cch+1);

    pIMalloc->Release();
    return pszDst;
    }




/*
 * StarObject_ObjectDescriptorFromOleObject
 *
 * Purpose:
 *  Fills and returns an OBJECTDESCRIPTOR structure. Information
 *  for the structure is obtained from an IOleObject instance.
 *
 * Parameters:
 *  pObj            IOleObject * from which to retrieve information.
 *  dwAspect        DWORD with the display aspect
 *  ptl             POINTL from upper-left corner of object where
 *                  mouse went down for use with Drag & Drop.
 *  pszl            LPSIZEL (optional) if the object is being scaled in
 *                  its container, then the container should pass the
 *                  extents that it is using to display the object.
 *
 * Return Value:
 *  HBGLOBAL         Handle to OBJECTDESCRIPTOR structure.
 */

STDAPI_(HGLOBAL) StarObject_ObjectDescriptorFromOleObject
    (IOleObject *pObj, DWORD dwAspect, POINTL ptl, LPSIZEL pszl)
    {
    CLSID           clsID;
    LPTSTR          pszName=NULL;
    LPTSTR          pszSrc=NULL;
    BOOL            fLink=FALSE;
    IOleLink       *pLink;
    TCHAR           szName[512];
    DWORD           dwMisc=0;
    HGLOBAL         hMem;
    HRESULT         hr;
   #ifdef WIN32ANSI
   //#ifndef UNICODE
    LPWSTR			pszw;
    char	szTemp[512];
   #endif

    if (SUCCEEDED(pObj->QueryInterface(IID_IOleLink
        , (void **)&pLink)))
        fLink=TRUE;

    if (FAILED(pObj->GetUserClassID(&clsID)))
        clsID=CLSID_NULL;

    //Get user string, expand to "Linked %s" if this is link
   #ifndef WIN32ANSI
   //#ifdef UNICODE
    pObj->GetUserType(USERCLASSTYPE_FULL, &pszName);
   #else
    pObj->GetUserType(USERCLASSTYPE_FULL, &pszw);
    WideCharToMultiByte(CP_ACP, 0, pszw, -1, szTemp
        , sizeof(szTemp), NULL, NULL);
    pszName=szTemp;
   #endif

    if (fLink && NULL!=pszName)
        sntprintf(szName, SAL_N_ELEMENTS(szName), TEXT("Linked %s"), pszName);
    else
        lstrcpyn(szName, pszName, SAL_N_ELEMENTS(szName));

   #ifndef WIN32ANSI
   //#ifdef UNICODE
    CoTaskMemFree(pszName);
   #else
    CoTaskMemFree(pszw);
   #endif

    /*
     * Get the source name of this object using either the
     * link display name (for link) or a moniker display
     * name.
     */

    if (fLink)
        {
       #ifndef WIN32ANSI
       //#ifdef UNICODE
        hr=pLink->GetSourceDisplayName(&pszSrc);
       #else
        hr=pLink->GetSourceDisplayName(&pszw);
        WideCharToMultiByte(CP_ACP, 0, pszw, -1, szTemp
            , sizeof(szTemp), NULL, NULL);
        pszSrc=szTemp;
       #endif
        }
    else
        {
        IMoniker   *pmk;

        hr=pObj->GetMoniker(OLEGETMONIKER_TEMPFORUSER
            , OLEWHICHMK_OBJFULL, &pmk);

        if (SUCCEEDED(hr))
            {
            IBindCtx  *pbc;
            CreateBindCtx(0, &pbc);

           #ifndef WIN32ANSI
           //#ifdef UNICODE
            pmk->GetDisplayName(pbc, NULL, &pszSrc);
           #else
            pmk->GetDisplayName(pbc, NULL, &pszw);
            WideCharToMultiByte(CP_ACP, 0, pszw, -1, szTemp
                , sizeof(szTemp), NULL, NULL);
            pszSrc=szTemp;
           #endif

            pbc->Release();

            pmk->Release();
            }
        }

    if (fLink)
        pLink->Release();

    //Get MiscStatus bits
    hr=pObj->GetMiscStatus(dwAspect, &dwMisc);

    //Get OBJECTDESCRIPTOR
    hMem=StarObject_AllocObjectDescriptor(clsID, dwAspect, *pszl, ptl
        , dwMisc, szName, pszSrc);

   #ifndef WIN32ANSI
   //#ifdef UNICODE
    CoTaskMemFree(pszSrc);
   #else
    CoTaskMemFree(pszw);
   #endif

    return hMem;
    }





/*
 * StarObject_AllocObjectDescriptor
 *
 * Purpose:
 *  Allocated and fills an OBJECTDESCRIPTOR structure.
 *
 * Parameters:
 *  clsID           CLSID to store.
 *  dwAspect        DWORD with the display aspect
 *  pszl            LPSIZEL (optional) if the object is being scaled in
 *                  its container, then the container should pass the
 *                  extents that it is using to display the object.
 *  ptl             POINTL from upper-left corner of object where
 *                  mouse went down for use with Drag & Drop.
 *  dwMisc          DWORD containing MiscStatus flags
 *  pszName         LPTSTR naming the object to copy
 *  pszSrc          LPTSTR identifying the source of the object.
 *
 * Return Value:
 *  HBGLOBAL         Handle to OBJECTDESCRIPTOR structure.
 */

STDAPI_(HGLOBAL) StarObject_AllocObjectDescriptor(CLSID clsID
    , DWORD dwAspect, SIZEL szl, POINTL ptl, DWORD dwMisc
    , LPTSTR pszName, LPTSTR pszSrc)
    {
    HGLOBAL              hMem=NULL;
    LPOBJECTDESCRIPTOR   pOD;
    DWORD                cb, cbStruct;
    DWORD                cchName, cchSrc;

    cchName=lstrlen(pszName)+1;

    if (NULL!=pszSrc)
        cchSrc=lstrlen(pszSrc)+1;
    else
        {
        cchSrc=cchName;
        pszSrc=pszName;
        }

    /*
     * Note:  CFSTR_OBJECTDESCRIPTOR is an ANSI structure.
     * That means strings in it must be ANSI.  OLE will do
     * internal conversions back to Unicode as necessary,
     * but we have to put ANSI strings in it ourselves.
     */
    cbStruct=sizeof(OBJECTDESCRIPTOR);
    cb=cbStruct+(sizeof(WCHAR)*(cchName+cchSrc));   //HACK

    hMem=GlobalAlloc(GHND, cb);

    if (NULL==hMem)
        return NULL;

    pOD=(LPOBJECTDESCRIPTOR)GlobalLock(hMem);

    pOD->cbSize=cb;
    pOD->clsid=clsID;
    pOD->dwDrawAspect=dwAspect;
    pOD->sizel=szl;
    pOD->pointl=ptl;
    pOD->dwStatus=dwMisc;

    if (pszName)
        {
        pOD->dwFullUserTypeName=cbStruct;
       #ifdef WIN32ANSI
        MultiByteToWideChar(CP_ACP, 0, pszName, -1
            , (LPWSTR)((LPBYTE)pOD+pOD->dwFullUserTypeName), cchName);
       #else
        lstrcpyn((LPTSTR)((LPBYTE)pOD+pOD->dwFullUserTypeName)
            , pszName, cchName);
       #endif
        }
    else
        pOD->dwFullUserTypeName=0;  //No string

    if (pszSrc)
        {
        pOD->dwSrcOfCopy=cbStruct+(cchName*sizeof(WCHAR));

       #ifdef WIN32ANSI
        MultiByteToWideChar(CP_ACP, 0, pszSrc, -1
            , (LPWSTR)((LPBYTE)pOD+pOD->dwSrcOfCopy), cchSrc);
       #else
        lstrcpyn((LPTSTR)((LPBYTE)pOD+pOD->dwSrcOfCopy), pszSrc, cchSrc);
       #endif
        }
    else
        pOD->dwSrcOfCopy=0;  //No string

    GlobalUnlock(hMem);
    return hMem;
    }





/*
 * StarObject_CreateStorageOnHGlobal
 *
 * Purpose:
 *  Create a memory based storage object on an memory lockbytes.
 *
 * Parameters:
 *  grfMode --  flags passed to StgCreateDocfileOnILockBytes
 *
 *  NOTE: if hGlobal is NULL, then a new IStorage is created and
 *              STGM_CREATE flag is passed to StgCreateDocfileOnILockBytes.
 *        if hGlobal is non-NULL, then it is assumed that the hGlobal already
 *              has an IStorage inside it and STGM_CONVERT flag is passed
 *              to StgCreateDocfileOnILockBytes.
 *
 * Return Value:
 *  IStorage *      Pointer to the storage we create.
 */

STDAPI_(IStorage *) StarObject_CreateStorageOnHGlobal(DWORD grfMode)
    {
    DWORD           grfCreate=grfMode | STGM_CREATE;
    HRESULT         hr;
    ILockBytes     *pLB=NULL;
    IStorage       *pIStorage=NULL;

    hr=CreateILockBytesOnHGlobal(NULL, TRUE, &pLB);

    if (FAILED(hr))
        return NULL;

    hr=StgCreateDocfileOnILockBytes(pLB, grfCreate, 0, &pIStorage);

    pLB->Release();

    return SUCCEEDED(hr) ? pIStorage : NULL;
    }








/*
 * StarObject_GetLinkSourceData
 *
 * Purpose:
 *  Creates a CFSTR_LINKSOURCE data format, that is, a serialized
 *  moniker in a stream.
 *
 * Parameters:
 *  pmk             IMoniker * of the link.
 *  pClsID          CLSID * to write into the stream.
 *  pFE             FORMATETC * describing the data which
 *                  should have TYMED_ISTREAM in it.
 *  pSTM            STGMEDIUM in which to return the data.
 *                  If this has TYMED_NULL, then this function
 *                  allocates the stream, otherwise it writes
 *                  into the stream in pSTM->pstm.
 */

STDAPI StarObject_GetLinkSourceData(IMoniker *pmk, LPCLSID pClsID
    , LPFORMATETC pFE, LPSTGMEDIUM pSTM)
    {
    LPSTREAM    pIStream = NULL;
    HRESULT     hr;

    pSTM->pUnkForRelease=NULL;

    if (TYMED_NULL==pSTM->tymed)
        {
        if (pFE->tymed & TYMED_ISTREAM)
            {
            hr=CreateStreamOnHGlobal(NULL, TRUE, &pIStream);

            if (FAILED(hr))
                return ResultFromScode(E_OUTOFMEMORY);

            pSTM->pstm=pIStream;
            pSTM->tymed=TYMED_ISTREAM;
            }
        else
            return ResultFromScode(DATA_E_FORMATETC);
        }
    else
        {
        if (TYMED_ISTREAM==pSTM->tymed)
            {
            pSTM->tymed=TYMED_ISTREAM;
            pSTM->pstm=pSTM->pstm;
            }
        else
            return ResultFromScode(DATA_E_FORMATETC);
       }

    hr=OleSaveToStream(pmk, pSTM->pstm);

    if (FAILED(hr))
        return hr;

    return WriteClassStm(pSTM->pstm, *pClsID);
    }





/*
 * StarObject_RegisterAsRunning
 *
 * Purpose:
 *  Registers a moniker as running in the running object
 *  table, revoking any existing registration.
 *
 * Parameters:
 *  pUnk            IUnknown * of the object.
 *  pmk             IMoniker * naming the object.
 *  dwFlags         DWORD flags to pass to IROT::Register
 *  pdwReg          LPDWORD in which to store the registration key.
 *
 * Return Value:
 *  None
 */

STDAPI_(void) StarObject_RegisterAsRunning(IUnknown *pUnk
    , IMoniker *pmk, DWORD dwFlags, LPDWORD pdwReg)
    {
    IRunningObjectTable    *pROT;
    HRESULT                 hr;
    DWORD                   dwReg=*pdwReg;

    if (NULL==pmk || NULL==pUnk || NULL==pdwReg)
        return;

    dwReg=*pdwReg;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return;

    hr=pROT->Register(dwFlags, pUnk, pmk, pdwReg);

    if (MK_S_MONIKERALREADYREGISTERED==GetScode(hr))
        {
        if (0!=dwReg)
            pROT->Revoke(dwReg);
        }

    pROT->Release();
    return;
    }



/*
 * StarObject_RevokeAsRunning
 *
 * Purpose:
 *  Wrapper for IRunningObjectTable::Revoke
 *
 * Parameters:
 *  pdwReg          LPDWORD containing the key on input, zeroed
 *                  on output.
 *
 * Return Value:
 *  None
 */

STDAPI_(void) StarObject_RevokeAsRunning(LPDWORD pdwReg)
    {
    IRunningObjectTable    *pROT;

    if (0==*pdwReg)
        return;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return;

    pROT->Revoke(*pdwReg);
    pROT->Release();

    *pdwReg=0;
    return;
    }



/*
 * StarObject_NoteChangeTime
 *
 * Purpose:
 *  Wrapper for IRunningObjectTable::NoteChangeTime
 *
 * Parameters:
 *  dwReg           DWORD identifying the running object to note.
 *  pft             FILETIME * containing the new time.  If NULL,
 *                  then we'll get it from CoFileTimeNow.
 *  pszFile         LPTSTR to a filename.  If pft is NULL, we'll
 *                  retrieve times from this file.
 *
 * Return Value:
 *  None
 */

STDAPI_(void) StarObject_NoteChangeTime(DWORD dwReg, FILETIME *pft
    , LPTSTR pszFile)
    {
    IRunningObjectTable    *pROT;
    FILETIME                ft;

    if (NULL==pft)
        {
        CoFileTimeNow(&ft);
        pft=&ft;
        }

    if (NULL!=pszFile)
        GetFileTimes(pszFile, pft);

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return;

    pROT->NoteChangeTime(dwReg, pft);
    pROT->Release();
    return;
    }




/*
 * GetFileTimes
 * (Internal)
 *
 * Purpose:
 *  Retrieve the FILETIME structure for a given file.
 */

static BOOL GetFileTimes(LPTSTR pszFileName, FILETIME *pft)
{
    WIN32_FIND_DATA fd;
    HANDLE          hFind;

    hFind=FindFirstFile(pszFileName, &fd);

    if (NULL==hFind || INVALID_HANDLE_VALUE==hFind)
        return FALSE;

    FindClose(hFind);
    *pft=fd.ftLastWriteTime;
    return TRUE;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
