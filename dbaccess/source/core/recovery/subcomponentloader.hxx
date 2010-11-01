/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
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
************************************************************************/

#ifndef SUBCOMPONENTLOADER_HXX
#define SUBCOMPONENTLOADER_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= SubComponentLoader
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XWindowListener
                                    >   SubComponentLoader_Base;
    struct SubComponentLoader_Data;
    /** is a helper class which loads/opens a given sub component as soon as the main application
        window becomes visible.
    */
    class DBACCESS_DLLPRIVATE SubComponentLoader : public SubComponentLoader_Base
    {
    public:
        SubComponentLoader(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& i_rApplicationController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandProcessor >& i_rSubDocumentDefinition
        );

        SubComponentLoader(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& i_rApplicationController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& i_rNonDocumentComponent
        );

        // XWindowListener
        virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~SubComponentLoader();

    private:
        SubComponentLoader_Data*    m_pData;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // SUBCOMPONENTLOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
