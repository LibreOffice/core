/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
        sal_Bool                RecalcLine();
        void                Draw( OutputDevice* pOutDev );
        bool                CheckHit( const Point& rMousePos ) const;
        String              GetSourceFieldName() const { return m_pData->GetSourceFieldName(); }
        String              GetDestFieldName() const { return m_pData->GetDestFieldName(); }

        sal_Bool                IsValid() const;

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
