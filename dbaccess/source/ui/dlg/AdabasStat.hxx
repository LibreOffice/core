/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/
#ifndef _DBAUI_ADASTAT_HXX_
#define _DBAUI_ADASTAT_HXX_

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef DBAUI_FIELDCONTROLS_HXX
#include "FieldControls.hxx"
#endif
#ifndef _PRGSBAR_HXX
#include <svtools/prgsbar.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


namespace dbaui
{
    //========================================================================
    // shows some statistics
    //========================================================================
    class OAdabasStatistics : public ModalDialog
    {
    protected:

        FixedLine               m_FL_FILES;
        FixedText               m_FT_SYSDEVSPACE;
        OPropEditCtrl           m_ET_SYSDEVSPACE;
        FixedText               m_FT_TRANSACTIONLOG;
        OPropEditCtrl           m_ET_TRANSACTIONLOG;
        FixedText               m_FT_DATADEVSPACE;
        OPropListBoxCtrl        m_LB_DATADEVS;
        FixedLine               m_FL_SIZES;
        FixedText               m_FT_SIZE;
        OPropEditCtrl           m_ET_SIZE;
        FixedText               m_FT_FREESIZE;
        OPropEditCtrl           m_ET_FREESIZE;
        FixedText               m_FT_MEMORYUSING;
        OPropNumericEditCtrl    m_ET_MEMORYUSING;

        OKButton                m_PB_OK;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                m_xConnection;  /// valid as long as the page is active
        sal_Bool                m_bErrorShown; // true when we shown already the error

        // check if the given table is accessable by the connected user
        sal_Bool checkSystemTable(const ::rtl::OUString& _rsSystemTable, ::rtl::OUString& _rsSchemaName );
        void showError();

    public:
        OAdabasStatistics(  Window* pParent,
                            const ::rtl::OUString& _rUser,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCurrentConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory);
        ~OAdabasStatistics();

    };
}
#endif //_DBAUI_ADASTAT_HXX_


