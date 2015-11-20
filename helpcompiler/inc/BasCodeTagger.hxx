/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_HELPCOMPILER_INC_BASCODETAGGER_HXX
#define INCLUDED_HELPCOMPILER_INC_BASCODETAGGER_HXX

#include <iostream>
#include <cstdlib>
#include <string>
#include <list>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <rtl/ustring.hxx>
#include <comphelper/syntaxhighlight.hxx>
#include <helpcompiler/dllapi.h>

class BasicCodeTagger;
class LibXmlTreeWalker;

//!Tagger class.
class L10N_DLLPUBLIC BasicCodeTagger
{
  private:
    xmlDocPtr             m_pDocument;
    std::list<xmlNodePtr> m_BasicCodeContainerTags;
    LibXmlTreeWalker   *m_pXmlTreeWalker;
    SyntaxHighlighter     m_Highlighter;
    bool m_bTaggingCompleted;
    void tagParagraph( xmlNodePtr paragraph );
    static xmlChar* getTypeString( TokenTypes tokenType );
    void getBasicCodeContainerNodes();
    void tagBasCodeParagraphs();

  public:
    enum TaggerException { FILE_WRITING, NULL_DOCUMENT, EMPTY_DOCUMENT };
    BasicCodeTagger( xmlDocPtr rootDoc );
    ~BasicCodeTagger();
    void tagBasicCodes();
};

//================LibXmlTreeWalker===========================================================

class L10N_DLLPUBLIC LibXmlTreeWalker
{
  private:
    xmlNodePtr            m_pCurrentNode;
    std::list<xmlNodePtr> m_Queue; //!Queue for breath-first search

  public:
    LibXmlTreeWalker( xmlDocPtr doc );
    ~LibXmlTreeWalker() {}
    void nextNode();
    xmlNodePtr currentNode() { return m_pCurrentNode;}
    bool end();
    void ignoreCurrNodesChildren();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
