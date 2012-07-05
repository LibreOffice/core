/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* PageSpan: Stores (and writes) page-based information (e.g.: margins,
 * headers/footers)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _PAGESPAN_H
#define _PAGESPAN_H
#include <libwpd/libwpd.h>
#include <vector>

class DocumentElement;
class OdfDocumentHandler;

class PageSpan
{
public:
    PageSpan(const WPXPropertyList &xPropList);
    virtual ~PageSpan();
    void writePageLayout(const int iNum, OdfDocumentHandler *pHandler) const;
    void writeMasterPages(const int iStartingNum, const int iPageLayoutNum, const bool bLastPageSpan, OdfDocumentHandler *pHandler) const;
    int getSpan() const;

    void setHeaderContent(std::vector<DocumentElement *> * pHeaderContent);
    void setFooterContent(std::vector<DocumentElement *> * pFooterContent);
    void setHeaderLeftContent(std::vector<DocumentElement *> * pHeaderContent);
    void setFooterLeftContent(std::vector<DocumentElement *> * pFooterContent);
protected:
    void _writeHeaderFooter(const char *headerFooterTagName, const std::vector<DocumentElement *> & headerFooterContent,
                            OdfDocumentHandler *pHandler) const;
private:
    WPXPropertyList mxPropList;
    std::vector<DocumentElement *> * mpHeaderContent;
    std::vector<DocumentElement *> * mpFooterContent;
    std::vector<DocumentElement *> * mpHeaderLeftContent;
    std::vector<DocumentElement *> * mpFooterLeftContent;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
