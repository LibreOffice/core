/* OdtGenerator: Collects sections and runs of text from a
 * wordperfect file (and styles to go along with them) and writes them
 * to a target file
 *
 * Copyright (C) 2002-2004 William Lachance (wrlach@gmail.com)
 * Copyright (C) 2003-2004 Net Integration Technologies (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef _ODTGENERATOR_H
#define _ODTGENERATOR_H

#include <libwpd/libwpd.h>

#include "OdfDocumentHandler.hxx"


typedef bool (*OdfEmbeddedObject)(const WPXBinaryData &data, OdfDocumentHandler *pHandler, const OdfStreamType streamType);

class OdtGeneratorPrivate;

class OdtGenerator : public WPXDocumentInterface
{
public:
    OdtGenerator(OdfDocumentHandler *pHandler, const OdfStreamType streamType);
    ~OdtGenerator();

    // WPXDocumentInterface's implementation
     void setDocumentMetaData(const WPXPropertyList &propList);
    void startDocument();
    void endDocument();

    void definePageStyle(const WPXPropertyList&);
    void openPageSpan(const WPXPropertyList &propList);
    void closePageSpan();

    void defineSectionStyle(const WPXPropertyList&, const WPXPropertyListVector&);
    void openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
    void closeSection();

    void openHeader(const WPXPropertyList &propList);
    void closeHeader();
    void openFooter(const WPXPropertyList &propList);
    void closeFooter();

    void defineParagraphStyle(const WPXPropertyList&, const WPXPropertyListVector&);
    void openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
    void closeParagraph();

    void defineCharacterStyle(const WPXPropertyList&);
    void openSpan(const WPXPropertyList &propList);
    void closeSpan();

    void insertTab();
    void insertSpace();
    void insertText(const WPXString &text);
     void insertLineBreak();
    void insertField(const WPXString &type, const WPXPropertyList &propList);

    void defineOrderedListLevel(const WPXPropertyList &propList);
    void defineUnorderedListLevel(const WPXPropertyList &propList);
    void openOrderedListLevel(const WPXPropertyList &propList);
    void openUnorderedListLevel(const WPXPropertyList &propList);
    void closeOrderedListLevel();
    void closeUnorderedListLevel();
    void openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
    void closeListElement();

    void openFootnote(const WPXPropertyList &propList);
    void closeFootnote();
    void openEndnote(const WPXPropertyList &propList);
    void closeEndnote();
    void openComment(const WPXPropertyList &propList);
    void closeComment();
    void openTextBox(const WPXPropertyList &propList);
    void closeTextBox();

     void openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
     void openTableRow(const WPXPropertyList &propList);
    void closeTableRow();
     void openTableCell(const WPXPropertyList &propList);
    void closeTableCell();
    void insertCoveredTableCell(const WPXPropertyList &propList);
     void closeTable();

    void openFrame(const WPXPropertyList & propList);
    void closeFrame();

    void insertBinaryObject(const WPXPropertyList &propList, const WPXBinaryData &data);
    void insertEquation(const WPXPropertyList &propList, const WPXString &data);

    // Register special converter for certain embedded binary objects
    void registerEmbeddedObjectHandler(const WPXString &mimeType, OdfEmbeddedObject objectHandler);

private:
    OdtGeneratorPrivate *mpImpl;
};
#endif
