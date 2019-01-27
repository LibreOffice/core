/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "xltoolbar.hxx"
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <map>

using namespace com::sun::star;

typedef std::map< sal_Int16, OUString > IdToString;

class MSOExcelCommandConvertor : public MSOCommandConvertor
{
   IdToString msoToOOcmd;
   IdToString tcidToOOcmd;
public:
    MSOExcelCommandConvertor();
    virtual OUString MSOCommandToOOCommand( sal_Int16 msoCmd ) override;
    virtual OUString MSOTCIDToOOCommand( sal_Int16 key ) override;
};

MSOExcelCommandConvertor::MSOExcelCommandConvertor()
{
/*
    // mso command id to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    msoToOOcmd[ 0x20b ] = ".uno:CloseDoc";
    msoToOOcmd[ 0x50 ] = ".uno:Open";

   // mso tcid to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
   tcidToOOcmd[ 0x9d9 ] = ".uno:Print";
*/
}

OUString MSOExcelCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    OUString sResult;
    IdToString::iterator it = msoToOOcmd.find( key );
    if ( it != msoToOOcmd.end() )
        sResult = it->second;
    return sResult;
}

OUString MSOExcelCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    OUString sResult;
    IdToString::iterator it = tcidToOOcmd.find( key );
    if ( it != tcidToOOcmd.end() )
        sResult = it->second;
    return sResult;
}

CTBS::CTBS() : bSignature(0), bVersion(0), reserved1(0), reserved2(0), reserved3(0), ctb(0), ctbViews(0), ictbView(0)
{
}

ScCTB::ScCTB(sal_uInt16 nNum ) : nViews( nNum ), ectbid(0)
{
}

bool ScCTB::Read( SvStream &rS )
{
    SAL_INFO("sc.filter", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    tb.Read( rS );
    for ( sal_uInt16 index = 0; index < nViews; ++index )
    {
        TBVisualData aVisData;
        aVisData.Read( rS );
        rVisualData.push_back( aVisData );
    }
    rS.ReadUInt32( ectbid );

    for ( sal_Int16 index = 0; index < tb.getcCL(); ++index )
    {
        ScTBC aTBC;
        aTBC.Read( rS );
        rTBC.push_back( aTBC );
    }
    return true;
}

#ifdef DEBUG_SC_EXCEL
void ScCTB::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] ScCTB -- dump\n", nOffSet );
    indent_printf( fp, "  nViews 0x%x\n", nViews);
    tb.Print( fp );

    sal_Int32 counter = 0;
    for ( auto& rItem : rVisualData )
    {
        indent_printf( fp, "  TBVisualData [%d]\n", counter++ );
        Indent b;
        rItem.Print( fp );
    }
    indent_printf( fp, "  ectbid 0x%x\n", ectbid);
    counter = 0;
    for ( auto& rItem : rTBC )
    {
        indent_printf( fp, "  ScTBC [%d]\n", counter++);
        Indent c;
        rItem.Print( fp );
    }
}
#endif

bool ScCTB::IsMenuToolbar()
{
    return tb.IsMenuToolbar();
}

bool ScCTB::ImportMenuTB( ScCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& xMenuDesc, CustomToolBarImportHelper& helper )
{
    for ( auto& rItem : rTBC )
    {
        if ( !rItem.ImportToolBarControl( rWrapper, xMenuDesc, helper, IsMenuToolbar() ) )
            return false;
    }
    return true;
}

bool ScCTB::ImportCustomToolBar( ScCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{

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
        xProps->setPropertyValue("UIName", uno::makeAny( name.getString() ) );

        OUString sToolBarName = "private:resource/toolbar/custom_" + name.getString();
        for ( auto& rItem : rTBC )
        {
            if ( !rItem.ImportToolBarControl( rWrapper, xIndexContainer, helper, IsMenuToolbar() ) )
                return false;
        }

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
    SAL_INFO("sc.filter", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUChar( bSignature ).ReadUChar( bVersion ).ReadUInt16( reserved1 ).ReadUInt16( reserved2 ).ReadUInt16( reserved3 ).ReadUInt16( ctb ).ReadUInt16( ctbViews ).ReadUInt16( ictbView );
    return true;
}

#ifdef DEBUG_SC_EXCEL
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
#endif

ScTBC::ScTBC()
{
}

bool
ScTBC::Read(SvStream &rS)
{
    SAL_INFO("sc.filter", "stream pos " << rS.Tell());
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

#ifdef DEBUG_SC_EXCEL
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
#endif

bool ScTBC::ImportToolBarControl( ScCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuToolbar )
{
    // how to identify built-in-command ?
//    bool bBuiltin = false;
    if ( tbcd.get() )
    {
        std::vector< css::beans::PropertyValue > props;
        bool bBeginGroup = false;
        tbcd->ImportToolBarControl( helper, props, bBeginGroup, bIsMenuToolbar );
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
                 uno::Reference< container::XIndexContainer > xMenuDesc = document::IndexedPropertyValues::create( comphelper::getProcessComponentContext() );
                 if ( !pCustTB->ImportMenuTB( rWrapper, xMenuDesc, helper ) )
                     return false;
                 if ( !bIsMenuToolbar )
                 {
                     if ( !helper.createMenu( pMenu->Name(), uno::Reference< container::XIndexAccess >( xMenuDesc, uno::UNO_QUERY ) ) )
                         return false;
                 }
                 else
                 {
                     beans::PropertyValue aProp;
                     aProp.Name = "ItemDescriptorContainer";
                     aProp.Value <<= xMenuDesc;
                     props.push_back( aProp );
                 }
            }
        }

        if ( bBeginGroup )
        {
            // insert spacer
            uno::Sequence< beans::PropertyValue > sProps( 1 );
            sProps[ 0 ].Name = "Type";
            sProps[ 0 ].Value <<= ui::ItemType::SEPARATOR_LINE;
            toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( sProps ) );
        }
        toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( comphelper::containerToSequence(props) ) );
    }
    return true;
}

#ifdef DEBUG_SC_EXCEL
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
#endif

bool TBCCmd::Read( SvStream &rS )
{
    SAL_INFO("sc.filter", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUInt16( cmdID );
    sal_uInt16 temp;
    rS.ReadUInt16( temp );
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
    SAL_INFO("sc.filter", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    if (!ctbSet.Read(rS))
        return false;

    //ScCTB is 1 TB which is min 15bytes, nViews TBVisualData which is min 20bytes
    //and one 32bit number (4 bytes)
    const size_t nMinRecordSize = 19 + ctbSet.ctbViews * 20;
    const size_t nMaxPossibleRecords = rS.remainingSize()/nMinRecordSize;
    if (ctbSet.ctb > nMaxPossibleRecords)
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

#ifdef DEBUG_SC_EXCEL
void
ScCTBWrapper::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] ScCTBWrapper -- dump\n", nOffSet );
    ctbSet.Print( fp );
    for ( auto& rItem : rCTB )
    {
        Indent b;
        rItem.Print( fp );
    }
}
#endif

ScCTB* ScCTBWrapper::GetCustomizationData( const OUString& sTBName )
{
    ScCTB* pCTB = nullptr;
    auto it = std::find_if(rCTB.begin(), rCTB.end(), [&sTBName](ScCTB& rItem) { return rItem.GetName() == sTBName; });
    if (it != rCTB.end())
        pCTB = &(*it);
    return pCTB;
}

void ScCTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    if(rCTB.empty())
        return;

    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( ui::theModuleUIConfigurationManagerSupplier::get(xContext) );

    for ( auto& rItem : rCTB )
    {
        // for each customtoolbar
        CustomToolBarImportHelper helper( rDocSh, xAppCfgSupp->getUIConfigurationManager( "com.sun.star.sheet.SpreadsheetDocument" ) );
        helper.setMSOCommandMap( new  MSOExcelCommandConvertor() );
        // Ignore menu toolbars, excel doesn't ( afaics ) store
        // menu customizations ( but you can have menus in a customtoolbar
        // such menus will be dealt with when they are encountered
        // as part of importing the appropriate MenuSpecific toolbar control )

        if ( !rItem.IsMenuToolbar() && !rItem.ImportCustomToolBar( *this, helper ) )
            return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
