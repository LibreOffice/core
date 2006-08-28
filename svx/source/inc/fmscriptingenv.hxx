/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmscriptingenv.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-28 15:02:33 $
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

#ifndef SVX_FMSCRIPTINGENV_HXX
#define SVX_FMSCRIPTINGENV_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

class FmFormModel;
//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= IFormScriptingEnvironment
    //====================================================================
    /** describes the interface implemented by a component which handles scripting requirements
        in a form/control environment.
    */
    class SAL_NO_VTABLE IFormScriptingEnvironment : public ::rtl::IReference
    {
    public:
        /** registers an XEventAttacherManager whose events should be monitored and handled

            @param _rxManager
                the XEventAttacherManager to monitor. Must not be <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if attaching as script listener to the manager fails with a RuntimeException itself
        */
        virtual void registerEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** registers an XEventAttacherManager whose events should not be monitored and handled anymore

            @param _rxManager
                the XEventAttacherManager which was previously registered. Must not ne <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if removing as script listener from the manager fails with a RuntimeException itself
        */
        virtual void revokeEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** disposes the scripting environment instance
        */
        virtual void dispose() = 0;

        virtual ~IFormScriptingEnvironment();
    };
    typedef ::rtl::Reference< IFormScriptingEnvironment >   PFormScriptingEnvironment;

    //====================================================================
    /** creates a default component implementing the IFormScriptingEnvironment interface
    */
    PFormScriptingEnvironment   createDefaultFormScriptingEnvironment( FmFormModel& _rFormModel );

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FMSCRIPTINGENV_HXX

