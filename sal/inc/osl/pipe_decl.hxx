/*************************************************************************
 *
 *  $RCSfile: pipe_decl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:28:31 $
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
#ifndef _OSL_PIPE_DECL_HXX_
#define _OSL_PIPE_DECL_HXX_

#ifndef _OSL_PIPE_H_
#include <osl/pipe.h>
#endif

#ifndef _OSL_SECURITY_HXX_
#   include <osl/security.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
        make this a useable pipe.
    */
    inline Pipe();

    /** Creates a pipe.
        @param strName
        @param Options
        @param Security
    */
    inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options);

    /** Creates a pipe.
        @param strName
        @param Options
        @param Security
    */
    inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options,const Security & rSecurity);

    /** Copy constructor.
    */
    inline Pipe(const Pipe& pipe);

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

    inline sal_Bool SAL_CALL is() const;

    /** Create a pipe with the given attributes.
        If the pipe was already created, the old one will be discarded.
        @param strName
        @param Options
        @param Security
        @return True if socket was successfully created.
    */
    inline sal_Bool create( const ::rtl::OUString & strName,
                            oslPipeOptions Options, const Security &rSec );

    /** Create a pipe with the given attributes.
        If socket was already created, the old one will be discarded.
        @param strName
        @param Options
        @return True if socket was successfully created.
    */
    inline sal_Bool create( const ::rtl::OUString & strName, oslPipeOptions Options = osl_Pipe_OPEN );

    /** releases the underlying handle
     */
    inline void SAL_CALL clear();

    /** Assignment operator. If pipe was already created, the old one will
        be discarded.
    */
    inline Pipe& SAL_CALL operator= (const Pipe& pipe);

    /** Assignment operator. If pipe was already created, the old one will
        be discarded.
    */
    inline Pipe& SAL_CALL operator= (const oslPipe pipe );

    /** Checks if the pipe is valid.
        @return True if the object represents a valid pipe.
    */
    inline sal_Bool SAL_CALL isValid() const;

    inline sal_Bool SAL_CALL operator==( const Pipe& rPipe ) const;

    /** Closes the pipe.
    */
    inline void SAL_CALL close();

    /** Accept connection on an existing pipe
    */
    inline oslPipeError SAL_CALL accept(StreamPipe& Connection);


    /** Delivers a constant decribing the last error for the pipe system.
        @return ENONE if no error occured, invalid_PipeError if
        an unknown (unmapped) error occured, otherwise an enum describing the
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

    /** Copy constructor.
        @param Pipe
    */
    inline StreamPipe(const StreamPipe& Pipe);

    /** Creates a pipe.
        @param strName
        @param Options
    */
    inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options = osl_Pipe_OPEN);

    /** Creates a pipe.
        @param strName
        @param Options
        @param rSec
    */
    inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec );

    /** Constructs a Pipe reference without acquiring the handle
    */
    inline StreamPipe( oslPipe pipe, __sal_NoAcquire noacquire );

    /** Attaches the oslPipe to this object. If the object
        already was attached to an oslPipe, the old one will
        be closed and destroyed.
        @param Pipe.
    */
    inline StreamPipe & SAL_CALL operator=(oslPipe Pipe);

    /** Assignment operator
    */
    inline StreamPipe& SAL_CALL operator=(const Pipe& pipe);

    /** Tries to receives BytesToRead data from the connected pipe,

        @param pBuffer [out] Points to a buffer that will be filled with the received
        data.
        @param BytesToRead [in] The number of bytes to read. pBuffer must have at least
        this size.
        @return the number of received bytes.
    */
    inline sal_Int32 SAL_CALL recv(void* pBuffer, sal_Int32 BytesToRead) const;

    /** Tries to sends BytesToSend data from the connected pipe.

        @param pBuffer [in] Points to a buffer that contains the send-data.
        @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
        this size.
        @return the number of transfered bytes.
    */
    inline sal_Int32 SAL_CALL send(const void* pBuffer, sal_Int32 BytesToSend) const;

    /** Retrieves n bytes from the stream and copies them into pBuffer.
        The method avoids incomplete reads due to packet boundaries.
        @param pBuffer receives the read data.
        @param n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!
        @return the number of read bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    inline sal_Int32 SAL_CALL read(void* pBuffer, sal_Int32 n) const;

    /** Writes n bytes from pBuffer to the stream. The method avoids
        incomplete writes due to packet boundaries.
        @param pBuffer contains the data to be written.
        @param n the number of bytes to write.
        @return the number of written bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    sal_Int32 SAL_CALL write(const void* pBuffer, sal_Int32 n) const;
};

}
#endif
