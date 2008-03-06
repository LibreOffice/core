/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IController.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:53:44 $
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

#ifndef DBAUI_ICONTROLLER_HXX
#define DBAUI_ICONTROLLER_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef DBAUI_IREFERENCE_HXX
#include "IReference.hxx"
#endif
#include "dbaccessdllapi.h"

namespace com { namespace sun { namespace star {
    namespace util {
        struct URL;
    }
    namespace frame {
        class XController;
    }
} } }

namespace dbaui
{
    // interface for controller depended calls like commands
    class DBACCESS_DLLPUBLIC IController : public IReference
    {
    public:
        /** executes the given command without checking if it is allowed
            @param  _rCommand   the URL of the command
        */
        virtual void executeUnChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _rCommand
                the URL of the command
        */
        virtual void executeChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command without checking if it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeUnChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

        /** executes the given command only when it is allowed
            @param  _nCommandId
                the id of the command URL
        */
        virtual void executeChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;


        /** checks if the given Command is enabled
            @param  _nCommandId
                the id of the command URL

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual sal_Bool isCommandEnabled(sal_uInt16 _nCommandId) const = 0;

        /** checks if the given Command is enabled
            @param  _rCompleteCommandURL
                the URL of the command

            @return
                <TRUE/> if the command is allowed, otherwise <FALSE/>.
        */
        virtual sal_Bool isCommandEnabled( const ::rtl::OUString& _rCompleteCommandURL ) const = 0;

        /** notifyHiContrastChanged will be called when the hicontrast mode changed.
            @param  _bHiContrast
                <TRUE/> when in hicontrast mode.
        */
        virtual void notifyHiContrastChanged() {}

        /** checks if the selected data source is read only
            @return
                <TRUE/> if read only, otherwise <FALSE/>
        */
        virtual sal_Bool isDataSourceReadOnly() const = 0;

        /** provides access to the model of the controller

            This must be the same model as returned by XController::getModel, and might be <NULL/> when
            the controller does not have an own model.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL
                getXController(void) throw( ::com::sun::star::uno::RuntimeException ) = 0;
    };
}
#endif // DBAUI_ICONTROLLER_HXX
