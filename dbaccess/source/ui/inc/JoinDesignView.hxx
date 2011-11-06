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



#ifndef DBAUI_JOINDESIGNVIEW_HXX
#define DBAUI_JOINDESIGNVIEW_HXX

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

#include <memory>

namespace dbaui
{
    class OTableConnection;
    class OConnectionLineData;
    class OJoinController;
    class OScrollWindowHelper;
    class OJoinTableView;
    class OTableWindow;

    class OJoinDesignView : public ODataView
    {
    protected:
        OScrollWindowHelper*    m_pScrollWindow;    // contains only the scrollbars
        OJoinTableView*         m_pTableView;       // presents the upper window
        OJoinController&        m_rController;

    public:
        OJoinDesignView(Window* pParent,
                        OJoinController& _rController,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OJoinDesignView();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // set the statement for representation
        /// late construction
        virtual void Construct();
        virtual void initialize();
        virtual void KeyInput( const KeyEvent& rEvt );

        virtual void SaveTabWinUIConfig(OTableWindow* pWin);
        OJoinController& getController() const { return m_rController; }
        // called when fields are deleted

        OJoinTableView* getTableView() const { return m_pTableView; }
        OScrollWindowHelper* getScrollHelper() const { return m_pScrollWindow; }
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);
        DECL_LINK( SplitHdl, void* );
    };
}
#endif // DBAUI_JOINDESIGNVIEW_HXX



