/*************************************************************************
 *
 *  $RCSfile: OBoundParam.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
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
#ifndef _CONNECTIVITY_OBOUNPARAM_HXX_
#define _CONNECTIVITY_OBOUNPARAM_HXX_

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

namespace connectivity
{
    namespace odbc
    {
        class OBoundParam
        {

        public:
            OBoundParam()
            {
                pA1=0;
                pA2=0;
                pB1=0;
                pB2=0;
                pC1=0;
                pC2=0;
                pS1=0;
                pS2=0;
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
            // setStreamType
            // Sets the input stream type used to register an OUT parameter
            //--------------------------------------------------------------------

            void setStreamType (sal_Int32 type)
            {
                streamType = type;
            }

            //--------------------------------------------------------------------
            // getStreamType
            // Gets the input stream type used to register an OUT parameter
            //--------------------------------------------------------------------

            sal_Int32 getStreamType ()
            {
                return streamType;
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
                                        // When an input stream is
                                        // bound to a parameter, the
                                        // input stream is saved
                                        // until needed.

            sal_Int32 paramInputStreamLen;                // Length of input stream

            sal_Int32 sqlType;                          // Java SQL type used to
                                                            // register an OUT parameter

            sal_Int32 streamType;                  // Input stream type
                                        // (ASCII, BINARY, UNICODE)


            sal_Bool outputParameter;   // true for OUTPUT parameters


            sal_Int32 pA1;              //pointers
            sal_Int32 pA2;
            sal_Int32 pB1;
            sal_Int32 pB2;
            sal_Int32 pC1;
            sal_Int32 pC2;
            sal_Int32 pS1;
            sal_Int32 pS2;// reserved for strings(UTFChars)

        public:
            static int ASCII;
            static int UNICODE;
            static int BINARY;
        };
    }
}
#endif // _CONNECTIVITY_OBOUNPARAM_HXX_

