/*************************************************************************
 *
 *  $RCSfile: xmlfmt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _FORMAT_HXX //autogen wg. SwFmt
#include <format.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen wg. SwTxtFmtColl
#include <fmtcol.hxx>
#endif
#ifndef _HINTS_HXX //autogen wg. SwFmtChg
#include <hints.hxx>
#endif

#ifndef _POOLFMT_HXX //autogen wg. RES_POOL_CHRFMT_TYPE
#include <poolfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen wg. SwFrmFmt
#include <frmfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#include "docary.hxx"
#ifndef _UNOSTYLE_HXX
#include "unostyle.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <xmloff/i18nmap.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLITEM_HXX
#include <xmloff/xmlitem.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_TXTSTYLI_HXX
#include <xmloff/txtstyli.hxx>
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif

#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLNUM_HXX
#include "xmlnum.hxx"
#endif
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::rtl;

class SwXMLConditionParser_Impl
{
    OUString sInput;

    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    sal_Int32 nPos;
    sal_Int32 nLength;

    inline sal_Bool SkipWS();
    inline sal_Bool MatchChar( sal_Unicode c );
    inline sal_Bool MatchName( OUString& rName );
    inline sal_Bool MatchNumber( sal_uInt32& rNumber );

public:

    SwXMLConditionParser_Impl( const OUString& rInp );

    sal_Bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
};

inline sal_Bool SwXMLConditionParser_Impl::SkipWS()
{
    while( nPos < nLength && ' ' == sInput[nPos] )
        nPos++;
    return sal_True;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchChar( sal_Unicode c )
{
    sal_Bool bRet = sal_False;
    if( nPos < nLength && c == sInput[nPos] )
    {
        nPos++;
        bRet = sal_True;
    }
    return bRet;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchName( OUString& rName )
{
    OUStringBuffer sBuffer( nLength );
    while( nPos < nLength &&
           ( ('a' <= sInput[nPos] && sInput[nPos] <= 'z') ||
              '-' == sInput[nPos] ) )
    {
        sBuffer.append( sInput[nPos] );
        nPos++;
    }
    rName = sBuffer.makeStringAndClear();
    return rName.getLength() > 0;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchNumber( sal_uInt32& rNumber )
{
    OUStringBuffer sBuffer( nLength );
    while( nPos < nLength && '0' <= sInput[nPos] && sInput[nPos] <= '9' )
    {
        sBuffer.append( sInput[nPos] );
        nPos++;
    }

    OUString sNum( sBuffer.makeStringAndClear() );
    if( sNum.getLength() )
        rNumber = sNum.toInt32();
    return sNum.getLength() > 0;
}

SwXMLConditionParser_Impl::SwXMLConditionParser_Impl( const OUString& rInp ) :
    sInput( rInp ),
    nPos( 0 ),
    nLength( rInp.getLength() ),
    nCondition( 0 ),
    nSubCondition( 0 )
{
    OUString sFunc;
    sal_Bool bHasSub = sal_False;
    sal_uInt32 nSub = 0;
    sal_Bool bOK = SkipWS() && MatchName( sFunc ) && SkipWS() &&
               MatchChar( '(' ) && SkipWS() && MatchChar( ')' ) && SkipWS();
    if( bOK && MatchChar( '=' ) )
    {
        bOK = SkipWS() && MatchNumber( nSub ) && SkipWS();
        bHasSub = sal_True;
    }

    bOK &= nPos == nLength;

    if( bOK )
    {
        switch( sFunc[0] )
        {
        case 'e':
            if( sFunc.compareToAscii( sXML_endnote ) == 0 && !bHasSub )
                nCondition = PARA_IN_ENDNOTE;
            break;

        case 'f':
            if( sFunc.compareToAscii( sXML_footer ) == 0 && !bHasSub )
                nCondition = PARA_IN_FOOTER;
            else if( sFunc.compareToAscii( sXML_table_header ) == 0 &&
                     !bHasSub )
                nCondition = PARA_IN_FOOTENOTE;
            break;

        case 'h':
            if( sFunc.compareToAscii( sXML_header ) == 0 && !bHasSub )
                nCondition = PARA_IN_HEADER;
            break;

        case 'l':
            if( sFunc.compareToAscii( sXML_list_level ) == 0 &&
                nSub >=1 && nSub < MAXLEVEL )
            {
                nCondition = PARA_IN_LIST;
                nSubCondition = nSub-1;
            }
            break;

        case 'o':
            if( sFunc.compareToAscii( sXML_outline_level ) == 0 &&
                nSub >=1 && nSub < MAXLEVEL )
            {
                nCondition = PARA_IN_OUTLINE;
                nSubCondition = nSub-1;
            }
            break;

        case 's':
            if( sFunc.compareToAscii( sXML_section ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_SECTION;
            }
            break;

        case 't':
            if( sFunc.compareToAscii( sXML_table ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_TABLEBODY;
            }
            else if( sFunc.compareToAscii( sXML_table_header ) == 0 &&
                     !bHasSub )
            {
                nCondition = PARA_IN_TABLEHEAD;
            }
            else if( sFunc.compareToAscii( sXML_text_box ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_FRAME;
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    OUString sApplyStyle;

    void ParseCondition( const OUString& rCond );

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLConditionContext_Impl();

    sal_Bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
    const OUString& GetApplyStyle() const { return sApplyStyle; }
};

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nCondition( 0 ),
    nSubCondition( 0 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( aLocalName.compareToAscii( sXML_condition ) == 0 )
            {
                SwXMLConditionParser_Impl aCondParser( rValue );
                if( aCondParser.IsValid() )
                {
                    nCondition = aCondParser.GetCondition();
                    nSubCondition = aCondParser.GetSubCondition();
                }
            }
            else if( aLocalName.compareToAscii( sXML_apply_style_name ) == 0 )
            {
                sApplyStyle = rValue;
            }
        }
    }
}

SwXMLConditionContext_Impl::~SwXMLConditionContext_Impl()
{
}

// ---------------------------------------------------------------------

typedef SwXMLConditionContext_Impl *SwXMLConditionContextPtr;
SV_DECL_PTRARR( SwXMLConditions_Impl, SwXMLConditionContextPtr, 5, 2 );

#ifndef XML_CORE_API

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    SwXMLConditions_Impl    *pConditions;

protected:

    virtual Reference < XStyle > Create();

public:

    TYPEINFO();

    SwXMLTextStyleContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nFamily,
            SvXMLStylesContext& rStyles );
    virtual ~SwXMLTextStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );
};

TYPEINIT1( SwXMLTextStyleContext_Impl, XMLTextStyleContext );

Reference < XStyle > SwXMLTextStyleContext_Impl::Create()
{
    Reference < XStyle > xNewStyle;

    if( pConditions && XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() )
    {
        Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
        if( xFactory.is() )
        {
            OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.style.ConditionalParagraphStyle" ) );
            Reference < XInterface > xIfc =
                xFactory->createInstance( sServiceName );
            if( xIfc.is() )
                xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
        }
    }
    else
    {
        xNewStyle = XMLTextStyleContext::Create();
    }

    return xNewStyle;
}

SwXMLTextStyleContext_Impl::SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 nFamily,
        SvXMLStylesContext& rStyles ) :
    XMLTextStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles ),
    pConditions( 0 )
{
    SetFamily( nFamily );
}

SwXMLTextStyleContext_Impl::~SwXMLTextStyleContext_Impl()
{
    if( pConditions )
    {
        while( pConditions->Count() )
        {
            SwXMLConditionContext_Impl *pCond = pConditions->GetObject(0);
            pConditions->Remove( 0UL );
            pCond->ReleaseRef();
        }
        delete pConditions;
    }
}

SvXMLImportContext *SwXMLTextStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.compareToAscii( sXML_map ) == 0 )
    {
        SwXMLConditionContext_Impl *pCond =
            new SwXMLConditionContext_Impl( GetImport(), nPrefix,
                                            rLocalName, xAttrList );
        if( pCond->IsValid() )
        {
            if( !pConditions )
               pConditions = new SwXMLConditions_Impl;
            pConditions->Insert( pCond, pConditions->Count() );
            pCond->AddRef();
        }
        pContext = pCond;
    }

    if( !pContext )
        pContext = XMLTextStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void SwXMLTextStyleContext_Impl::Finish( sal_Bool bOverwrite )
{
    XMLTextStyleContext::Finish( bOverwrite );

    if( !pConditions || XML_STYLE_FAMILY_TEXT_PARAGRAPH != GetFamily() )
        return;

    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() )
        return;

    const SwXStyle* pStyle = 0;
    Reference<XUnoTunnel> xStyleTunnel( xStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId() );
    }
    if( !pStyle )
        return;

    SwDoc& rDoc = ((SwXMLImport&)GetImport()).GetDoc();

    const OUString& rName =
                SwXStyleFamilies::GetUIName( GetName(), SFX_STYLE_FAMILY_PARA );
    SwTxtFmtColl *pColl = rDoc.FindTxtFmtCollByName( rName );
    ASSERT( pColl, "Text collection not found" );
    if( !pColl || RES_CONDTXTFMTCOLL != pColl->Which() )
        return;

    sal_uInt16 nCount = pConditions->Count();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        const SwXMLConditionContext_Impl *pCond = (*pConditions)[i];
        const OUString& rName =
                SwXStyleFamilies::GetUIName( pCond->GetApplyStyle(),
                                             SFX_STYLE_FAMILY_PARA  );
        SwTxtFmtColl* pCondColl = rDoc.FindTxtFmtCollByName( rName );
        ASSERT( pCondColl,
            "SwXMLItemSetStyleContext_Impl::ConnectConditions: cond coll missing" );
        if( pCondColl )
        {
            SwCollCondition aCond( pCondColl, pCond->GetCondition(),
                                              pCond->GetSubCondition() );
            ((SwConditionTxtFmtColl*)pColl)->InsertCondition( aCond );
        }
    }
}
#endif

// ---------------------------------------------------------------------

class SwXMLItemSetStyleContext_Impl : public SvXMLStyleContext
{
#ifdef XML_CORE_API
    OUString                sListStyleName;
    SwFmt                   *pFmt;
#endif
    SfxItemSet              *pItemSet;

#ifdef XML_CORE_API
    SwXMLConditions_Impl    *pConditions;

    sal_uInt16  nPoolId;                    // PoolId
    sal_Bool    bAutoUpdate;
#endif

    SvXMLImportContext *CreateItemSetContext(
            sal_uInt16 nPrefix,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList);

protected:

#ifdef XML_CORE_API
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue );
#endif

    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    TYPEINFO();

    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList
#ifndef XML_CORE_API
            ,sal_uInt16 nFamily
#endif
                                 );
    virtual ~SwXMLItemSetStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

#ifdef XML_CORE_API
    sal_uInt16 GetPoolId() const { return nPoolId; }
    sal_Bool IsAutoUpdate() const { return bAutoUpdate; }
#endif

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return pItemSet; }
    const SfxItemSet *GetItemSet() const { return pItemSet; }

#ifdef XML_CORE_API
    SwFmt *GetFmt() { return pFmt; }
    void SetFmt( SwFmt *p ) { pFmt = p; }

    SwTxtFmtColl* GetColl();

    sal_Bool HasConditions() const { return 0 != pConditions &&
                                 pConditions->Count() > 0; }
    const OUString& GetListStyle() const { return sListStyleName; }

    void ConnectParent();
    void ConnectFollow();
    void ConnectConditions();
    void ConnectListStyle();
    void ConnectAutoListStyle();
#endif
};

#ifdef XML_CORE_API
void SwXMLItemSetStyleContext_Impl::SetAttribute( sal_uInt16 nPrefixKey,
                                           const OUString& rLocalName,
                                           const OUString& rValue )
{
    // TODO: use a map here
    if( XML_NAMESPACE_STYLE == nPrefixKey )
    {
        if( rLocalName.compareToAscii( sXML_family ) == 0 )
        {
            SfxStyleFamily eFamily = SFX_STYLE_FAMILY_ALL;
            sal_uInt16 nSubFamily = 0U;
            if( rValue.compareToAscii( sXML_paragraph ) == 0 )
            {
                eFamily = SFX_STYLE_FAMILY_PARA;
            }
            else if( rValue.compareToAscii( sXML_text ) == 0 )
            {
                eFamily = SFX_STYLE_FAMILY_CHAR;
            }
            else if( 0 == rValue.compareToAscii( sXML_table, 5L ) )
            {
                eFamily = SFX_STYLE_FAMILY_FRAME;
                if( 5L == rValue.getLength() )
                    nSubFamily = SW_STYLE_SUBFAMILY_TABLE;
                else if( rValue.compareToAscii( sXML_table_column ) == 0 )
                    nSubFamily = SW_STYLE_SUBFAMILY_TABLE_COL;
                else if( rValue.compareToAscii( sXML_table_row ) == 0 )
                    nSubFamily = SW_STYLE_SUBFAMILY_TABLE_LINE;
                else if( rValue.compareToAscii( sXML_table_cell ) == 0 )
                    nSubFamily = SW_STYLE_SUBFAMILY_TABLE_BOX;
                else
                    eFamily = SFX_STYLE_FAMILY_ALL;
            }

            if( SFX_STYLE_FAMILY_ALL != eFamily )
            {
                SetFamily( eFamily );
                SetSubFamily( nSubFamily );
            }
        }
        else if( rLocalName.compareToAscii( sXML_pool_id ) == 0 )
        {
            sal_Int32 nTmp = rValue.toInt32();
            nPoolId =
                (nTmp < 0L) ? 0U : ( (nTmp > USHRT_MAX) ? USHRT_MAX
                                                        : (sal_uInt16)nTmp );
        }
        else if( rLocalName.compareToAscii( sXML_auto_update ) == 0 )
        {
            if( rValue.compareToAscii( sXML_true ) == 0 )
                bAutoUpdate = sal_True;
        }
        else if( rLocalName.compareToAscii( sXML_list_style_name ) == 0 )
        {
            sListStyleName = rValue;
        }
        else
        {
            SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
        }
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}
#endif

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateItemSetContext(
        sal_uInt16 nPrefix, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    ASSERT( !pItemSet,
            "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: item set exists" );

    SvXMLImportContext *pContext = 0;
    SfxItemPool& rItemPool = GetSwImport().GetDoc().GetAttrPool();

#ifdef XML_CORE_API
    switch( GetFamily() )
    {
    case SFX_STYLE_FAMILY_PARA:
        pItemSet = new SfxItemSet( rItemPool, aTxtFmtCollSetRange );
        pContext = GetSwImport().CreateParaItemImportContext(
                                nPrefix, rLName, xAttrList, *pItemSet );
        break;

    case SFX_STYLE_FAMILY_CHAR:
        pItemSet = new SfxItemSet( rItemPool, aCharFmtSetRange );
        pContext = GetSwImport().CreateParaItemImportContext(
                                nPrefix, rLName, xAttrList, *pItemSet );
        break;

    case SFX_STYLE_FAMILY_FRAME:
        {
            switch( GetSubFamily() )
            {
            case SW_STYLE_SUBFAMILY_TABLE:
                pItemSet = new SfxItemSet( rItemPool, aTableSetRange );
                break;
            case SW_STYLE_SUBFAMILY_TABLE_COL:
                pItemSet = new SfxItemSet( rItemPool, RES_FRM_SIZE,
                                           RES_FRM_SIZE, 0 );
                break;
            case SW_STYLE_SUBFAMILY_TABLE_LINE:
                pItemSet = new SfxItemSet( rItemPool, aTableLineSetRange );
                break;
            case SW_STYLE_SUBFAMILY_TABLE_BOX:
                pItemSet = new SfxItemSet( rItemPool, aTableBoxSetRange );
                break;
            }
            if( pItemSet )
                pContext = GetSwImport().CreateTableItemImportContext(
                                    nPrefix, rLName, xAttrList, GetSubFamily(),
                                    *pItemSet );
            ASSERT( pItemSet,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: frames are unsopprted");
        }
        break;

    default:
        ASSERT( !this,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: unknown family" );
        break;
    }
#else
    switch( GetFamily() )
    {
    case XML_STYLE_FAMILY_TABLE_TABLE:
        pItemSet = new SfxItemSet( rItemPool, aTableSetRange );
        break;
    case XML_STYLE_FAMILY_TABLE_COLUMN:
        pItemSet = new SfxItemSet( rItemPool, RES_FRM_SIZE, RES_FRM_SIZE, 0 );
        break;
    case XML_STYLE_FAMILY_TABLE_ROW:
        pItemSet = new SfxItemSet( rItemPool, aTableLineSetRange );
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
        pItemSet = new SfxItemSet( rItemPool, aTableBoxSetRange );
        break;
    default:
        ASSERT( !this,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: unknown family" );
        break;
    }
    if( pItemSet )
        pContext = GetSwImport().CreateTableItemImportContext(
                                nPrefix, rLName, xAttrList, GetFamily(),
                                *pItemSet );
#endif

    if( !pContext )
    {
        delete pItemSet;
        pItemSet = 0;
    }

    return pContext;
}

TYPEINIT1( SwXMLItemSetStyleContext_Impl, SvXMLStyleContext );

SwXMLItemSetStyleContext_Impl::SwXMLItemSetStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList
#ifndef XML_CORE_API
        ,sal_uInt16 nFamily
#endif
        ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
#ifdef XML_CORE_API
    nPoolId( USHRT_MAX ),
    bAutoUpdate( sal_False ),
    pFmt( 0 ),
    pConditions( 0 ),
#endif
    pItemSet( 0 )
{
#ifndef XML_CORE_API
    SetFamily( nFamily );
#endif
}

SwXMLItemSetStyleContext_Impl::~SwXMLItemSetStyleContext_Impl()
{
    delete pItemSet;
#ifdef XML_CORE_API
    if( pConditions )
    {
        while( pConditions->Count() )
        {
            SwXMLConditionContext_Impl *pCond = pConditions->GetObject(0);
            pConditions->Remove( 0UL );
            pCond->ReleaseRef();
        }
        delete pConditions;
    }
#endif
}

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( rLocalName.compareToAscii( sXML_properties ) == 0 )
        {
            pContext = CreateItemSetContext( nPrefix, rLocalName, xAttrList );
        }
#ifdef XML_CORE_API
        else if( rLocalName.compareToAscii( sXML_map ) == 0 )
        {
            SwXMLConditionContext_Impl *pCond =
                new SwXMLConditionContext_Impl( GetSwImport(), nPrefix,
                                                   rLocalName, xAttrList );
            if( pCond->IsValid() )
            {
                if( !pConditions )
                   pConditions = new SwXMLConditions_Impl;
                pConditions->Insert( pCond, pConditions->Count() );
                pCond->AddRef();
            }
            pContext = pCond;
        }
#endif
    }

    if( !pContext )
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

#ifdef XML_CORE_API
SwTxtFmtColl* SwXMLItemSetStyleContext_Impl::GetColl()
{
    return (GetFamily() & SFX_STYLE_FAMILY_PARA) != 0
            ? (SwTxtFmtColl*) pFmt
            : NULL;
}

void SwXMLItemSetStyleContext_Impl::ConnectParent()
{
    if( !pFmt )
        return;

    SwDoc *pDoc = pFmt->GetDoc();
    String aName( GetSwImport().GetI18NMap().Get( GetFamily(), GetParent() ) );

//  String aName( SwXStyleFamilies::GetUIName( GetParent(),
//                                             (SfxStyleFamily)GetFamily() ) );
    SwFmt* pParent;
    switch( GetFamily() )
    {
    case SFX_STYLE_FAMILY_PARA:
        if( aName.Len() )
            pParent = pDoc->FindTxtFmtCollByName( aName );
        else
            pParent = (*pDoc->GetTxtFmtColls())[ 0 ];
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( aName.Len() )
            pParent = pDoc->FindFrmFmtByName( aName );
        else
            pParent = (*pDoc->GetFrmFmts())[ 0 ];
        break;

    case SFX_STYLE_FAMILY_CHAR:
        if( aName.Len() )
            pParent = pDoc->FindCharFmtByName( aName );
        else
            pParent = (*pDoc->GetCharFmts())[ 0 ];
        break;
    }

    if( pParent )
        pFmt->SetDerivedFrom( pParent );
}

void SwXMLItemSetStyleContext_Impl::ConnectFollow()
{
    if( !pFmt || GetFamily() != SFX_STYLE_FAMILY_PARA )
        return;

    SwDoc *pDoc = pFmt->GetDoc();
    String aName( GetSwImport().GetI18NMap().Get( GetFamily(), GetFollow() ) );
//  String aName( SwXStyleFamilies::GetUIName( GetFollow(), (SfxStyleFamily)GetFamily() ) );

    SwTxtFmtColl* pFollow;
    if( aName.Len() )
        pFollow = pDoc->FindTxtFmtCollByName( aName );
    else
        pFollow = GetColl();
    if( pFollow )
        GetColl()->SetNextTxtFmtColl( *pFollow );
}

void SwXMLItemSetStyleContext_Impl::ConnectConditions()
{
    if( !pFmt || GetFamily() != SFX_STYLE_FAMILY_PARA || !pConditions ||
         RES_CONDTXTFMTCOLL != pFmt->Which() )
        return;

    SwDoc *pDoc = pFmt->GetDoc();
    SvI18NMap& rI18NMap = ((SwXMLImport&)GetImport()).GetI18NMap();

    sal_uInt16 nCount = pConditions->Count();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        const SwXMLConditionContext_Impl *pCond = (*pConditions)[i];
        OUString sName( rI18NMap.Get( SFX_STYLE_FAMILY_PARA,
                                         pCond->GetApplyStyle() ) );
//      OUString sName( SwXStyleFamilies::GetUIName(
//                  pCond->GetApplyStyle(), SFX_STYLE_FAMILY_PARA ) );
        SwTxtFmtColl* pCondColl = pDoc->FindTxtFmtCollByName( sName );
        ASSERT( pCondColl,
            "SwXMLItemSetStyleContext_Impl::ConnectConditions: cond coll missing" );
        if( pCondColl )
        {
            SwCollCondition aCond( pCondColl, pCond->GetCondition(),
                                              pCond->GetSubCondition() );
            ((SwConditionTxtFmtColl*)pFmt)->InsertCondition( aCond );
        }
    }
}

void SwXMLItemSetStyleContext_Impl::ConnectListStyle()
{
    if( !pFmt || GetFamily() != SFX_STYLE_FAMILY_PARA ||
        !GetListStyle().getLength() )
        return;

    SwDoc *pDoc = pFmt->GetDoc();
    String aName( GetSwImport().GetI18NMap().Get( SFX_STYLE_FAMILY_PSEUDO,
                                                  GetListStyle() ) );
//  String aName( SwXStyleFamilies::GetUIName(
//                  GetListStyle(), SFX_STYLE_FAMILY_PSEUDO ) );
    // Styles must not reference automatic num rules
    SwNumRule *pNumRule = pDoc->FindNumRulePtr( aName );
    if( pNumRule && !pNumRule->IsAutoRule() )
        GetColl()->SetAttr( SwNumRuleItem(aName) );
}

void SwXMLItemSetStyleContext_Impl::ConnectAutoListStyle()
{
    if( GetFamily() != SFX_STYLE_FAMILY_PARA || !GetListStyle().getLength() )
        return;

    SwDoc& rDoc = GetSwImport().GetDoc();
    String aName( GetSwImport().GetI18NMap().Get( SFX_STYLE_FAMILY_PSEUDO,
                                                  GetListStyle() ) );
//  String aName( SwXStyleFamilies::GetUIName(
//                  GetListStyle(), SFX_STYLE_FAMILY_PSEUDO ) );
    if( rDoc.FindNumRulePtr( aName ) )
    {
        if( !pItemSet )
        {
            SfxItemPool& rItemPool = rDoc.GetAttrPool();
            pItemSet = new SfxItemSet( rItemPool, aTxtFmtCollSetRange );
        }
        pItemSet->Put( SwNumRuleItem(aName) );
    }
}
#endif

// ---------------------------------------------------------------------

#ifdef XML_CORE_API
enum SwXMLStyleStylesElemTokens
{
    SW_XML_TOK_STYLE_STYLE,
    SW_XML_TOK_TEXT_LIST_STYLE,
    SW_XML_TOK_TEXT_OUTLINE,
    SW_XML_TOK_STYLE_STYLES_ELEM_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aStyleStylesElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_style,         SW_XML_TOK_STYLE_STYLE },
    { XML_NAMESPACE_TEXT,   sXML_list_style,    SW_XML_TOK_TEXT_LIST_STYLE},
    { XML_NAMESPACE_TEXT,   sXML_outline_style, SW_XML_TOK_TEXT_OUTLINE },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& SwXMLImport::GetStyleStylesElemTokenMap()
{
    if( !pStyleStylesElemTokenMap )
        pStyleStylesElemTokenMap =
            new SvXMLTokenMap( aStyleStylesElemTokenMap );

    return *pStyleStylesElemTokenMap;
}
#endif

// ---------------------------------------------------------------------
//
class SwXMLStylesContext_Impl : public SvXMLStylesContext
{
    SwXMLItemSetStyleContext_Impl *GetSwStyle( sal_uInt16 i ) const;
#ifdef XML_CORE_API
    SwXMLListStyleContext *GetSwListStyle( sal_uInt16 i ) const;

    SwFmt *FindFmtByName( const String& rName,
                          sal_uInt16 eFamily ) const;
    SwXMLItemSetStyleContext_Impl *FindByPoolId( sal_uInt16 nPoolId ) const;
#endif

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }

protected:

#ifdef XML_CORE_API
    // Create a style context.
    virtual SvXMLStyleContext *CreateStyleChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
#else
    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList );
    // HACK
    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;

    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;
    // HACK
#endif

public:

    TYPEINFO();

    SwXMLStylesContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLStylesContext_Impl();

    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;
#ifdef XML_CORE_API
    void CopyStylesToDoc();
    void CopyAutoStylesToDoc();
#endif
};

TYPEINIT1( SwXMLStylesContext_Impl, SvXMLStylesContext );

inline SwXMLItemSetStyleContext_Impl *SwXMLStylesContext_Impl::GetSwStyle(
        sal_uInt16 i ) const
{
    return PTR_CAST( SwXMLItemSetStyleContext_Impl, GetStyle( i ) );
}

#ifdef XML_CORE_API
inline SwXMLListStyleContext *SwXMLStylesContext_Impl::GetSwListStyle(
        sal_uInt16 i ) const
{
    return PTR_CAST( SwXMLListStyleContext, GetStyle( i ) );
}

sal_Bool lcl_xmlfmt_isValidPoolId( sal_uInt16 nPoolId, sal_uInt16 eFamily )
{
    sal_Bool bValid = sal_False;

    // check if pool id is valid
    if( eFamily & SFX_STYLE_FAMILY_CHAR )
    {
        bValid = (RES_POOLCHR_NORMAL_BEGIN <= nPoolId &&
                                        nPoolId < RES_POOLCHR_NORMAL_END) ||
                 (RES_POOLCHR_HTML_BEGIN <= nPoolId &&
                                        nPoolId < RES_POOLCHR_HTML_END);
    }
    else if( eFamily & SFX_STYLE_FAMILY_FRAME )
    {
        bValid = RES_POOLFRM_BEGIN <= nPoolId && nPoolId < RES_POOLFRM_END;
    }
    else if( eFamily & SFX_STYLE_FAMILY_PARA )
    {
        bValid = (RES_POOLCOLL_TEXT_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_TEXT_END) ||
                 (RES_POOLCOLL_LISTS_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_LISTS_END) ||
                 (RES_POOLCOLL_EXTRA_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_EXTRA_END) ||
                 (RES_POOLCOLL_REGISTER_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_REGISTER_END) ||
                 (RES_POOLCOLL_DOC_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_DOC_END) ||
                 (RES_POOLCOLL_HTML_BEGIN <= nPoolId &&
                                    nPoolId < RES_POOLCOLL_HTML_END);
    }

    return bValid;
}

SwFmt *SwXMLStylesContext_Impl::FindFmtByName( const String& rName,
                                               sal_uInt16 eFamily ) const
{
    const SwDoc& rDoc = GetSwImport().GetDoc();
    SwFmt *pFmt = 0;
    switch( eFamily )
    {
    case SFX_STYLE_FAMILY_PARA:
        pFmt = rDoc.FindTxtFmtCollByName( rName );
        break;
    case SFX_STYLE_FAMILY_FRAME:
        pFmt = rDoc.FindFrmFmtByName( rName );
        break;
    case SFX_STYLE_FAMILY_CHAR:
        pFmt = rDoc.FindCharFmtByName( rName );
        break;
    }

    return pFmt;
}

SwXMLItemSetStyleContext_Impl *SwXMLStylesContext_Impl::FindByPoolId(
        sal_uInt16 nPoolId ) const
{
    SwXMLItemSetStyleContext_Impl *pStyle = 0;
    sal_uInt16 nCount = GetStyleCount();
    for( sal_uInt16 i=0; i < nCount && !pStyle; i++ )
    {
        SwXMLItemSetStyleContext_Impl *pTmp = GetSwStyle( i );
        if( pTmp && pTmp->GetPoolId() == nPoolId )
            pStyle = pTmp;
    }

    return pStyle;
}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetStyleStylesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case SW_XML_TOK_STYLE_STYLE:
        pStyle = new SwXMLItemSetStyleContext_Impl( GetSwImport(), nPrefix, rLocalName,
                                                xAttrList);
        break;
    case SW_XML_TOK_TEXT_LIST_STYLE:
        pStyle = new SwXMLListStyleContext( GetSwImport(), nPrefix,
                                            rLocalName, xAttrList );
        break;
    case SW_XML_TOK_TEXT_OUTLINE:
        pStyle = new SwXMLListStyleContext( GetSwImport(), nPrefix,
                                            rLocalName, xAttrList, sal_True );
        break;
    default:
        pStyle = SvXMLStylesContext::CreateStyleChildContext( nPrefix,
                                                              rLocalName,
                                                              xAttrList );
        break;
    }

    return pStyle;
}
#else
SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = 0;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        pStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nPrefix,
                            rLocalName, xAttrList, nFamily, *this );
        break;
    case XML_STYLE_FAMILY_TABLE_TABLE:
    case XML_STYLE_FAMILY_TABLE_COLUMN:
    case XML_STYLE_FAMILY_TABLE_ROW:
    case XML_STYLE_FAMILY_TABLE_CELL:
        pStyle = new SwXMLItemSetStyleContext_Impl( GetSwImport(), nPrefix,
                            rLocalName, xAttrList, nFamily  );
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        // HACK until CL is back again
        pStyle = new XMLTextStyleContext( GetImport(), nPrefix,
                            rLocalName, xAttrList, *this );
        break;
    default:
        pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily,
                                                                   nPrefix,
                                                              rLocalName,
                                                              xAttrList );
        break;
    }

    return pStyle;
}
#endif

SwXMLStylesContext_Impl::SwXMLStylesContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

SwXMLStylesContext_Impl::~SwXMLStylesContext_Impl()
{
}

#ifndef XML_CORE_API
sal_Bool SwXMLStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    const SwXMLImport& rSwImport = GetSwImport();
    sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    sal_Bool bIns = sal_True;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_PARA) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_CHAR) != 0;
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_FRAME) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_LIST:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_PSEUDO) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_OUTLINE:
        bIns = !(rSwImport.IsInsertMode() || rSwImport.IsStylesOnlyMode());
        break;
    default:
        bIns = SvXMLStylesContext::InsertStyleFamily( nFamily );
        break;
    }

    return bIns;
}

UniReference < SvXMLImportPropertyMapper > SwXMLStylesContext_Impl::GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        xMapper = ((SvXMLImport *)&GetImport())->GetTextImport()
                     ->GetFrameImportPropertySetMapper();
    else
        xMapper = SvXMLStylesContext::GetImportPropertyMapper( nFamily );

    return xMapper;
}

Reference < XNameContainer > SwXMLStylesContext_Impl::GetStylesContainer(
                                                sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        xStyles = ((SvXMLImport *)&GetImport())->GetTextImport()->GetFrameStyles();
    else
        xStyles = SvXMLStylesContext::GetStylesContainer( nFamily );

    return xStyles;
}

OUString SwXMLStylesContext_Impl::GetServiceName( sal_uInt16 nFamily ) const
{
    String sServiceName;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        sServiceName = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.FrameStyle") );
    else
        sServiceName = SvXMLStylesContext::GetServiceName( nFamily );

    return sServiceName;
}
#endif

#ifdef XML_CORE_API
void SwXMLStylesContext_Impl::CopyStylesToDoc()
{
    SwXMLImport& rSwImport = GetSwImport();
    SwDoc& rDoc = rSwImport.GetDoc();
    SvI18NMap& rI18NMap = rSwImport.GetI18NMap();
    sal_Bool bInsertMode = rSwImport.IsInsertMode();
    sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    // The following families are of interest for pass 1
    sal_uInt16 nFamilies = nStyleFamilyMask &
                       ( SFX_STYLE_FAMILY_FRAME|SFX_STYLE_FAMILY_PARA|
                         SFX_STYLE_FAMILY_CHAR );

    // pass 1: create text, paragraph and frame styles
    sal_uInt16 nCount = GetStyleCount();
    for( sal_uInt16 i=0; i<nCount; i++ )
    {
        SwXMLItemSetStyleContext_Impl *pStyle = GetSwStyle( i );
        if( !pStyle )
        {
            continue;
        }

        sal_uInt16 eFamily = pStyle->GetFamily();
        if( !(nFamilies & eFamily) )
        {
            pStyle->SetValid( sal_False );
            continue;
        }

        const OUString& rName = pStyle->GetName();
        if( 0 == rName.getLength() )
        {
            pStyle->SetValid( sal_False );
            continue;
        }
        OUString sName =
            SwXStyleFamilies::GetUIName( rName, (SfxStyleFamily)eFamily );

//      sal_uInt16 nPoolId = pStyle->GetPoolId();

        sal_Bool bNewFmt = sal_False;
        SwFmt *pFmt = FindFmtByName( sName, eFamily );
        if( !pFmt )
        {
            // it is a new format
            SwGetPoolIdFromName eNameType;
            if( eFamily & SFX_STYLE_FAMILY_CHAR )
                eNameType = GET_POOLID_CHRFMT;
            else if( eFamily & SFX_STYLE_FAMILY_FRAME )
                eNameType = GET_POOLID_FRMFMT;
            else
                eNameType = GET_POOLID_TXTCOLL;

            sal_uInt16 nPoolId = rDoc.GetPoolId( sName, eNameType );
            bNewFmt = sal_True;
//          sal_Bool bIsUserDefined = (nPoolId & USER_FMT) != 0;
            sal_Bool bIsUserDefined = nPoolId == USHRT_MAX;
#if 0
            if( !bIsUserDefined )
            {
                bIsUserDefined = !lcl_xmlfmt_isValidPoolId( nPoolId, eFamily );
                if( bIsUserDefined )
                {
                    // If the pool id is invalid create a user style
                    nPoolId |= (USHRT_MAX &
                                ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID));
                }
            }
            else
            {
                // The style is not a pool style, but maybe there is
                // a pool style with this name now?
                SwGetPoolIdFromName eNameType;
                if( eFamily & SFX_STYLE_FAMILY_CHAR )
                    eNameType = GET_POOLID_CHRFMT;
                else if( eFamily & SFX_STYLE_FAMILY_FRAME )
                    eNameType = GET_POOLID_FRMFMT;
                else
                    eNameType = GET_POOLID_TXTCOLL;
                sal_uInt16 nId = rDoc.GetPoolId( rName, eNameType );
                if( nId != USHRT_MAX )
                {
                    if( FindByPoolId( nId ) )
                    {
                        // There is a style with the new pool id, too.
                        // This means, that the user style will be ignored.
                        continue;
                    }
                    else
                    {
                        bIsUserDefined = sal_False;
                        nPoolId = nId;
                    }
                }
            }
#endif

            if( bIsUserDefined )
            {
                if( eFamily & SFX_STYLE_FAMILY_CHAR )
                {
                    pFmt = rDoc.MakeCharFmt( rName, NULL );
                    pFmt->SetAuto( sal_False );
                }
                else if( eFamily & SFX_STYLE_FAMILY_FRAME )
                {
                    pFmt = rDoc.MakeFrmFmt( rName, NULL );
                    pFmt->SetAuto( sal_False );
                }
                else if( pStyle->HasConditions() )
                {
                    SwTxtFmtColl* pDer = (*rDoc.GetTxtFmtColls())[ 0 ];
                    pFmt = rDoc.MakeCondTxtFmtColl( rName, pDer );
                }
                else
                {
                    SwTxtFmtColl* pDer = (*rDoc.GetTxtFmtColls())[ 0 ];
                    pFmt = rDoc.MakeTxtFmtColl( rName, pDer );
                }

//              pFmt->SetPoolFmtId( nPoolId );
//              if( pStyle->GetHelpFile().getLength() )
//                  pFmt->SetPoolHlpFileId
//                      ( (sal_Int8) rDoc.SetDocPattern(
//                                          pStyle->GetHelpFile() ) );
//              pFmt->SetPoolHelpId( (sal_uInt16)pStyle->GetHelpId() );
            }
            else
            {
                if( eFamily & SFX_STYLE_FAMILY_CHAR )
                {
                    sal_uInt16 nStyleCnt = rDoc.GetCharFmts()->Count();
                    pFmt = rDoc.GetCharFmtFromPool( nPoolId );
                    bNewFmt = nStyleCnt != rDoc.GetCharFmts()->Count();
                }
                else if( eFamily & SFX_STYLE_FAMILY_FRAME )
                {
                    sal_uInt16 nStyleCnt = rDoc.GetFrmFmts()->Count();
                    pFmt = rDoc.GetFrmFmtFromPool( nPoolId );
                    bNewFmt = nStyleCnt != rDoc.GetFrmFmts()->Count();
                }
                else
                {
                    sal_uInt16 nStyleCnt = rDoc.GetTxtFmtColls()->Count();
                    pFmt = rDoc.GetTxtCollFromPool( nPoolId );
                    bNewFmt = nStyleCnt != rDoc.GetTxtFmtColls()->Count();
                }

                // If the name of the pool style has been changed, add
                // a i18n map entry.
                if( String(rName) != pFmt->GetName() )
                    rI18NMap.Add( eFamily, pStyle->GetName(), pFmt->GetName() );
            }
        }
#if 0
        else if( (nPoolId & USER_FMT) != 0 )
        {
            // If a pool style has been renamed it may have the same
            // name a a user style. If this is the case, the user style
            // will be ignored.
            sal_uInt16 nId = pFmt->GetPoolFmtId();
            SwXMLItemSetStyleContext_Impl *pTmp;
            if( nId != nPoolId && (nId & USER_FMT) == 0 &&
                (pTmp = FindByPoolId( nPoolId )) != 0 &&
                pTmp->GetFmt() == pFmt )
            {
                pStyle->SetValid( sal_False );
                continue;
            }
        }
#endif

        pStyle->SetFmt( pFmt );

        if( !bInsertMode || bNewFmt )
        {
            if( pStyle->GetItemSet() )
            {
                pFmt->ResetAllAttr(); // delete default attributes

                ((SwAttrSet&) pFmt->GetAttrSet()).Put( *pStyle->GetItemSet() );
            }
            SwTxtFmtColl* pColl = pStyle->GetColl();
            if( pColl )
            {
                ((SwAttrSet&) pColl->GetAttrSet()).SetModifyAtAttr( pColl );
            }
            pFmt->SetAutoUpdateFmt( pStyle->IsAutoUpdate() );
        }
        else
        {
            pStyle->SetValid( sal_False );
        }
    }

    // pass 2: connect parent/next styles and create list styles
    for( i=0; i<nCount; i++ )
    {
        SwXMLItemSetStyleContext_Impl *pStyle = GetSwStyle( i );
        if( !pStyle )
        {
            if( (nStyleFamilyMask & SFX_STYLE_FAMILY_PSEUDO) != 0 )
            {
                SwXMLListStyleContext *pListStyle = GetSwListStyle( i );
                if( pListStyle )
                    pListStyle->InsertNumRule( sal_False );
            }
            continue;
        }
        else if( !pStyle->IsValid() )
            continue;

        sal_uInt16 eFamily = pStyle->GetFamily();
        if( (nFamilies & eFamily) != 0 )
        {
            pStyle->ConnectParent();
            pStyle->ConnectFollow();

            // The format has been changed
            SwFmtChg aHint( pStyle->GetFmt() );
            pStyle->GetFmt()->Modify( &aHint, &aHint );
        }
    }

    // pass 3: connect conditions and list styles
    for( i=0; i<nCount; i++ )
    {
        SwXMLItemSetStyleContext_Impl *pStyle = GetSwStyle( i );
        if( pStyle && pStyle->IsValid() &&
            (nFamilies & pStyle->GetFamily()) != 0 )
        {
            pStyle->ConnectListStyle();
            if( pStyle->HasConditions() )
                pStyle->ConnectConditions();
        }
    }
}

void SwXMLStylesContext_Impl::CopyAutoStylesToDoc()
{
    // pass 1: create list styles
    sal_uInt16 nCount = GetStyleCount();
    if( (GetSwImport().GetStyleFamilyMask() & SFX_STYLE_FAMILY_PSEUDO) != 0 )
    {
        for( sal_uInt16 i=0; i<nCount; i++ )
        {
            SwXMLListStyleContext *pListStyle = GetSwListStyle( i );
            if( pListStyle && !pListStyle->IsOutline() )
                pListStyle->InsertNumRule( sal_True );
        }
    }

    // pass 3: connect list styles
    if( (GetSwImport().GetStyleFamilyMask() & SFX_STYLE_FAMILY_PARA) != 0 )
    {
        for( sal_uInt16 i=0; i<nCount; i++ )
        {
            SwXMLItemSetStyleContext_Impl *pStyle = GetSwStyle( i );
            if( pStyle && SFX_STYLE_FAMILY_PARA == pStyle->GetFamily() )
            {
                pStyle->ConnectAutoListStyle();
            }
        }
    }
}
#endif

// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateStylesContext(
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bAuto )
{
    SvXMLImportContext *pContext =
        new SwXMLStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                       xAttrList );
    if( bAuto )
        xAutoStyles = pContext;
    else
        xStyles = pContext;

    return pContext;
}

void SwXMLImport::InsertStyles()
{
    sal_Bool bStylesValid = xStyles.Is();
    bAutoStylesValid  = xAutoStyles.Is();

    if( bStylesValid )
#ifdef XML_CORE_API
        ((SwXMLStylesContext_Impl *)&xStyles)->CopyStylesToDoc();
#else
        ((SwXMLStylesContext_Impl *)&xStyles)->CopyStylesToDoc(
                                                !IsInsertMode() );
#endif
    xStyles = 0;

    if( bAutoStylesValid )
#ifdef XML_CORE_API
        ((SwXMLStylesContext_Impl *)&xAutoStyles)->CopyAutoStylesToDoc();
#else
        GetTextImport()->SetAutoStyles( (SwXMLStylesContext_Impl *)&xAutoStyles );
#endif
    else
        xAutoStyles = 0;
}

sal_Bool SwXMLImport::FindAutomaticStyle(
#ifdef XML_CORE_API
        SfxStyleFamily eFamily,
        sal_uInt16 nSubFamily,
#else
        sal_uInt16 nFamily,
#endif
        const OUString& rName,
        const SfxItemSet **ppItemSet,
        OUString *pParent ) const
{
    const SwXMLItemSetStyleContext_Impl *pStyle = 0;
    if( bAutoStylesValid && xAutoStyles.Is() )
    {
#ifdef XML_CORE_API
        pStyle = PTR_CAST( SwXMLItemSetStyleContext_Impl,
              ((SwXMLStylesContext_Impl *)&xAutoStyles)->
                    FindStyleChildContext( eFamily, nSubFamily, rName,
                                           sal_True ) );
#else
        pStyle = PTR_CAST( SwXMLItemSetStyleContext_Impl,
              ((SwXMLStylesContext_Impl *)&xAutoStyles)->
                    FindStyleChildContext( nFamily, rName,
                                           sal_True ) );
#endif
        if( pStyle )
        {
            if( ppItemSet )
                (*ppItemSet) = pStyle->GetItemSet();
            if( pParent )
                *pParent = pStyle->GetParent();
        }
    }

    return pStyle != 0;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlfmt.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.42  2000/09/18 16:05:05  willem.vandorp
      OpenOffice header added.

      Revision 1.41  2000/09/18 11:58:02  mib
      text frames/graphics import and export continued

      Revision 1.40  2000/08/31 14:26:32  mib
      conditional styles

      Revision 1.39  2000/08/24 11:16:41  mib
      text import continued

      Revision 1.38  2000/08/10 10:22:15  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.37  2000/07/27 08:06:34  mib
      text import continued

      Revision 1.36  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.35  2000/07/07 13:58:36  mib
      text styles using StarOffice API

      Revision 1.34  2000/06/26 08:31:15  mib
      removed SfxStyleFamily

      Revision 1.33  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.32  2000/05/03 12:08:05  mib
      unicode

      Revision 1.31  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.30  2000/03/06 10:46:10  mib
      #72585#: toInt32

      Revision 1.29  2000/02/18 09:20:12  mib
      #70271#: initialization of pContext was missing

      Revision 1.28  2000/02/17 14:40:30  mib
      #70271#: XML table import

      Revision 1.26  2000/01/27 08:59:02  mib
      #70271#: outline numbering

      Revision 1.25  2000/01/20 10:03:15  mib
      #70271#: Lists reworked

      Revision 1.24  2000/01/06 15:08:27  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.23  1999/12/13 08:28:25  mib
      #70271#: Support for element items added

      Revision 1.22  1999/12/06 11:41:33  mib
      #70258#: Container item for unkown attributes

      Revision 1.21  1999/11/26 11:13:57  mib
      loading of styles only and insert mode

      Revision 1.20  1999/11/22 15:52:34  os
      headers added

      Revision 1.19  1999/11/17 20:08:49  nn
      document language

      Revision 1.18  1999/11/12 14:50:28  mib
      meta import and export reactivated

      Revision 1.17  1999/11/12 11:43:03  mib
      using item mapper, part iii

      Revision 1.16  1999/11/10 15:08:09  mib
      Import now uses XMLItemMapper

      Revision 1.15  1999/11/09 15:40:08  mib
      Using XMLItemMapper for export

      Revision 1.14  1999/11/05 19:44:11  nn
      handle office:meta

      Revision 1.13  1999/11/01 11:38:50  mib
      List style import

      Revision 1.12  1999/10/25 10:41:48  mib
      Using new OUString ASCII methods

      Revision 1.11  1999/10/22 09:49:16  mib
      List style export

      Revision 1.10  1999/10/15 12:37:05  mib
      integrated SvXMLStyle into SvXMLStyleContext

      Revision 1.9  1999/10/08 11:47:45  mib
      moved some file to SVTOOLS/SVX

      Revision 1.8  1999/10/01 13:02:51  mib
      no comparisons between OUString and char* any longer

      Revision 1.7  1999/09/28 10:47:58  mib
      char fmts again

      Revision 1.6  1999/09/28 08:31:15  mib
      char fmts, hints

      Revision 1.5  1999/09/23 11:53:58  mib
      i18n, token maps and hard paragraph attributes

      Revision 1.4  1999/09/22 11:56:57  mib
      string -> wstring

      Revision 1.3  1999/08/19 12:57:42  MIB
      attribute import added


      Rev 1.2   19 Aug 1999 14:57:42   MIB
   attribute import added

      Rev 1.1   18 Aug 1999 17:03:36   MIB
   Style import

      Rev 1.0   13 Aug 1999 16:18:10   MIB
   Initial revision.


*************************************************************************/

