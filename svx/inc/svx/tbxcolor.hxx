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


#ifndef _SVX_TBXCOLOR_HXX
#define _SVX_TBXCOLOR_HXX

#include <com/sun/star/frame/XLayoutManager.hpp>
#include "svx/svxdllapi.h"

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= ToolboxAccess
    //====================================================================
    class SVX_DLLPUBLIC ToolboxAccess
    {
    private:
        bool                                                    m_bDocking;
        ::rtl::OUString                                         m_sToolboxResName;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XLayoutManager >   m_xLayouter;

    public:
        ToolboxAccess( const ::rtl::OUString& rToolboxName );

    public:
        /** toggles the toolbox
        */
        void        toggleToolbox() const;

        /** determines whether the toolbox is currently visible
        */
        bool        isToolboxVisible() const;

        /** forces that the toolbox is docked
        */
        inline void forceDocking() { m_bDocking = true; }
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // #ifndef _SVX_TBXCOLOR_HXX

