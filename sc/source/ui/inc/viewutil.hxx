/*************************************************************************
 *
 *  $RCSfile: viewutil.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
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

#ifndef SC_VIEWUTIL_HXX
#define SC_VIEWUTIL_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class String;
class SvxFontItem;

class ScChangeAction;
class ScChangeViewSettings;
class ScDocument;


enum ScUpdateMode { SC_UPDATE_ALL, SC_UPDATE_CHANGED, SC_UPDATE_MARKS };


// ---------------------------------------------------------------------------

class ScViewUtil                                // static Methoden
{
public:
    static BOOL ExecuteCharMap( const SvxFontItem&  rOldFont,
                                SvxFontItem&        rNewFont,
                                String&             rString );

    static BOOL IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument );
};

// ---------------------------------------------------------------------------

class ScUpdateRect
{
private:
    USHORT  nOldStartX;
    USHORT  nOldStartY;
    USHORT  nOldEndX;
    USHORT  nOldEndY;
    USHORT  nNewStartX;
    USHORT  nNewStartY;
    USHORT  nNewEndX;
    USHORT  nNewEndY;
    USHORT  nContX1;
    USHORT  nContY1;
    USHORT  nContX2;
    USHORT  nContY2;
public:
            ScUpdateRect( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 );
    void    SetNew( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 );
    BOOL    GetDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 );
    BOOL    GetXorDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, BOOL& rCont );
    void    GetContDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 );
};




#endif

