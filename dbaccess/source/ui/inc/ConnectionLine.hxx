/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionLine.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:14:37 $
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
#include <functional>

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
    class OTableWindow;
    class OConnectionLine
    {
        OTableConnection*       m_pTabConn;
        OConnectionLineDataRef  m_pData;

        Point                   m_aSourceConnPos,
                                m_aDestConnPos;
        Point                   m_aSourceDescrLinePos,
                                m_aDestDescrLinePos;
    public:
        OConnectionLine( OTableConnection* pConn, OConnectionLineDataRef pLineData );
        OConnectionLine( const OConnectionLine& rLine );
        virtual ~OConnectionLine();

        virtual OConnectionLine& operator=( const OConnectionLine& rLine );

        Rectangle           GetBoundingRect();
        BOOL                RecalcLine();
        void                Draw( OutputDevice* pOutDev );
        bool                CheckHit( const Point& rMousePos ) const;
        String              GetSourceFieldName() const { return m_pData->GetSourceFieldName(); }
        String              GetDestFieldName() const { return m_pData->GetDestFieldName(); }

        BOOL                IsValid() const;

        Rectangle           GetSourceTextPos() const;
        Rectangle           GetDestTextPos() const;

        OConnectionLineDataRef  GetData() const { return m_pData; }

        Point               getMidPoint() const;
    };
    /// unary_function Functor object for class OConnectionLine returntype is void
    /// draws a connectionline object on outputdevice
    struct TConnectionLineDrawFunctor : ::std::unary_function<OConnectionLine*,void>
    {
        OutputDevice* pDevice;
        TConnectionLineDrawFunctor(OutputDevice* _pDevice)
        {
            pDevice = _pDevice;
        }
        inline void operator()(OConnectionLine* _pLine)
        {
            _pLine->Draw(pDevice);
        }
    };
    /// binary_function Functor object for class OConnectionLine returntype is bool
    /// checks if the point is on connectionline
    struct TConnectionLineCheckHitFunctor : ::std::binary_function<OConnectionLine*,Point,bool>
    {
        inline bool operator()(const OConnectionLine* lhs,const Point& rhs) const
        {
            return lhs->CheckHit(rhs);
        }
    };

}
#endif // DBAUI_CONNECTIONLINE_HXX
