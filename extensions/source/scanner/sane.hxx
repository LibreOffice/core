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
#ifndef _SANE_HXX
#define _SANE_HXX

#include <osl/thread.h>
#include <osl/module.h>
#include <vcl/bitmap.hxx>
#include <sane/sane.h>
#include <scanner.hxx>


// - BitmapTransporter -


class BitmapTransporter : public OWeakObject, css::awt::XBitmap
{
    SvMemoryStream                      m_aStream;
    osl::Mutex                          m_aProtector;

public:

                                        BitmapTransporter();
    virtual                             ~BitmapTransporter();


    // XInterface
    virtual Any SAL_CALL                queryInterface( const Type & rType ) throw( RuntimeException, std::exception );
    virtual void SAL_CALL               acquire() throw() { OWeakObject::acquire(); }
    virtual void SAL_CALL               release() throw() { OWeakObject::release(); }

    virtual css::awt::Size SAL_CALL          getSize() throw(std::exception);
    virtual Sequence< sal_Int8 > SAL_CALL    getDIB() throw(std::exception);
    virtual Sequence< sal_Int8 > SAL_CALL    getMaskDIB() throw(std::exception) { return Sequence< sal_Int8 >(); }

    // Misc
    void                                lock() { m_aProtector.acquire(); }
    void                                unlock() { m_aProtector.release(); }
    SvMemoryStream&                     getStream() { return m_aStream; }
};


// - Sane -


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
    static OUString         GetName( int n )
        { return ppDevices[n]->name ? OUString( ppDevices[n]->name, strlen(ppDevices[n]->name),  osl_getThreadTextEncoding() ) : OUString(); }
    static OUString         GetVendor( int n )
        { return ppDevices[n]->vendor ? OUString( ppDevices[n]->vendor, strlen(ppDevices[n]->vendor), osl_getThreadTextEncoding() ) : OUString(); }
    static OUString         GetModel( int n )
        { return ppDevices[n]->model ? OUString( ppDevices[n]->model, strlen(ppDevices[n]->model), osl_getThreadTextEncoding() ) : OUString(); }
    static OUString         GetType( int n )
        { return ppDevices[n]->type ? OUString( ppDevices[n]->type, strlen(ppDevices[n]->type), osl_getThreadTextEncoding() ) : OUString(); }

    OUString        GetOptionName( int n )
        { return mppOptions[n]->name ? OUString( (char*)mppOptions[n]->name, strlen((char*)mppOptions[n]->name), osl_getThreadTextEncoding() ) : OUString(); }
    OUString        GetOptionTitle( int n )
        { return mppOptions[n]->title ? OUString( (char*)mppOptions[n]->title, strlen((char*)mppOptions[n]->title), osl_getThreadTextEncoding() ) : OUString(); }
    SANE_Value_Type GetOptionType( int n )
        { return mppOptions[n]->type; }
    SANE_Unit       GetOptionUnit( int n )
        { return mppOptions[n]->unit; }
    OUString        GetOptionUnitName( int n );
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
    sal_Bool            GetOptionValue( int, OString& );
    sal_Bool            GetOptionValue( int, double&, int nElement = 0 );
    sal_Bool            GetOptionValue( int, double* );

    sal_Bool            SetOptionValue( int, sal_Bool );
    sal_Bool            SetOptionValue( int, const OUString& );
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
