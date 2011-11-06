/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _PPTTOXML_HXX
#define _PPTTOXML_HXX

#include "pptcom.hxx"

// ------------
// - PptToXml -
// ------------

class PptToXml
{
        REF( NMSP_SAX::XDocumentHandler ) xHdl;

    public:

        PptToXml();
        ~PptToXml();

        sal_Bool filter( const SEQ( NMSP_BEANS::PropertyValue )& aDescriptor,
                            REF(NMSP_SAX::XDocumentHandler) xHandler );
        void cancel();
};

#endif
