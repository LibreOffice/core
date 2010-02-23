/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_LISTTABLE_HXX
#define INCLUDED_LISTTABLE_HXX

#include <WriterFilterDllApi.hxx>
#include <PropertyMap.hxx>
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
    rtl::OUString GetStyleName( sal_Int32 nListId );
    void CreateNumberingRules( );
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
            GetNumberingRules(sal_Int32 nListId);

    void setOverwriteLevel(sal_Int32 nAbstractNumId, sal_Int32 nLevel);
    void resetOverwrite();
};
typedef boost::shared_ptr< ListTable >          ListTablePtr;
}}

#endif //
