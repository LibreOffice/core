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
#ifndef INCLUDED_SECTIONCOLUMNHANDLER_HXX
#define INCLUDED_SECTIONCOLUMNHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>


namespace writerfilter {
namespace dmapper
{
class PropertyMap;

struct _Column
{
    sal_Int32 nWidth;
    sal_Int32 nSpace;
};


class WRITERFILTER_DLLPRIVATE SectionColumnHandler : public Properties
{
    bool        bEqualWidth;
    sal_Int32   nSpace;
    sal_Int32   nNum;
    bool        bSep;
    std::vector<_Column> aCols;

    _Column   aTempColumn;    

public:
    SectionColumnHandler();
    virtual ~SectionColumnHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    bool        IsEqualWidth() const { return bEqualWidth; }
    sal_Int32   GetSpace() const { return nSpace; }
    sal_Int32   GetNum() const { return nNum; }
    bool        IsSeparator() const { return bSep; }
    
    const std::vector<_Column>& GetColumns() const { return aCols;}

};
typedef boost::shared_ptr< SectionColumnHandler >          SectionColumnHandlerPtr;
}}

#endif //
