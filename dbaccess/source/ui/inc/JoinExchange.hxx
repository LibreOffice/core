/*************************************************************************
 *
 *  $RCSfile: JoinExchange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:05:53 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
    struct OJoinExchangeData
    {
    public:
        OTableWindowListBox*    pListBox;       // die ListBox innerhalb desselben (daraus kann man sich das TabWin und daraus den WinName besorgen)
        SvLBoxEntry*            pEntry;         // der Eintrag, der gedraggt oder auf den gedroppt wurde

        OJoinExchangeData(OTableWindowListBox* pBox) : pListBox(pBox), pEntry(pBox->FirstSelected()) { }
        OJoinExchangeData() : pListBox(NULL), pEntry(NULL) { }
    };

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

    protected:
        OJoinExchangeData           m_jxdSourceDescription;
        IDragTransferableListener*  m_pDragListener;

    public:
        OJoinExchObj(const OJoinExchangeData& jxdSource);
        ~OJoinExchObj();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

        void StartDrag( Window* pWindow, sal_Int8 nDragSourceActions, IDragTransferableListener* _pListener );

        static OJoinExchangeData    GetSourceDescription(const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxObject);
        static sal_Bool             isFormatAvailable( const DataFlavorExVector& _rFormats );

    protected:
        virtual void                AddSupportedFormats();
        virtual sal_Bool            GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void                DragFinished( sal_Int8 nDropAction );

        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };
}
#endif // DBAUI_JOINEXCHANGE_HXX


