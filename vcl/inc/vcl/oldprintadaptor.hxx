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



#ifndef _VCL_OLDPRINTADAPTOR
#define _VCL_OLDPRINTADAPTOR

#include "vcl/print.hxx"

namespace vcl
{
    struct ImplOldStyleAdaptorData;
    class VCL_DLLPUBLIC OldStylePrintAdaptor : public PrinterController
    {
        ImplOldStyleAdaptorData*    mpData;
    public:
        OldStylePrintAdaptor( const boost::shared_ptr< Printer >& );
        virtual ~OldStylePrintAdaptor();

        void StartPage();
        void EndPage();

        virtual int  getPageCount() const;
        virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > getPageParameters( int i_nPage ) const;
        virtual void printPage( int i_nPage ) const;
    };
}

#endif
