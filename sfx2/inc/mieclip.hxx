/*************************************************************************
 *
 *  $RCSfile: mieclip.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MIECLIP_HXX
#define _MIECLIP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SvStream;
class SotDataObject;
class SvData;

class MSE40HTMLClipFormatObj
{
    SvStream* pStrm;
    String sBaseURL;

public:
    MSE40HTMLClipFormatObj() : pStrm(0)         {}
    ~MSE40HTMLClipFormatObj();

    BOOL GetData( SotDataObject& );
    BOOL GetData( SvData& );

    const SvStream* GetStream() const       { return pStrm; }
          SvStream* GetStream()             { return pStrm; }
    const String& GetBaseURL() const        { return sBaseURL; }
};


////////////////////////////////////////////////////////////////////////////////
//
/*
   $Log: not supported by cvs2svn $
   Revision 1.8  2000/09/17 16:47:07  willem.vandorp
   OpenOffice header added.

   Revision 1.7  2000/09/06 14:49:36  willem.vandorp
   Header and footer replaced.

   Revision 1.6  1999/12/16 19:32:37  er
   #60614# add: GetData with SvData

   Revision 1.5  1999/07/23 08:34:06  hr
   #65293#: misplaced comment

   Revision 1.4  1999/01/18 13:35:50  JP
   Task #59398#: unnoetiges RegisterClipb... gegen IDS ausgetauscht


      Rev 1.3   18 Jan 1999 14:35:50   JP
   Task #59398#: unnoetiges RegisterClipb... gegen IDS ausgetauscht

      Rev 1.2   22 Jun 1998 21:20:36   JP
   SvDataObject gegen SorDataObject ausgetauscht

      Rev 1.1   23 Feb 1998 17:05:14   TJ
   include

      Rev 1.0   18 Feb 1998 17:28:26   OK
   NEW: MSE40HTMLClipFormatObj

*/

#endif //_MIECLIP_HXX

