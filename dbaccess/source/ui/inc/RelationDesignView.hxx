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



#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#define DBAUI_RELATIONDESIGNVIEW_HXX

#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
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
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif

namespace dbaui
{
    class OAddTableDlg;
    class OTableConnection;
    class ORelationTableConnectionData;
    class OConnectionLineData;
    class ORelationController;

    class ORelationDesignView : public OJoinDesignView
    {
    public:
        ORelationDesignView(Window* pParent, ORelationController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~ORelationDesignView();

        // set the statement for representation
        /// late construction
        virtual void Construct();
        virtual void initialize();


        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();
    };
}
#endif // DBAUI_RELATIONDESIGNVIEW_HXX



