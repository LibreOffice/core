/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleLayoutWrapper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:41:26 $
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
#include "precompiled_sd.hxx"

#include "SimpleLayoutWrapper.hxx"

#include <vcl/window.hxx>

namespace sd { namespace toolpanel {

SimpleLayoutWrapper::SimpleLayoutWrapper (
    ::std::auto_ptr< ::Window> pWindow,
    int nPreferredWidth,
    int nPreferredHeight,
    bool bIsResizable)
    : mpWindow (pWindow),
      mnPreferredWidth(nPreferredWidth),
      mnPreferredHeight(nPreferredHeight),
      mbIsResizable (bIsResizable)
{
}




SimpleLayoutWrapper::~SimpleLayoutWrapper (void)
{
}




Size SimpleLayoutWrapper::GetPreferredSize (void)
{
    return Size (mnPreferredWidth, mnPreferredHeight);
}




int SimpleLayoutWrapper::GetPreferredWidth (int )
{
    return mnPreferredWidth;
}




int SimpleLayoutWrapper::GetPreferredHeight (int )
{
    return mnPreferredHeight;
}




bool SimpleLayoutWrapper::IsResizable (void)
{
    return mbIsResizable;
}




::Window* SimpleLayoutWrapper::GetWindow (void)
{
    return mpWindow.get();
}


} } // end of namespace ::sd::toolpanel

