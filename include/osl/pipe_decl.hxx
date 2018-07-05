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
#ifndef INCLUDED_OSL_PIPE_DECL_HXX
#define INCLUDED_OSL_PIPE_DECL_HXX

#include "osl/pipe.h"
#   include "osl/security.hxx"
#include "rtl/ustring.hxx"

namespace osl {

class StreamPipe;

/** Represents a pipe.
*/
class Pipe
{
protected:
    oslPipe m_handle;

public:

    /** Does not create a pipe. Use assignment operator to
        make this a usable pipe.
    */
    inline Pipe();

    /** Creates an insecure pipe that is accessible for all users.
        @param strName
        @param Options
    */
    inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options);

    /** Creates a secure pipe that access depends on the umask settings.
        @param strName
        @param Options
        @param rSecurity
    */
    inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options,const Security & rSecurity);

    /** Copy constructor.
    */
    inline Pipe(const Pipe& pipe);

#if defined LIBO_INTERNAL_ONLY
    inline Pipe(Pipe && other);
#endif

    /** Constructs a Pipe reference without acquiring the handle
    */
    inline Pipe( oslPipe pipe, __sal_NoAcquire noacquire );

    /** Creates pipe as wrapper around the underlying oslPipe.
        @param Pipe
    */
    inline Pipe(oslPipe Pipe);

    /** Destructor. Destroys the underlying oslPipe.
    */
    inline ~Pipe();

    inline bool SAL_CALL is() const;

    /** Creates an insecure pipe that is accessible for all users
        with the given attributes.
        If the pipe was already created, the old one will be discarded.
        @param strName
        @param Options
        @param rSec
        @return True if socket was successfully created.
    */
    inline bool create( const ::rtl::OUString & strName,
                            oslPipeOptions Options, const Security &rSec );

    /** Creates a secure that access rights depend on the umask settings
        with the given attributes.

        If socket was already created, the old one will be discarded.
        @param strName
        @param Options
        @return True if socket was successfully created.
    */
    inline bool create( const ::rtl::OUString & strName, oslPipeOptions Options = osl_Pipe_OPEN );

    /** releases the underlying handle
     */
    inline void SAL_CALL clear();

    /** Assignment operator. If pipe was already created, the old one will
        be discarded.
    */
    inline Pipe& SAL_CALL operator= (const Pipe& pipe);

#if defined LIBO_INTERNAL_ONLY
    inline Pipe & operator =(Pipe && other);
#endif

    /** Assignment operator. If pipe was already created, the old one will
        be discarded.
    */
    inline Pipe& SAL_CALL operator= (const oslPipe pipe );

    /** Checks if the pipe is valid.
        @return True if the object represents a valid pipe.
    */
    inline bool SAL_CALL isValid() const;

    inline bool SAL_CALL operator==( const Pipe& rPipe ) const;

    /** Closes the pipe.
    */
    inline void SAL_CALL close();

    /** Accept connection on an existing pipe
    */
    inline oslPipeError SAL_CALL accept(StreamPipe& Connection);


    /** Delivers a constant describing the last error for the pipe system.
        @return ENONE if no error occurred, invalid_PipeError if
        an unknown (unmapped) error occurred, otherwise an enum describing the
        error.
    */
    inline oslPipeError SAL_CALL getError() const;

    inline oslPipe SAL_CALL getHandle() const;
};

/** A pipe to send or receive a stream of data.
*/
class StreamPipe : public Pipe
{
public:

    /** Creates an unattached pipe. You must attach the pipe to an oslPipe
        e.g. by using the operator=(oslPipe), before you can use the stream-
        functionality of the object.
    */
    inline StreamPipe();

    /** Creates pipe as wrapper around the underlying oslPipe.

        @param Pipe
    */
    inline StreamPipe(oslPipe Pipe);

    /** Creates a pipe.

        @param[in] strName Pipe name
        @param[in] Options Pipe options
    */
    inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options = osl_Pipe_OPEN);

    /** Creates a pipe.

        @param[in] strName Pipe name
        @param[in] Options Pipe options
        @param[in] rSec    Security for the pipe
    */
    inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec );

    /** Constructs a Pipe reference without acquiring the handle
    */
    inline StreamPipe( oslPipe pipe, __sal_NoAcquire noacquire );

    /** Attaches the oslPipe to this object. If the object
        already was attached to an oslPipe, the old one will
        be closed and destroyed.

        @param[in] Pipe    Pipe to attach to this object
    */
    inline StreamPipe & SAL_CALL operator=(oslPipe Pipe);

    /** Assignment operator
    */
    inline StreamPipe& SAL_CALL operator=(const Pipe& pipe);

    /** Tries to receives BytesToRead data from the connected pipe,

        @param[out] pBuffer Points to a buffer that will be filled with the received
                     data.
        @param[in] BytesToRead The number of bytes to read. pBuffer must have at least
                     this size.

        @return the number of received bytes.
    */
    inline sal_Int32 SAL_CALL recv(void* pBuffer, sal_Int32 BytesToRead) const;

    /** Tries to sends BytesToSend data from the connected pipe.

        @param[in] pBuffer Points to a buffer that contains the send-data.
        @param[in] BytesToSend The number of bytes to send. pBuffer must have at least
                       this size.

        @return the number of transferred bytes.
    */
    inline sal_Int32 SAL_CALL send(const void* pBuffer, sal_Int32 BytesToSend) const;

    /** Retrieves n bytes from the stream and copies them into pBuffer.
        The method avoids incomplete reads due to packet boundaries.

        @param[in] pBuffer receives the read data.
        @param[in] n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!

        @return the number of read bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
    */
    inline sal_Int32 SAL_CALL read(void* pBuffer, sal_Int32 n) const;

    /** Writes n bytes from pBuffer to the stream. The method avoids
        incomplete writes due to packet boundaries.

        @param[in] pBuffer contains the data to be written.
        @param[in] n the number of bytes to write.

        @return the number of written bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
    */
    sal_Int32 SAL_CALL write(const void* pBuffer, sal_Int32 n) const;
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
