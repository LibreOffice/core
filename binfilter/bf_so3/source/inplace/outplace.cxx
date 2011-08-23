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

#include <com/sun/star/datatransfer/XSystemTransferable.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase2.hxx>

#ifdef WNT
#include <../ole/socont.h>
#include <vcl/sysdata.hxx>
#endif

#include <tools/debug.hxx>
#include <tools/cachestr.hxx>
#include <sot/storinfo.hxx>
#include <sot/stg.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svuidlg.hrc>
#include <bf_so3/svstor.hxx>
#include "bf_so3/soerr.hxx"
#include <soimpl.hxx>
#include "insdlg.hxx"
#include "bf_so3/outplace.hxx"
#include <viscache.hxx>
#include <osl/module.hxx>
#include <sot/formats.hxx>
#include <bf_svtools/filter.hxx>
#include <comphelper/classids.hxx>
#include <rtl/process.h>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <bf_svtools/wmf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

namespace binfilter {

static UINT32 nUniqueId = 1;
#ifdef WNT
static BOOL	bOleInited = FALSE;
inline void InitOle()
{
    if( !bOleInited )
    {
        OleInitialize( NULL );
        bOleInited = TRUE;
    }
}

class OLEWrapper_Impl : public cppu::WeakImplHelper2
<
    com::sun::star::lang::XComponent,
    com::sun::star::lang::XUnoTunnel
>
{
    CSO_Cont* pOleContent;

public:

    OLEWrapper_Impl( CSO_Cont* pCont )
        : pOleContent( pCont )
    {}

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException) ;

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
};

//sal_Int64 SAL_CALL OLEWrapper_Impl::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException) ;

sal_Int64 SAL_CALL OLEWrapper_Impl::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    SvGlobalName aName;
    aName.MakeFromMemory( (void*) aIdentifier.getConstArray() );
    if ( aName.GetCLSID() == pOleContent->GetCLSID() )
    {
        return (sal_Int64) (IUnknown*) pOleContent->GetOleObj();
    }
    else
        return 0;
}

void SAL_CALL OLEWrapper_Impl::dispose() throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL OLEWrapper_Impl::addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >&) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL OLEWrapper_Impl::removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >&) throw (::com::sun::star::uno::RuntimeException)
{
}
#endif

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*************************************************************************/
BOOL Impl_OlePres::Read( SvStream & rStm )
{
    ULONG nBeginPos = rStm.Tell();
    INT32 n;
    rStm >> n;
    if( n != -1 )
    {
        pBmp = new Bitmap;
        rStm >> *pBmp;
        if( rStm.GetError() == SVSTREAM_OK )
        {
            nFormat = FORMAT_BITMAP;
            aSize = pBmp->GetPrefSize();
            MapMode aMMSrc;
            if( !aSize.Width() || !aSize.Height() )
            {
                // letzte Chance
                aSize = pBmp->GetSizePixel();
                aMMSrc = MAP_PIXEL;
            }
            else
                aMMSrc = pBmp->GetPrefMapMode();
            MapMode aMMDst( MAP_100TH_MM );
            aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
            return TRUE;
        }
        else
        {
            delete pBmp;
            pBmp = NULL;

            pMtf = new GDIMetaFile();
            rStm.ResetError();
            rStm >> *pMtf;
            if( rStm.GetError() == SVSTREAM_OK )
            {
                nFormat = FORMAT_GDIMETAFILE;
                aSize = pMtf->GetPrefSize();
                MapMode aMMSrc = pMtf->GetPrefMapMode();
                MapMode aMMDst( MAP_100TH_MM );
                aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
                return TRUE;
            }
            else
            {
                delete pMtf;
                pMtf = NULL;
            }
        }

    }

    rStm.ResetError();
    rStm.Seek( nBeginPos );
    nFormat = ReadClipboardFormat( rStm );
    // JobSetup, bzw. TargetDevice ueberlesen
    // Information aufnehmen, um sie beim Schreiben nicht zu verlieren
    nJobLen = 0;
    rStm >> nJobLen;
    if( nJobLen >= 4 )
    {
        nJobLen -= 4;
        if( nJobLen )
        {
            pJob = new BYTE[ nJobLen ];
            rStm.Read( pJob, nJobLen );
        }
    }
    else
    {
        rStm.SetError( SVSTREAM_GENERALERROR );
        return FALSE;
    }
    UINT32 nAsp;
    rStm >> nAsp;
    USHORT nSvAsp = USHORT( nAsp );
    SetAspect( nSvAsp );
    rStm.SeekRel( 4 ); //L-Index ueberlesen
    rStm >> nAdvFlags;
    rStm.SeekRel( 4 ); //Compression
    UINT32 nWidth  = 0;
    UINT32 nHeight = 0;
    UINT32 nSize   = 0;
    rStm >> nWidth >> nHeight >> nSize;
    aSize.Width() = nWidth;
    aSize.Height() = nHeight;

    if( nFormat == FORMAT_GDIMETAFILE )
    {
        pMtf = new GDIMetaFile();
        ReadWindowMetafile( rStm, *pMtf, NULL );
    }
    else if( nFormat == FORMAT_BITMAP )
    {
        pBmp = new Bitmap();
        rStm >> *pBmp;
    }
    else
    {
        BYTE* p = new BYTE[ nSize ];
        rStm.Read( p, nSize );
        delete [] p;
        return FALSE;
    }
    return TRUE;
}

/************************************************************************/
void Impl_OlePres::Write( SvStream & rStm )
{
    WriteClipboardFormat( rStm, FORMAT_GDIMETAFILE );
    rStm << (INT32)(nJobLen +4);       // immer leeres TargetDevice
    if( nJobLen )
        rStm.Write( pJob, nJobLen );
    rStm << (UINT32)nAspect;
    rStm << (INT32)-1;      //L-Index immer -1
    rStm << (INT32)nAdvFlags;
    rStm << (INT32)0;       //Compression
    rStm << (INT32)aSize.Width();
    rStm << (INT32)aSize.Height();
    ULONG nPos = rStm.Tell();
    rStm << (INT32)0;

    if( GetFormat() == FORMAT_GDIMETAFILE && pMtf )
    {
        // Immer auf 1/100 mm, bis Mtf-Loesung gefunden
        // Annahme (keine Skalierung, keine Org-Verschiebung)
        DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleX() == Fraction( 1, 1 ),
                    "X-Skalierung im Mtf" );
        DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleY() == Fraction( 1, 1 ),
                    "Y-Skalierung im Mtf" );
        DBG_ASSERT( pMtf->GetPrefMapMode().GetOrigin() == Point(),
                    "Origin-Verschiebung im Mtf" );
        MapUnit nMU = pMtf->GetPrefMapMode().GetMapUnit();
        if( MAP_100TH_MM != nMU )
        {
            Size aPrefS( pMtf->GetPrefSize() );
            Size aS( aPrefS );
            aS = OutputDevice::LogicToLogic( aS, nMU, MAP_100TH_MM );

            pMtf->Scale( Fraction( aS.Width(), aPrefS.Width() ),
                         Fraction( aS.Height(), aPrefS.Height() ) );
            pMtf->SetPrefMapMode( MAP_100TH_MM );
            pMtf->SetPrefSize( aS );
        }
        WriteWindowMetafileBits( rStm, *pMtf );
    }
    else
    {
        DBG_ERROR( "unknown format" );
    }
    ULONG nEndPos = rStm.Tell();
    rStm.Seek( nPos );
    rStm << (UINT32)(nEndPos - nPos - 4);
    rStm.Seek( nEndPos );
}

Impl_OlePres * CreateCache_Impl( SotStorage * pStor )
{
    SotStorageStreamRef xOleObjStm =pStor->OpenSotStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                                        STREAM_READ | STREAM_NOCREATE );
    if( xOleObjStm->GetError() )
        return NULL;
    SotStorageRef xOleObjStor = new SotStorage( *xOleObjStm );
    if( xOleObjStor->GetError() )
        return NULL;

    String aStreamName;
    if( xOleObjStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) );
    else if( xOleObjStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) );

    if( aStreamName.Len() == 0 )
        return NULL;


    for( USHORT i = 1; i < 10; i++ )
    {
        SotStorageStreamRef xStm = xOleObjStor->OpenSotStream( aStreamName,
                                                STREAM_READ | STREAM_NOCREATE );
        if( xStm->GetError() )
            break;

        xStm->SetBufferSize( 8192 );
        Impl_OlePres * pEle = new Impl_OlePres( 0 );
        if( pEle->Read( *xStm ) && !xStm->GetError() )
        {
            if( pEle->GetFormat() == FORMAT_GDIMETAFILE || pEle->GetFormat() == FORMAT_BITMAP )
                return pEle;
        }
        delete pEle;
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres00" ) );
        aStreamName += String( i );
    };
    return NULL;
}


//=========================================================================
//=========================================================================
//=========================================================================
struct SvOutPlace_Impl
/*	[Beschreibung]

    In diesem struct werden sich Member von SvOutPlaceObject gemerkt, um
    nicht immer inkompatibel zu werden.
*/
{
    SvVerbList		aVerbs;
    Impl_OlePres *	pOP;
    UINT32			dwAspect;
    bool			bSetExtent;
    SvStorageRef	xWorkingStg;
    BOOL			bGetVisAreaFromInfoEle; // can only by read after load in FF 4.0
    BOOL			bGetVisAreaFromOlePress; // can only by read if OlePress representation is there
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > m_xComponent;
#ifdef WNT
    CSO_Cont *		pSO_Cont;
#endif

    SvOutPlace_Impl()
        : pOP( NULL )
        , dwAspect( ASPECT_CONTENT )
        , bSetExtent( false )
        , bGetVisAreaFromInfoEle( FALSE )
        , bGetVisAreaFromOlePress( FALSE )
#ifdef WNT
        , pSO_Cont( NULL )
#endif
    {}
};

//=========================================================================
//============== SvOutPlaceObject ===========================================
//=========================================================================
//SO2_IMPL_BASIC_CLASS1_DLL( SvOutPlaceObject, SvFactory, SvInPlaceObject,
SO2_IMPL_BASIC_CLASS1( SvOutPlaceObject, SvFactory, SvInPlaceObject,
              SvGlobalName( SO3_OUT_CLASSID ) )

//=========================================================================
SvOutPlaceObject::SvOutPlaceObject()
    : pImpl( new SvOutPlace_Impl )
/*	[Beschreibung]

    Konstruktor der Klasse SvOutPlaceObject. Die Verbliste und das
    Format werden global initialisiert.
*/
{
}

//=========================================================================
SvOutPlaceObject::~SvOutPlaceObject()
/*	[Beschreibung]

    Destruktor der Klasse SvOutPlaceObject.
*/
{
#ifdef WNT
    if( pImpl->pSO_Cont )
    {
        pImpl->pSO_Cont->Close( FALSE );
        pImpl->pSO_Cont->Release();
        if ( pImpl->m_xComponent.is() )
        {
            pImpl->m_xComponent->dispose();
            pImpl->m_xComponent = NULL;
        }
    }

#endif
    delete pImpl->pOP;
    delete pImpl;
}

#ifdef WNT
void SvOutPlaceObject::ClearCache()
{
    delete pImpl->pOP;
    pImpl->pOP = NULL;
}

SotStorage * SvOutPlaceObject::GetWorkingStorage()
{
    return pImpl->xWorkingStg;
}
#endif

//=========================================================================
const SvVerbList & SvOutPlaceObject::GetVerbList() const
/*	[Beschreibung]

    Liefert eine Liste der Aktionen zurueck, die auf diesem Objekt ausgefuehrt werden koennen.
*/
{
#ifdef WNT
    if( !pImpl->aVerbs.Count() )
    {
        InitOle();
        ((SvOutPlaceObject *)this)->LoadSO_Cont();
        if( pImpl->pSO_Cont && pImpl->pSO_Cont->GetOleObj() )
        {
            LPENUMOLEVERB pEnum;
            if( SUCCEEDED( pImpl->pSO_Cont->GetOleObj()->EnumVerbs( &pEnum ) ) )
            {
                #define     MAX_ELE     20
                OLEVERB     szEle[ MAX_ELE ];
                HRESULT     nErr;
                ULONG       nNo;
                // Aktuelle Elemente holen und hochzaehlen
                do
                {
                    nErr = pEnum->Next( MAX_ELE, szEle, &nNo );
                    if( S_OK == GetScode( nErr )
                      || S_FALSE == GetScode( nErr ) )
                    {
                        for( ULONG i = 0; i < nNo; i++ )
                        {
                            sal_Unicode * pName = reinterpret_cast<sal_Unicode*>(szEle[ i ].lpszVerbName);
                            if( pName )
                            {
                                String aName;
                                while( *pName )
                                    aName += *pName++;

                                // Windows accelerator to vcl accelerator
                                const sal_Unicode* pStr = aName.GetBuffer();
                                USHORT      n    = 0;
                                while ( *pStr )
                                {
                                    if ( *pStr == '&' )
                                    {
                                        if ( *(pStr+1) == '&' )
                                        {
                                            aName.Erase( n, 1 );
                                            pStr = aName.GetBuffer() + n;
                                        }
                                        else
                                        {
                                            aName.SetChar(n, '~' );
                                            pStr = aName.GetBuffer() + n;
                                        }
                                    }
                                    else
                                    {
                                        if ( *pStr == '~' )
                                        {
                                            aName.Insert( '~', n );
                                            pStr = aName.GetBuffer() + n;
                                            pStr++;
                                            n++;
                                        }
                                    }
                                    pStr++;
                                    n++;
                                }

                                BOOL      bNeverDirty = FALSE;
                                BOOL      bOnContMenu = FALSE;
                                if( szEle[ i ].grfAttribs & OLEVERBATTRIB_NEVERDIRTIES )
                                    bNeverDirty = TRUE;
                                if( szEle[ i ].grfAttribs & OLEVERBATTRIB_ONCONTAINERMENU )
                                    bOnContMenu = TRUE;
                                SvVerb aVerb( szEle[ i ].lVerb,
                                              aName,
                                              bNeverDirty,
                                              bOnContMenu );
                                pImpl->aVerbs.Append( aVerb );
                            }
                        }
                    }
                    else
                        break;
                }
                while( nNo == MAX_ELE );
                pEnum->Release();
            }
        }
    }
#endif
    return pImpl->aVerbs;
}

//=========================================================================
void SvOutPlaceObject::DataChanged_Impl
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
        if( bOnlyEmbedSource )
            ViewChanged( ASPECT_CONTENT );
    }
}

//=========================================================================
#ifdef WNT
void SvOutPlaceObject::LoadSO_Cont()
{
    if( !pImpl->pSO_Cont && pImpl->xWorkingStg->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ) ) )
    {
        Rectangle aVisRect = GetVisArea( ASPECT_CONTENT );
        //Rectangle aVisRect = GetVisArea( (UINT16)pImpl->dwAspect );
        RECTL aRect;
        aRect.top    = aVisRect.Top();
        aRect.left   = aVisRect.Left();
        if( aVisRect.Right() == RECT_EMPTY )
            aRect.right  = aRect.left;
        else
            aRect.right  = (aVisRect.Right() +1);
        if( aVisRect.Bottom() == RECT_EMPTY )
            aRect.bottom = aRect.top;
        else
            aRect.bottom = (aVisRect.Bottom() +1);

        InitOle();
        pImpl->pSO_Cont = new CSO_Cont( 1, NULL, this );
        pImpl->pSO_Cont->AddRef();
        pImpl->pSO_Cont->Load( pImpl->xWorkingStg, pImpl->dwAspect, pImpl->bSetExtent, aRect );
    }
}
#endif

//=========================================================================
void SvOutPlaceObject::FillClass
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
    *pFormat    	= SOT_FORMATSTR_ID_OUTPLACE_OBJ;
    *pAppName		= String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "OutPlace Object 1.0" ) );
    *pFullTypeName  = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "OutPlace Object 1.0" ) );
    *pShortTypeName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "OutPlace Object" ) );
}


#ifdef WNT
extern "C" {
typedef UINT STDAPICALLTYPE OleUIInsertObjectW_Type(LPOLEUIINSERTOBJECTW);
typedef UINT STDAPICALLTYPE OleUIInsertObjectA_Type(LPOLEUIINSERTOBJECTA);
}
#endif

SvInPlaceObjectRef SvOutPlaceObject::InsertObject
(
    Window *,
    SvStorage * pIStorage,
    BOOL & bOk,
    const SvGlobalName& rName,
    String & rFileName
)
{
    SvOutPlaceObjectRef xRet;
    bOk = TRUE;
    //rTypeName.Erase();
    rFileName.Erase();
#ifdef WNT
    InitOle();
    OLEUIINSERTOBJECT   io;
    DWORD               dwData=0;
    TCHAR               szFile[CCHPATHMAX];

    memset(&io, 0, sizeof(io));

    io.cbStruct=sizeof(io);
    io.hWndOwner=GetActiveWindow();

    szFile[0]=0;
    io.lpszFile=szFile;
    io.cchFile=CCHPATHMAX;

    io.dwFlags=IOF_SELECTCREATENEW | IOF_DISABLELINK;
    io.clsid = rName.GetCLSID();

    osl::Module aOleDlgLib;
    if( !aOleDlgLib.load( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "oledlg" ) ) ) )
        return &xRet;

#ifdef UNICODE
    OleUIInsertObjectW_Type * pInsertFct = (OleUIInsertObjectW_Type *)
                                aOleDlgLib.getSymbol( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "OleUIInsertObjectW" ) ) );
#else
    OleUIInsertObjectA_Type * pInsertFct = (OleUIInsertObjectA_Type *)
                                aOleDlgLib.getSymbol( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "OleUIInsertObjectA" ) ) );
#endif
    if( !pInsertFct )
        return &xRet;

    //uTemp=pInsertFct(&io);
    //uTemp=OleUIInsertObject(&io);

    //if (OLEUI_OK==uTemp)
    {
        TENANTTYPE      tType;
        LPVOID          pv;
        FORMATETC       fe;

        SETDefFormatEtc(fe, 0, TYMED_NULL);

        if (io.dwFlags & IOF_SELECTCREATENEW)
            {
            tType=TENANTTYPE_EMBEDDEDOBJECT;
            pv=&io.clsid;
            }
        else
            {
            tType=TENANTTYPE_EMBEDDEDFILE;
            pv=szFile;
            rFileName.Assign( String( szFile, gsl_getSystemTextEncoding() ) );
            }

        if ((io.dwFlags & IOF_CHECKDISPLAYASICON)
            && NULL!=io.hMetaPict)
            {
            fe.dwAspect=DVASPECT_ICON;
            dwData=(DWORD)(UINT)io.hMetaPict;
            }

        xRet = new SvOutPlaceObject();
        xRet->DoInitNew( pIStorage );

        xRet->pImpl->pSO_Cont = new CSO_Cont( 1, NULL, xRet );
        xRet->pImpl->pSO_Cont->AddRef();

        POINTL      ptl;
        SIZEL       szl;
        UINT uRet = xRet->pImpl->pSO_Cont->Create(tType, pv, &fe, &ptl, &szl, pIStorage, NULL, dwData);
        if (CREATE_FAILED==uRet)
        {
            xRet = SvOutPlaceObjectRef();
            bOk = FALSE;
        }
        else
        {
            xRet->pImpl->pSO_Cont->Invalidate();
            if( tType == TENANTTYPE_EMBEDDEDFILE )
                xRet->pImpl->pSO_Cont->Update();

            //RECTL rcl;
            //SETRECTL(rcl, 0, 0, szl.cx, szl.cy);
            //xRet->pImpl->pSO_Cont->RectSet(&rcl, FALSE, TRUE);
            xRet->SetVisAreaSize( Size( szl.cx, szl.cy ) );
            WIN_BOOL fSetExtent;
            xRet->pImpl->pSO_Cont->GetInfo( xRet->pImpl->dwAspect, fSetExtent );
            xRet->pImpl->bSetExtent = fSetExtent;

            //Free this regardless of what we do with it.
            StarObject_MetafilePictIconFree(io.hMetaPict);
        }
    }
#else
    (void)pIStorage;
    (void)rName;
#endif
    return &xRet;
}


//=========================================================================
struct SvObjectServerListHolder {
    ::binfilter::SvObjectServerList	mObjList;
    SvObjectServerListHolder() { mObjList.FillInsertObjects(); }
};

const ::binfilter::SvObjectServer* SvOutPlaceObject::GetInternalServer_Impl( const SvGlobalName& aGlobName )
{
    static SvObjectServerListHolder aObj;

    const ::binfilter::SvObjectServer* pObjServ = NULL;

    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SW_CLASSID_60 ) );
    else if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SC_CLASSID_60 ) );
    else if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SIMPRESS_CLASSID_60 ) );
    else if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SDRAW_CLASSID_60 ) );
    else if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SM_CLASSID_60 ) );
    else if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 ) )
        pObjServ = aObj.mObjList.Get( SvGlobalName( SO3_SCH_CLASSID_60 ) );

    return pObjServ;
}

#ifdef WNT

/* The function creates an wrapper for an MS Ole object. The argument xTrans provides the data
   needed for the creation of the MS OLE object.
*/
SvInPlaceObjectRef   SvOutPlaceObject::CreateFromData( const Reference<XTransferable>& xTrans,
                                                SvStorage* pIStorage)
{
    SvOutPlaceObjectRef xRet;
    InitOle();
    IDataObject * pDO = NULL;
    Reference<XSystemTransferable> xSys( xTrans, UNO_QUERY);
    if( xSys.is())
    {
        sal_uInt8 arId[16];
        rtl_getGlobalProcessId( arId);

        Any data= xSys->getData( Sequence<sal_Int8>( (sal_Int8*) arId, 16));
        sal_uInt32 npData;
        if( data >>= npData)
        {
            pDO= reinterpret_cast<IDataObject*>( npData);

            TENANTTYPE tType;
            HRESULT hr = OleQueryCreateFromData( pDO );
            if( S_OK == GetScode( hr ) )
                tType=TENANTTYPE_EMBEDDEDOBJECTFROMDATA;
            else if( OLE_S_STATIC == GetScode( hr ) )
                tType=TENANTTYPE_STATIC;
            else
            {
                pDO->Release();
                return &xRet;
            }
            xRet = new SvOutPlaceObject();
            xRet->DoInitNew( pIStorage );

            xRet->pImpl->pSO_Cont = new CSO_Cont( 1, NULL, xRet );
            xRet->pImpl->pSO_Cont->AddRef();
            POINTL			ptl;
            SIZEL			szl;
            FORMATETC       fe;
            SETDefFormatEtc(fe, 0, TYMED_NULL);
            UINT uRet = xRet->pImpl->pSO_Cont->Create(tType, pDO, &fe, &ptl, &szl, pIStorage, NULL, 0 );

            if (CREATE_FAILED==uRet)
                xRet = SvOutPlaceObjectRef();
            else
            {
                xRet->pImpl->pSO_Cont->Update();
                xRet->pImpl->pSO_Cont->Invalidate();
                //RECTL rcl;
                //SETRECTL(rcl, 0, 0, szl.cx, szl.cy);
                //xRet->pImpl->pSO_Cont->RectSet(&rcl, FALSE, TRUE);
                xRet->SetVisAreaSize( Size( szl.cx, szl.cy ) );
                WIN_BOOL fSetExtent;
                xRet->pImpl->pSO_Cont->GetInfo( xRet->pImpl->dwAspect, fSetExtent );
                xRet->pImpl->bSetExtent = fSetExtent;
            }
            pDO->Release();
        }
    }
    return &xRet;

}

//=========================================================================
SvGlobalName	SvOutPlaceObject::GetCLSID( const String & rFileName )
{
    SvGlobalName aCLSID;
    InitOle();
    OLECHAR               szFile[CCHPATHMAX];

    if( rFileName.Len() >= CCHPATHMAX )
        return aCLSID;

    const sal_Unicode * pStr = rFileName.GetBuffer();
    int i = 0;
    while( *pStr )
        szFile[i++] = *pStr++;
    szFile[i]=0;

    CLSID aClsId;
    if( SUCCEEDED( GetClassFile( szFile, &aClsId ) ) )
    {
        // this method should return no CLSID in case of native XML format
        SvGlobalName aFromWin = SvGlobalName( aClsId );
        if ( !GetInternalServer_Impl( aFromWin ) )
            aCLSID = SvGlobalName( aClsId );
    }
    return aCLSID;
}
#endif

//=========================================================================
void SvOutPlaceObject::Open( BOOL bOpen )
{
#ifdef WNT
    if( !bOpen )
    {
        if( pImpl->pSO_Cont )
        {
            WIN_BOOL fSetExtent;
            pImpl->pSO_Cont->GetInfo( pImpl->dwAspect, fSetExtent );
            pImpl->bSetExtent = fSetExtent;
            pImpl->pSO_Cont->Close( IsAutoSave() && IsModified() && !IsHandsOff() );
            pImpl->pSO_Cont->Release();
            pImpl->pSO_Cont = NULL;
            if ( pImpl->m_xComponent.is() )
            {
                pImpl->m_xComponent->dispose();
                pImpl->m_xComponent = NULL;
            }
        }
    }
#endif
    SvInPlaceObject::Open( bOpen );
}

//=========================================================================
void SvOutPlaceObject::Embed
(
    BOOL bActivate	/* TRUE, OutPlace Aktivierung beginnt.
                       FALSE, OutPlace Aktivierung endet. */
)
/*	[Beschreibung]

    Das Objekt wird PutPlace aktiviert oder deaktiviert.

    [Querverweise]

    <SvInPlaceObject::Embed()>
*/
{
    SvInPlaceObject::Embed( bActivate );
#ifdef WNT
    if( !bActivate )
    {
        if( pImpl->pSO_Cont )
        {
            SIZEL aS;
            if ( pImpl->pSO_Cont->GetExtent( &aS ) )
                SvInPlaceObject::SetVisArea( Rectangle( Point(), Size( aS.cx, aS.cy ) ) );
        }
    }
#endif
}

//=========================================================================
ErrCode SvOutPlaceObject::Verb
(
    long 				nVerb,		/* welche Art des Aktivierens ist
                                       gew"unscht */
    SvEmbeddedClient *,				/* Callback-Svhnittstelle des Aufruffers */
    Window * 			pWin,		/* Parent auf dem aktiviert werden soll */
    const Rectangle *				/* Position und Gr"osse, an der das Objekt
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
    ErrCode nRet = ERRCODE_SO_GENERALERROR;
#ifdef WNT
    DoOpen( TRUE );
    LoadSO_Cont();
    if( pImpl->pSO_Cont )
    {
        HWND hWnd = NULL;
        if( pWin )
        {
            const SystemChildData*	pCD = pWin->GetSystemData();
            if( pCD )
                // hier wird das C++-Wrapper-Objekt fuer ein Java-Objekt erzeugt
                hWnd = (HWND)pCD->hWnd;
        }
        if( pImpl->pSO_Cont->Activate(nVerb, hWnd) )
        {
            pImpl->pSO_Cont->Update();
            nRet = ERRCODE_NONE;
        }
    }
#else
    (void)pWin;
    (void)nVerb;
#endif
/*
    switch( nVerb )
    {
        case SVVERB_SHOW:
        case SVVERB_OPEN:
        case 0L:
            nRet = GetProtocol().EmbedProtocol();
            break;
        case SVVERB_HIDE:
            nRet = ERRCODE_NONE;
            break;
        default:
            ErrCode nErr = ERRCODE_SO_GENERALERROR;
            break;
    }
*/
    return nRet;
}

//=========================================================================
void SvOutPlaceObject::SetVisArea
(
    const Rectangle & rVisArea	/* neue Position und Groesse des
                                   sichtbaren Ausschnitts */
)
/*	[Beschreibung]

    Der sichtbare Ausschnitt beginnt immer an der Position (0, 0).
*/
{
    Rectangle aOldVisArea = GetVisArea(
        sal::static_int_cast< USHORT >( GetViewAspect() ) );
    if( rVisArea.GetSize() != aOldVisArea.GetSize() )
    {
        if( !aOldVisArea.IsEmpty() )
            pImpl->bSetExtent = true;

        aOldVisArea.SetSize( rVisArea.GetSize() );
        SvInPlaceObject::SetVisArea( aOldVisArea );
#ifdef WNT
        if( pImpl->pSO_Cont )
        {
            SIZEL aS = { rVisArea.GetSize().Width(), rVisArea.GetSize().Height() };
            pImpl->pSO_Cont->SizeSet( &aS, FALSE, TRUE );
        }
#endif

        DataChanged_Impl( TRUE );
    }
}

//=========================================================================
Rectangle SvOutPlaceObject::GetVisArea( USHORT nAspect ) const
{
    if( pImpl->bGetVisAreaFromInfoEle )
    {
        pImpl->bGetVisAreaFromInfoEle = FALSE;
        SvPersist * pPar = GetParent();
        if( pPar )
        {
            SvInfoObject * p = pPar->Find( this );
            SvEmbeddedInfoObject * pEle = PTR_CAST( SvEmbeddedInfoObject, p );
            if( pEle )
            {
                BOOL bIsEnableSetModified = IsEnableSetModified();
                if( bIsEnableSetModified )
                    ((SvOutPlaceObject *)this)->EnableSetModified( FALSE );
                ((SvOutPlaceObject *)this)->SetVisArea( pEle->GetInfoVisArea() );
                if( bIsEnableSetModified )
                    ((SvOutPlaceObject *)this)->EnableSetModified( TRUE );
            }
            else if( p )
                // a bug in 4.0 file formats, save SvInfoObject instead of SvEmbeddedInfoObject
                ((SvOutPlaceObject *)this)->SvInPlaceObject::SetVisArea( Rectangle( Point(), Size( 5000, 5000 ) ) );
        }
    }
    else if( pImpl->bGetVisAreaFromOlePress )
    {
        pImpl->bGetVisAreaFromOlePress = FALSE;

        if( !pImpl->pOP )
            pImpl->pOP = CreateCache_Impl( pImpl->xWorkingStg );

        if( pImpl->pOP )
        {
            GDIMetaFile * pMtf = pImpl->pOP->GetMetaFile();
            if( pMtf )
            {
                ((SvOutPlaceObject *)this)->SetVisArea( Rectangle( Point(), pMtf->GetPrefSize() ) );
            }
            else
            {
                Bitmap * pBmp = pImpl->pOP->GetBitmap();
                if( pBmp )
                    ((SvOutPlaceObject *)this)->SetVisArea( Rectangle( Point(), pBmp->GetPrefSize() ) );
            }
        }
    }

    Rectangle aRect = SvInPlaceObject::GetVisArea( nAspect );
#ifdef WNT
    if( pImpl->pSO_Cont && nAspect == GetViewAspect() )
    {
        SIZEL aS;
        if ( pImpl->pSO_Cont->GetExtent( &aS ) )
            aRect.SetSize( Size( aS.cx, aS.cy ) );
    }
#endif
    return aRect;
}

UINT32 SvOutPlaceObject::GetViewAspect() const
{
    if ( pImpl->dwAspect )
        return pImpl->dwAspect;

    SvPersist * pPar = GetParent();
    if( pPar )
    {
        SvInfoObject * p = pPar->Find( this );
        SvEmbeddedInfoObject * pEle = PTR_CAST( SvEmbeddedInfoObject, p );
        if( pEle )
        {
            pImpl->dwAspect = pEle->GetInfoViewAspect();
            return pImpl->dwAspect;
        }
    }

    return ASPECT_CONTENT;
}


//=========================================================================
// aus PlugIn
void SoPaintReplacement( const Rectangle &rRect, String &rText,
                         OutputDevice *pOut );

//=========================================================================
void SvOutPlaceObject::ViewChanged( USHORT nAspects )
{
    SvInPlaceObject::ViewChanged( nAspects );
}

//=========================================================================
void SvOutPlaceObject::Draw
(
    OutputDevice * 	pDev,	/* in dieses Device findet die Ausgabe statt */
    const JobSetup &,		/* fuer dieses Device soll formatiert werden */
    USHORT					/* Darstellungsvariante des Objektes */
)
/*	[Beschreibung]

    Ein Ausgabe ist nicht m"oglich. Deswegen wird eine Bitmap
    und als Unterschrift der URL ausgegeben,

    [Querverweise]

    <SvInPlaceObject::Draw>
*/
{
    if( !pImpl->pOP )
        pImpl->pOP = CreateCache_Impl( pImpl->xWorkingStg );

    Rectangle aOutRect = GetVisArea( ASPECT_CONTENT );
    if( pImpl->pOP )
    {
        GDIMetaFile * pMtf = pImpl->pOP->GetMetaFile();
        if( pMtf )
        {
            pMtf->WindStart();
            pMtf->Play( pDev, aOutRect.TopLeft(), aOutRect.GetSize() );
        }
        else
        {
            Bitmap * pBmp = pImpl->pOP->GetBitmap();
            if( pBmp )
                pDev->DrawBitmap( aOutRect.TopLeft(), aOutRect.GetSize(), *pBmp );
        }

        return;
    }
    else
    {
#ifdef WNT
        BOOL bPlayed = FALSE;
        if (!pImpl->pSO_Cont)
            LoadSO_Cont();
        if( pImpl->pSO_Cont )
        {
            long nMapMode;
            Size aSize;
            HMETAFILE hMet;
            if ( pImpl->pSO_Cont->GetMetaFile( nMapMode, aSize, hMet ) )
            {
                ULONG nBufSize = GetMetaFileBitsEx( hMet, 0, NULL );
                unsigned char* pBuf = new unsigned char[nBufSize+22];
                *((long*)pBuf) = 0x9ac6cdd7L;
                *((short*)(pBuf+6)) = (SHORT) 0;
                *((short*)(pBuf+8)) = (SHORT) 0;
                *((short*)(pBuf+10)) = (SHORT) aSize.Width();
                *((short*)(pBuf+12)) = (SHORT) aSize.Height();
                *((short*)(pBuf+14)) = (USHORT) 2540;
                if ( nBufSize && nBufSize == GetMetaFileBitsEx( hMet, nBufSize, pBuf+22 ) )
                {
                    SvMemoryStream aStream( pBuf, nBufSize+22, STREAM_READ );
                    //SvFileStream aFile(String::CreateFromAscii("file:///d:/test.wmf"), STREAM_STD_READWRITE);
                    //aStream >> aFile;
                    aStream.Seek(0);
                    GDIMetaFile aMtf;
                    if( ReadWindowMetafile( aStream, aMtf, NULL ) )
                    {
                        aMtf.WindStart();
                        MapMode aMode( MAP_100TH_MM );
                        Size aSize = OutputDevice::LogicToLogic( aOutRect.GetSize(), aMode, pDev->GetMapMode() );
    //						AllSettings aNew( pDev->GetSettings() );
    //						StyleSettings aSet ( aNew.GetStyleSettings() );
    //						aSet.SetAntialiasingMinPixelHeight( 5 );
    //						aNew.SetStyleSettings( aSet );
    //						pDev->SetSettings( aNew );

                        //Point aPoint;
                        //Size aSize( aOutRect.GetSize() );
                        aMtf.Play( pDev, Point(), aSize );
                        bPlayed = TRUE;
                    }
                }

                DeleteMetaFile( hMet );
                delete[] pBuf;
            }
        }

        if ( !bPlayed )
#endif
        {
            Rectangle aVisArea_ = GetVisArea( ASPECT_CONTENT );
            String a = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "SvOutPlaceObject" ) );
            SoPaintReplacement( aVisArea_, a, pDev );
        }
    }
}


//=========================================================================
BOOL SvOutPlaceObject::InitNew
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
    pImpl->xWorkingStg = pStor;
    if( SvInPlaceObject::InitNew( pStor ) )
    {
        // Standardgr"osse
        SetVisArea( Rectangle( Point(), Size( 5000, 5000 ) ) );
        return TRUE;
    }
    return FALSE;
}

//=========================================================================
#define DOCNAME "OutPlace Object"

BOOL SvOutPlaceObject::Load
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
    pImpl->xWorkingStg = pStor;
    if( SvInPlaceObject::Load( pStor ) )
    {
        BOOL bNewVersion = pStor->IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ) );
        if( bNewVersion )
        {
            SvStorageStreamRef xStm;
            xStm = pImpl->xWorkingStg->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ), STREAM_STD_READ );
            xStm->SetVersion( pImpl->xWorkingStg->GetVersion() );
            xStm->SetBufferSize( 8192 );

            // nicht vorhandener Stream ist kein Fehler
            if( xStm->GetError() == SVSTREAM_FILE_NOT_FOUND )
            {
                pImpl->bGetVisAreaFromOlePress = TRUE;
                pImpl->dwAspect = 0;

                return TRUE;
            }

            UINT16 nLen;
            *xStm >> nLen;
            *xStm >> pImpl->dwAspect;
            BOOL b;
            *xStm >> b;
            pImpl->bSetExtent = b;

            if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40  || pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 )
            {
                pImpl->xWorkingStg = new SvStorage( FALSE, String(), STREAM_STD_READWRITE, STORAGE_DELETEONRELEASE );
                pStor->CopyTo( pImpl->xWorkingStg );
            }

            return xStm->GetError() == ERRCODE_NONE;
        }
        else
        {
            pImpl->bGetVisAreaFromInfoEle = TRUE;

            pImpl->dwAspect = 0;

            pImpl->xWorkingStg = new SvStorage( FALSE, String(), STREAM_STD_READWRITE,
                                                    STORAGE_DELETEONRELEASE );

            SetupStorage( pImpl->xWorkingStg );
            SotStorageStreamRef xOleObjStm = pImpl->xWorkingStg->OpenSotStream(
                                    String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ), STREAM_STD_READWRITE );
            if( xOleObjStm->GetError() )
                return FALSE;

            SotStorageRef xOleObjStor = new SotStorage( *xOleObjStm );
            if( xOleObjStor->GetError() )
                return FALSE;

            xOleObjStm->SetBufferSize( 0xff00 );

            pStor->CopyTo( xOleObjStor );
            xOleObjStor->Commit();
            xOleObjStor.Clear();

            xOleObjStm->Commit();

            return xOleObjStm->GetError() == ERRCODE_NONE;
        }
    }
    return FALSE;
}

//=========================================================================
BOOL SvOutPlaceObject::Save()
/*	[Beschreibung]

    Der Inhalt des Objektes wird in den, in <SvOutPlaceObject::InitNew>
    oder <SvOutPlaceObject::Load> "ubergebenen Storage, geschrieben.

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
        SvStorage * pStor = pImpl->xWorkingStg;
        SvStorageStreamRef xStm;
        xStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ),
                                    STREAM_STD_WRITE | STREAM_TRUNC );
        xStm->SetVersion( pStor->GetVersion() );
        xStm->SetBufferSize( 128 );

        // write Length
        *xStm << (UINT16)(2 + 5 );
#ifdef WNT
        if( pImpl->pSO_Cont )
        {
            WIN_BOOL fSetExtent;
            pImpl->pSO_Cont->GetInfo( pImpl->dwAspect, fSetExtent );
            pImpl->bSetExtent = fSetExtent;

            pImpl->pSO_Cont->Save( pStor );
        }
#endif
        *xStm << pImpl->dwAspect;
        *xStm << sal::static_int_cast< BOOL >(pImpl->bSetExtent);

        if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40 )
        {
            SotStorageStreamRef xOleObjStm =
                pImpl->xWorkingStg->OpenSotStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                                    STREAM_STD_READ );
            if( xOleObjStm->GetError() )
                return FALSE;

            SvCacheStream aStm;
            aStm << *xOleObjStm;
            aStm.Seek( 0 );
            SotStorageRef xOleObjStor = new SotStorage( aStm );
            if( xOleObjStor->GetError() )
                return FALSE;

            // delete all storage entries
            SvStorageInfoList aList;
            pStor->FillInfoList( &aList );
            for( UINT32 i = 0; i < aList.Count(); i++ )
            {
                // workaround a bug in our storage implementation
                String aTmpName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Temp-Delete" ) ) );
                aTmpName += String::CreateFromInt32( nUniqueId++ );
                pStor->Rename( aList[i].GetName(), aTmpName );
                pStor->Remove( aTmpName );

                //pStor->Remove( aList[i].GetName() );
            }
            xOleObjStor->CopyTo( GetStorage() );
        }

        return xStm->GetError() == ERRCODE_NONE;
    }
    return FALSE;
}

//=========================================================================
BOOL SvOutPlaceObject::SaveAs
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
        if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40 || pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 )
        {
#ifdef WNT
            if( pImpl->pSO_Cont )
                pImpl->pSO_Cont->Save( pImpl->xWorkingStg );
#endif
            SotStorageStreamRef xOleObjStm =
                pImpl->xWorkingStg->OpenSotStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                                    STREAM_STD_READ );
            if( xOleObjStm->GetError() )
                return FALSE;

            SvCacheStream aStm;
            aStm << *xOleObjStm;
            aStm.Seek( 0 );
            SotStorageRef xOleObjStor = new SotStorage( aStm );
            if( xOleObjStor->GetError() )
                return FALSE;

            // delete all storage entries
            SvStorageInfoList aList;
            pStor->FillInfoList( &aList );
            for( UINT32 i = 0; i < aList.Count(); i++ )
            {
                // workaround a bug in our storage implementation
                String aTmpName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Temp-Delete" ) ) );
                aTmpName += String::CreateFromInt32( nUniqueId++ );
                pStor->Rename( aList[i].GetName(), aTmpName );
                pStor->Remove( aTmpName );

                //pStor->Remove( aList[i].GetName() );
            }
            xOleObjStor->CopyTo( pStor );
            return pStor->GetError() == ERRCODE_NONE;
        }
        else
        {
            SvStorageStreamRef xStm;
            xStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ),
                                    STREAM_STD_WRITE | STREAM_TRUNC );
            xStm->SetVersion( pStor->GetVersion() );
            xStm->SetBufferSize( 128 );

            // write Length
            *xStm << (UINT16)(2 + 5 );
#ifdef WNT
            if( pImpl->pSO_Cont )
            {
                WIN_BOOL fSetExtent;
                pImpl->pSO_Cont->GetInfo( pImpl->dwAspect, fSetExtent );
                pImpl->bSetExtent = fSetExtent;

                pImpl->pSO_Cont->Save( pStor );
            }
            else
#endif
            {
                pImpl->xWorkingStg->CopyTo( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                    pStor, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ) );
            }
            *xStm << pImpl->dwAspect;
            *xStm << pImpl->bSetExtent;

            return xStm->GetError() == ERRCODE_NONE;
        }
    }
    return FALSE;
}

//=========================================================================
void    SvOutPlaceObject::HandsOff()
{
    if( HasStorage() && pImpl->xWorkingStg == GetStorage() )
        pImpl->xWorkingStg.Clear();
    SvInPlaceObject::HandsOff();
}

//=========================================================================
BOOL    SvOutPlaceObject::SaveCompleted( SvStorage * pStor )
{
    BOOL bResult = TRUE;

    if( pStor )
    {
        if( !pImpl->xWorkingStg.Is() )
        {
            pImpl->xWorkingStg = pStor;
            BOOL bNewVersion = pStor->IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ) );
            if( bNewVersion )
            {
                SvStorageStreamRef xStm;
                xStm = pImpl->xWorkingStg->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DOCNAME ) ), STREAM_STD_READ );
                xStm->SetVersion( pImpl->xWorkingStg->GetVersion() );
                xStm->SetBufferSize( 8192 );

                // nicht vorhandener Stream ist kein Fehler
                if( xStm->GetError() != SVSTREAM_FILE_NOT_FOUND )
                {
                    UINT16 nLen;
                    *xStm >> nLen;
                    *xStm >> pImpl->dwAspect;
                    BOOL b;
                    *xStm >> b;
                    pImpl->bSetExtent = b;

                    if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_40  || pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 )
                    {
                        pImpl->xWorkingStg = new SvStorage( FALSE, String(), STREAM_STD_READWRITE, STORAGE_DELETEONRELEASE );
                        pStor->CopyTo( pImpl->xWorkingStg );
                    }

                    bResult = ( xStm->GetError() == ERRCODE_NONE );
                }
            }
            else
                bResult = MakeWorkStorageWrap_Impl( pStor );
        }
        else if( !IsHandsOff() )
         {
            BOOL bNewVersion = pStor->IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ) );
             if( bNewVersion )
                // Full storage in 5.0 or newer format
                pImpl->xWorkingStg = pStor;
            else
                bResult = MakeWorkStorageWrap_Impl( pStor );
        }
    }

    return SvInPlaceObject::SaveCompleted( pStor ) && bResult;
}

BOOL    SvOutPlaceObject::MakeWorkStorageWrap_Impl( SvStorage * pStor )
{
    BOOL bResult = FALSE;

    pImpl->xWorkingStg = new SvStorage( FALSE, String(), STREAM_STD_READWRITE, STORAGE_DELETEONRELEASE );

    SetupStorage( pImpl->xWorkingStg );
    SotStorageStreamRef xOleObjStm = pImpl->xWorkingStg->OpenSotStream(
                                                String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                                STREAM_STD_READWRITE );

    if( !xOleObjStm->GetError() )
    {
        SotStorageRef xOleObjStor = new SotStorage( *xOleObjStm );
        if( !xOleObjStor->GetError() )
        {
            xOleObjStm->SetBufferSize( 0xff00 );

            pStor->CopyTo( xOleObjStor );
            xOleObjStor->Commit();
            xOleObjStor.Clear();

            xOleObjStm->Commit();

            bResult = ( xOleObjStm->GetError() == ERRCODE_NONE );
        }
    }

    return bResult;
}

com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > SvOutPlaceObject::GetUnoComponent() const
{
#ifdef WNT
    if ( !pImpl->m_xComponent.is() )
    {
        if ( !pImpl->pSO_Cont )
            ((SvOutPlaceObject*)this)->LoadSO_Cont();
        if ( pImpl->pSO_Cont )
            pImpl->m_xComponent = new OLEWrapper_Impl( pImpl->pSO_Cont );
    }
#endif
    return pImpl->m_xComponent;
}

SvGlobalName SvOutPlaceObject::GetObjectCLSID()
{
    SvGlobalName aRet;
#ifdef WNT
    if(pImpl->pSO_Cont)
        aRet = pImpl->pSO_Cont->GetCLSID();
#endif
    return aRet;
}

void SvOutPlaceObject::DrawObject
(
    OutputDevice * 	pDev,	/* in dieses Device findet die Ausgabe statt */
    const JobSetup &,		/* fuer dieses Device soll formatiert werden */
    const Size&
#if defined WNT
        rSize
#endif
    ,
    USHORT
)
/*	[Beschreibung]

    Ein Ausgabe ist nicht m"oglich. Deswegen wird eine Bitmap
    und als Unterschrift der URL ausgegeben,

    [Querverweise]

    <SvInPlaceObject::Draw>
*/
{
    if( !pImpl->pOP )
        pImpl->pOP = CreateCache_Impl( pImpl->xWorkingStg );

    Rectangle aOutRect = GetVisArea( ASPECT_CONTENT );
    if( pImpl->pOP )
    {
        GDIMetaFile * pMtf = pImpl->pOP->GetMetaFile();
        if( pMtf )
        {
            pMtf->WindStart();
            pMtf->Play( pDev, aOutRect.TopLeft(), aOutRect.GetSize() );
        }
        else
        {
            Bitmap * pBmp = pImpl->pOP->GetBitmap();
            if( pBmp )
                pDev->DrawBitmap( aOutRect.TopLeft(), aOutRect.GetSize(), *pBmp );
        }

        return;
    }
    else
    {
#ifdef WNT
        BOOL bPlayed = FALSE;
        if (!pImpl->pSO_Cont)
            LoadSO_Cont();

        if( pImpl->pSO_Cont )
        {
            long nMapMode;
            Size aSize;
            HMETAFILE hMet;
            if ( pImpl->pSO_Cont->GetMetaFile( nMapMode, aSize, hMet ) )
            {
                ULONG nBufSize = GetMetaFileBitsEx( hMet, 0, NULL );
                unsigned char* pBuf = new unsigned char[nBufSize+22];
                *((long*)pBuf) = 0x9ac6cdd7L;
                *((short*)(pBuf+6)) = (SHORT) 0;
                *((short*)(pBuf+8)) = (SHORT) 0;
                *((short*)(pBuf+10)) = (SHORT) aSize.Width();
                *((short*)(pBuf+12)) = (SHORT) aSize.Height();
                *((short*)(pBuf+14)) = (USHORT) 2540;
                if ( nBufSize && nBufSize == GetMetaFileBitsEx( hMet, nBufSize, pBuf+22 ) )
                {
                    SvMemoryStream aStream( pBuf, nBufSize+22, STREAM_READ );
                    //SvFileStream aFile(String::CreateFromAscii("file:///d:/test.wmf"), STREAM_STD_READWRITE);
                    //aStream >> aFile;
                    aStream.Seek(0);
                    GDIMetaFile aMtf;
                    if( ReadWindowMetafile( aStream, aMtf, NULL ) )
                    {
                        aMtf.WindStart();
                        MapMode aMode( MAP_100TH_MM );
                        //Size aSize = OutputDevice::LogicToLogic( aOutRect.GetSize(), aMode, pDev->GetMapMode() );
    //						AllSettings aNew( pDev->GetSettings() );
    //						StyleSettings aSet ( aNew.GetStyleSettings() );
    //						aSet.SetAntialiasingMinPixelHeight( 5 );
    //						aNew.SetStyleSettings( aSet );
    //						pDev->SetSettings( aNew );

                        //Point aPoint;
                        //Size aSize( aOutRect.GetSize() );
                        Size aSize = OutputDevice::LogicToLogic( rSize, aMode, pDev->GetMapMode() );
                        aMtf.Play( pDev, Point(), aSize );
                        bPlayed = TRUE;
                    }
                }

                DeleteMetaFile( hMet );
                delete[] pBuf;
            }
        }

        if ( !bPlayed )
#endif
        {
            Rectangle aVisArea_ = GetVisArea( ASPECT_CONTENT );
            String a = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "SvOutPlaceObject" ) );
            SoPaintReplacement( aVisArea_, a, pDev );
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
