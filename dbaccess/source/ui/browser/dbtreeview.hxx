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



#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#define DBACCESS_UI_DBTREEVIEW_HXX

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

class SvTreeListBox;
class SvLBoxTreeList;
namespace dbaui
{
    class DBTreeListBox;
    // ------------------
    // - DBTreeView -
    // ------------------

    class DBTreeView : public Window
    {
    private:
        DBTreeListBox*  m_pTreeListBox;
    protected:
        // window overridables
        virtual void Resize();
    public:

        DBTreeView( Window* pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                WinBits nBits );
        ~DBTreeView();

        /** sets a handler which is called when an list box entry is to be expanded.
            <p>When calling the link, the parameter is an SvLBoxEntry marking the entry to be expanded.
            </p>
        */
        void    SetPreExpandHandler(const Link& _rHdl);
        /// gets the currently set NodeExpansionHandler

        void    setCopyHandler(const Link& _rHdl);


        void                setModel(SvLBoxTreeList* _pTreeModel);
        void                setSelChangeHdl(const Link& _rHdl);

        DBTreeListBox&      getListBox() const { return *m_pTreeListBox; }

        virtual void GetFocus();
    };
}

#endif // DBACCESS_UI_DBTREEVIEW_HXX

