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
#ifndef INCLUDED_CELLMARGINHANDLER_HXX
#define INCLUDED_CELLMARGINHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE CellMarginHandler : public Properties
{
private:
    sal_Int32   m_nValue;

public:
    sal_Int32   m_nLeftMargin;
    bool        m_bLeftMarginValid;
    sal_Int32   m_nRightMargin;
    bool        m_bRightMarginValid;
    sal_Int32   m_nTopMargin;
    bool        m_bTopMarginValid;
    sal_Int32   m_nBottomMargin;
    bool        m_bBottomMarginValid;

public:
    CellMarginHandler( );
    virtual ~CellMarginHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    ::boost::shared_ptr<TablePropertyMap>            getProperties();

};
typedef boost::shared_ptr< CellMarginHandler >          CellMarginHandlerPtr;
}}

#endif //
