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
#include "vbaworksheets.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>

#include <ooo/vba/excel/XApplication.hpp>
#include <tabvwsh.hxx>

#include "excelvbahelper.hxx"
#include "vbaglobals.hxx"
#include "vbaworksheet.hxx"
#include "vbaworkbook.hxx"
#include <unonames.hxx>
#include <markdata.hxx>

#include <vector>
#include <prevwsh.hxx>
#include <preview.hxx>
using namespace ::ooo::vba;
using namespace ::com::sun::star;

// a map ( or hashmap ) won't do as we need also to preserve the order
// (as added ) of the items
typedef std::vector< uno::Reference< sheet::XSpreadsheet > >  SheetMap;

// #FIXME #TODO the implementation of the Sheets collections sucks,
// e.g. there is no support for tracking sheets added/removed from the collection

class WorkSheetsEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    SheetMap mSheetMap;
    SheetMap::iterator mIt;
public:
    explicit WorkSheetsEnumeration( const SheetMap& sMap ) : mSheetMap( sMap ), mIt( mSheetMap.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( mIt != mSheetMap.end() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< sheet::XSpreadsheet > xSheet( *mIt++ );
        return uno::makeAny( xSheet ) ;
    }
};

class SheetCollectionHelper : public ::cppu::WeakImplHelper< container::XNameAccess,
                                                              container::XIndexAccess,
                                                              container::XEnumerationAccess >
{
    SheetMap mSheetMap;
    SheetMap::iterator cachePos;
public:
    explicit SheetCollectionHelper( const SheetMap& sMap ) : mSheetMap( sMap ), cachePos(mSheetMap.begin()) {}
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  cppu::UnoType<sheet::XSpreadsheet>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return ( !mSheetMap.empty() ); }
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return uno::makeAny( *cachePos );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< OUString > sNames( mSheetMap.size() );
        OUString* pString = sNames.getArray();

        for ( const auto& rItem : mSheetMap )
        {
            uno::Reference< container::XNamed > xName( rItem, uno::UNO_QUERY_THROW );
            *pString = xName->getName();
            ++pString;
        }
        return sNames;
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        cachePos = mSheetMap.begin();
        SheetMap::iterator it_end = mSheetMap.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            uno::Reference< container::XNamed > xName( *cachePos, uno::UNO_QUERY_THROW );
            if ( aName == xName->getName() )
                break;
        }
        return ( cachePos != it_end );
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override { return mSheetMap.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        return uno::makeAny( mSheetMap[ Index ] );

    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new WorkSheetsEnumeration( mSheetMap );
    }
};

class SheetsEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
public:
    /// @throws uno::RuntimeException
    SheetsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel( xModel ) {}

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Reference< sheet::XSpreadsheet > xSheet( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        uno::Reference< XHelperInterface > xIf = excel::getUnoSheetModuleObj( xSheet );
        uno::Any aRet;
        if ( !xIf.is() )
        {
            // if the Sheet is in a document created by the api unfortunately ( at the
            // moment, it actually won't have the special Document modules
            uno::Reference< excel::XWorksheet > xNewSheet( new ScVbaWorksheet( m_xParent, m_xContext, xSheet, m_xModel ) );
            aRet <<= xNewSheet;
        }
        else
            aRet <<= xIf;
        return aRet;
    }

};

ScVbaWorksheets::ScVbaWorksheets( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xSheets, const uno::Reference< frame::XModel >& xModel ): ScVbaWorksheets_BASE( xParent, xContext,  xSheets ), mxModel( xModel ), m_xSheets( uno::Reference< sheet::XSpreadsheets >( xSheets, uno::UNO_QUERY ) )
{
}

ScVbaWorksheets::ScVbaWorksheets( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< container::XEnumerationAccess >& xEnumAccess, const uno::Reference< frame::XModel >& xModel  ):  ScVbaWorksheets_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xEnumAccess, uno::UNO_QUERY ) ), mxModel(xModel)
{
}

// XEnumerationAccess
uno::Type
ScVbaWorksheets::getElementType()
{
    return cppu::UnoType<excel::XWorksheet>::get();
}

uno::Reference< container::XEnumeration >
ScVbaWorksheets::createEnumeration()
{
    if ( !m_xSheets.is() )
    {
        uno::Reference< container::XEnumerationAccess > xAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
        return xAccess->createEnumeration();
    }
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xSheets, uno::UNO_QUERY_THROW );
    return new SheetsEnumeration( this, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
ScVbaWorksheets::createCollectionObject( const uno::Any& aSource )
{
    uno::Reference< sheet::XSpreadsheet > xSheet( aSource, uno::UNO_QUERY );
    uno::Reference< XHelperInterface > xIf = excel::getUnoSheetModuleObj( xSheet );
    uno::Any aRet;
    if ( !xIf.is() )
    {
        // if the Sheet is in a document created by the api unfortunately ( at the
        // moment, it actually won't have the special Document modules
        uno::Reference< excel::XWorksheet > xNewSheet( new ScVbaWorksheet( getParent(), mxContext, xSheet, mxModel ) );
        aRet <<= xNewSheet;
    }
    else
        aRet <<= xIf;
    return aRet;
}

// XWorksheets
uno::Any
ScVbaWorksheets::Add( const uno::Any& Before, const uno::Any& After,
                     const uno::Any& Count, const uno::Any& Type )
{
    if ( isSelectedSheets() )
        return uno::Any(); // or should we throw?

    OUString aStringSheet;
    bool bBefore(true);
    SCTAB nSheetIndex = 0;
    SCTAB nNewSheets = 1, nType = 0;
    Count >>= nNewSheets;
    Type >>= nType;
    SCTAB nCount = 0;

    uno::Reference< excel::XWorksheet > xBeforeAfterSheet;

    if ( Before.hasValue() )
    {
        if ( Before >>= xBeforeAfterSheet )
            aStringSheet = xBeforeAfterSheet->getName();
        else
            Before >>= aStringSheet;
    }

    if (aStringSheet.isEmpty() && After.hasValue() )
    {
        if ( After >>= xBeforeAfterSheet )
            aStringSheet = xBeforeAfterSheet->getName();
        else
            After >>= aStringSheet;
        bBefore = false;
    }
    if (aStringSheet.isEmpty())
    {
        uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
        aStringSheet = xApplication->getActiveWorkbook()->getActiveSheet()->getName();
        bBefore = true;
    }
    nCount = static_cast< SCTAB >( m_xIndexAccess->getCount() );
    for (SCTAB i=0; i < nCount; i++)
    {
            uno::Reference< sheet::XSpreadsheet > xSheet(m_xIndexAccess->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            if (xNamed->getName() == aStringSheet)
            {
                nSheetIndex = i;
                break;
            }
    }

    if(!bBefore)
        nSheetIndex++;

    SCTAB nSheetName = nCount + 1;
    OUString aStringBase( "Sheet" );
    uno::Any result;
    for (SCTAB i=0; i < nNewSheets; i++, nSheetName++)
    {
        OUString aStringName = aStringBase + OUString::number(nSheetName);
        while (m_xNameAccess->hasByName(aStringName))
        {
            nSheetName++;
            aStringName = aStringBase + OUString::number(nSheetName);
        }
        m_xSheets->insertNewByName(aStringName, nSheetIndex + i);
        result = getItemByStringIndex( aStringName );
    }
    uno::Reference< excel::XWorksheet > xNewSheet( result, uno::UNO_QUERY );
    if ( xNewSheet.is() )
        xNewSheet->Activate();
    return  result;
}

void
ScVbaWorksheets::Delete()
{
    // #TODO #INVESTIGATE
    // mmm this method could be trouble if the underlying
    // uno objects ( the m_xIndexAccess etc ) aren't aware of the
    // contents that are deleted
    sal_Int32 nElems = getCount();
    for ( sal_Int32 nItem = 1; nItem <= nElems; ++nItem )
    {
        uno::Reference< excel::XWorksheet > xSheet( Item( uno::makeAny( nItem ), uno::Any() ), uno::UNO_QUERY_THROW );
        xSheet->Delete();
    }
}

bool
ScVbaWorksheets::isSelectedSheets()
{
    return !m_xSheets.is();
}

void SAL_CALL
ScVbaWorksheets::PrintOut( const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& ActivePrinter, const uno::Any& PrintToFile, const uno::Any& Collate, const uno::Any& PrToFileName )
{
    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    bool bSelection = false;
    From >>= nFrom;
    To >>= nTo;

    if ( !( nFrom || nTo ) )
        if ( isSelectedSheets() )
            bSelection = true;

    PrintOutHelper( excel::getBestViewShell( mxModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, bSelection );
}

uno::Any SAL_CALL
ScVbaWorksheets::getVisible()
{
    bool bVisible = true;
    uno::Reference< container::XEnumeration > xEnum( createEnumeration(), uno::UNO_QUERY_THROW );
    while ( xEnum->hasMoreElements() )
    {
        uno::Reference< excel::XWorksheet > xSheet( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        if ( xSheet->getVisible() == 0 )
        {
                bVisible = false;
                break;
        }
    }
    return uno::makeAny( bVisible );
}

void SAL_CALL
ScVbaWorksheets::setVisible( const uno::Any& _visible )
{
    bool bState = false;
    if ( !(_visible >>= bState) )
        throw uno::RuntimeException("Visible property doesn't support non boolean #FIXME" );

    uno::Reference< container::XEnumeration > xEnum( createEnumeration(), uno::UNO_QUERY_THROW );
    while ( xEnum->hasMoreElements() )
    {
        uno::Reference< excel::XWorksheet > xSheet( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        xSheet->setVisible( bState ? 1 : 0 );
    }

}

void SAL_CALL
ScVbaWorksheets::Select( const uno::Any& Replace )
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( mxModel );
    if ( !pViewShell )
        throw uno::RuntimeException("Cannot obtain view shell" );

    ScMarkData& rMarkData = pViewShell->GetViewData().GetMarkData();
    bool bReplace = true;
    Replace >>= bReplace;
    // Replace is defaulted to True, meaning this current collection
    // becomes the Selection, if it were false then the current selection would
    // be extended
    bool bSelectSingle = bReplace;
    sal_Int32 nElems = getCount();
    for ( sal_Int32 nItem = 1; nItem <= nElems; ++nItem )
    {
        uno::Reference< excel::XWorksheet > xSheet( Item( uno::makeAny( nItem ), uno::Any() ), uno::UNO_QUERY_THROW );
        ScVbaWorksheet* pSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSheet );
        if ( bSelectSingle )
        {
            rMarkData.SelectOneTable( static_cast< SCTAB >( pSheet->getSheetID() ) );
            bSelectSingle = false;
        }
        else
            rMarkData.SelectTable( static_cast< SCTAB >( pSheet->getSheetID() ), true );
    }

}

void SAL_CALL
ScVbaWorksheets::Copy ( const uno::Any& Before, const uno::Any& After)
{
    uno::Reference<excel::XWorksheet> xSheet;
    sal_Int32 nElems = getCount();
    bool bAfter = After.hasValue();
    std::vector< uno::Reference< excel::XWorksheet > > Sheets;
    sal_Int32 nItem = 0;

    for ( nItem = 1; nItem <= nElems; ++nItem)
    {
        uno::Reference<excel::XWorksheet> xWorksheet(Item( uno::makeAny( nItem ), uno::Any() ), uno::UNO_QUERY_THROW );
        Sheets.push_back(xWorksheet);
    }
    bool bNewDoc = (!(Before >>= xSheet) && !(After >>=xSheet)&& !(Before.hasValue()) && !(After.hasValue()));

    uno::Reference< excel::XWorksheet > xSrcSheet;
    if ( bNewDoc )
    {
        bAfter = true;
        xSrcSheet = Sheets.at(0);
        ScVbaWorksheet* pSrcSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSrcSheet );
        xSheet = pSrcSheet->createSheetCopyInNewDoc(xSrcSheet->getName());
        nItem = 1;
    }
    else
    {
        nItem=0;
    }

    for (; nItem < nElems; ++nItem )
    {
        xSrcSheet = Sheets[nItem];
        ScVbaWorksheet* pSrcSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSrcSheet );
        if ( bAfter )
            xSheet = pSrcSheet->createSheetCopy(xSheet, bAfter);
        else
            pSrcSheet->createSheetCopy(xSheet, bAfter);
    }
}

//ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaWorksheets::Item(const uno::Any& Index, const uno::Any& Index2)
{
    if ( Index.getValueTypeClass() == uno::TypeClass_SEQUENCE )
    {
        const uno::Reference< script::XTypeConverter >& xConverter = getTypeConverter(mxContext);
        uno::Any aConverted;
        aConverted = xConverter->convertTo( Index, cppu::UnoType<uno::Sequence< uno::Any >>::get() );
        SheetMap aSheets;
        uno::Sequence< uno::Any > sIndices;
        aConverted >>= sIndices;
        sal_Int32 nElems = sIndices.getLength();
        for( sal_Int32 index = 0; index < nElems; ++index )
        {
            uno::Reference< excel::XWorksheet > xWorkSheet( ScVbaWorksheets_BASE::Item( sIndices[ index ], Index2 ), uno::UNO_QUERY_THROW );
            ScVbaWorksheet* pWorkSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xWorkSheet );
            uno::Reference< sheet::XSpreadsheet > xSheet( pWorkSheet->getSheet() , uno::UNO_QUERY_THROW );
            uno::Reference< container::XNamed > xName( xSheet, uno::UNO_QUERY_THROW );
            aSheets.push_back( xSheet );
        }
        uno::Reference< container::XIndexAccess > xIndexAccess = new SheetCollectionHelper( aSheets );
        uno::Reference< XCollection > xSelectedSheets(  new ScVbaWorksheets( getParent(), mxContext, xIndexAccess, mxModel ) );
        return uno::makeAny( xSelectedSheets );
    }
    return  ScVbaWorksheets_BASE::Item( Index, Index2 );
}

OUString
ScVbaWorksheets::getServiceImplName()
{
    return OUString("ScVbaWorksheets");
}

css::uno::Sequence<OUString>
ScVbaWorksheets::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        "ooo.vba.excel.Worksheets"
    };
    return sNames;
}

bool ScVbaWorksheets::nameExists( const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, const OUString & name, SCTAB& nTab )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( "nameExists() xSpreadDoc is null", uno::Reference< uno::XInterface  >(), 1 );
    uno::Reference <container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if ( xIndex.is() )
    {
        SCTAB  nCount = static_cast< SCTAB >( xIndex->getCount() );
        for (SCTAB i=0; i < nCount; i++)
        {
            uno::Reference< container::XNamed > xNamed( xIndex->getByIndex(i), uno::UNO_QUERY_THROW );
            if (xNamed->getName() == name)
            {
                nTab = i;
                return true;
            }
        }
    }
    return false;
}

void ScVbaWorksheets::PrintPreview( const css::uno::Any& /*EnableChanges*/ )
{
    // need test, print preview current active sheet
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( mxModel );
    SfxViewFrame* pViewFrame = nullptr;
    if ( pViewShell )
        pViewFrame = pViewShell->GetViewFrame();
    if ( pViewFrame )
    {
        if ( !pViewFrame->GetFrame().IsInPlace() )
        {
            dispatchExecute( pViewShell, SID_VIEWSHELL1 );
            SfxViewShell*  pShell = SfxViewShell::Get( pViewFrame->GetFrame().GetFrameInterface()->getController() );

            if (  dynamic_cast<const ScPreviewShell*>( pShell) !=  nullptr )
            {
                ScPreviewShell* pPrvShell = static_cast<  ScPreviewShell* >( pShell );
                ScPreview* pPrvView = pPrvShell->GetPreview();
                ScMarkData aMarkData;
                sal_Int32 nElems = getCount();
                for ( sal_Int32 nItem = 1; nItem <= nElems; ++nItem )
                {
                    uno::Reference< excel::XWorksheet > xSheet( Item( uno::makeAny( nItem ), uno::Any() ), uno::UNO_QUERY_THROW );
                    ScVbaWorksheet* pSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSheet );
                    if ( pSheet )
                        aMarkData.SelectTable(static_cast< SCTAB >( pSheet->getSheetID() ), true );
                }
                // save old selection, setting the selectedtabs in the preview
                // can affect the current selection when preview has been
                // closed
                ScMarkData::MarkedTabsType aOldTabs = pPrvView->GetSelectedTabs();
                pPrvView->SetSelectedTabs( aMarkData );
                // force update
                pPrvView->DataChanged(false);
                // set sensible first page
                long nPage = pPrvView->GetFirstPage( 1 );
                pPrvView->SetPageNo( nPage );
                WaitUntilPreviewIsClosed( pViewFrame );
                // restore old tab selection
                pViewShell = excel::getBestViewShell( mxModel );
                pViewShell->GetViewData().GetMarkData().SetSelectedTabs(aOldTabs);
            }
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
