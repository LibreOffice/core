/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLParserState.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:05:35 $
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
#ifndef INCLUDE_OOXML_PARSER_STATE_HXX
#define INCLUDE_OOXML_PARSER_STATE_HXX

#include <ooxml/OOXMLDocument.hxx>
#include "OOXMLPropertySetImpl.hxx"

namespace writerfilter {
namespace ooxml
{

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
    rtl::OUString msXNoteId;
    OOXMLPropertySet::Pointer_t mpCharacterProps;
    OOXMLPropertySet::Pointer_t mpTableProps;

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

    void incContextCount();
    const string getHandle() const;
    void setHandle();
    unsigned int getContextCount() const;

    void setDocument(OOXMLDocument * pDocument);
    OOXMLDocument * getDocument() const;

    void setXNoteId(const rtl::OUString & rId);
    const rtl::OUString & getXNoteId() const;

    void newCharacterProperty(const Id & rId, OOXMLValue::Pointer_t pVal);
    void resolveCharacterProperties(Stream & rStream);
    void setCharacterProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveTableProperties(Stream & rStream);
    void setTableProperties(OOXMLPropertySet::Pointer_t pProps);

    string toString() const;
};

}}

#endif // INCLUDE_OOXML_PARSER_STATE_HXX
