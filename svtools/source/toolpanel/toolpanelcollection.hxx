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



#ifndef TOOLPANELCOLLECTION_HXX
#define TOOLPANELCOLLECTION_HXX

#include "svtools/toolpanel/toolpaneldeck.hxx"

#include <memory>

//........................................................................
namespace svt
{
//........................................................................

    struct ToolPanelCollection_Data;

    //====================================================================
    //= ToolPanelCollection
    //====================================================================
    class ToolPanelCollection : public IToolPanelDeck
    {
    public:
        ToolPanelCollection();
        ~ToolPanelCollection();

        // IToolPanelDeck
        virtual size_t      GetPanelCount() const;
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const;
        virtual ::boost::optional< size_t >
                            GetActivePanel() const;
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel );
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual PToolPanel  RemovePanel( const size_t i_nPosition );
        virtual void        AddListener( IToolPanelDeckListener& i_rListener );
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener );

    private:
        ::std::auto_ptr< ToolPanelCollection_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // TOOLPANELCOLLECTION_HXX
