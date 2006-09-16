/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ole2uno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:03:53 $
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
#include "precompiled_extensions.hxx"
#include "osl/getglobalmutex.hxx"
#include "rtl/instance.hxx"
#include "ole2uno.hxx"

using namespace osl;
namespace ole_adapter
{

struct MutexInit
{
    Mutex * operator () ()
    {
        static Mutex aInstance;
        return &aInstance;
    }
};


Mutex * getBridgeMutex()
{
    return rtl_Instance< Mutex, MutexInit, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            MutexInit(), ::osl::GetGlobalMutex());
}


// Mutex* getBridgeMutex()
// {
//  static Mutex* pMutex= NULL;

//  if( ! pMutex)
//  {
//      MutexGuard guard( Mutex::getGlobalMutex() );
//      if( !pMutex)
//      {
//          static Mutex aMutex;
//          pMutex= &aMutex;
//      }
//  }
//  return pMutex;
// }

}
