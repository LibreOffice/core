/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableHandler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:37:34 $
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
#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX

#ifndef INCLUDED_TABLE_MANAGER_HXX
#include <resourcemodel/TableManager.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include "PropertyMap.hxx"
#endif

#include <com/sun/star/text/XTextAppendAndConvert.hpp>

namespace writerfilter {
namespace dmapper {

typedef ::com::sun::star::text::XTextRange TextRange_t;
typedef ::com::sun::star::uno::Reference< TextRange_t > Handle_t;
typedef ::com::sun::star::uno::Sequence< Handle_t> CellSequence_t;
typedef boost::shared_ptr<CellSequence_t> CellSequencePointer_t;
typedef ::com::sun::star::uno::Sequence< CellSequence_t > RowSequence_t;
typedef boost::shared_ptr<RowSequence_t> RowSequencePointer_t;
typedef ::com::sun::star::uno::Sequence< RowSequence_t> TableSequence_t;
typedef boost::shared_ptr<TableSequence_t> TableSequencePointer_t;
typedef ::com::sun::star::text::XTextAppendAndConvert Text_t;
typedef ::com::sun::star::uno::Reference<Text_t> TextReference_t;

typedef ::com::sun::star::beans::PropertyValues                     TablePropertyValues_t;
typedef ::com::sun::star::uno::Sequence< TablePropertyValues_t >    RowPropertyValuesSeq_t;
typedef ::com::sun::star::uno::Sequence< RowPropertyValuesSeq_t>    CellPropertyValuesSeq_t;

typedef std::vector<PropertyMapPtr>     PropertyMapVector1;
typedef std::vector<PropertyMapVector1> PropertyMapVector2;

class DomainMapperTableHandler : public TableDataHandler<Handle_t , PropertyMapPtr >
{
    TextReference_t         m_xText;
    CellSequencePointer_t   m_pCellSeq;
    RowSequencePointer_t    m_pRowSeq;
    TableSequencePointer_t  m_pTableSeq;

    // properties
    PropertyMapVector2      m_aCellProperties;
    PropertyMapVector1      m_aRowProperties;
    PropertyMapPtr          m_aTableProperties;

    sal_Int32 m_nCellIndex;
    sal_Int32 m_nRowIndex;

public:
    typedef boost::shared_ptr<DomainMapperTableHandler> Pointer_t;

    DomainMapperTableHandler(TextReference_t xText)
    : m_xText(xText),
        m_nCellIndex(0),
        m_nRowIndex(0)
    {
    }
    virtual ~DomainMapperTableHandler() {}

    virtual void startTable(unsigned int nRows, unsigned int nDepth,
                            PropertyMapPtr pProps);
    virtual void endTable();
    virtual void startRow(unsigned int nCells, PropertyMapPtr pProps);
    virtual void endRow();
    virtual void startCell(const Handle_t & start, PropertyMapPtr pProps);
    virtual void endCell(const Handle_t & end);
};

}}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX
