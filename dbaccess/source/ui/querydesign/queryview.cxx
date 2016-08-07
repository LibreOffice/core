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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
DBG_NAME(OQueryView)
// -------------------------------------------------------------------------
OQueryView::OQueryView(Window* _pParent, OQueryController& _rController,const Reference< XMultiServiceFactory >& _rFactory)
    :OJoinDesignView( _pParent, _rController, _rFactory )
{
    DBG_CTOR(OQueryView,NULL);

}
// -----------------------------------------------------------------------------
OQueryView::~OQueryView()
{

    DBG_DTOR(OQueryView,NULL);
}
// -----------------------------------------------------------------------------




