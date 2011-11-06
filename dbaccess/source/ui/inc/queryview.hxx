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


#ifndef DBAUI_QUERYVIEW_HXX
#define DBAUI_QUERYVIEW_HXX

#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif

namespace dbaui
{
    class OQueryController;
    class OQueryView : public OJoinDesignView
    {
    public:
        OQueryView(Window* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OQueryView();

        virtual sal_Bool isCutAllowed()     = 0;
        virtual sal_Bool isPasteAllowed()   = 0;
        virtual sal_Bool isCopyAllowed()    = 0;
        virtual void copy()     = 0;
        virtual void cut()      = 0;
        virtual void paste()    = 0;
        // clears the whole query
        virtual void clear() = 0;
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly) = 0;
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement) = 0;
        // returns the current sql statement
        virtual ::rtl::OUString getStatement() = 0;
    };
}
#endif // DBAUI_QUERYVIEW_HXX
