/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: streamsection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:40:28 $
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

#ifndef _COMPHELPER_STREAMSECTION_HXX_
#define _COMPHELPER_STREAMSECTION_HXX_

#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAINPUTSTREAM_HPP_
#include <com/sun/star/io/XDataInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XDataOutputStream.hpp>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

namespace comphelper
{

    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

/** implements handling for compatibly reading/writing data from/into an input/output stream.
    data written in a block secured by this class should be readable by older versions which
    use the same mechanism.

    @author Frank Schoenheit
    @since  00/26/05
*/

class COMPHELPER_DLLPUBLIC OStreamSection
{
    staruno::Reference< stario::XMarkableStream >       m_xMarkStream;
    staruno::Reference< stario::XDataInputStream >      m_xInStream;
    staruno::Reference< stario::XDataOutputStream >     m_xOutStream;

    sal_Int32   m_nBlockStart;
    sal_Int32   m_nBlockLen;

public:
    /** starts reading of a "skippable" section of data within the given input stream<BR>
        @param      _rxInput    the stream to read from. Must support the
                                <type scope="com::sun::star::io">XMarkableStream</type> interface
    */
    OStreamSection(const staruno::Reference< stario::XDataInputStream >& _rxInput);

    /** starts writing of a "skippable" section of data into the given output stream
        @param      _rxOutput           the stream the stream to write to. Must support the
                                        <type scope="com::sun::star::io">XMarkableStream</type> interface
        @param      _nPresumedLength    estimation for the length of the upcoming section. If greater 0, this
                                        value will be written as section length and corrected (in the dtor) only if
                                        needed. If you know how much bytes you are about to write, you may
                                        want to use this param, saving some stream operations this way.
    */
    OStreamSection(const staruno::Reference< stario::XDataOutputStream >& _rxOutput, sal_Int32 _nPresumedLength = 0);

    /** dtor. <BR>If constructed for writing, the section "opened" by this object will be "closed".<BR>
        If constructed for reading, any remaining bytes 'til the end of the section will be skipped.
    */
    ~OStreamSection();
    /**
        return the number of bytes which are still available
    */
    sal_Int32 available();
};

}   // namespace comphelper

#endif // _COMPHELPER_STREAMSECTION_HXX_
