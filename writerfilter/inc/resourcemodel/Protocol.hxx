/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
