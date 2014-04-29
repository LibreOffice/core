/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

        // check if the given table is accessible by the connected user
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


