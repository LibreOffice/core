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
#include "precompiled_svtools.hxx"
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/color.hxx>

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/gradient.hxx>
#include <vcl/cvtgrf.hxx>

#include <svtools/wallitem.hxx>
#include <svl/cntwall.hxx>

// -----------------------------------------------------------------------

//static
void SfxBrushItemLink::Set( SfxBrushItemLink* pLink )
{
    SfxBrushItemLink** ppLink =  (SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM);
    if( !*ppLink )
        *ppLink = pLink;
    else
        delete pLink;
}

