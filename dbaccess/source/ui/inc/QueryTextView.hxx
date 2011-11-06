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


#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#define DBAUI_QUERYVIEW_TEXT_HXX

#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#include "querycontainerwindow.hxx"
#endif

class Splitter;
namespace dbaui
{
    class OSqlEdit;
    class OQueryTextView : public Window
    {
        friend class OQueryViewSwitch;
        OSqlEdit*   m_pEdit;
    public:
        OQueryTextView( OQueryContainerWindow* pParent );
        virtual ~OQueryTextView();

        virtual void GetFocus();

        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // check if the statement is correct when not returning false
        virtual sal_Bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement);
        virtual ::rtl::OUString getStatement();
        // allow access to our edit
        OSqlEdit* getSqlEdit() const { return m_pEdit; }

        OQueryContainerWindow*  getContainerWindow() { return static_cast< OQueryContainerWindow* >( GetParent() ); }
    protected:
        virtual void Resize();
    };
}
#endif // DBAUI_QUERYVIEW_TEXT_HXX


