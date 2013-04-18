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
};

}   // namespace comphelper

#endif // _COMPHELPER_STREAMSECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
