/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <BasCodeTagger.hxx>

LibXmlTreeWalker::LibXmlTreeWalker( xmlDocPtr doc )
{
    if ( doc == nullptr )
        throw BasicCodeTagger::NULL_DOCUMENT;
    m_pCurrentNode = xmlDocGetRootElement( doc );
    if ( m_pCurrentNode == nullptr )
        throw BasicCodeTagger::EMPTY_DOCUMENT;
    else if ( m_pCurrentNode->xmlChildrenNode != nullptr )
        m_Queue.push_back( m_pCurrentNode->xmlChildrenNode );
    nextNode();
}

void LibXmlTreeWalker::nextNode()
{

      //next node
    if ( m_pCurrentNode->next == nullptr )
    {
        m_pCurrentNode = m_Queue.front();
        m_Queue.pop_front();
    }
    else
        m_pCurrentNode = m_pCurrentNode->next;
    //queue children if they exist
    if ( m_pCurrentNode->xmlChildrenNode != nullptr )
        m_Queue.push_back( m_pCurrentNode->xmlChildrenNode );
}

void LibXmlTreeWalker::ignoreCurrNodesChildren()
{
    if ( m_pCurrentNode->xmlChildrenNode != nullptr )
          m_Queue.pop_back();
}

bool LibXmlTreeWalker::end()
{
    return m_pCurrentNode->next == nullptr && m_Queue.empty();
}




BasicCodeTagger::BasicCodeTagger( xmlDocPtr rootDoc ):
    m_Highlighter(HIGHLIGHT_BASIC)
{
      if ( rootDoc == nullptr )
          throw NULL_DOCUMENT;
      m_pDocument = rootDoc;
    m_pXmlTreeWalker = nullptr;
    m_bTaggingCompleted = false;

}

BasicCodeTagger::~BasicCodeTagger()
{
    if ( m_pXmlTreeWalker != nullptr )
      delete m_pXmlTreeWalker;
}
//!Gathers all the <bascode> tag nodes from xml tree.
/*!
 *    Assumes m_pDocument is valid. Handles m_pXmlTreeWalker and m_BasicCodeContainerTags members.
 */
void BasicCodeTagger::getBasicCodeContainerNodes()
{
      xmlNodePtr currentNode;

    m_BasicCodeContainerTags.clear();

    if ( m_pXmlTreeWalker != nullptr )
      delete m_pXmlTreeWalker;
    m_pXmlTreeWalker = new LibXmlTreeWalker( m_pDocument );

    currentNode = m_pXmlTreeWalker->currentNode();
    if ( !( xmlStrcmp( currentNode->name, reinterpret_cast<const xmlChar*>("bascode") ) ) )
    { //Found <bascode>
        m_BasicCodeContainerTags.push_back( currentNode ); //it goes to the end of the list
    }
    while ( !m_pXmlTreeWalker->end() )
    {
          m_pXmlTreeWalker->nextNode();
        if ( !( xmlStrcmp( m_pXmlTreeWalker->currentNode()->name, reinterpret_cast<const xmlChar*>("bascode") ) ) )
        { //Found <bascode>
            m_BasicCodeContainerTags.push_back( m_pXmlTreeWalker->currentNode() ); //it goes to the end of the list
            m_pXmlTreeWalker->ignoreCurrNodesChildren();
        }
    }
}

//! Extracts Basic Codes contained in <bascode> tags.
/*!
 *  For each <bascode> this method iterates through it's <paragraph> tags and "inserts" <item> tags according
 *  to the Basic code syntax found in that paragraph.
 */
void BasicCodeTagger::tagBasCodeParagraphs()
{
    //helper variables
    xmlNodePtr currBascodeNode;
    xmlNodePtr currParagraph;
    while ( !m_BasicCodeContainerTags.empty() )
    {
        currBascodeNode = m_BasicCodeContainerTags.front();
        currParagraph = currBascodeNode->xmlChildrenNode; //first <paragraph>
        while ( currParagraph != nullptr )
        {
            tagParagraph( currParagraph );
            currParagraph=currParagraph->next;
        }
        m_BasicCodeContainerTags.pop_front(); //next element
    }
}

//! Used by tagBasCodeParagraphs(). It does the work on the current paragraph containing Basic code.
void BasicCodeTagger::tagParagraph( xmlNodePtr paragraph )
{
    //1. get paragraph text
    xmlChar* codeSnippet;
    codeSnippet = xmlNodeListGetString( m_pDocument, paragraph->xmlChildrenNode, 1 );
    if ( codeSnippet == nullptr )
    {
        return; //no text, nothing more to do here
    }
    //2. delete every child from paragraph (except attributes)
    xmlNodePtr curNode = paragraph->xmlChildrenNode;
    xmlNodePtr sibling;
    while ( curNode != nullptr )
    {
        sibling = curNode->next;
        xmlUnlinkNode( curNode );
        xmlFreeNode( curNode );
        curNode = sibling;
    }

    //3. create new paragraph content
    OUString strLine( reinterpret_cast<const sal_Char*>(codeSnippet),
                                strlen(reinterpret_cast<const char*>(codeSnippet)),
                                RTL_TEXTENCODING_UTF8 );
    std::vector<HighlightPortion> portions;
    m_Highlighter.getHighlightPortions( strLine, portions );
    for (std::vector<HighlightPortion>::iterator i(portions.begin());
         i != portions.end(); ++i)
    {
        OString sToken(OUStringToOString(strLine.copy(i->nBegin, i->nEnd-i->nBegin), RTL_TEXTENCODING_UTF8));
        xmlNodePtr text = xmlNewText(reinterpret_cast<const xmlChar*>(sToken.getStr()));
        if ( i->tokenType != TT_WHITESPACE )
        {
            xmlChar* typeStr = getTypeString( i->tokenType );
            curNode = xmlNewTextChild( paragraph, nullptr, reinterpret_cast<xmlChar const *>("item"), nullptr );
            xmlNewProp( curNode, reinterpret_cast<xmlChar const *>("type"), typeStr );
            xmlAddChild( curNode, text );
            xmlFree( typeStr );
        }
        else
            xmlAddChild( paragraph, text );
    }
    xmlFree( codeSnippet );
}

//! Manages tagging process.
/*!
 *    This is the "main" function of BasicCodeTagger.
 */
void BasicCodeTagger::tagBasicCodes()
{
      if ( m_bTaggingCompleted )
        return;
    //gather <bascode> nodes
    try
    {
        getBasicCodeContainerNodes();
    }
    catch (TaggerException &ex)
    {
          std::cout << "BasCodeTagger error occurred. Error code:" << ex << std::endl;
    }

    //tag basic code paragraphs in <bascode> tag
    tagBasCodeParagraphs();
    m_bTaggingCompleted = true;
}

//! Converts SyntaxHighlighter's TokenTypes enum to a type string for <item type=... >
xmlChar* BasicCodeTagger::getTypeString( TokenTypes tokenType )
{
    const char* str;
    switch ( tokenType )
    {
        case TT_UNKNOWN :
            str = "unknown";
            break;
        case TT_IDENTIFIER :
            str = "identifier";
            break;
        case TT_WHITESPACE :
            str = "whitespace";
            break;
        case TT_NUMBER :
            str = "number";
            break;
        case TT_STRING :
            str = "string";
            break;
        case TT_EOL :
            str = "eol";
            break;
        case TT_COMMENT :
            str = "comment";
            break;
        case TT_ERROR :
            str = "error";
            break;
        case TT_OPERATOR :
            str = "operator";
            break;
        case TT_KEYWORDS :
            str = "keyword";
            break;
        case TT_PARAMETER :
            str = "parameter";
            break;
        default :
            str = "unknown";
            break;
    }
    return xmlCharStrdup( str );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
