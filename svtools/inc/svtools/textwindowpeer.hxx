/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textwindowpeer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 10:03:58 $
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

#if !defined INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
#define INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

#include <memory>

class TextEngine;
class TextView;

namespace svt
{
    class AccessibleFactoryAccess ;

class TextWindowPeer: public ::VCLXWindow
{
public:
    SVT_DLLPUBLIC TextWindowPeer(::TextView & rView, bool bCompoundControlChild = false);

    virtual ~TextWindowPeer();

private:
    // VCLXWindow inherits funny copy constructor and assignment operator from
    // ::cppu::OWeakObject, so override them here:

    TextWindowPeer(TextWindowPeer &); // not implemented

    void operator =(TextWindowPeer); // not implemented

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext >
    CreateAccessibleContext();

    ::TextEngine & m_rEngine;
    ::TextView & m_rView;
    bool m_bCompoundControlChild;

    ::std::auto_ptr< ::svt::AccessibleFactoryAccess  >  m_pFactoryAccess;
};

}

#endif // INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
