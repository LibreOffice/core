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



#ifndef _SVX_XMLEXCHG_HXX_
#define _SVX_XMLEXCHG_HXX_

#include <svtools/transfer.hxx>
#include "svx/svxdllapi.h"

namespace com {
    namespace sun {
        namespace star {
            namespace beans {
                class SAL_NO_VTABLE XPropertySet;
            }
        }
    }
}

//........................................................................
namespace svx
{
//........................................................................


    //====================================================================
    //= OXFormsDescriptor
    //====================================================================

    struct SVX_DLLPUBLIC OXFormsDescriptor {

        String szName;
        String szServiceName;
        ::com::sun::star::uno::Reference
            < ::com::sun::star::beans::XPropertySet >
                xPropSet;

        inline OXFormsDescriptor( void ) {}
        inline OXFormsDescriptor( const OXFormsDescriptor &rhs ) { *this=rhs; }
        inline OXFormsDescriptor &operator = ( const OXFormsDescriptor &rhs ) {
            szName = rhs.szName;
            szServiceName = rhs.szServiceName;
            xPropSet = rhs.xPropSet;
            return (*this);
        }
    };

    //====================================================================
    //= OXFormsTransferable
    //====================================================================
    class SVX_DLLPUBLIC OXFormsTransferable : public TransferableHelper {

        protected:

            // TransferableHelper overridables
            virtual void        AddSupportedFormats();
            virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

            static sal_uInt32   getDescriptorFormatId();

            OXFormsDescriptor m_aDescriptor;

        public:

            /** construct the transferable
            */
            OXFormsTransferable( const OXFormsDescriptor &rhs );

            /** extracts an xform descriptor from the transferable given
            */
            static const OXFormsDescriptor &extractDescriptor( const TransferableDataHelper& _rData );
    };


//........................................................................
}   // namespace svx
//........................................................................

#endif // _SVX_XMLEXCHG_HXX_

