/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbadialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:50:19 $
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
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <org/openoffice/excel/XApplication.hpp>

#include <tools/string.hxx>

#include "vbahelper.hxx"
#include "vbadialog.hxx"
#include "vbaglobals.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

// fails silently
void
ScVbaDialog::Show() throw(uno::RuntimeException)
{
    rtl::OUString aURL;
        uno::Reference< frame::XModel > xModel = getCurrentDocument();
    if ( xModel.is() )
    {
        aURL = mapIndexToName( mnIndex );
        if( aURL.getLength()  )
            throw uno::RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " Unable to open the specified dialog " ) ),
                uno::Reference< XInterface > () );
        dispatchRequests( xModel, aURL );
    }
}


static const rtl::OUString aStringList[]=
{
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Open" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatCellDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertCell" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Print" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PasteSpecial" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ToolProtectionDocument" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ColumnWidth" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineName" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConfigureDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:HyperlinkDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertGraphic" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertObject" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PageFormatDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataSort" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:RowHeight" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoCorrectDlg" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ConditionalFormatDialog" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataConsolidate" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CreateNames" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillSeries" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Validation") ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DefineLabelRange" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterAutoFilter" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DataFilterSpecialFilter" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoFormat" ) )
};

const sal_Int32 nDialogSize = sizeof( aStringList ) / sizeof( aStringList[ 0 ] );

rtl::OUString
ScVbaDialog::mapIndexToName( sal_Int32 nIndex )
{
    if( nIndex < nDialogSize )
        return aStringList[ nIndex ];
    return rtl::OUString();
}

rtl::OUString&
ScVbaDialog::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPivotTable") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaDialog::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.PivotTable" ) );
    }
    return aServiceNames;
}
