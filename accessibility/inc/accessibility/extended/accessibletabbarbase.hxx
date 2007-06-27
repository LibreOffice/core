/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletabbarbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:20:36 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX

#ifndef COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX
#include <comphelper/accessiblecomponenthelper.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class TabBar;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;

//.........................................................................
namespace accessibility
{
//.........................................................................

// ============================================================================

typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleExtendedComponentHelper_BASE;

class AccessibleTabBarBase : public AccessibleExtendedComponentHelper_BASE
{
public:
    explicit            AccessibleTabBarBase( TabBar* pTabBar );
    virtual             ~AccessibleTabBarBase();

protected:
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    virtual void        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing();

private:
    void                SetTabBarPointer( TabBar* pTabBar );
    void                ClearTabBarPointer();

protected:
    VCLExternalSolarLock* m_pExternalLock;
    TabBar*             m_pTabBar;
};

// ============================================================================

//.........................................................................
}   // namespace accessibility
//.........................................................................

#endif

