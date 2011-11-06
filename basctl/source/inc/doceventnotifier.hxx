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



#ifndef BASCTL_DOCEVENTNOTIFIER_HXX
#define BASCTL_DOCEVENTNOTIFIER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>

//........................................................................
namespace basctl
{
//........................................................................

    class ScriptDocument;

    //====================================================================
    //= DocumentEventListener
    //====================================================================
    class SAL_NO_VTABLE DocumentEventListener : ::boost::noncopyable
    {
    public:
        virtual void onDocumentCreated( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentOpened( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentSave( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentSaveDone( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentSaveAs( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentClosed( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument ) = 0;
        virtual void onDocumentModeChanged( const ScriptDocument& _rDocument ) = 0;

        virtual ~DocumentEventListener();
    };

    //====================================================================
    //= DocumentEventNotifier
    //====================================================================
    class DocumentEventNotifier_Impl;
    /** allows registering at the GlobalEventBroadcaster for global document events
    */
    class DocumentEventNotifier
    {
    public:
        /** create a notifier instance which notifies about events of all documents in the whole application
        */
        DocumentEventNotifier( DocumentEventListener& _rListener );

        /** creates a notifier instance which notifies about events at a single document
        */
        DocumentEventNotifier( DocumentEventListener& _rListener,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument );

        ~DocumentEventNotifier();

    public:
        void    dispose();

    private:
        ::rtl::Reference< DocumentEventNotifier_Impl >  m_pImpl;
    };

//........................................................................
} // namespace basctl
//........................................................................

#endif // BASCTL_DOCEVENTNOTIFIER_HXX
