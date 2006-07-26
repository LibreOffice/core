/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsys.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 09:11:32 $
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

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SV_SALSYS_H
#define _SV_SALSYS_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _VCL_DLLAPI_H
#include "dllapi.h"
#endif

#include <list>

class VCL_DLLPUBLIC X11SalSystem : public SalSystem
{
public:
    X11SalSystem() {}
    virtual ~X11SalSystem();

    // overload pure virtual methods
    virtual unsigned int GetDisplayScreenCount();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual int ShowNativeDialog( const String& rTitle,
                                  const String& rMessage,
                                  const std::list< String >& rButtons,
                                  int nDefButton );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};

#endif // _SV_SALSYS_H
