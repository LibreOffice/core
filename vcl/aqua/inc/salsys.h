/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsys.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:11:48 $
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

#ifndef _SV_SALSYS_H
#define _SV_SALSYS_H

#include "vcl/sv.h"
#include "vcl/salsys.hxx"

#include <list>

// -----------------
// - SalSystemData -
// -----------------

//struct SalSystemData
//{
//};

class VCL_DLLPUBLIC AquaSalSystem : public SalSystem
{
public:
    AquaSalSystem() {}
    virtual ~AquaSalSystem();

    // get info about the display
    virtual unsigned int GetDisplayScreenCount();
    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );

    virtual rtl::OUString GetScreenName( unsigned int nScreen );
    // overload pure virtual methods
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
