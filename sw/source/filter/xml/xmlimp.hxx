/************************************************************************
 *
 *  $RCSfile: xmlimp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-07 14:05:53 $
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

#ifndef _XMLIMP_HXX
#define _XMLIMP_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

class SwDoc;
class SwPaM;
class SvXMLUnitConverter;
class SvXMLTokenMap;
class SvXMLImportItemMapper;
class SfxItemSet;
class SwNodeIndex;
class XMLTextImportHelper;

class SwXMLImport: public SvXMLImport
{
    SwNodeIndex             *pSttNdIdx;

    SvXMLUnitConverter      *pTwipUnitConv;
    SvXMLImportItemMapper   *pTableItemMapper;// paragraph item import
    SvXMLTokenMap           *pDocElemTokenMap;
    SvXMLTokenMap           *pTableElemTokenMap;

    SvXMLItemMapEntriesRef  xTableItemMap;
    SvXMLItemMapEntriesRef  xTableColItemMap;
    SvXMLItemMapEntriesRef  xTableRowItemMap;
    SvXMLItemMapEntriesRef  xTableCellItemMap;

    SvXMLImportContextRef   xStyles;
    SvXMLImportContextRef   xAutoStyles;

    sal_uInt16              nStyleFamilyMask;// Mask of styles to load
    sal_Bool                bLoadDoc : 1;   // Load doc or styles only
    sal_Bool                bInsert : 1;    // Insert mode. If styles are
                                            // loaded only sal_False means that
                                            // existing styles will be
                                            // overwritten.
    sal_Bool                bAutoStylesValid : 1;


    void                    _InitItemImport();
    void                    _FinitItemImport();

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual XMLTextImportHelper* CreateTextImport();

    virtual XMLShapeImportHelper* CreateShapeImport();

public:

    SwXMLImport( SwDoc& rDoc, const SwPaM& rPaM, sal_Bool bLoadDoc,
                 sal_Bool bInsertMode, sal_uInt16 nStyleFamMask,
                 const ::com::sun::star::uno::Reference<
                     ::com::sun::star::frame::XModel > & rModel );

    ~SwXMLImport();

    void                    InsertStyles( sal_Bool bAuto );
    void                    FinishStyles();
    // namespace office
    SvXMLImportContext *CreateMetaContext( const ::rtl::OUString& rLocalName );
    SvXMLImportContext *CreateStylesContext(
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                sal_Bool bAuto );
    SvXMLImportContext *CreateMasterStylesContext(
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext *CreateBodyContext( const ::rtl::OUString& rLocalName );
    sal_uInt16 GetStyleFamilyMask() const { return nStyleFamilyMask; }
    sal_Bool IsInsertMode() const { return bInsert; }
    sal_Bool IsStylesOnlyMode() const { return !bLoadDoc; }

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;
    inline const SvXMLImportItemMapper& GetTableItemMapper() const;
    SvXMLImportContext *CreateTableItemImportContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                sal_uInt16 nSubFamily, SfxItemSet& rItemSet );

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();

    sal_Bool FindAutomaticStyle( sal_uInt16 nFamily,
                             const ::rtl::OUString& rName,
                             const SfxItemSet **ppItemSet=0,
                             ::rtl::OUString *pParent=0 ) const;
};

inline const SvXMLUnitConverter& SwXMLImport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}

inline const SvXMLImportItemMapper& SwXMLImport::GetTableItemMapper() const
{
    return *pTableItemMapper;
}

#endif  //  _XMLIMP_HXX
