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



#ifndef _CLIPLISTENER_HXX
#define _CLIPLISTENER_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>

class Window;


class SVT_DLLPUBLIC TransferableClipboardListener : public ::cppu::WeakImplHelper1<
                            ::com::sun::star::datatransfer::clipboard::XClipboardListener >
{
    Link    aLink;

public:
            // Link is called with a TransferableDataHelper pointer
            TransferableClipboardListener( const Link& rCallback );
            ~TransferableClipboardListener();

    void    AddRemoveListener( Window* pWin, sal_Bool bAdd );
    void    ClearCallbackLink();

            // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                                            throw(::com::sun::star::uno::RuntimeException);
            // XClipboardListener
    virtual void SAL_CALL changedContents( const ::com::sun::star::datatransfer::clipboard::ClipboardEvent& event )
                                            throw(::com::sun::star::uno::RuntimeException);
};

#endif

