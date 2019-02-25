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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OBOUNDPARAM_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OBOUNDPARAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <odbc/odbcbasedllapi.hxx>

namespace connectivity
{
    namespace odbc
    {
        class OOO_DLLPUBLIC_ODBCBASE OBoundParam
        {

        public:
            OBoundParam()
                : binaryData(nullptr)
                , paramLength(0)
                , paramInputStreamLen(0)
            {
            }
            ~OBoundParam()
            {
                free(binaryData);
            }

            // allocBindDataBuffer
            // Allocates and returns a new bind data buffer of the specified
            // length

            void* allocBindDataBuffer (sal_Int32 bufLen)
            {
                // Reset the input stream and sequence, we are doing a new bind
                setInputStream (nullptr, 0);
                aSequence.realloc(0);

                free(binaryData);
                binaryData = (bufLen > 0) ? malloc(bufLen) : nullptr;

                return binaryData;
            }


            // getBindLengthBuffer
            // Returns the length buffer to be used when binding to a parameter

            SQLLEN& getBindLengthBuffer ()
            {
                return paramLength;
            }


            // setInputStream
            // Sets the input stream for the bound parameter

            void setInputStream(const css::uno::Reference< css::io::XInputStream>& inputStream,
                                sal_Int32 len)
            {
                paramInputStream = inputStream;
                paramInputStreamLen = len;
            }

            void setSequence(const css::uno::Sequence< sal_Int8 >& _aSequence)
            {
                aSequence = _aSequence;
            }


            // getInputStream
            // Gets the input stream for the bound parameter

            const css::uno::Reference< css::io::XInputStream>& getInputStream ()
            {
                return paramInputStream;
            }


            // getInputStreamLen
            // Gets the input stream length for the bound parameter

            sal_Int32 getInputStreamLen ()
            {
                return paramInputStreamLen;
            }


        private:

            // Data attributes


            void  *binaryData;       // Storage area to be used
                                     // when binding the parameter

            SQLLEN paramLength;      // Storage area to be used
                                     // for the bound length of the
                                     // parameter.  Note that this
                                     // data is in native format.

            css::uno::Reference< css::io::XInputStream> paramInputStream;
            css::uno::Sequence< sal_Int8 > aSequence;
                                        // When an input stream is
                                        // bound to a parameter, a
                                        // reference to the input stream is saved
                                        // until not needed anymore.

            sal_Int32 paramInputStreamLen;                // Length of input stream
       };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OBOUNDPARAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
