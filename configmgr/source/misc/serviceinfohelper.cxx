/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: serviceinfohelper.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "serviceinfohelper.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------

    sal_Int32 ServiceInfoHelper::countServices( ) const
    {
        if (m_info == 0)
            return 0;

        sal_Int32 nCount = 0;
        if (AsciiServiceName const* p= m_info->registeredServiceNames)
        {
            while (*p != 0)
            {
                ++nCount;
                ++p;
            }
        }
        if (AsciiServiceName const* p= m_info->additionalServiceNames)
        {
            while (*p != 0)
            {
                ++nCount;
                ++p;
            }
        }

        return nCount;
    }
// ---------------------------------------------------------------------------

    OUString ServiceInfoHelper::getImplementationName( ) const
        throw(uno::RuntimeException)
    {
        AsciiServiceName p= m_info ? m_info->implementationName : 0;

        return p ? OUString::createFromAscii(p) : OUString();
    }
// ---------------------------------------------------------------------------

    sal_Bool ServiceInfoHelper::supportsService( OUString const & ServiceName ) const
        throw(uno::RuntimeException)
    {
        if (m_info == 0)
            return false;

        if (AsciiServiceName const* p= m_info->registeredServiceNames)
        {
            while (*p != 0)
            {
                if (ServiceName.equalsAscii(*p))
                    return true;
                ++p;
            }
        }
        if (AsciiServiceName const* p= m_info->additionalServiceNames)
        {
            while (*p != 0)
            {
                if (ServiceName.equalsAscii(*p))
                    return true;
                ++p;
            }
        }

        return false;
    }
// ---------------------------------------------------------------------------

    uno::Sequence< OUString > ServiceInfoHelper::getSupportedServiceNames( ) const
        throw(uno::RuntimeException)
    {
        sal_Int32 const nCount = countServices();

        uno::Sequence< OUString > aServices( nCount );

        if (nCount)
        {
            OSL_ASSERT(m_info);
            sal_Int32 i = 0;
            if (AsciiServiceName const* p= m_info->registeredServiceNames)
            {
                while (*p != 0)
                {
                    aServices[i++] = OUString::createFromAscii(*p++);
                }
            }
            if (AsciiServiceName const* p= m_info->additionalServiceNames)
            {
                while (*p != 0)
                {
                    aServices[i++] = OUString::createFromAscii(*p++);
                }
            }
            OSL_ASSERT( i == nCount );
        }

        return aServices;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

    sal_Int32 ServiceRegistrationHelper::countServices( ) const
    {
        if (m_info == 0)
            return 0;

        sal_Int32 nCount = 0;
        if (AsciiServiceName const* p= m_info->registeredServiceNames)
        {
            while (*p != 0)
            {
                ++nCount;
                ++p;
            }
        }

        return nCount;
    }
// ---------------------------------------------------------------------------

    OUString ServiceRegistrationHelper::getImplementationName( ) const
        throw(uno::RuntimeException)
    {
        AsciiServiceName p= m_info ? m_info->implementationName : 0;

        return p ? OUString::createFromAscii(p) : OUString();
    }
// ---------------------------------------------------------------------------

    uno::Sequence< OUString > ServiceRegistrationHelper::getRegisteredServiceNames( ) const
        throw(uno::RuntimeException)
    {
        sal_Int32 const nCount = countServices();

        uno::Sequence< OUString > aServices( nCount );

        if (nCount)
        {
            OSL_ASSERT(m_info);
            sal_Int32 i = 0;
            if (AsciiServiceName const* p= m_info->registeredServiceNames)
            {
                while (*p != 0)
                {
                    aServices[i++] = OUString::createFromAscii(*p++);
                }
            }
            OSL_ASSERT( i == nCount );
        }

        return aServices;
    }
// ---------------------------------------------------------------------------
} // namespace config


