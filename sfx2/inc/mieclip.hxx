/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mieclip.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:03:19 $
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

#ifndef _MIECLIP_HXX
#define _MIECLIP_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SvStream;
class SotDataObject;
class SvData;

class SFX2_DLLPUBLIC MSE40HTMLClipFormatObj
{
    SvStream* pStrm;
    String sBaseURL;

public:
    MSE40HTMLClipFormatObj() : pStrm(0)         {}
    ~MSE40HTMLClipFormatObj();

//JP 31.01.2001: old interfaces
    SAL_DLLPRIVATE BOOL GetData( SotDataObject& );
    SAL_DLLPRIVATE BOOL GetData( SvData& );
//JP 31.01.2001: the new one
    SvStream* IsValid( SvStream& );

    const SvStream* GetStream() const       { return pStrm; }
          SvStream* GetStream()             { return pStrm; }
    const String& GetBaseURL() const        { return sBaseURL; }
};


#endif //_MIECLIP_HXX

