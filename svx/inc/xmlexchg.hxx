/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlexchg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:49:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_XMLEXCHG_HXX_
#define _SVX_XMLEXCHG_HXX_

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

