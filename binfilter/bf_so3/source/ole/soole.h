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


#ifndef _SOOLE_H_
#define _SOOLE_H_

#define INC_OLE2
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <ole2.h>
#include <ole2ver.h>

#ifdef WNT
#define WIN32ANSI
#endif

//#ifdef INC_OLEUI
#ifdef WIN32
/*
 * Comment the next line and remove the comment from the
 * line after it to compile for Windows NT 3.5.
 */
#include <oledlg.h>
//#include <ole2ui.h>
#else
#include <ole2ui.h>
#endif
//#endif

//#include <bookguid.h>

#ifdef INC_CONTROLS
#define INC_AUTOMATION
#endif

#ifdef INC_AUTOMATION
#ifndef WIN32
#include <dispatch.h>
#include <olenls.h>
#else
#include <oleauto.h>
#endif
#endif

#ifdef INC_CONTROLS
#include <olectl.h>
#ifndef INITGUIDS
#include <olectlid.h>
#endif
#endif

#ifdef WIN32
#include <tchar.h>
#ifdef UNICODE
#include <wchar.h>
#endif
#endif

#ifndef WIN32
#include <shellapi.h>
#include <malloc.h>
#endif

//#include <book1632.h>
//#include <dbgout.h>

#ifdef INC_CLASSLIB
extern "C"
    {
    #include <commdlg.h>
   #ifndef WIN32
    #include <print.h>
    #include <dlgs.h>
   #endif
    }

//#include <classlib.h>
#endif


namespace binfilter {
//Types that OLE2.H et. al. leave out

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif  //PPVOID
#define CCHPATHMAX          256


#if defined(_OLE2_H_) || defined(_OLE2_H)   //May not include ole2.h at all times.

#ifndef PPOINTL
typedef POINTL * PPOINTL;
#endif  //PPOINTL


#ifndef _WIN32
#ifndef OLECHAR
typedef sal_Char OLECHAR;
typedef OLECHAR FAR* LPOLESTR;
typedef const OLECHAR FAR* LPCOLESTR;
#endif //OLECHAR
#endif //_WIN32


//Useful macros.
#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    }

#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    }


#define SETRECTL(rcl, l, t, r, b) \
    {\
    (rcl).left=l;\
    (rcl).top=t;\
    (rcl).right=r;\
    (rcl).bottom=b;\
    }

#define SETSIZEL(szl, h, v) \
    {\
    (szl).cx=h;\
    (szl).cy=v;\
    }


#define RECTLFROMRECT(rcl, rc)\
    {\
    (rcl).left=(long)(rc).left;\
    (rcl).top=(long)(rc).top;\
    (rcl).right=(long)(rc).right;\
    (rcl).bottom=(long)(rc).bottom;\
    }


#define RECTFROMRECTL(rc, rcl)\
    {\
    (rc).left=(int)(rcl).left;\
    (rc).top=(int)(rcl).top;\
    (rc).right=(int)(rcl).right;\
    (rc).bottom=(int)(rcl).bottom;\
    }


#define POINTLFROMPOINT(ptl, pt) \
    { \
    (ptl).x=(long)(pt).x; \
    (ptl).y=(long)(pt).y; \
    }


#define POINTFROMPOINTL(pt, ptl) \
    { \
    (pt).x=(int)(ptl).x; \
    (pt).y=(int)(ptl).y; \
    }

//Here's one that should be in windows.h
#define SETPOINT(pt, h, v) \
    {\
    (pt).x=h;\
    (pt).y=v;\
    }

#define SETPOINTL(ptl, h, v) \
    {\
    (ptl).x=h;\
    (ptl).y=v;\
    }

#endif  //_OLE2_H_

#ifdef INC_AUTOMATION

//Macros for setting DISPPARAMS structures
#define SETDISPPARAMS(dp, numArgs, pvArgs, numNamed, pNamed) \
    {\
    (dp).cArgs=numArgs;\
    (dp).rgvarg=pvArgs;\
    (dp).cNamedArgs=numNamed;\
    (dp).rgdispidNamedArgs=pNamed;\
    }

#define SETNOPARAMS(dp) SETDISPPARAMS(dp, 0, NULL, 0, NULL)

//Macros for setting EXCEPINFO structures
#define SETEXCEPINFO(ei, excode, src, desc, file, ctx, func, scd) \
    {\
    (ei).wCode=excode;\
    (ei).wReserved=0;\
    (ei).bstrSource=src;\
    (ei).bstrDescription=desc;\
    (ei).bstrHelpFile=file;\
    (ei).dwHelpContext=ctx;\
    (ei).pvReserved=NULL;\
    (ei).pfnDeferredFillIn=func;\
    (ei).scode=scd;\
    }


#define INITEXCEPINFO(ei) \
        SETEXCEPINFO(ei,0,NULL,NULL,NULL,0L,NULL,S_OK)

#endif


/*
 * State flags for IPersistStorage implementations.  These
 * are kept here to avoid repeating the code in all samples.
 */

typedef enum
    {
    PSSTATE_UNINIT,     //Uninitialized
    PSSTATE_SCRIBBLE,   //Scribble
    PSSTATE_ZOMBIE,     //No scribble
    PSSTATE_HANDSOFF    //Hand-off
    } PSSTATE;


/*
 * Identifers to describe which persistence model an object
 * is using, along with a union type that holds on the the
 * appropriate pointers that a client may need.
 */
typedef enum
    {
    PERSIST_UNKNOWN=0,
    PERSIST_STORAGE,
    PERSIST_STREAM,
    PERSIST_STREAMINIT,
    PERSIST_FILE
    } PERSIST_MODEL;

typedef struct
    {
    PERSIST_MODEL   psModel;
    union
        {
        IPersistStorage    *pIPersistStorage;
        IPersistStream     *pIPersistStream;
       #ifdef INC_CONTROLS
        IPersistStreamInit *pIPersistStreamInit;
       #endif
        IPersistFile       *pIPersistFile;
        } pIP;

    } PERSISTPOINTER, *PPERSISTPOINTER;


//To identify a storage in which to save, load, or create.
typedef struct
    {
    PERSIST_MODEL   psModel;
    union
        {
        IStorage    *pIStorage;
        IStream     *pIStream;
        } pIS;

    } STGPOINTER, *PSTGPOINTER;



//Type for an object-destroyed callback
typedef void (*PFNDESTROYED)(void);


//DeleteInterfaceImp calls 'delete' and NULLs the pointer
#define DeleteInterfaceImp(p)\
            {\
            if (NULL!=p)\
                {\
                delete p;\
                p=NULL;\
                }\
            }


//ReleaseInterface calls 'Release' and NULLs the pointer
#define ReleaseInterface(p)\
            {\
            if (NULL!=p)\
                {\
                p->Release();\
                p=NULL;\
                }\
            }


//OLE Documents Clipboard Formats

#define CFSTR_EMBEDSOURCE       TEXT("Embed Source")
#define CFSTR_EMBEDDEDOBJECT    TEXT("Embedded Object")
#define CFSTR_LINKSOURCE        TEXT("Link Source")
#define CFSTR_CUSTOMLINKSOURCE  TEXT("Custom Link Source")
#define CFSTR_OBJECTDESCRIPTOR  TEXT("Object Descriptor")
#define CFSTR_LINKSRCDESCRIPTOR TEXT("Link Source Descriptor")



//Functions in the helper DLL, SOOLE.DLL

//UI Effects
STDAPI_(void) UIDrawHandles(LPRECT, HDC, DWORD, UINT, BOOL);
STDAPI_(void) UIDrawShading(LPRECT, HDC, DWORD, UINT);
STDAPI_(void) UIShowObject(LPCRECT, HDC, BOOL);

//For UIDrawHandles
#define UI_HANDLES_USEINVERSE    0x00000001L
#define UI_HANDLES_NOBORDER      0x00000002L
#define UI_HANDLES_INSIDE        0x00000004L
#define UI_HANDLES_OUTSIDE       0x00000008L

//For UIDrawShading
#define UI_SHADE_FULLRECT        1
#define UI_SHADE_BORDERIN        2
#define UI_SHADE_BORDEROUT       3

//Coordinate Munging
STDAPI_(int)  XformWidthInHimetricToPixels(HDC,  int);
STDAPI_(int)  XformWidthInPixelsToHimetric(HDC,  int);
STDAPI_(int)  XformHeightInHimetricToPixels(HDC, int);
STDAPI_(int)  XformHeightInPixelsToHimetric(HDC, int);

STDAPI_(void) XformRectInPixelsToHimetric(HDC, LPRECT,  LPRECT);
STDAPI_(void) XformRectInHimetricToPixels(HDC, LPRECT,  LPRECT);
STDAPI_(void) XformSizeInPixelsToHimetric(HDC, LPSIZEL, LPSIZEL);
STDAPI_(void) XformSizeInHimetricToPixels(HDC, LPSIZEL, LPSIZEL);


//Helpers
STDAPI_(void)     StarObject_MetafilePictIconFree(HGLOBAL);
STDAPI            StarObject_SwitchDisplayAspect(IUnknown *, LPDWORD, DWORD
                      , HGLOBAL, BOOL, BOOL, IAdviseSink *, BOOL *);
STDAPI            StarObject_SetIconInCache(IUnknown *, HGLOBAL);
STDAPI_(UINT)     StarObject_GetUserTypeOfClass(REFCLSID, UINT, LPTSTR
                      , UINT);
STDAPI            StarObject_DoConvert(IStorage *, REFCLSID);
STDAPI_(LPTSTR)   StarObject_CopyString(LPTSTR);
STDAPI_(HGLOBAL)  StarObject_ObjectDescriptorFromOleObject(IOleObject *
                      , DWORD, POINTL, LPSIZEL);
STDAPI_(HGLOBAL)  StarObject_AllocObjectDescriptor(CLSID, DWORD
                      , SIZEL, POINTL, DWORD, LPTSTR, LPTSTR);
STDAPI_(IStorage *) StarObject_CreateStorageOnHGlobal(DWORD);
STDAPI            StarObject_GetLinkSourceData(IMoniker *, LPCLSID
                      , LPFORMATETC, LPSTGMEDIUM);
STDAPI_(void)     StarObject_RegisterAsRunning(IUnknown *, IMoniker *
                      , DWORD, LPDWORD);
STDAPI_(void)     StarObject_RevokeAsRunning(LPDWORD);
STDAPI_(void)     StarObject_NoteChangeTime(DWORD, FILETIME *, LPTSTR);



/*
 * These are for ANSI compilations on Win32.  Source code assumes
 * a Win32 Unicode environment (or Win16 Ansi).  These macros
 * route Win32 ANSI compiled functions to wrappers which do the
 * proper Unicode conversion.
 */


#ifdef WIN32ANSI
STDAPI StarObject_ConvertStringToANSI(LPCWSTR, LPSTR *);
STDAPI StarObject_StringFromCLSID(REFCLSID, LPSTR *);
STDAPI StarObject_StringFromGUID2(REFGUID, LPSTR, int);
STDAPI StarObject_ProgIDFromCLSID(REFCLSID, LPSTR *);

STDAPI StarObject_ReadFmtUserTypeStg(IStorage *, CLIPFORMAT *, LPSTR *);
STDAPI StarObject_WriteFmtUserTypeStg(IStorage *, CLIPFORMAT, LPSTR);

STDAPI StarObject_StgIsStorageFile(LPCSTR);
STDAPI StarObject_StgCreateDocfile(LPCSTR, DWORD, DWORD, IStorage **);
STDAPI StarObject_StgOpenStorage(LPCSTR, IStorage *, DWORD, SNB
           , DWORD, IStorage **);


STDAPI StarObject_CreateFileMoniker(LPCSTR, LPMONIKER *);
STDAPI StarObject_CreateItemMoniker(LPCSTR, LPCSTR, LPMONIKER *);
STDAPI StarObject_MkParseDisplayName(LPBC, LPCSTR, ULONG *, LPMONIKER *);

STDAPI StarObject_OleCreateLinkToFile(LPCSTR, REFIID, DWORD, LPFORMATETC
           , LPOLECLIENTSITE, LPSTORAGE, LPVOID *);
STDAPI StarObject_OleCreateFromFile(REFCLSID, LPCSTR, REFIID
           , DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, LPVOID *);

#ifdef skbdkbasdfbasbjdas
//#ifndef NOMACROREDIRECT
#undef StringFromCLSID
#define StringFromCLSID(c, pp) StarObject_StringFromCLSID(c, pp)

#undef StringFromGUID2
#define StringFromGUID2(r, p, i) StarObject_StringFromGUID2(r, p, i)

#undef ProgIDFromCLSID
#define ProgIDFromCLSID(c, pp) StarObject_ProgIDFromCLSID(c, pp)

#undef ReadFmtUserTypeStg
#define ReadFmtUserTypeStg(p, c, s) StarObject_ReadFmtUserTypeStg(p, c, s)

#undef WriteFmtUserTypeStg
#define WriteFmtUserTypeStg(p, c, s) StarObject_WriteFmtUserTypeStg(p, c, s)

#undef StgIsStorageFile
#define StgIsStorageFile(s) StarObject_StgIsStorageFile(s)

#undef StgCreateDocfile
#define StgCreateDocfile(a, b, c, d) StarObject_StgCreateDocfile(a, b, c, d)

#undef StgOpenStorage
#define StgOpenStorage(a,b,c,d,e,f) StarObject_StgOpenStorage(a,b,c,d,e,f)

#undef CreateFileMoniker
#define CreateFileMoniker(p, i) StarObject_CreateFileMoniker(p, i)

//#undef CreateItemMoniker
//#define CreateItemMoniker(p1, p2, i) StarObject_CreateItemMoniker(p1, p2, i)

#undef MkParseDisplayName
#define MkParseDisplayName(b, p, u, i) StarObject_MkParseDisplayName(b, p, u, i)

#undef OleCreateLinkToFile
#define OleCreateLinkToFile(s, i, d, fe, cs, st, pv) StarObject_OleCreateLinkToFile(s, i, d, fe, cs, st, pv)

#undef OleCreateFromFile
#define OleCreateFromFile(c, s, i, d, fe, cs, st, pv) StarObject_OleCreateFromFile(c, s, i, d, fe, cs, st, pv)

#endif

#endif

}

#endif //_StarObject_H_
