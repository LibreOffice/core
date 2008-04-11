/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: doceventnotifier.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
