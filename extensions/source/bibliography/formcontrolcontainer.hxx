/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formcontrolcontainer.hxx,v $
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

#ifndef EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX
#define EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX

#include <comphelper/broadcasthelper.hxx>
#include "loadlisteneradapter.hxx"
#include <com/sun/star/awt/XControlContainer.hpp>

class BibDataManager;

//.........................................................................
namespace bib
{
//.........................................................................

    //=====================================================================
    //= FormControlContainer
    //=====================================================================
    class FormControlContainer
            :public ::comphelper::OBaseMutex
            ,public ::bib::OLoadListener
    {
    private:
        OLoadListenerAdapter*   m_pFormAdapter;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >
                                m_xForm;
    private:
        void    implSetDesignMode( sal_Bool _bDesign );

    protected:
        FormControlContainer( );
        ~FormControlContainer( );

        sal_Bool    isFormConnected() const { return NULL != m_pFormAdapter; }
        void        connectForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxForm );
        void        disconnectForm();

        void        ensureDesignMode();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    getControlContainer() = 0;

    protected:
    // XLoadListener equivalents
        virtual void _loaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _unloading( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _unloaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _reloading( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _reloaded( const ::com::sun::star::lang::EventObject& _rEvent );

    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif // EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX

