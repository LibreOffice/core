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

#define _TRANSPRT_CXX "$Revision: 1.6 $"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAMLISTENER_HPP_
#include <com/sun/star/io/XStreamListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_CHAOSPROGRESSSTART_HPP_
#include <com/sun/star/ucb/CHAOSProgressStart.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_DOCUMENTHEADERFIELD_HPP_
#include <com/sun/star/ucb/DocumentHeaderField.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVECHAOSEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveCHAOSException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_POSTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/PostCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#ifndef _EINF_HXX
#include <tools/errinf.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <tools/inetmsg.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _INETTYPE_HXX
#include <bf_svtools/inettype.hxx>
#endif

#ifndef _BINDING_HXX
#include <bf_so3/binding.hxx>
#endif
#ifndef _BINDDATA_HXX
#include <binddata.hxx>
#endif
#ifndef _TRANSPRT_HXX
#include <bf_so3/transprt.hxx>
#endif
#ifndef _TRANSUNO_HXX
#include <transuno.hxx>
#endif
#ifndef _SO2DEFS_HXX
#include <bf_so3/so2defs.hxx>
#endif

#ifndef CONTENT_TYPE_STR_APP_WWWFORM
#define CONTENT_TYPE_STR_APP_WWWFORM "application/x-www-form-urlencoded"
#endif

#ifndef ERRCODE_CHAOS_SERVER_ERROR /* cntids.hrc */
#define ERRCODE_CHAOS_SERVER_ERROR (ERRCODE_AREA_CHAOS + 5)
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace cppu;
using rtl::OUString;

namespace binfilter {

/*========================================================================
 *
 * TODO:
 *
 *  - UcbHTTPTransport_Impl:
 *     - interaction handling (SVBIND_SUCCESSONERRORDOC).
 *
 *======================================================================*/

/*========================================================================
 *
 * SvBindingTransport implementation.
 *
 *======================================================================*/
/*
 * SvBindingTransport.
 */
SvBindingTransport::SvBindingTransport (void)
{
}

/*
 * ~SvBindingTransport.
 */
SvBindingTransport::~SvBindingTransport (void)
{
}

/*========================================================================
 *
 * SvBindingTransportFactory implementation.
 *
 *======================================================================*/
SvBindingTransportFactory::SvBindingTransportFactory (void)
{
    SvBindingTransportFactoryList &rList = BAPP()->m_aTransportFactories;
    rList.Insert (this, rList.Count());
}

SvBindingTransportFactory::~SvBindingTransportFactory (void)
{
    SvBindingTransportFactoryList &rList = BAPP()->m_aTransportFactories;
    rList.Remove (this);
}

SvBindingTransportContext::~SvBindingTransportContext (void)
{
}

/*========================================================================
 *
 * SvLockBytesFactory implementation.
 *
 *======================================================================*/

/*
 * ~SvLockBytesFactory.
 */
SvLockBytesFactory::~SvLockBytesFactory (void)
{
    SvLockBytesFactoryList &rList = BAPP()->m_aLockBytesFactories;
    rList.Remove (this);
}

/*
 * GetFactory.
 */
SvLockBytesFactory* SvLockBytesFactory::GetFactory (const String &rUrl)
{
    SvLockBytesFactoryList &rList = BAPP()->m_aLockBytesFactories;
    SvLockBytesFactory *pFactory = NULL;

    ULONG i, n = rList.Count();
    for (i = 0; i < n; i++)
    {
        pFactory = rList.GetObject(i);
        if (pFactory)
        {
            WildCard aWild (pFactory->GetWildcard());
            if (aWild.Matches(rUrl))
                break;
        }
        pFactory = NULL;
    }
    return pFactory;
}

/*========================================================================
 *
 * SfxSimpleLockBytesFactory implementation.
 *
 *======================================================================*/
USHORT SfxSimpleLockBytesFactory::m_nCounter = 0;


/*
 * ~SfxSimpleLockBytesFactory.
 */
SfxSimpleLockBytesFactory::~SfxSimpleLockBytesFactory (void)
{
}

/*
 * CreateLockBytes.
 */
SvLockBytesRef SfxSimpleLockBytesFactory::CreateLockBytes (
    const String &rUrl, String &rMime)
{
    (void)rUrl;

    rMime = m_aMime;
    return m_xLockBytes;
}

/*========================================================================
 *
 * SvLockBytesTransport interface.
 *
 *======================================================================*/
class SvLockBytesTransport : public SvBindingTransport
{
    /** Representation.
    */
    String                      m_aUrl;
    SvLockBytesFactory         &m_rFactory;
    SvBindingTransportCallback *m_pCallback;

    /** Not implemented.
    */
    COPYCTOR_API(SvLockBytesTransport);

public:
    SvLockBytesTransport (
        const String               &rUrl,
        SvLockBytesFactory         &rFactory,
        SvBindingTransportCallback *pCB);

    virtual ~SvLockBytesTransport (void);

    virtual void Start (void);
    virtual void Abort (void);
};

/*========================================================================
 *
 * SvLockBytesTransport implementation.
 *
 *======================================================================*/
/*
 * SvLockBytesTransport.
 */
SvLockBytesTransport::SvLockBytesTransport (
    const String               &rUrl,
    SvLockBytesFactory         &rFactory,
    SvBindingTransportCallback *pCB)
    : m_aUrl      (rUrl),
      m_rFactory  (rFactory),
      m_pCallback (pCB)
{
}

/*
 * ~SvLockBytesTransport.
 */
SvLockBytesTransport::~SvLockBytesTransport (void)
{
}

/*
 * Start.
 */
void SvLockBytesTransport::Start (void)
{
    DBG_ASSERT(
        m_pCallback, "SvLockBytesTransport::Start(): No callback");
    if (m_pCallback == NULL)
        return;

    String         aMime;
    SvLockBytesRef xLockBytes (m_rFactory.CreateLockBytes (m_aUrl, aMime));
    if (xLockBytes.Is())
    {
        m_pCallback->OnMimeAvailable (aMime);
        m_pCallback->OnDataAvailable (
            SVBSCF_LASTDATANOTIFICATION, 0, xLockBytes);
    }
    else
    {
        m_pCallback->OnError (ERRCODE_IO_ABORT);
    }
}

/*
 * Abort.
 */
void SvLockBytesTransport::Abort (void)
{
}

/*========================================================================
 *
 * SvLockBytesTransportFactory implementation.
 *
 *======================================================================*/
/*
 * SvLockBytesTransportFactory.
 */
SvLockBytesTransportFactory::SvLockBytesTransportFactory (void)
    : SvBindingTransportFactory ()
{
}

/*
 * ~SvLockBytesTransportFactory.
 */
SvLockBytesTransportFactory::~SvLockBytesTransportFactory (void)
{
}

/*
 * HasTransport.
 */
BOOL SvLockBytesTransportFactory::HasTransport (const String &rUrl)
{
    SvLockBytesFactory *pFactory = SvLockBytesFactory::GetFactory (rUrl);
    if (pFactory)
        return TRUE;
    else
        return FALSE;
}

/*
 * CreateTransport.
 */
SvBindingTransport* SvLockBytesTransportFactory::CreateTransport (
    const String               &rUrl,
    SvBindingTransportContext  &rCtx,
    SvBindingTransportCallback *pCB)
{
    (void)rCtx;

    SvLockBytesFactory *pFactory = SvLockBytesFactory::GetFactory (rUrl);
    if (pFactory)
        return new SvLockBytesTransport (rUrl, *pFactory, pCB);
    else
        return NULL;
}

/*========================================================================
 *
 * UcbTransportLockBytes.
 *
 *======================================================================*/
class UcbTransportLockBytes :
    public virtual SvLockBytes,
    public virtual SvRefBase
{
public:
    inline UcbTransportLockBytes (void);

    inline Reference<XInputStream> getInputStream (void);

    inline sal_Bool hasInputStream (void) const;

    inline sal_Bool setInputStream (
        const Reference<XInputStream> &rxInputStream);

    inline void terminate (void);

    /** SvLockBytes.
     */
    virtual void SetSynchronMode (BOOL bSynchron);

    virtual ErrCode ReadAt (
        ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const;

    virtual ErrCode WriteAt (
        ULONG, const void*, ULONG, ULONG *pWritten);

    virtual ErrCode Flush (void) const;

    virtual ErrCode SetSize (ULONG);

    virtual ErrCode Stat (
        SvLockBytesStat *pStat, SvLockBytesStatFlag) const;

protected:
    /** Destruction (SvRefBase).
     */
    virtual ~UcbTransportLockBytes (void);

private:
    /** Representation.
     */
    Reference<XInputStream> m_xInputStream;
    sal_Bool                m_bTerminated;

    sal_uInt32 m_nRead;
    sal_uInt32 m_nSize;

    /** Not implemented.
     */
    COPYCTOR_API(UcbTransportLockBytes);
};

/*
 * UcbTransportLockBytesRef.
 */
SV_DECL_IMPL_REF(UcbTransportLockBytes);

/*
 * UcbTransportLockBytes.
 */
inline UcbTransportLockBytes::UcbTransportLockBytes (void)
    : m_xInputStream (NULL),
      m_bTerminated  (sal_False),
      m_nRead (0),
      m_nSize (0)
{
}

/*
 * UcbTransportLockBytes.
 */
UcbTransportLockBytes::~UcbTransportLockBytes (void)
{
    if (m_xInputStream.is())
        m_xInputStream->closeInput();
}

/*
 * getInputStream.
 */
inline Reference<XInputStream> UcbTransportLockBytes::getInputStream (void)
{
    return m_xInputStream;
}

/*
 * hasInputStream.
 */
inline sal_Bool UcbTransportLockBytes::hasInputStream (void) const
{
    return m_xInputStream.is();
}

/*
 * setInputStream.
 */
inline sal_Bool UcbTransportLockBytes::setInputStream (
    const Reference<XInputStream> &rxInputStream)
{
    if (m_xInputStream.is())
        m_xInputStream->closeInput();

    m_xInputStream = rxInputStream;
    return m_xInputStream.is();
}

/*
 * terminate.
 */
/* inline */ void UcbTransportLockBytes::terminate (void)
{
    Reference<XSeekable> xSeekable (m_xInputStream, UNO_QUERY);
    if (xSeekable.is())
        m_nSize = sal_uInt32(xSeekable->getLength());

    m_bTerminated = sal_True;
}

/*
 * SetSynchronMode.
 */
void UcbTransportLockBytes::SetSynchronMode (BOOL bSynchron)
{
    SvLockBytes::SetSynchronMode (bSynchron);
}

/*
 * ReadAt.
 */
ErrCode UcbTransportLockBytes::ReadAt (
    ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const
{
    if (pRead)
        *pRead = 0;
    if (!m_xInputStream.is())
        return ERRCODE_IO_CANTREAD;

    Reference<XSeekable> xSeekable (m_xInputStream, UNO_QUERY);
    if (!xSeekable.is())
        return ERRCODE_IO_CANTREAD;

    try
    {
        xSeekable->seek (nPos);
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTSEEK;
    }

    Sequence<sal_Int8> aData;
    sal_Int32          nSize;

    nCount = VOS_MIN(nCount, 0x7FFFFFFF);
    try
    {
        while (!m_bTerminated)
        {
            sal_uInt64 nLen = xSeekable->getLength();
            if (IsSynchronMode())
            {
                if (nPos + nCount > nLen)
                    Application::Yield();
                else
                    break;
            }
            else
            {
                if (nPos + nCount > nLen)
                    return ERRCODE_IO_PENDING;
                else
                    break;
            }
        }
        nSize = m_xInputStream->readSomeBytes (aData, sal_Int32(nCount));
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTREAD;
    }

    rtl_copyMemory (pBuffer, aData.getConstArray(), nSize);
    if (pRead)
        *pRead = ULONG(nSize);

    sal_uInt32 nRead = VOS_MAX(m_nRead, nPos + nSize);
    SAL_CONST_CAST(UcbTransportLockBytes*, this)->m_nRead = nRead;

    return ERRCODE_NONE;
}

/*
 * WriteAt.
 */
ErrCode UcbTransportLockBytes::WriteAt (
    ULONG, const void*, ULONG, ULONG *pWritten)
{
    if (pWritten)
        *pWritten = 0;
    return ERRCODE_IO_CANTWRITE;
}

/*
 * Flush.
 */
ErrCode UcbTransportLockBytes::Flush (void) const
{
    return ERRCODE_NONE;
}

/*
 * SetSize.
 */
ErrCode UcbTransportLockBytes::SetSize (ULONG)
{
    return ERRCODE_IO_NOTSUPPORTED;
}

/*
 * Stat.
 */
ErrCode UcbTransportLockBytes::Stat (
    SvLockBytesStat *pStat, SvLockBytesStatFlag) const
{
    if (!pStat)
        return ERRCODE_IO_INVALIDPARAMETER;
    if (!m_xInputStream.is())
        return ERRCODE_IO_INVALIDACCESS;

    Reference<XSeekable> xSeekable (m_xInputStream, UNO_QUERY);
    if (!xSeekable.is())
        return ERRCODE_IO_INVALIDACCESS;

    try
    {
        pStat->nSize = ULONG(xSeekable->getLength());
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTTELL;
    }

    if (!m_bTerminated)
        return ERRCODE_IO_PENDING;
    else
        return ERRCODE_NONE;
}

/*========================================================================
 *
 * UcbTransportDataSink_Impl.
 *
 *======================================================================*/
class UcbTransportDataSink_Impl :
    public OWeakObject,
    public XActiveDataControl,
    public XActiveDataSink
{
public:
    inline UcbTransportDataSink_Impl (void);

    inline SvLockBytes* getLockBytes (void);

    /** XInterface.
     */
    virtual Any SAL_CALL queryInterface (
        const Type &rType) throw(RuntimeException);

    virtual void SAL_CALL acquire (void) throw();

    virtual void SAL_CALL release (void) throw();

    /** XActiveDataControl.
     */
    virtual void SAL_CALL addListener (
        const Reference<XStreamListener> &rxListener)
        throw(RuntimeException);

    virtual void SAL_CALL removeListener (
        const Reference<XStreamListener> &rxListener)
        throw(RuntimeException);

    virtual void SAL_CALL start (void) throw(RuntimeException);

    virtual void SAL_CALL terminate (void) throw(RuntimeException);

    /** XActiveDataSink.
     */
    virtual void SAL_CALL setInputStream (
        const Reference<XInputStream> &rxInputStream)
        throw(RuntimeException);

    virtual Reference<XInputStream> SAL_CALL getInputStream (void)
        throw(RuntimeException);

protected:
    /** Destruction (OWeakObject).
     */
    virtual ~UcbTransportDataSink_Impl (void);

private:
    /** Representation.
     */
    UcbTransportLockBytesRef m_xLockBytes;

    /** Not implemented.
     */
    COPYCTOR_API(UcbTransportDataSink_Impl);
};

/*
 * UcbTransportDataSink_Impl.
 */
inline UcbTransportDataSink_Impl::UcbTransportDataSink_Impl (void)
    : m_xLockBytes (new UcbTransportLockBytes())
{
}

/*
 * ~UcbTransportDataSink_Impl.
 */
UcbTransportDataSink_Impl::~UcbTransportDataSink_Impl (void)
{
}

/*
 * getLockBytes.
 */
inline SvLockBytes* UcbTransportDataSink_Impl::getLockBytes (void)
{
    if (m_xLockBytes->hasInputStream())
        return m_xLockBytes;
    else
        return NULL;
}

/*
 * XInterface: queryInterface.
 */
Any SAL_CALL UcbTransportDataSink_Impl::queryInterface (
    const Type &rType) throw(RuntimeException)
{
    Any aRet (cppu::queryInterface (
        rType,
        // SAL_STATIC_CAST(XActiveDataControl*, this),
        SAL_STATIC_CAST(XActiveDataSink*, this)));
       return aRet.hasValue() ? aRet : OWeakObject::queryInterface (rType);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL UcbTransportDataSink_Impl::acquire (void) throw()
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL UcbTransportDataSink_Impl::release (void) throw()
{
    OWeakObject::release();
}

/*
 * XActiveDataControl: addListener (not supported).
 */
void SAL_CALL UcbTransportDataSink_Impl::addListener (
    const Reference<XStreamListener> & ) throw(RuntimeException)
{
}

/*
 * XActiveDataControl: removeListener (not supported).
 */
void SAL_CALL UcbTransportDataSink_Impl::removeListener (
    const Reference<XStreamListener> & ) throw(RuntimeException)
{
}

/*
 * XActiveDataControl: start (not supported).
 */
void SAL_CALL UcbTransportDataSink_Impl::start (void) throw(RuntimeException)
{
}

/*
 * XActiveDataControl: terminate.
 */
void SAL_CALL UcbTransportDataSink_Impl::terminate (void)
    throw(RuntimeException)
{
    m_xLockBytes->terminate();
}

/*
 * XActiveDataSink: setInputStream.
 */
void SAL_CALL UcbTransportDataSink_Impl::setInputStream (
    const Reference<XInputStream> &rxInputStream) throw(RuntimeException)
{
    m_xLockBytes->setInputStream (rxInputStream);
}

/*
 * XActiveDataSink: getInputStream.
 */
Reference<XInputStream> SAL_CALL
UcbTransportDataSink_Impl::getInputStream (void) throw(RuntimeException)
{
    return m_xLockBytes->getInputStream();
}

/*========================================================================
 *
 * UcbTransportInputStream_Impl.
 *
 *======================================================================*/
class UcbTransportInputStream_Impl :
    public OWeakObject,
    public XInputStream,
    public XSeekable
{
public:
    inline UcbTransportInputStream_Impl (SvLockBytes *pLockBytes);

    /** XInterface.
     */
    virtual Any SAL_CALL queryInterface(
        const Type &rType) throw (RuntimeException);

    virtual void SAL_CALL acquire (void) throw ();

    virtual void SAL_CALL release (void) throw ();

    /** XInputStream.
     */
    virtual sal_Int32 SAL_CALL readBytes (
        Sequence<sal_Int8> &rData, sal_Int32 nBytesToRead)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);

    virtual sal_Int32 SAL_CALL readSomeBytes (
        Sequence<sal_Int8> &rData, sal_Int32 nMaxBytesToRead)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);

    virtual void SAL_CALL skipBytes (sal_Int32 nBytesToSkip)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);

    virtual sal_Int32 SAL_CALL available (void)
        throw (NotConnectedException, IOException);

    virtual void SAL_CALL closeInput (void)
        throw (NotConnectedException, IOException);

    /** XSeekable.
     */
    virtual void SAL_CALL seek (sal_Int64 nPosition)
        throw (IllegalArgumentException, IOException);

    virtual sal_Int64 SAL_CALL getPosition (void) throw (IOException);

    virtual sal_Int64 SAL_CALL getLength (void) throw (IOException);

protected:
    /** Destruction (OWeakObject).
     */
    virtual ~UcbTransportInputStream_Impl (void);

private:
    /** Representation.
     */
    SvLockBytesRef m_xLockBytes;
    sal_uInt32     m_nPosition;

    /** Not implemented.
     */
    COPYCTOR_API(UcbTransportInputStream_Impl);
};

/*
 * UcbTransportInputStream_Impl.
 */
inline UcbTransportInputStream_Impl::UcbTransportInputStream_Impl (
    SvLockBytes *pLockBytes)
    : m_xLockBytes (pLockBytes),
      m_nPosition  (0)
{
}

/*
 * ~UcbTransportInputStream_Impl.
 */
UcbTransportInputStream_Impl::~UcbTransportInputStream_Impl (void)
{
}

/*
 * XInterface: queryInterface.
 */
Any SAL_CALL UcbTransportInputStream_Impl::queryInterface (
    const Type &rType) throw(RuntimeException)
{
    Any aRet (cppu::queryInterface (
        rType,
        SAL_STATIC_CAST (XInputStream*, this),
        SAL_STATIC_CAST (XSeekable*, this)));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface (rType);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL UcbTransportInputStream_Impl::acquire (void)
    throw ()
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL UcbTransportInputStream_Impl::release (void)
    throw ()
{
    OWeakObject::release();
}

/*
 * XInputStream: readBytes.
 */
sal_Int32 SAL_CALL UcbTransportInputStream_Impl::readBytes (
    Sequence<sal_Int8> &rData, sal_Int32 nBytesToRead)
    throw(NotConnectedException, BufferSizeExceededException, IOException)
{
    if (!m_xLockBytes.Is())
        throw NotConnectedException();

    rData.realloc(nBytesToRead);
    sal_Int32 nSize = 0;
    while (nSize < nBytesToRead)
    {
        ULONG nCount = 0;
        ErrCode nError = m_xLockBytes->ReadAt (
            m_nPosition,
            rData.getArray() + nSize,
            nBytesToRead - nSize,
            &nCount);
        if (nError != ERRCODE_NONE && nError != ERRCODE_IO_PENDING)
            throw IOException();
        m_nPosition += nCount;
        nSize += nCount;
        if (nError == ERRCODE_NONE && nCount == 0)
            break;
    }

    rData.realloc(nSize);
    return nSize;
}

/*
 * XInputStream: readSomeBytes.
 */
sal_Int32 SAL_CALL UcbTransportInputStream_Impl::readSomeBytes (
    Sequence<sal_Int8> &rData, sal_Int32 nMaxBytesToRead)
    throw(NotConnectedException, BufferSizeExceededException, IOException)
{
    if (!m_xLockBytes.Is())
        throw NotConnectedException();

    rData.realloc(nMaxBytesToRead);
    ULONG nCount;
    ErrCode nError;
    do
    {
        nError = m_xLockBytes->ReadAt (
            m_nPosition,
            rData.getArray(),
            nMaxBytesToRead < 0 ? 0 : nMaxBytesToRead,
            &nCount);
        if (nError != ERRCODE_NONE && nError != ERRCODE_IO_PENDING)
            throw IOException();
        m_nPosition += nCount;
    }
    while (nCount == 0 && nError == ERRCODE_IO_PENDING);

    rData.realloc(sal_Int32(nCount));
    return sal_Int32(nCount);
}

/*
 * XInputStream: skipBytes.
 */
void SAL_CALL UcbTransportInputStream_Impl::skipBytes (sal_Int32 nBytesToSkip)
    throw(NotConnectedException, BufferSizeExceededException, IOException)
{
    if (!m_xLockBytes.Is())
        throw NotConnectedException();
    if (nBytesToSkip < 0)
        throw IOException();
    if (m_nPosition > 0xFFFFFFFF - nBytesToSkip)
        throw BufferSizeExceededException();
    m_nPosition += nBytesToSkip;
}

/*
 * XInputStream: available.
 */
sal_Int32 SAL_CALL UcbTransportInputStream_Impl::available (void)
    throw(NotConnectedException, IOException)
{
    if (!m_xLockBytes.Is())
        throw NotConnectedException();

    SvLockBytesStat aStat;
    if (m_xLockBytes->Stat(&aStat, SVSTATFLAG_DEFAULT) != ERRCODE_NONE)
        throw IOException();

    return aStat.nSize <= m_nPosition ? 0 :
        aStat.nSize - m_nPosition < 0x7FFFFFFF ?
        sal_Int32(aStat.nSize - m_nPosition) : 0x7FFFFFFF;
}

/*
 * XInputStream: closeInput.
 */
void SAL_CALL UcbTransportInputStream_Impl::closeInput (void)
    throw(NotConnectedException, IOException)
{
    if (!m_xLockBytes.Is())
        throw NotConnectedException();
    m_xLockBytes = 0;
}

/*
 * XSeekable: seek.
 */
void SAL_CALL UcbTransportInputStream_Impl::seek (sal_Int64 nPosition)
    throw (IllegalArgumentException, IOException)
{
    if (nPosition < 0)
        throw IllegalArgumentException();
    if (nPosition > 0x7FFFFFFF)
        throw IOException();
    if (!m_xLockBytes.Is())
        throw IOException();
    m_nPosition = sal::static_int_cast<sal_uInt32>( nPosition );
}

/*
 * XSeekable: getPosition.
 */
sal_Int64 SAL_CALL UcbTransportInputStream_Impl::getPosition (void)
    throw (IOException)
{
    if (!m_xLockBytes.Is())
        throw IOException();
    return m_nPosition;
}

/*
 * XSeekable: getLength.
 */
sal_Int64 SAL_CALL UcbTransportInputStream_Impl::getLength (void)
    throw (IOException)
{
    if (!m_xLockBytes.Is())
        throw IOException();

    SvLockBytesStat aStat;
    if (m_xLockBytes->Stat (&aStat, SVSTATFLAG_DEFAULT) != ERRCODE_NONE)
        throw IOException();

    return aStat.nSize;
}

/*========================================================================
 *
 * UcbTransport_Impl.
 *
 *======================================================================*/

class TransportThread_Impl : public ::vos::OThread
{
    Link            m_aLink;
public:
                    TransportThread_Impl( const Link& rLink )
                        : m_aLink( rLink )
                    {}

    virtual void SAL_CALL 	run();
    void SAL_CALL 			onTerminated();
};

void TransportThread_Impl::run()
{
    if( schedule() && m_aLink.IsSet() )
        m_aLink.Call(0);
}

void TransportThread_Impl::onTerminated()
{
    delete this;
}

class UcbTransport_Impl :
    public OWeakObject,
    public XCommandEnvironment,
    public XInteractionHandler,
    public XProgressHandler,
    public XPropertiesChangeListener
{
public:
    inline UcbTransport_Impl (
        const String               &rUrl,
        SvBindingTransportContext  &rCtx,
        SvBindingTransportCallback *pCB);

    virtual void start (void);
    virtual void abort (void);

    /** XInterface.
     */
    virtual Any SAL_CALL queryInterface ( const Type &rType) throw(RuntimeException);
    virtual void SAL_CALL acquire (void) throw();
    virtual void SAL_CALL release (void) throw();

    virtual Reference<XInteractionHandler> SAL_CALL getInteractionHandler (void) throw(RuntimeException);
    virtual Reference<XProgressHandler> SAL_CALL getProgressHandler (void) throw(RuntimeException);

    virtual void SAL_CALL disposing ( const EventObject &rEvent) throw(RuntimeException);

    /** XInteractionHandler.
     */
    virtual void SAL_CALL handle ( const Reference<XInteractionRequest> &rxRequest) throw(RuntimeException);

    /** XProgressHandler.
     */
    virtual void SAL_CALL push ( const Any &rStatus) throw(RuntimeException);
    virtual void SAL_CALL update ( const Any &rStatus) throw(RuntimeException);
    virtual void SAL_CALL pop (void) throw(RuntimeException);

    /** XPropertiesChangeListener.
     */
    virtual void SAL_CALL propertiesChange ( const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException);

protected:
    /** Representation.
     */
    Command                              m_aCommand;
    String                               m_aUrl;
    SvBindingTransportContext           &m_rCtx;
    SvBindingTransportCallback          *m_pCallback;
    vos::ORef<UcbTransportDataSink_Impl> m_xSink;
    SvLockBytesRef                       m_xLockBytes;
    Reference<XContent>                  m_xContent;
    OUString                             m_aContentType;
    sal_Int32                            m_nCommandId;
    sal_Bool                             m_bMimeAvail : 1;

    /** Destruction (OWeakObject).
     */
    virtual ~UcbTransport_Impl (void);

    /** dispose_Impl.
     */
    void dispose_Impl (void);

    /** getCallback_Impl.
     */
    inline sal_Bool getCallback_Impl (
        SvBindingTransportCallback *&rpCallback);

    /** getMutex_Impl.
     */
    inline vos::IMutex& getMutex_Impl (void);

    /** createContent_Impl.
     */
    inline Reference<XContent> createContent_Impl (
        const OUString &rUrl);

    /** getContentType_Impl.
     */
    static OUString getContentType_Impl ( const Reference<XCommandProcessor> &rxProcessor);
    DECL_LINK(                          ExecuteCallback, void* );

private:
    /** Representation.
     */
    vos::OMutex                         m_aMutex;
    Reference<XInteractionHandler>      m_xInteractionHdl;
    oslInterlockedCount                 m_nProgressDepth;
    sal_Int32                           m_nProgressMin;
    sal_Int32                           m_nProgressMax;

    /** Not implemented.
     */
    COPYCTOR_API(UcbTransport_Impl);
};

/*
 * getCallback_Impl.
 */
inline sal_Bool UcbTransport_Impl::getCallback_Impl ( SvBindingTransportCallback *&rpCallback)
{
    vos::OGuard aGuard (m_aMutex);
    rpCallback = m_pCallback;
    return (!!rpCallback);
}

/*
 * getMutex_Impl.
 */
inline vos::IMutex& UcbTransport_Impl::getMutex_Impl (void)
{
    return m_aMutex;
}

/*
 * createContent_Impl.
 */
inline Reference<XContent>
UcbTransport_Impl::createContent_Impl (const OUString &rUrl)
{
    return SvBindingTransport_Impl::createContent (rUrl);
}

/*
 * UcbTransport_Impl.
 */
inline UcbTransport_Impl::UcbTransport_Impl (
    const String               &rUrl,
    SvBindingTransportContext  &rCtx,
    SvBindingTransportCallback *pCB)
    : m_aUrl       (rUrl),
      m_rCtx       (rCtx),
      m_pCallback  (pCB),
      m_nCommandId     (0),
      m_bMimeAvail (sal_False),
      m_nProgressDepth (0),
      m_nProgressMin   (0),
      m_nProgressMax   (0)
{
}

/*
 * ~UcbTransport_Impl.
 */
UcbTransport_Impl::~UcbTransport_Impl (void)
{
}

IMPL_LINK( UcbTransport_Impl, ExecuteCallback, void*, pVoid )
{
    (void)pVoid;

    // protect against destruction in dispose_impl
    Reference < XCommandEnvironment > xEnv( this );

    Reference<XCommandProcessor> xProcessor( m_xContent, UNO_QUERY );
    if ( xProcessor.is() && m_nCommandId )
    {
        SvBindingTransportCallback *pCB=NULL;
        if( getCallback_Impl( pCB ) )
            // notify that transport will start now
            pCB->OnStart();

        if ( m_bMimeAvail && getCallback_Impl(pCB) )
            // notify contenttype if available
            pCB->OnMimeAvailable( m_aContentType );

        Any aResult;
        bool bException = false;
        bool bAborted = false;

        try
        {
            // do it
            aResult = xProcessor->execute( m_aCommand, m_nCommandId, xEnv );
        }
        catch ( CommandAbortedException )
        {
            bAborted = true;
        }
        catch ( Exception )
        {
            bException = true;
        }

        m_nCommandId = 0;
        if ( bAborted || bException  )
        {
            // download aborted
            if ( m_xSink.isValid() )
            {
                m_xSink->terminate();
                m_xSink.unbind();
            }

            if ( getCallback_Impl(pCB) )
                pCB->OnError( ERRCODE_ABORT );
        }
        else
        {
            if ( !m_bMimeAvail )
            {
                // contenttype was not detected until now, so get it
                m_aContentType = getContentType_Impl(xProcessor);
            }

            if ( m_xSink.isValid() )
            {
                m_xSink->terminate();
                if( !m_xLockBytes.Is() )
                    m_xLockBytes = m_xSink->getLockBytes();
                m_xSink.unbind();
            }

            if ( !m_bMimeAvail )
            {
                // contenttype was not notified until now
                m_bMimeAvail = sal_True;
                if( getCallback_Impl(pCB) )
                    pCB->OnMimeAvailable( m_aContentType );
            }

            if( getCallback_Impl(pCB) )
            {
                // last data notification.
                pCB->OnDataAvailable( SVBSCF_LASTDATANOTIFICATION, m_nProgressMax, m_xLockBytes );
            }
        }
    }

    dispose_Impl();
    return 0;
}

/*
 * start.
 */
void UcbTransport_Impl::start (void)
{
    // Check context.
    DBG_ASSERT( m_pCallback, "UcbTransport_Impl::start(): No callback");
    if (m_pCallback == NULL)
        return;

    // Create content.
    m_xContent = createContent_Impl (m_aUrl);
    if (!m_xContent.is())
    {
        m_pCallback->OnError (ERRCODE_IO_GENERAL);
        return;
    }

    Reference<XCommandProcessor> xProcessor (m_xContent, UNO_QUERY);
    if (!xProcessor.is())
    {
        m_pCallback->OnError (ERRCODE_IO_GENERAL);
        return;
    }

    // Check BindAction.
    SvBindAction eAction = m_rCtx.GetBindAction();
    if( eAction == BINDACTION_GET )
    {
        Reference<XPropertiesChangeNotifier> xProps (m_xContent, UNO_QUERY);
        if (xProps.is())
        {
            m_aContentType = getContentType_Impl (xProcessor);
            if (m_aContentType.getLength() == 0)
                m_aContentType = OUString::createFromAscii(
                                     CONTENT_TYPE_STR_APP_OCTSTREAM);
            m_bMimeAvail = sal_True;
            xProps->addPropertiesChangeListener (Sequence<OUString>(), this);
        }

        OpenCommandArgument2 aArgument;
        aArgument.Mode = OpenMode::DOCUMENT;
        aArgument.Priority = m_rCtx.GetPriority();

        m_xSink = new UcbTransportDataSink_Impl();
        aArgument.Sink = SAL_STATIC_CAST(OWeakObject*, m_xSink.getBodyPtr());

        if (m_rCtx.GetBindMode() & SVBIND_NEWESTVERSION)
            m_aCommand.Name = OUString::createFromAscii ("synchronize");
        else
            m_aCommand.Name = OUString::createFromAscii ("open");
        m_aCommand.Handle = -1;
        m_aCommand.Argument <<= aArgument;

        m_nCommandId = xProcessor->createCommandIdentifier();
        TransportThread_Impl *pThread = new TransportThread_Impl( LINK( this, UcbTransport_Impl, ExecuteCallback ) );
        pThread->create();
    }
    else if( eAction == BINDACTION_PUT )
    {
        SvLockBytesRef xLockBytes (m_rCtx.GetPostLockBytes());
        if (!xLockBytes.Is())
        {
            m_pCallback->OnError (ERRCODE_IO_INVALIDPARAMETER);
            return;
        }

        m_aCommand.Name = OUString::createFromAscii ("insert");
        m_aCommand.Handle = -1;
        m_aCommand.Argument <<= InsertCommandArgument( Reference<XInputStream> ( new UcbTransportInputStream_Impl(xLockBytes) ),
                                                       sal_False );

        m_nCommandId = xProcessor->createCommandIdentifier();
        TransportThread_Impl *pThread = new TransportThread_Impl( LINK( this, UcbTransport_Impl, ExecuteCallback ) );
        pThread->create();
    }
    else
    {
        // Unsupported.
        m_pCallback->OnError (ERRCODE_IO_NOTSUPPORTED);
    }
}

/*
 * abort.
 */
void UcbTransport_Impl::abort (void)
{
    m_aMutex.acquire();
    m_pCallback = NULL;
    m_aMutex.release();

    if ( m_nCommandId )
    {
        Reference<XCommandProcessor> xProcessor( m_xContent, UNO_QUERY );
        xProcessor->abort( m_nCommandId );
        m_nCommandId = 0;
    }
}

/*
 * XInterface: queryInterface.
 */
Any SAL_CALL UcbTransport_Impl::queryInterface ( const Type &rType) throw(RuntimeException )
{
    Any aRet (cppu::queryInterface (
        rType,
        SAL_STATIC_CAST(XCommandEnvironment*, this),
        SAL_STATIC_CAST(XInteractionHandler*, this),
        SAL_STATIC_CAST(XProgressHandler*, this),
        SAL_STATIC_CAST(XPropertiesChangeListener*, this)));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface (rType);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL UcbTransport_Impl::acquire (void) throw()
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL UcbTransport_Impl::release (void) throw()
{
    OWeakObject::release();
}

Reference<XInteractionHandler> SAL_CALL UcbTransport_Impl::getInteractionHandler (void) throw(RuntimeException)
{
    return this;
}

Reference<XProgressHandler> SAL_CALL UcbTransport_Impl::getProgressHandler (void) throw(RuntimeException)
{
    return this;
}

/*
 * XEventListener: disposing.
 */
void SAL_CALL UcbTransport_Impl::disposing ( const EventObject & ) throw(RuntimeException)
{
    vos::OGuard aGuard (m_aMutex);
    // m_xTask = NULL;
}

/*
 * XInteractionHandler: handle.
 */
void SAL_CALL UcbTransport_Impl::handle ( const Reference<XInteractionRequest> &rxRequest) throw(RuntimeException)
{
    if (!m_xInteractionHdl.is())
    {
        Reference< XMultiServiceFactory >
            xFactory(SvBindingTransport_Impl::getProcessServiceFactory(),
                     UNO_QUERY);
        if (xFactory.is())
            m_xInteractionHdl
                = Reference< XInteractionHandler >(
                      xFactory->
                          createInstance(
                              OUString::createFromAscii(
                                  "com.sun.star.task.InteractionHandler")),
                      UNO_QUERY);
    }
    DBG_ASSERT(m_xInteractionHdl.is(),
               "UcbTransport_Impl::handle(): No handler");
    if (m_xInteractionHdl.is())
        m_xInteractionHdl->handle (rxRequest);
}

/*
 * XProgressHandler: push.
 */
void SAL_CALL UcbTransport_Impl::push ( const Any &rStatus) throw(RuntimeException)
{
    if (osl_incrementInterlockedCount (&m_nProgressDepth) == 1)
    {
        CHAOSProgressStart aStart;
        if (rStatus >>= aStart)
        {
            m_nProgressMin = aStart.Minimum;
            m_nProgressMax = aStart.Maximum;
        }

        SvBindingTransportCallback *pCB = NULL;
        if (getCallback_Impl (pCB))
        {
            pCB->OnProgress ( m_nProgressMin, m_nProgressMax, SVBINDSTATUS_DOWNLOADINGDATA);
        }
        if (!m_xLockBytes.Is())
        {
            if (m_xSink.isValid())
                m_xLockBytes = m_xSink->getLockBytes();
        }
        if (m_bMimeAvail && m_xLockBytes.Is() && getCallback_Impl (pCB))
        {
            // First data notification.
            pCB->OnDataAvailable ( SVBSCF_FIRSTDATANOTIFICATION, m_nProgressMin, m_xLockBytes);
        }
    }
}

/*
 * XProgressHandler: update.
 */
void SAL_CALL UcbTransport_Impl::update ( const Any &rStatus) throw(RuntimeException)
{
    if (m_nProgressDepth == 1)
    {
        sal_Int32 nProgress = 0;
        if ((rStatus >>= nProgress) && (nProgress > 0))
        {
            SvBindingTransportCallback *pCB = NULL;
            if (getCallback_Impl (pCB))
            {
                pCB->OnProgress ( nProgress, m_nProgressMax, SVBINDSTATUS_DOWNLOADINGDATA);
            }
            if (!m_xLockBytes.Is())
            {
                if (m_xSink.isValid())
                    m_xLockBytes = m_xSink->getLockBytes();
            }
            if (m_bMimeAvail && m_xLockBytes.Is() && getCallback_Impl (pCB))
            {
                // Intermediate data notification.
                pCB->OnDataAvailable ( SVBSCF_INTERMEDIATEDATANOTIFICATION, nProgress, m_xLockBytes);
            }
        }
    }
}

/*
 * XProgressHandler: pop.
 */
void SAL_CALL UcbTransport_Impl::pop (void) throw(RuntimeException)
{
    if ( osl_decrementInterlockedCount (&m_nProgressDepth) == 0 )
    {
        SvBindingTransportCallback *pCB = NULL;
        if (getCallback_Impl (pCB))
        {
            pCB->OnProgress ( m_nProgressMax, m_nProgressMax, SVBINDSTATUS_ENDDOWNLOADDATA);
        }
    }
}

/*
 * XPropertiesChangeListener: propertiesChange.
 */
void SAL_CALL UcbTransport_Impl::propertiesChange ( const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException)
{
    SvBindingTransportCallback *pCB = NULL;
    sal_Int32 i, n = rEvent.getLength();
    for (i = 0; i < n; i++)
    {
        PropertyChangeEvent evt (rEvent[i]);
        if (evt.PropertyName == OUString::createFromAscii ("ContentType"))
        {
            if (evt.NewValue >>= m_aContentType)
            {
                if (!m_bMimeAvail)
                {
                    m_bMimeAvail = sal_True;
                    if (getCallback_Impl (pCB))
                        pCB->OnMimeAvailable (m_aContentType);
                }
            }
            continue;
        }
        if (evt.PropertyName == OUString::createFromAscii ("DocumentBody"))
        {
            if (m_xSink.isValid())
                m_xLockBytes = m_xSink->getLockBytes();
            continue;
        }
    }
}

/*
 * dispose_Impl.
 */
void UcbTransport_Impl::dispose_Impl (void)
{
    vos::OGuard aGuard (m_aMutex);

    Reference<XPropertiesChangeNotifier> xProps (m_xContent, UNO_QUERY);
    if (xProps.is())
        xProps->removePropertiesChangeListener (Sequence<OUString>(), this);
    m_xContent = NULL;
    m_nProgressDepth = 0;
}

/*
 * getContentType_Impl.
 */
OUString UcbTransport_Impl::getContentType_Impl ( const Reference<XCommandProcessor> &rxProcessor)
{
    OUString aResult;
    if (rxProcessor.is())
    {
        OUString aName (OUString::createFromAscii ("ContentType"));
        Sequence<Property> aProps(1);

        aProps[0].Name = aName;
        aProps[0].Handle = -1;

        Any aAny = SvBindingTransport_Impl::getProperties (rxProcessor, aProps);
        Reference< XRow > xValues;
        if ( aAny >>= xValues )
        {
            Any aValue = xValues->getObject( 1, Reference< XNameAccess>() );
            aValue >>= aResult;
        }
    }

    return aResult;
}

/*========================================================================
 *
 * UcbHTTPTransport_Impl.
 *
 *======================================================================*/
class UcbHTTPTransport_Impl : public UcbTransport_Impl
{
public:
    inline UcbHTTPTransport_Impl (
        const String               &rUrl,
        SvBindingTransportContext  &rCtx,
        SvBindingTransportCallback *pCB);

    /** UcbTransport_Impl.
     */
    virtual void start (void);

    /** XInteractionHandler.
     */
    virtual void SAL_CALL handle (
        const Reference<XInteractionRequest> &rxRequest)
        throw(RuntimeException);

    /** XPropertiesChangeListener.
     */
    virtual void SAL_CALL propertiesChange (
        const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException);

protected:
    /** Destruction (OWeakObject).
     */
    virtual ~UcbHTTPTransport_Impl (void);

private:
    /** Representation.
     */
    sal_Bool m_bNeedHead : 1;
    sal_Bool m_bNeedMime : 1;

    /** analyzeHeader_Impl.
     */
    void analyzeHeader_Impl (
        const Sequence<DocumentHeaderField> &rHdr);

   /** handleError_Impl.
    */
   void handleError_Impl (
       const InteractiveCHAOSException &rException);

    /** Not implemented.
     */
    COPYCTOR_API(UcbHTTPTransport_Impl);
};

/*
 * UcbHTTPTransport_Impl.
 */
inline UcbHTTPTransport_Impl::UcbHTTPTransport_Impl (
    const String               &rUrl,
    SvBindingTransportContext  &rCtx,
    SvBindingTransportCallback *pCB)
    : UcbTransport_Impl (rUrl, rCtx, pCB),
      m_bNeedHead       (sal_True),
      m_bNeedMime       (sal_True)
{
}

/*
 * ~UcbHTTPTransport_Impl.
 */
UcbHTTPTransport_Impl::~UcbHTTPTransport_Impl (void)
{
}

/*
 * UcbTransport_Impl: start.
 */
void UcbHTTPTransport_Impl::start (void)
{
    // Check context.
    DBG_ASSERT( m_pCallback, "UcbHTTPTransport_Impl::start(): No callback");
    if (m_pCallback == NULL)
        return;

    // Check HTTP initialization.
    if (!BAPP()->HasHttpCache())
    {
        m_pCallback->OnError (ERRCODE_IO_NOTSUPPORTED);
        return;
    }

    // Setup content URL.
    String aOwnUrl (String::CreateFromAscii (
        RTL_CONSTASCII_STRINGPARAM ("private:httpcache#")));
    aOwnUrl += m_aUrl;

    // Create content.
    m_xContent = createContent_Impl ( aOwnUrl );
    if (!m_xContent.is())
    {
        m_pCallback->OnError (ERRCODE_IO_GENERAL);
        return;
    }

    Reference<XCommandProcessor> xProcessor (m_xContent, UNO_QUERY);
    if (!xProcessor.is())
    {
        m_pCallback->OnError (ERRCODE_IO_GENERAL);
        return;
    }

    Sequence<PropertyValue> aProps;

    // Check Referer URL.
    String aUrl (m_rCtx.GetReferer());
    INetProtocol eProto = INetURLObject::CompareProtocolScheme (aUrl);
    if ((eProto == INET_PROT_HTTPS) ||
        (eProto == INET_PROT_HTTP ) ||
        (eProto == INET_PROT_FTP  )    )
    {
        sal_Int32 n = aProps.getLength();
        aProps.realloc (n + 1);

        aProps[n].Name = OUString::createFromAscii ("Referer");
        aProps[n].Handle = -1;
        aProps[n].Value <<= OUString(aUrl);
    }

    // Check BindAction.
    SvBindAction eAction = m_rCtx.GetBindAction();
    if (eAction == BINDACTION_GET)
    {
        Reference<XPropertiesChangeNotifier> xProps (m_xContent, UNO_QUERY);
        if (xProps.is())
        {
            SvBindingTransport_Impl::setProperties (xProcessor, aProps);
            xProps->addPropertiesChangeListener (Sequence<OUString>(), this);
        }

        m_xSink = new UcbTransportDataSink_Impl();

        if (m_rCtx.GetBindMode() & SVBIND_NEWESTVERSION)
            m_aCommand.Name = OUString::createFromAscii ("synchronize");
        else
            m_aCommand.Name = OUString::createFromAscii ("open");
        m_aCommand.Handle = -1;

        OpenCommandArgument2 aArgument;
        aArgument.Mode = OpenMode::DOCUMENT;
        aArgument.Priority = m_rCtx.GetPriority();
        aArgument.Sink = SAL_STATIC_CAST(OWeakObject*, m_xSink.getBodyPtr());
        m_aCommand.Argument <<= aArgument;

        m_nCommandId = xProcessor->createCommandIdentifier();
        TransportThread_Impl *pThread = new TransportThread_Impl( LINK( this, UcbTransport_Impl, ExecuteCallback ) );
        pThread->create();
    }
    else if ((eAction == BINDACTION_PUT) || (eAction == BINDACTION_POST))
    {
        SvLockBytesRef xLockBytes (m_rCtx.GetPostLockBytes());
        if (!xLockBytes.Is())
        {
            m_pCallback->OnError (ERRCODE_IO_INVALIDPARAMETER);
            return;
        }

        Reference<XPropertiesChangeNotifier > xProps (m_xContent, UNO_QUERY);
        if (xProps.is())
        {
            String aMimeType (m_rCtx.GetSendMimeType());
            if (!aMimeType.Len())
            {
                if (eAction == BINDACTION_POST)
                    aMimeType.AppendAscii (CONTENT_TYPE_STR_APP_WWWFORM);
                else
                    aMimeType.AppendAscii (CONTENT_TYPE_STR_APP_OCTSTREAM);
            }

            sal_Int32 n = aProps.getLength();
            aProps.realloc (n + 1);
            aProps[n].Name = OUString::createFromAscii ("ContentType");
            aProps[n].Value <<= OUString(aMimeType);

            SvBindingTransport_Impl::setProperties (xProcessor, aProps);
            xProps->addPropertiesChangeListener (Sequence<OUString>(), this);
        }

        if (eAction == BINDACTION_PUT)
        {
            m_aCommand.Name = OUString::createFromAscii ("insert");
            m_aCommand.Handle = -1;
            m_aCommand.Argument <<= InsertCommandArgument( Reference<XInputStream> ( new UcbTransportInputStream_Impl(xLockBytes)),
                                                           sal_False );
        }
        else
        {
            m_xSink = new UcbTransportDataSink_Impl();

            m_aCommand.Name = OUString::createFromAscii ("post");
            m_aCommand.Handle = -1;

            PostCommandArgument aArgument;
            aArgument.Source = new UcbTransportInputStream_Impl(xLockBytes);
            aArgument.Sink = static_cast< OWeakObject* >(m_xSink.getBodyPtr());
            m_aCommand.Argument <<= aArgument;
        }

        m_nCommandId = xProcessor->createCommandIdentifier();
        TransportThread_Impl *pThread = new TransportThread_Impl( LINK( this, UcbTransport_Impl, ExecuteCallback ) );
        pThread->create();
    }
    else
    {
        // Unsupported.
        m_pCallback->OnError (ERRCODE_IO_NOTSUPPORTED);
    }
}

/*
 * XInteractionHandler: handle.
 */
void SAL_CALL UcbHTTPTransport_Impl::handle ( const Reference<XInteractionRequest> &rxRequest) throw(RuntimeException)
{
     if (rxRequest.is())
    {
        Any aRequest (rxRequest->getRequest());
        InteractiveCHAOSException aException;
        if (aRequest >>= aException)
        {
            if (aException.ID == ERRCODE_CHAOS_SERVER_ERROR)
            {
                typedef Reference<XInteractionContinuation> continue_type;
                Sequence<continue_type>
                    aContinuations (rxRequest->getContinuations());

                sal_Int32 i, n = aContinuations.getLength();
                for (i = 0; i < n; i++)
                {
                    if (m_rCtx.GetBindMode() & SVBIND_SUCCESSONERRORDOC)
                    {
                        // Continue (and deliver error document).
                        Reference<XInteractionApprove>
                            xApprove (aContinuations[i], UNO_QUERY);
                        if (xApprove.is())
                        {
                            xApprove->select();
                            return;
                        }

                    }
                    else
                    {
                        // Abort (and report error).
                        Reference<XInteractionAbort>
                            xAbort (aContinuations[i], UNO_QUERY);
                        if (xAbort.is())
                        {
                            handleError_Impl (aException);
                            xAbort->select();
                            return;
                        }
                    }
                }
            }
        }
    }

    UcbTransport_Impl::handle (rxRequest);
}

/*
 * XPropertiesChangeListener: propertiesChange.
 */
void SAL_CALL UcbHTTPTransport_Impl::propertiesChange ( const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException)
{
    SvBindingTransportCallback *pCB = NULL;
    sal_Int32 i, n = rEvent.getLength();
    for (i = 0; i < n; i++)
    {
        PropertyChangeEvent evt (rEvent[i]);
        if (evt.PropertyName == OUString::createFromAscii ("DocumentHeader"))
        {
            Sequence<DocumentHeaderField> aHead;
            if (evt.NewValue >>= aHead)
            {
                if (m_bNeedHead)
                {
                    m_bNeedHead = sal_False;
                    analyzeHeader_Impl (aHead);
                }

                if (!m_bNeedMime && !m_bMimeAvail)
                {
                    m_bMimeAvail = sal_True;
                    if (getCallback_Impl (pCB))
                        pCB->OnMimeAvailable (m_aContentType);
                }
            }
            continue;
        }
        if (evt.PropertyName == OUString::createFromAscii ("PresentationURL"))
        {
            OUString aUrl;
            if (evt.NewValue >>= aUrl)
            {
                OUString aBad (OUString::createFromAscii ("private:"));
                if (!(aUrl.compareTo (aBad, aBad.getLength()) == 0))
                {
                    // URL changed (Redirection).
                    m_aUrl = String( aUrl );

                    if (getCallback_Impl (pCB))
                        pCB->OnRedirect (m_aUrl);
                }
            }
            continue;
        }
    }

    UcbTransport_Impl::propertiesChange (rEvent);
}

/*
 * analyzeHeader_Impl.
 */
void UcbHTTPTransport_Impl::analyzeHeader_Impl (
    const Sequence<DocumentHeaderField> &rHdr)
{
    SvBindingTransportCallback *pCB = NULL;
    sal_Int32 k, m = rHdr.getLength();
    for (k = 0; k < m; k++)
    {
        String aName( rHdr[k].Name );
        String aValue( rHdr[k].Value );

        if (getCallback_Impl (pCB))
            pCB->OnHeaderAvailable (aName, aValue);

        if (aName.CompareIgnoreCaseToAscii("Content-Type") == COMPARE_EQUAL)
        {
            vos::OGuard aGuard (getMutex_Impl());

            m_aContentType = aValue;
            m_bNeedMime = sal_False;

            continue;
        }
        if (aName.CompareIgnoreCaseToAscii("Expires") == COMPARE_EQUAL)
        {
            DateTime aExpires (0, 0);
            if (INetRFC822Message::ParseDateField (aValue, aExpires))
            {
                aExpires.ConvertToLocalTime();
                if (getCallback_Impl (pCB))
                    pCB->OnExpiresAvailable (aExpires);
            }
            continue;
        }
    }
}

/*
 * handleError_Impl.
 */
void UcbHTTPTransport_Impl::handleError_Impl (
    const InteractiveCHAOSException &rException)
{
    sal_uInt32 eErrCode = rException.ID;

    if (rException.Arguments.getLength() >= 2)
        eErrCode = *new TwoStringErrorInfo (
            eErrCode,
            String (rException.Arguments[0].getStr()),
            String (rException.Arguments[1].getStr()));

    if (rException.Arguments.getLength() == 1)
        eErrCode = *new StringErrorInfo (
            eErrCode,
            String (rException.Arguments[0].getStr()));

    ErrorHandler::HandleError (eErrCode);
}

/*========================================================================
 *
 * UcbTransport.
 *
 *======================================================================*/
class UcbTransport : public SvBindingTransport
{
public:
    UcbTransport (const vos::ORef<UcbTransport_Impl> &rxImpl);
    virtual ~UcbTransport (void);

    /** SvBindingTransport.
     */
    virtual void Start (void);
    virtual void Abort (void);

private:
    /** Representation.
     */
    vos::ORef<UcbTransport_Impl> m_xImpl;

    /** Not implemented.
     */
    COPYCTOR_API(UcbTransport);
};

/*
 * UcbTransport.
 */
UcbTransport::UcbTransport (const vos::ORef<UcbTransport_Impl> &rxImpl)
    : m_xImpl (rxImpl)
{
}

/*
 * ~UcbTransport.
 */
UcbTransport::~UcbTransport (void)
{
}

/*
 * Start.
 */
void UcbTransport::Start (void)
{
    if (m_xImpl.isValid())
        m_xImpl->start();
}

/*
 * Abort.
 */
void UcbTransport::Abort (void)
{
    if (m_xImpl.isValid())
        m_xImpl->abort();
}

/*========================================================================
 *
 * CntTransportFactory implementation.
 *
 *======================================================================*/
/*
 * CntTransportFactory.
 */
CntTransportFactory::CntTransportFactory (void)
    : SvBindingTransportFactory ()
{
}

/*
 * ~CntTransportFactory.
 */
CntTransportFactory::~CntTransportFactory (void)
{
}

/*
 * HasTransport.
 */
BOOL CntTransportFactory::HasTransport (const String &rUrl)
{
    INetProtocol eProto = INetURLObject::CompareProtocolScheme (rUrl);
    return ((eProto == INET_PROT_HTTPS) ||
            (eProto == INET_PROT_HTTP ) ||
            (eProto == INET_PROT_FILE ) ||
            (eProto == INET_PROT_FTP  )    );
}

/*
 * CreateTransport.
 */
SvBindingTransport* CntTransportFactory::CreateTransport (
    const String               &rUrl,
    SvBindingTransportContext  &rCtx,
    SvBindingTransportCallback *pCB)
{
    INetProtocol eProto = INetURLObject::CompareProtocolScheme (rUrl);
    if ((eProto == INET_PROT_HTTPS) ||
        (eProto == INET_PROT_HTTP ) ||
        (eProto == INET_PROT_FILE ) ||
        (eProto == INET_PROT_FTP  )    )
    {
        vos::ORef<UcbTransport_Impl> xImpl;
        if ((eProto == INET_PROT_HTTPS) || (eProto == INET_PROT_HTTP))
            xImpl = new UcbHTTPTransport_Impl (rUrl, rCtx, pCB);
        else if ((eProto == INET_PROT_FTP) && BAPP()->ShouldUseFtpProxy (rUrl))
            xImpl = new UcbHTTPTransport_Impl (rUrl, rCtx, pCB);
        else
            xImpl = new UcbTransport_Impl (rUrl, rCtx, pCB);
        return new UcbTransport (xImpl);
    }
    return NULL;
}

}
