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


#ifndef DBAUI_SCROLLHELPER_HXX
#define DBAUI_SCROLLHELPER_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

namespace dbaui
{
    class OScrollHelper
    {
        Link    m_aUpScroll;
        Link    m_aDownScroll;
    public:
        /** default constructor
        */
        OScrollHelper();

        ~OScrollHelper();

        /** set the memthod which should be called when scrolling up
            @param  _rUpScroll
                the method to set
        */
        inline void setUpScrollMethod( const Link& _rUpScroll )
        {
            m_aUpScroll = _rUpScroll;
        }

        /** set the memthod which should be called when scrolling down
            @param  _rDownScroll
                the method to set
        */
        inline void setDownScrollMethod( const Link& _rDownScroll )
        {
            m_aDownScroll = _rDownScroll;
        }

        /** check if a scroll method has to be called
            @param  _rPoint
                the current selection point
            @param  _rOutputSize
                the output size of the window
        */
        void scroll(const Point& _rPoint, const Size& _rOutputSize);
    };
}
#endif // DBAUI_SCROLLHELPER_HXX

