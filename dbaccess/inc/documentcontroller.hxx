/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentcontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:50:04 $
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

        /** destroys the object, and disconnects the controller from the model
        */
        ~ModelControllerConnector();

        /** determines whether the object is empty
        */
        inline  bool    empty() const { return !m_xController.is(); }

        /** exchanges the content of <code>this</code> and the given ->ModelControllerConnector instance
        */
        void swap( ModelControllerConnector& _rSwapWith );

        /** clears the object

            The controller is disconnected from the model, and references to the model and the controller
            are released.
        */
        inline void clear()
        {
            ModelControllerConnector aEmpty;
            swap( aEmpty );
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

