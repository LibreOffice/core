/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworkbook.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:12:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <tools/urlobj.hxx>

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "vbaworksheet.hxx"
#include "vbaworksheets.hxx"
#include "vbaworkbook.hxx"
#include "vbawindows.hxx"
#include "vbahelper.hxx"

#include <stdio.h>

// Much of the impl. for the equivalend UNO module is
// sc/source/ui/unoobj/docuno.cxx, viewuno.cxx

using namespace ::org::openoffice;
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
    ActiveSheet( uno::Reference< uno::XComponentContext >& xContext ) : ScVbaWorksheet( xContext ) {}

};


::rtl::OUString
ScVbaWorkbook::getName() throw (uno::RuntimeException)
{
    rtl::OUString sName = getModel()->getURL();
    if ( sName.getLength() )
    {

        INetURLObject aURL( getModel()->getURL() );
        sName = aURL.GetLastName();
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
    return new ActiveSheet( m_xContext );
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
    uno::Reference<sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< vba::XCollection > xWorkSheets( uno::Reference< excel::XWorksheets >( new ScVbaWorksheets(m_xContext, xSheets, xModel ) ), uno::UNO_QUERY_THROW );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        return uno::Any( xWorkSheets );
    }
    return uno::Any( xWorkSheets->Item( aIndex ) );
}
uno::Any SAL_CALL
ScVbaWorkbook::Windows( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< vba::XCollection >  xWindows = ScVbaWindows::Windows( m_xContext );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return uno::Any( xWindows->Item( aIndex ) );
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
ScVbaWorkbook::Activate() throw (uno::RuntimeException)
{
    uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    xFrame->activate();
}

