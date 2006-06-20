/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JoinExchange.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:11:58 $
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
#ifndef DBAUI_JOINEXCHANGE_HXX
#define DBAUI_JOINEXCHANGE_HXX

#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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


