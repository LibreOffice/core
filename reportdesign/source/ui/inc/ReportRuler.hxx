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


#ifndef RPTUI_RULER_HXX
#define RPTUI_RULER_HXX

#include <vcl/window.hxx>
#include <com/sun/star/report/XSection.hpp>

namespace rptui
{
    class OReportSection;
    class OReportWindow;
    class OReportRuler : public Window
    {
        OReportSection*     m_pSection;
        OReportWindow*      m_pParent;
        sal_Bool            m_bShow;
        OReportRuler(OReportRuler&);
        void operator =(OReportRuler&);
    public:
        OReportRuler(Window* _pParent,OReportWindow* _pReportWindowt,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportRuler();

        // windows overloads
        virtual void Resize();

        inline OReportSection*      getSection()        const { return m_pSection; }
        inline OReportWindow*       getView()           const { return m_pParent; }

        /** makes the grid visible
        *
        * \param _bVisible when <TRUE/> the grid is made visible
        */
        void SetGridVisible(sal_Bool _bVisible);
    };
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_RULER_HXX
