/*************************************************************************
 *
 *  $RCSfile: XMLLineNumberingImportContext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:55:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLLINENUMBERINGIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLLINENUMBERINGIMPORTCONTEXT_HXX_

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }


enum LineNumberingToken
{
    XML_TOK_LINENUMBERING_STYLE_NAME,
    XML_TOK_LINENUMBERING_NUMBER_LINES,
    XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES,
    XML_TOK_LINENUMBERING_COUNT_IN_TEXT_BOXES,
    XML_TOK_LINENUMBERING_RESTART_NUMBERING,
    XML_TOK_LINENUMBERING_OFFSET,
    XML_TOK_LINENUMBERING_NUM_FORMAT,
    XML_TOK_LINENUMBERING_NUM_LETTER_SYNC,
    XML_TOK_LINENUMBERING_NUMBER_POSITION,
    XML_TOK_LINENUMBERING_INCREMENT
//  XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION,
//  XML_TOK_LINENUMBERING_INCREMENT,
//  XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR,
};


/** import <text:linenumbering-configuration> elements */
class XMLLineNumberingImportContext : public SvXMLStyleContext
{
    const ::rtl::OUString sCharStyleName;
    const ::rtl::OUString sCountEmptyLines;
    const ::rtl::OUString sCountLinesInFrames;
    const ::rtl::OUString sDistance;
    const ::rtl::OUString sInterval;
    const ::rtl::OUString sSeparatorText;
    const ::rtl::OUString sNumberPosition;
    const ::rtl::OUString sNumberingType;
    const ::rtl::OUString sIsOn;
    const ::rtl::OUString sRestartAtEachPage;
    const ::rtl::OUString sSeparatorInterval;

    ::rtl::OUString sStyleName;
    ::rtl::OUString sNumFormat;
    ::rtl::OUString sNumLetterSync;
    ::rtl::OUString sSeparator;
    sal_Int32 nOffset;
    sal_Int16 nNumberPosition;
    sal_Int16 nIncrement;
    sal_Int16 nSeparatorIncrement;
    sal_Bool bNumberLines;
    sal_Bool bCountEmptyLines;
    sal_Bool bCountInFloatingFrames;
    sal_Bool bRestartNumbering;

public:

    TYPEINFO();

    XMLLineNumberingImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    ~XMLLineNumberingImportContext();

    // to be used by child context: set separator info
    void SetSeparatorText(const ::rtl::OUString& sText);
    void SetSeparatorIncrement(sal_Int16 nIncr);

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    void ProcessAttribute(
        enum LineNumberingToken eToken,
        ::rtl::OUString sValue);

    virtual void CreateAndInsert(sal_Bool bOverwrite);

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttribute(
        const ::rtl::OUString sLocalName,
        const ::rtl::OUString sValue);
};

#endif
