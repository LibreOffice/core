/*************************************************************************
 *
 *  $RCSfile: txtparae.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-31 09:00:40 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTDARR_LONGS_DECL
#define _SVSTDARR_LONGS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif


#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_TXTEXPPR_HXX
#include "txtexppr.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#include "XMLAnchorTypePropHdl.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_TXTFLDE_HXX
#include "txtflde.hxx"
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include "txtprmap.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#include "XMLTextNumRuleInfo.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include "XMLTextListAutoStylePool.hxx"
#endif
#ifndef _XMLOFF_TXTPARAE_HXX
#include "txtparae.hxx"
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::util;

typedef OUString *OUStringPtr;
SV_DECL_PTRARR_DEL( OUStrings_Impl, OUStringPtr, 20, 10 )
SV_IMPL_PTRARR( OUStrings_Impl, OUStringPtr )

SV_DECL_PTRARR_SORT_DEL( OUStringsSort_Impl, OUStringPtr, 20, 10 )
SV_IMPL_OP_PTRARR_SORT( OUStringsSort_Impl, OUStringPtr )

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                    const Reference < XPropertySet > & rPropSet )
{
    UniReference < XMLPropertySetMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        xPropMapper = GetTextPropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        xPropMapper = GetAutoFramePropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper()->getPropertySetMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );

    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );

    if( xPropStates.size() > 0L )
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            rPropSet->getPropertySetInfo();
        OUString sParent, sCondParent;
        Any aAny;
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
            if( xPropSetInfo->hasPropertyByName( sParaStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sParaStyleName );
                aAny >>= sParent;
            }
            if( xPropSetInfo->hasPropertyByName( sParaConditionalStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sParaConditionalStyleName );
                aAny >>= sCondParent;
            }
            if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
            {
                aAny = rPropSet->getPropertyValue( sNumberingRules );
                Reference < XIndexReplace > xNumRule;
                aAny >>= xNumRule;
                if( xNumRule.is() )
                {
                    Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                    OUString sName;
                    if( xNamed.is() )
                        sName = xNamed->getName();
                    if( !sName.getLength() || !pListAutoPool->HasName( sName ) )
                    {
                        // HasName returns false if the num rule is a style,
                        // because all style names have been registered while
                        // all automatic rules not.
                        pListAutoPool->Add( xNumRule );
                    }
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_TEXT:
            if( xPropSetInfo->hasPropertyByName( sCharStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sCharStyleName );
                aAny >>= sParent;
            }
            break;
        case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
            if( xPropSetInfo->hasPropertyByName( sFrameStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sFrameStyleName );
                aAny >>= sParent;
            }
            break;
        case XML_STYLE_FAMILY_TEXT_SECTION:
            ; // section styles have no parents
            break;
        }

        GetAutoStylePool().Add( nFamily, sParent, xPropStates );
        if( sCondParent.getLength() && sParent != sCondParent )
            GetAutoStylePool().Add( nFamily, sCondParent, xPropStates );
    }
}

OUString XMLTextParagraphExport::Find(
        sal_uInt16 nFamily,
           const Reference < XPropertySet > & rPropSet,
        const OUString& rParent ) const
{
    OUString sName( rParent );
    UniReference < XMLPropertySetMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        xPropMapper = GetTextPropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        xPropMapper = GetAutoFramePropMapper()->getPropertySetMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper()->getPropertySetMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );
    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );

    if( xPropStates.size() > 0L )
        sName = GetAutoStylePool().Find( nFamily, sName, xPropStates );

    return sName;
}

OUString XMLTextParagraphExport::FindTextStyle(
           const Reference < XPropertySet > & rPropSet ) const
{
    Reference< XPropertySetInfo > xPropSetInfo =
        rPropSet->getPropertySetInfo();

    OUString sStyle;
    if( xPropSetInfo->hasPropertyByName( sCharStyleName ) )
    {
        Any aAny = rPropSet->getPropertyValue( sCharStyleName );
        aAny >>= sStyle;
    }

    return Find( XML_STYLE_FAMILY_TEXT_TEXT, rPropSet, sStyle );
}


void XMLTextParagraphExport::exportListChange(
        const XMLTextNumRuleInfo& rPrevInfo,
        const XMLTextNumRuleInfo& rNextInfo )
{
    // end a list
    if( rPrevInfo.GetLevel() > 0 &&
        ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
          rNextInfo.GetLevel() < rPrevInfo.GetLevel() ||
             rNextInfo.IsRestart()  ) )
    {
        sal_Int16 nPrevLevel = rPrevInfo.GetLevel();
        sal_Int16 nNextLevel =
            ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
              rNextInfo.IsRestart() ) ?  0 : rNextInfo.GetLevel();

        DBG_ASSERT( pListElements &&
                pListElements->Count() >= 2*(nNextLevel-nPrevLevel),
                "SwXMLExport::ExportListChange: list elements missing" );

        for( sal_Int16 i=nPrevLevel; i > nNextLevel; i-- )
        {
            for( sal_uInt16 j=0; j<2; j++ )
            {
                OUString *pElem = (*pListElements)[pListElements->Count()-1];
                pListElements->Remove( pListElements->Count()-1 );

                GetExport().GetDocHandler()->ignorableWhitespace(
                        GetExport().sWS );
                GetExport().GetDocHandler()->endElement( *pElem );

                delete pElem;
            }
        }
    }

    // start a new list
    if( rNextInfo.GetLevel() > 0 &&
        ( !rPrevInfo.HasSameNumRules( rNextInfo ) ||
          rPrevInfo.GetLevel() < rNextInfo.GetLevel() ||
             rNextInfo.IsRestart() ) )
    {
        sal_Int16 nPrevLevel =
            ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
              rNextInfo.IsRestart() ) ? 0 : rPrevInfo.GetLevel();
        sal_Int16 nNextLevel = rNextInfo.GetLevel();

        // Find out whether this is the first application of the list or not.
        // For named lists, we use the internal name. For unnamed lists, we
        // use the generated name. This works well, because there are either
        // unnamed or either named lists only.
        sal_Bool bListExported = sal_False;
        OUString sName;
        if( rNextInfo.IsNamed() )
            sName = rNextInfo.GetName();
        else
            sName = pListAutoPool->Find( rNextInfo.GetNumRules() );
        DBG_ASSERT( sName.getLength(), "list without a name" );
        if( sName.getLength() )
        {
            bListExported = pExportedLists &&
                             pExportedLists->Seek_Entry( (OUString *)&sName );
            if( !bListExported )
            {
                if( !pExportedLists )
                    pExportedLists = new OUStringsSort_Impl;
                pExportedLists->Insert( new OUString(sName) );
            }
        }
        sal_Bool bContinue = !rNextInfo.IsRestart() && bListExported &&
                             !rPrevInfo.HasSameNumRules( rNextInfo );

        for( sal_Int16 i=nPrevLevel; i < nNextLevel; i++)
        {
            // <text:ordered-list> or <text:unordered-list>
            GetExport().CheckAttrList();
            if( 0 == i )
            {
                // For named list, the name might be the name of an automatic
                // rule, so we have to take a look into the style pool.
                // For unnamed lists, we have done this already.
                if( rNextInfo.IsNamed() )
                {
                    OUString sTmp( pListAutoPool->Find(
                                            rNextInfo.GetNumRules() ) );
                    if( sTmp.getLength() )
                        sName = sTmp;
                }
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name,
                                          sName );
            }
            if( bContinue && rNextInfo.IsOrdered() )
                GetExport().AddAttributeASCII( XML_NAMESPACE_TEXT,
                                               sXML_continue_numbering,
                                                  sXML_true );

            const sal_Char *pLName =
                    rNextInfo.IsOrdered() ? sXML_ordered_list
                                          : sXML_unordered_list;

            OUString *pElem = new OUString(
                    GetExport().GetNamespaceMap().GetQNameByKey(
                                        XML_NAMESPACE_TEXT,
                                        OUString::createFromAscii(pLName) ) );
            GetExport().GetDocHandler()->ignorableWhitespace(
                    GetExport().sWS );
            GetExport().GetDocHandler()->startElement(
                    *pElem, GetExport().GetXAttrList() );
            GetExport().ClearAttrList();

            if( !pListElements )
                pListElements = new OUStrings_Impl;
            pListElements->Insert( pElem, pListElements->Count() );

            // <text:list-header> or <text:list-item>
            GetExport().CheckAttrList();
            if( rNextInfo.HasStartValue() )
            {
                OUStringBuffer aBuffer;
                aBuffer.append( (sal_Int32)rNextInfo.GetStartValue() );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_start_value,
                              aBuffer.makeStringAndClear() );
            }
            pLName = (rNextInfo.IsNumbered() || i+1 < nNextLevel)
                        ? sXML_list_item
                        : sXML_list_header;
            pElem = new OUString(  GetExport().GetNamespaceMap().GetQNameByKey(
                                        XML_NAMESPACE_TEXT,
                                        OUString::createFromAscii(pLName) ) );
            GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
            GetExport().GetDocHandler()->startElement(
                    *pElem, GetExport().GetXAttrList() );
            GetExport().ClearAttrList();

            pListElements->Insert( pElem, pListElements->Count() );
        }
    }

    if( rNextInfo.GetLevel() > 0 && rNextInfo.IsNumbered() &&
        rPrevInfo.HasSameNumRules( rNextInfo ) &&
        rPrevInfo.GetLevel() >= rNextInfo.GetLevel() &&
        !rNextInfo.IsRestart() )
    {
        // </text:list-item> or </text:list-header>
        DBG_ASSERT( pListElements && pListElements->Count() >= 2,
                "SwXMLExport::ExportListChange: list elements missing" );

        OUString *pElem = (*pListElements)[pListElements->Count()-1];
        GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
        GetExport().GetDocHandler()->endElement( *pElem );

        pListElements->Remove( pListElements->Count()-1 );
        delete pElem;

        // <text:list-item>
        GetExport().CheckAttrList();
        pElem = new OUString( GetExport().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT,
                                OUString::createFromAscii(sXML_list_item) ) );
        GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
        GetExport().GetDocHandler()->startElement( *pElem,
                GetExport().GetXAttrList() );
        GetExport().ClearAttrList();

        pListElements->Insert( pElem, pListElements->Count() );
    }
}


void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    // currently, section import does not work properly; thus, we also
    // disable section import. Simply call exportSectionChange

    if (!bAutoStyles)
    {
        exportListChange(rPrevRule, rNextRule);
    }


//  Reference<XTextSection> xNextSection;
//
//  // first: get current XTextSection
//  Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
//  if (xPropSet.is())
//  {
//      if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextSection))
//      {
//          Any aAny = xPropSet->getPropertyValue(sTextSection);
//          aAny >>= xNextSection;
//      }
//      // else: no current section
//  }
//  // else: no current section
//
//  // careful: exportListChange may only be called for (!bAutoStyles)
//  // I'd like a cleaner solution! Maybe export all section styles upfront.
//  if ( bAutoStyles )
//  {
//      if ( xNextSection.is() )
//      {
//          Reference<XPropertySet> xPropSet( xNextSection, UNO_QUERY);
//          Add( XML_STYLE_FAMILY_TEXT_SECTION, xPropSet );
//      }
//  }
//  else
//  {
//      // old != new? -> start/equal?
//      if (rPrevSection != xNextSection)
//      {
//          // a new section started, or an old one gets closed!
//
//          // close old list
//          XMLTextNumRuleInfo aEmptyNumRule;
//          exportListChange(rPrevRule, aEmptyNumRule);
//
//          // build stacks of old and new sections
//          vector<Reference<XTextSection> > aOldStack;
//          Reference<XTextSection> aCurrent = rPrevSection;
//          while(aCurrent.is())
//          {
//              aOldStack.push_back(aCurrent);
//              aCurrent = aCurrent->getParentSection();
//          }
//
//          vector<Reference<XTextSection> > aNewStack;
//          aCurrent = xNextSection;
//          while(aCurrent.is())
//          {
//              aNewStack.push_back(aCurrent);
//              aCurrent = aCurrent->getParentSection();
//          }
//
//          // compare the two stacks
//          vector<Reference<XTextSection> > ::reverse_iterator aOld =
//              aOldStack.rbegin();
//          vector<Reference<XTextSection> > ::reverse_iterator aNew =
//              aNewStack.rbegin();
//          while ( (aOld != aOldStack.rend()) &&
//                  (aNew != aNewStack.rend()) &&
//                  (*aOld) == (*aNew) )
//          {
//              aOld++;
//              aNew++;
//          }
//
//          // close all elements of aOld, open all of aNew
//          while (aOld != aOldStack.rend())
//          {
//              Reference<XNamed> xName(*aOld, UNO_QUERY);
//              GetExport().GetDocHandler()->endElement(sText_Section);
//              GetExport().GetDocHandler()->ignorableWhitespace(
//                  GetExport().sWS );
//              aOld++;
//          }
//
//          while (aNew != aNewStack.rend())
//          {
//              exportSectionStart(*aNew);
//              aNew++;
//          }
//
//          // start new list
//          exportListChange(aEmptyNumRule, rNextRule);
//      }
//      else
//      {
//          // list change, if sections have not changed
//          exportListChange(rPrevRule, rNextRule);
//      }
//  }
//
//  // save old section (old numRule gets saved in calling method
//  rPrevSection = xNextSection;
}

void XMLTextParagraphExport::exportSectionStart(
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextSection > & rSection)
{
    // any old attributes?
    GetExport().CheckAttrList();

    Reference<XNamed> xName(rSection, UNO_QUERY);
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_name,
                             xName->getName());

    // get XPropertySet for other values
    Reference<XPropertySet> xPropSet(rSection, UNO_QUERY);
    Any aAny;

    // style name
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_style_name,
                             Find( XML_STYLE_FAMILY_TEXT_SECTION,
                                   xPropSet, sEmpty ));

    // protected
    aAny = xPropSet->getPropertyValue(sIsProtected);
    if (*(sal_Bool*)aAny.getValue())
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      sXML_protected, sXML_true);
    }

    // condition and display
    aAny = xPropSet->getPropertyValue(sCondition);
    OUString sCond;
    aAny >>= sCond;
    sal_Char* pDisplay;
    if (sCond.getLength() > 0)
    {
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_condition,
                                 sCond);
        pDisplay = sXML_condition;
    }
    else
    {
        pDisplay = sXML_none;
    }
    aAny = xPropSet->getPropertyValue(sIsVisible);
    if (! *(sal_Bool*)aAny.getValue())
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT, sXML_display,
                                      pDisplay);
    }

    // export element
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
    GetExport().GetDocHandler()->startElement( sText_Section,
                                               GetExport().GetXAttrList() );
    GetExport().ClearAttrList();

    // data source
    // unfortunately, we have to test all relevant strings for non-zero length
    aAny = xPropSet->getPropertyValue(sFileLink);
    SectionFileLink aFileLink;
    aAny >>= aFileLink;

    aAny = xPropSet->getPropertyValue(sLinkRegion);
    OUString sRegionName;
    aAny >>= sRegionName;

    if ( (aFileLink.FileURL.getLength() > 0) ||
         (aFileLink.FilterName.getLength() > 0) ||
         (sRegionName.getLength() > 0) )
    {
        if (aFileLink.FileURL.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, sXML_href,
                                     aFileLink.FileURL);
        }

        if (aFileLink.FilterName.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_filter_name,
                                     aFileLink.FilterName);
        }

        if (sRegionName.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_section_name,
                                     sRegionName);
        }

        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, sXML_section_source,
                                 sal_True, sal_True);
    }
    else
    {
        // data source DDE
        // unfortunately, we have to test all relevant strings for
        // non-zero length
        aAny = xPropSet->getPropertyValue(sDdeCommandFile);
        OUString sApplication;
        aAny >>= sApplication;
        aAny = xPropSet->getPropertyValue(sDdeCommandType);
        OUString sTopic;
        aAny >>= sTopic;
        aAny = xPropSet->getPropertyValue(sDdeCommandElement);
        OUString sItem;
        aAny >>= sItem;

        if ( (sApplication.getLength() > 0) ||
             (sTopic.getLength() > 0) ||
             (sItem.getLength() > 0 )   )
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_application,
                                     sApplication);
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_topic,
                                     sTopic);
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_item,
                                     sItem);

            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_TEXT,
                                     sXML_section_source_dde,
                                     sal_True, sal_True);
        }
        // else: no data source
    }
}

XMLTextParagraphExport::XMLTextParagraphExport(
        SvXMLExport& rExp,
        SvXMLAutoStylePoolP & rASP ) :
    XMLStyleExport( rExp, OUString(), &rASP ),
//  rExport( rExp ),
    rAutoStylePool( rASP ),
    pListElements( 0 ),
    pExportedLists( 0 ),
    pListAutoPool( new XMLTextListAutoStylePool( this->GetExport() ) ),
    pFieldExport( 0 ),
    pPageTextFrameIdxs( 0 ),
    pPageGraphicIdxs( 0 ),
    pFrameTextFrameIdxs( 0 ),
    pFrameGraphicIdxs( 0 ),
    sParagraphService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Paragraph")),
    sTableService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextTable")),
    sTextFieldService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField")),
    sTextFrameService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")),
    sTextGraphicService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextGraphicObject")),
    sTextEmbeddedService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextEmbeddedObject")),
    sTextEndnoteService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Endnote")),
    sTextContentService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextContent")),
    sParaStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")),
    sParaConditionalStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaConditionalStyleName")),
    sParaChapterNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("ParaChapterNumberingLevel")),
    sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
    sFrameStyleName(RTL_CONSTASCII_USTRINGPARAM("FrameStyleName")),
    sTextField(RTL_CONSTASCII_USTRINGPARAM("TextField")),
    sText(RTL_CONSTASCII_USTRINGPARAM("Text")),
    sFrame(RTL_CONSTASCII_USTRINGPARAM("Frame")),
    sCategory(RTL_CONSTASCII_USTRINGPARAM("Category")),
    sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")),
    sTextPortionType(RTL_CONSTASCII_USTRINGPARAM("TextPortionType")),
    sFootnote(RTL_CONSTASCII_USTRINGPARAM("Footnote")),
    sBookmark(RTL_CONSTASCII_USTRINGPARAM("Bookmark")),
    sReferenceMark(RTL_CONSTASCII_USTRINGPARAM("ReferenceMark")),
    sIsCollapsed(RTL_CONSTASCII_USTRINGPARAM("IsCollapsed")),
    sIsStart(RTL_CONSTASCII_USTRINGPARAM("IsStart")),
    sReferenceId(RTL_CONSTASCII_USTRINGPARAM("ReferenceId")),
    sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
    sPageStyleName(RTL_CONSTASCII_USTRINGPARAM("PageStyleName")),
    sPageDescName(RTL_CONSTASCII_USTRINGPARAM("PageDescName")),
    sPrefix(RTL_CONSTASCII_USTRINGPARAM("Prefix")),
    sStartAt(RTL_CONSTASCII_USTRINGPARAM("StartAt")),
    sSuffix(RTL_CONSTASCII_USTRINGPARAM("Suffix")),
    sPositionEndOfDoc(RTL_CONSTASCII_USTRINGPARAM("PositionEndOfDoc")),
    sFootnoteCounting(RTL_CONSTASCII_USTRINGPARAM("FootnoteCounting")),
    sEndNotice(RTL_CONSTASCII_USTRINGPARAM("EndNotice")),
    sBeginNotice(RTL_CONSTASCII_USTRINGPARAM("BeginNotice")),
    sFrameWidthAbsolute(RTL_CONSTASCII_USTRINGPARAM("FrameWidthAbsolute")),
    sFrameWidthPercent(RTL_CONSTASCII_USTRINGPARAM("FrameWidthPercent")),
    sFrameHeightAbsolute(RTL_CONSTASCII_USTRINGPARAM("FrameHeightAbsolute")),
    sFrameHeightPercent(RTL_CONSTASCII_USTRINGPARAM("FrameHeightPercent")),
    sWidth(RTL_CONSTASCII_USTRINGPARAM("Width")),
    sRelativeWidth(RTL_CONSTASCII_USTRINGPARAM("RelativeWidth")),
    sHeight(RTL_CONSTASCII_USTRINGPARAM("Height")),
    sRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("RelativeHeight")),
    sSizeType(RTL_CONSTASCII_USTRINGPARAM("SizeType")),
    sSizeRelative(RTL_CONSTASCII_USTRINGPARAM("SizeRelative")),
    sHoriOrient(RTL_CONSTASCII_USTRINGPARAM("HoriOrient")),
    sHoriOrientPosition(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")),
    sVertOrient(RTL_CONSTASCII_USTRINGPARAM("VertOrient")),
    sVertOrientPosition(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType")),
    sAnchorPageNo(RTL_CONSTASCII_USTRINGPARAM("AnchorPageNo")),
    sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")),
    sGraphicFilter(RTL_CONSTASCII_USTRINGPARAM("GraphicFilter")),
    sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
    sHyperLinkURL(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")),
    sHyperLinkName(RTL_CONSTASCII_USTRINGPARAM("HyperLinkName")),
    sHyperLinkTarget(RTL_CONSTASCII_USTRINGPARAM("HyperLinkTarget")),
    sUnvisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("UnvisitedCharStyleName")),
    sVisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("VisitedCharStyleName")),
    sTextSection(RTL_CONSTASCII_USTRINGPARAM("TextSection")),
    sSection(RTL_CONSTASCII_USTRINGPARAM(sXML_section)),
    sIsProtected(RTL_CONSTASCII_USTRINGPARAM("IsProtected")),
    sIsVisible(RTL_CONSTASCII_USTRINGPARAM("IsVisible")),
    sCondition(RTL_CONSTASCII_USTRINGPARAM("Condition")),
    sFileLink(RTL_CONSTASCII_USTRINGPARAM("FileLink")),
    sLinkRegion(RTL_CONSTASCII_USTRINGPARAM("LinkRegion")),
    sDdeCommandFile(RTL_CONSTASCII_USTRINGPARAM("DDECommandFile")),
    sDdeCommandType(RTL_CONSTASCII_USTRINGPARAM("DDECommandType")),
    sDdeCommandElement(RTL_CONSTASCII_USTRINGPARAM("DDECommandElement")),
    sEmpty()
{
    sText_Section = GetExport().GetNamespaceMap().GetQNameByKey(
        XML_NAMESPACE_TEXT, sSection);

    UniReference < XMLPropertySetMapper > xPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA );
    xParaPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );

    OUString sFamily( RTL_CONSTASCII_USTRINGPARAM(sXML_paragraph) );
    OUString sPrefix( 'P' );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              xPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    xTextPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_text) );
    sPrefix = OUString( 'T' );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_TEXT, sFamily,
                              xPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_AUTO_FRAME );
    xAutoFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                                  GetExport() );
    sFamily = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME) );
    sPrefix = OUString( 'G' );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_SD_GRAPHICS_ID, sFamily,
                              xPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_SECTION );
    xSectionPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_section ) );
    sPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM( "Sect" ) );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_SECTION, sFamily,
                              xPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME );
    xFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                              GetExport() );

    pFieldExport = new XMLTextFieldExport( rExp );
}

XMLTextParagraphExport::~XMLTextParagraphExport()
{
    delete pFieldExport;
    delete pListElements;
    delete pExportedLists;
    delete pListAutoPool;
    delete pPageTextFrameIdxs;
    delete pPageGraphicIdxs;
    delete pFrameTextFrameIdxs;
    delete pFrameGraphicIdxs;
}

void XMLTextParagraphExport::collectFrames()
{
    Reference < XTextFramesSupplier > xTFS( GetExport().GetModel(), UNO_QUERY );
    if( xTFS.is() )
    {
        xTextFrames = Reference < XIndexAccess >( xTFS->getTextFrames(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xTextFrames->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xTextFrames->getByIndex( i );
            Reference < XTextFrame > xTxtFrame;
            aAny >>= xTxtFrame;
            Reference < XPropertySet > xPropSet( xTxtFrame, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            switch( eAnchor )
            {
            case TextContentAnchorType_AT_PAGE:
                if( !pPageTextFrameIdxs )
                    pPageTextFrameIdxs = new SvLongs;
                pPageTextFrameIdxs->Insert( i, pPageTextFrameIdxs->Count() );
                break;
            case TextContentAnchorType_AT_FRAME:
                if( !pFrameTextFrameIdxs )
                    pFrameTextFrameIdxs = new SvLongs;
                pFrameTextFrameIdxs->Insert( i, pFrameTextFrameIdxs->Count() );
                break;
            }
        }
    }

    Reference < XTextGraphicObjectsSupplier > xTGOS( GetExport().GetModel(),
                                                    UNO_QUERY );
    if( xTGOS.is() )
    {
        xGraphics = Reference < XIndexAccess >( xTGOS->getGraphicObjects(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xGraphics->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xGraphics->getByIndex( i );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            Reference < XPropertySet > xPropSet( xTxtCntnt, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            switch( eAnchor )
            {
            case TextContentAnchorType_AT_PAGE:
                if( !pPageGraphicIdxs )
                    pPageGraphicIdxs = new SvLongs;
                pPageGraphicIdxs->Insert( i, pPageGraphicIdxs->Count() );
                break;
            case TextContentAnchorType_AT_FRAME:
                if( !pFrameGraphicIdxs )
                    pFrameGraphicIdxs = new SvLongs;
                pFrameGraphicIdxs->Insert( i, pFrameGraphicIdxs->Count() );
                break;
            }
        }
    }
}

void XMLTextParagraphExport::exportPageFrames( sal_Bool bAutoStyles )
{
    if( pPageTextFrameIdxs )
    {
        for( sal_Int32 i = 0; i < pPageTextFrameIdxs->Count(); i++ )
        {
            Any aAny = xTextFrames->getByIndex( (*pPageTextFrameIdxs)[i] );
            Reference < XTextFrame > xTxtFrame;
            aAny >>= xTxtFrame;
            Reference < XTextContent > xTxtCntnt( xTxtFrame, UNO_QUERY );
            exportTextFrame( xTxtCntnt, bAutoStyles );
        }
    }
    if( pPageGraphicIdxs )
    {
        for( sal_Int32 i = 0; i < pPageGraphicIdxs->Count(); i++ )
        {
            Any aAny = xGraphics->getByIndex( (*pPageGraphicIdxs)[i] );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            exportTextGraphic( xTxtCntnt, bAutoStyles );
        }
    }
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        sal_Bool bAutoStyles )
{
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum = xEA->createEnumeration();

    exportTextContentEnumeration( xParaEnum, bAutoStyles );
}

void XMLTextParagraphExport::exportTextContentEnumeration(
        const Reference < XEnumeration > & rContEnum,
        sal_Bool bAutoStyles )
{
    XMLTextNumRuleInfo aPrevNumInfo;
    XMLTextNumRuleInfo aNextNumInfo;

    sal_Bool bHasContent sal_False;
    Reference<XTextSection> xCurrentTextSection;

    while( rContEnum->hasMoreElements() )
    {
        Any aAny = rContEnum->nextElement();
        Reference < XTextContent > xTxtCntnt;
        aAny >>= xTxtCntnt;

        Reference<XServiceInfo> xServiceInfo( xTxtCntnt, UNO_QUERY );
        if( xServiceInfo->supportsService( sParagraphService ) )
        {
            if( !bAutoStyles )
            {
                aNextNumInfo.Set( xTxtCntnt );
            }

            exportListAndSectionChange( xCurrentTextSection, xTxtCntnt,
                                        aPrevNumInfo, aNextNumInfo,
                                        bAutoStyles );

            exportParagraph( xTxtCntnt, bAutoStyles );
            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTableService ) )
        {
            if( !bAutoStyles )
            {
                aNextNumInfo.Reset();
                exportListChange( aPrevNumInfo, aNextNumInfo );
            }
            exportTable( xTxtCntnt, bAutoStyles );
            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTextFrameService ) )
        {
            exportTextFrame( xTxtCntnt, bAutoStyles );
        }
        else if( xServiceInfo->supportsService( sTextGraphicService ) )
        {
            exportTextGraphic( xTxtCntnt, bAutoStyles );
        }
        else if( xServiceInfo->supportsService( sTextEmbeddedService ) )
        {
            exportTextEmbedded( xTxtCntnt, bAutoStyles );
        }
        else
        {
            DBG_ASSERT( !xTxtCntnt.is(), "unknown text content" );
        }

        if( !bAutoStyles )
        {
            aPrevNumInfo = aNextNumInfo;
        }
    }

    if( bHasContent && !bAutoStyles )
    {
        aNextNumInfo.Reset();

        // close open lists and sections; no new styles
        Reference<XTextContent> xEmpty;
        exportListAndSectionChange( xCurrentTextSection, xEmpty,
                                    aPrevNumInfo, aNextNumInfo,
                                    bAutoStyles );
    }
}

void XMLTextParagraphExport::exportParagraph(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles )
{
    sal_Int8 nOutlineLevel = -1;

    Reference < XPropertySet > xPropSet( rTextContent, UNO_QUERY );

    if( bAutoStyles )
    {
        Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet );
    }
    else
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        OUString sStyle;
        Any aAny;
        if( xPropSetInfo->hasPropertyByName( sParaStyleName ) )
        {
            aAny = xPropSet->getPropertyValue( sParaStyleName );
            aAny >>= sStyle;
        }

        OUString sAutoStyle( sStyle );
        sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet, sStyle );
        if( sAutoStyle.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name,
                                      sAutoStyle );

        if( xPropSetInfo->hasPropertyByName( sParaConditionalStyleName ) )
        {
            OUString sCondStyle;
            aAny = xPropSet->getPropertyValue( sParaConditionalStyleName );
            aAny >>= sCondStyle;
            if( sCondStyle != sStyle )
            {
                sCondStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet,
                                      sCondStyle );
                if( sCondStyle.getLength() )
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              sXML_cond_style_name,
                                              sCondStyle );
            }
        }

        if( xPropSetInfo->hasPropertyByName( sParaChapterNumberingLevel ) )
        {
            aAny = xPropSet->getPropertyValue( sParaChapterNumberingLevel );
            aAny >>= nOutlineLevel;
            if( -1 != nOutlineLevel )
            {
                OUStringBuffer sTmp;
                sTmp.append( (sal_Int32)nOutlineLevel+1L );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_level,
                              sTmp.makeStringAndClear() );
            }
        }
    }

    Reference < XEnumerationAccess > xEA( rTextContent, UNO_QUERY );
    Reference < XEnumeration > xTextEnum = xEA->createEnumeration();

    Reference < XEnumeration> xContentEnum;
    Reference < XContentEnumerationAccess > xCEA( rTextContent, UNO_QUERY );
    if( xCEA.is() )
        xContentEnum = xCEA->createContentEnumeration( sTextContentService );

    if( bAutoStyles )
    {
        if( xContentEnum.is() )
            exportTextContentEnumeration( xContentEnum, bAutoStyles );
        exportTextRangeEnumeration( xTextEnum, bAutoStyles );
    }
    else
    {
        const sal_Char *pElem =
            -1 == nOutlineLevel ? sXML_p : sXML_h;
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, pElem,
                                  sal_True, sal_False );
        if( xContentEnum.is() )
            exportTextContentEnumeration( xContentEnum, bAutoStyles );
        exportTextRangeEnumeration( xTextEnum, bAutoStyles );
    }
}

// bookmarks, reference marks (and TOC marks) are the same except for the
// element names. We use the same method for export and it an array with
// the proper element names
static const sal_Char* lcl_XmlReferenceElements[] = {
    sXML_reference_mark, sXML_reference_mark_start, sXML_reference_mark_end };
static const sal_Char* lcl_XmlBookmarkElements[] = {
    sXML_bookmark, sXML_bookmark_start, sXML_bookmark_end };


void XMLTextParagraphExport::exportTextRangeEnumeration(
        const Reference < XEnumeration > & rTextEnum,
        sal_Bool bAutoStyles )
{
    sal_Bool bPrevCharIsSpace = sal_False;

    while( rTextEnum->hasMoreElements() )
    {
        Any aAny = rTextEnum->nextElement();
        Reference < XTextRange > xTxtRange;
        aAny >>= xTxtRange;

        Reference<XPropertySet> xPropSet(xTxtRange, UNO_QUERY);
        Reference<XPropertySetInfo> xPropInfo =
            xPropSet->getPropertySetInfo();

        if (xPropInfo->hasPropertyByName(sTextPortionType))
        {
            Any aAny;
            aAny = xPropSet->getPropertyValue(sTextPortionType);
            OUString sType;
            aAny >>= sType;

            if( sType.equals(sText))
            {
                exportTextRange( xTxtRange, bAutoStyles,
                                 bPrevCharIsSpace );
            }
            else if( sType.equals(sTextField))
            {
                exportTextField( xTxtRange, bAutoStyles );
                bPrevCharIsSpace = sal_False;
            }
            else if( sType.equals( sFrame ) )
            {
                Reference < XEnumeration> xContentEnum;
                Reference < XContentEnumerationAccess > xCEA( xTxtRange,
                                                              UNO_QUERY );
                if( xCEA.is() )
                    xContentEnum = xCEA->createContentEnumeration(
                                                    sTextContentService );
                if( xContentEnum.is() )
                    exportTextContentEnumeration( xContentEnum, bAutoStyles );
                bPrevCharIsSpace = sal_False;
            }
            else if (sType.equals(sFootnote))
            {
                exportTextFootnote(xPropSet,
                                   xTxtRange->getString(),
                                   bAutoStyles);
                bPrevCharIsSpace = sal_False;
            }
            else if (sType.equals(sBookmark))
            {
                exportTextMark(xPropSet,
                               sBookmark,
                               lcl_XmlBookmarkElements,
                               bAutoStyles);
                bPrevCharIsSpace = sal_False;
            }
            else if (sType.equals(sReferenceMark))
            {
                exportTextMark(xPropSet,
                               sReferenceMark,
                               lcl_XmlReferenceElements,
                               bAutoStyles);
                bPrevCharIsSpace = sal_False;
            }
            else
                DBG_ERROR("unknown text portion type");
        }
        else
        {
            Reference<XServiceInfo> xServiceInfo( xTxtRange, UNO_QUERY );
            if( xServiceInfo->supportsService( sTextFieldService ) )
            {
                exportTextField( xTxtRange, bAutoStyles );
                bPrevCharIsSpace = sal_False;
            }
            else
            {
                // no TextPortionType property -> non-Writer app -> text
                exportTextRange( xTxtRange, bAutoStyles, bPrevCharIsSpace );
            }
        }
    }
}

void XMLTextParagraphExport::exportTable(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles )
{
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextRange > & rTextRange,
        sal_Bool bAutoStyles )
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    // non-Writer apps need not support Property TextField, so test first
    if (xPropSet->getPropertySetInfo()->hasPropertyByName( sTextField ))
    {
        Any aAny = xPropSet->getPropertyValue( sTextField );

        Reference < XTextField > xTxtFld;
        aAny >>= xTxtFld;
        DBG_ASSERT( xTxtFld.is(), "text field missing" );
        if( xTxtFld.is() )
        {
            if( bAutoStyles )
            {
                Add( XML_STYLE_FAMILY_TEXT_TEXT, xPropSet );
                pFieldExport->ExportFieldAutoStyle( xTxtFld );
            }
            else
            {
                OUString sStyle = FindTextStyle( xPropSet );
                OUString sText = rTextRange->getString();
                if( sStyle.getLength() )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              sXML_style_name, sStyle );
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              sXML_span, sal_False, sal_False);
                    pFieldExport->ExportField( xTxtFld );
                }
                else
                {
                    pFieldExport->ExportField( xTxtFld );
                }
            }
        }
    }
}

void XMLTextParagraphExport::exportTextMark(
    const Reference<XPropertySet> & rPropSet,
    const OUString sProperty,
    const sal_Char* pElements[],
    sal_Bool bAutoStyles)
{
    // mib said: "Hau wech!"
    //
    // (Originally, I'd export a span element in case the (book|reference)mark
    //  was formatted. This actually makes a difference in case some pervert
    //  sets a point reference mark in the document and, say, formats it bold.
    //  This basically meaningless formatting will now been thrown away
    //  (aka cleaned up), since mib said: ...                   dvo

     if (!bAutoStyles)
    {
        // get reference (as text content)
        Any aAny;
        aAny = rPropSet->getPropertyValue(sProperty);
        Reference<XTextContent> xRefMark;
        aAny >>= xRefMark;

        // name element
        Reference<XNamed> xName(xRefMark, UNO_QUERY);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_name,
                                 xName->getName());

        // start, end, or point-reference?
        sal_Int8 nElement;
        aAny = rPropSet->getPropertyValue(sIsCollapsed);
        if( *(sal_Bool *)aAny.getValue() )
        {
            nElement = 0;
        }
        else
        {
            aAny = rPropSet->getPropertyValue(sIsStart);
            nElement = *(sal_Bool *)aAny.getValue() ? 1 : 2;
        }

        // export element
        DBG_ASSERT(pElements != NULL, "illegal element array");
        DBG_ASSERT(nElement >= 0, "illegal element number");
        DBG_ASSERT(nElement <= 2, "illegal element number");
        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, pElements[nElement],
                                 sal_False, sal_False);
    }
    // else: no styles. (see above)
}

void XMLTextParagraphExport::addTextFrameAttributes(
    const Reference < XPropertySet >& rPropSet )
{
    // draw:name
    Reference < XNamed > xNamed( rPropSet, UNO_QUERY );
    if( xNamed.is() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, sXML_name,
                                  xNamed->getName() );

    Any aAny;
    OUStringBuffer sValue;

    // text:anchor-type
    aAny = rPropSet->getPropertyValue( sAnchorType );
    TextContentAnchorType eAnchor = TextContentAnchorType_AT_PARAGRAPH;
    aAny >>= eAnchor;
    {
        XMLAnchorTypePropHdl aAnchorTypeHdl;
        OUString sTmp;
        aAnchorTypeHdl.exportXML( sTmp, aAny,
                                  GetExport().GetMM100UnitConverter() );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_anchor_type, sTmp );
    }

    // text:anchor-page-number
    if( TextContentAnchorType_AT_PAGE == eAnchor )
    {
        aAny = rPropSet->getPropertyValue( sAnchorPageNo );
        sal_Int16 nPage;
        aAny >>= nPage;
        GetExport().GetMM100UnitConverter().convertNumber( sValue,
                                                           (sal_Int32)nPage );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_anchor_page_number,
                                  sValue.makeStringAndClear() );
    }

    // svg:x
    sal_Int16 nHoriOrient =  HoriOrientation::NONE;
    aAny = rPropSet->getPropertyValue( sHoriOrient );
    aAny >>= nHoriOrient;
    if( HoriOrientation::NONE == nHoriOrient )
    {
        sal_Int32 nPos = 0;
        Any aAny = rPropSet->getPropertyValue( sHoriOrientPosition );
        aAny >>= nPos;
        GetExport().GetMM100UnitConverter().convertMeasure( sValue, nPos );
        GetExport().AddAttribute( XML_NAMESPACE_SVG, sXML_x,
                                  sValue.makeStringAndClear() );
    }

    // svg:y
    sal_Int16 nVertOrient =  VertOrientation::NONE;
    aAny = rPropSet->getPropertyValue( sVertOrient );
    aAny >>= nVertOrient;
    if( VertOrientation::NONE == nVertOrient )
    {
        sal_Int32 nPos = 0;
        Any aAny = rPropSet->getPropertyValue( sVertOrientPosition );
        aAny >>= nPos;
        GetExport().GetMM100UnitConverter().convertMeasure( sValue, nPos );
        GetExport().AddAttribute( XML_NAMESPACE_SVG, sXML_y,
                                  sValue.makeStringAndClear() );
    }


    // svg:width
    sal_Int8 nRelWidth =  0;
    aAny = rPropSet->getPropertyValue( sRelativeWidth );
    aAny >>= nRelWidth;
    DBG_ASSERT( nRelWidth >= 0 && nRelWidth <= 100,
                "Got illegal relative width from API" );
    if( nRelWidth > 0 )
    {
        // TODO: instead of checking this value for 255 a new property
        // must be introduced like for heights.
        if( nRelWidth != 255 )
            GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                                nRelWidth );
    }
    else
    {
        sal_Int32 nWidth =  0;
        aAny = rPropSet->getPropertyValue( sWidth );
        aAny >>= nWidth;
        GetExport().GetMM100UnitConverter().convertMeasure( sValue, nWidth );
    }
    GetExport().AddAttribute( XML_NAMESPACE_SVG, sXML_width,
                              sValue.makeStringAndClear() );

    // svg:height or fo:min-height
    aAny = rPropSet->getPropertyValue( sSizeRelative );
    sal_Bool bSyncHeight = *(sal_Bool *)aAny.getValue();

    Reference< XPropertySetInfo > xPropSetInfo = rPropSet->getPropertySetInfo();
    sal_Int16 nSizeType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sSizeType ) )
    {
        aAny = rPropSet->getPropertyValue( sSizeType );
        aAny >>= nSizeType;
    }

    DBG_ASSERT( !bSyncHeight || SizeType::VARIABLE != nSizeType,
                "sync height and variable size unexpected" );

    if( !bSyncHeight && SizeType::VARIABLE != nSizeType )
    {
        sal_Int8 nRelHeight =  0;
        aAny = rPropSet->getPropertyValue( sRelativeHeight );
        aAny >>= nRelHeight;
        if( nRelHeight > 0 )
        {
            GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                                nRelHeight );
        }
        else
        {
            sal_Int32 nHeight =  0;
            aAny = rPropSet->getPropertyValue( sHeight );
            aAny >>= nHeight;
            GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                                nHeight );
        }
        if( SizeType::MIN == nSizeType )
            GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_min_height,
                                      sValue.makeStringAndClear() );
        else
            GetExport().AddAttribute( XML_NAMESPACE_SVG, sXML_height,
                                      sValue.makeStringAndClear() );
    }
}

void XMLTextParagraphExport::_exportTextFrame(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    Reference < XTextFrame > xTxtFrame( rPropSet, UNO_QUERY );
    Reference < XText > xTxt = xTxtFrame->getText();

    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rPropSet, sStyle );
    if( sAutoStyle.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, sXML_style_name,
                                  sAutoStyle );
    addTextFrameAttributes( rPropSet );

    // draw:chain-next-name
    if( rPropSetInfo->hasPropertyByName( sChainNextName ) )
    {
        OUString sNext;
        aAny = rPropSet->getPropertyValue( sChainNextName );
        if( (aAny >>= sNext) && sNext.getLength() > 0 )
            GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                                      sXML_chain_next_name,
                                      sNext );
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              sXML_text_box, sal_False, sal_True );
    exportText( xTxt, sal_False );
}

void XMLTextParagraphExport::exportTextFrame(
        const Reference < XTextContent > & rTxtCntnt,
        sal_Bool bAutoStyles )
{
    Reference < XPropertySet > xPropSet( rTxtCntnt, UNO_QUERY );

    if( bAutoStyles )
    {
        Reference < XTextFrame > xTxtFrame( rTxtCntnt, UNO_QUERY );
        Reference < XText > xTxt = xTxtFrame->getText();
        Add( XML_STYLE_FAMILY_SD_GRAPHICS_ID, xPropSet );
        exportText( xTxt, bAutoStyles );
    }
    else
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        if( xPropState.is() &&
            xPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
            PropertyState_DIRECT_VALUE ==
                                xPropState->getPropertyState( sHyperLinkURL ) )
        {
            addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                                      sXML_a, sal_False, sal_False );
            _exportTextFrame( xPropSet, xPropSetInfo );
        }
        else
        {
            _exportTextFrame( xPropSet, xPropSetInfo );
        }
    }
}

void XMLTextParagraphExport::_exportTextGraphic(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rPropSet, sStyle );
    if( sAutoStyle.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, sXML_style_name,
                                  sAutoStyle );
    addTextFrameAttributes( rPropSet );

    // xlink:href
    OUString sURL;
    aAny = rPropSet->getPropertyValue( sGraphicURL );
    aAny >>= sURL;
    GetExport().AddAttribute(XML_NAMESPACE_XLINK, sXML_href, sURL );
    GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type,
                                   sXML_simple );
    GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show,
                                   sXML_embed );
    GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate,
                                   sXML_onRequest );

    // draw:filter-name
    OUString sGrfFilter;
    aAny = rPropSet->getPropertyValue( sGraphicFilter );
    aAny >>= sGrfFilter;
    if( sGrfFilter.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, sXML_filter_name,
                                  sGrfFilter );

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              sXML_image, sal_False, sal_True );

    // svg:desc
    OUString sAltText;
    aAny = rPropSet->getPropertyValue( sAlternativeText );
    aAny >>= sAltText;
    if( sAltText.getLength() )
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                  sXML_desc, sal_True, sal_False );
        GetExport().GetDocHandler()->characters( sAltText );
    }
}

void XMLTextParagraphExport::exportTextGraphic(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles )
{
    Reference < XPropertySet > xPropSet( rTextContent, UNO_QUERY );

    if( bAutoStyles )
    {
        Add( XML_STYLE_FAMILY_SD_GRAPHICS_ID, xPropSet );
    }
    else
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        if( xPropState.is() &&
            xPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
            PropertyState_DIRECT_VALUE ==
                                xPropState->getPropertyState( sHyperLinkURL ) )
        {
            addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                                      sXML_a, sal_False, sal_False );
            _exportTextGraphic( xPropSet, xPropSetInfo );
        }
        else
        {
            _exportTextGraphic( xPropSet, xPropSetInfo );
        }
    }
}

void XMLTextParagraphExport::exportTextEmbedded(
        const Reference < XTextContent > & rTxtCntnt,
        sal_Bool bAutoStyles )
{
}

void XMLTextParagraphExport::_exportTextRange(
        const Reference < XTextRange > & rTextRange,
        const Reference < XPropertySet > & rPropSet,
        sal_Bool& rPrevCharIsSpace )
{
    OUString sStyle = FindTextStyle( rPropSet );
    OUString sText = rTextRange->getString();
    if( sStyle.getLength() )
    {
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name,
                                  sStyle );
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                  sXML_span, sal_False, sal_False );
        exportText( sText, rPrevCharIsSpace );
    }
    else
    {
        exportText( sText, rPrevCharIsSpace );
    }
}

void XMLTextParagraphExport::addHyperlinkAttributes(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertyState > & rPropState,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK,
                                   sXML_type, sXML_simple );

    OUString sTmp;
    Any aAny = rPropSet->getPropertyValue( sHyperLinkURL );
    aAny >>= sTmp;
    GetExport().AddAttribute( XML_NAMESPACE_XLINK, sXML_href, sTmp );

    if( rPropSetInfo->hasPropertyByName( sHyperLinkName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sHyperLinkName );
        aAny >>= sTmp;
        GetExport().AddAttribute( XML_NAMESPACE_OFFICE, sXML_name,
                                  sTmp );
    }

    if( rPropSetInfo->hasPropertyByName( sHyperLinkTarget ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkTarget ) ) )
    {
        aAny = rPropSet->getPropertyValue( sHyperLinkTarget );
        aAny >>= sTmp;
        if( sTmp.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE,
                                      sXML_target_frame_name, sTmp );
            const sal_Char *pStr =
                sTmp.equalsAsciiL( "_blank", sizeof("_blank")-1 )
                    ? sXML_new : sXML_replace;
            GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK,
                                      sXML_show, pStr );
        }
    }

    if( rPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
            rPropState->getPropertyState( sUnvisitedCharStyleName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sUnvisitedCharStyleName );
        aAny >>= sTmp;
        if( sTmp.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      sXML_style_name, sTmp );
    }

    if( rPropSetInfo->hasPropertyByName( sVisitedCharStyleName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
            rPropState->getPropertyState( sVisitedCharStyleName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sVisitedCharStyleName );
        aAny >>= sTmp;
        if( sTmp.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      sXML_visited_style_name, sTmp );
    }
}

void XMLTextParagraphExport::exportTextRange(
        const Reference < XTextRange > & rTextRange,
        sal_Bool bAutoStyles,
        sal_Bool& rPrevCharIsSpace )
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    if( bAutoStyles )
    {
        Add( XML_STYLE_FAMILY_TEXT_TEXT, xPropSet );
    }
    else
    {
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        Reference < XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( xPropState.is() &&
            xPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
            PropertyState_DIRECT_VALUE ==
                                xPropState->getPropertyState( sHyperLinkURL ) )
        {
            addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      sXML_a, sal_False, sal_False );
            _exportTextRange( rTextRange, xPropSet, rPrevCharIsSpace );
        }
        else
        {
            _exportTextRange( rTextRange, xPropSet, rPrevCharIsSpace );
        }
    }
}

void XMLTextParagraphExport::exportText( const OUString& rText,
                                           sal_Bool& rPrevCharIsSpace )
{
    sal_Int32 nExpStartPos = 0L;
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nSpaceChars = 0;
    for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
    {
        sal_Unicode cChar = rText[nPos];
        sal_Bool bExpCharAsText = sal_True;
        sal_Bool bExpCharAsElement = sal_False;
        sal_Bool bCurrCharIsSpace = sal_False;
        switch( cChar )
        {
        case 0x0009:    // Tab
        case 0x000A:    // LF
            // These characters are exported as text.
            bExpCharAsElement = sal_True;
            bExpCharAsText = sal_False;
            break;
        case 0x0020:    // Blank
            if( rPrevCharIsSpace )
            {
                // If the previous character is a space character,
                // too, export a special space element.
                bExpCharAsText = sal_False;
            }
            bCurrCharIsSpace = sal_True;
            break;
        }

        // If the current character is not exported as text
           // the text that has not been exported by now has to be exported now.
        if( nPos > nExpStartPos && !bExpCharAsText )
        {
            DBG_ASSERT( 0==nSpaceChars, "pending spaces" );
            OUString sExp( rText.copy( nExpStartPos, nPos - nExpStartPos ) );
            GetExport().GetDocHandler()->characters( sExp );
            nExpStartPos = nPos;
        }

        // If there are spaces left that have not been exported and the
        // current chracter is not a space , the pending spaces have to be
        // exported now.
        if( nSpaceChars > 0 && !bCurrCharIsSpace )
        {
            DBG_ASSERT( nExpStartPos == nPos, " pending characters" );

            if( nSpaceChars > 1 )
            {
                OUStringBuffer sTmp;
                sTmp.append( (sal_Int32)nSpaceChars );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_c,
                              sTmp.makeStringAndClear() );
            }

            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      sXML_s, sal_False, sal_False );

            nSpaceChars = 0;
        }

        // If the current character has to be exported as a special
        // element, the elemnt will be exported now.
        if( bExpCharAsElement )
        {
            switch( cChar )
            {
            case 0x0009:    // Tab
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              sXML_tab_stop, sal_False,
                                              sal_False );
                }
                break;
            case 0x000A:    // LF
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              sXML_line_break, sal_False,
                                              sal_False );
                }
                break;
            }
        }

        // If the current character is a space, and the previous one
        // is a space, too, the number of pending spaces is incremented
        // only.
        if( bCurrCharIsSpace && rPrevCharIsSpace )
            nSpaceChars++;
        rPrevCharIsSpace = bCurrCharIsSpace;

        // If the currect character is not exported as text, the start
        // position for text is the position behind the current position.
        if( !bExpCharAsText )
        {
            DBG_ASSERT( nExpStartPos == nPos, "wrong export start pos" );
            nExpStartPos = nPos+1;
        }
    }

    if( nExpStartPos < nEndPos )
    {
        DBG_ASSERT( 0==nSpaceChars, " pending spaces " );
        OUString sExp( rText.copy( nExpStartPos, nEndPos - nExpStartPos ) );
        GetExport().GetDocHandler()->characters( sExp );
    }

    // If there are some spaces left, they have to be exported now.
    if( nSpaceChars > 0 )
    {
        if( nSpaceChars > 1 )
        {
            OUStringBuffer sTmp;
            sTmp.append( (sal_Int32)nSpaceChars );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_c,
                          sTmp.makeStringAndClear() );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, sXML_s,
                                  sal_False, sal_False );
    }
}

void XMLTextParagraphExport::exportTextDeclarations()
{
    pFieldExport->ExportFieldDeclarations();
}

void XMLTextParagraphExport::exportTextAutoStyles()
{
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                   *GetParaPropMapper().get(),
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_TEXT,
                                   *GetTextPropMapper().get(),
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

/*
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_SD_GRAPHICS_ID,
                                   *GetAutoFramePropMapper().get(),
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );
*/

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_SECTION,
                                  *GetSectionPropMapper().get(),
                                  GetExport().GetDocHandler(),
                                  GetExport().GetMM100UnitConverter(),
                                  GetExport().GetNamespaceMap() );
    pListAutoPool->exportXML();
}


