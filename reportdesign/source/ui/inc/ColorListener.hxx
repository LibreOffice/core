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


#ifndef RPTUI_COLORLISTENER_HXX
#define RPTUI_COLORLISTENER_HXX

#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include "ModuleHelper.hxx"
#include <tools/link.hxx>

namespace rptui
{
    class OColorListener : public Window, public SfxListener
    {
        OColorListener(const OColorListener&);
        void operator =(const OColorListener&);
    protected:
        OModuleClient                       m_aModuleClient;
        Link                                m_aCollapsedLink;
        svtools::ColorConfig                m_aColorConfig;
        svtools::ExtendedColorConfig        m_aExtendedColorConfig;
        ::rtl::OUString                     m_sColorEntry;
        sal_Int32                           m_nColor;
        sal_Int32                           m_nTextBoundaries;
        sal_Bool                            m_bCollapsed;
        sal_Bool                            m_bMarked;

        virtual void ImplInitSettings() = 0;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
    public:
        OColorListener(Window* _pParent,const ::rtl::OUString& _sColorEntry);
        virtual ~OColorListener();

        using Window::Notify;
        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

        /** set the marker as marked or not marked
            @param  _bMark  set the new state of the marker
        */
        void    setMarked(sal_Bool _bMark);

        /** returns if the section is marked
        */
        inline sal_Bool isMarked() const { return m_bMarked; }

        inline void     setCollapsedHdl(const Link& _aLink ){ m_aCollapsedLink = _aLink; }
        inline sal_Bool isCollapsed() const { return m_bCollapsed; }

        /** collapse or expand
         *
         * \param _bCollapsed
         */
        virtual void    setCollapsed(sal_Bool _bCollapsed);
    };
}
#endif // RPTUI_COLORLISTENER_HXX

