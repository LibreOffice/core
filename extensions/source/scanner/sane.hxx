/*************************************************************************
 *
 *  $RCSfile: sane.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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
#ifndef _SANE_HXX
#define _SANE_HXX

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
    vos::OMutex                         m_aProtector;

public:

                                        BitmapTransporter() {}
    virtual                             ~BitmapTransporter() {}


    // XInterface
    virtual ANY SAL_CALL                queryInterface( const Type & rType ) throw( RuntimeException );
    virtual void SAL_CALL               acquire() throw( RuntimeException ) { OWeakObject::acquire(); }
    virtual void SAL_CALL               release() throw( RuntimeException ) { OWeakObject::release(); }

    virtual AWT::Size SAL_CALL          getSize();
    virtual SEQ( sal_Int8 ) SAL_CALL    getDIB();
    virtual SEQ( sal_Int8 ) SAL_CALL    getMaskDIB() { return SEQ( sal_Int8 )(); }

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
    static void*            pSaneLib;

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
    static const SANE_String_Const (*p_strstatus)( SANE_Status );

    static SANE_Int             nVersion;
    static SANE_Device**        ppDevices;
    static int                  nDevices;

    const SANE_Option_Descriptor**  mppOptions;
    int                             mnOptions;
    int                             mnDevice;
    SANE_Handle                     maHandle;

    Link                        maReloadOptionsLink;

    inline void*    LoadSymbol( char* );
    void            Init();
    void            DeInit();
    void            Stop();

    SANE_Status ControlOption( int, SANE_Action, void* );

    BOOL CheckConsistency( const char*, BOOL bInit = FALSE );

public:
    Sane();
    ~Sane();

    static BOOL         IsSane()
        { return pSaneLib ? TRUE : FALSE; }
    BOOL            IsOpen()
        { return maHandle ? TRUE : FALSE; }
    static int              CountDevices()
        { return nDevices; }
    static String           GetName( int n )
        { return String( ppDevices[n]->name, gsl_getSystemTextEncoding() ); }
    static String           GetVendor( int n )
        { return String( ppDevices[n]->vendor, gsl_getSystemTextEncoding() ); }
    static String           GetModel( int n )
        { return String( ppDevices[n]->model, gsl_getSystemTextEncoding() ); }
    static String           GetType( int n )
        { return String( ppDevices[n]->type, gsl_getSystemTextEncoding() ); }

    String          GetOptionName( int n )
        { return String( (char*)mppOptions[n]->name, gsl_getSystemTextEncoding() ); }
    String          GetOptionTitle( int n )
        { return String( (char*)mppOptions[n]->title, gsl_getSystemTextEncoding() ); }
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
    BOOL            GetOptionValue( int, BOOL& );
    BOOL            GetOptionValue( int, ByteString& );
    BOOL            GetOptionValue( int, double&, int nElement = 0 );
    BOOL            GetOptionValue( int, double* );

    BOOL            SetOptionValue( int, BOOL );
    BOOL            SetOptionValue( int, const String& );
    BOOL            SetOptionValue( int, double, int nElement = 0 );
    BOOL            SetOptionValue( int, double* );

    BOOL            ActivateButtonOption( int );

    int             CountOptions() { return mnOptions; }
    int             GetDeviceNumber() { return mnDevice; }

    BOOL            Open( const char* );
    BOOL            Open( int );
    void            Close();
    void            ReloadDevices();
    void            ReloadOptions();

    BOOL            Start( BitmapTransporter& );

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
