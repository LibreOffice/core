/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentcontroller.hxx,v $
 * $Revision: 1.3.2.2 $
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


#ifndef DBACCESS_SOURCE_UI_INC_DOCUMENTCONTROLLER_HXX
#define DBACCESS_SOURCE_UI_INC_DOCUMENTCONTROLLER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#include <cppuhelper/weakref.hxx>
/** === end UNO includes === **/

#ifndef INCLUDED_DBACCESSDLLAPI_H
#include "dbaccessdllapi.h"
#endif
//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= ModelControllerConnector
    //====================================================================
    /** a helper class for controllers associated with an ->XModel

        Instances of this class take an ->XModel, which they connect to at
        construction time (->XModel::connectController), and disconnect from
        at destruction time (->XModel::disconnectController).

        Additionally, they keep the controller alive (by being a CloseVetoListener)
        as long as they themself are alive.
    */
    class DBACCESS_DLLPUBLIC ModelControllerConnector
    {
    private:
        typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >         Model;
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    Controller;

        Model       m_xModel;
        Controller  m_xController;

    public:
        /** constructs the object, and connects the controller to the model

        @param _rxModel
            the model to which the controller should be connected
        @param _rxController
            the controller which should be connected to the model
        */
        ModelControllerConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& _rxController
        );

        /** connects the controller to the model

        @param _rxModel
            the model to which the controller should be connected
        @param _rxController
            the controller which should be connected to the model
        */
        void connect(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& _rxController
        );

        /** destroys the object, and disconnects the controller from the model
        */
        ~ModelControllerConnector();

        /** determines whether the object is empty
        */
        inline  bool    empty() const { return !m_xController.is(); }

        /** clears the object

            The controller is disconnected from the model, and references to the model and the controller
            are released.
        */
        inline void clear()
        {
            connect( NULL, NULL );
        }

    public:
        ModelControllerConnector();
        ModelControllerConnector( const ModelControllerConnector& _rSource );
        ModelControllerConnector& operator=( const ModelControllerConnector& _rSource );

    private:
        void impl_connect();
        void impl_disconnect();
        void impl_copyFrom( const ModelControllerConnector& _rSource );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_DOCUMENTCONTROLLER_HXX

