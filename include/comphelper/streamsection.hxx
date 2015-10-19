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

#ifndef INCLUDED_COMPHELPER_STREAMSECTION_HXX
#define INCLUDED_COMPHELPER_STREAMSECTION_HXX

#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XDataInputStream.hpp>
#include <com/sun/star/io/XDataOutputStream.hpp>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

/** implements handling for compatibly reading/writing data from/into an input/output stream.
    data written in a block secured by this class should be readable by older versions which
    use the same mechanism.

    @author Frank Schoenheit
    @since  00/26/05
*/

class COMPHELPER_DLLPUBLIC OStreamSection
{
    css::uno::Reference< css::io::XMarkableStream >       m_xMarkStream;
    css::uno::Reference< css::io::XDataInputStream >      m_xInStream;
    css::uno::Reference< css::io::XDataOutputStream >     m_xOutStream;

    sal_Int32   m_nBlockStart;
    sal_Int32   m_nBlockLen;

public:
    /** starts reading of a "skippable" section of data within the given input stream<BR>
        @param      _rxInput    the stream to read from. Must support the
                                css::io::XMarkableStream interface
    */
    OStreamSection(const css::uno::Reference< css::io::XDataInputStream >& _rxInput);

    /** starts writing of a "skippable" section of data into the given output stream
        @param      _rxOutput           the stream the stream to write to. Must support the
                                        css::io::XMarkableStream interface
        @param      _nPresumedLength    estimation for the length of the upcoming section. If greater 0, this
                                        value will be written as section length and corrected (in the dtor) only if
                                        needed. If you know how much bytes you are about to write, you may
                                        want to use this param, saving some stream operations this way.
    */
    OStreamSection(const css::uno::Reference< css::io::XDataOutputStream >& _rxOutput, sal_Int32 _nPresumedLength = 0);

    /** dtor. <BR>If constructed for writing, the section "opened" by this object will be "closed".<BR>
        If constructed for reading, any remaining bytes 'til the end of the section will be skipped.
    */
    ~OStreamSection();
};

}   // namespace comphelper

#endif // INCLUDED_COMPHELPER_STREAMSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
