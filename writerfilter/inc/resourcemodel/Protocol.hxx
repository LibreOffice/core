/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
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
#ifndef INCLUDE_WRITERFILTER_PROTOCOL_HXX
#define INCLUDE_WRITERFILTER_PROTOCOL_HXX

#ifdef DEBUG

#include <boost/shared_ptr.hpp>
#include "WW8ResourceModel.hxx"
#include "TagLogger.hxx"
namespace writerfilter
{

class StreamProtocol : public Stream
{
    Stream * m_pStream;
    TagLogger::Pointer_t m_pTagLogger;

public:
    typedef boost::shared_ptr<StreamProtocol> Pointer_t;
    
    StreamProtocol(Stream * pStream, TagLogger::Pointer_t pTagLogger);
    virtual ~StreamProtocol();

    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name, 
                       writerfilter::Reference<Table>::Pointer_t ref);
    virtual void substream(Id name, 
                           writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & rInfo);
    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape();
};

class PropertiesProtocol : public Properties
{
    Properties * m_pProperties;
    TagLogger::Pointer_t m_pTagLogger;

public:
    typedef boost::shared_ptr<PropertiesProtocol> Pointer_t;
    
    PropertiesProtocol(Properties * pProperties, TagLogger::Pointer_t pTagLogger);
    virtual ~PropertiesProtocol();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class TableProtocol : public Table
{
    Table * m_pTable;
    TagLogger::Pointer_t m_pTagLogger;

public:
    typedef boost::shared_ptr<TableProtocol> Pointer_t;
    
    TableProtocol(Table * pTable, TagLogger::Pointer_t pTagLogger);
    virtual ~TableProtocol();

    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);
};

}

#endif // DEBUG
#endif // INCLUDE_WRITERFILTER_PROTOCOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
