/*************************************************************************
 *
 *  $RCSfile: JoinExchange.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:06:46 $
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
#include "JoinExchange.hxx"
#endif
#ifndef _SV_EXCHANGE_HXX
#include <vcl/exchange.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::datatransfer;

    String OJoinExchObj::m_sJoinFormat;
    //==================================================================
    // class OJoinExchObj
    //==================================================================
    //------------------------------------------------------------------------
    OJoinExchObj::OJoinExchObj(const OJoinExchangeData& jxdSource)
        :m_jxdSourceDescription(jxdSource)
        ,m_pDragListener(NULL)
    {
        // Verfuegbare Typen in Liste einfuegen
    }

    //------------------------------------------------------------------------
    OJoinExchObj::~OJoinExchObj()
    {
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::StartDrag( Window* _pWindow, sal_Int8 _nDragSourceActions, IDragTransferableListener* _pListener )
    {
        m_pDragListener = _pListener;
        TransferableHelper::StartDrag(_pWindow, _nDragSourceActions);
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::DragFinished( sal_Int8 nDropAction )
    {
        if (m_pDragListener)
            m_pDragListener->dragFinished();
        m_pDragListener = NULL;
    }

    //------------------------------------------------------------------------
    sal_Bool OJoinExchObj::isFormatAvailable( const DataFlavorExVector& _rFormats )
    {
        for (   DataFlavorExVector::const_iterator aCheck = _rFormats.begin();
                aCheck != _rFormats.end();
                ++aCheck
            )
        {
            if (SOT_FORMATSTR_ID_SBA_JOIN == aCheck->mnSotId)
                return sal_True;
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    OJoinExchangeData OJoinExchObj::GetSourceDescription(const Reference< XTransferable >& _rxObject)
    {
        OJoinExchangeData aReturn;
        Reference< XUnoTunnel > xTunnel(_rxObject, UNO_QUERY);
        if (xTunnel.is())
        {
            OJoinExchObj* pImplementation = reinterpret_cast<OJoinExchObj*>(xTunnel->getSomething(getUnoTunnelImplementationId()));
            if (pImplementation)
                aReturn = pImplementation->m_jxdSourceDescription;
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    Sequence< sal_Int8 > OJoinExchObj::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //------------------------------------------------------------------------
    sal_Int64 SAL_CALL OJoinExchObj::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
    {
        if (_rIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
            return reinterpret_cast<sal_Int64>(this);

        return 0;
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::AddSupportedFormats()
    {
        AddFormat( SOT_FORMATSTR_ID_SBA_JOIN );
    }

    //------------------------------------------------------------------------
    sal_Bool OJoinExchObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uInt32 nFormat = SotExchange::GetFormat(rFlavor);
        if ( SOT_FORMATSTR_ID_SBA_JOIN == nFormat )
            // this is a HACK
            // we don't really copy our data, the instances using us have to call GetSourceDescription ....
            // if, one day, we have a _lot_ of time, this hack should be removed ....
            return sal_True;

        return sal_False;
    }

    //------------------------------------------------------------------------
    Any SAL_CALL OJoinExchObj::queryInterface( const Type& _rType ) throw(RuntimeException)
    {
        Any aReturn = TransferableHelper::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = OJoinExchObj_Base::queryInterface(_rType);
        return aReturn;
    }

    //------------------------------------------------------------------------
    void SAL_CALL OJoinExchObj::acquire(  ) throw()
    {
        TransferableHelper::acquire( );
    }

    //------------------------------------------------------------------------
    void SAL_CALL OJoinExchObj::release(  ) throw()
    {
        TransferableHelper::release( );
    }


}   // namespace dbaui
