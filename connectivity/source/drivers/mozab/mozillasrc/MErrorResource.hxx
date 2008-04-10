/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MErrorResource.hxx,v $
 * $Revision: 1.3 $
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

#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#define CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX

#include <sal/types.h>

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
