#ifndef _UNTOOLS_UCBLOCKBYTES_HXX
#define _UNTOOLS_UCBLOCKBYTES_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#include <vos/thread.hxx>
#include <vos/conditn.hxx>
#include <vos/mutex.hxx>
#include <tools/stream.hxx>
#include <tools/link.hxx>
#include <tools/errcode.hxx>
#include <tools/datetime.hxx>

namespace utl
{
SV_DECL_REF( UcbLockBytes )

class UcbLockBytesHandler : public SvRefBase
{
    sal_Bool        m_bActive;
public:
    enum LoadHandlerItem
    {
        BEFOREWAIT,
        AFTERWAIT,
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

class CommandThread_Impl;
class UcbLockBytes : public virtual SvLockBytes
{
    vos::OCondition         m_aInitialized;
    vos::OCondition         m_aTerminated;
    vos::OMutex             m_aMutex;

    String                  m_aContentType;
    String                  m_aRealURL;
    DateTime                m_aExpireDate;

    NS_UNO::Reference < NS_IO::XInputStream > m_xInputStream;
    CommandThread_Impl*     m_pCommandThread;
    UcbLockBytesHandlerRef  m_xHandler;

    sal_uInt32              m_nRead;
    sal_uInt32              m_nSize;
    ErrCode                 m_nError;

    sal_Bool                m_bTerminated : 1;
    sal_Bool                m_bDontClose : 1;
    sal_Bool                m_bStreamValid : 1;

    DECL_LINK(              DataAvailHdl, void * );

                            UcbLockBytes( UcbLockBytesHandler* pHandler );
protected:
    virtual                 ~UcbLockBytes (void);

public:

    static UcbLockBytesRef  CreateInputLockBytes( const NS_UNO::Reference < NS_UCB::XContent > xContent, UcbLockBytesHandler* pHandler=0 );
    static UcbLockBytesRef  CreateInputLockBytes( const NS_UNO::Reference < NS_IO::XInputStream > xContent, UcbLockBytesHandler* pHandler=0 );

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

    void                    Cancel();

    // the following properties are available when and after the first DataAvailable callback has been executed
    String                  GetContentType() const;
    String                  GetRealURL() const;
    DateTime                GetExpireDate() const;

#if __PRIVATE
    sal_Bool                setInputStream_Impl( const NS_UNO::Reference < NS_IO::XInputStream > &rxInputStream );
    void                    terminate_Impl (void);

    NS_UNO::Reference < NS_IO::XInputStream > getInputStream_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xInputStream;
                            }

    sal_Bool                hasInputStream_Impl() const
                            {
                                vos::OGuard aGuard( SAL_CONST_CAST(UcbLockBytes*, this)->m_aMutex );
                                return m_xInputStream.is();
                            }

    void                    setCommandThread_Impl( CommandThread_Impl* pThread )
                            { m_pCommandThread = pThread; }

    void                    setDontClose_Impl()
                            { m_bDontClose = sal_True; }

    void                    SetContentType_Impl( const String& rType ) { m_aContentType = rType; }
    void                    SetRealURL_Impl( const String& rURL )  { m_aRealURL = rURL; }
    void                    SetExpireDate_Impl( const DateTime& rDateTime )  { m_aExpireDate = rDateTime; }
    void                    SetStreamValid_Impl();
#endif
};

class UcbStreamHelper : public SvStream
{
public:
    static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode, UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
    static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream, UcbLockBytesHandler* pHandler=0 );
};

//----------------------------------------------------------------------------
SV_IMPL_REF( UcbLockBytes );

};

#endif
