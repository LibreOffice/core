/*************************************************************************
 *
 *  $RCSfile: seqstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-02 14:56:08 $
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
#ifndef _COMPHELPER_SEQSTREAM_HXX
#define _COMPHELPER_SEQSTREAM_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace comphelper
{

    typedef ::com::sun::star::uno::Sequence<sal_Int8> ByteSequence;

//==================================================================
// SequenceInputStream
// stream for reading data from a sequence of bytes
//==================================================================


class SequenceInputStream
: public ::cppu::WeakImplHelper1< ::com::sun::star::io::XInputStream >
{
    ::osl::Mutex    m_aMutex;
    ByteSequence    m_aData;
    sal_Int32       m_nPos;

public:
    SequenceInputStream(const ByteSequence& rData);

// com::sun::star::io::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
              ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
              ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
              ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

private:
    inline sal_Int32 avail();
};
typedef ::cppu::WeakImplHelper1< ::com::sun::star::io::XOutputStream > OSequenceOutputStream_Base;

class OSequenceOutputStream : public OSequenceOutputStream_Base
{
protected:
    ::com::sun::star::uno::Sequence< sal_Int8 >&    m_rSequence;
    double                                          m_nResizeFactor;
    sal_Int32                                       m_nMinimumResize;
    sal_Int32                                       m_nMaximumResize;
    sal_Int32                                       m_nSize;
        // the size of the virtual stream. This is not the size of the sequence, but the number of bytes written
        // into the stream at a given moment.

    sal_Bool                                        m_bConnected;
        // closeOutput has been called ?

    ::osl::Mutex                                    m_aMutex;

protected:
    ~OSequenceOutputStream() { if (m_bConnected) closeOutput(); }

public:
    /** constructs the object. Everything written into the stream through the XOutputStream methods will be forwarded
        to the sequence, reallocating it if neccessary. Writing will start at offset 0 within the sequence.
        @param      _rSeq               a reference to the sequence which will be used for output.
                                        The caller is responsible for taking care of the lifetime of the stream
                                        object and the sequence. If you're in doubt about this, use <code>closeOutput</code>
                                        before destroying the sequence
        @param      _nResizeFactor      the factor which is used for resizing the sequence when neccessary. In every
                                        resize step, the new sequence size will be calculated by multiplying the current
                                        size with this factor, rounded off to the next multiple of 4.
        @param      _nMinimumResize     the minmal number of bytes which is additionally allocated on resizing
        @param      _nMaximumResize     as the growth of the stream size is exponential, you may want to specify a
                                        maxmimum amount of memory which the sequence will grow by. If -1 is used,
                                        no limit is applied
        @see        closeOutput
    */
    OSequenceOutputStream(
        ::com::sun::star::uno::Sequence< sal_Int8 >& _rSeq,
        double _nResizeFactor = 1.3,
        sal_Int32 _nMinimumResize = 128,
        sal_Int32 _nMaximumResize = -1
        );

    /// same as XOutputStream::writeBytes (as expected :)
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    /// this is a dummy in this implementation, no buffering is used
    virtual void SAL_CALL flush(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    /** closes the output stream. In the case of this class, this means that the sequence used for writing is
        resized to the really used size and not used any further, every subsequent call to one of the XOutputStream
        methods will throw a <code>NotConnectedException</code>.
    */
    virtual void SAL_CALL closeOutput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

} // namespace comphelper

#endif //_COMPHELPER_SEQSTREAM_HXX


