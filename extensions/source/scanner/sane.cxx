/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cstdarg>
#include <type_traits>
#include <math.h>
#include <osl/file.h>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include "sane.hxx"
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sal/config.h>
#include <sal/macros.h>
#include <memory>

#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
#include <stdarg.h>
#define dump_state( a, b, c, d ) fprintf( stderr, a, b, c, d );
#else
#define dump_state( a, b, c, d ) ;
#endif
inline void dbg_msg( const char* pString, ... )
{
#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
    va_list ap;
    va_start( ap, pString );
    vfprintf( stderr, pString, ap );
    va_end( ap );
#else
    (void)pString;
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

int             Sane::nRefCount = 0;
oslModule       Sane::pSaneLib = nullptr;
SANE_Int        Sane::nVersion = 0;
SANE_Device**   Sane::ppDevices = nullptr;
int             Sane::nDevices = 0;

SANE_Status     (*Sane::p_init)( SANE_Int*,
                                 SANE_Auth_Callback ) = nullptr;
void            (*Sane::p_exit)() = nullptr;
SANE_Status     (*Sane::p_get_devices)( const SANE_Device***,
                                        SANE_Bool ) = nullptr;
SANE_Status     (*Sane::p_open)( SANE_String_Const, SANE_Handle ) = nullptr;
void            (*Sane::p_close)( SANE_Handle ) = nullptr;
const SANE_Option_Descriptor* (*Sane::p_get_option_descriptor)(
    SANE_Handle, SANE_Int ) = nullptr;
SANE_Status     (*Sane::p_control_option)( SANE_Handle, SANE_Int,
                                           SANE_Action, void*,
                                           SANE_Int* ) = nullptr;
SANE_Status     (*Sane::p_get_parameters)( SANE_Handle,
                                           SANE_Parameters* ) = nullptr;
SANE_Status     (*Sane::p_start)( SANE_Handle ) = nullptr;
SANE_Status     (*Sane::p_read)( SANE_Handle, SANE_Byte*, SANE_Int,
                                 SANE_Int* ) = nullptr;
void            (*Sane::p_cancel)( SANE_Handle ) = nullptr;
SANE_Status     (*Sane::p_set_io_mode)( SANE_Handle, SANE_Bool ) = nullptr;
SANE_Status     (*Sane::p_get_select_fd)( SANE_Handle, SANE_Int* ) = nullptr;
SANE_String_Const (*Sane::p_strstatus)( SANE_Status ) = nullptr;

static bool bSaneSymbolLoadFailed = false;

inline oslGenericFunction Sane::LoadSymbol( const char* pSymbolname )
{
    oslGenericFunction pFunction = osl_getAsciiFunctionSymbol( pSaneLib, pSymbolname );
    if( ! pFunction )
    {
        fprintf( stderr, "Could not load symbol %s\n",
                 pSymbolname );
        bSaneSymbolLoadFailed = true;
    }
    return pFunction;
}

SANE_Status Sane::ControlOption( int nOption, SANE_Action nAction,
                                 void* pData )
{
    SANE_Int    nInfo = 0;

    SANE_Status nStatus = p_control_option( maHandle, static_cast<SANE_Int>(nOption),
                                nAction, pData, &nInfo );
    DUMP_STATE( nStatus, "sane_control_option" );
#if OSL_DEBUG_LEVEL > 0
    if( nStatus != SANE_STATUS_GOOD )
    {
        const char* pAction = "Unknown";
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
                 OUStringToOString(GetOptionName(nOption), osl_getThreadTextEncoding()).getStr(),
                 pAction );
    }
#endif
    if( nInfo &  SANE_INFO_RELOAD_OPTIONS )
        ReloadOptions();
    return nStatus;
}

Sane::Sane() :
        mppOptions( nullptr ),
        mnOptions( 0 ),
        mnDevice( -1 ),
        maHandle( nullptr )
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
    OUString sSaneLibName( "libsane" SAL_DLLEXTENSION  );
    pSaneLib = osl_loadModule( sSaneLibName.pData, SAL_LOADMODULE_LAZY );
    if( ! pSaneLib )
    {
        sSaneLibName = "libsane" SAL_DLLEXTENSION ".1";
        pSaneLib = osl_loadModule( sSaneLibName.pData, SAL_LOADMODULE_LAZY );
    }
    // try reasonable places that might not be in the library search path
    if( ! pSaneLib )
    {
        OUString sSaneLibSystemPath( "/usr/local/lib/libsane" SAL_DLLEXTENSION  );
        osl_getFileURLFromSystemPath( sSaneLibSystemPath.pData, &sSaneLibName.pData );
        pSaneLib = osl_loadModule( sSaneLibName.pData, SAL_LOADMODULE_LAZY );
    }

    if( pSaneLib )
    {
        bSaneSymbolLoadFailed = false;
        p_init = reinterpret_cast<SANE_Status(*)(SANE_Int*, SANE_Auth_Callback )>(
            LoadSymbol( "sane_init" ));
        p_exit = reinterpret_cast<void(*)()>(
            LoadSymbol( "sane_exit" ));
        p_get_devices = reinterpret_cast<SANE_Status(*)(const SANE_Device***,
                                        SANE_Bool )>(
            LoadSymbol( "sane_get_devices" ));
        p_open = reinterpret_cast<SANE_Status(*)(SANE_String_Const, SANE_Handle )>(
            LoadSymbol( "sane_open" ));
        p_close = reinterpret_cast<void(*)(SANE_Handle)>(
            LoadSymbol( "sane_close" ));
        p_get_option_descriptor = reinterpret_cast<const SANE_Option_Descriptor*(*)(SANE_Handle,
                                                              SANE_Int)>(
            LoadSymbol( "sane_get_option_descriptor" ));
        p_control_option = reinterpret_cast<SANE_Status(*)(SANE_Handle, SANE_Int,
                                           SANE_Action, void*, SANE_Int*)>(
            LoadSymbol( "sane_control_option" ));
        p_get_parameters = reinterpret_cast<SANE_Status(*)(SANE_Handle,SANE_Parameters*)>(
            LoadSymbol( "sane_get_parameters" ));
        p_start = reinterpret_cast<SANE_Status(*)(SANE_Handle)>(
            LoadSymbol( "sane_start" ));
        p_read = reinterpret_cast<SANE_Status(*)(SANE_Handle, SANE_Byte*,
                                 SANE_Int, SANE_Int* )>(
            LoadSymbol( "sane_read" ));
        p_cancel = reinterpret_cast<void(*)(SANE_Handle)>(
            LoadSymbol( "sane_cancel" ));
        p_set_io_mode = reinterpret_cast<SANE_Status(*)(SANE_Handle, SANE_Bool)>(
            LoadSymbol( "sane_set_io_mode" ));
        p_get_select_fd = reinterpret_cast<SANE_Status(*)(SANE_Handle, SANE_Int*)>(
            LoadSymbol( "sane_get_select_fd" ));
        p_strstatus = reinterpret_cast<SANE_String_Const(*)(SANE_Status)>(
            LoadSymbol( "sane_strstatus" ));
        if( bSaneSymbolLoadFailed )
            DeInit();
        else
        {
            SANE_Status nStatus = p_init( &nVersion, nullptr );
            FAIL_SHUTDOWN_STATE( nStatus, "sane_init", );
            nStatus = p_get_devices( const_cast<const SANE_Device***>(&ppDevices),
                                     SANE_FALSE );
            FAIL_SHUTDOWN_STATE( nStatus, "sane_get_devices", );
            for( nDevices = 0 ; ppDevices[ nDevices ]; nDevices++ ) ;
        }
    }
#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
    else
        fprintf( stderr, "libsane%s could not be opened: %s\n", SAL_DLLEXTENSION,
                 dlerror() );
#endif
}

void Sane::DeInit()
{
    if( pSaneLib )
    {
        p_exit();
        osl_unloadModule( pSaneLib );
        pSaneLib = nullptr;
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

    const SANE_Option_Descriptor* pZero = p_get_option_descriptor( maHandle, 0 );
    SANE_Word pOptions[2];
    SANE_Status nStatus = p_control_option( maHandle, 0, SANE_ACTION_GET_VALUE,
                                            static_cast<void*>(pOptions), nullptr );
    if( nStatus != SANE_STATUS_GOOD )
        fprintf( stderr, "Error: sane driver returned %s while reading number of options !\n", p_strstatus( nStatus ) );

    mnOptions = pOptions[ 0 ];
    if( static_cast<size_t>(pZero->size) > sizeof( SANE_Word ) )
        fprintf( stderr, "driver returned number of options with larger size tha SANE_Word !!!\n" );
    mppOptions.reset(new const SANE_Option_Descriptor*[ mnOptions ]);
    mppOptions[ 0 ] = pZero;
    for( int i = 1; i < mnOptions; i++ )
        mppOptions[ i ] = p_get_option_descriptor( maHandle, i );

    CheckConsistency( nullptr, true );

    maReloadOptionsLink.Call( *this );
}

bool Sane::Open( const char* name )
{
    SANE_Status nStatus = p_open( reinterpret_cast<SANE_String_Const>(name), &maHandle );
    FAIL_STATE( nStatus, "sane_open", false );

    ReloadOptions();

    if( mnDevice == -1 )
    {
        OString aDevice( name );
        for( int i = 0; i < nDevices; i++ )
        {
            if( aDevice == ppDevices[i]->name )
            {
                mnDevice = i;
                break;
            }
        }
    }

    return true;
}

bool Sane::Open( int n )
{
    if( n >= 0 && n < nDevices )
    {
        mnDevice = n;
        return Open( ppDevices[n]->name );
    }
    return false;
}

void Sane::Close()
{
    if( maHandle )
    {
        p_close( maHandle );
        mppOptions.reset();
        maHandle = nullptr;
        mnDevice = -1;
    }
}

int Sane::GetOptionByName( const char* rName )
{
    int i;
    OString aOption( rName );
    for( i = 0; i < mnOptions; i++ )
    {
        if( mppOptions[i]->name && aOption == mppOptions[i]->name )
            return i;
    }
    return -1;
}

bool Sane::GetOptionValue( int n, bool& rRet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_BOOL )
        return false;
    SANE_Word nRet;
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, &nRet );
    if( nStatus != SANE_STATUS_GOOD )
        return false;

    rRet = nRet;
    return true;
}

bool Sane::GetOptionValue( int n, OString& rRet )
{
    bool bSuccess = false;
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_STRING )
        return false;
    std::unique_ptr<char[]> pRet(new char[mppOptions[n]->size+1]);
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pRet.get() );
    if( nStatus == SANE_STATUS_GOOD )
    {
        bSuccess = true;
        rRet = pRet.get();
    }
    return bSuccess;
}

bool Sane::GetOptionValue( int n, double& rRet, int nElement )
{
    bool bSuccess = false;

    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return false;

    std::unique_ptr<SANE_Word[]> pRet(new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)]);
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pRet.get() );
    if( nStatus == SANE_STATUS_GOOD )
    {
        bSuccess = true;
        if( mppOptions[n]->type == SANE_TYPE_INT )
            rRet = static_cast<double>(pRet[ nElement ]);
        else
            rRet = SANE_UNFIX( pRet[nElement] );
    }
    return bSuccess;
}

bool Sane::GetOptionValue( int n, double* pSet )
{
    if( ! maHandle  || ! ( mppOptions[n]->type == SANE_TYPE_FIXED ||
                           mppOptions[n]->type == SANE_TYPE_INT ) )
        return false;

    std::unique_ptr<SANE_Word[]> pFixedSet(new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)]);
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pFixedSet.get() );
    if( nStatus != SANE_STATUS_GOOD )
        return false;
    for( size_t i = 0; i <mppOptions[n]->size/sizeof(SANE_Word); i++ )
    {
        if( mppOptions[n]->type == SANE_TYPE_FIXED )
            pSet[i] = SANE_UNFIX( pFixedSet[i] );
        else
            pSet[i] = static_cast<double>(pFixedSet[i]);
    }
    return true;
}

void Sane::SetOptionValue( int n, bool bSet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_BOOL )
        return;
    SANE_Word nRet = bSet ? SANE_TRUE : SANE_FALSE;
    ControlOption( n, SANE_ACTION_SET_VALUE, &nRet );
}

void Sane::SetOptionValue( int n, const OUString& rSet )
{
    if( ! maHandle  ||  mppOptions[n]->type != SANE_TYPE_STRING )
        return;
    OString aSet(OUStringToOString(rSet, osl_getThreadTextEncoding()));
    ControlOption( n, SANE_ACTION_SET_VALUE, const_cast<char *>(aSet.getStr()) );
}

void Sane::SetOptionValue( int n, double fSet, int nElement )
{
    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return;

    if( mppOptions[n]->size/sizeof(SANE_Word) > 1 )
    {
        std::unique_ptr<SANE_Word[]> pSet(new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)]);
        SANE_Status nStatus = ControlOption( n, SANE_ACTION_GET_VALUE, pSet.get() );
        if( nStatus == SANE_STATUS_GOOD )
        {
            pSet[nElement] = mppOptions[n]->type == SANE_TYPE_INT ?
                static_cast<SANE_Word>(fSet) : SANE_FIX( fSet );
            ControlOption(  n, SANE_ACTION_SET_VALUE, pSet.get() );
        }
    }
    else
    {
        SANE_Word nSetTo =
            mppOptions[n]->type == SANE_TYPE_INT ?
            static_cast<SANE_Word>(fSet) : SANE_FIX( fSet );

        ControlOption( n, SANE_ACTION_SET_VALUE, &nSetTo );
    }
}

void Sane::SetOptionValue( int n, double const * pSet )
{
    if( ! maHandle  ||  ( mppOptions[n]->type != SANE_TYPE_INT &&
                          mppOptions[n]->type != SANE_TYPE_FIXED ) )
        return;
    std::unique_ptr<SANE_Word[]> pFixedSet(new SANE_Word[mppOptions[n]->size/sizeof(SANE_Word)]);
    for( size_t i = 0; i < mppOptions[n]->size/sizeof(SANE_Word); i++ )
    {
        if( mppOptions[n]->type == SANE_TYPE_FIXED )
            pFixedSet[i] = SANE_FIX( pSet[i] );
        else
            pFixedSet[i] = static_cast<SANE_Word>(pSet[i]);
    }
    ControlOption( n, SANE_ACTION_SET_VALUE, pFixedSet.get() );
}

enum FrameStyleType {
    FrameStyle_BW, FrameStyle_Gray, FrameStyle_RGB, FrameStyle_Separated
};

#define BYTE_BUFFER_SIZE 32768

static inline sal_uInt8 ReadValue( FILE* fp, int depth )
{
    if( depth == 16 )
    {
        sal_uInt16 nWord;
        // data always come in native byte order !
        // 16 bits is not really supported by backends as of now
        // e.g. UMAX Astra 1200S delivers 16 bit but in BIGENDIAN
        // against SANE documentation (xscanimage gets the same result
        // as we do
        size_t items_read = fread( &nWord, 1, 2, fp );

        if (items_read != 2)
        {
             SAL_WARN( "extensions.scanner", "short read, abandoning" );
             return 0;
        }

        return static_cast<sal_uInt8>( nWord / 256 );
    }
    sal_uInt8 nByte;
    size_t items_read = fread( &nByte, 1, 1, fp );
    if (items_read != 1)
    {
        SAL_WARN( "extensions.scanner", "short read, abandoning" );
        return 0;
    }
    return nByte;
}

bool Sane::CheckConsistency( const char* pMes, bool bInit )
{
    static const SANE_Option_Descriptor** pDescArray = nullptr;
    static const SANE_Option_Descriptor*  pZero = nullptr;

    if( bInit )
    {
        pDescArray = mppOptions.get();
        if( mppOptions )
            pZero = mppOptions[0];
        return true;
    }

    bool bConsistent = true;

    if( pDescArray != mppOptions.get() )
        bConsistent = false;
    if( pZero != mppOptions[0] )
        bConsistent = false;

    if( ! bConsistent )
        dbg_msg( "Sane is not consistent. (%s)\n", pMes );

    return bConsistent;
}

bool Sane::Start( BitmapTransporter& rBitmap )
{
    int nStream = 0, nLine = 0, i = 0;
    SANE_Parameters aParams;
    FrameStyleType eType = FrameStyle_Gray;
    bool bSuccess = true;
    bool bWidthSet = false;

    if( ! maHandle )
        return false;

    int nWidthMM    = 0;
    int nHeightMM   = 0;
    double fTLx, fTLy, fResl = 0.0;
    int nOption;
    if( ( nOption = GetOptionByName( "tl-x" ) ) != -1   &&
        GetOptionValue( nOption, fTLx )              &&
        GetOptionUnit( nOption ) == SANE_UNIT_MM )
    {
        double fBRx;
        if( ( nOption = GetOptionByName( "br-x" ) ) != -1   &&
            GetOptionValue( nOption, fBRx )              &&
            GetOptionUnit( nOption ) == SANE_UNIT_MM )
        {
            nWidthMM = static_cast<int>(fabs(fBRx - fTLx));
        }
    }
    if( ( nOption = GetOptionByName( "tl-y" ) ) != -1   &&
        GetOptionValue( nOption, fTLy )              &&
        GetOptionUnit( nOption ) == SANE_UNIT_MM )
    {
        double fBRy;
        if( ( nOption = GetOptionByName( "br-y" ) ) != -1   &&
            GetOptionValue( nOption, fBRy )              &&
            GetOptionUnit( nOption ) == SANE_UNIT_MM )
        {
            nHeightMM = static_cast<int>(fabs(fBRy - fTLy));
        }
    }
    if( ( nOption = GetOptionByName( "resolution" ) ) != -1 )
        (void)GetOptionValue( nOption, fResl );

    sal_uInt8* pBuffer = nullptr;

    SANE_Status nStatus = SANE_STATUS_GOOD;

    rBitmap.lock();
    SvMemoryStream& aConverter = rBitmap.getStream();
    aConverter.Seek( 0 );
    aConverter.SetEndian( SvStreamEndian::LITTLE );

    // write bitmap stream header
    aConverter.WriteChar( 'B' ).WriteChar( 'M' );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 60 );

    // write BITMAPINFOHEADER
    aConverter.WriteUInt32( 40 );
    aConverter.WriteUInt32( 0 ); // fill in width later
    aConverter.WriteUInt32( 0 ); // fill in height later
    aConverter.WriteUInt16( 1 );
    // create header for 24 bits
    // correct later if necessary
    aConverter.WriteUInt16( 24 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );
    aConverter.WriteUInt32( 0 );

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
            if (nStatus != SANE_STATUS_GOOD || aParams.bytes_per_line == 0)
            {
                bSuccess = false;
                break;
            }
#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
            const char* const ppFormats[] = { "SANE_FRAME_GRAY", "SANE_FRAME_RGB",
                                  "SANE_FRAME_RED", "SANE_FRAME_GREEN",
                                  "SANE_FRAME_BLUE", "Unknown !!!" };
            fprintf( stderr, "Parameters for frame %d:\n", nStream );
            if( static_cast<
                    typename std::make_unsigned<
                        typename std::underlying_type<SANE_Frame>::type>::type>(
                            aParams.format)
                > 4 )
            {
                aParams.format = SANE_Frame(5);
            }
            fprintf( stderr, "format:           %s\n", ppFormats[ static_cast<int>(aParams.format) ] );
            fprintf( stderr, "last_frame:       %s\n", aParams.last_frame ? "TRUE" : "FALSE" );
            fprintf( stderr, "depth:            %d\n", static_cast<int>(aParams.depth) );
            fprintf( stderr, "pixels_per_line:  %d\n", static_cast<int>(aParams.pixels_per_line) );
            fprintf( stderr, "bytes_per_line:   %d\n", static_cast<int>(aParams.bytes_per_line) );
#endif
            if( ! pBuffer )
            {
                pBuffer = new sal_uInt8[ BYTE_BUFFER_SIZE < 4*aParams.bytes_per_line ? 4*aParams.bytes_per_line : BYTE_BUFFER_SIZE ];
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

            bool bSynchronousRead = true;

            // should be fail safe, but ... ??
            nStatus = p_set_io_mode( maHandle, SANE_FALSE );
            CheckConsistency( "sane_set_io_mode" );
            if( nStatus != SANE_STATUS_GOOD )
            {
                bSynchronousRead = false;
                nStatus = p_set_io_mode(maHandle, SANE_TRUE);
                CheckConsistency( "sane_set_io_mode" );
                if (nStatus != SANE_STATUS_GOOD)
                {
                    SAL_WARN("extensions.scanner", "SANE driver status is: " << nStatus);
                }
            }

            SANE_Int nLen=0;
            SANE_Int fd = 0;

            if( ! bSynchronousRead )
            {
                nStatus = p_get_select_fd( maHandle, &fd );
                DUMP_STATE( nStatus, "sane_get_select_fd" );
                CheckConsistency( "sane_get_select_fd" );
                if( nStatus != SANE_STATUS_GOOD )
                    bSynchronousRead = true;
            }
            utl::TempFile aFrame;
            aFrame.EnableKillingFile();
            FILE* pFrame = fopen(OUStringToOString(aFrame.GetFileName(), osl_getThreadTextEncoding()).getStr(), "w+b");
            if( ! pFrame )
            {
                bSuccess = false;
                break;
            }
            do {
                if( ! bSynchronousRead )
                {
                    fd_set fdset;
                    struct timeval tv;

                    FD_ZERO( &fdset );
                    FD_SET( static_cast<int>(fd), &fdset );
                    tv.tv_sec = 5;
                    tv.tv_usec = 0;
                    if( select( fd+1, &fdset, nullptr, nullptr, &tv ) == 0 )
                        fprintf( stderr, "Timeout on sane_read descriptor\n" );
                }
                nLen = 0;
                nStatus = p_read( maHandle, pBuffer, BYTE_BUFFER_SIZE, &nLen );
                CheckConsistency( "sane_read" );
                if( nLen && ( nStatus == SANE_STATUS_GOOD ||
                              nStatus == SANE_STATUS_EOF ) )
                {
                    bSuccess = (static_cast<size_t>(nLen) == fwrite( pBuffer, 1, nLen, pFrame ));
                    if (!bSuccess)
                        break;
                }
                else
                    DUMP_STATE( nStatus, "sane_read" );
            } while( nStatus == SANE_STATUS_GOOD );
            if (nStatus != SANE_STATUS_EOF || !bSuccess)
            {
                fclose( pFrame );
                bSuccess = false;
                break;
            }

            int nFrameLength = ftell( pFrame );
            fseek( pFrame, 0, SEEK_SET );
            sal_uInt32 nWidth = static_cast<sal_uInt32>(aParams.pixels_per_line);
            sal_uInt32 nHeight = static_cast<sal_uInt32>(nFrameLength / aParams.bytes_per_line);
            if( ! bWidthSet )
            {
                if( ! fResl )
                    fResl = 300; // if all else fails that's a good guess
                if( ! nWidthMM )
                    nWidthMM = static_cast<int>((static_cast<double>(nWidth) / fResl) * 25.4);
                if( ! nHeightMM )
                    nHeightMM = static_cast<int>((static_cast<double>(nHeight) / fResl) * 25.4);
                SAL_INFO("extensions.scanner", "set dimensions to(" << nWidth << ", " << nHeight << ") Pixel, (" << nWidthMM << ", " << nHeightMM <<
                    ") mm, resolution is " << fResl);

                aConverter.Seek( 18 );
                aConverter.WriteUInt32( nWidth );
                aConverter.WriteUInt32( nHeight );
                aConverter.Seek( 38 );
                aConverter.WriteUInt32( 1000*nWidth/nWidthMM );
                aConverter.WriteUInt32( 1000*nHeight/nHeightMM );
                bWidthSet = true;
            }
            aConverter.Seek(60);

            if( eType == FrameStyle_BW )
            {
                aConverter.Seek( 10 );
                aConverter.WriteUInt32( 64 );
                aConverter.Seek( 28 );
                aConverter.WriteUInt16( 1 );
                aConverter.Seek( 54 );
                // write color table
                aConverter.WriteUInt16( 0xffff );
                aConverter.WriteUChar( 0xff );
                aConverter.WriteUChar( 0 );
                aConverter.WriteUInt32( 0 );
                aConverter.Seek( 64 );
            }
            else if( eType == FrameStyle_Gray )
            {
                 aConverter.Seek( 10 );
                 aConverter.WriteUInt32( 1084 );
                aConverter.Seek( 28 );
                aConverter.WriteUInt16( 8 );
                aConverter.Seek( 54 );
                // write color table
                for( nLine = 0; nLine < 256; nLine++ )
                {
                    aConverter.WriteUChar( nLine );
                    aConverter.WriteUChar( nLine );
                    aConverter.WriteUChar( nLine );
                    aConverter.WriteUChar( 0 );
                }
                aConverter.Seek( 1084 );
            }

            for (nLine = nHeight-1; nLine >= 0; --nLine)
            {
                fseek( pFrame, nLine * aParams.bytes_per_line, SEEK_SET );
                if( eType == FrameStyle_BW ||
                    ( eType == FrameStyle_Gray && aParams.depth == 8 )
                    )
                {
                    SANE_Int items_read = fread( pBuffer, 1, aParams.bytes_per_line, pFrame );
                    if (items_read != aParams.bytes_per_line)
                    {
                        SAL_WARN( "extensions.scanner", "short read, padding with zeros" );
                        memset(pBuffer + items_read, 0, aParams.bytes_per_line - items_read);
                    }
                    aConverter.WriteBytes(pBuffer, aParams.bytes_per_line);
                }
                else if( eType == FrameStyle_Gray )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        sal_uInt8 nGray = ReadValue( pFrame, aParams.depth );
                        aConverter.WriteUChar( nGray );
                    }
                }
                else if( eType == FrameStyle_RGB )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        sal_uInt8 nRed, nGreen, nBlue;
                        nRed    = ReadValue( pFrame, aParams.depth );
                        nGreen  = ReadValue( pFrame, aParams.depth );
                        nBlue   = ReadValue( pFrame, aParams.depth );
                        aConverter.WriteUChar( nBlue );
                        aConverter.WriteUChar( nGreen );
                        aConverter.WriteUChar( nRed );
                    }
                }
                else if( eType == FrameStyle_Separated )
                {
                    for( i = 0; i < (aParams.pixels_per_line); i++ )
                    {
                        sal_uInt8 nValue = ReadValue( pFrame, aParams.depth );
                        switch( aParams.format )
                        {
                            case SANE_FRAME_RED:
                                aConverter.SeekRel( 2 );
                                aConverter.WriteUChar( nValue );
                                break;
                            case SANE_FRAME_GREEN:
                                aConverter.SeekRel( 1 );
                                aConverter.WriteUChar( nValue );
                                aConverter.SeekRel( 1 );
                                break;
                            case SANE_FRAME_BLUE:
                                aConverter.WriteUChar( nValue );
                                aConverter.SeekRel( 2 );
                                break;
                            case SANE_FRAME_GRAY:
                            case SANE_FRAME_RGB:
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
            bSuccess = false;
    }
    // get stream length
    aConverter.Seek( STREAM_SEEK_TO_END );
    int nPos = aConverter.Tell();

    aConverter.Seek( 2 );
    aConverter.WriteUInt32( nPos+1 );
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
        delete [] pBuffer;

    ReloadOptions();


    dbg_msg( "Sane::Start returns with %s\n", bSuccess ? "TRUE" : "FALSE" );

    return bSuccess;
}

int Sane::GetRange( int n, std::unique_ptr<double[]>& rpDouble )
{
    if( mppOptions[n]->constraint_type != SANE_CONSTRAINT_RANGE &&
        mppOptions[n]->constraint_type != SANE_CONSTRAINT_WORD_LIST )
    {
        return -1;
    }

    rpDouble = nullptr;
    int nItems, i;
    bool bIsFixed = mppOptions[n]->type == SANE_TYPE_FIXED;

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
            fMin = static_cast<double>(mppOptions[n]->constraint.range->min);
            fMax = static_cast<double>(mppOptions[n]->constraint.range->max);
            fQuant = static_cast<double>(mppOptions[n]->constraint.range->quant);
        }
        if( fQuant != 0.0 )
        {
            dbg_msg( "quantum range [ %lg ; %lg ; %lg ]\n",
                     fMin, fQuant, fMax );
            nItems = static_cast<int>((fMax - fMin)/fQuant)+1;
            rpDouble.reset(new double[ nItems ]);
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
            rpDouble.reset(new double[2]);
            rpDouble[0] = fMin;
            rpDouble[1] = fMax;
            return 0;
        }
    }
    else
    {
        nItems = mppOptions[n]->constraint.word_list[0];
        rpDouble.reset(new double[nItems]);
        for( i=0; i<nItems; i++ )
        {
            rpDouble[i] = bIsFixed ?
                SANE_UNFIX( mppOptions[n]->constraint.word_list[i+1] ) :
                static_cast<double>(mppOptions[n]->constraint.word_list[i+1]);
        }
        dbg_msg( "wordlist [ %lg ... %lg ]\n",
                 rpDouble[ 0 ], rpDouble[ nItems-1 ] );
        return nItems;
    }
}

static const char *ppUnits[] = {
    "",
    "[Pixel]",
    "[Bit]",
    "[mm]",
    "[DPI]",
    "[%]",
    "[usec]"
};

OUString Sane::GetOptionUnitName( int n )
{
    OUString aText;
    SANE_Unit nUnit = mppOptions[n]->unit;
    size_t nUnitAsSize = static_cast<size_t>(nUnit);
    if (nUnitAsSize >= SAL_N_ELEMENTS( ppUnits ))
        aText = "[unknown units]";
    else
        aText = OUString( ppUnits[ nUnit ], strlen(ppUnits[ nUnit ]), osl_getThreadTextEncoding() );
    return aText;
}

bool Sane::ActivateButtonOption( int n )
{
    SANE_Status nStatus = ControlOption( n, SANE_ACTION_SET_VALUE, nullptr );
    if( nStatus != SANE_STATUS_GOOD )
        return false;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
