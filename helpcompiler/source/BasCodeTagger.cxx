/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <BasCodeTagger.hxx>

LibXmlTreeWalker::LibXmlTreeWalker( xmlDocPtr doc )
{
    if ( doc == NULL )
        throw BasicCodeTagger::NULL_DOCUMENT;
    m_pCurrentNode = xmlDocGetRootElement( doc );
    if ( m_pCurrentNode == NULL )
        throw BasicCodeTagger::EMPTY_DOCUMENT;
    else if ( m_pCurrentNode->xmlChildrenNode != NULL )
        m_Queue.push_back( m_pCurrentNode->xmlChildrenNode );
    nextNode();
}

void LibXmlTreeWalker::nextNode()
{

      
    if ( m_pCurrentNode->next == NULL )
    {
        m_pCurrentNode = m_Queue.front();
        m_Queue.pop_front();
    }
    else
        m_pCurrentNode = m_pCurrentNode->next;
    
    if ( m_pCurrentNode->xmlChildrenNode != NULL )
        m_Queue.push_back( m_pCurrentNode->xmlChildrenNode );
}

void LibXmlTreeWalker::ignoreCurrNodesChildren()
{
    if ( m_pCurrentNode->xmlChildrenNode != NULL )
          m_Queue.pop_back();
}

bool LibXmlTreeWalker::end()
{
    return m_pCurrentNode->next == NULL && m_Queue.empty();
}

xmlNodePtr LibXmlTreeWalker::currentNode()
{
    return m_pCurrentNode;
}



BasicCodeTagger::BasicCodeTagger( xmlDocPtr rootDoc ):
    m_Highlighter(HIGHLIGHT_BASIC)
{
      if ( rootDoc == NULL )
          throw NULL_DOCUMENT;
      m_pDocument = rootDoc;
    m_pXmlTreeWalker = NULL;
    m_bTaggingCompleted = false;

}

BasicCodeTagger::~BasicCodeTagger()
{
    if ( m_pXmlTreeWalker != NULL )
      delete m_pXmlTreeWalker;
}

/*!
 *    Assumes m_pDocument is valid. Handles m_pXmlTreeWalker and m_BasicCodeContainerTags members.
 */
void BasicCodeTagger::getBasicCodeContainerNodes()
{
      xmlNodePtr currentNode;

    m_BasicCodeContainerTags.clear();

    if ( m_pXmlTreeWalker != NULL )
      delete m_pXmlTreeWalker;
    m_pXmlTreeWalker = new LibXmlTreeWalker( m_pDocument );

    currentNode = m_pXmlTreeWalker->currentNode();
    if ( !( xmlStrcmp( currentNode->name, (const xmlChar*) "bascode" ) ) )
    { 
        m_BasicCodeContainerTags.push_back( currentNode ); 
    }
    while ( !m_pXmlTreeWalker->end() )
    {
          m_pXmlTreeWalker->nextNode();
        if ( !( xmlStrcmp( m_pXmlTreeWalker->currentNode()->name, (const xmlChar*) "bascode" ) ) )
        { 
            m_BasicCodeContainerTags.push_back( m_pXmlTreeWalker->currentNode() ); 
            m_pXmlTreeWalker->ignoreCurrNodesChildren();
        }
    }
}


/*!
 *  For each <bascode> this method iterates trough it's <paragraph> tags and "inserts" <item> tags according
 *  to the Basic code syntax found in that paragraph.
 */
void BasicCodeTagger::tagBasCodeParagraphs()
{
    
    xmlNodePtr currBascodeNode;
    xmlNodePtr currParagraph;
    while ( !m_BasicCodeContainerTags.empty() )
    {
        currBascodeNode = m_BasicCodeContainerTags.front();
        currParagraph = currBascodeNode->xmlChildrenNode; 
        while ( currParagraph != NULL )
        {
            tagParagraph( currParagraph );
            currParagraph=currParagraph->next;
        }
        m_BasicCodeContainerTags.pop_front(); 
    }
}


void BasicCodeTagger::tagParagraph( xmlNodePtr paragraph )
{
    
    xmlChar* codeSnippet;
    codeSnippet = xmlNodeListGetString( m_pDocument, paragraph->xmlChildrenNode, 1 );
    if ( codeSnippet == NULL )
    {
        return; 
    }
    
    xmlNodePtr curNode = paragraph->xmlChildrenNode;
    xmlNodePtr sibling;
    while ( curNode != NULL )
    {
        sibling = curNode->next;
        xmlUnlinkNode( curNode );
        xmlFreeNode( curNode );
        curNode = sibling;
    }

    
    OUString strLine( reinterpret_cast<const sal_Char*>(codeSnippet),
                                strlen(reinterpret_cast<const char*>(codeSnippet)),
                                RTL_TEXTENCODING_UTF8 );
    std::vector<HighlightPortion> portions;
    m_Highlighter.getHighlightPortions( strLine, portions );
    for (std::vector<HighlightPortion>::iterator i(portions.begin());
         i != portions.end(); ++i)
    {
        OString sToken(OUStringToOString(strLine.copy(i->nBegin, i->nEnd-i->nBegin), RTL_TEXTENCODING_UTF8));
        xmlNodePtr text = xmlNewText((const xmlChar*)sToken.getStr());
        if ( i->tokenType != TT_WHITESPACE )
        {
            xmlChar* typeStr = getTypeString( i->tokenType );
            curNode = xmlNewTextChild( paragraph, 0, (xmlChar*)"item", 0 );
            xmlNewProp( curNode, (xmlChar*)"type", typeStr );
            xmlAddChild( curNode, text );
            xmlFree( typeStr );
        }
        else
            xmlAddChild( paragraph, text );
    }
    xmlFree( codeSnippet );
}


/*!
 *    This is the "main" function of BasicCodeTagger.
 */
void BasicCodeTagger::tagBasicCodes()
{
      if ( m_bTaggingCompleted )
        return;
    
    try
    {
        getBasicCodeContainerNodes();
    }
    catch (TaggerException &ex)
    {
          std::cout << "BasCodeTagger error occured. Error code:" << ex << std::endl;
    }

    
    tagBasCodeParagraphs();
    m_bTaggingCompleted = true;
}


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
