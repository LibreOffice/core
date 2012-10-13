/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "vbaworksheets.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/itemwrapper.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <ooo/vba/excel/XApplication.hpp>
#include <tools/string.hxx>
#include "tabvwsh.hxx"

#include "vbaglobals.hxx"
#include "vbaworksheet.hxx"
#include "vbaworkbook.hxx"
#include "unonames.hxx"
#include "markdata.hxx"

#include <vector>
#include "prevwsh.hxx"
#include "preview.hxx"
using namespace ::ooo::vba;
using namespace ::com::sun::star;


typedef ::cppu::WeakImplHelper1< container::XEnumeration > SheetEnumeration_BASE;
typedef ::cppu::WeakImplHelper3< container::XNameAccess, container::XIndexAccess, container::XEnumerationAccess > SheetCollectionHelper_BASE;
// a map ( or hashmap ) wont do as we need also to preserve the order
// (as added ) of the items
typedef std::vector< uno::Reference< sheet::XSpreadsheet > >  SheetMap;


// #FIXME #TODO the implementation of the Sheets collections sucks,
// e.g. there is no support for tracking sheets added/removed from the collection

static uno::Reference< uno::XInterface >
lcl_getModulAsUnoObject( const uno::Reference< sheet::XSpreadsheet >& xSheet, const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xRet;
    if ( !xSheet.is() )
        throw uno::RuntimeException();
    uno::Reference< beans::XPropertySet > xProps( xSheet, uno::UNO_QUERY_THROW );
    rtl::OUString sName;
    xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_CODENAME ) ) ) >>= sName;

    ScDocShell* pShell = excel::getDocShell( xModel );

    if ( pShell )
        xRet = getUnoDocModule(  sName, pShell );
    return xRet;
}

class WorkSheetsEnumeration : public SheetEnumeration_BASE
{
    SheetMap mSheetMap;
    SheetMap::iterator mIt;
public:
    WorkSheetsEnumeration( const SheetMap& sMap ) : mSheetMap( sMap ), mIt( mSheetMap.begin() ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( mIt != mSheetMap.end() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< sheet::XSpreadsheet > xSheet( *mIt++ );
        return uno::makeAny( xSheet ) ;
    }
};

class SheetCollectionHelper : public SheetCollectionHelper_BASE
{
    SheetMap mSheetMap;
    SheetMap::iterator cachePos;
public:
    SheetCollectionHelper( const SheetMap& sMap ) : mSheetMap( sMap ), cachePos(mSheetMap.begin()) {}
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  sheet::XSpreadsheet::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return ( !mSheetMap.empty() ); }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return uno::makeAny( *cachePos );
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > sNames( mSheetMap.size() );
        rtl::OUString* pString = sNames.getArray();
        SheetMap::iterator it = mSheetMap.begin();
        SheetMap::iterator it_end = mSheetMap.end();

        for ( ; it != it_end; ++it, ++pString )
        {
            uno::Reference< container::XNamed > xName( *it, uno::UNO_QUERY_THROW );
            *pString = xName->getName();
        }
        return sNames;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        cachePos = mSheetMap.begin();
        SheetMap::iterator it_end = mSheetMap.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            uno::Reference< container::XNamed > xName( *cachePos, uno::UNO_QUERY_THROW );
            if ( aName.equals( xName->getName() ) )
                break;
        }
        return ( cachePos != it_end );
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException) { return mSheetMap.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        return uno::makeAny( mSheetMap[ Index ] );

    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new WorkSheetsEnumeration( mSheetMap );
    }
};

class SheetsEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
public:
    SheetsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel( xModel ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< sheet::XSpreadsheet > xSheet( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        uno::Reference< uno::XInterface > xIf = lcl_getModulAsUnoObject( xSheet, m_xModel );
        uno::Any aRet;
        if ( !xIf.is() )
                {
            // if the Sheet is in a document created by the api unfortunately ( at the
            // moment, it actually wont have the special Document modules
            uno::Reference< excel::XWorksheet > xNewSheet( new ScVbaWorksheet( m_xParent, m_xContext, xSheet, m_xModel ) );
            aRet <<= xNewSheet;
                }
                else
            aRet <<= xIf;
        return aRet;
    }

};

ScVbaWorksheets::ScVbaWorksheets( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xSheets, const uno::Reference< frame::XModel >& xModel ): ScVbaWorksheets_BASE( xParent, xContext,  xSheets ), mxModel( xModel ), m_xSheets( uno::Reference< sheet::XSpreadsheets >( xSheets, uno::UNO_QUERY ) )
{
}

ScVbaWorksheets::ScVbaWorksheets( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XEnumerationAccess >& xEnumAccess, const uno::Reference< frame::XModel >& xModel  ):  ScVbaWorksheets_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xEnumAccess, uno::UNO_QUERY ) ), mxModel(xModel)
{
}

// XEnumerationAccess
uno::Type
ScVbaWorksheets::getElementType() throw (uno::RuntimeException)
{
    return excel::XWorksheet::static_type(0);
}

uno::Reference< container::XEnumeration >
ScVbaWorksheets::createEnumeration() throw (uno::RuntimeException)
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
    uno::Reference< XInterface > xIf = lcl_getModulAsUnoObject( xSheet, mxModel );
    uno::Any aRet;
    if ( !xIf.is() )
    {
        // if the Sheet is in a document created by the api unfortunately ( at the
        // moment, it actually wont have the special Document modules
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
                     const uno::Any& Count, const uno::Any& Type ) throw (uno::RuntimeException)
{
    if ( isSelectedSheets() )
        return uno::Any(); // or should we throw?

    rtl::OUString aStringSheet;
    sal_Bool bBefore(sal_True);
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
        bBefore = sal_True;
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

    SCTAB nSheetName = nCount + 1L;
    String aStringBase( RTL_CONSTASCII_USTRINGPARAM("Sheet") );
    uno::Any result;
    for (SCTAB i=0; i < nNewSheets; i++, nSheetName++)
    {
        String aStringName = aStringBase;
        aStringName += String::CreateFromInt32(nSheetName);
        while (m_xNameAccess->hasByName(aStringName))
        {
            nSheetName++;
            aStringName = aStringBase;
            aStringName += String::CreateFromInt32(nSheetName);
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
ScVbaWorksheets::Delete() throw (uno::RuntimeException)
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
ScVbaWorksheets::PrintOut( const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& ActivePrinter, const uno::Any& PrintToFile, const uno::Any& Collate, const uno::Any& PrToFileName ) throw (uno::RuntimeException)
{
    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    sal_Int16 nCopies = 1;
    sal_Bool bCollate = false;
    sal_Bool bSelection = false;
    From >>= nFrom;
    To >>= nTo;
    Copies >>= nCopies;
    if ( nCopies > 1 ) // Collate only useful when more that 1 copy
        Collate >>= bCollate;

    if ( !( nFrom || nTo ) )
        if ( isSelectedSheets() )
            bSelection = sal_True;

    PrintOutHelper( excel::getBestViewShell( mxModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, bSelection );
}

uno::Any SAL_CALL
ScVbaWorksheets::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible = sal_True;
    uno::Reference< container::XEnumeration > xEnum( createEnumeration(), uno::UNO_QUERY_THROW );
    while ( xEnum->hasMoreElements() )
    {
        uno::Reference< excel::XWorksheet > xSheet( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        if ( xSheet->getVisible() == false )
        {
                bVisible = false;
                break;
        }
    }
    return uno::makeAny( bVisible );
}

void SAL_CALL
ScVbaWorksheets::setVisible( const uno::Any& _visible ) throw (uno::RuntimeException)
{
    sal_Bool bState = false;
    if ( _visible >>= bState )
    {
        uno::Reference< container::XEnumeration > xEnum( createEnumeration(), uno::UNO_QUERY_THROW );
        while ( xEnum->hasMoreElements() )
        {
            uno::Reference< excel::XWorksheet > xSheet( xEnum->nextElement(), uno::UNO_QUERY_THROW );
            xSheet->setVisible( bState );
        }
    }
    else
        throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM( "Visible property doesn't support non boolean #FIXME" ) ), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaWorksheets::Select( const uno::Any& Replace ) throw (uno::RuntimeException)
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( mxModel );
    if ( !pViewShell )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot obtain view shell" ) ), uno::Reference< uno::XInterface >() );

    ScMarkData& rMarkData = pViewShell->GetViewData()->GetMarkData();
    sal_Bool bReplace = sal_True;
    Replace >>= bReplace;
    // Replace is defaulted to True, meanining this current collection
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
            rMarkData.SelectTable( static_cast< SCTAB >( pSheet->getSheetID() ), sal_True );
    }


}

void SAL_CALL
ScVbaWorksheets::Copy ( const uno::Any& Before, const uno::Any& After) throw (css::uno::RuntimeException)
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
ScVbaWorksheets::Item( const uno::Any& Index, const uno::Any& Index2  ) throw (uno::RuntimeException)
{
    if ( Index.getValueTypeClass() == uno::TypeClass_SEQUENCE )
    {
        uno::Reference< script::XTypeConverter > xConverter = getTypeConverter(mxContext);
        uno::Any aConverted;
        aConverted = xConverter->convertTo( Index, getCppuType((uno::Sequence< uno::Any >*)0) );
        SheetMap mSheets;
        uno::Sequence< uno::Any > sIndices;
        aConverted >>= sIndices;
        sal_Int32 nElems = sIndices.getLength();
        for( sal_Int32 index = 0; index < nElems; ++index )
        {
            uno::Reference< excel::XWorksheet > xWorkSheet( ScVbaWorksheets_BASE::Item( sIndices[ index ], Index2 ), uno::UNO_QUERY_THROW );
            ScVbaWorksheet* pWorkSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xWorkSheet );
            uno::Reference< sheet::XSpreadsheet > xSheet( pWorkSheet->getSheet() , uno::UNO_QUERY_THROW );
            uno::Reference< container::XNamed > xName( xSheet, uno::UNO_QUERY_THROW );
            mSheets.push_back( xSheet );
        }
        uno::Reference< container::XIndexAccess > xIndexAccess = new SheetCollectionHelper( mSheets );
        uno::Reference< XCollection > xSelectedSheets(  new ScVbaWorksheets( this->getParent(), mxContext, xIndexAccess, mxModel ) );
        return uno::makeAny( xSelectedSheets );
    }
    return  ScVbaWorksheets_BASE::Item( Index, Index2 );
}

uno::Any
ScVbaWorksheets::getItemByStringIndex( const rtl::OUString& sIndex ) throw (uno::RuntimeException)
{
    return ScVbaWorksheets_BASE::getItemByStringIndex( sIndex );
}

rtl::OUString
ScVbaWorksheets::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaWorksheets"));
}

css::uno::Sequence<rtl::OUString>
ScVbaWorksheets::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Worksheets") );
    }
    return sNames;
}

bool ScVbaWorksheets::nameExists( uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, const ::rtl::OUString & name, SCTAB& nTab ) throw ( lang::IllegalArgumentException )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nameExists() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
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

void ScVbaWorksheets::PrintPreview( const css::uno::Any& /*EnableChanges*/ ) throw (css::uno::RuntimeException)
{
    // need test, print preview current active sheet
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( mxModel );
    SfxViewFrame* pViewFrame = NULL;
    if ( pViewShell )
        pViewFrame = pViewShell->GetViewFrame();
    if ( pViewFrame )
    {
        if ( !pViewFrame->GetFrame().IsInPlace() )
        {
            dispatchExecute( pViewShell, SID_VIEWSHELL1 );
            SfxViewShell*  pShell = SfxViewShell::Get( pViewFrame->GetFrame().GetFrameInterface()->getController() );

            if (  pShell->ISA( ScPreviewShell ) )
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
                pPrvView->DataChanged();
                // set sensible first page
                long nPage = pPrvView->GetFirstPage( 1 );
                pPrvView->SetPageNo( nPage );
                WaitUntilPreviewIsClosed( pViewFrame );
                // restore old tab selection
                pViewShell = excel::getBestViewShell( mxModel );
                pViewShell->GetViewData()->GetMarkData().SetSelectedTabs(aOldTabs);
            }
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
