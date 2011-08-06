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
#ifndef _SANE_HXX
#define _SANE_HXX

#include <osl/thread.h>
#include <osl/module.h>
#include <tools/string.hxx>
#include <vcl/bitmap.hxx>
#include <sane/sane.h>
#include <scanner.hxx>

// ---------------------
// - BitmapTransporter -
// ---------------------

class BitmapTransporter : public OWeakObject, AWT::XBitmap
{
    SvMemoryStream                      m_aStream;
    osl::Mutex                          m_aProtector;

public:

                                        BitmapTransporter();
    virtual                             ~BitmapTransporter();


    // XInterface
    virtual ANY SAL_CALL                queryInterface( const Type & rType ) throw( RuntimeException );
    virtual void SAL_CALL               acquire() throw() { OWeakObject::acquire(); }
    virtual void SAL_CALL               release() throw() { OWeakObject::release(); }

    virtual AWT::Size SAL_CALL          getSize() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL    getDIB() throw();
    virtual SEQ( sal_Int8 ) SAL_CALL    getMaskDIB() throw() { return SEQ( sal_Int8 )(); }

    // Misc
    void                                lock() { m_aProtector.acquire(); }
    void                                unlock() { m_aProtector.release(); }
    SvMemoryStream&                     getStream() { return m_aStream; }
};

// --------
// - Sane -
// --------

class Sane
{
private:
    static int              nRefCount;
    static oslModule        pSaneLib;

    static SANE_Status      (*p_init)( SANE_Int*,
                                       SANE_Auth_Callback );
    static void             (*p_exit)();
    static SANE_Status      (*p_get_devices)( const SANE_Device***,
                                              SANE_Bool );
    static SANE_Status      (*p_open)( SANE_String_Const, SANE_Handle );
    static void             (*p_close)( SANE_Handle );
    static const SANE_Option_Descriptor* (*p_get_option_descriptor)(
        SANE_Handle, SANE_Int );
    static SANE_Status      (*p_control_option)( SANE_Handle, SANE_Int,
                                                 SANE_Action, void*,
                                                 SANE_Int* );
    static SANE_Status      (*p_get_parameters)( SANE_Handle,
                                                 SANE_Parameters* );
    static SANE_Status      (*p_start)( SANE_Handle );
    static SANE_Status      (*p_read)( SANE_Handle, SANE_Byte*, SANE_Int,
                                       SANE_Int* );
    static void             (*p_cancel)( SANE_Handle );
    static SANE_Status      (*p_set_io_mode)( SANE_Handle, SANE_Bool );
    static SANE_Status      (*p_get_select_fd)( SANE_Handle, SANE_Int* );
    static SANE_String_Const (*p_strstatus)( SANE_Status );

    static SANE_Int             nVersion;
    static SANE_Device**        ppDevices;
    static int                  nDevices;

    const SANE_Option_Descriptor**  mppOptions;
    int                             mnOptions;
    int                             mnDevice;
    SANE_Handle                     maHandle;

    Link                        maReloadOptionsLink;

    inline oslGenericFunction
                    LoadSymbol( const char* );
    void            Init();
    void            DeInit();

    SANE_Status ControlOption( int, SANE_Action, void* );

    sal_Bool CheckConsistency( const char*, sal_Bool bInit = sal_False );

public:
    Sane();
    ~Sane();

    static sal_Bool         IsSane()
        { return pSaneLib ? sal_True : sal_False; }
    sal_Bool            IsOpen()
        { return maHandle ? sal_True : sal_False; }
    static int              CountDevices()
        { return nDevices; }
    static String           GetName( int n )
        { return String( ppDevices[n]->name ? ppDevices[n]->name : "", osl_getThreadTextEncoding() ); }
    static String           GetVendor( int n )
        { return String( ppDevices[n]->vendor ? ppDevices[n]->vendor : "", osl_getThreadTextEncoding() ); }
    static String           GetModel( int n )
        { return String( ppDevices[n]->model ? ppDevices[n]->model : "", osl_getThreadTextEncoding() ); }
    static String           GetType( int n )
        { return String( ppDevices[n]->type ? ppDevices[n]->type : "", osl_getThreadTextEncoding() ); }

    String          GetOptionName( int n )
        { return String( mppOptions[n]->name ? (char*)mppOptions[n]->name : "", osl_getThreadTextEncoding() ); }
    String          GetOptionTitle( int n )
        { return String( mppOptions[n]->title ? (char*)mppOptions[n]->title : "", osl_getThreadTextEncoding() ); }
    SANE_Value_Type GetOptionType( int n )
        { return mppOptions[n]->type; }
    SANE_Unit       GetOptionUnit( int n )
        { return mppOptions[n]->unit; }
    String          GetOptionUnitName( int n );
    SANE_Int        GetOptionCap( int n )
        { return mppOptions[n]->cap; }
    SANE_Constraint_Type GetOptionConstraintType( int n )
        { return mppOptions[n]->constraint_type; }
    const char**    GetStringConstraint( int n )
        { return (const char**)mppOptions[n]->constraint.string_list; }
    int             GetRange( int, double*& );

    inline int      GetOptionElements( int n );
    int             GetOptionByName( const char* );
    sal_Bool            GetOptionValue( int, sal_Bool& );
    sal_Bool            GetOptionValue( int, ByteString& );
    sal_Bool            GetOptionValue( int, double&, int nElement = 0 );
    sal_Bool            GetOptionValue( int, double* );

    sal_Bool            SetOptionValue( int, sal_Bool );
    sal_Bool            SetOptionValue( int, const String& );
    sal_Bool            SetOptionValue( int, double, int nElement = 0 );
    sal_Bool            SetOptionValue( int, double* );

    sal_Bool            ActivateButtonOption( int );

    int             CountOptions() { return mnOptions; }
    int             GetDeviceNumber() { return mnDevice; }

    sal_Bool            Open( const char* );
    sal_Bool            Open( int );
    void            Close();
    void            ReloadDevices();
    void            ReloadOptions();

    sal_Bool            Start( BitmapTransporter& );

    inline Link     SetReloadOptionsHdl( const Link& rLink );
};

inline int Sane::GetOptionElements( int n )
{
    if( mppOptions[n]->type == SANE_TYPE_FIXED ||
        mppOptions[n]->type == SANE_TYPE_INT )
    {
        return mppOptions[n]->size/sizeof( SANE_Word );
    }
    return 1;
}

inline Link Sane::SetReloadOptionsHdl( const Link& rLink )
{
    Link aRet = maReloadOptionsLink;
    maReloadOptionsLink = rLink;
    return aRet;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
