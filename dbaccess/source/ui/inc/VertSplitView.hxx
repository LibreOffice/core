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


#ifndef DBAUI_VERTSPLITVIEW_HXX
#define DBAUI_VERTSPLITVIEW_HXX

#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif

namespace dbaui
{
    //==================================================================
    class OSplitterView : public Window
    {
        Splitter*                           m_pSplitter;
        Window*                             m_pLeft;
        Window*                             m_pRight;
        sal_Bool                            m_bVertical;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OSplitterView(Window* _pParent,sal_Bool _bVertical = sal_True);
        virtual ~OSplitterView();
        // window overloads
        virtual void GetFocus();

        void setSplitter(Splitter* _pSplitter);
        void set(Window* _pRight,Window* _pLeft = NULL);
        virtual void Resize();
    };
}
#endif // DBAUI_VERTSPLITVIEW_HXX

