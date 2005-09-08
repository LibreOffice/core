/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AdabasStat.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:40:06 $
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


