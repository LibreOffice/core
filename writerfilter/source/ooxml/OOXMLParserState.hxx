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


#ifndef INCLUDE_OOXML_PARSER_STATE_HXX
#define INCLUDE_OOXML_PARSER_STATE_HXX

#include <stack>
#include <ooxml/OOXMLDocument.hxx>
#include "OOXMLPropertySetImpl.hxx"

#ifdef DEBUG
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/XPathLogger.hxx>
#endif

namespace writerfilter {
namespace ooxml
{

using ::std::stack;

class OOXMLParserState
{
    bool mbInSectionGroup;
    bool mbInParagraphGroup;
    bool mbInCharacterGroup;
    bool mbLastParagraphInSection;
    bool mbForwardEvents;
    unsigned int mnContexts;
    unsigned int mnHandle;
    OOXMLDocument * mpDocument;
    rtl::OUString msTarget;
    OOXMLPropertySet::Pointer_t mpCharacterProps;
    stack<OOXMLPropertySet::Pointer_t> mCellProps;
    stack<OOXMLPropertySet::Pointer_t> mRowProps;
    stack<OOXMLPropertySet::Pointer_t> mTableProps;
#ifdef DEBUG
    XPathLogger m_xPathLogger;
#endif

public:
    typedef boost::shared_ptr<OOXMLParserState> Pointer_t;

    OOXMLParserState();
    virtual ~OOXMLParserState();

    bool isInSectionGroup() const;
    void setInSectionGroup(bool bInSectionGroup);

    void setLastParagraphInSection(bool bLastParagraphInSection);
    bool isLastParagraphInSection() const;

    bool isInParagraphGroup() const;
    void setInParagraphGroup(bool bInParagraphGroup);

    bool isInCharacterGroup() const;
    void setInCharacterGroup(bool bInCharacterGroup);

    void setForwardEvents(bool bForwardEvents);
    bool isForwardEvents() const;

    const string getHandle() const;
    void setHandle();

    void setDocument(OOXMLDocument * pDocument);
    OOXMLDocument * getDocument() const;

    const rtl::OUString & getTarget() const;

    void resolveCharacterProperties(Stream & rStream);
    void setCharacterProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveCellProperties(Stream & rStream);
    void setCellProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveRowProperties(Stream & rStream);
    void setRowProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveTableProperties(Stream & rStream);
    void setTableProperties(OOXMLPropertySet::Pointer_t pProps);

    void startTable();
    void endTable();

    void incContextCount();

#ifdef DEBUG
public:
    unsigned int getContextCount() const;
    string toString() const;
    XMLTag::Pointer_t toTag() const;
    XPathLogger & getXPathLogger();
#endif

};

}}

#endif // INCLUDE_OOXML_PARSER_STATE_HXX
