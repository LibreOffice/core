/*************************************************************************
 *
 *  $RCSfile: ConnectionLine.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:06:26 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_CONNECTIONLINE_HXX
#define DBAUI_CONNECTIONLINE_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif

class SvLBoxEntry;
class OutputDevice;
namespace dbaui
{

    //==================================================================
    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine
    //==================================================================

    /*
        the class OConnectionLine represents the graphical line between the to two windows
    **/
    class OConnectionLineData;
    class OTableConnection;
    class OConnectionLine
    {
        OTableConnection*       m_pTabConn;
        OConnectionLineData*    m_pData;

        SvLBoxEntry*            m_pSourceEntry;
        SvLBoxEntry*            m_pDestEntry;

        Point                   m_aSourceConnPos,
                                m_aDestConnPos;
        Point                   m_aSourceDescrLinePos,
                                m_aDestDescrLinePos;

    public:
        OConnectionLine( OTableConnection* pConn, const String& rSourceFieldName = String(),
                         const String& rDestFieldName = String() );
        OConnectionLine( OTableConnection* pConn, OConnectionLineData* pLineData );
        OConnectionLine( const OConnectionLine& rLine );
        virtual ~OConnectionLine();

        virtual OConnectionLine& operator=( const OConnectionLine& rLine );

        Rectangle           GetBoundingRect();
        BOOL                RecalcLine();
        void                Draw( OutputDevice* pOutDev );
        BOOL                CheckHit( const Point& rMousePos );
        String              GetSourceFieldName() const { return m_pData->GetSourceFieldName(); }
        String              GetDestFieldName() const { return m_pData->GetDestFieldName(); }

        void                SetSourceFieldName( const String& rSourceFieldName );
        void                SetDestFieldName( const String& rDestFieldName );
        BOOL                Connect( const String& rSourceFieldName, const String& rDestFieldName );
        BOOL                IsValid();

        Rectangle           GetSourceTextPos() const;
        Rectangle           GetDestTextPos() const;

        OConnectionLineData*    GetData() const { return m_pData; }
    };
}
#endif // DBAUI_CONNECTIONLINE_HXX
