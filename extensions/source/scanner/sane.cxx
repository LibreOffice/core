/*************************************************************************
 *
 *  $RCSfile: sane.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-07 14:44:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <cstdarg>
#include <tools/stream.hxx>
#include <sane.hxx>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#if defined DEBUG || defined DBG_UTIL
#include <stdarg.h>
#define dump_state( a, b, c, d ) fprintf( stderr, a, b, c, d );
#else
#define dump_state( a, b, c, d ) ;
#endif
inline void dbg_msg( char* pString, ... )
{
#if defined DEBUG || defined DBG_UTIL
    va_list ap;
    va_start( ap, pString );
    vfprintf( stderr, pString, ap );
    va_end( ap );
#endif
}

#define FAIL_SHUTDOWN_STATE( x, y, z ) \
    if( x != SANE_STATUS_GOOD )                             \
    {                                                       \
        dump_state( "%s returned error %d (%s)\n",          \
                 y, x, p_strstatus( x ) );                  \
        DeInit();                                           \
        return z;                                           \
    }

#define FAIL_STATE( x, y, z ) \
    if( x != SANE_STATUS_GOOD )                             \
    {                                                       \
        dump_state( "%s returned error %d (%s)\n",          \
                 y, x, p_strstatus( x ) );                  \
        return z;                                           \
    }

#define DUMP_STATE( x, y ) \
    if( x != SANE_STATUS_GOOD )                             \
    {                                                       \
        dump_state( "%s returned error %d (%s)\n",          \
                 y, x, p_strstatus( x ) );                  \
    }

#define CHECK_STATE( x, y ) \
    if( x != SANE_STATUS_GOOD )                             \
    {                                                       \
        dump_state( "%s returned error %d (%s)\n",          \
                 y, x, p_strstatus( x ) );                  \
    }                                                       \
    else

int             Sane::nRefCount = 0;
void*           Sane::pSaneLib = 0;
SANE_Int        Sane::nVersion = 0;
SANE_Device**   Sane::ppDevices = 0;
int             Sane::nDevices = 0;

SANE_Status     (*Sane::p_init)( SANE_Int*,
                                 SANE_Auth_Callback ) = 0;
void            (*Sane::p_exit)() = 0;
SANE_Status     (*Sane::p_get_devices)( const SANE_Device***,
                                        SANE_Bool ) = 0;
SANE_Status     (*Sane::p_open)( SANE_String_Const, SANE_Handle ) = 0;
void            (*Sane::p_close)( SANE_Handle ) = 0;
const SANE_Option_Descriptor* (*Sane::p_get_option_descriptor)(
    SANE_Handle, SANE_Int ) = 0;
SANE_Status     (*Sane::p_control_option)( SANE_Handle, SANE_Int,
                                           SANE_Action, void*,
                                           SANE_Int* ) = 0;
SANE_Status     (*Sane::p_get_parameters)( SANE_Handle,
                                           SANE_Parameters* ) = 0;
SANE_Status     (*Sane::p_start)( SANE_Handle ) = 0;
SANE_Status     (*Sane::p_read)( SANE_Handle, SANE_Byte*, SANE_Int,
                                 SANE_Int* ) = 0;
void            (*Sane::p_cancel)( SANE_Handle ) = 0;
SANE_Status     (*Sane::p_set_io_mode)( SANE_Handle, SANE_Bool ) = 0;
SANE_Status     (*Sane::p_get_select_fd)( SANE_Handle, SANE_Int* ) = 0;
const SANE_String_Const (*Sane::p_strstatus)( SANE_Status ) = 0;

static BOOL bSaneSymbolLoadFailed = FALSE;

inline void* Sane::LoadSymbol( char* pSymbolname )
{
    void *pRet = dlsym( pSaneLib, pSymbolname );
    if( ! pRet )
    {
        fprintf( stderr, "Could not load symbol %s: %s\n",
                 pSymbolname, dlerror() );
        bSaneSymbolLoadFailed = TRUE;
    }
    return pRet;
}

SANE_Status Sane::ControlOption( int nOption, SANE_Action nAction,
                                 void* pData )
{
    SANE_Status nStatus = SANE_STATUS_GOOD;
    SANE_Int    nInfo = 0;

    nStatus = p_control_option( maHandle, (SANE_Int)nOption,
                                nAction, pData, &nInfo );
    DUMP_STATE( nStatus, "sane_control_option" );
#ifdef DEBUG
    if( nStatus != SANE_STATUS_GOOD )
    {
        char* pAction = "Unknown";
        switch( nAction )
        {
            case SANE_ACTION_GET_VALUE:
                pAction = "SANE_ACTION_GET_VALUE";break;
            case SANE_ACTION_SET_VALUE:
                pAction = "SANE_ACTION_SET_VALUE";break;
            case SANE_ACTION_SET_AUTO:
                pAction = "SANE_ACTION_SET_AUTO";break;
        }
        dbg_msg( "Option: \"%s\" action: %s\n",
                 ByteString( GetOptionName( nOption ), gsl_getSystemTextEncoding() ).GetBuffer(),
                 pAction );
    }
#endif
//  if( nInfo & ( SANE_INFO_RELOAD_OPTIONS | SANE_INFO_RELOAD_PARAMS ) )
    if( nInfo &  SANE_INFO_RELOAD_OPTIONS )
        ReloadOptions();
    return nStatus;
}

Sane::Sane() :
        maHandle( 0 ),
        mppOptions( 0 ),
        mnOptions( 0 ),
        mnDevice( -1 )
{
    if( ! nRefCount || ! pSaneLib )
        Init();
    nRefCount++;
};

Sane::~Sane()
{
    if( IsOpen() )
        Close();
    nRefCount--;
    if( ! nRefCount && pSaneLib )
        DeInit();
}

void Sane::Init()
{
    pSaneLib = dlopen( "libsane.so", RTLD_LAZY );
    if( pSaneLib )
    {
        bSaneSymbolLoadFailed = FALSE;
        p_init = (SANE_Status(*)(SANE_Int*, SANE_Auth_Callback ))
            LoadSymbol( "sane_init" );
        p_exit = (void(*)())
            LoadSymbol( "sane_exit" );
        p_get_devices = (SANE_Status(*)(const SANE_Device***,
                                        SANE_Bool ))
            LoadSymbol( "sane_get_devices" );
        p_open = (SANE_Status(*)(SANE_String_Const, SANE_Handle ))
            LoadSymbol( "sane_open" );
        p_close = (void(*)(SANE_Handle))
            LoadSymbol( "sane_close" );
        p_get_option_descriptor = (const SANE_Option_Descriptor*(*)(SANE_Handle,
                                                              SANE_Int))
            LoadSymbol( "sane_get_option_descriptor" );
        p_control_option = (SANE_Status(*)(SANE_Handle, SANE_Int,
                                           SANE_Action, void*, SANE_Int*))
            LoadSymbol( "sane_control_option" );
        p_get_parameters = (SANE_Status(*)(SANE_Handle,SANE_Parameters*))
            LoadSymbol( "sane_get_parameters" );
        p_start = (SANE_Status(*)(SANE_Handle))
            LoadSymbol( "sane_start" );
        p_read = (SANE_Status(*)(SANE_Handle, SANE_Byte*,
                                 SANE_Int, SANE_Int* ))
            LoadSymbol( "sane_read" );
        p_cancel = (void(*)(SANE_Handle))
            LoadSymbol( "sane_cancel" );
        p_set_io_mode = (SANE_Status(*)(SANE_Handle, SANE_Bool))
            LoadSymbol( "sane_set_io_mode" );
        p_get_select_fd = (SANE_Status(*)(SANE_Handle, SANE_Int*))
            LoadSymbol( "sane_get_select_fd" );
        p_strstatus = (const SANE_String_Const(*)(SANE_Status))
            LoadSymbol( "sane_strstatus" );
        if( bSaneSymbolLoadFailed )
            DeInit();
        else
        {
            SANE_Status nStatus = p_init( &nVersion, 0 );
            FAIL_SHUTDOWN_STATE( nStatus, "sane_init", );
            nStatus = p_get_devices( (const SANE_Device***)&ppDevices,
                                     SANE_FALSE );
            FAIL_SHUTDOWN_STATE( nStatus, "sane_get_devices", );
            for( nDevices = 0 ; ppDevices[ nDevices ]; nDevices++ );
        }
    }
#if defined DEBUG || defined DBG_UTIL
    else
        fprintf( stderr, "libsane.so could not be opened: %s\n",
                 dlerror() );
#endif
}

void Sane::DeInit()
{
    if( pSaneLib )
    {
        p_exit();
        dlclose( pSaneLib );
        pSaneLib = 0;
    }
}

void Sane::ReloadDevices()
{
    if( IsOpen() )
        Close();
    DeInit();
    Init();
}

void Sane::ReloadOptions()
{
    if( ! IsOpen() )
        return;

    SANE_Option_Descriptor* pZero = (SANE_Option_Descriptor*)
        p_get_option_descriptor( maHandle, 0 );
    SANE_Word pOptions[2];
    SANE_Status nStatus = p_control_option( maHandle, 0, SANE_ACTION_GET_VALUE,
                                            (void*)pOptions, NULL );
    if( nStatus != SANE_STATUS_GOOD )
        fprintf( stderr, "Error: sane driver returned %s while reading number of options !\n", p_strstatus( nStatus ) );

    mnOptions = pOptions[ 0 ];
    if( pZero->size > sizeof( SANE_Word ) )
        fprintf( stderr, "driver returned numer of options with larger size tha SANE_Word !!!\n" );
    if( mppOptions )
        delete mppOptions;
    mppOptions = (const SANE_Option_Descriptor**)new SANE_Option_Descriptor*[ mnOptions ];
    mppOptions[ 0 ] = (SANE_Option_Descriptor*)pZero;
    for( int i = 1; i < mnOptions; i++ )
        mppOptions[ i ] =  (SANE_Option_Descriptor*)
            p_get_option_descriptor( maHandle, i );

    CheckConsistency( NULL, TRUE );

    maReloadOptionsLink.Call( this );
}

BOOL Sane::Open( const char* name )
{
    int i;

    SANE_Status nStatus = p_open( (SANE_String_Const)name, &maHandle );
    FAIL_STATE( nStatus, "sane_open", FALSE );

    ReloadOptions();

    if( mnDevice == -1 )
    {
        ByteString aDevice( name );
        for( i = 0; i < nDevices; i++ )
        {
            if( aDevice.Equals( ppDevices[i]->name ) )
            {
                mnDevice = i;
                break;
            }
        }
    }

    return TRUE;
}

BOOL Sane::Open( int n )
{
    if( n >= 0 && n < nDevices )
    {
        mnDevice = n;
        return Open( (char*)ppDevices[n]->name );
    }
    return FALSE;
}

void Sane::Close()
{
    if( maHandle )
    {
        p_close( maHandle );
        delete mppOptions;
        mppOptions = 0;
        maHandle = 0;
        mnDevice = -1;
    }
}

void Sane::Stop()
{
    if( maHandle )
    {
        int nDevice = mnDevice;
        Close();
        Open( nDevice );
    }
}

int Sane::GetOptionByName( const char* rName )
{
    int i;
    ByteString aOption( rName );
    for( i = 0; i < mnOptions; i++ )
    {
        if( mppOptions[i]->name && aOption.Equals( mppOptions[i]->name ) )
            return i;
    }
    return -1;
}

BOOL Sane::GetOptionValue( int n, BOOL& rRet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_BOOL )
        return FALSE;
    SANE_Word nRet;
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, &nRet );
    if( nStatus != SANE_STATUS_GOOD )
        return FALSE;

    rRet = nRet;
    return TRUE;
}

BOOL Sane::GetOptionValue( int n, ByteString& rRet )
{
    BOOL bSuccess = FALSE;
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_STRING )
        return FALSE;
    char* pRet = new char[mppOptions[n]->size+1];
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pRet );
    if( nStatus == SANE_STATUS_GOOD )
    {
        bSuccess = TRUE;
        rRet = pRet;
    }
    delete pRet;
    return bSuccess;
}

BOOL Sane::GetOptionValue( int n, double& rRet, int nElement )
{
    BOOL bSuccess = FALSE;

    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return FALSE;

    SANE_Word* pRet = new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)];
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pRet );
    if( nStatus == SANE_STATUS_GOOD )
    {
        bSuccess = TRUE;
        if( mppOptions[n]->type == SANE_TYPE_INT )
            rRet = (double)pRet[ nElement ];
        else
            rRet = SANE_UNFIX( pRet[nElement] );
    }
    delete pRet;
    return bSuccess;
}

BOOL Sane::GetOptionValue( int n, double* pSet )
{
    if( ! maHandle  || ! ( mppOptions[n]->type == SANE_TYPE_FIXED ||
                           mppOptions[n]->type == SANE_TYPE_INT ) )
        return FALSE;

    SANE_Word* pFixedSet = new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)];
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pFixedSet );
    if( nStatus != SANE_STATUS_GOOD )
    {
        delete pFixedSet;
        return FALSE;
    }
    for( int i = 0; i <mppOptions[n]->size/sizeof(SANE_Word); i++ )
    {
        if( mppOptions[n]->type == SANE_TYPE_FIXED )
            pSet[i] = SANE_UNFIX( pFixedSet[i] );
        else
            pSet[i] = (double) pFixedSet[i];
    }
    delete pFixedSet;
    return TRUE;
}

BOOL Sane::SetOptionValue( int n, BOOL bSet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_BOOL )
        return FALSE;
    SANE_Word nRet = bSet ? SANE_TRUE : SANE_FALSE;
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, &nRet );
    if( nStatus != SANE_STATUS_GOOD )
        return FALSE;
    return TRUE;
}

BOOL Sane::SetOptionValue( int n, const String& rSet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_STRING )
        return FALSE;
    ByteString aSet( rSet, gsl_getSystemTextEncoding() );
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, (void*)aSet.GetBuffer() );
    if( nStatus != SANE_STATUS_GOOD )
        return FALSE;
    return TRUE;
}

BOOL Sane::SetOptionValue( int n, double fSet, int nElement )
{
    BOOL bSuccess = FALSE;

    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return FALSE;

    SANE_Status nStatus;
    if( mppOptions[n]->size/sizeof(SANE_Word) > 1 )
    {
        SANE_Word* pSet = new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)];
        nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pSet );
        if( nStatus == SANE_STATUS_GOOD )
        {
            pSet[nElement] = mppOptions[n]->type == SANE_TYPE_INT ?
                (SANE_Word)fSet : SANE_FIX( fSet );
            nStatus = ControlOption(  n, SANE_ACTION_SET_VALUE, pSet );
        }
        delete pSet;
    }
    else
    {
        SANE_Word nSetTo =
            mppOptions[n]->type == SANE_TYPE_INT ?
            nSetTo = (SANE_Word)fSet : nSetTo = SANE_FIX( fSet );

        nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, &nSetTo );
        if( nStatus == SANE_STATUS_GOOD )
            bSuccess = TRUE;
    }
    return bSuccess;
}

BOOL Sane::SetOptionValue( int n, double* pSet )
{
    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return FALSE;
    SANE_Word* pFixedSet = new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)];
    for( int i = 0; i <mppOptions[n]->size/sizeof(SANE_Word); i++ )
    {
        if( mppOptions[n]->type == SANE_TYPE_FIXED )
            pFixedSet[i] = SANE_FIX( pSet[i] );
        else
            pFixedSet[i] = (SANE_Word)pSet[i];
    }
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, pFixedSet );
    delete pFixedSet;
    if( nStatus != SANE_STATUS_GOOD )
        return FALSE;
    return TRUE;
}

enum FrameStyleType {
    FrameStyle_BW, FrameStyle_Gray, FrameStyle_RGB, FrameStyle_Separated
};

#define BYTE_BUFFER_SIZE 32768

static inline UINT8 _ReadValue( FILE* fp, int depth )
{
    if( depth == 16 )
    {
        UINT16 nWord;
        // data always come in native byte order !
        // 16 bits is not really supported by backends as of now
        // e.g. UMAX Astra 1200S delivers 16 bit but in BIGENDIAN
        // against SANE documentation (xscanimage gets the same result
        // as we do
        fread( &nWord, 1, 2, fp );
        return (UINT8)( nWord / 256 );
    }
    UINT8 nByte;
    fread( &nByte, 1, 1, fp );
    return nByte;
}

BOOL Sane::CheckConsistency( const char* pMes, BOOL bInit )
{
    static SANE_Option_Descriptor** pDescArray = NULL;
    static SANE_Option_Descriptor*  pZero = NULL;

    if( bInit )
    {
        pDescArray = (SANE_Option_Descriptor**)mppOptions;
        if( mppOptions )
            pZero = (SANE_Option_Descriptor*)mppOptions[0];
        return TRUE;
    }

    BOOL bConsistent = TRUE;

    if( pDescArray != mppOptions )
        bConsistent = FALSE;
    if( pZero != mppOptions[0] )
        bConsistent = FALSE;

    if( ! bConsistent )
        dbg_msg( "Sane is not consistent. (%s)\n", pMes );

    return bConsistent;
}

BOOL Sane::Start( BitmapTransporter& rBitmap )
{
    int nStream = 0, nLine = 0, i = 0;
    SANE_Parameters aParams;
    FrameStyleType eType = FrameStyle_Gray;
    BOOL bSuccess = TRUE;
    BOOL bWidthSet = FALSE;

    if( ! maHandle )
        return FALSE;

    BYTE* pBuffer = NULL;

    SANE_Status nStatus = SANE_STATUS_GOOD;

    rBitmap.lock();
    SvMemoryStream& aConverter = rBitmap.getStream();
    aConverter.Seek( 0 );
    aConverter.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // write bitmap stream header
    aConverter << 'B' << 'M';
    aConverter << (ULONG) 0;
    aConverter << (ULONG) 0;
    aConverter << (ULONG) 60;

    // write BITMAPINFOHEADER
    aConverter << (UINT32)40;
    aConverter << (UINT32)0; // fill in width later
    aConverter << (UINT32)0; // fill in height later
    aConverter << (UINT16)1;
    // create header for 24 bits
    // correct later if necessary
    aConverter << (UINT16)24;
    aConverter << (UINT32)0;
    aConverter << (UINT32)0;
    aConverter << (UINT32)0;
    aConverter << (UINT32)0;
    aConverter << (UINT32)0;
    aConverter << (UINT32)0;

    for( nStream=0; nStream < 3 && bSuccess ; nStream++ )
    {
        nStatus = p_start( maHandle );
        DUMP_STATE( nStatus, "sane_start" );
        CheckConsistency( "sane_start" );
        if( nStatus == SANE_STATUS_GOOD )
        {
            nStatus = p_get_parameters( maHandle, &aParams );
            DUMP_STATE( nStatus, "sane_get_parameters" );
            CheckConsistency( "sane_get_parameters" );
#if defined DEBUG || defined DBG_UTIL
            char* ppFormats[] = { "SANE_FRAME_GRAY", "SANE_FRAME_RGB",
                                  "SANE_FRAME_RED", "SANE_FRAME_GREEN",
                                  "SANE_FRAME_BLUE", "Unknown !!!" };
            fprintf( stderr, "Parameters for frame %d:\n", nStream );
            if( aParams.format < 0 || aParams.format > 4 )
                aParams.format = (SANE_Frame)5;
            fprintf( stderr, "format:           %s\n", ppFormats[ (int)aParams.format ] );
            fprintf( stderr, "last_frame:       %s\n", aParams.last_frame ? "TRUE" : "FALSE" );
            fprintf( stderr, "depth:            %d\n", (int)aParams.depth );
            fprintf( stderr, "pixels_per_line:  %d\n", (int)aParams.pixels_per_line );
            fprintf( stderr, "bytes_per_line:   %d\n", (int)aParams.bytes_per_line );
#endif
            if( ! pBuffer )
            {
                pBuffer = new BYTE[ BYTE_BUFFER_SIZE < 4*aParams.bytes_per_line ? 4*aParams.bytes_per_line : BYTE_BUFFER_SIZE ];
            }

            if( aParams.last_frame )
                nStream=3;

            switch( aParams.format )
            {
                case SANE_FRAME_GRAY:
                    eType = FrameStyle_Gray;
                    if( aParams.depth == 1 )
                        eType = FrameStyle_BW;
                    break;
                case SANE_FRAME_RGB:
                    eType = FrameStyle_RGB;
                    break;
                case SANE_FRAME_RED:
                case SANE_FRAME_GREEN:
                case SANE_FRAME_BLUE:
                    eType = FrameStyle_Separated;
                    break;
                default:
                    fprintf( stderr, "Warning: unknown frame style !!!\n" );
            }

            BOOL bSynchronousRead = TRUE;

            // should be fail safe, but ... ??
            nStatus = p_set_io_mode( maHandle, SANE_FALSE );
            CheckConsistency( "sane_set_io_mode" );
            if( nStatus != SANE_STATUS_GOOD )
            {
                bSynchronousRead = FALSE;
                nStatus = p_set_io_mode( maHandle, SANE_TRUE );
                CheckConsistency( "sane_set_io_mode" );
#if defined DEBUG || defined DBG_UTIL
                if( nStatus != SANE_STATUS_GOOD )
                    // what ?!?
                    fprintf( stderr, "Sane::Start: driver is confused\n" );
#endif
            }

            SANE_Int nLen=0;
            SANE_Int fd = 0;

            if( ! bSynchronousRead )
            {
                nStatus = p_get_select_fd( maHandle, &fd );
                DUMP_STATE( nStatus, "sane_get_select_fd" );
                CheckConsistency( "sane_get_select_fd" );
                if( nStatus != SANE_STATUS_GOOD )
                    bSynchronousRead = TRUE;
            }
            FILE* pFrame = tmpfile();
            if( ! pFrame )
            {
                bSuccess = FALSE;
                break;
            }
            do {
                if( ! bSynchronousRead )
                {
                    fd_set fdset;
                    struct timeval tv;

                    FD_ZERO( &fdset );
                    FD_SET( (int)fd, &fdset );
                    tv.tv_sec = 5;
                    tv.tv_usec = 0;
                    if( select( fd+1, &fdset, NULL, NULL, &tv ) == 0 )
                        fprintf( stderr, "Timout on sane_read descriptor\n" );
                }
                nLen = 0;
                nStatus = p_read( maHandle, pBuffer, BYTE_BUFFER_SIZE, &nLen );
                CheckConsistency( "sane_read" );
                if( nLen && ( nStatus == SANE_STATUS_GOOD ||
                              nStatus == SANE_STATUS_EOF ) )
                {
                    fwrite( pBuffer, 1, nLen, pFrame );
                }
                else
                    DUMP_STATE( nStatus, "sane_read" );
            } while( nStatus == SANE_STATUS_GOOD );
            if( nStatus != SANE_STATUS_EOF )
            {
                fclose( pFrame );
                bSuccess = FALSE;
                break;
            }

            int nFrameLength = ftell( pFrame );
            fseek( pFrame, 0, SEEK_SET );
            UINT32 nWidth = (UINT32) aParams.pixels_per_line;
            UINT32 nHeight = (UINT32) (nFrameLength / aParams.bytes_per_line);
            if( ! bWidthSet )
            {
#ifdef DEBUG
                fprintf( stderr, "set dimensions to %d, %d\n", nWidth, nHeight );
#endif
                aConverter.Seek( 18 );
                aConverter << nWidth;
                aConverter << nHeight;
                bWidthSet = TRUE;
            }
            aConverter.Seek(60);

            if( eType == FrameStyle_BW )
            {
                aConverter.Seek( 10 );
                aConverter << (ULONG)64;
                aConverter.Seek( 28 );
                aConverter << (UINT16) 1;
                aConverter.Seek( 54 );
                // write color table
                aConverter << (UINT32)0;
                aConverter << (UINT16)0xffff;
                aConverter << (UINT8)0xff;
                aConverter << (UINT8)0;
                aConverter.Seek( 64 );
            }
            else if( eType == FrameStyle_Gray )
            {
                 aConverter.Seek( 10 );
                 aConverter << (ULONG)1084;
                aConverter.Seek( 28 );
                aConverter << (UINT16) 8;
                aConverter.Seek( 54 );
                // write color table
                for( nLine = 0; nLine < 256; nLine++ )
                {
                    aConverter << (UINT8)nLine;
                    aConverter << (UINT8)nLine;
                    aConverter << (UINT8)nLine;
                    aConverter << (UINT8)0;
                }
                aConverter.Seek( 1084 );
            }

            for( nLine = nHeight-1;
                 nLine >= 0; nLine-- )
            {
                fseek( pFrame, nLine * aParams.bytes_per_line, SEEK_SET );
                if( eType == FrameStyle_BW ||
                    ( eType == FrameStyle_Gray && aParams.depth == 8 )
                    )
                {
                    fread( pBuffer, 1, aParams.bytes_per_line, pFrame );
                    aConverter.Write( pBuffer, aParams.bytes_per_line );
                }
                else if( eType == FrameStyle_Gray )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        UINT8 nGray = _ReadValue( pFrame, aParams.depth );
                        aConverter << nGray;
                    }
                }
                else if( eType == FrameStyle_RGB )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        UINT8 nRed, nGreen, nBlue;
                        nRed    = _ReadValue( pFrame, aParams.depth );
                        nGreen  = _ReadValue( pFrame, aParams.depth );
                        nBlue   = _ReadValue( pFrame, aParams.depth );
                        aConverter << nBlue;
                        aConverter << nGreen;
                        aConverter << nRed;
                    }
                }
                else if( eType == FrameStyle_Separated )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        UINT8 nValue = _ReadValue( pFrame, aParams.depth );
                        switch( aParams.format )
                        {
                            case SANE_FRAME_RED:
                                aConverter.SeekRel( 2 );
                                aConverter << nValue;
                                break;
                            case SANE_FRAME_GREEN:
                                aConverter.SeekRel( 1 );
                                aConverter << nValue;
                                aConverter.SeekRel( 1 );
                                break;
                            case SANE_FRAME_BLUE:
                                aConverter << nValue;
                                aConverter.SeekRel( 2 );
                                break;
                        }
                    }
                }
                 int nGap = aConverter.Tell() & 3;
                 if( nGap )
                     aConverter.SeekRel( 4-nGap );
            }
            fclose( pFrame ); // deletes tmpfile
            if( eType != FrameStyle_Separated )
                break;
        }
        else
            bSuccess = FALSE;
    }
    // get stream length
    aConverter.Seek( STREAM_SEEK_TO_END );
    int nPos = aConverter.Tell();

    aConverter.Seek( 2 );
    aConverter << (ULONG) nPos+1;
    aConverter.Seek( 0 );

    rBitmap.unlock();

    if( bSuccess )
    {
        // only cancel a successful operation
        // sane disrupts memory else
        p_cancel( maHandle );
        CheckConsistency( "sane_cancel" );
    }
    if( pBuffer )
        delete pBuffer;

    ReloadOptions();


    dbg_msg( "Sane::Start returns with %s\n", bSuccess ? "TRUE" : "FALSE" );

    return bSuccess;
}

int Sane::GetRange( int n, double*& rpDouble )
{
    if( mppOptions[n]->constraint_type != SANE_CONSTRAINT_RANGE &&
        mppOptions[n]->constraint_type != SANE_CONSTRAINT_WORD_LIST )
    {
        return -1;
    }

    rpDouble = 0;
    int nItems, i;
    BOOL bIsFixed = mppOptions[n]->type == SANE_TYPE_FIXED ? TRUE : FALSE;

    dbg_msg( "Sane::GetRange of option %s ", mppOptions[n]->name );
    if(mppOptions[n]->constraint_type == SANE_CONSTRAINT_RANGE )
    {
        double fMin, fMax, fQuant;
        if( bIsFixed )
        {
            fMin = SANE_UNFIX( mppOptions[n]->constraint.range->min );
            fMax = SANE_UNFIX( mppOptions[n]->constraint.range->max );
            fQuant = SANE_UNFIX( mppOptions[n]->constraint.range->quant );
        }
        else
        {
            fMin = (double)mppOptions[n]->constraint.range->min;
            fMax = (double)mppOptions[n]->constraint.range->max;
            fQuant = (double)mppOptions[n]->constraint.range->quant;
        }
        if( fQuant != 0.0 )
        {
            dbg_msg( "quantum range [ %lg ; %lg ; %lg ]\n",
                     fMin, fQuant, fMax );
            nItems = (int)((fMax - fMin)/fQuant)+1;
            rpDouble = new double[ nItems ];
            double fValue = fMin;
            for( i = 0; i < nItems; i++, fValue += fQuant )
                rpDouble[i] = fValue;
            rpDouble[ nItems-1 ] = fMax;
            return nItems;
        }
        else
        {
            dbg_msg( "normal range [ %lg %lg ]\n",
                     fMin, fMax );
            rpDouble = new double[2];
            rpDouble[0] = fMin;
            rpDouble[1] = fMax;
            return 0;
        }
    }
    else
    {
        nItems = mppOptions[n]->constraint.word_list[0];
        rpDouble = new double[nItems];
        for( i=0; i<nItems; i++ )
        {
            rpDouble[i] = bIsFixed ?
                SANE_UNFIX( mppOptions[n]->constraint.word_list[i+1] ) :
                (double)mppOptions[n]->constraint.word_list[i+1];
        }
        dbg_msg( "worlist [ %lg ... %lg ]\n",
                 rpDouble[ 0 ], rpDouble[ nItems-1 ] );
        return nItems;
    }

    return -1;
}

static char *ppUnits[] = {
    "",
    "[Pixel]",
    "[Bit]",
    "[mm]",
    "[DPI]",
    "[%]",
    "[usec]"
};

String Sane::GetOptionUnitName( int n )
{
    String aText;
    SANE_Unit nUnit = mppOptions[n]->unit;
    if( nUnit < 0 || nUnit > sizeof( ppUnits )/sizeof( ppUnits[0] ) )
        aText = String::CreateFromAscii( "[unknown units]" );
    else
        aText = String( ppUnits[ nUnit ], gsl_getSystemTextEncoding() );
    return aText;
}

BOOL Sane::ActivateButtonOption( int n )
{
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, NULL );
    if( nStatus != SANE_STATUS_GOOD )
        return FALSE;
    return TRUE;
}
