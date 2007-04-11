/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:58 $
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
#ifndef INCLUDED_SVTOOLS_DEFAULTOPTIONS_HXX
#define INCLUDED_SVTOOLS_DEFAULTOPTIONS_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

class String;
class SvtDefaultOptions_Impl;

// class SvtDefaultOptions -----------------------------------------------

class SvtDefaultOptions_Impl;
class SVL_DLLPUBLIC SvtDefaultOptions: public svt::detail::Options
{
private:
    SvtDefaultOptions_Impl* pImp;

public:

    SvtDefaultOptions();
    virtual ~SvtDefaultOptions();

    String  GetDefaultPath( USHORT nId ) const;
};

#endif // #ifndef INCLUDED_SVTOOLS_DEFAULTOPTIONS_HXX

