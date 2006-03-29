/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MErrorResource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-29 12:18:47 $
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

#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#define CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace connectivity
{
    namespace mozab
    {
        class ErrorResourceAccess
        {
        private:
            mutable sal_Int32               m_nErrorResourceId;

        protected:
            ErrorResourceAccess() : m_nErrorResourceId(0) { }

            inline void setError( sal_Int32 _nErrorResourceId ) const { const_cast< ErrorResourceAccess* >( this )->m_nErrorResourceId = _nErrorResourceId; }
            inline void resetError( ) const { const_cast< ErrorResourceAccess* >( this )->m_nErrorResourceId = 0; }
        public:
            inline sal_Int32    getErrorResourceId() const
                                { return m_nErrorResourceId; }
        };
    }
}

#endif // CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
