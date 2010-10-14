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


#ifndef _VOS_PIPE_HXX_
#define _VOS_PIPE_HXX_

#   include <osl/pipe.h>
#   include <vos/types.hxx>
#   include <vos/object.hxx>
#   include <vos/istream.hxx>
#ifndef _VOS_REFERMCE_HXX_
#   include <vos/refernce.hxx>
#endif
#   include <vos/refobj.hxx>
#   include <vos/security.hxx>
#   include <rtl/ustring.hxx>


namespace vos
{

class OStreamPipe;

/** Represents a pipe.
*/
class OPipe : public NAMESPACE_VOS(OReference),
              public NAMESPACE_VOS(OObject)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OPipe));

public:
    /*
        Represents pipe-options
    */
    enum TPipeOption {
        TOption_Open   = osl_Pipe_OPEN,         /* open existing pipe */
        TOption_Create = osl_Pipe_CREATE        /* create pipe and open it, */
                                                /*  fails if already existst */
    };

    enum TPipeError {
        E_None              = osl_Pipe_E_None,              /* no error */
        E_NotFound          = osl_Pipe_E_NotFound,          /* Pipe could not be found */
        E_AlreadyExists     = osl_Pipe_E_AlreadyExists,     /* Pipe already exists */
        E_NoProtocol        = osl_Pipe_E_NoProtocol,        /* Protocol not available */
        E_NetworkReset      = osl_Pipe_E_NetworkReset,      /* Network dropped connection because of reset */
        E_ConnectionAbort   = osl_Pipe_E_ConnectionAbort,   /* Software caused connection abort */
        E_ConnectionReset   = osl_Pipe_E_ConnectionReset,   /* Connection reset by peer */
        E_NoBufferSpace     = osl_Pipe_E_NoBufferSpace,     /* No buffer space available */
        E_TimedOut          = osl_Pipe_E_TimedOut,          /* Connection timed out */
        E_ConnectionRefused = osl_Pipe_E_ConnectionRefused, /* Connection refused */
        E_invalidError      = osl_Pipe_E_invalidError       /* unmapped error */
    };

protected:
    typedef ORefObj<oslPipe> PipeRef;

    PipeRef* m_pPipeRef;

public:

    /** Does not create a pipe. Use assignment operator to
        make this a useable pipe.
    */
    OPipe();

    /** Creates a pipe.
        @param strName
        @param Options
    */
    OPipe(const ::rtl::OUString& strName, TPipeOption Options = TOption_Open);

    /** Creates a pipe.
        @param strName
        @param Options
        @param Security
    */
    OPipe(const ::rtl::OUString& strName, TPipeOption Options,
          const NAMESPACE_VOS(OSecurity)& rSecurity);

    /** Copy constructor.
    */
    OPipe(const OPipe& pipe);

    /** Creates pipe as wrapper around the underlying oslPipe.
        @param Pipe
    */
    OPipe(oslPipe Pipe);

    /** Destructor. Destroys the underlying oslPipe.
    */
    virtual ~OPipe();

    /** Create a pipe with the given attributes.
        If socket was already created, the old one will be discarded.
        @param strName
        @param Options
        @return True if socket was successfully created.
    */
    sal_Bool SAL_CALL create(const ::rtl::OUString& strName, TPipeOption Options = TOption_Open);

    /** Create a pipe with the given attributes.
        If socket was already created, the old one will be discarded.
        @param strName
        @param Options
        @param Security
        @return True if socket was successfully created.
    */
    sal_Bool SAL_CALL create(const ::rtl::OUString& strName, TPipeOption Options,
                   const NAMESPACE_VOS(OSecurity)& rSecurity);

    /** Assignment operator. If pipe was already created, the old one will
        be discarded.
    */
    OPipe& SAL_CALL operator= (const OPipe& pipe);

    /** Allow cast to underlying oslPipe.
    */
    SAL_CALL operator oslPipe() const;

    /** Checks if the pipe is valid.
        @return True if the object represents a valid pipe.
    */
    sal_Bool SAL_CALL isValid() const;

    sal_Bool SAL_CALL operator==( const OPipe& rPipe )
    {
        return m_pPipeRef == rPipe.m_pPipeRef;
    }

    /** Closes the pipe.
    */
    virtual void SAL_CALL close();

    /** Accept connection on an existing pipe
    */
    TPipeError SAL_CALL accept(OStreamPipe& Connection);

    /** Tries to receives BytesToRead data from the connected pipe,

        @param pBuffer [out] Points to a buffer that will be filled with the received
        data.
        @param BytesToRead [in] The number of bytes to read. pBuffer must have at least
        this size.
        @return the number of received bytes.
    */
    sal_Int32   SAL_CALL recv(void* pBuffer, sal_uInt32 BytesToRead);

    /** Tries to sends BytesToSend data from the connected pipe.

        @param pBuffer [in] Points to a buffer that contains the send-data.
        @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
        this size.
        @return the number of transfered bytes.
    */
    sal_Int32 SAL_CALL send(const void* pBuffer, sal_uInt32 BytesToSend);

    /** Delivers a constant decribing the last error for the pipe system.
        @return ENONE if no error occured, invalid_PipeError if
        an unknown (unmapped) error occured, otherwise an enum describing the
        error.
    */
    TPipeError SAL_CALL getError() const;

};

/** A pipe to send or receive a stream of data.
*/
class OStreamPipe : public NAMESPACE_VOS(OPipe),
                    public NAMESPACE_VOS(IStream)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OStreamPipe));
public:

    /** Creates an unattached pipe. You must attach the pipe to an oslPipe
        e.g. by using the operator=(oslPipe), before you can use the stream-
        functionality of the object.
    */
    OStreamPipe();

    /** Creates pipe as wrapper around the underlying oslPipe.
        @param Pipe
    */
    OStreamPipe(oslPipe Pipe);

    /** Copy constructor.
        @param Pipe
    */
    OStreamPipe(const OStreamPipe& Pipe);

    /** Destructor. Calls shutdown(readwrite) and close().
    */
    virtual ~OStreamPipe();

    /** Attaches the oslPipe to this object. If the object
        already was attached to an oslPipe, the old one will
        be closed and destroyed.
        @param Pipe.
    */
    OStreamPipe& SAL_CALL operator=(oslPipe Pipe);

    /** Assignment operator
    */
    OStreamPipe& SAL_CALL operator=(const OPipe& pipe);

    /** Retrieves n bytes from the stream and copies them into pBuffer.
        The method avoids incomplete reads due to packet boundaries.
        @param pBuffer receives the read data.
        @param n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!
        @return the number of read bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    virtual sal_Int32 SAL_CALL read(void* pBuffer, sal_uInt32 n) const;

    /** Writes n bytes from pBuffer to the stream. The method avoids
        incomplete writes due to packet boundaries.
        @param pBuffer contains the data to be written.
        @param n the number of bytes to write.
        @return the number of written bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    virtual sal_Int32 SAL_CALL write(const void* pBuffer, sal_uInt32 n);

    /** Checks if pipe is closed.
        @return True if pipe is closed.
    */
    virtual sal_Bool SAL_CALL isEof() const;
};

}

#endif // _VOS_PIPE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
