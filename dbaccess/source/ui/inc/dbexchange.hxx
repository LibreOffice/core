/*************************************************************************
 *
 *  $RCSfile: dbexchange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-16 15:54:53 $
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
#ifndef DBAUI_DBEXCHANGE_HXX
#define DBAUI_DBEXCHANGE_HXX

#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

namespace dbaui
{
    //==================================================================
    // ODataExchange :
    // Basisklasse fuer den Datenaustausch in im browser controller
    //==================================================================
    class ODataExchange : public SvDataMemberObject
    {
        SvRefBaseRef xExchObj;
    protected:
        static String aDataExchangeFormat;
        static String aRTFExchangeFormat;
        static String aHTMLExchangeFormat;

        String m_aDataExchange;
    public:
        TYPEINFO();
        ODataExchange(){}
        ODataExchange(const String& _rExchangeStr);
        virtual ~ODataExchange();

        virtual void        SetExchObj( SvRefBase* pExchObj ){ xExchObj = pExchObj; }
        virtual SvRefBase*  GetExchObj(){ return xExchObj; }

        virtual BOOL GetData( SvData* );
        //virtual BOOL SetData( SvData * );
        virtual void SetBookmark( const String& rURL, const String& rLinkName );

    };

    SV_DECL_IMPL_REF( ODataExchange );

    class OHTMLImportExport;
    class ORTFImportExport;
    class ODataClipboard : public TransferableHelper
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aSeq;
        ::std::auto_ptr<OHTMLImportExport>      m_pHtml;
        ::std::auto_ptr<ORTFImportExport>       m_pRtf;
    public:
        ODataClipboard(::std::auto_ptr<OHTMLImportExport>   _pHtml,::std::auto_ptr<ORTFImportExport>    _pRtf)
            :m_pHtml(_pHtml)
            ,m_pRtf(_pRtf)
        {}
        ODataClipboard(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aSeq)
            :m_aSeq(_aSeq)
            ,m_pHtml(NULL)
            ,m_pRtf(NULL)
        {
        }
    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
        virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    };
}

#endif //  DBAUI_DBEXCHANGE_HXX







