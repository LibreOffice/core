/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibilityclient.hxx,v $
 * $Revision: 1.4 $
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

#ifndef TOOLKIT_HELPER_ACCESSIBILITY_CLIENT_HXX
#define TOOLKIT_HELPER_ACCESSIBILITY_CLIENT_HXX

#include <toolkit/helper/accessiblefactory.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= AccessibilityClient
    //====================================================================
    /** a client for the accessibility implementations which have been
        outsourced from the main toolkit library

        All instances of this class share a reference to a common IAccessibleFactory
        instance, which is used for creating all kind of Accessibility related
        components.

        When the AccessibilityClient goes aways, this factory goes aways, to, and the respective
        library is unloaded.

        This class is not thread-safe.
    */
    class AccessibilityClient
    {
    private:
        bool    m_bInitialized;

    public:
        AccessibilityClient();
        ~AccessibilityClient();

        IAccessibleFactory& getFactory();

    private:
        void ensureInitialized();
    };

//........................................................................
}   // namespace toolkit
//........................................................................

#endif // TOOLKIT_HELPER_ACCESSIBILITY_CLIENT_HXX

