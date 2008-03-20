/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucblockbytes.hxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-20 15:29:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UNTOOLS_UCBLOCKBYTES_HXX
#define _UNTOOLS_UCBLOCKBYTES_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#include <vos/thread.hxx>
#include <vos/conditn.hxx>
#include <vos/mutex.hxx>
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
    sal_Bool        m_bActive;
public:
    enum LoadHandlerItem
    {
        DATA_AVAILABLE,
        DONE,
        CANCEL
    };

                    UcbLockBytesHandler()
                        : m_bActive( sal_True )
                    {}

    virtual void    Handle( LoadHandlerItem nWhich, UcbLockBytesRef xLockBytes ) = 0;
    void            Activate( BOOL bActivate = sal_True ) { m_bActive = bActivate; }
    sal_Bool        IsActive() const { return m_bActive; }
};

SV_DECL_IMPL_REF( UcbLockBytesHandler )

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_UCB ::com::sun::star::ucb
#define NS_BEANS ::com::sun::star::beans
#define NS_TASK ::com::sun::star::task

class UNOTOOLS_DLLPUBLIC UcbLockBytes : public virtual SvLockBytes
{
    vos::OCondition         m_aInitialized;
    vos::OCondition         m_aTerminated;
    vos::OMutex             m_aMutex;

    String                  m_aContentType;
    String                  m_aRealURL;
    DateTime                m_aExpireDate;

    NS_UNO::Reference < NS_IO::XInputStream >  m_xInputStream;
    NS_UNO::Reference < NS_IO::XOutputStream > m_xOutputStream;
    NS_UNO::Reference < NS_IO::XSeekable >     m_xSeekable;
    void*                   m_pCommandThread; // is alive only for compatibility reasons
    UcbLockBytesHandlerRef  m_xHandler;

    sal_uInt32              m_nRead;
    sal_uInt32              m_nSize;
    ErrCode                 m_nError;

    sal_Bool                m_bTerminated : 1;
    sal_Bool                m_bDontClose : 1;
    sal_Bool                m_bStreamValid : 1;

    DECL_LINK(              DataAvailHdl, void * );

                            UcbLockBytes( UcbLockBytesHandler* pHandler=NULL );
protected:
    virtual                 ~UcbLockBytes (void);

public:
                            // properties: Referer, PostMimeType
    static UcbLockBytesRef  CreateLockBytes( const NS_UNO::Reference < NS_UCB::XContent >& xContent,
                                            const ::rtl::OUString& rReferer,
                                            const ::rtl::OUString& rMediaType,
                                            const NS_UNO::Reference < NS_IO::XInputStream >& xPostData,
                                            const NS_UNO::Reference < NS_TASK::XInteractionHandler >& xInter,
                                            UcbLockBytesHandler* pHandler=0 );

    static UcbLockBytesRef  CreateLockBytes( const NS_UNO::Reference < NS_UCB::XContent >& xContent,
                                            const NS_UNO::Sequence < NS_BEANS::PropertyValue >& rProps,
                                            StreamMode eMode,
                                            const NS_UNO::Reference < NS_TASK::XInteractionHandler >& xInter,
                                            UcbLockBytesHandler* pHandler=0 );

    static UcbLockBytesRef  CreateInputLockBytes( const NS_UNO::Reference < NS_IO::XInputStream >& xContent );
    static UcbLockBytesRef  CreateLockBytes( const NS_UNO::Reference < NS_IO::XStream >& xContent );

    // SvLockBytes
    virtual void            SetSynchronMode (BOOL bSynchron);
    virtual ErrCode         ReadAt ( ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const;
    virtual ErrCode         WriteAt ( ULONG, const void*, ULONG, ULONG *pWritten);
    virtual ErrCode         Flush (void) const;
    virtual ErrCode         SetSize (ULONG);
    virtual ErrCode         Stat ( SvLockBytesStat *pStat, SvLockBytesStatFlag) const;

    void                    SetError( ErrCode nError )
                            { m_nError = nError; }

    ErrCode                 GetError() const
                            { return m_nError; }

    void                    Cancel(); // is alive only for compatibility reasons

    // the following properties are available when and after the first DataAvailable callback has been executed
    String                  GetContentType() const;
    String                  GetRealURL() const;
    DateTime                GetExpireDate() const;

    // calling this method delegates the responsibility to call closeinput to the caller!
    NS_UNO::Reference < NS_IO::XInputStream > getInputStream();
    NS_UNO::Reference < NS_IO::XStream > getStream();

#if _SOLAR__PRIVATE
    sal_Bool                setInputStream_Impl( const NS_UNO::Reference < NS_IO::XInputStream > &rxInputStream,
                                                 sal_Bool bSetXSeekable = sal_True );
    sal_Bool                setStream_Impl( const NS_UNO::Reference < NS_IO::XStream > &rxStream );
    void                    terminate_Impl (void);

    NS_UNO::Reference < NS_IO::XInputStream > getInputStream_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xInputStream;
                            }

    NS_UNO::Reference < NS_IO::XOutputStream > getOutputStream_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xOutputStream;
                            }

    NS_UNO::Reference < NS_IO::XSeekable > getSeekable_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xSeekable;
                            }

    sal_Bool                hasInputStream_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xInputStream.is();
                            }

    void                    setDontClose_Impl()
                            { m_bDontClose = sal_True; }

    void                    SetContentType_Impl( const String& rType ) { m_aContentType = rType; }
    void                    SetRealURL_Impl( const String& rURL )  { m_aRealURL = rURL; }
    void                    SetExpireDate_Impl( const DateTime& rDateTime )  { m_aExpireDate = rDateTime; }
    void                    SetStreamValid_Impl();
#endif
};

SV_IMPL_REF( UcbLockBytes );

}

#endif
