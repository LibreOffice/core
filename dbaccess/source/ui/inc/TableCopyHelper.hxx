/*************************************************************************
 *
 *  $RCSfile: TableCopyHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:17:17 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBUI_TABLECOPYHELPER_HXX
#define DBUI_TABLECOPYHELPER_HXX

#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#include <functional>

class SvLBoxEntry;
//........................................................................
namespace dbaui
{
//........................................................................
    class OGenericUnoController;
    /// unary_function Functor object for class DataFlavorExVector::value_type returntype is bool
    struct TAppSupportedSotFunctor : ::std::unary_function<DataFlavorExVector::value_type,bool>
    {
        ElementType eEntryType;
        sal_Bool    bQueryDrop;
        TAppSupportedSotFunctor(const ElementType& _eEntryType,sal_Bool _bQueryDrop)
            : eEntryType(_eEntryType)
            , bQueryDrop(_bQueryDrop)
        {
        }

        inline bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
                case SOT_FORMAT_RTF:                    // RTF data descriptions
                case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
                case SOT_FORMATSTR_ID_HTML_SIMPLE:      // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                    return (E_TABLE == eEntryType);
                    break;
                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                    return ((E_QUERY == eEntryType) || ( !bQueryDrop && E_TABLE == eEntryType));
                    break;
            }
            return false;
        }
    };

    class OTableCopyHelper
    {
        OGenericUnoController* m_pController;
    public:
        // is needed to describe the drop target
        struct DropDescriptor
        {
            ::svx::ODataAccessDescriptor    aDroppedData;
            String                          aUrl;
            SotStorageStreamRef             aHtmlRtfStorage;
            ElementType                     nType;
            SvLBoxEntry*                    pDroppedAt;
            sal_Int8                        nAction;
            sal_Bool                        bHtml;
            sal_Bool                        bError;

            DropDescriptor() : nType(E_TABLE),nAction(DND_ACTION_NONE),pDroppedAt(NULL) { }
        };

        OTableCopyHelper(OGenericUnoController* _pControler);

        /** pastes a table into the data source
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( const TransferableDataHelper& _rTransData
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** pastes a table into the data source
            @param  _nFormatId
                The format which should be copied.
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( SotFormatStringId _nFormatId
                        ,const TransferableDataHelper& _rTransData
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** pastes a table into the data source
            @param  _rPasteData
                The data descriptor.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( ::svx::ODataAccessDescriptor& _rPasteData
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xDestConnection);

        /** insert a table into the data source. The source can eihter be a table or a query
            @param  _nCommandType
                The command type.
            @param  _xSrcConnection
                The connection of the source.
            @param  _xSrcRs
                The ResultSet of the source.
            @param  _aSelection
                The selection of the rows to copy.
            @param  _bBookmarkSelection
                If <TRUE/> the selection is bookmark selection.
            @param  _sCommand
                The name of the query or table.
            @param  _sSrcDataSourceName
                The name of the source data source.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void insertTable( sal_Int32 _nCommandType
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xSrcConnection
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>&   _xSrcRs         // the source resultset may be empty
                        ,const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _aSelection
                        ,sal_Bool _bBookmarkSelection
                        ,const ::rtl::OUString& _sCommand
                        ,const ::rtl::OUString& _sSrcDataSourceName
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xDestConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
            @param  _xConnection
                The connection
        */
        sal_Bool copyTagTable(  DropDescriptor& _rDesc
                                , sal_Bool _bCheck
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
            @param  _xConnection
                The connection
        */
        void asyncCopyTagTable(  DropDescriptor& _rDesc
                                ,const ::rtl::OUString& _sDestDataSourceName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _aDroppedData
                The dropped data
            @param  _rDesc
                IN/OUT parameter
            @param  _xConnection
                The connection
        */
        sal_Bool copyTagTable(const TransferableDataHelper& _aDroppedData
                              ,DropDescriptor& _rAsyncDrop
                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /// returns <TRUE/> if the clipboard supports a table format, otherwise <FALSE/>.
        sal_Bool isTableFormat(const TransferableDataHelper& _rClipboard) const;

    };
//........................................................................
}   // namespace dbaui
//........................................................................
#endif // DBUI_TABLECOPYHELPER_HXX

