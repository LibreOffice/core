/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textwindowpeer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-05 17:38:34 $
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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/textwindowpeer.hxx>

#ifndef _TEXTVIEW_HXX
#include <svtools/textview.hxx>
#endif

#ifndef SVTOOLS_ACCESSIBLE_FACTORY_ACCESS_HXX
#include "svtaccessiblefactory.hxx"
#endif

namespace css = ::com::sun::star;

namespace svt
{
    TextWindowPeer::TextWindowPeer(::TextView & rView, bool bCompoundControlChild):
        m_rEngine(*rView.GetTextEngine()), m_rView(rView), m_bCompoundControlChild(bCompoundControlChild)
    {
        SetWindow(rView.GetWindow());
        m_pFactoryAccess.reset( new AccessibleFactoryAccess );
    }

    // virtual
    TextWindowPeer::~TextWindowPeer()
    {
    }

    ::css::uno::Reference< ::css::accessibility::XAccessibleContext > TextWindowPeer::CreateAccessibleContext()
    {
        return m_pFactoryAccess->getFactory().createAccessibleTextWindowContext(
            this, m_rEngine, m_rView, m_bCompoundControlChild
        );
    }
}
