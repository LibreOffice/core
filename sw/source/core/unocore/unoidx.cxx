/*************************************************************************
 *
 *  $RCSfile: unoidx.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: os $ $Date: 2001-02-15 13:46:37 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#include <cmdid.h>
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _DOCTXM_HXX //autogen
#include <doctxm.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDPART_HPP_
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATAFIELD_HPP_
#include <com/sun/star/text/BibliographyDataField.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _DOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::rtl;

//-----------------------------------------------------------------------------
String lcl_AnyToString(uno::Any rVal) throw(IllegalArgumentException)
{
    if(rVal.getValueType() != ::getCppuType((OUString*)0))
        throw IllegalArgumentException();
    return String(*(OUString*)rVal.getValue());
}
//-----------------------------------------------------------------------------
sal_Int16 lcl_AnyToInt16(uno::Any rVal) throw(IllegalArgumentException)
{
    if(rVal.getValueType() != ::getCppuType((sal_Int16*)0))
        throw IllegalArgumentException();
    sal_Int16 nRet;
    rVal >>= nRet;
    return nRet;
}
//-----------------------------------------------------------------------------
sal_Bool lcl_AnyToBool(uno::Any rVal) throw(IllegalArgumentException)
{
    if(rVal.getValueType() != ::getCppuBooleanType())
        throw IllegalArgumentException();
    return *(sal_Bool*) rVal.getValue();
}
/******************************************************************************
 *
 ******************************************************************************/
SwTOXMark* lcl_GetMark(SwTOXType* pType, const SwTOXMark* pOwnMark)
{
    SwClientIter aIter(*pType);
    SwTOXMark* pMark = (SwTOXMark*)aIter.First(TYPE(SwTOXMark));
    while( pMark )
    {
        if(pMark == pOwnMark)
            return pMark;
        else
            pMark = (SwTOXMark*)aIter.Next();
    }
    return 0;
}

/******************************************************************
 * SwXDocumentIndex
 ******************************************************************/
/* -----------------20.06.98 11:06-------------------
 *
 * --------------------------------------------------*/
class SwDocIdxProperties_Impl
{
    SwTOXBase*      pTOXBase;
public:
    SwTOXBase&      GetTOXBase() {return *pTOXBase;}
    SwDocIdxProperties_Impl(const SwTOXType* pType);
    ~SwDocIdxProperties_Impl(){delete pTOXBase;}
};
/* -----------------20.06.98 11:41-------------------
 *
 * --------------------------------------------------*/
SwDocIdxProperties_Impl::SwDocIdxProperties_Impl(const SwTOXType* pType)
{
    SwForm aForm(pType->GetType());
    pTOXBase = new SwTOXBase(pType, aForm,
                                TOX_MARK, pType->GetTypeName());
}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXDocumentIndex::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXDocumentIndex::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDocumentIndex::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndex");
}
/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDocumentIndex::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.BaseIndex") == rServiceName ||
                ( TOX_INDEX == eTOXType && C2U("com.sun.star.text.DocumentIndex") == rServiceName) ||
                ( TOX_CONTENT == eTOXType && C2U("com.sun.star.text.ContentIndex") == rServiceName) ||
                ( TOX_USER == eTOXType && C2U("com.sun.star.text.UserDefinedIndex") == rServiceName) ||
                ( TOX_ILLUSTRATIONS == eTOXType && C2U("com.sun.star.text.IllustrationIndex") == rServiceName) ||
                ( TOX_TABLES == eTOXType && C2U("com.sun.star.text.TableIndex") == rServiceName) ||
                ( TOX_OBJECTS == eTOXType && C2U("com.sun.star.text.ObjectIndex") == rServiceName) ||
                ( TOX_AUTHORITIES == eTOXType && C2U("com.sun.star.text.Bibliography") == rServiceName);
}
/* -----------------------------06.04.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXDocumentIndex::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseIndex");
    switch( eTOXType )
    {
        case TOX_INDEX:         pArray[1] = C2U("com.sun.star.text.DocumentIndex");break;
        case TOX_CONTENT:       pArray[1] = C2U("com.sun.star.text.ContentIndex");break;
        case TOX_TABLES:        pArray[1] = C2U("com.sun.star.text.TableIndex");break;
        case TOX_ILLUSTRATIONS: pArray[1] = C2U("com.sun.star.text.IllustrationIndex");break;
        case TOX_OBJECTS:       pArray[1] = C2U("com.sun.star.text.ObjectIndex");break;
        case TOX_AUTHORITIES :  pArray[1] = C2U("com.sun.star.text.Bibliography");break;
        //case TOX_USER:
        default:
            pArray[1] = C2U("com.sun.star.text.UserDefinedIndex");
    }
    return aRet;
}
/*-- 14.12.98 09:35:03---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXDocumentIndex, SwClient)
SwXDocumentIndex::SwXDocumentIndex(const SwTOXBaseSection* pB, SwDoc* pDc) :
    m_pDoc(pDc),
    aLstnrCntnr( (text::XTextContent*)this),
    pBase(pB),
    pProps(0),
    _pMap(0),
    pStyleAccess(0),
    pTokenAccess(0),
    bIsDescriptor(sal_False),
    eTOXType(TOX_USER)
{
    if(pBase && m_pDoc)
    {
        pBase->GetFmt()->Add(this);
        sal_uInt16 PropertyId;
        eTOXType = pBase->SwTOXBase::GetType();
        switch( eTOXType )
        {
            case TOX_INDEX:     PropertyId = PROPERTY_MAP_INDEX_IDX; break;
            case TOX_CONTENT:   PropertyId = PROPERTY_MAP_INDEX_CNTNT; break;
            case TOX_TABLES:        PropertyId = PROPERTY_MAP_INDEX_TABLES; break;
            case TOX_ILLUSTRATIONS: PropertyId = PROPERTY_MAP_INDEX_ILLUSTRATIONS; break;
            case TOX_OBJECTS:       PropertyId = PROPERTY_MAP_INDEX_OBJECTS; break;
            case TOX_AUTHORITIES :  PropertyId = PROPERTY_MAP_BIBLIOGRAPHY; break;
            //case TOX_USER:
            default:
                PropertyId = PROPERTY_MAP_INDEX_USER;
        }
        _pMap = aSwMapProvider.GetPropertyMap(PropertyId);
    }
}
/* -----------------15.01.99 14:59-------------------
 *
 * --------------------------------------------------*/
SwXDocumentIndex::SwXDocumentIndex(TOXTypes eType, SwDoc& rDoc) :
    m_pDoc(0),
    aLstnrCntnr( (text::XTextContent*)this),
    pBase(0),
    pProps(new SwDocIdxProperties_Impl(rDoc.GetTOXType(eType, 0))),
    pStyleAccess(0),
    pTokenAccess(0),
    bIsDescriptor(sal_True),
    eTOXType(eType)
{
    sal_uInt16 PropertyId;
    switch(eType)
    {
        case TOX_INDEX:     PropertyId = PROPERTY_MAP_INDEX_IDX; break;
        case TOX_CONTENT:   PropertyId = PROPERTY_MAP_INDEX_CNTNT; break;
        case TOX_TABLES:        PropertyId = PROPERTY_MAP_INDEX_TABLES; break;
        case TOX_ILLUSTRATIONS: PropertyId = PROPERTY_MAP_INDEX_ILLUSTRATIONS; break;
        case TOX_OBJECTS:       PropertyId = PROPERTY_MAP_INDEX_OBJECTS; break;
        case TOX_AUTHORITIES :  PropertyId = PROPERTY_MAP_BIBLIOGRAPHY; break;
        //case TOX_USER:
        default:
            PropertyId = PROPERTY_MAP_INDEX_USER;
    }
    _pMap = aSwMapProvider.GetPropertyMap(PropertyId);
}

/*-- 14.12.98 09:35:04---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndex::~SwXDocumentIndex()
{
    delete pProps;
}
/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXDocumentIndex::getServiceName(void) throw( RuntimeException )
{
    USHORT nObjectType = SW_SERVICE_TYPE_INDEX;
    switch(eTOXType)
    {
//      case TOX_INDEX:             break;
        case TOX_USER:              nObjectType = SW_SERVICE_USER_INDEX;break;
        case TOX_CONTENT:           nObjectType = SW_SERVICE_CONTENT_INDEX;break;
        case TOX_ILLUSTRATIONS:     nObjectType = SW_SERVICE_INDEX_ILLUSTRATIONS;break;
        case TOX_OBJECTS:           nObjectType = SW_SERVICE_INDEX_OBJECTS;break;
        case TOX_TABLES:            nObjectType = SW_SERVICE_INDEX_TABLES;break;
        case TOX_AUTHORITIES:       nObjectType = SW_SERVICE_INDEX_BIBLIOGRAPHY;break;
    }
    return SwXServiceProvider::GetProviderName(nObjectType);
}
/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::update(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXBase* pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    if(!pTOXBase)
        throw RuntimeException();
    ((SwTOXBaseSection*)pTOXBase)->Update();
    // Seitennummern eintragen
    ((SwTOXBaseSection*)pTOXBase)->UpdatePageNum();
}
/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXDocumentIndex::getPropertySetInfo(void) throw( RuntimeException )
{
    Reference< XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}
/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::setPropertyValue(const OUString& rPropertyName,
                                        const uno::Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException,
                 IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    SwTOXBase* pTOXBase;
    if(GetFmt())
        pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    else if(bIsDescriptor)
        pTOXBase = &pProps->GetTOXBase();
    if(pTOXBase)
    {
        sal_uInt16 nCreate = pTOXBase->GetCreateType();
        sal_uInt16 nTOIOptions = 0;
        sal_uInt16 nOLEOptions = pTOXBase->GetOLEOptions();
        TOXTypes eTxBaseType = pTOXBase->GetTOXType()->GetType();
        if( eTxBaseType == TOX_INDEX )
            nTOIOptions = pTOXBase->GetOptions();
        SwForm  aForm(pTOXBase->GetTOXForm());
        sal_Bool bForm = sal_False;
        SfxItemSet* pAttrSet = 0;
        switch(pMap->nWID)
        {
            case WID_IDX_TITLE  :
            {
                OUString sNewName;
                aValue >>= sNewName;
                ShellResource* pShellRes = ViewShell::GetShellRes();
                switch(eTOXType)
                {
                    case TOX_CONTENT :
                        if(!sNewName.compareToAscii("Table of Contents"))
                            sNewName = pShellRes->aTOXContentName;
                    break;
                    case TOX_INDEX  :
                        if(!sNewName.compareToAscii("Alphabetical Index"))
                            sNewName = pShellRes->aTOXIndexName;
                    break;
                    case TOX_USER:
                        if(!sNewName.compareToAscii("User-Defined"))
                            sNewName = pShellRes->aTOXUserName;
                    break;
                    case TOX_ILLUSTRATIONS:
                        if(!sNewName.compareToAscii("Illustration Index"))
                            sNewName = pShellRes->aTOXIllustrationsName;
                    break;
                    case TOX_OBJECTS:
                        if(!sNewName.compareToAscii("Table of Objects"))
                            sNewName = pShellRes->aTOXObjectsName;
                    break;
                    case TOX_TABLES:
                        if(!sNewName.compareToAscii("Index of Tables"))
                            sNewName = pShellRes->aTOXTablesName;
                    break;
                    case TOX_AUTHORITIES:
                        if(!sNewName.compareToAscii("Bibliography"))
                            sNewName = pShellRes->aTOXAuthoritiesName;
                    break;
                }
                pTOXBase->SetTitle(sNewName);
            }
            break;
            case WID_LEVEL      :
                pTOXBase->SetLevel(lcl_AnyToInt16(aValue));
            break;
            case WID_CREATE_FROM_MARKS                 :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_MARK: nCreate & ~TOX_MARK;
            break;
            case WID_CREATE_FROM_OUTLINE               :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_OUTLINELEVEL: nCreate & ~TOX_OUTLINELEVEL;
            break;
//          case WID_PARAGRAPH_STYLE_NAMES             :DBG_ERROR("not implemented")
//          break;
            case WID_CREATE_FROM_CHAPTER               :
                pTOXBase->SetFromChapter(lcl_AnyToBool(aValue));
            break;
            case WID_CREATE_FROM_LABELS                :
                pTOXBase->SetFromObjectNames(! lcl_AnyToBool(aValue));
            break;
            case WID_PROTECTED                         :
                pTOXBase->SetProtected(lcl_AnyToBool(aValue));
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_ALPHA_DELIMITTER : nTOIOptions & ~TOI_ALPHA_DELIMITTER;
            break;
            case WID_USE_KEY_AS_ENTRY                  :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_KEY_AS_ENTRY : nTOIOptions & ~TOI_KEY_AS_ENTRY;
            break;
            case WID_USE_COMBINED_ENTRIES              :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_SAME_ENTRY : nTOIOptions & ~TOI_SAME_ENTRY;
            break;
            case WID_IS_CASE_SENSITIVE                 :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_CASE_SENSITIVE : nTOIOptions & ~TOI_CASE_SENSITIVE;
            break;
            case WID_USE_P_P                           :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_FF : nTOIOptions & ~TOI_FF;
            break;
            case WID_USE_DASH                          :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_DASH : nTOIOptions & ~TOI_DASH;
            break;
            case WID_USE_UPPER_CASE                    :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_INITIAL_CAPS : nTOIOptions & ~TOI_INITIAL_CAPS;
            break;
            case WID_IS_COMMA_SEPARATED :
                bForm = sal_True;
                aForm.SetCommaSeparated(lcl_AnyToBool(aValue));
            break;
            case WID_LABEL_CATEGORY                    :
                pTOXBase->SetSequenceName(lcl_AnyToString(aValue));
            break;
            case WID_LABEL_DISPLAY_TYPE                :
            {
                sal_Int16 nVal = lcl_AnyToInt16(aValue);
                    sal_uInt16 nSet = CAPTION_COMPLETE;
                    switch (nVal)
                    {
                        case text::ReferenceFieldPart::TEXT: nSet = CAPTION_COMPLETE;
                        break;
                        case text::ReferenceFieldPart::CATEGORY_AND_NUMBER  : nSet = CAPTION_NUMBER;
                        break;
                        case text::ReferenceFieldPart::ONLY_CAPTION : nSet = CAPTION_TEXT;
                        break;
                        default:
                            throw IllegalArgumentException();
                    }
                    pTOXBase->SetCaptionDisplay((SwCaptionDisplay)nSet);
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE             :
                pTOXBase->SetLevelFromChapter(lcl_AnyToBool(aValue));
            break;
//          case WID_RECALC_TAB_STOPS                  :DBG_ERROR("not implemented")
//              lcl_AnyToBool(aValue) ?
//          break;
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME   :
                pTOXBase->SetMainEntryCharStyle(
                    SwXStyleFamilies::GetUIName(lcl_AnyToString(aValue), SFX_STYLE_FAMILY_CHAR));
            break;
            case WID_CREATE_FROM_TABLES                :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_TABLE : nCreate & ~TOX_TABLE;
            break;
            case WID_CREATE_FROM_TEXT_FRAMES           :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_FRAME : nCreate & ~TOX_FRAME;
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS       :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_GRAPHIC : nCreate & ~TOX_GRAPHIC;
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS      :
                if(lcl_AnyToBool(aValue))
                    nCreate |= TOX_OLE;
                else
                    nCreate &= ~TOX_OLE;
            break;
            case WID_CREATE_FROM_STAR_MATH:
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_MATH : nOLEOptions & ~TOO_MATH;
            break;
            case WID_CREATE_FROM_STAR_CHART            :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_CHART : nOLEOptions & ~TOO_CHART;
            break;
            case WID_CREATE_FROM_STAR_CALC             :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_CALC : nOLEOptions & ~TOO_CALC;
            break;
            case WID_CREATE_FROM_STAR_DRAW             :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_DRAW_IMPRESS : nOLEOptions & ~TOO_DRAW_IMPRESS;
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_OTHER : nOLEOptions & ~TOO_OTHER;
            break;
            case WID_PARA_HEAD             :
                bForm = sal_True;
                //Header steht an Pos 0
                aForm.SetTemplate( 0, SwXStyleFamilies::GetUIName(
                    lcl_AnyToString(aValue), SFX_STYLE_FAMILY_PARA));
            break;
            case WID_IS_RELATIVE_TABSTOPS:
                bForm = sal_True;
                aForm.SetRelTabPos(lcl_AnyToBool(aValue));
            break;
            case WID_PARA_SEP              :
                bForm = sal_True;
                aForm.SetTemplate( 1, SwXStyleFamilies::GetUIName(
                    lcl_AnyToString(aValue), SFX_STYLE_FAMILY_PARA));
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                nCreate = lcl_AnyToBool(aValue) ?
                    (nCreate | TOX_TEMPLATE) : (nCreate & ~TOX_TEMPLATE);
            break;

            case WID_PARA_LEV1             :
            case WID_PARA_LEV2             :
            case WID_PARA_LEV3             :
            case WID_PARA_LEV4             :
            case WID_PARA_LEV5             :
            case WID_PARA_LEV6             :
            case WID_PARA_LEV7             :
            case WID_PARA_LEV8             :
            case WID_PARA_LEV9             :
            case WID_PARA_LEV10            :
            {
                bForm = sal_True;
                // im sdbcx::Index beginnt Lebel 1 bei Pos 2 sonst bei Pos 1
                sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                aForm.SetTemplate(nLPos + pMap->nWID - WID_PARA_LEV1,
                    SwXStyleFamilies::GetUIName(
                        lcl_AnyToString(aValue), SFX_STYLE_FAMILY_PARA));
            }
            break;
            default:
                //this is for items only
                if(WID_PRIMARY_KEY > pMap->nWID)
                {
                    SfxItemPropertySet aPropSet(_pMap);
                    const SwAttrSet& rSet = m_pDoc->GetTOXBaseAttrSet(*pTOXBase);
                    pAttrSet = new SfxItemSet(rSet);
                    aPropSet.setPropertyValue(*pMap, aValue, *pAttrSet);

                    const SwSectionFmts& rSects = m_pDoc->GetSections();
                    const SwSectionFmt* pOwnFmt = GetFmt();
                    for(sal_uInt16 i = 0; i < rSects.Count(); i++)
                    {
                        const SwSectionFmt* pTmpFmt = rSects[ i ];
                        if(pTmpFmt == pOwnFmt)
                        {
                            m_pDoc->ChgSection( i, *(SwTOXBaseSection*)pTOXBase, pAttrSet );
                            break;
                        }
                    }
                }
        }
        pTOXBase->SetCreate(nCreate);
        pTOXBase->SetOLEOptions(nOLEOptions);
        if(pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            pTOXBase->SetOptions(nTOIOptions);
        if(bForm)
            pTOXBase->SetTOXForm(aForm);
        delete pAttrSet;
    }
    else
        throw RuntimeException();

}
/*-- 14.12.98 09:35:05---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndex::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    SwTOXBase* pTOXBase;
    if(GetFmt())
        pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    else if(bIsDescriptor)
        pTOXBase = &pProps->GetTOXBase();
    if(pTOXBase)
    {
        sal_uInt16 nCreate = pTOXBase->GetCreateType();
        sal_uInt16 nTOIOptions = 0;
        sal_uInt16 nOLEOptions = pTOXBase->GetOLEOptions();
        if(pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            nTOIOptions = pTOXBase->GetOptions();
        const SwForm& rForm = pTOXBase->GetTOXForm();
        sal_Bool bBOOL = sal_True;
        sal_Bool bRet = sal_False;
        switch(pMap->nWID)
        {
            case WID_IDX_CONTENT_SECTION:
            case WID_IDX_HEADER_SECTION :
                bBOOL = sal_False;
                if(WID_IDX_CONTENT_SECTION == pMap->nWID)
                {
                    Reference <XTextSection> xContentSect = SwXTextSections::GetObject( *GetFmt() );
                    aRet <<= xContentSect;
                }
                else
                {
                    SwSections aSectArr;
                    GetFmt()->GetChildSections( aSectArr, SORTSECT_NOT, FALSE);
                    for(USHORT i = 0; i < aSectArr.Count(); i++)
                    {
                        SwSection* pSect = aSectArr[i];
                        if(pSect->GetType() == TOX_HEADER_SECTION)
                        {
                            Reference <XTextSection> xHeaderSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                            aRet <<= xHeaderSect;
                            break;
                        }
                    }
                }
            break;
            case WID_IDX_TITLE  :
            {
                bBOOL = sal_False;
                OUString uRet(pTOXBase->GetTitle());
                //I18N
                ShellResource* pShellRes = ViewShell::GetShellRes();
                switch(eTOXType)
                {
                    case TOX_CONTENT :
                        if(!uRet.compareTo(pShellRes->aTOXContentName))
                            uRet = C2U("Table of Contents");
                    break;
                    case TOX_INDEX  :
                        if(!uRet.compareTo(pShellRes->aTOXIndexName))
                            uRet = C2U("Alphabetical Index");
                    break;
                    case TOX_USER:
                        if(!uRet.compareTo(pShellRes->aTOXUserName))
                            uRet = C2U("User-Defined");
                    break;
                    case TOX_ILLUSTRATIONS:
                        if(!uRet.compareTo(pShellRes->aTOXIllustrationsName))
                            uRet = C2U("Illustration Index");
                    break;
                    case TOX_OBJECTS:
                        if(!uRet.compareTo(pShellRes->aTOXObjectsName))
                            uRet = C2U("Table of Objects");
                    break;
                    case TOX_TABLES:
                        if(!uRet.compareTo(pShellRes->aTOXTablesName))
                            uRet = C2U("Index of Tables");
                    break;
                    case TOX_AUTHORITIES:
                        if(!uRet.compareTo(pShellRes->aTOXAuthoritiesName))
                            uRet = C2U("Bibliography");
                    break;
                }
                aRet <<= uRet;
            }
            break;
            case WID_LEVEL      :
                bBOOL = sal_False;
                aRet <<= (sal_Int16)pTOXBase->GetLevel();
            break;
            case WID_CREATE_FROM_MARKS                 :
                bRet = 0 != (nCreate & TOX_MARK);
            break;
            case WID_CREATE_FROM_OUTLINE               :
                bRet = 0 != (nCreate & TOX_OUTLINELEVEL);
            break;
            case WID_CREATE_FROM_CHAPTER               :
                bRet = pTOXBase->IsFromChapter();
            break;
            case WID_CREATE_FROM_LABELS                :
                bRet = ! pTOXBase->IsFromObjectNames();
            break;
            case WID_PROTECTED                         :
                bRet = pTOXBase->IsProtected();
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                bRet = 0 != (nTOIOptions & TOI_ALPHA_DELIMITTER);
            break;
            case WID_USE_KEY_AS_ENTRY                  :
                bRet = 0 != (nTOIOptions & TOI_KEY_AS_ENTRY);
            break;
            case WID_USE_COMBINED_ENTRIES              :
                bRet = 0 != (nTOIOptions & TOI_SAME_ENTRY);
            break;
            case WID_IS_CASE_SENSITIVE                 :
                bRet = 0 != (nTOIOptions & TOI_CASE_SENSITIVE);
            break;
            case WID_USE_P_P:
                bRet = 0 != (nTOIOptions & TOI_FF);
            break;
            case WID_USE_DASH                          :
                bRet = 0 != (nTOIOptions & TOI_DASH);
            break;
            case WID_USE_UPPER_CASE                    :
                bRet = 0 != (nTOIOptions & TOI_INITIAL_CAPS);
            break;
            case WID_IS_COMMA_SEPARATED :
                bRet = rForm.IsCommaSeparated();
            break;
            case WID_LABEL_CATEGORY                    :
                aRet <<= OUString(pTOXBase->GetSequenceName());
                bBOOL = sal_False;
            break;
            case WID_LABEL_DISPLAY_TYPE                :
            {
                bBOOL = sal_False;
                sal_Int16 nSet = text::ReferenceFieldPart::TEXT;
                switch (pTOXBase->GetCaptionDisplay())
                {
                    case CAPTION_COMPLETE:  nSet = text::ReferenceFieldPart::TEXT;break;
                    case CAPTION_NUMBER  :  nSet = text::ReferenceFieldPart::CATEGORY_AND_NUMBER;   break;
                    case CAPTION_TEXT    :  nSet = text::ReferenceFieldPart::ONLY_CAPTION;      break;
                }
                aRet <<= nSet;
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE             :
                bRet = pTOXBase->IsLevelFromChapter();
            break;
            case WID_LEVEL_FORMAT                      :
            {
                Reference< container::XIndexReplace >  xTokenAcc =
                                    ((SwXDocumentIndex*)this)->GetTokenAccess();
                if(!xTokenAcc.is())
                    xTokenAcc = new SwXIndexTokenAccess_Impl(*
                                                (SwXDocumentIndex*)this);
                aRet.setValue(&xTokenAcc, ::getCppuType((const Reference<container::XIndexReplace>*)0));
                bBOOL = sal_False;
            }
            break;
            case WID_LEVEL_PARAGRAPH_STYLES            :
            {
                Reference< container::XIndexReplace >  xStyleAcc =
                                    ((SwXDocumentIndex*)this)->GetStyleAccess();
                if(!xStyleAcc.is())
                    xStyleAcc = new SwXIndexStyleAccess_Impl(*
                                                (SwXDocumentIndex*)this);
                aRet.setValue(&xStyleAcc, ::getCppuType((const Reference<container::XIndexReplace>*)0));
                bBOOL = sal_False;
            }
            break;
//          case WID_RECALC_TAB_STOPS                  :
//              tab stops are alway recalculated
//          break;
            //case WID_???                             :
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME   :
                bBOOL = sal_False;
                aRet <<= OUString(
                    SwXStyleFamilies::GetProgrammaticName(
                        pTOXBase->GetMainEntryCharStyle(), SFX_STYLE_FAMILY_CHAR));
            break;
            case WID_CREATE_FROM_TABLES                :
                bRet = 0 != (nCreate & TOX_TABLE);
            break;
            case WID_CREATE_FROM_TEXT_FRAMES           :
                bRet = 0 != (nCreate & TOX_FRAME);
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS       :
                bRet = 0 != (nCreate & TOX_GRAPHIC);
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS      :
                bRet = 0 != (nCreate & TOX_OLE);
            break;
            case WID_CREATE_FROM_STAR_MATH:
                bRet = 0 != (nOLEOptions & TOO_MATH);
            break;
            case WID_CREATE_FROM_STAR_CHART            :
                bRet = 0 != (nOLEOptions & TOO_CHART);
            break;
            case WID_CREATE_FROM_STAR_CALC             :
                bRet = 0 != (nOLEOptions & TOO_CALC);
            break;
            case WID_CREATE_FROM_STAR_DRAW             :
                bRet = 0 != (nOLEOptions & TOO_DRAW_IMPRESS);
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                bRet = 0 != (nOLEOptions & TOO_OTHER);
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                bRet = 0 != (nCreate & TOX_TEMPLATE);
            break;
            case WID_PARA_HEAD             :
                //Header steht an Pos 0
                aRet <<= OUString(
                    SwXStyleFamilies::GetProgrammaticName(rForm.GetTemplate( 0 ), SFX_STYLE_FAMILY_PARA));
                bBOOL = sal_False;
            break;
            case WID_PARA_SEP              :
                aRet <<= OUString(
                    SwXStyleFamilies::GetProgrammaticName(
                        rForm.GetTemplate( 1 ), SFX_STYLE_FAMILY_PARA));
                bBOOL = sal_False;
            break;
            case WID_PARA_LEV1             :
            case WID_PARA_LEV2             :
            case WID_PARA_LEV3             :
            case WID_PARA_LEV4             :
            case WID_PARA_LEV5             :
            case WID_PARA_LEV6             :
            case WID_PARA_LEV7             :
            case WID_PARA_LEV8             :
            case WID_PARA_LEV9             :
            case WID_PARA_LEV10            :
            {
                // im sdbcx::Index beginnt Lebel 1 bei Pos 2 sonst bei Pos 1
                sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                aRet <<= OUString(
                SwXStyleFamilies::GetProgrammaticName(
                        rForm.GetTemplate(nLPos + pMap->nWID - WID_PARA_LEV1),
                                                            SFX_STYLE_FAMILY_PARA));
                bBOOL = sal_False;
            }
            break;
            case WID_IS_RELATIVE_TABSTOPS:
                bRet = rForm.IsRelTabPos();
            break;
            case WID_INDEX_MARKS:
            {
                SwTOXMarks aMarks;
                const SwTOXType* pType = pTOXBase->GetTOXType();
                SwClientIter aIter(*(SwTOXType*)pType);
                SwTOXMark* pMark = (SwTOXMark*)aIter.First(TYPE(SwTOXMark));
                while( pMark )
                {
                    if(pMark->GetTxtTOXMark())
                        aMarks.C40_INSERT(SwTOXMark, pMark, aMarks.Count());
                    pMark = (SwTOXMark*)aIter.Next();
                }
                Sequence< Reference < XDocumentIndexMark > > aXMarks(aMarks.Count());
                Reference<XDocumentIndexMark>* pxMarks = aXMarks.getArray();
                for(USHORT i = 0; i < aMarks.Count(); i++)
                {
                    SwTOXMark* pMark = aMarks.GetObject(i);
                    pxMarks[i] = SwXDocumentIndexMark::GetObject((SwTOXType*)pType, pMark, m_pDoc);
                }
                aRet.setValue(&aXMarks, ::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0));
                bBOOL = sal_False;
            }
            break;
            default:
                //this is for items only
                bBOOL = sal_False;
                if(WID_PRIMARY_KEY > pMap->nWID)
                {
                    SfxItemPropertySet aPropSet(_pMap);
                    const SwAttrSet& rSet = m_pDoc->GetTOXBaseAttrSet(*pTOXBase);
                    aRet = aPropSet.getPropertyValue(*pMap, rSet);
                }
        }
        if(bBOOL)
            aRet.setValue(&bRet, ::getCppuBooleanType());
    }
    return aRet;
}
/*-- 14.12.98 09:35:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::addPropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 09:35:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::removePropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 09:35:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::addVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 09:35:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::removeVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 09:35:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    if(pOld && pOld->Which() == RES_REMOVE_UNO_OBJECT &&
        (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}
/* -----------------18.02.99 13:39-------------------
 *
 * --------------------------------------------------*/
void SwXDocumentIndex::attachToRange(const Reference< text::XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    if(!bIsDescriptor)
        throw RuntimeException();
    Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc )
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        //TODO: Unterscheidung innerhalb von Benutzerverzeichnissen einbauen
//      const SwTOXType* pType = pDoc->GetTOXType(pProps->eToxType, 0);
//      DBG_ASSERT(pType, "Wieso gibt es diesen Typ nicht?" )

            const SwTOXBase* pOld = pDoc->GetCurTOX( *aPam.Start() );
            if(!pOld)
            {
                UnoActionContext aAction(pDoc);
                if(aPam.HasMark())
                    pDoc->DeleteAndJoin(aPam);

                SwTOXBase& rTOXBase = pProps->GetTOXBase();
                //TODO: apply Section attributes (columns and background)
                const SwTOXBaseSection* pTOX = pDoc->InsertTableOf(
                                    *aPam.GetPoint(), rTOXBase, 0, sal_False );

                pDoc->SetTOXBaseName( *pTOX, pProps->GetTOXBase().GetTOXName() );

                // Seitennummern eintragen
                pBase = (const SwTOXBaseSection*)pTOX;
                pBase->GetFmt()->Add(this);
                ((SwTOXBaseSection*)pTOX)->UpdatePageNum();
            }
            else
                throw IllegalArgumentException();

        DELETEZ(pProps);
        m_pDoc = pDoc;
        bIsDescriptor = sal_False;
    }
}
/*-- 15.01.99 14:23:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::attach(const Reference< text::XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
}
/*-- 15.01.99 14:23:56---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XTextRange >  SwXDocumentIndex::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< text::XTextRange >   xRet;
    if(GetRegisteredIn())
    {
        SwSectionFmt*  pSectFmt = GetFmt();
        const SwNodeIndex* pIdx;
        if( 0 != ( pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, fnGoCntnt );
            aPaM.SetMark();
            aPaM.GetPoint()->nNode = *pIdx->GetNode().EndOfSectionNode();
            aPaM.Move( fnMoveBackward, fnGoCntnt );
            xRet = SwXTextRange::CreateTextRangeFromPosition(pSectFmt->GetDoc(),
                *aPaM.GetMark(), aPaM.GetPoint());
        }
    }
    else
        throw RuntimeException();
    return xRet;
}
/*-- 15.01.99 15:46:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_RemoveChildSections(SwSectionFmt& rParentFmt)
{
    SwSections aTmpArr;
    SwDoc* pDoc = rParentFmt.GetDoc();
    sal_uInt16 nCnt = rParentFmt.GetChildSections(aTmpArr,SORTSECT_POS);
    if( nCnt )
    {
        for( sal_uInt16 n = 0; n < nCnt; ++n )
            if( aTmpArr[n]->GetFmt()->IsInNodesArr() )
            {
                SwSectionFmt* pFmt = aTmpArr[n]->GetFmt();
                lcl_RemoveChildSections(*pFmt);
                pDoc->DelSectionFmt( pFmt );
            }
    }
}
void SwXDocumentIndex::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(GetRegisteredIn())
    {
        SwSectionFmt*  pSectFmt = GetFmt();
        pSectFmt->GetDoc()->DeleteTOX( *(SwTOXBaseSection*)pSectFmt->GetSection(), sal_True);
    }
    else
        throw RuntimeException();
}
/*-- 15.01.99 15:46:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::addEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 15.01.99 15:46:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndex::removeEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}
/* -----------------30.07.99 11:28-------------------

 --------------------------------------------------*/
OUString SwXDocumentIndex::getName(void) throw( RuntimeException )
{
    SwSectionFmt* pSectionFmt = GetFmt();
    OUString uRet;
    if(bIsDescriptor)
    {
        uRet = OUString(pProps->GetTOXBase().GetTOXName());
    }
    else if(pSectionFmt)
    {
        uRet = OUString(pSectionFmt->GetSection()->GetName());
    }
    else
        throw RuntimeException();
    return uRet;
}
/* -----------------30.07.99 11:28-------------------

 --------------------------------------------------*/
void SwXDocumentIndex::setName(const OUString& rName) throw( RuntimeException )
{
    SwSectionFmt* pSectionFmt = GetFmt();
    String sNewName(rName);
    sal_Bool bExcept = sal_False;
    if(!sNewName.Len())
        bExcept = sal_True;
    if(bIsDescriptor)
    {
        pProps->GetTOXBase().SetTOXName(sNewName);
    }
    else if(!pSectionFmt ||
        !pSectionFmt->GetDoc()->SetTOXBaseName(
                *(SwTOXBaseSection*)pSectionFmt->GetSection(), sNewName))
        bExcept = sal_True;

    if(bExcept)
        throw RuntimeException();
}

/******************************************************************
 * SwXDocumentIndexMark
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXDocumentIndexMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXDocumentIndexMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXDocumentIndexMark, SwClient)
const sal_Char cBaseMark[] = "com.sun.star.text.BaseIndexMark";
const sal_Char cContentMark[] = "com.sun.star.text.ContentIndexMark";
const sal_Char cIdxMark[] = "com.sun.star.text.DocumentIndexMark";
const sal_Char cUserMark[] = "com.sun.star.text.UserIndexMark";
const sal_Char cTextContent[] = "com.sun.star.text.TextContent";
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDocumentIndexMark::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndexMark");
}
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDocumentIndexMark::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cBaseMark)||
        !rServiceName.compareToAscii(cTextContent) ||
        (eType == TOX_USER && !rServiceName.compareToAscii(cUserMark)) ||
        (eType == TOX_CONTENT && !rServiceName.compareToAscii(cContentMark)) ||
        (eType == TOX_INDEX && !rServiceName.compareToAscii(cIdxMark));
}
/* -----------------------------06.04.00 15:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXDocumentIndexMark::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U(cBaseMark);
    pArray[1] = C2U(cTextContent);
    switch(eType)
    {
        case TOX_USER:      pArray[2] = C2U(cUserMark); break;
        case TOX_CONTENT:   pArray[2] = C2U(cContentMark);break;
        case TOX_INDEX:     pArray[2] = C2U(cIdxMark);break;
    }
    return aRet;
}
/*-- 14.12.98 10:25:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::SwXDocumentIndexMark(TOXTypes eToxType) :
    aTypeDepend(this, 0),
    m_pDoc(0),
    aLstnrCntnr( (text::XTextContent*)this),
    m_pTOXMark(0),
    nLevel(USHRT_MAX),
    eType(eToxType),
    bMainEntry(sal_False),
    bIsDescriptor(sal_True)
{
    InitMap(eToxType);
}
/*-- 14.12.98 10:25:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::SwXDocumentIndexMark(const SwTOXType* pType,
                                    const SwTOXMark* pMark,
                                    SwDoc* pDc) :
    aTypeDepend(this, (SwTOXType*)pType),
    aLstnrCntnr( (text::XTextContent*)this),
    m_pDoc(pDc),
    m_pTOXMark(pMark),
    nLevel(USHRT_MAX),
    eType(pType->GetType()),
    bIsDescriptor(sal_False),
    bMainEntry(sal_False)
{
    m_pDoc->GetUnoCallBack()->Add(this);
    InitMap(eType);
}
/*-- 14.12.98 10:25:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark::~SwXDocumentIndexMark()
{

}
/* -----------------21.04.99 09:36-------------------
 *
 * --------------------------------------------------*/
void SwXDocumentIndexMark::InitMap(TOXTypes eToxType)
{
    sal_uInt16 nMapId = PROPERTY_MAP_USER_MARK; //case TOX_USER:
    switch( eToxType )
    {
        case TOX_INDEX:
            nMapId = PROPERTY_MAP_INDEX_MARK ;
        break;
        case TOX_CONTENT:
            nMapId = PROPERTY_MAP_CNTIDX_MARK;
        break;
        //case TOX_USER:
    }
    _pMap = aSwMapProvider.GetPropertyMap(nMapId);
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXDocumentIndexMark::getMarkEntry(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    OUString sRet;
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        SwTOXMark aMark(*pCurMark);
        sRet = OUString(aMark.GetAlternativeText());
    }
    else if(bIsDescriptor)
         sRet = sAltText;
    else
        throw RuntimeException();
    return sRet;
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::setMarkEntry(const OUString& rIndexEntry) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        SwTOXMark aMark(*pCurMark);
        aMark.SetAlternativeText(rIndexEntry);
        SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
        SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
        aPam.SetMark();
        if(pTxtMark->GetEnd())
        {
            aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
        }
        else
            aPam.GetPoint()->nContent++;

        //die alte Marke loeschen
        m_pDoc->Delete(pCurMark);
        m_pTOXMark = pCurMark = 0;

        sal_Bool bInsAtPos = aMark.IsAlternativeText();
        const SwPosition *pStt = aPam.Start(),
                            *pEnd = aPam.End();
        SwUnoCrsr* pCrsr = 0;
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            m_pDoc->Insert( aTmp, aMark, 0 );
            pCrsr = m_pDoc->CreateUnoCrsr( *aTmp.Start() );
            pCrsr->Left(1);
        }
        else if( *pEnd != *pStt )
        {
            m_pDoc->Insert( aPam, aMark, SETATTR_DONTEXPAND );
            pCrsr = m_pDoc->CreateUnoCrsr( *aPam.Start() );
        }
        //und sonst - Marke geloescht?

        if(pCrsr)
        {
            SwTxtAttr* pTxtAttr = pCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                        pCrsr->GetPoint()->nContent, RES_TXTATR_TOXMARK);
            if(pTxtAttr)
                m_pTOXMark = &pTxtAttr->GetTOXMark();
        }
    }
    else if(bIsDescriptor)
    {
        sAltText = rIndexEntry;
    }
    else
        throw RuntimeException();
}
/* -----------------18.02.99 13:40-------------------
 *
 * --------------------------------------------------*/
void SwXDocumentIndexMark::attachToRange(const Reference< text::XTextRange > & xTextRange)
                throw( IllegalArgumentException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!bIsDescriptor)
        throw RuntimeException();

    Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;

    if(pDoc )
    {
        const SwTOXType* pTOXType = 0;
        switch(eType)
        {
            case TOX_INDEX:
            case TOX_CONTENT:
                pTOXType = pDoc->GetTOXType( eType, 0 );
            break;
            case TOX_USER:
            {
                if(!sUserIndexName.Len())
                    pTOXType = pDoc->GetTOXType( eType, 0 );
                else
                {
                    sal_uInt16 nCount = pDoc->GetTOXTypeCount( eType);
                    for(sal_uInt16 i = 0; i < nCount; i++)
                    {
                        const SwTOXType* pTemp = pDoc->GetTOXType( eType, i );
                        if(sUserIndexName == pTemp->GetTypeName())
                        {
                            pTOXType = pTemp;
                            break;
                        }
                    }
                    if(!pTOXType)
                    {
                        SwTOXType aUserType(TOX_USER, sUserIndexName);
                        pTOXType = pDoc->InsertTOXType(aUserType);
                    }
                }
            }
            break;
        }
        if(!pTOXType)
            throw IllegalArgumentException();
        pDoc->GetUnoCallBack()->Add(this);
        ((SwTOXType*)pTOXType)->Add(&aTypeDepend);

        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        SwTOXMark* pMark = new SwTOXMark(pTOXType);
        if(sAltText.Len())
            pMark->SetAlternativeText(sAltText);
        switch(eType)
        {
            case TOX_INDEX:
                if(sPrimaryKey.Len())
                    pMark->SetPrimaryKey(sPrimaryKey);
                if(sSecondaryKey.Len())
                    pMark->SetSecondaryKey(sSecondaryKey);
            break;
            case TOX_CONTENT:
                if(USHRT_MAX != nLevel)
                    pMark->SetLevel(nLevel);
            break;
        }
        UnoActionContext aAction(pDoc);
        sal_Bool bMark = *aPam.GetPoint() != *aPam.GetMark();
        // Marks ohne Alternativtext ohne selektierten Text koennen nicht eingefuegt werden,
        // deshalb hier ein Leerzeichen - ob das die ideale Loesung ist?
        if(!bMark && !pMark->GetAlternativeText().Len())
            pMark->SetAlternativeText(String::CreateFromAscii(" "));
        pDoc->Insert(aPam, *pMark, SETATTR_DONTEXPAND);
        if(bMark && *aPam.GetPoint() > *aPam.GetMark())
            aPam.Exchange();
        SwUnoCrsr* pCrsr = pDoc->CreateUnoCrsr( *aPam.Start() );
        if(!bMark)
        {
            pCrsr->SetMark();
            pCrsr->Left(1);
        }
        SwTxtAttr* pTxtAttr = pCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                        pCrsr->GetPoint()->nContent, RES_TXTATR_TOXMARK);
        delete pCrsr;
        if(pTxtAttr)
        {
            m_pTOXMark = &pTxtAttr->GetTOXMark();
            m_pDoc = pDoc;
            bIsDescriptor = sal_False;
        }
        else
            throw RuntimeException();
    }
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::attach(const Reference< text::XTextRange > & xTextRange)
                throw( IllegalArgumentException, RuntimeException )
{
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XTextRange >  SwXDocumentIndexMark::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< text::XTextRange >  aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark && pCurMark->GetTxtTOXMark())
        {
            SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
            SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
            aPam.SetMark();
            if(pTxtMark->GetEnd())
            {
                aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
            }
            else
                aPam.GetPoint()->nContent++;
            Reference< frame::XModel >  xModel = m_pDoc->GetDocShell()->GetBaseModel();
            Reference< text::XTextDocument > xTDoc(xModel, uno::UNO_QUERY);
            aRet = new SwXTextRange(aPam, xTDoc->getText());
        }
    }
    if(!aRet.is())
        throw RuntimeException();
    return aRet;
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType)
    {
        SwTOXMark* pTMark = lcl_GetMark(pType, GetTOXMark());
        m_pDoc->Delete(pTMark);
    }
    else
        throw RuntimeException();
}
/*-- 14.12.98 10:25:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addEventListener(const Reference< XEventListener > & aListener)
    throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removeEventListener(const Reference< XEventListener > & aListener)
    throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXDocumentIndexMark::getPropertySetInfo(void)
    throw( RuntimeException )
{
    uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(_pMap);
    // extend PropertySetInfo!
    const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
    Reference< XPropertySetInfo >  xRef = new SfxExtItemPropertySetInfo(
        aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
        aPropSeq );
    return xRef;
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::setPropertyValue(const OUString& rPropertyName,
                                            const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    if(pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();

    if(pType)
    {
        SwDoc* pLocalDoc = m_pDoc;
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark)
        {
            SwTOXMark aMark(*pCurMark);
            switch(pMap->nWID)
            {
                case WID_ALT_TEXT:
                    aMark.SetAlternativeText(lcl_AnyToString(aValue));
                break;
                case WID_LEVEL:
                    aMark.SetLevel(std::min((sal_Int8) MAXLEVEL, (sal_Int8)lcl_AnyToInt16(aValue)));
                break;
                case WID_PRIMARY_KEY  :
                    aMark.SetPrimaryKey(lcl_AnyToString(aValue));
                break;
                case WID_SECONDARY_KEY:
                    aMark.SetSecondaryKey(lcl_AnyToString(aValue));
                break;
                case WID_MAIN_ENTRY:
                    aMark.SetMainEntry(*(sal_Bool*)aValue.getValue());
                break;
            }

            SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
            SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
            aPam.SetMark();
            if(pTxtMark->GetEnd())
            {
                aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
            }
            else
                aPam.GetPoint()->nContent++;

            //die alte Marke loeschen
            pLocalDoc->Delete(pCurMark);
            m_pTOXMark = pCurMark = 0;

            sal_Bool bInsAtPos = aMark.IsAlternativeText();
            const SwPosition *pStt = aPam.Start(),
                                *pEnd = aPam.End();
            SwUnoCrsr* pCrsr = 0;
            if( bInsAtPos )
            {
                SwPaM aTmp( *pStt );
                pLocalDoc->Insert( aTmp, aMark, 0 );
                pCrsr = pLocalDoc->CreateUnoCrsr( *aTmp.Start() );
                pCrsr->Left(1);
            }
            else if( *pEnd != *pStt )
            {
                pLocalDoc->Insert( aPam, aMark, SETATTR_DONTEXPAND );
                pCrsr = pLocalDoc->CreateUnoCrsr( *aPam.Start() );
            }
            m_pDoc = pLocalDoc;
            //und sonst - Marke geloescht?

            if(pCrsr)
            {
                SwTxtAttr* pTxtAttr = pCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                            pCrsr->GetPoint()->nContent, RES_TXTATR_TOXMARK);
                if(pTxtAttr)
                {
                    m_pTOXMark = &pTxtAttr->GetTOXMark();
                    m_pDoc->GetUnoCallBack()->Add(this);
                    pType->Add(&aTypeDepend);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        switch(pMap->nWID)
        {
            case WID_ALT_TEXT:
                sAltText = lcl_AnyToString(aValue);
            break;
            case WID_LEVEL:
            {
                sal_uInt16 nVal = lcl_AnyToInt16(aValue);
                if(nVal >= 0 && nVal < MAXLEVEL)
                    nLevel = nVal;
                else
                    throw IllegalArgumentException();
            }
            break;
            case WID_PRIMARY_KEY  :
                sPrimaryKey = lcl_AnyToString(aValue);
            break;
            case WID_SECONDARY_KEY:
                sSecondaryKey = lcl_AnyToString(aValue);
            break;
            case WID_USER_IDX_NAME :
                sUserIndexName = lcl_AnyToString(aValue);
            break;
            case WID_MAIN_ENTRY:
                bMainEntry = *(sal_Bool*)aValue.getValue();
            break;
        }
    }
    else
        throw RuntimeException();
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexMark::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);

    if(!pMap)
        throw UnknownPropertyException();
     if(SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName, pMap->nWID))
        return aRet;
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark)
        {
            switch(pMap->nWID)
            {
                case WID_ALT_TEXT:
                    aRet <<= OUString(pCurMark->GetAlternativeText());
                break;
                case WID_LEVEL:
                    aRet <<= (sal_Int16)pCurMark->GetLevel();
                break;
                case WID_PRIMARY_KEY  :
                    aRet <<= OUString(pCurMark->GetPrimaryKey());
                break;
                case WID_SECONDARY_KEY:
                    aRet <<= OUString(pCurMark->GetSecondaryKey());
                break;
                case WID_USER_IDX_NAME :
                    aRet <<= OUString(pType->GetTypeName());
                break;
                case WID_MAIN_ENTRY:
                {
                    sal_Bool bTemp = pCurMark->IsMainEntry();
                    aRet.setValue(&bTemp, ::getBooleanCppuType());
                }
                break;
            }
        }
    }
    else if(bIsDescriptor)
    {
        switch(pMap->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= OUString(sAltText);
            break;
            case WID_LEVEL:
                aRet <<= (sal_Int16)nLevel;
            break;
            case WID_PRIMARY_KEY  :
                aRet <<= OUString(sPrimaryKey);
            break;
            case WID_SECONDARY_KEY:
                aRet <<= OUString(sSecondaryKey);
            break;
            case WID_USER_IDX_NAME :
                aRet <<= OUString(sUserIndexName);
            break;
            case WID_MAIN_ENTRY:
            {
                aRet.setValue(&bMainEntry, ::getBooleanCppuType());
            }
            break;
        }
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addPropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 10:25:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removePropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::addVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::removeVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexMark*   SwXDocumentIndexMark::GetObject(SwTOXType* pType,
                                    const SwTOXMark* pMark, SwDoc* pDoc)
{
    SwClientIter aIter(*pType);
    SwXDocumentIndexMark* pxMark = (SwXDocumentIndexMark*)
                                            aIter.First(TYPE(SwXDocumentIndexMark));
    while( pxMark )
    {
        if(pxMark->GetTOXMark() == pMark)
            return pxMark;
        pxMark = (SwXDocumentIndexMark*)aIter.Next();
    }
    return new SwXDocumentIndexMark(pType, pMark, pDoc);
}
/*-- 14.12.98 10:25:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXDocumentIndexMark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}
/* -----------------------------16.10.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
void    SwXDocumentIndexMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        if(aTypeDepend.GetRegisteredIn())
            ((SwModify*)aTypeDepend.GetRegisteredIn())->Remove(&aTypeDepend);
        aLstnrCntnr.Disposing();
        m_pTOXMark = 0;
        m_pDoc = 0;
    }
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDocumentIndexes::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndexes");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDocumentIndexes::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexes") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXDocumentIndexes::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexes");
    return aRet;
}
/*-- 05.05.99 13:14:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexes::SwXDocumentIndexes(SwDoc* pDoc) :
    SwUnoCollection(pDoc)
{
}
/*-- 05.05.99 13:15:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDocumentIndexes::~SwXDocumentIndexes()
{
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXDocumentIndexes::getCount(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();

    sal_uInt32 nRet = 0;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexes::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();

    uno::Any aRet;
    sal_uInt32 nIdx = 0;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
            nIdx++ == nIndex )
            {
               Reference< text::XDocumentIndex >  xTmp = new SwXDocumentIndex(
                                    (SwTOXBaseSection*)pSect, GetDoc() );
               aRet.setValue(&xTmp, ::getCppuType((Reference<text::XDocumentIndex>*)0));
               return aRet;
            }
    }

    throw IndexOutOfBoundsException();
    return aRet;
}

/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDocumentIndexes::getByName(const OUString& rName)
    throw( container::NoSuchElementException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();

    uno::Any aRet;
    sal_uInt32 nIdx = 0;

    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
                ((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
            {
               Reference< text::XDocumentIndex >  xTmp = new SwXDocumentIndex(
                                    (SwTOXBaseSection*)pSect, GetDoc() );
               aRet.setValue(&xTmp, ::getCppuType((Reference<text::XDocumentIndex>*)0));
               return aRet;
            }
    }
    throw container::NoSuchElementException();
    return aRet;
}
/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXDocumentIndexes::getElementNames(void)
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    sal_Int32 nCount = 0;
    sal_uInt16 n;
    for( n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nCount;
    }

    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    sal_uInt16 nCnt;
    for( n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            pArray[nCnt++] = OUString(((SwTOXBaseSection*)pSect)->GetTOXName());
        }
    }
    return aRet;
}
/*-- 31.01.00 10:12:31---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXDocumentIndexes::hasByName(const OUString& rName)
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();

    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            if(((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
                return sal_True;
        }
    }
    return sal_False;
}
/*-- 05.05.99 13:15:01---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXDocumentIndexes::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((Reference< text::XDocumentIndex> *)0);
}
/*-- 05.05.99 13:15:02---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXDocumentIndexes::hasElements(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw RuntimeException();
    return 0 != getCount();
}

SwXDocumentIndex* SwXDocumentIndexes::GetObject(const SwTOXBaseSection* pTOX)
{
    SwSectionFmt* pFmt = pTOX->GetFmt();
    SwClientIter aIter(*pFmt);
    SwXDocumentIndex* pxIdx = (SwXDocumentIndex*)aIter.First(TYPE(SwXDocumentIndex));
    if(pxIdx)
        return pxIdx;
    else
        return new SwXDocumentIndex(pTOX, pFmt->GetDoc());
}

/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXIndexStyleAccess_Impl::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXIndexStyleAccess_Impl");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXIndexStyleAccess_Impl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexParagraphStyles") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXIndexStyleAccess_Impl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexParagraphStyles");
    return aRet;
}
/*-- 13.09.99 16:52:28---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexStyleAccess_Impl::SwXIndexStyleAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    rParent.SetStyleAccess(this);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexStyleAccess_Impl::~SwXIndexStyleAccess_Impl()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    rParent.SetStyleAccess(0);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXIndexStyleAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( IllegalArgumentException, IndexOutOfBoundsException,
          WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    if(nIndex < 0 || nIndex > MAXLEVEL)
        throw IndexOutOfBoundsException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
            (SwTOXBaseSection*)pSectFmt->GetSection();
    if(rElement.getValueType() != ::getCppuType((uno::Sequence<OUString>*)0))
        throw IllegalArgumentException();
    const uno::Sequence<OUString>* pSeq = (const uno::Sequence<OUString>*)rElement.getValue();

    sal_uInt16 nStyles = pSeq->getLength();
    const OUString* pStyles = pSeq->getConstArray();
    String sSetStyles;
    for(sal_uInt16 i = 0; i < nStyles; i++)
    {
        if(i)
            sSetStyles += TOX_STYLE_DELIMITER;
        sSetStyles += SwXStyleFamilies::GetUIName(pStyles[i], SFX_STYLE_FAMILY_PARA);
    }
    pTOXBase->SetStyleNames(sSetStyles, (sal_uInt16) nIndex);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXIndexStyleAccess_Impl::getCount(void) throw( RuntimeException )
{
    return MAXLEVEL;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXIndexStyleAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException,
                 RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    if(nIndex < 0 || nIndex > MAXLEVEL)
        throw IndexOutOfBoundsException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
                (SwTOXBaseSection*)pSectFmt->GetSection();

    const String& rStyles = pTOXBase->GetStyleNames((sal_uInt16) nIndex);
    sal_uInt16 nStyles = rStyles.GetTokenCount(TOX_STYLE_DELIMITER);
    uno::Sequence<OUString> aStyles(nStyles);
    OUString* pStyles = aStyles.getArray();
    for(sal_uInt16 i = 0; i < nStyles; i++)
    {
        pStyles[i] = OUString(SwXStyleFamilies::GetProgrammaticName(
            rStyles.GetToken(i, TOX_STYLE_DELIMITER), SFX_STYLE_FAMILY_PARA));
    }
    uno::Any aRet(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
    return aRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXIndexStyleAccess_Impl::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((uno::Sequence<OUString>*)0);
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXIndexStyleAccess_Impl::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

/* -----------------13.09.99 16:51-------------------

 --------------------------------------------------*/
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXIndexTokenAccess_Impl::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXIndexTokenAccess_Impl");
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXIndexTokenAccess_Impl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexLevelFormat") == rServiceName;
}
/* -----------------------------06.04.00 15:08--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXIndexTokenAccess_Impl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexLevelFormat");
    return aRet;
}
/*-- 13.09.99 16:52:28---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexTokenAccess_Impl::SwXIndexTokenAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx),
    nCount(SwForm::GetFormMaxLevel(rParent.GetTOXType()))
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    rParent.SetTokenAccess(this);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXIndexTokenAccess_Impl::~SwXIndexTokenAccess_Impl()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    rParent.SetTokenAccess(0);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXIndexTokenAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( IllegalArgumentException, IndexOutOfBoundsException,
            WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();

    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
                            (SwTOXBaseSection*)pSectFmt->GetSection();
    if(nIndex < 0 ||
        (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
            throw IndexOutOfBoundsException();
    if(rElement.getValueType() != ::getCppuType((uno::Sequence<PropertyValues>*)0))
        throw IllegalArgumentException();
    const uno::Sequence<PropertyValues>* pSeq = (uno::Sequence<PropertyValues>* )rElement.getValue();

    String sPattern;
    sal_uInt16 nTokens = pSeq->getLength();
    const PropertyValues* pTokens = pSeq->getConstArray();
    for(sal_uInt16 i = 0; i < nTokens; i++)
    {
        const PropertyValue* pProperties = pTokens[i].getConstArray();
        sal_uInt16 nProperties = pTokens[i].getLength();
        //create an invalid token
        SwFormToken aToken(TOKEN_END);
        for(sal_uInt16 j = 0; j < nProperties; j++)
        {
            if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TokenType"))
            {
                const String sTokenType =
                        lcl_AnyToString(pProperties[j].Value);
                if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenEntryNumber"))
                    aToken.eTokenType = TOKEN_ENTRY_NO;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenEntryText" ))
                    aToken.eTokenType = TOKEN_ENTRY_TEXT;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenTabStop"   ))
                    aToken.eTokenType = TOKEN_TAB_STOP;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenText"      ))
                    aToken.eTokenType = TOKEN_TEXT;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenPageNumber"))
                    aToken.eTokenType = TOKEN_PAGE_NUMS;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenChapterInfo"      ))
                    aToken.eTokenType = TOKEN_CHAPTER_INFO;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenHyperlinkStart" ))
                    aToken.eTokenType = TOKEN_LINK_START;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenHyperlinkEnd"))
                    aToken.eTokenType = TOKEN_LINK_END;
                else if(COMPARE_EQUAL == sTokenType.CompareToAscii("TokenBibliographyDataField" ))
                    aToken.eTokenType = TOKEN_AUTHORITY;
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("CharacterStyleName"  )  )
            {
                const String sCharStyleName =
                    SwXStyleFamilies::GetUIName(
                        lcl_AnyToString(pProperties[j].Value),
                            SFX_STYLE_FAMILY_CHAR);
                aToken.sCharStyleName = sCharStyleName;
                aToken.nPoolId = pSectFmt->GetDoc()->
                            GetPoolId( sCharStyleName, GET_POOLID_CHRFMT );
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TabStopRightAligned") )
            {
                sal_Bool bRight = lcl_AnyToBool(pProperties[j].Value);
                aToken.eTabAlign = bRight ?
                                    SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT;
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TabStopPosition"  ))
            {
                sal_Int32 nPosition;
                if(pProperties[j].Value.getValueType() != ::getCppuType((sal_Int32*)0))
                    throw IllegalArgumentException();
                pProperties[j].Value >>= nPosition;
                nPosition = MM100_TO_TWIP(nPosition);
                if(nPosition < 0)
                    throw IllegalArgumentException();
                aToken.nTabStopPosition = nPosition;
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TabStopFillCharacter" ))
            {
                const String sFillChar =
                    lcl_AnyToString(pProperties[j].Value);
                if(sFillChar.Len() > 1)
                    throw IllegalArgumentException();
                aToken.cTabFillChar = sFillChar.Len() ?
                                sFillChar.GetChar(0) : ' ';
            }

            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("Text" ))
               {
                const String sText =
                    lcl_AnyToString(pProperties[j].Value);
                aToken.sText = sText;
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("ChapterFormat"    ))
            {
                sal_Int16 nFormat = lcl_AnyToInt16(pProperties[j].Value);
                switch(nFormat)
                {
                    case text::ChapterFormat::NUMBER:           nFormat = CF_NUMBER;
                    break;
                    case text::ChapterFormat::NAME:             nFormat = CF_TITLE;
                    break;
                    case text::ChapterFormat::NAME_NUMBER:      nFormat = CF_NUM_TITLE;
                    break;
                    case text::ChapterFormat::NO_PREFIX_SUFFIX:nFormat = CF_NUMBER_NOPREPST;
                    break;
                    case text::ChapterFormat::DIGIT:           nFormat = CF_NUM_NOPREPST_TITLE;
                    break;
                    default:
                        throw IllegalArgumentException();
                }
                aToken.nChapterFormat = nFormat;
            }
            else if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("BibliographyDataField"))
            {
                sal_Int16 nType; pProperties[j].Value >>= nType;
                if(nType < 0 || nType > BibliographyDataField::ISBN)
                {
                    IllegalArgumentException aExcept;
                    aExcept.Message = C2U("BibliographyDataField - wrong value");
                    aExcept.ArgumentPosition = j;
                    throw aExcept;
                }
                aToken.nAuthorityField = nType;
            }

        }
        //exception if wrong TokenType
        if(TOKEN_END <= aToken.eTokenType )
            throw IllegalArgumentException();
        // set TokenType from TOKEN_ENTRY_TEXT to TOKEN_ENTRY if it is
        // not a content index
        if(TOKEN_ENTRY_TEXT == aToken.eTokenType &&
                                TOX_CONTENT != pTOXBase->GetType())
            aToken.eTokenType = TOKEN_ENTRY;
        sPattern += aToken.GetString();
    }
    SwForm aForm(pTOXBase->GetTOXForm());
    aForm.SetPattern((sal_uInt16) nIndex, sPattern);
    pTOXBase->SetTOXForm(aForm);
}
/*-- 13.09.99 16:52:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXIndexTokenAccess_Impl::getCount(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    sal_Int32 nRet = bDescriptor ?
        nCount :
        ((SwTOXBaseSection*)pSectFmt->GetSection())->
                                            GetTOXForm().GetFormMax();
    return nRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXIndexTokenAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException,
         RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
            (SwTOXBaseSection*)pSectFmt->GetSection();
    if(nIndex < 0 ||
    (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
        throw IndexOutOfBoundsException();

    SwFormTokenEnumerator aEnumerator(pTOXBase->GetTOXForm().
                                        GetPattern((sal_uInt16) nIndex));
    sal_uInt16 nTokenCount = 0;
    uno::Sequence< PropertyValues > aRetSeq;
    while(aEnumerator.HasNextToken())
    {
        nTokenCount++;
        aRetSeq.realloc(nTokenCount);
        PropertyValues* pTokenProps = aRetSeq.getArray();
        SwFormToken  aToken = aEnumerator.GetNextToken();

        Sequence< PropertyValue >& rCurTokenSeq = pTokenProps[nTokenCount-1];
        switch(aToken.eTokenType)
        {
            case TOKEN_ENTRY_NO     :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenEntryNumber");
//              pArr[0].Value <<= C2U("TokenEntryNumber");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= OUString(
                    SwXStyleFamilies::GetProgrammaticName(
                        aToken.sCharStyleName,
                            SFX_STYLE_FAMILY_CHAR));
            }
            break;
            case TOKEN_ENTRY        :   // no difference between Entry and Entry Text
            case TOKEN_ENTRY_TEXT   :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenEntryText");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= OUString(aToken.sCharStyleName);
            }
            break;
            case TOKEN_TAB_STOP     :
            {
                rCurTokenSeq.realloc(3);
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenTabStop");


                if(SVX_TAB_ADJUST_END == aToken.eTabAlign)
                {
                    pArr[1].Name = C2U("TabStopRightAligned");
                    BOOL bTemp = sal_True;
                    pArr[1].Value.setValue(&bTemp, ::getCppuBooleanType());
                }
                else
                {
                    pArr[1].Name = C2U("TabStopPosition");
                    pArr[1].Value <<= (sal_Int32)(TWIP_TO_MM100(aToken.nTabStopPosition));
                }
                pArr[2].Name = C2U("TabStopFillCharacter");
                pArr[2].Value <<= OUString(aToken.cTabFillChar);
            }
            break;
            case TOKEN_TEXT         :
            {
                rCurTokenSeq.realloc( 3 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenText");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= OUString(aToken.sCharStyleName);

                pArr[2].Name = C2U("Text");
                pArr[2].Value <<= OUString(aToken.sText);
            }
            break;
            case TOKEN_PAGE_NUMS    :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenPageNumber");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= OUString(aToken.sCharStyleName);
            }
            break;
            case TOKEN_CHAPTER_INFO :
            {
                rCurTokenSeq.realloc( 3 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenChapterInfo");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= OUString(aToken.sCharStyleName);

                pArr[2].Name = C2U("ChapterFormat");
                sal_Int16 nVal = text::ChapterFormat::NUMBER;
                switch(aToken.nChapterFormat)
                {
                    case CF_NUMBER:             nVal = text::ChapterFormat::NUMBER; break;
                    case CF_TITLE:              nVal = text::ChapterFormat::NAME; break;
                    case CF_NUM_TITLE:          nVal = text::ChapterFormat::NAME_NUMBER; break;
                    case CF_NUMBER_NOPREPST:    nVal = text::ChapterFormat::NO_PREFIX_SUFFIX; break;
                    case CF_NUM_NOPREPST_TITLE: nVal = text::ChapterFormat::DIGIT; break;
                }
                pArr[2].Value <<= (sal_Int16)nVal;
            }
            break;
            case TOKEN_LINK_START   :
            {
                rCurTokenSeq.realloc( 1 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenHyperlinkStart");
            }
            break;
            case TOKEN_LINK_END     :
            {
                rCurTokenSeq.realloc( 1 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenHyperlinkEnd");
            }
            break;
            case TOKEN_AUTHORITY :
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString::createFromAscii("TokenBibliographyDataField");
                pArr[1].Name = C2U("BibliographyDataField");
                pArr[1].Value <<= sal_Int16(aToken.nAuthorityField);
            break;
        }
    }

    uno::Any aRet(&aRetSeq, ::getCppuType((uno::Sequence< PropertyValues >*)0));

    return aRet;
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXIndexTokenAccess_Impl::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((uno::Sequence< PropertyValues >*)0);
}
/*-- 13.09.99 16:52:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXIndexTokenAccess_Impl::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

