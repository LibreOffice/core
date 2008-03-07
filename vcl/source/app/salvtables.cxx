/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvtables.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:41:11 $
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
#include "precompiled_vcl.hxx"

#include <vcl/salframe.hxx>
#include <vcl/salinst.hxx>
#include <vcl/salvd.hxx>
#include <vcl/salprn.hxx>
#include <vcl/saltimer.hxx>
#include <vcl/salogl.hxx>
#include <vcl/salimestatus.hxx>
#include <vcl/salsys.hxx>
#include <vcl/salbmp.hxx>
#include <vcl/salobj.hxx>
#include <vcl/salmenu.hxx>
#include <vcl/salctrlhandle.hxx>

// this file contains the virtual destructors of the sal interface
// compilers ususally put their vtables where the destructor is

SalFrame::~SalFrame()
{
}

// -----------------------------------------------------------------------

// default to full-frame flushes
// on ports where partial-flushes are much cheaper this method should be overridden
void SalFrame::Flush( const Rectangle& )
{
    Flush();
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
}

SalTimer::~SalTimer()
{
}

SalOpenGL::~SalOpenGL()
{
}

SalBitmap::~SalBitmap()
{
}

SalI18NImeStatus::~SalI18NImeStatus()
{
}

SalSystem::~SalSystem()
{
}

SalPrinter::~SalPrinter()
{
}

BOOL SalPrinter::StartJob( const String*, const String&,
                           ImplJobSetup*, ImplQPrinter* )
{
    return FALSE;
}

SalInfoPrinter::~SalInfoPrinter()
{
}

SalVirtualDevice::~SalVirtualDevice()
{
}

SalObject::~SalObject()
{
}

SalMenu::~SalMenu()
{
}

SalMenuItem::~SalMenuItem()
{
}
SalControlHandle::~SalControlHandle()
{
}

