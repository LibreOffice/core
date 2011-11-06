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



#ifndef BIBTOOLS_HXX

#include <vcl/taskpanelist.hxx>     // includes also vcl/window.hxx

namespace bib
{
    // source in bibbeam.cxx

    void HandleTaskPaneList( Window* pWindow, sal_Bool bAddToList );
        // pWindow: just an system window or something wich is child of a system window

    inline void AddToTaskPaneList( Window* pWindowToBeHandled )
    {
        HandleTaskPaneList( pWindowToBeHandled, sal_True );
    }

    inline void RemoveFromTaskPaneList( Window* pWindowToBeHandled )
    {
        HandleTaskPaneList( pWindowToBeHandled, sal_False );
    }
}

#endif
