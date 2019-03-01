/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <tools/diagnose_ex.h>

#include <xmloff/table/XMLTableImport.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include "table.hxx"

#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <memory>

using namespace ::xmloff::token;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

namespace {

struct ColumnInfo
{
    OUString msStyleName;
    OUString msDefaultCellStyleName;
};

}

class XMLProxyContext : public SvXMLImportContext
{
public:
    XMLProxyContext( SvXMLImport& rImport, const SvXMLImportContextRef& xParent, sal_uInt16 nPrfx, const OUString& rLName );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList ) override;

private:
    SvXMLImportContextRef mxParent;
};

struct MergeInfo
{
    sal_Int32 const mnStartColumn;
    sal_Int32 const mnStartRow;
    sal_Int32 const mnEndColumn;
    sal_Int32 const mnEndRow;

    MergeInfo( sal_Int32 nStartColumn, sal_Int32 nStartRow, sal_Int32 nColumnSpan, sal_Int32 nRowSpan )
        : mnStartColumn( nStartColumn ), mnStartRow( nStartRow ), mnEndColumn( nStartColumn + nColumnSpan - 1 ), mnEndRow( nStartRow + nRowSpan - 1 ) {};
};

typedef std::vector< std::shared_ptr< MergeInfo > > MergeInfoVector;

class XMLTableImportContext : public SvXMLImportContext
{
public:
    XMLTableImportContext( const rtl::Reference< XMLTableImport >& xThis, sal_uInt16 nPrfx, const OUString& rLName, Reference< XColumnRowRange > const & xColumnRowRange );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList ) override;

    virtual void StartElement( const Reference< XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;

    void InitColumns();

    SvXMLImportContextRef ImportColumn( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );
    SvXMLImportContext * ImportRow( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );
    SvXMLImportContextRef ImportCell( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );

    OUString GetDefaultCellStyleName() const;

    css::uno::Reference< css::table::XTable > mxTable;
    Reference< XTableColumns > mxColumns;
    Reference< XTableRows > mxRows;

    std::vector< std::shared_ptr< ColumnInfo > > maColumnInfos;
    sal_Int32 mnCurrentRow;
    sal_Int32 mnCurrentColumn;

    // default cell style name for the current row
    OUString msDefaultCellStyleName;

    MergeInfoVector maMergeInfos;
};

class XMLCellImportContext : public SvXMLImportContext
{
public:
    XMLCellImportContext( SvXMLImport& rImport,
                          const Reference< XMergeableCell >& xCell,
                          const OUString& sDefaultCellStyleName,
                          sal_uInt16 nPrfx, const OUString& rLName,
                          const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;

    sal_Int32 getColumnSpan() const { return mnColSpan; }
    sal_Int32 getRowSpan() const { return mnRowSpan; }
    sal_Int32 getRepeated() const { return mnRepeated; }

    Reference< XMergeableCell > mxCell;
    Reference< XTextCursor >    mxCursor;
    Reference< XTextCursor >    mxOldCursor;
    bool                        mbListContextPushed;

    sal_Int32 mnColSpan, mnRowSpan, mnRepeated;
};

class XMLTableTemplateContext : public SvXMLStyleContext
{
public:
    XMLTableTemplateContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList ) override;

    virtual void StartElement( const Reference< XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;

    virtual void CreateAndInsert( bool bOverwrite ) override;
private:
    XMLTableTemplate maTableTemplate;
    OUString msTemplateStyleName;
};

// class XMLProxyContext

XMLProxyContext::XMLProxyContext( SvXMLImport& rImport, const SvXMLImportContextRef& xParent, sal_uInt16 nPrfx, const OUString& rLName )
: SvXMLImportContext( rImport, nPrfx, rLName )
, mxParent( xParent )
{
}

SvXMLImportContextRef XMLProxyContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( mxParent.is() )
        return mxParent->CreateChildContext( nPrefix, rLocalName, xAttrList );
    else
        return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

// class XMLTableImport

XMLTableImport::XMLTableImport( SvXMLImport& rImport, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef )
: mrImport( rImport )
{
    bool bWriter = false;
    // check if called by Writer
    Reference<XMultiServiceFactory> xFac(rImport.GetModel(), UNO_QUERY);
    if (xFac.is()) try
    {
        Sequence<OUString> sSNS = xFac->getAvailableServiceNames();
        const sal_Int32 nLength = sSNS.getLength();
        const OUString* pSNS = sSNS.getConstArray();
        for (sal_Int32 i=0; i < nLength; ++i, ++pSNS)
        {
            if (*pSNS == "com.sun.star.style.TableStyle")
            {
                bWriter = true;
                break;
            }
        }
    }
    catch(const Exception&)
    {
        SAL_WARN("xmloff.table", "Error while checking available service names");
    }

    if (bWriter)
    {
        mxCellImportPropertySetMapper = XMLTextImportHelper::CreateTableCellExtPropMapper(rImport);
    }
    else
    {
        mxCellImportPropertySetMapper = new SvXMLImportPropertyMapper( xCellPropertySetMapper.get(), rImport );
        mxCellImportPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(rImport));
        mxCellImportPropertySetMapper->ChainImportMapper(new SvXMLImportPropertyMapper(new XMLPropertySetMapper(getCellPropertiesMap(), xFactoryRef.get(), true), rImport));
    }

    rtl::Reference < XMLPropertySetMapper > xRowMapper( new XMLPropertySetMapper( getRowPropertiesMap(), xFactoryRef.get(), false ) );
    mxRowImportPropertySetMapper = new SvXMLImportPropertyMapper( xRowMapper, rImport );

    rtl::Reference < XMLPropertySetMapper > xColMapper( new XMLPropertySetMapper( getColumnPropertiesMap(), xFactoryRef.get(), false ) );
    mxColumnImportPropertySetMapper = new SvXMLImportPropertyMapper( xColMapper, rImport );
}

XMLTableImport::~XMLTableImport()
{
}

SvXMLImportContext* XMLTableImport::CreateTableContext( sal_uInt16 nPrfx, const OUString& rLName, Reference< XColumnRowRange > const & xColumnRowRange )
{
    rtl::Reference< XMLTableImport > xThis( this );
    return new XMLTableImportContext( xThis, nPrfx, rLName, xColumnRowRange );
}

SvXMLStyleContext* XMLTableImport::CreateTableTemplateContext( sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList )
{
    return new XMLTableTemplateContext( mrImport, nPrfx, rLName, xAttrList );
}

void XMLTableImport::addTableTemplate( const OUString& rsStyleName, XMLTableTemplate& xTableTemplate )
{
    std::shared_ptr< XMLTableTemplate > xPtr( new XMLTableTemplate );
    xPtr->swap( xTableTemplate );
    maTableTemplates[rsStyleName] = xPtr;
}

void XMLTableImport::insertTabletemplate(const OUString& rsStyleName, bool bOverwrite)
{
    XMLTableTemplateMap::iterator it = maTableTemplates.find(rsStyleName);
    if (it == maTableTemplates.end())
        return;

    try
    {
        Reference<XStyleFamiliesSupplier> xFamiliesSupp(mrImport.GetModel(), UNO_QUERY_THROW);
        Reference<XNameAccess> xFamilies(xFamiliesSupp->getStyleFamilies());
        const OUString sFamilyName("TableStyles");
        const OUString sCellFamilyName("CellStyles");

        Reference<XNameContainer> xTableFamily(xFamilies->getByName(sFamilyName), UNO_QUERY_THROW);
        Reference<XIndexAccess> xCellFamily(xFamilies->getByName(sCellFamilyName), UNO_QUERY_THROW);

        const OUString sTemplateName(it->first);
        Reference<XMultiServiceFactory> xFactory(mrImport.GetModel(), UNO_QUERY_THROW);
        Reference<XNameReplace> xTemplate(xFactory->createInstance("com.sun.star.style.TableStyle"), UNO_QUERY_THROW);

        std::shared_ptr<XMLTableTemplate> xT(it->second);

        for (const auto& rStyle : *xT) try
        {
            const OUString sPropName(rStyle.first);
            const OUString sStyleName(rStyle.second);
            // Internally unassigned cell styles are stored by display name.
            // However table-template elements reference cell styles by its encoded name.
            // This loop is looking for cell style by their encoded names.
            sal_Int32 nCount = xCellFamily->getCount();
            for (sal_Int32 i=0; i < nCount; ++i)
            {
                Any xCellStyle = xCellFamily->getByIndex(i);
                OUString sEncodedStyleName = mrImport.GetMM100UnitConverter().encodeStyleName(
                    xCellStyle.get<Reference<XStyle>>()->getName());
                if (sEncodedStyleName == sStyleName)
                {
                    xTemplate->replaceByName(sPropName, xCellStyle);
                    break;
                }
            }
        }
        catch (Exception const &)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("xmloff.table", "XMLTableImport::insertTabletemplate(), exception caught!" << exceptionToString(ex));
        }

        if (xTemplate.is())
        {
            if (xTableFamily->hasByName(sTemplateName) && bOverwrite)
               xTableFamily->replaceByName(sTemplateName, Any(xTemplate));
            else
               xTableFamily->insertByName(sTemplateName, Any(xTemplate));
        }
    }
    catch (Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("xmloff.table", "XMLTableImport::insertTabletemplate(), exception caught!" << exceptionToString(ex));
    }
}

void XMLTableImport::finishStyles()
{
    if( !maTableTemplates.empty() ) try
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( mrImport.GetModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        const OUString sFamilyName( "table" );
        const OUString sCellFamilyName( "cell" );

        Reference< XNameContainer > xTableFamily( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );
        Reference< XNameAccess > xCellFamily( xFamilies->getByName( sCellFamilyName ), UNO_QUERY_THROW );

        Reference< XSingleServiceFactory > xFactory( xTableFamily, UNO_QUERY_THROW );

        for( const auto& rTemplate : maTableTemplates ) try
        {
            const OUString sTemplateName( rTemplate.first );
            Reference< XNameReplace > xTemplate( xFactory->createInstance(), UNO_QUERY_THROW );

            std::shared_ptr< XMLTableTemplate > xT( rTemplate.second );

            for( const auto& rStyle : *xT ) try
            {
                const OUString sPropName( rStyle.first );
                const OUString sStyleName( rStyle.second );
                xTemplate->replaceByName( sPropName, xCellFamily->getByName( sStyleName ) );
            }
            catch( Exception& )
            {
                OSL_FAIL("xmloff::XMLTableImport::finishStyles(), exception caught!");
            }

            if( xTemplate.is() )
            {
                if( xTableFamily->hasByName( sTemplateName ) )
                    xTableFamily->replaceByName( sTemplateName, Any( xTemplate ) );
                else
                    xTableFamily->insertByName( sTemplateName, Any( xTemplate ) );
            }

        }
        catch( Exception& )
        {
            OSL_FAIL("xmloff::XMLTableImport::finishStyles(), exception caught!");
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableImport::finishStyles(), exception caught!");
    }
}

// class XMLTableImport

XMLTableImportContext::XMLTableImportContext( const rtl::Reference< XMLTableImport >& xImporter, sal_uInt16 nPrfx, const OUString& rLName,  Reference< XColumnRowRange > const & xColumnRowRange )
: SvXMLImportContext( xImporter->mrImport, nPrfx, rLName )
, mxTable( xColumnRowRange, UNO_QUERY )
, mxColumns( xColumnRowRange->getColumns() )
, mxRows( xColumnRowRange->getRows() )
, mnCurrentRow( -1 )
, mnCurrentColumn( -1 )
{
}

SvXMLImportContextRef XMLTableImportContext::ImportColumn( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( mxColumns.is() && (mnCurrentRow == -1) ) try
    {
        std::shared_ptr< ColumnInfo > xInfo ( new ColumnInfo );

        sal_Int32 nRepeated = 1;

        // read attributes for the table-column
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            const OUString sAttrName( xAttrList->getNameByIndex( i ) );
            const OUString sValue( xAttrList->getValueByIndex( i ) );
            OUString aLocalName;

            sal_uInt16 nPrefix2 = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            if( XML_NAMESPACE_TABLE == nPrefix2 )
            {
                if( IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
                {
                    nRepeated = sValue.toInt32();
                }
                else if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                {
                    xInfo->msStyleName = sValue;
                }
                else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                {
                    xInfo->msDefaultCellStyleName = sValue;
                }
            }
            else if ( (XML_NAMESPACE_XML == nPrefix2) &&
                 IsXMLToken(aLocalName, XML_ID)   )
            {
//FIXME: TODO
            }
        }

        if( nRepeated <= 1 )
        {
            maColumnInfos.push_back( xInfo );
        }
        else
        {
            maColumnInfos.insert( maColumnInfos.end(), nRepeated, xInfo );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableImportContext::ImportTableColumn(), exception caught!");
    }

    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList);
}

void XMLTableImportContext::InitColumns()
{
    if( mxColumns.is() ) try
    {
        const sal_Int32 nCount1 = mxColumns->getCount();
        const sal_Int32 nCount2 = sal::static_int_cast< sal_Int32 >( maColumnInfos.size() );
        if( nCount1 < nCount2 )
            mxColumns->insertByIndex( nCount1, nCount2 - nCount1 );

        SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();

        for( sal_Int32 nCol = 0; nCol < nCount2; nCol++ )
        {
            std::shared_ptr< ColumnInfo > xInfo( maColumnInfos[nCol] );

            if( pAutoStyles && !xInfo->msStyleName.isEmpty() )
            {
                const XMLPropStyleContext* pStyle =
                    dynamic_cast< const XMLPropStyleContext* >(
                        pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN, xInfo->msStyleName) );

                if( pStyle )
                {
                    Reference< XPropertySet > xColProps( mxColumns->getByIndex(nCol), UNO_QUERY_THROW );
                    const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xColProps );
                }
            }

        }
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableImportContext::ImportTableColumn(), exception caught!");
    }
}

SvXMLImportContext * XMLTableImportContext::ImportRow( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( mxRows.is() )
    {
        mnCurrentRow++;
        if( mnCurrentRow == 0 )
            InitColumns();      // first init columns

        mnCurrentColumn = -1;

        const sal_Int32 nRowCount = mxRows->getCount();
        if( ( nRowCount - 1) < mnCurrentRow )
        {
            const sal_Int32 nCount = mnCurrentRow - nRowCount + 1;
            mxRows->insertByIndex( nRowCount, nCount );
        }

        Reference< XPropertySet > xRowSet( mxRows->getByIndex(mnCurrentRow), UNO_QUERY );

        OUString sStyleName;

        // read attributes for the table-row
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            const OUString sAttrName( xAttrList->getNameByIndex( i ) );
            const OUString sValue( xAttrList->getValueByIndex( i ) );
            OUString aLocalName;

            sal_uInt16 nPrefix2 = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            if( nPrefix2 == XML_NAMESPACE_TABLE )
            {
                if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                {
                    sStyleName = sValue;
                }
                else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                {
                    msDefaultCellStyleName = sValue;
                }
            }
            else if ( (XML_NAMESPACE_XML == nPrefix2) &&
                 IsXMLToken(aLocalName, XML_ID)   )
            {
                (void) sValue;
//FIXME: TODO
            }
        }

        if( !sStyleName.isEmpty() )
        {
            SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();
            if( pAutoStyles )
            {
                const XMLPropStyleContext* pStyle =
                    dynamic_cast< const XMLPropStyleContext* >(
                        pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_ROW, sStyleName) );

                if( pStyle )
                {
                    const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xRowSet );
                }
            }
        }
    }

    SvXMLImportContextRef xThis( this );
    return new XMLProxyContext( GetImport(), xThis, nPrefix, rLocalName );
}

SvXMLImportContextRef XMLTableImportContext::ImportCell( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    mnCurrentColumn++;
    if( mxColumns.is() ) try
    {
        if( mxColumns->getCount() <= mnCurrentColumn )
            mxColumns->insertByIndex( mxColumns->getCount(), mnCurrentColumn - mxColumns->getCount() + 1 );

        Reference< XMergeableCell > xCell( mxTable->getCellByPosition( mnCurrentColumn, mnCurrentRow ), UNO_QUERY_THROW );
        XMLCellImportContext* pCellContext = new XMLCellImportContext( GetImport(), xCell, GetDefaultCellStyleName(), nPrefix, rLocalName, xAttrList );

        const sal_Int32 nColumnSpan = pCellContext->getColumnSpan();
        const sal_Int32 nRowSpan = pCellContext->getRowSpan();
        if( (nColumnSpan > 1) || (nRowSpan > 1) )
            maMergeInfos.push_back( std::make_shared< MergeInfo >( mnCurrentColumn, mnCurrentRow, nColumnSpan, nRowSpan ) );

        const sal_Int32 nRepeated = pCellContext->getRepeated();
        if( nRepeated > 1 )
        {
            OSL_FAIL("xmloff::XMLTableImportContext::ImportCell(), import of repeated Cells not implemented (TODO)");
            mnCurrentColumn  += nRepeated - 1;
        }

        return pCellContext;
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableImportContext::ImportCell(), exception caught!");
    }

    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList);
}

SvXMLImportContextRef XMLTableImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( nPrefix == XML_NAMESPACE_TABLE )
    {
        if( IsXMLToken( rLocalName, XML_TABLE_COLUMN ) )
            return ImportColumn( nPrefix, rLocalName, xAttrList );
        else if( IsXMLToken( rLocalName, XML_TABLE_ROW ) )
            return ImportRow( nPrefix, rLocalName, xAttrList );
        else if( IsXMLToken( rLocalName, XML_TABLE_CELL ) || IsXMLToken( rLocalName, XML_COVERED_TABLE_CELL ) )
            return ImportCell( nPrefix, rLocalName, xAttrList );
        else if( IsXMLToken( rLocalName, XML_TABLE_COLUMNS ) || IsXMLToken( rLocalName, XML_TABLE_ROWS ) )
        {
            SvXMLImportContextRef xThis( this );
            return new XMLProxyContext( GetImport(), xThis, nPrefix, rLocalName );
        }
    }

    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList);
}

void XMLTableImportContext::StartElement( const Reference< XAttributeList >& /*xAttrList*/ )
{
}

void XMLTableImportContext::EndElement()
{
    for( const std::shared_ptr< MergeInfo >& xInfo : maMergeInfos )
    {
        if( xInfo.get() ) try
        {
            Reference< XCellRange > xRange( mxTable->getCellRangeByPosition( xInfo->mnStartColumn, xInfo->mnStartRow, xInfo->mnEndColumn, xInfo->mnEndRow ) );
            Reference< XMergeableCellRange > xCursor( mxTable->createCursorByRange( xRange ), UNO_QUERY_THROW );
            xCursor->merge();
        }
        catch( Exception& )
        {
            OSL_FAIL("XMLTableImportContext::EndElement(), exception caught while merging cells!");
        }
    }
}

OUString XMLTableImportContext::GetDefaultCellStyleName() const
{
    OUString sStyleName( msDefaultCellStyleName );

    // if there is still no style name, try default style name from column
    if( (sStyleName.isEmpty()) && (mnCurrentColumn < sal::static_int_cast<sal_Int32>(maColumnInfos.size())) )
        sStyleName = maColumnInfos[mnCurrentColumn]->msDefaultCellStyleName;

    return sStyleName;
}

// XMLCellImportContext

XMLCellImportContext::XMLCellImportContext( SvXMLImport& rImport, const Reference< XMergeableCell >& xCell, const OUString& sDefaultCellStyleName, sal_uInt16 nPrfx, const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList )
: SvXMLImportContext( rImport, nPrfx, rLName )
, mxCell( xCell )
, mbListContextPushed( false )
, mnColSpan( 1 )
, mnRowSpan( 1 )
, mnRepeated( 1 )
{
    OUString sStyleName;

    // read attributes for the table-cell
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString sAttrName( xAttrList->getNameByIndex( i ) );
        const OUString sValue( xAttrList->getValueByIndex( i ) );
        OUString aLocalName;

        sal_uInt16 nPrefix2 = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if( XML_NAMESPACE_TABLE == nPrefix2 )
        {
            if( IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
            {
                mnRepeated = sValue.toInt32();
            }
            else if( IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_SPANNED ) )
            {
                mnColSpan = sValue.toInt32();
            }
            else if( IsXMLToken( aLocalName, XML_NUMBER_ROWS_SPANNED ) )
            {
                mnRowSpan = sValue.toInt32();
            }
            else if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                sStyleName = sValue;
            }
        }
        else if ( (XML_NAMESPACE_XML == nPrefix2) &&
             IsXMLToken(aLocalName, XML_ID)   )
        {
//FIXME: TODO
        }
//FIXME: RDFa (table:table-cell)
    }

    // if there is no style name at the cell, try default style name from row
    if( sStyleName.isEmpty() )
        sStyleName = sDefaultCellStyleName;

    if( !sStyleName.isEmpty() )
    {
        SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();
        if( pAutoStyles )
        {
            const XMLPropStyleContext* pStyle =
                dynamic_cast< const XMLPropStyleContext* >(
                    pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL, sStyleName) );

            if( pStyle )
            {
                Reference< XPropertySet > xCellSet( mxCell, UNO_QUERY );
                if( xCellSet.is() )
                    const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xCellSet );
            }
        }
    }
}

SvXMLImportContextRef XMLCellImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    // create text cursor on demand
    if( !mxCursor.is() )
    {
        Reference< XText > xText( mxCell, UNO_QUERY );
        if( xText.is() )
        {
            rtl::Reference < XMLTextImportHelper > xTxtImport( GetImport().GetTextImport() );
            mxOldCursor = xTxtImport->GetCursor();
            mxCursor = xText->createTextCursor();
            if( mxCursor.is() )
                xTxtImport->SetCursor( mxCursor );

            // remember old list item and block (#91964#) and reset them
            // for the text frame
            xTxtImport->PushListContext();
            mbListContextPushed = true;
        }
    }

    SvXMLImportContext * pContext = nullptr;

    // if we have a text cursor, lets  try to import some text
    if( mxCursor.is() )
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    if( pContext )
        return pContext;
    else
        return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList);
}

void XMLCellImportContext::EndElement()
{
    if(mxCursor.is())
    {
        // delete addition newline
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( "" );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item (if necessary) #91964#
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }
}

// class XMLTableTemplateContext

XMLTableTemplateContext::XMLTableTemplateContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList )
: SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_TABLE_TEMPLATE_ID, false )
{
}

void XMLTableTemplateContext::StartElement( const Reference< XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName;
        sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( i ), &sAttrName );
        if( (nAttrPrefix == XML_NAMESPACE_TEXT && IsXMLToken( sAttrName, XML_STYLE_NAME ))
            // Writer specific: according to oasis odf 1.2 prefix should be "table" and element name should be "name"
            || (nAttrPrefix == XML_NAMESPACE_TABLE && IsXMLToken( sAttrName, XML_NAME )))
        {
            msTemplateStyleName = xAttrList->getValueByIndex( i );
            break;
        }
    }
}

void XMLTableTemplateContext::EndElement()
{
    rtl::Reference< XMLTableImport > xTableImport( GetImport().GetShapeImport()->GetShapeTableImport() );
    if( xTableImport.is() )
        xTableImport->addTableTemplate( msTemplateStyleName, maTableTemplate );
}

void XMLTableTemplateContext::CreateAndInsert(bool bOverwrite)
{
    rtl::Reference<XMLTableImport> xTableImport(GetImport().GetShapeImport()->GetShapeTableImport());
    if(xTableImport.is())
       xTableImport->insertTabletemplate(msTemplateStyleName, bOverwrite);
}

SvXMLImportContextRef XMLTableTemplateContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( nPrefix == XML_NAMESPACE_TABLE )
    {
        const TableStyleElement* pElements = getTableStyleMap();
        while( (pElements->meElement != XML_TOKEN_END) && !IsXMLToken( rLocalName, pElements->meElement ) )
            pElements++;

        if( pElements->meElement != XML_TOKEN_END )
        {
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for(sal_Int16 i=0; i < nAttrCount; i++)
            {
                OUString sAttrName;
                sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( i ), &sAttrName );
                if( (nAttrPrefix == XML_NAMESPACE_TEXT || nAttrPrefix == XML_NAMESPACE_TABLE) &&
                    IsXMLToken( sAttrName, XML_STYLE_NAME ) )
                {
                    maTableTemplate[pElements->msStyleName] = xAttrList->getValueByIndex( i );
                    break;
                }
            }
        }
    } else if (nPrefix == XML_NAMESPACE_LO_EXT) // Writer specific cell styles
    {
        const TableStyleElement* pElements = getWriterSpecificTableStyleMap();
        while ((pElements->meElement != XML_TOKEN_END) && !IsXMLToken(rLocalName, pElements->meElement ))
            pElements++;

        if (pElements->meElement != XML_TOKEN_END)
        {
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for (sal_Int16 i=0; i < nAttrCount; i++)
            {
                OUString sAttrName;
                sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(xAttrList->getNameByIndex( i ), &sAttrName);
                if( (nAttrPrefix == XML_NAMESPACE_TEXT || nAttrPrefix == XML_NAMESPACE_TABLE) &&
                    IsXMLToken( sAttrName, XML_STYLE_NAME ) )
                {
                    maTableTemplate[pElements->msStyleName] = xAttrList->getValueByIndex(i);
                    break;
                }
            }
        }
    }

    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
