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



#ifndef __FRAMEWORK_LAYOUTMANAGER_PANELWINDOW_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_PANELWINDOW_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <vcl/dockwin.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

class PanelWindow : public DockingWindow
{
    public:
                        PanelWindow( Window* pParent, WinBits nWinBits  =0);
        virtual         ~PanelWindow();

        const ::rtl::OUString& getResourceURL() const;
        void            setResourceURL(const ::rtl::OUString& rResourceURL);
        Window*         getContentWindow() const;
        void            setContentWindow( Window* pContentWindow );

        virtual void    Command ( const CommandEvent& rCEvt );
        virtual void    StateChanged( StateChangedType nType );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
        virtual void    Resize();

        // Provide additional handlers to support external implementations
        void            SetCommandHdl( const Link& aLink ) { m_aCommandHandler = aLink; }
        const Link&     GetCommandHdl() const { return m_aCommandHandler; }
        void            SetStateChangedHdl( const Link& aLink ) { m_aStateChangedHandler = aLink; }
        const Link&     GetStateChangedHdl() const { return m_aStateChangedHandler; }
        void            SetDataChangedHdl( const Link& aLink ) { m_aDataChangedHandler = aLink; }
        const Link&     GetDataChangedHdl() { return m_aDataChangedHandler; }

    private:
        ::rtl::OUString m_aResourceURL;
        Link            m_aCommandHandler;
        Link            m_aStateChangedHandler;
        Link            m_aDataChangedHandler;
        Window*         m_pContentWindow;
};

}

#endif // __FRAMEWORK_UIELEMENT_PANELWINDOW_HXX_
