/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fpinteraction.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SVTOOLS_FILEPICKER_INTERACTION_HXX
#define SVTOOLS_FILEPICKER_INTERACTION_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

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

