/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ListTable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:39:55 $
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
#include <resourcemodel/WW8ResourceModel.hxx>

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

namespace writerfilter {
namespace dmapper
{
class DomainMapper;
struct ListTable_Impl;
class WRITERFILTER_DLLPRIVATE ListTable :
        public Properties,
        public Table
{
    ListTable_Impl   *m_pImpl;
    sal_Int32       m_nOverwriteListId;
    sal_Int32       m_nOverwriteLevel;

    void    ApplyLevelValues( sal_Int32 nId, sal_Int32 nIntValue);
public:
    ListTable(
            DomainMapper& rDMapper,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory);
    virtual ~ListTable();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // BinaryObj
//    virtual void data(const sal_Int8* buf, size_t len,
//                      writerfilter::Reference<Properties>::Pointer_t ref);

    sal_uInt32          size() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
            GetNumberingRules(sal_Int32 nListId);

    void setOverwriteLevel(sal_Int32 nAbstractNumId, sal_Int32 nLevel);
    void resetOverwrite();
};
typedef boost::shared_ptr< ListTable >          ListTablePtr;
}}

#endif //
