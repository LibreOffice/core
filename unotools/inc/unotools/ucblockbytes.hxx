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

class UCB_Link_Helper : public SvRefBase
{
    ::vos::OMutex   maMutex;

    Link            maDoneLink;
    Link            maDataAvailLink;
    Link            maCancelLink;

    BOOL            mbSet;

                    ~UCB_Link_Helper(){;}
public:
                    UCB_Link_Helper()
                    { mbSet = FALSE;}

    void            SetDoneLink( const Link& rLink );
    void            SetDataAvailLink( const Link& rLink );
    void            SetCancelLink( const Link& rLink );

    void            Done();
    void            DataAvail();
    void            Cancel();

    void            Clear();
};

SV_DECL_IMPL_REF( UCB_Link_Helper )

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_UCB ::com::sun::star::ucb

SV_DECL_REF( UcbLockBytes );


class CommandThread_Impl;
class UcbLockBytes : public virtual SvLockBytes
{
    vos::OCondition         m_aInitialized;
    vos::OCondition         m_aTerminated;
    vos::OMutex             m_aMutex;
    NS_UNO::Reference < NS_IO::XInputStream > m_xInputStream;
    CommandThread_Impl*     m_pCommandThread;

    sal_Bool                m_bTerminated;
    sal_Bool                m_bDontClose;

    sal_uInt32              m_nRead;
    sal_uInt32              m_nSize;

    UCB_Link_HelperRef      m_aLinkList;

    DECL_LINK(              DataAvailHdl, void * );

protected:
    virtual                 ~UcbLockBytes (void);

public:

    static UcbLockBytesRef  CreateInputLockBytes( NS_UNO::Reference < NS_UCB::XContent > xContent, UCB_Link_HelperRef xLinkList );

                            UcbLockBytes( UCB_Link_HelperRef xLink )
                                : m_xInputStream (NULL)
                                , m_pCommandThread( NULL )
                                , m_bTerminated  (sal_False)
                                , m_bDontClose( sal_False )
                                , m_nRead (0)
                                , m_nSize (0)
                                , m_aLinkList( xLink )
                            {}

    // SvLockBytes
    virtual void            SetSynchronMode (BOOL bSynchron);
    virtual ErrCode         ReadAt ( ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const;
    virtual ErrCode         WriteAt ( ULONG, const void*, ULONG, ULONG *pWritten);
    virtual ErrCode         Flush (void) const;
    virtual ErrCode         SetSize (ULONG);
    virtual ErrCode         Stat ( SvLockBytesStat *pStat, SvLockBytesStatFlag) const;

    void                    Cancel();

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
#endif
};

//----------------------------------------------------------------------------
SV_IMPL_REF( UcbLockBytes );

#endif
