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
#ifndef INCLUDED_UNOTOOLS_UCBLOCKBYTES_HXX
#define INCLUDED_UNOTOOLS_UCBLOCKBYTES_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/unotoolsdllapi.h>

#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/link.hxx>
#include <tools/errcode.hxx>
#include <tools/datetime.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace task
            {
                class XInteractionHandler;
            }
            namespace io
            {
                class XStream;
                class XInputStream;
                class XOutputStream;
                class XSeekable;
            }
            namespace ucb
            {
                class XContent;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

namespace utl
{
SV_DECL_REF( UcbLockBytes )

class UcbLockBytesHandler : public SvRefBase
{
    bool            m_bActive;
public:
    enum LoadHandlerItem
    {
        DATA_AVAILABLE,
        DONE,
        CANCEL
    };

                    UcbLockBytesHandler()
                        : m_bActive( true )
                    {}

    virtual void    Handle( LoadHandlerItem nWhich, UcbLockBytesRef xLockBytes ) = 0;
    void            Activate( bool bActivate = true ) { m_bActive = bActivate; }
    bool            IsActive() const { return m_bActive; }
};

SV_DECL_IMPL_REF( UcbLockBytesHandler )

class UNOTOOLS_DLLPUBLIC UcbLockBytes : public virtual SvLockBytes
{
    osl::Condition          m_aInitialized;
    osl::Condition          m_aTerminated;
    osl::Mutex              m_aMutex;

    OUString                m_aContentType;
    OUString                m_aRealURL;
    DateTime                m_aExpireDate;

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >  m_xInputStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > m_xOutputStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable >     m_xSeekable;
    void*                   m_pCommandThread; // is alive only for compatibility reasons
    UcbLockBytesHandlerRef  m_xHandler;

    sal_uInt32              m_nRead;
    sal_uInt32              m_nSize;
    ErrCode                 m_nError;

    bool                    m_bTerminated;
    bool                    m_bDontClose;
    bool                    m_bStreamValid;

    DECL_LINK(              DataAvailHdl, void * );

                            UcbLockBytes( UcbLockBytesHandler* pHandler=NULL );
protected:
    virtual                 ~UcbLockBytes (void);

public:
                            // properties: Referer, PostMimeType
    static UcbLockBytesRef  CreateLockBytes( const ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XContent >& xContent,
                                            const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rProps,
                                            StreamMode eMode,
                                            const ::com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionHandler >& xInter,
                                            UcbLockBytesHandler* pHandler=0 );

    static UcbLockBytesRef  CreateInputLockBytes( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xContent );
    static UcbLockBytesRef  CreateLockBytes( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream >& xContent );

    // SvLockBytes
    virtual void            SetSynchronMode (bool bSynchron) SAL_OVERRIDE;
    virtual ErrCode         ReadAt(sal_uInt64 nPos, void *pBuffer, sal_uLong nCount, sal_uLong *pRead) const SAL_OVERRIDE;
    virtual ErrCode         WriteAt(sal_uInt64, const void*, sal_uLong, sal_uLong *pWritten) SAL_OVERRIDE;
    virtual ErrCode         Flush() const SAL_OVERRIDE;
    virtual ErrCode         SetSize(sal_uInt64) SAL_OVERRIDE;
    virtual ErrCode         Stat ( SvLockBytesStat *pStat, SvLockBytesStatFlag) const SAL_OVERRIDE;

    void                    SetError( ErrCode nError )
                            { m_nError = nError; }

    ErrCode                 GetError() const
                            { return m_nError; }

    // the following properties are available when and after the first DataAvailable callback has been executed
    OUString                GetContentType() const;
    OUString                GetRealURL() const;
    DateTime                GetExpireDate() const;

    // calling this method delegates the responsibility to call closeinput to the caller!
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > getInputStream();

    bool                    setInputStream_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > &rxInputStream,
                                                 bool bSetXSeekable = true );
    bool                    setStream_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream > &rxStream );
    void                    terminate_Impl (void);

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > getInputStream_Impl() const
                            {
                                osl::MutexGuard aGuard( (const_cast< UcbLockBytes* >(this))->m_aMutex );
                                return m_xInputStream;
                            }

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > getOutputStream_Impl() const
                            {
                                osl::MutexGuard aGuard( (const_cast< UcbLockBytes* >(this))->m_aMutex );
                                return m_xOutputStream;
                            }

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable > getSeekable_Impl() const
                            {
                                osl::MutexGuard aGuard( (const_cast< UcbLockBytes* >(this))->m_aMutex );
                                return m_xSeekable;
                            }

    bool                    hasInputStream_Impl() const
                            {
                                osl::MutexGuard aGuard( (const_cast< UcbLockBytes* >(this))->m_aMutex );
                                return m_xInputStream.is();
                            }

    void                    setDontClose_Impl()
                            { m_bDontClose = true; }

    void                    SetContentType_Impl( const OUString& rType ) { m_aContentType = rType; }
    void                    SetRealURL_Impl( const OUString& rURL )  { m_aRealURL = rURL; }
    void                    SetExpireDate_Impl( const DateTime& rDateTime )  { m_aExpireDate = rDateTime; }
    void                    SetStreamValid_Impl();
};

SV_IMPL_REF( UcbLockBytes );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
