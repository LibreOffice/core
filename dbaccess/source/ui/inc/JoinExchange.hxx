/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_JOINEXCHANGE_HXX
#define DBAUI_JOINEXCHANGE_HXX

#include "dbexchange.hxx"
#include "TableWindowListBox.hxx"

#include <svtools/transfer.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase1.hxx>

namespace dbaui
{
    //==================================================================
    // OJoinExchObj :
    // Zusaetzliche Daten fuer das Erzeugen von Joins in der JoinShell
    //==================================================================

    typedef ::cppu::ImplHelper1< ::com::sun::star::lang::XUnoTunnel > OJoinExchObj_Base;
    class OJoinExchObj
                    :public TransferableHelper
                    ,public OJoinExchObj_Base
    {
        static String           m_sJoinFormat;
        sal_Bool                m_bFirstEntry;

    protected:
        OJoinExchangeData           m_jxdSourceDescription;
        IDragTransferableListener*  m_pDragListener;

        virtual ~OJoinExchObj();
    public:
        OJoinExchObj(const OJoinExchangeData& jxdSource,sal_Bool _bFirstEntry=sal_False);


        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

        void StartDrag( Window* pWindow, sal_Int8 nDragSourceActions, IDragTransferableListener* _pListener );

        static OJoinExchangeData    GetSourceDescription(const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxObject);
        static sal_Bool             isFormatAvailable( const DataFlavorExVector& _rFormats ,SotFormatStringId _nSlotID=SOT_FORMATSTR_ID_SBA_JOIN);

    protected:
        virtual void                AddSupportedFormats();
        virtual sal_Bool            GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void                DragFinished( sal_Int8 nDropAction );

        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    private:
        using TransferableHelper::StartDrag;
    };
}
#endif // DBAUI_JOINEXCHANGE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
