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
#ifndef _CONNECTIVITY_OBOUNPARAM_HXX_
#define _CONNECTIVITY_OBOUNPARAM_HXX_

#include <com/sun/star/io/XInputStream.hpp>
#include "odbc/odbcbasedllapi.hxx"

namespace connectivity
{
    namespace odbc
    {
        class OOO_DLLPUBLIC_ODBCBASE OBoundParam
        {

        public:
            OBoundParam()
            {
                paramLength = NULL;
                binaryData  = NULL;
                pA1=0;
                pA2=0;
                pB1=0;
                pB2=0;
                pC1=0;
                pC2=0;
                pS1=0;
                pS2=0;
            }
            ~OBoundParam()
            {
                delete [] binaryData;
                delete [] paramLength;
            }
            //--------------------------------------------------------------------
            // initialize
            // Perform an necessary initialization
            //--------------------------------------------------------------------
            void initialize ()
            {
                // Allocate storage for the length.  Note - the length is
                // stored in native format, and will have to be converted
                // to a Java sal_Int32.  The jdbcodbc 'C' bridge provides an
                // interface to do this.

                paramLength = new sal_Int8[4];
            }

            //--------------------------------------------------------------------
            // allocBindDataBuffer
            // Allocates and returns a new bind data buffer of the specified
            // length
            //--------------------------------------------------------------------
            sal_Int8* allocBindDataBuffer (sal_Int32 bufLen)
            {
                if ( binaryData )
                    delete [] binaryData;
                binaryData = new sal_Int8[bufLen];

                // Reset the input stream, we are doing a new bind
                setInputStream (NULL, 0);

                return binaryData;
            }

            //--------------------------------------------------------------------
            // getBindDataBuffer
            // Returns the data buffer to be used when binding to a parameter
            //--------------------------------------------------------------------
            sal_Int8* getBindDataBuffer ()
            {
                return binaryData;
            }

            //--------------------------------------------------------------------
            // getBindLengthBuffer
            // Returns the length buffer to be used when binding to a parameter
            //--------------------------------------------------------------------
            sal_Int8* getBindLengthBuffer ()
            {
                return paramLength;
            }

            //--------------------------------------------------------------------
            // setInputStream
            // Sets the input stream for the bound parameter
            //--------------------------------------------------------------------
            void setInputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& inputStream,
                                sal_Int32 len)
            {
                paramInputStream = inputStream;
                paramInputStreamLen = len;
            }

            void setSequence(const ::com::sun::star::uno::Sequence< sal_Int8 >& _aSequence)
            {
                aSequence = _aSequence;
            }

            //--------------------------------------------------------------------
            // getInputStream
            // Gets the input stream for the bound parameter
            //--------------------------------------------------------------------
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> getInputStream ()
            {
                return paramInputStream;
            }

            //--------------------------------------------------------------------
            // getInputStreamLen
            // Gets the input stream length for the bound parameter
            //--------------------------------------------------------------------
            sal_Int32 getInputStreamLen ()
            {
                return paramInputStreamLen;
            }

            //--------------------------------------------------------------------
            // setSqlType
            // Sets the Java sql type used to register an OUT parameter
            //--------------------------------------------------------------------

            void setSqlType(sal_Int32 type)
            {
                sqlType = type;
            }

            //--------------------------------------------------------------------
            // getSqlType
            // Gets the Java sql type used to register an OUT parameter
            //--------------------------------------------------------------------

            sal_Int32 getSqlType ()
            {
                return sqlType;
            }

            //--------------------------------------------------------------------
            // setOutputParameter
            // Sets the flag indicating if this is an OUTPUT parameter
            //--------------------------------------------------------------------

            void setOutputParameter (sal_Bool output)
            {
                outputParameter = output;
            }

            //--------------------------------------------------------------------
            // isOutputParameter
            // Gets the OUTPUT parameter flag
            //--------------------------------------------------------------------

            sal_Bool isOutputParameter ()
            {
                return outputParameter;
            }

        protected:
            //====================================================================
            // Data attributes
            //====================================================================

            sal_Int8* binaryData;       // Storage area to be used
                                        // when binding the parameter

            sal_Int8* paramLength;      // Storage area to be used
                                        // for the bound length of the
                                        // parameter.  Note that this
                                        // data is in native format.

            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> paramInputStream;
            ::com::sun::star::uno::Sequence< sal_Int8 > aSequence;
                                        // When an input stream is
                                        // bound to a parameter, the
                                        // input stream is saved
                                        // until needed.

            sal_Int32 paramInputStreamLen;                // Length of input stream

            sal_Int32 sqlType;                          // Java SQL type used to
                                                            // register an OUT parameter

            sal_Bool outputParameter;   // true for OUTPUT parameters


            sal_Int32 pA1;              //pointers
            sal_Int32 pA2;
            sal_Int32 pB1;
            sal_Int32 pB2;
            sal_Int32 pC1;
            sal_Int32 pC2;
            sal_Int32 pS1;
            sal_Int32 pS2;// reserved for strings(UTFChars)
        };
    }
}
#endif // _CONNECTIVITY_OBOUNPARAM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
