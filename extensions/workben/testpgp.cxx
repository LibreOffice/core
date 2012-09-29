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

#include <sal/types.h>
#ifndef _RTL_WSTRING_
#include <rtl/wstring>
#endif

#include <usr/smartservices.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/pgp/RecipientsEvent.hpp>
#include <com/sun/star/pgp/SignatureEvent.hpp>
#include <com/sun/star/pgp/XPGPDecoder.hpp>
#include <com/sun/star/pgp/XPGPDecoderListener.hpp>
#include <com/sun/star/pgp/XPGPEncoder.hpp>
#include <com/sun/star/pgp/XPGPPreferences.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <io.h>

using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::pgp;
using namespace com::sun::star::uno;

/*========================================================================
 *
 * DataSource_Impl interface.
 *
 *======================================================================*/
class DataSource_Impl :
    public OWeakObject,
    public XInputStream
{
    Sequence<sal_Int8> m_buffer;
    sal_Int32          m_position;
    int                m_fd;

public:
    DataSource_Impl (int fd = 0);
    virtual ~DataSource_Impl (void);

    void setBuffer (const Sequence<sal_Int8> &rBuffer);

    /** XInterface.
     */
    virtual sal_Bool SAL_CALL queryInterface (
        const Uik &rUik, Any &rIfc) throw(RuntimeException);

    virtual void SAL_CALL acquire (void) throw(RuntimeException);

    virtual void SAL_CALL release (void) throw(RuntimeException);

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
};

/*========================================================================
 *
 * DataSink_Impl interface.
 *
 *======================================================================*/
class DataSink_Impl :
    public OWeakObject,
    public XOutputStream
{
    Sequence<sal_Int8> m_buffer;

public:
    DataSink_Impl (void);
    virtual ~DataSink_Impl (void);

    const Sequence<sal_Int8>& getBuffer (void) const { return m_buffer; }

    /** XInterface.
     */
    virtual sal_Bool SAL_CALL queryInterface (
        const Uik &rUik, Any &rIfc) throw(RuntimeException);

    virtual void SAL_CALL acquire (void) throw(RuntimeException);
    virtual void SAL_CALL release (void) throw(RuntimeException);

    /** XOutputStream.
     */
    virtual void SAL_CALL writeBytes (
        const Sequence<sal_Int8> &rBuffer)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);

    virtual void SAL_CALL flush (void)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);

    virtual void SAL_CALL closeOutput (void)
        throw (NotConnectedException,
               BufferSizeExceededException,
               IOException);
};

/*========================================================================
 *
 * DecoderListener_Impl interface.
 *
 *======================================================================*/
class DecoderListener_Impl :
    public OWeakObject,
    public XPGPDecoderListener
{
public:
    DecoderListener_Impl (void);
    virtual ~DecoderListener_Impl (void);

    /** XInterface.
     */
    virtual sal_Bool SAL_CALL queryInterface (
        const Uik &rUik, Any &rIfc) throw(RuntimeException);

    virtual void SAL_CALL acquire (void) throw(RuntimeException);

    virtual void SAL_CALL release (void) throw(RuntimeException);

    /** XEventListener.
     */
    virtual void SAL_CALL disposing (const EventObject &rSource);

    /** XPGPDecoderListener.
     */
    virtual void SAL_CALL decrypted (const RecipientsEvent &rEvent);
    virtual void SAL_CALL verified  (const SignatureEvent &rEvent);
};

/*========================================================================
 *
 * DataSource_Impl implementation.
 *
 *======================================================================*/
/*
 * DataSource_Impl.
 */
DataSource_Impl::DataSource_Impl (int fd)
    : m_fd (fd), m_position (0)
{
}

/*
 * ~DataSource_Impl.
 */
DataSource_Impl::~DataSource_Impl (void)
{
}

/*
 * DataSource_Impl: setBuffer.
 */
void DataSource_Impl::setBuffer (const Sequence<sal_Int8> &rBuffer)
{
    m_buffer = rBuffer;
    if (!m_buffer.getLength())
    {
        // Fill buffer from file descriptor.
        char buffer[1024];
        memset (buffer, 0, sizeof(buffer));

        int k = read (m_fd, buffer, sizeof(buffer));
        while (k > 0)
        {
            sal_Int32 n = m_buffer.getLength();
            m_buffer.realloc (n + k);

            memcpy (m_buffer.getArray() + n, buffer, k);
            n += k;

            memset (buffer, 0, k);
            k = read (m_fd, buffer, sizeof(buffer));
        }
    }
    m_position = 0;
}

/*
 * XInterface: queryInterface.
 */
sal_Bool SAL_CALL DataSource_Impl::queryInterface (
    const Uik &rUik, Any &rIfc) throw(RuntimeException)
{
    if (com::sun::star::uno::queryInterface (
        rUik, rIfc,
        (static_cast< XInputStream* >(this))))
        return sal_True;
    else
        return OWeakObject::queryInterface (rUik, rIfc);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL DataSource_Impl::acquire (void) throw(RuntimeException)
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL DataSource_Impl::release (void) throw(RuntimeException)
{
    OWeakObject::release();
}

/*
 * XInputStream: readBytes.
 */
sal_Int32 SAL_CALL DataSource_Impl::readBytes (
    Sequence<sal_Int8> &rData, sal_Int32 nBytesToRead)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    if (nBytesToRead < 0)
        throw IOException();

    sal_Int32 k = m_buffer.getLength() - m_position;
    k = SAL_BOUND(k, 0, nBytesToRead);
    if (k > 0)
    {
        rData.realloc(k);
        memcpy (
            rData.getArray(), m_buffer.getConstArray() + m_position, k);
        m_position += k;
    }
    return k;
}

/*
 * XInputStream: readSomeBytes.
 */
sal_Int32 SAL_CALL DataSource_Impl::readSomeBytes (
    Sequence<sal_Int8> &rData, sal_Int32 nMaxBytesToRead)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    return readBytes (rData, nMaxBytesToRead);
}

/*
 * XInputStream: skipBytes.
 */
void SAL_CALL DataSource_Impl::skipBytes (sal_Int32 nBytesToSkip)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    if (nBytesToSkip < 0)
        throw IOException();

    m_position += nBytesToSkip;
}

/*
 * XInputStream: available.
 */
sal_Int32 SAL_CALL DataSource_Impl::available (void)
    throw (NotConnectedException, IOException)
{
    sal_Int32 k = m_buffer.getLength() - m_position;
    return ((k > 0) ? k : 0);
}

/*
 * XInputStream: closeInput.
 */
void SAL_CALL DataSource_Impl::closeInput (void)
    throw (NotConnectedException, IOException)
{
}

/*========================================================================
 *
 * DataSink_Impl implementation.
 *
 *======================================================================*/
/*
 * DataSink_Impl.
 */
DataSink_Impl::DataSink_Impl (void)
{
}

/*
 * ~DataSink_Impl.
 */
DataSink_Impl::~DataSink_Impl (void)
{
}

/*
 * XInterface: queryInterface.
 */
sal_Bool SAL_CALL DataSink_Impl::queryInterface (
    const Uik &rUik, Any &rIfc) throw(RuntimeException)
{
    if (com::sun::star::uno::queryInterface (
        rUik, rIfc,
        (static_cast< XOutputStream* >(this))))
        return sal_True;
    else
        return OWeakObject::queryInterface (rUik, rIfc);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL DataSink_Impl::acquire (void) throw(RuntimeException)
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL DataSink_Impl::release (void) throw(RuntimeException)
{
    OWeakObject::release();
}

/*
 * XOutputStream: writeBytes.
 */
void SAL_CALL DataSink_Impl::writeBytes (const Sequence<sal_Int8> &rBuffer)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    if (rBuffer.getLength())
    {
        sal_Int32 n = m_buffer.getLength();
        m_buffer.realloc (n + rBuffer.getLength());

        memcpy (
            m_buffer.getArray() + n,
            rBuffer.getConstArray(),
            rBuffer.getLength());
    }
}

/*
 * XOutputStream: flush.
 */
void SAL_CALL DataSink_Impl::flush (void)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    if (m_buffer.getLength())
    {
        // Write data to stdout.
        const sal_Int8 *pData = m_buffer.getConstArray();
        sal_Int32       nData = m_buffer.getLength();

        int prev = ::setmode (1, O_BINARY);
        if (!(prev < 0))
        {
            int k = ::write (1, pData, nData);
            if (k > 0)
                ::write (1, "\n", 1);
            ::setmode (1, prev);
        }
    }
}

/*
 * XOutputStream: closeOutput.
 */
void SAL_CALL DataSink_Impl::closeOutput (void)
    throw (NotConnectedException, BufferSizeExceededException, IOException)
{
    flush();
}

/*========================================================================
 *
 * DecoderListener_Impl implementation.
 *
 *======================================================================*/
/*
 * DecoderListener_Impl.
 */
DecoderListener_Impl::DecoderListener_Impl (void)
{
}

/*
 * ~DecoderListener_Impl.
 */
DecoderListener_Impl::~DecoderListener_Impl (void)
{
}

/*
 * XInterface: queryInterface.
 */
sal_Bool SAL_CALL DecoderListener_Impl::queryInterface (
    const Uik &rUik, Any &rIfc) throw(RuntimeException)
{
    if (com::sun::star::uno::queryInterface (
        rUik, rIfc,
        (static_cast< XEventListener* >(this)),
        (static_cast< XPGPDecoderListener* >(this))))
        return sal_True;
    else
        return OWeakObject::queryInterface (rUik, rIfc);
}

/*
 * XInterface: acquire.
 */
void SAL_CALL DecoderListener_Impl::acquire (void) throw(RuntimeException)
{
    OWeakObject::acquire();
}

/*
 * XInterface: release.
 */
void SAL_CALL DecoderListener_Impl::release (void) throw(RuntimeException)
{
    OWeakObject::release();
}

/*
 * XEventListener: disposing.
 */
void SAL_CALL DecoderListener_Impl::disposing (const EventObject &rSource)
{
}

/*
 * XPGPDecoderListener: decrypted.
 */
void SAL_CALL DecoderListener_Impl::decrypted (const RecipientsEvent &rEvent)
{
}

/*
 * XPGPDecoderListener: verified.
 */
void SAL_CALL DecoderListener_Impl::verified (const SignatureEvent &rEvent)
{
}

/*========================================================================
 *
 * Main.
 *
 *======================================================================*/
inline rtl::OWString S2U (const sal_Char *ascii)
{
    return rtl::OWString::createFromAscii (ascii);
}

/*
 * main.
 */
int SAL_CALL main (int argc, char **argv)
{
    enum Option
    {
        OPTION_INSTALL   = 0x01,
        OPTION_UNINSTALL = 0x02,

        OPTION_DECRYPT   = 0x04,
        OPTION_ENCRYPT   = 0x08,
        OPTION_SIGN      = 0x10,

        OPTION_FILE      = 0x20,
        OPTION_HELP      = 0x40
    };

    int fd = 0;
    unsigned long nOptions = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *opt = argv[i];
        if (opt[0] == '-')
        {
            switch (opt[1])
            {
                case 'i':
                    nOptions |= OPTION_INSTALL;
                    break;

                case 'u':
                    nOptions |= OPTION_UNINSTALL;
                    break;

                case 'd':
                    nOptions |= OPTION_DECRYPT;
                    break;

                case 'e':
                    nOptions |= OPTION_ENCRYPT;
                    break;

                case 's':
                    nOptions |= OPTION_SIGN;
                    break;

                case 'f':
                    nOptions |= OPTION_FILE;
                    break;

                case 'h':
                default:
                    nOptions |= OPTION_HELP;
                    break;
            }
        }
        else
        {
            if (nOptions & OPTION_FILE)
            {
                if ((fd = open (argv[i], O_RDONLY | O_BINARY)) < 0)
                {
                    printf ("Error: can't open file: %s\n", argv[i]);
                    exit (0);
                }
            }
        }
    }

    if ((nOptions == 0) || (nOptions & OPTION_HELP))
    {
        printf ("Options:\n");
        printf ("-i\tinstall module\n");
        printf ("-u\tuninstall module\n");
        printf ("-d\tdecrypt and verify\n");
        printf ("-e\tencrypt test pattern\n");
        printf ("-s\tsign test pattern\n");
        printf ("-h\thelp\n");
        exit (0);
    }

    Reference<XMultiServiceFactory> xManager (
        usr::createDefaultSmartRegistryServiceFactory());
    if (!xManager.is())
    {
        printf ("Error: no ProcessServiceManager.\n");
        exit (1);
    }
    usr::setProcessServiceFactory (xManager);

    if (nOptions & OPTION_INSTALL)
    {

    }

    if (nOptions & (OPTION_DECRYPT | OPTION_ENCRYPT | OPTION_SIGN))
    {
        Reference<XMultiServiceFactory> xProv (
            xManager->createInstance (
                S2U("com.sun.star.pgp.PGPFactory")),
            UNO_QUERY);
        if (!xProv.is())
        {
            printf ("Error: no PGPFactory service.\n");
            exit (1);
        }

        Reference<XInterface> xProvInst (
            xProv->createInstance (
                S2U("com.sun.star.pgp.SimplePGPMailer")));
        if (!xProvInst.is())
        {
            printf ("Error: no SimplePGPMailer service.\n");
            exit (2);
        }

        Reference<XPGPPreferences> xPrefs (xProvInst, UNO_QUERY);
        if (xPrefs.is())
        {
            unsigned long nDefaults = 0;

            if (xPrefs->getEncryptByDefault())
                nDefaults |= OPTION_ENCRYPT;
            if (xPrefs->getSignByDefault())
                nDefaults |= OPTION_SIGN;
            if (xPrefs->getAutoDecrypt())
                nDefaults |= OPTION_DECRYPT;

            if (nDefaults)
            {
            }
        }

        static const sal_Int8 pData[] = "" /* "Hello PGP World." */;
        Sequence<sal_Int8> buffer (pData, sizeof (pData) - 1);

        if (nOptions & (OPTION_ENCRYPT | OPTION_SIGN))
        {
            Reference<XPGPEncoder> xEncoder (xProvInst, UNO_QUERY);
            if (!xEncoder.is())
            {
                printf ("Error: no PGPEncoder interface.\n");
                exit (4);
            }

            DataSource_Impl *source = new DataSource_Impl (fd);
            source->setBuffer (buffer);

            DataSink_Impl *sink = new DataSink_Impl;

            Reference<XInputStream>  xPlainText  (source);
            Reference<XOutputStream> xCipherText (sink);

            if (nOptions & OPTION_ENCRYPT)
            {
                rtl::OWString aRecipients[] =
                {
                    S2U("er@stardiv.de"),
                    // L"mhu@stardivision.de",
                    S2U("mhu@rabbit")
                };

                sal_Int32 nRecipients = SAL_N_ELEMENTS(aRecipients);

                if (nOptions & OPTION_SIGN)
                {
                    xEncoder->encryptAndSign (
                        Sequence<rtl::OWString>(aRecipients, nRecipients),
                        xPlainText,
                        xCipherText);
                    nOptions &= ~OPTION_SIGN;
                }
                else
                {
                    xEncoder->encrypt (
                        Sequence<rtl::OWString>(aRecipients, nRecipients),
                        xPlainText,
                        xCipherText);
                }
                nOptions &= ~OPTION_ENCRYPT;
            }

            if (nOptions & OPTION_SIGN)
            {
                sal_Bool bDataIsAscii = (fd == 0); // stdin.

                xEncoder->sign (
                    bDataIsAscii,
                    xPlainText,
                    xCipherText);
                nOptions &= ~OPTION_SIGN;
            }

            buffer = sink->getBuffer();
        }

        if (nOptions & OPTION_DECRYPT)
        {
            Reference<XPGPDecoder> xDecoder (xProvInst, UNO_QUERY);
            if (!xDecoder.is())
            {
                printf ("Error: no PGPDecoder interface.\n");
                exit (5);
            }

            DataSource_Impl *source = new DataSource_Impl;
            source->setBuffer (buffer);

            DataSink_Impl *sink = new DataSink_Impl;

            Reference<XInputStream>  xCipherText (source);
            Reference<XOutputStream> xPlainText  (sink);

            Reference<XPGPDecoderListener> xListener (
                new DecoderListener_Impl);
            xDecoder->addDecoderListener (xListener);

            xDecoder->decryptAndVerify (
                xCipherText,
                xPlainText);
            nOptions &= ~OPTION_DECRYPT;

            xDecoder->removeDecoderListener (xListener);

            buffer = sink->getBuffer();
        }
    }

    if (nOptions & OPTION_UNINSTALL)
    {

    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
