/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: serviceinfohelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:12:42 $
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


