#ifndef RPT_XMLTABLE_HXX
#define RPT_XMLTABLE_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:58:35 $
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

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLTable : public SvXMLImportContext
    {
    public:
        struct TCell
        {
            sal_Int32 nWidth;
            sal_Int32 nHeight;
            sal_Int32 nColSpan;
            sal_Int32 nRowSpan;
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> > xElements;
            TCell() : nWidth(0),nHeight(0),nColSpan(1),nRowSpan(1){}
        };
    private:
        ::std::vector< ::std::vector<TCell> >                                               m_aGrid;
        ::std::vector<sal_Int32>                                                            m_aHeight;
        ::std::vector<sal_Int32>                                                            m_aWidth;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >              m_xSection;
        ::rtl::OUString                                                                     m_sStyleName;
        sal_Int32                                                                           m_nColSpan;
        sal_Int32                                                                           m_nRowSpan;
        sal_Int32                                                                           m_nRowIndex;
        sal_Int32                                                                           m_nColumnIndex;
        ORptFilter& GetOwnImport();

        OXMLTable(const OXMLTable&);
        void operator =(const OXMLTable&);
    public:

        OXMLTable( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                    );
        virtual ~OXMLTable();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        inline void addHeight(sal_Int32 _nHeight)   { m_aHeight.push_back( _nHeight ); }
        inline void addWidth(sal_Int32 _nWidth)     { m_aWidth.push_back( _nWidth ); }

        inline void setColumnSpanned(sal_Int32 _nColSpan)     { m_nColSpan = _nColSpan; }
        inline void setRowSpanned(   sal_Int32 _nRowSpan)     { m_nRowSpan = _nRowSpan; }

        void incrementRowIndex();
        inline void incrementColumnIndex()  { ++m_nColumnIndex; }

        inline sal_Int32 getRowIndex() const { return m_nRowIndex; }
        inline sal_Int32 getColumnIndex() const { return m_nColumnIndex; }

        void addCell(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xElement);

        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const { return m_xSection; }
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLTABLE_HXX
