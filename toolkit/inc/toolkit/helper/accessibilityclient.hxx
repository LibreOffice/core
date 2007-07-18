/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibilityclient.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 10:05:58 $
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

#ifndef TOOLKIT_HELPER_ACCESSIBILITY_CLIENT_HXX
#define TOOLKIT_HELPER_ACCESSIBILITY_CLIENT_HXX

#ifndef TOOLKIT_HELPER_ACCESSIBLE_FACTORY_HXX
#include <toolkit/helper/accessiblefactory.hxx>
#endif

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

