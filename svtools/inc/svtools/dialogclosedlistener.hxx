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



#ifndef _SVTOOLS_DIALOGCLOSEDLISTENER_HXX
#define _SVTOOLS_DIALOGCLOSEDLISTENER_HXX

#include "svtools/svtdllapi.h"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <tools/link.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= ODialogClosedListener
    //=====================================================================
    /**
        C++ class to implement a ::com::sun::star::ui::dialogs::XDialogClosedListener
    */
    class SVT_DLLPUBLIC DialogClosedListener :
        public ::cppu::WeakImplHelper1< ::com::sun::star::ui::dialogs::XDialogClosedListener >
    {
    private:
        /**
            This link will be called when the dialog was closed.

            The link must have the type:
            DECL_LINK( DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );
        */
        Link    m_aDialogClosedLink;

    public:
        DialogClosedListener();
        DialogClosedListener( const Link& rLink );

        inline void SetDialogClosedLink( const Link& rLink ) { m_aDialogClosedLink = rLink; }

        // XDialogClosedListener methods
        virtual void SAL_CALL   dialogClosed( const ::com::sun::star::ui::dialogs::DialogClosedEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener methods
        virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif// COMPHELPER_DIALOGCLOSEDLISTENER_HXX

