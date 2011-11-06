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


#ifndef DBAUI_RELTABLEWINDOW_HXX
#define DBAUI_RELTABLEWINDOW_HXX

#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif

namespace dbaui
{
    class ORelationTableWindow : public OTableWindow
    {
    public:
        ORelationTableWindow( Window* pParent,const TTableWindowData::value_type& pTabWinData)
            : OTableWindow(pParent, pTabWinData) {}

        /** returns the name which should be used when displaying join or relations
            @return
                The composed name or the window name.
        */
        virtual ::rtl::OUString GetName() const { return GetComposedName(); }
    };
}
#endif //DBAUI_RELTABLEWINDOW_HXX


