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

