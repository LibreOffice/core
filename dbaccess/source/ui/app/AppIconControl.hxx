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


#ifndef DBAUI_APPICONCONTROL_HXX
#define DBAUI_APPICONCONTROL_HXX

#ifndef _ICNVW_HXX
#include <svtools/ivctrl.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif


namespace dbaui
{
    class IControlActionListener;
    //==================================================================
    class OApplicationIconControl   :public SvtIconChoiceCtrl
                                    ,public DropTargetHelper
    {
        Point                       m_aMousePos;
        IControlActionListener*     m_pActionListener;

    public:
        OApplicationIconControl(Window* _pParent);
        virtual ~OApplicationIconControl();

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        IControlActionListener* getControlActionListener( ) const { return m_pActionListener; }

    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );
    };
}
#endif // DBAUI_APPICONCONTROL_HXX

