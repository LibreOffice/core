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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>

#include "drawpage.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "pageuno.hxx"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

ScDrawPage::ScDrawPage(ScDrawLayer& rNewModel, StarBASIC* pBasic, sal_Bool bMasterPage) :
    FmFormPage(rNewModel, pBasic, bMasterPage)
{
    SetSize( Size( LONG_MAX, LONG_MAX ) );
}

// -----------------------------------------------------------------------

__EXPORT ScDrawPage::~ScDrawPage()
{
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ScDrawPage::createUnoPage()
{
    return static_cast<cppu::OWeakObject*>( new ScPageObj( this ) );
}


