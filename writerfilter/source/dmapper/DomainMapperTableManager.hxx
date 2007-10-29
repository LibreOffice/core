/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableManager.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:30:38 $
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
#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

#ifndef INCLUDED_TABLE_MANAGER_HXX
#include <doctok/TableManager.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include "PropertyMap.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#include <vector>

namespace dmapper {
typedef ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > Handle_t;
typedef doctok::TableManager<Handle_t , PropertyMapPtr > DomainMapperTableManager_Base_t;
class DomainMapperTableManager : public DomainMapperTableManager_Base_t
{
    sal_uInt32      m_nRow;
    sal_uInt32      m_nCell;
    sal_uInt32      m_nCellBorderIndex; //borders are provided for all cells and need counting
    sal_Int32       m_nHeaderRepeat; //counter of repeated headers - if == -1 then the repeating stops
    sal_Int32       m_nGapHalf; // necessary value to calculate width and columns
    sal_Int32       m_nLeftMargin; // to-be-combined width m_nGapHalf
    sal_Int32       m_nTableWidth; //might be set directly or has to be calculated from the column positions
    bool            m_bFullWidth; //width is set to full, disable setting of different orientation values
    ::rtl::OUString m_sTableStyleName;
    ::std::vector<sal_Int32>  m_aCellWidths;

    virtual void clearData();

public:

    DomainMapperTableManager();
    virtual ~DomainMapperTableManager();

    virtual bool sprm(doctok::Sprm & rSprm);

    virtual void endOfCellAction();
    virtual void endOfRowAction();

    const ::rtl::OUString& getTableStyleName() const { return m_sTableStyleName; }
};

}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
