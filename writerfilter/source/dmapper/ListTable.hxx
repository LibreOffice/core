/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ListTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2006-11-02 12:37:24 $
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
#ifndef INCLUDED_LISTTABLE_HXX
#define INCLUDED_LISTTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>

namespace com{ namespace sun { namespace star {
    namespace text{
        class XTextDocument;
    }
    namespace container{
        class XIndexReplace;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
}}}
namespace dmapper
{
class DomainMapper;
struct ListTable_Impl;
class WRITERFILTER_DLLPRIVATE ListTable :
        public doctok::Properties,
        public doctok::Table
{
    ListTable_Impl   *m_pImpl;

public:
    ListTable(
            DomainMapper& rDMapper,
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory);
    virtual ~ListTable();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    // BinaryObj
//    virtual void data(const sal_Int8* buf, size_t len,
//                      doctok::Reference<Properties>::Pointer_t ref);

    sal_uInt32          size();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
            GetNumberingRules(sal_Int32 nListId);
};
typedef boost::shared_ptr< ListTable >          ListTablePtr;
}

#endif //
