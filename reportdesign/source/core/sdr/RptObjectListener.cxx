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


#include "precompiled_reportdesign.hxx"
#include "RptObjectListener.hxx"
#include "RptObject.hxx"
#include "RptDef.hxx"

namespace rptui
{
//============================================================================
// OObjectListener
//============================================================================

//----------------------------------------------------------------------------
DBG_NAME(rpt_OObjectListener)
OObjectListener::OObjectListener(OObjectBase* _pObject)
          :m_pObject(_pObject)
{
    DBG_CTOR(rpt_OObjectListener,NULL);
}

//----------------------------------------------------------------------------

OObjectListener::~OObjectListener()
{
    DBG_DTOR(rpt_OObjectListener,NULL);
}

// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL OObjectListener::disposing( const  ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException)
{
    /*
    // disconnect the listener
    if (m_pObject)
    {
        (m_pObject->m_xPropertyChangeListener).clear();
    }
    */
}

// XPropertyChangeListener
//----------------------------------------------------------------------------

void SAL_CALL OObjectListener::propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    m_pObject->_propertyChange( evt );
}

//----------------------------------------------------------------------------

//============================================================================
// DlgEdHint
//============================================================================

TYPEINIT1( DlgEdHint, SfxHint );

//----------------------------------------------------------------------------

DlgEdHint::DlgEdHint( DlgEdHintKind eHint )
    :eHintKind( eHint )
{
}

//----------------------------------------------------------------------------
DlgEdHint::~DlgEdHint()
{
}
//----------------------------------------------------------------------------
}
