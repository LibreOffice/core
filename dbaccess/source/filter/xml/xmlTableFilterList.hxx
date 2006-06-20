/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTableFilterList.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:52:28 $
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
#ifndef DBA_XMLTABLEFILTERLIST_HXX
#define DBA_XMLTABLEFILTERLIST_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#include <vector>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTableFilterList : public SvXMLImportContext
    {
        ::std::vector< ::rtl::OUString> m_aPatterns;
        ::std::vector< ::rtl::OUString> m_aTypes;

        ODBFilter& GetOwnImport();
    public:

        OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName);

        virtual ~OXMLTableFilterList();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        /** pushes a new TableFilterPattern to the list of patterns
            @param  _sTableFilterPattern
                The new filter pattern.
        */
        inline void pushTableFilterPattern(const ::rtl::OUString& _sTableFilterPattern)
        {
            m_aPatterns.push_back(_sTableFilterPattern);
        }

        /** pushes a new TableTypeFilter to the list of patterns
            @param  _sTypeFilter
                The new type filter.
        */
        inline void pushTableTypeFilter(const ::rtl::OUString& _sTypeFilter)
        {
            m_aTypes.push_back(_sTypeFilter);
        }
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLEFILTERLIST_HXX
