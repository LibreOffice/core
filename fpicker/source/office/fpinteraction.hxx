/*************************************************************************
 *
 *  $RCSfile: fpinteraction.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 08:52:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVTOOLS_FILEPICKER_INTERACTION_HXX
#define SVTOOLS_FILEPICKER_INTERACTION_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OFilePickerInteractionHandler
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::task::XInteractionHandler
                                    >   OFilePickerInteractionHandler_Base;

    /** a InteractionHandler implementation which extends another handler with some customizability
    */
    class OFilePickerInteractionHandler : public OFilePickerInteractionHandler_Base
    {
    public:
        /** flags, which indicates special handled interactions
            These values will be used combained as flags - so they must
            in range [2^n]!
         */
        enum EInterceptedInteractions
        {
            E_NOINTERCEPTION = 0,
            E_DOESNOTEXIST   = 1
            // next values [2,4,8,16 ...]!
        };

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > m_xMaster        ; // our master handler
        ::com::sun::star::uno::Any                                                      m_aException     ; // the last handled request
        sal_Bool                                                                        m_bUsed          ; // indicates using of this interaction handler instance
        EInterceptedInteractions                                                        m_eInterceptions ; // enable/disable interception of some special interactions

    public:
        OFilePickerInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxMaster );

        // some generic functions
        void     enableInterceptions( EInterceptedInteractions eInterceptions );
        sal_Bool wasUsed            () const;
        void     resetUseState      ();
        void     forgetRequest      ();

        // functions to analyze last cached request
        sal_Bool wasAccessDenied() const;

    protected:
        // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _rxRequest ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ~OFilePickerInteractionHandler();
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_FILEPICKER_INTERACTION_HXX

