/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: streamsection.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _COMPHELPER_STREAMSECTION_HXX_
#define _COMPHELPER_STREAMSECTION_HXX_

#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XDataInputStream.hpp>
#include <com/sun/star/io/XDataOutputStream.hpp>
#include "comphelper/comphelperdllapi.h"

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
