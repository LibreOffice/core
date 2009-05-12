/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbaworkbook.cxx,v $
 * $Revision: 1.5 $
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
#include "helperdecl.hxx"
#include <tools/urlobj.hxx>
#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XlFileFormat.hpp>

#include "scextopt.hxx"
#include "vbaworksheet.hxx"
#include "vbaworksheets.hxx"
#include "vbaworkbook.hxx"
#include "vbawindows.hxx"
#include "vbastyles.hxx"
#include "vbahelper.hxx"
#include "vbapalette.hxx"
#include <osl/file.hxx>
#include <stdio.h>
#include "vbanames.hxx"  // Amelia Wang
#include "nameuno.hxx"

// Much of the impl. for the equivalend UNO module is
// sc/source/ui/unoobj/docuno.cxx, viewuno.cxx

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ActiveSheet : public ScVbaWorksheet
{
protected:
    virtual uno::Reference< frame::XModel > getModel()
    {
        return getCurrentDocument();
    }
    virtual uno::Reference< sheet::XSpreadsheet > getSheet()
    {
        uno::Reference< frame::XModel > xModel = getModel();
        uno::Reference< sheet::XSpreadsheet > xSheet;
        if ( xModel.is() )
        {
            uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
                            xModel->getCurrentController(), uno::UNO_QUERY );
            if ( xSpreadsheet.is() )
                xSheet = xSpreadsheet->getActiveSheet();
        }
        return xSheet;
    }
public:
    ActiveSheet( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext ) : ScVbaWorksheet( xParent, xContext ) {}

};

uno::Sequence< sal_Int32 > ScVbaWorkbook::ColorData;

void ScVbaWorkbook::initColorData( const uno::Sequence< sal_Int32 >& sColors )
{
        const sal_Int32* pSource = sColors.getConstArray();
        sal_Int32* pDest = ColorData.getArray();
        const sal_Int32* pEnd = pSource + sColors.getLength();
        for ( ; pSource != pEnd; ++pSource, ++pDest )
            *pDest = *pSource;
}


void SAL_CALL
ScVbaWorkbook::ResetColors(  ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
        uno::Reference< container::XIndexAccess > xIndexAccess( ScVbaPalette::getDefaultPalette(), uno::UNO_QUERY_THROW );
        sal_Int32 nLen = xIndexAccess->getCount();
        ColorData.realloc( nLen );

        uno::Sequence< sal_Int32 > dDefaultColors( nLen );
        sal_Int32* pDest = dDefaultColors.getArray();
        for ( sal_Int32 index=0; index < nLen; ++pDest, ++index )
            xIndexAccess->getByIndex( index )  >>= (*pDest);
        initColorData( dDefaultColors );
}

::uno::Any SAL_CALL
ScVbaWorkbook::Colors( const ::uno::Any& Index ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
    uno::Any aRet;
    if ( Index.getValue() )
    {
        sal_Int32 nIndex = 0;
        Index >>= nIndex;
        aRet = uno::makeAny( XLRGBToOORGB( ColorData[ --nIndex ] ) );
    }
    else
        aRet = uno::makeAny( ColorData );
    return aRet;
}

::sal_Int32 SAL_CALL
ScVbaWorkbook::FileFormat(  ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
        sal_Int32 aFileFormat = 0;
        rtl::OUString aFilterName;
        uno::Sequence< beans::PropertyValue > aArgs = getModel()->getArgs();

        // #FIXME - seems suspect should we not walk through the properties
        // to find the FilterName
        if (aArgs[0].Name.equalsAscii( "FilterName")) {
            aArgs[0].Value >>= aFilterName;
        } else {
           aArgs[1].Value >>= aFilterName;
        }

        if (aFilterName.equalsAscii("Text - txt - csv (StarCalc)")) {
            aFileFormat = excel::XlFileFormat::xlCSV; //xlFileFormat.
        }

        if (aFilterName.equalsAscii("DBF")) {
            aFileFormat = excel::XlFileFormat::xlDBF4;
        }

        if (aFilterName.equalsAscii("DIF")) {
            aFileFormat = excel::XlFileFormat::xlDIF;
        }

        if (aFilterName.equalsAscii("Lotus")) {
            aFileFormat = excel::XlFileFormat::xlWK3;
        }

        if (aFilterName.equalsAscii("MS Excel 4.0")) {
            aFileFormat = excel::XlFileFormat::xlExcel4Workbook;
        }

        if (aFilterName.equalsAscii("MS Excel 5.0/95")) {
            aFileFormat = excel::XlFileFormat::xlExcel5;
        }

        if (aFilterName.equalsAscii("MS Excel 97")) {
            aFileFormat = excel::XlFileFormat::xlExcel9795;
        }

        if (aFilterName.equalsAscii("HTML (StarCalc)")) {
            aFileFormat = excel::XlFileFormat::xlHtml;
        }

        if (aFilterName.equalsAscii("calc_StarOffice_XML_Calc_Template")) {
            aFileFormat = excel::XlFileFormat::xlTemplate;
        }

        if (aFilterName.equalsAscii("StarOffice XML (Calc)")) {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }
        if (aFilterName.equalsAscii("calc8")) {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }

        return aFileFormat;
}

void
ScVbaWorkbook::init()
{
    if ( !ColorData.getLength() )
        ResetColors();
}
ScVbaWorkbook::ScVbaWorkbook(   const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext) :ScVbaWorkbook_BASE( xParent, xContext ), mxModel(NULL)
{
    //#FIXME this persists the color data per office instance and
    // not per workbook instance, need to hook the data into XModel
    // ( e.g. we already store the imported palette in there )
    // so we should,
    // a) make the class that does that a service
    // b) make that service implement XIndexContainer
    init();
}

ScVbaWorkbook::ScVbaWorkbook(   const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::frame::XModel > xModel ) : ScVbaWorkbook_BASE( xParent, xContext ),  mxModel( xModel )
{
    init();
}

ScVbaWorkbook::ScVbaWorkbook( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) : ScVbaWorkbook_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext ),  mxModel( getXSomethingFromArgs< frame::XModel >( args, 1 ) )

{
    init();
}

::rtl::OUString
ScVbaWorkbook::getName() throw (uno::RuntimeException)
{
    rtl::OUString sName = getModel()->getURL();
    if ( sName.getLength() )
    {

        INetURLObject aURL( getModel()->getURL() );
        ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    }
    else
    {
        const static rtl::OUString sTitle( RTL_CONSTASCII_USTRINGPARAM("Title" ) );
        // process "UntitledX - $(PRODUCTNAME)"
        uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue(sTitle ) >>= sName;
        sal_Int32 pos = 0;
        sName = sName.getToken(0,' ',pos);
    }
    return sName;
}
::rtl::OUString
ScVbaWorkbook::getPath() throw (uno::RuntimeException)
{
    INetURLObject aURL( getModel()->getURL() );
    aURL.CutLastName();
    return aURL.GetURLPath();
}

::rtl::OUString
ScVbaWorkbook::getFullName() throw (uno::RuntimeException)
{
    INetURLObject aURL( getModel()->getURL() );
    return aURL.GetURLPath();
}
uno::Reference< excel::XWorksheet >
ScVbaWorkbook::getActiveSheet() throw (uno::RuntimeException)
{
    return new ActiveSheet( this, mxContext );
}
uno::Any SAL_CALL
ScVbaWorkbook::Sheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return Worksheets( aIndex );
}



uno::Any SAL_CALL
ScVbaWorkbook::Worksheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel() );
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY_THROW );
    uno::Reference<container::XIndexAccess > xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xWorkSheets(  new ScVbaWorksheets( this, mxContext, xSheets, xModel ) );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        return uno::Any( xWorkSheets );
    }
    // pass on to collection
    return uno::Any( xWorkSheets->Item( aIndex, uno::Any() ) );
}
uno::Any SAL_CALL
ScVbaWorkbook::Windows( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection >  xWindows = ScVbaWindows::Windows( mxContext );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return uno::Any( xWindows->Item( aIndex, uno::Any() ) );
}
void
ScVbaWorkbook::Close( const uno::Any &rSaveArg, const uno::Any &rFileArg,
                      const uno::Any &rRouteArg ) throw (uno::RuntimeException)
{
    sal_Bool bSaveChanges = sal_False;
    rtl::OUString aFileName;
    sal_Bool bRouteWorkbook = sal_True;

    rSaveArg >>= bSaveChanges;
    sal_Bool bFileName =  ( rFileArg >>= aFileName );
    rRouteArg >>= bRouteWorkbook;
    uno::Reference< frame::XStorable > xStorable( getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< util::XModifiable > xModifiable( getModel(), uno::UNO_QUERY_THROW );

    if( bSaveChanges )
    {
        if( xStorable->isReadonly() )
        {
            throw uno::RuntimeException(::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "Unable to save to a read only file ") ),
                            uno::Reference< XInterface >() );
        }
        if( bFileName )
            xStorable->storeAsURL( aFileName, uno::Sequence< beans::PropertyValue >(0) );
        else
            xStorable->store();
    }
    else
        xModifiable->setModified( false );

    uno::Reference< util::XCloseable > xCloseable( getModel(), uno::UNO_QUERY );

    if( xCloseable.is() )
        // use close(boolean DeliverOwnership)

        // The boolean parameter DeliverOwnership tells objects vetoing the close process that they may
        // assume ownership if they object the closure by throwing a CloseVetoException
        // Here we give up ownership. To be on the safe side, catch possible veto exception anyway.
        xCloseable->close(sal_True);
    // If close is not supported by this model - try to dispose it.
    // But if the model disagree with a reset request for the modify state
    // we shouldn't do so. Otherwhise some strange things can happen.
    else
    {
        uno::Reference< lang::XComponent > xDisposable ( getCurrentDocument(), uno::UNO_QUERY );
        if ( xDisposable.is() )
            xDisposable->dispose();
    }
}

void
ScVbaWorkbook::Protect( const uno::Any &aPassword ) throw (uno::RuntimeException)
{
    rtl::OUString rPassword;
    uno::Reference< util::XProtectable > xProt( getModel(), uno::UNO_QUERY_THROW );
    SC_VBA_FIXME(("Workbook::Protect stub"));
    if(  aPassword >>= rPassword )
        xProt->protect( rPassword );
    else
        xProt->protect( rtl::OUString() );
}

void
ScVbaWorkbook::Unprotect( const uno::Any &aPassword ) throw (uno::RuntimeException)
{
    rtl::OUString rPassword;
    uno::Reference< util::XProtectable > xProt( getModel(), uno::UNO_QUERY_THROW );
    if( !getProtectStructure() )
        throw uno::RuntimeException(::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM( "File is already unprotected" ) ),
            uno::Reference< XInterface >() );
    else
    {
        if( aPassword >>= rPassword )
            xProt->unprotect( rPassword );
        else
            xProt->unprotect( rtl::OUString() );
    }
}

::sal_Bool
ScVbaWorkbook::getProtectStructure() throw (uno::RuntimeException)
{
    uno::Reference< util::XProtectable > xProt( getModel(), uno::UNO_QUERY_THROW );
    return xProt->isProtected();
}

void
ScVbaWorkbook::setSaved( sal_Bool bSave ) throw (uno::RuntimeException)
{
    uno::Reference< util::XModifiable > xModifiable( getModel(), uno::UNO_QUERY_THROW );
    xModifiable->setModified( bSave );
}

sal_Bool
ScVbaWorkbook::getSaved() throw (uno::RuntimeException)
{
    uno::Reference< util::XModifiable > xModifiable( getModel(), uno::UNO_QUERY_THROW );
    return xModifiable->isModified();
}

void
ScVbaWorkbook::Save() throw (uno::RuntimeException)
{
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Save"));
    uno::Reference< frame::XModel > xModel = getModel();
    dispatchRequests(xModel,url);
}

void
ScVbaWorkbook::SaveCopyAs( const rtl::OUString& sFileName ) throw ( uno::RuntimeException)
{
    rtl::OUString aURL;
    osl::FileBase::getFileURLFromSystemPath( sFileName, aURL );
    uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
    uno::Sequence<  beans::PropertyValue > storeProps(1);
    storeProps[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
    storeProps[0].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MS Excel 97" ) );
    xStor->storeToURL( aURL, storeProps );
}

void
ScVbaWorkbook::Activate() throw (uno::RuntimeException)
{
    uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    xFrame->activate();
}

css::uno::Any SAL_CALL
ScVbaWorkbook::Styles( const::uno::Any& Item ) throw (uno::RuntimeException)
{
    // quick look and Styles object doesn't seem to have a valid parent
    // or a least the object browser just shows an object that has no
    // variables ( therefore... leave as NULL for now )
    uno::Reference< XCollection > dStyles = new ScVbaStyles( uno::Reference< XHelperInterface >(), mxContext, getModel() );
    if ( Item.hasValue() )
        return dStyles->Item( Item, uno::Any() );
    return uno::makeAny( dStyles );
}

// Amelia Wang
uno::Any SAL_CALL
ScVbaWorkbook::Names( const css::uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel() );
    uno::Reference< beans::XPropertySet > xProps( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges(  xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NamedRanges") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xNames( new ScVbaNames( this , mxContext , xNamedRanges , xModel ));
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
    return uno::Any( xNames );
}
    return uno::Any( xNames->Item( aIndex, uno::Any() ) );
}

rtl::OUString&
ScVbaWorkbook::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaWorkbook") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaWorkbook::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Workbook" ) );
    }
    return aServiceNames;
}

::rtl::OUString SAL_CALL
ScVbaWorkbook::getCodeName() throw (css::uno::RuntimeException)
{
#ifdef VBA_OOBUILD_HACK
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocShell( xModel )->GetDocument();
    ScExtDocOptions* pExtOptions = pDoc->GetExtDocOptions();
    ScExtDocSettings pExtSettings = pExtOptions->GetDocSettings();
    ::rtl::OUString sGlobCodeName = pExtSettings.maGlobCodeName;
    return sGlobCodeName;
#else
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
#endif
}
#ifdef VBA_OOBUILD_HACK
void SAL_CALL
ScVbaWorkbook::setCodeName( const ::rtl::OUString& sGlobCodeName ) throw (css::uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocShell( xModel )->GetDocument();
    ScExtDocOptions* pExtOptions = pDoc->GetExtDocOptions();
    ScExtDocSettings pExtSettings = pExtOptions->GetDocSettings();
    pExtSettings.maGlobCodeName = sGlobCodeName;
#else
void SAL_CALL
ScVbaWorkbook::setCodeName( const ::rtl::OUString& ) throw (css::uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
#endif
}

namespace workbook
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWorkbook, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWorkbook",
    "ooo.vba.excel.Workbook" );
}
