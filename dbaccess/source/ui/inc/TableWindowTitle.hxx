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


#ifndef DBAUI_TABLEWINDOWTITLE_HXX
#define DBAUI_TABLEWINDOWTITLE_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

namespace dbaui
{
    class OTableWindow;
    class OTableWindowTitle : public FixedText
    {
        OTableWindow* m_pTabWin;

    protected:
        virtual void Command(const CommandEvent& rEvt);
        //  virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonDown( const MouseEvent& rEvt );
        virtual void KeyInput( const KeyEvent& rEvt );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

    public:
        OTableWindowTitle( OTableWindow* pParent );
        virtual ~OTableWindowTitle();
        virtual void LoseFocus();
        virtual void GetFocus();
        virtual void RequestHelp( const HelpEvent& rHEvt );
        // window override
        virtual void StateChanged( StateChangedType nStateChange );
    };
}
#endif // DBAUI_TABLEWINDOWTITLE_HXX

