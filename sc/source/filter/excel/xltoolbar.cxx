/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Noel Power <noel.power@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *       Noel Power <noel.power@novell.com>
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include "xltoolbar.hxx"
#include <rtl/ustrbuf.hxx>
#include <stdarg.h>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <fstream>
#include <comphelper/processfactory.hxx>
#include <vcl/graph.hxx>
#include <map>
using namespace com::sun::star;

typedef std::map< sal_Int16, rtl::OUString > IdToString;

class MSOExcelCommandConvertor : public MSOCommandConvertor
{
   IdToString msoToOOcmd;
   IdToString tcidToOOcmd;
public:
    MSOExcelCommandConvertor();
    virtual rtl::OUString MSOCommandToOOCommand( sal_Int16 msoCmd );
    virtual rtl::OUString MSOTCIDToOOCommand( sal_Int16 key );
};

MSOExcelCommandConvertor::MSOExcelCommandConvertor()
{
/*
    // mso command id to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    msoToOOcmd[ 0x20b ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:CloseDoc") );
    msoToOOcmd[ 0x50 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Open") );

   // mso tcid to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
   tcidToOOcmd[ 0x9d9 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Print") );
*/
}

rtl::OUString MSOExcelCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = msoToOOcmd.find( key );
    if ( it != msoToOOcmd.end() )
        sResult = it->second;
    return sResult;
}

rtl::OUString MSOExcelCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = tcidToOOcmd.find( key );
    if ( it != tcidToOOcmd.end() )
        sResult = it->second;
    return sResult;
}



CTBS::CTBS() : bSignature(0), bVersion(0), reserved1(0), reserved2(0), reserved3(0), ctb(0), ctbViews(0), ictbView(0)
{
}

ScCTB::ScCTB() : nViews( 0 ), ectbid(0)
{
}

ScCTB::ScCTB(sal_uInt16 nNum ) : nViews( nNum ), ectbid(0)
{
}

bool ScCTB::Read( SvStream &rS )
{
    OSL_TRACE("ScCTB::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    tb.Read( rS );
    for ( sal_uInt16 index = 0; index < nViews; ++index )
    {
        TBVisualData aVisData;
        aVisData.Read( rS );
        rVisualData.push_back( aVisData );
    }
    rS >> ectbid;

    for ( sal_Int16 index = 0; index < tb.getcCL(); ++index )
    {
        ScTBC aTBC;
        aTBC.Read( rS );
        rTBC.push_back( aTBC );
    }
    return true;
}

void ScCTB::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] ScCTB -- dump\n", nOffSet );
    indent_printf( fp, "  nViews 0x%x\n", nViews);
    tb.Print( fp );

    std::vector<TBVisualData>::iterator visData_end = rVisualData.end();
    sal_Int32 counter = 0;
    for ( std::vector<TBVisualData>::iterator it = rVisualData.begin(); it != visData_end; ++it )
    {

        indent_printf( fp, "  TBVisualData [%d]\n", counter++ );
        Indent b;
        it->Print( fp );
    }
    indent_printf( fp, "  ectbid 0x%x\n", ectbid);
    std::vector<ScTBC>::iterator it_end = rTBC.end();
    counter = 0;
    for ( std::vector<ScTBC>::iterator it = rTBC.begin(); it != it_end; ++it )
    {
        indent_printf( fp, "  ScTBC [%d]\n", counter++);
        Indent c;
        it->Print( fp );
    }
}

bool ScCTB::IsMenuToolbar()
{
    return tb.IsMenuToolbar();
}

bool ScCTB::ImportMenuTB( ScCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& xMenuDesc, CustomToolBarImportHelper& helper )
{
    sal_Int32 index = 0;
    for ( std::vector< ScTBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it, ++index )
    {
        if ( !it->ImportToolBarControl( rWrapper, xMenuDesc, helper, IsMenuToolbar() ) )
            return false;
    }
    return true;
}

bool ScCTB::ImportCustomToolBar( ScCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{

    static rtl::OUString sToolbarPrefix( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/custom_" ) );
    bool bRes = false;
    try
    {
        if ( !tb.IsEnabled() )
            return true;  // didn't fail, just ignoring

        // Create default setting
        uno::Reference< container::XIndexContainer > xIndexContainer( helper.getCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xIndexContainer, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xIndexContainer, uno::UNO_QUERY_THROW );
        WString& name = tb.getName();
        // set UI name for toolbar
        xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UIName") ), uno::makeAny( name.getString() ) );

        rtl::OUString sToolBarName = sToolbarPrefix.concat( name.getString() );
        for ( std::vector< ScTBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it )
        {
            if ( !it->ImportToolBarControl( rWrapper, xIndexContainer, helper, IsMenuToolbar() ) )
                return false;
        }

        OSL_TRACE("Name of toolbar :-/ %s", rtl::OUStringToOString( sToolBarName, RTL_TEXTENCODING_UTF8 ).getStr() );

        helper.getCfgManager()->insertSettings( sToolBarName, xIndexAccess );
        helper.applyIcons();

        uno::Reference< ui::XUIConfigurationPersistence > xPersistence( helper.getCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();

        xPersistence.set( helper.getCfgManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();

        bRes = true;
    }
    catch( uno::Exception& )
    {
       bRes = false;
    }
    return bRes;
}
bool CTBS::Read( SvStream &rS )
{
    OSL_TRACE("CTBS::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> bSignature >> bVersion >> reserved1 >> reserved2 >> reserved3 >> ctb >> ctbViews >> ictbView;
    return true;
}

void CTBS::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] CTBS -- dump\n", nOffSet );

    indent_printf( fp, "  bSignature 0x%x\n", bSignature);
    indent_printf( fp, "  bVersion 0x%x\n", bVersion);

    indent_printf( fp, "  reserved1 0x%x\n", reserved1 );
    indent_printf( fp, "  reserved2 0x%x\n", reserved2 );
    indent_printf( fp, "  reserved3 0x%x\n", reserved3 );

    indent_printf( fp, "  ctb 0x%x\n", ctb );
    indent_printf( fp, "  ctbViews 0x%x\n", ctbViews );
    indent_printf( fp, "  ictbView 0x%x\n", ictbView );
}

ScTBC::ScTBC()
{
}

bool
ScTBC::Read(SvStream &rS)
{
    OSL_TRACE("ScTBC::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( !tbch.Read( rS ) )
        return false;
    sal_uInt16 tcid = tbch.getTcID();
    sal_uInt8 tct = tbch.getTct();
    if (  ( tcid != 0x0001 && tcid != 0x06CC && tcid != 0x03D8 && tcid != 0x03EC && tcid != 0x1051 ) && ( ( tct > 0 && tct < 0x0B ) || ( ( tct > 0x0B && tct < 0x10 ) || tct == 0x15 ) ) )
    {
        tbcCmd.reset( new TBCCmd );
        if ( !  tbcCmd->Read( rS ) )
            return false;
    }
    if ( tct != 0x16 )
    {
        tbcd.reset( new TBCData( tbch ) );
        if ( !tbcd->Read( rS ) )
            return false;
    }
    return true;
}


void
ScTBC::Print(FILE* fp)
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] ScTBC -- dump\n", nOffSet );
    tbch.Print( fp );
    if ( tbcCmd.get() )
        tbcCmd->Print( fp );
    if ( tbcd.get() )
        tbcd->Print( fp );
}

bool ScTBC::ImportToolBarControl( ScCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuToolbar )
{
    // how to identify built-in-command ?
//    bool bBuiltin = false;
    if ( tbcd.get() )
    {
        std::vector< css::beans::PropertyValue > props;
        bool bBeginGroup = false;
        if ( ! tbcd->ImportToolBarControl( helper, props, bBeginGroup, bIsMenuToolbar ) )
            return false;
        TBCMenuSpecific* pMenu = tbcd->getMenuSpecific();
        if ( pMenu )
        {
            // search for ScCTB with the appropriate name ( it contains the
            // menu items, although we cannot import ( or create ) a menu on
            // a custom toolbar we can import the menu items in a separate
            // toolbar ( better than nothing )
            ScCTB* pCustTB = rWrapper.GetCustomizationData( pMenu->Name() );
            if ( pCustTB )
            {
                 uno::Reference< container::XIndexContainer > xMenuDesc;
                 uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                 xMenuDesc.set( xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.IndexedPropertyValues" ) ) ), uno::UNO_QUERY_THROW );
                 if ( !pCustTB->ImportMenuTB( rWrapper, xMenuDesc, helper ) )
                     return false;
                 if ( !bIsMenuToolbar )
                 {
                     if ( !helper.createMenu( pMenu->Name(), uno::Reference< container::XIndexAccess >( xMenuDesc, uno::UNO_QUERY ), true ) )
                         return false;
                 }
                 else
                 {
                     beans::PropertyValue aProp;
                     aProp.Name =  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ItemDescriptorContainer") );
                     aProp.Value <<= xMenuDesc;
                     props.push_back( aProp );
                 }
            }
        }

        if ( bBeginGroup )
        {
            // insert spacer
            uno::Sequence< beans::PropertyValue > sProps( 1 );
            sProps[ 0 ].Name =  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Type") );
            sProps[ 0 ].Value = uno::makeAny( ui::ItemType::SEPARATOR_LINE );
            toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( sProps ) );
        }
        uno::Sequence< beans::PropertyValue > sProps( props.size() );
        beans::PropertyValue* pProp = sProps.getArray();

        for ( std::vector< css::beans::PropertyValue >::iterator it = props.begin(); it != props.end(); ++it, ++pProp )
            *pProp = *it;

        toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( sProps ) );
    }
    return true;
}

void
TBCCmd::Print(FILE* fp)
{
    Indent a;
    indent_printf( fp, " TBCCmd -- dump\n" );
    indent_printf( fp, "   cmdID 0x%x\n", cmdID );
    indent_printf( fp, "   A ( fHideDrawing ) %s\n", A ? "true" : "false" );
    indent_printf( fp, "   B ( reserved - ignored ) %s\n", A ? "true" : "false" );
    indent_printf( fp, "   cmdType 0x%x\n", cmdType );
    indent_printf( fp, "   C ( reserved - ignored ) %s\n", A ? "true" : "false" );
    indent_printf( fp, "   reserved3 0x%x\n", reserved3 );
}

bool TBCCmd::Read( SvStream &rS )
{
    OSL_TRACE("TBCCmd::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> cmdID;
    sal_uInt16 temp;
    rS >> temp;
    OSL_TRACE("TBCmd temp = 0x%x", temp );
    A = (temp & 0x8000 ) == 0x8000;
    B = (temp & 0x4000) == 0x4000;
    cmdType = ( temp & 0x3E00 ) >> 9;
    C = ( temp & 0x100 ) == 0x100;
    reserved3 = ( temp & 0xFF );
    return true;
}

ScCTBWrapper::ScCTBWrapper()
{
}

ScCTBWrapper::~ScCTBWrapper()
{
}

bool
ScCTBWrapper::Read( SvStream &rS)
{
    OSL_TRACE("ScCTBWrapper::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( !ctbSet.Read( rS ) )
        return false;
    for ( sal_uInt16 index = 0; index < ctbSet.ctb; ++index )
    {
        ScCTB aCTB( ctbSet.ctbViews );
        if ( !aCTB.Read( rS ) )
            return false;
        rCTB.push_back( aCTB );
    }
    return true;
}

void
ScCTBWrapper::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] ScCTBWrapper -- dump\n", nOffSet );
    ctbSet.Print( fp );
    std::vector<ScCTB>::iterator it_end = rCTB.end();
    for ( std::vector<ScCTB>::iterator it = rCTB.begin(); it != it_end; ++it )
    {
        Indent b;
        it->Print( fp );
    }
}

ScCTB* ScCTBWrapper::GetCustomizationData( const rtl::OUString& sTBName )
{
    ScCTB* pCTB = NULL;
    for ( std::vector< ScCTB >::iterator it = rCTB.begin(); it != rCTB.end(); ++it )
    {
        if ( it->GetName().equals( sTBName ) )
        {
            pCTB = &(*it);
            break;
        }
    }
    return pCTB;
}

bool ScCTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    if(rCTB.empty())
        return true;

    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( ui::ModuleUIConfigurationManagerSupplier::create(xContext) );

    std::vector<ScCTB>::iterator it_end = rCTB.end();
    for ( std::vector<ScCTB>::iterator it = rCTB.begin(); it != it_end; ++it )
    {
        // for each customtoolbar
        CustomToolBarImportHelper helper( rDocSh, xAppCfgSupp->getUIConfigurationManager( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument" ) ) ) );
        helper.setMSOCommandMap( new  MSOExcelCommandConvertor() );
        // Ignore menu toolbars, excel doesn't ( afaics ) store
        // menu customizations ( but you can have menus in a customtoolbar
        // such menus will be dealt with when they are encountered
        // as part of importing the appropriate MenuSpecific toolbar control )


        if ( !(*it).IsMenuToolbar() )
        {
            if ( !(*it).ImportCustomToolBar( *this, helper ) )
                return false;
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
