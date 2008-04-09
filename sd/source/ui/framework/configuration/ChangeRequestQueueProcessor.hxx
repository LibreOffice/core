/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChangeRequestQueueProcessor.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-09 16:27:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_FRAMEWORK_CHANGE_REQUEST_QUEUE_PROCESSOR_HXX
#define SD_FRAMEWORK_CHANGE_REQUEST_QUEUE_PROCESSOR_HXX

#include "ChangeRequestQueue.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <rtl/ref.hxx>

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGEREQUEST_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_CONFIGURATIONCHANGEEVENT_HPP_
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#endif

#include <cppuhelper/interfacecontainer.hxx>
#include <tools/link.hxx>

#include <boost/shared_ptr.hpp>

namespace sd { namespace framework {

class ConfigurationController;
class ConfigurationUpdater;

/** The ChangeRequestQueueProcessor ownes the ChangeRequestQueue and
    processes the configuration change requests.

    When after processing one entry the queue is empty then the
    XConfigurationController::update() method is called so that the changes
    made to the local XConfiguration reference are reflected by the UI.

    Queue entries are processed asynchronously by calling PostUserEvent().
*/
class ChangeRequestQueueProcessor
{
public:
    /** The queue processor is created with a reference to an
        ConfigurationController so that its UpdateConfiguration() method can
        be called when the queue becomes empty.
    */
    ChangeRequestQueueProcessor (
        const ::rtl::Reference<ConfigurationController>& rxController,
        const ::boost::shared_ptr<ConfigurationUpdater>& rpUpdater);
    ~ChangeRequestQueueProcessor (void);

    /** Sets the configuration who will be changed by subsequent change
        requests.  This method should be called only by the configuration
        controller who owns the configuration.
    */
    void SetConfiguration (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration);

    /** The given request is appended to the end of the queue and will
        eventually be processed when all other entries in front of it have
        been processed.
    */
    void AddRequest (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationChangeRequest>& rxRequest);

    /** Returns </TRUE> when the queue is empty.
    */
    bool IsEmpty (void) const;

    /** Process all events in the queue synchronously.

        <p>This method is typically called when the framework is shut down
        to establish an empty configuration.</p>
    */
    void ProcessUntilEmpty (void);

    /** Process the first event in queue.
    */
    void ProcessOneEvent (void);

    /** Remove all events from the queue.

        <p>This method is typically called when the framework is shut down
        to avoid the processing of still pending activation requests.</p>
    */
    void Clear (void);

private:
    mutable ::osl::Mutex maMutex;

    ChangeRequestQueue maQueue;

    /** The id returned by the last PostUserEvent() call.  This id is stored
        so that a pending user event can be removed whent he queue processor
        is destroyed.
    */
    sal_uIntPtr mnUserEventId;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxConfiguration;

    ::rtl::Reference<ConfigurationController> mpConfigurationController;

    ::boost::shared_ptr<ConfigurationUpdater> mpConfigurationUpdater;

    /** Initiate the processing of the entries in the queue.  The actual
        processing starts asynchronously.
    */
    void StartProcessing (void);

    /** Callback function for the PostUserEvent() call.
    */
    DECL_LINK(ProcessEvent,void*);
};


} } // end of namespace sd::framework

#endif
