/*************************************************************************
 *
 *  $RCSfile: pipe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 10:01:09 $
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


#ifndef _VOS_PIPE_HXX_
#define _VOS_PIPE_HXX_

#ifndef _OSL_PIPE_H_
#   include <osl/pipe.h>
#endif
#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _VOS_ISTREAM_HXX_
#   include <vos/istream.hxx>
#endif
#ifndef _VOS_REFERMCE_HXX_
#   include <vos/refernce.hxx>
#endif
#ifndef _VOS_REFOBJ_HXX_
#   include <vos/refobj.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
//# include <vos/security.hxx>
#endif
#ifndef _RTL_USTRING_
#   include <rtl/ustring>
#endif


#ifdef _USE_NAMESPACE
namespace vos
{
#endif


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
    OPipe(const NAMESPACE_RTL(OUString)& strName, TPipeOption Options = TOption_Open);

    /** Creates a pipe.
        @param strName
        @param Options
        @param Security
    */
    OPipe(const NAMESPACE_RTL(OUString)& strName, TPipeOption Options,
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
    sal_Bool SAL_CALL create(const NAMESPACE_RTL(OUString)& strName, TPipeOption Options = TOption_Open);

    /** Create a pipe with the given attributes.
        If socket was already created, the old one will be discarded.
        @param strName
        @param Options
        @param Security
        @return True if socket was successfully created.
    */
    sal_Bool SAL_CALL create(const NAMESPACE_RTL(OUString)& strName, TPipeOption Options,
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


#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_PIPE_HXX_

