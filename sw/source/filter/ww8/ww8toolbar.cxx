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
#include "ww8toolbar.hxx"
#include "ww8scan.hxx"
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

// no. of visual data elements in a CTB ( fixed )
const short nVisualData = 5;

typedef std::map< sal_Int16, rtl::OUString > IdToString;

class MSOWordCommandConvertor : public MSOCommandConvertor
{
   IdToString msoToOOcmd;
   IdToString tcidToOOcmd;
public:
    MSOWordCommandConvertor();
    virtual rtl::OUString MSOCommandToOOCommand( sal_Int16 msoCmd );
    virtual rtl::OUString MSOTCIDToOOCommand( sal_Int16 key );
};

MSOWordCommandConvertor::MSOWordCommandConvertor()
{
    // mso command id to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    msoToOOcmd[ 0x20b ] = ".uno:CloseDoc";
    msoToOOcmd[ 0x50 ] = ".uno:Open";

   // mso tcid to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
   tcidToOOcmd[ 0x9d9 ] = ".uno:Print";
}

rtl::OUString MSOWordCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = msoToOOcmd.find( key );
    if ( it != msoToOOcmd.end() )
        sResult = it->second;
    return sResult;
}

rtl::OUString MSOWordCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = tcidToOOcmd.find( key );
    if ( it != tcidToOOcmd.end() )
        sResult = it->second;
    return sResult;
}


CTBWrapper::CTBWrapper( bool bReadId ) : Tcg255SubStruct( bReadId )
,reserved2(0)
,reserved3(0)
,reserved4(0)
,reserved5(0)
,cbTBD(0)
,cCust(0)
,cbDTBC(0)
,rtbdc(0)
{
}

CTBWrapper::~CTBWrapper()
{
}

Customization* CTBWrapper::GetCustomizaton( sal_Int16 index )
{
    if ( index < 0 || index >= static_cast<sal_Int16>( rCustomizations.size() ) )
        return NULL;
    return &rCustomizations[ index ];
}

CTB* CTBWrapper::GetCustomizationData( const rtl::OUString& sTBName )
{
    CTB* pCTB = NULL;
    for ( std::vector< Customization >::iterator it = rCustomizations.begin(); it != rCustomizations.end(); ++it )
    {
        if ( it->GetCustomizationData() && it->GetCustomizationData()->GetName().equals( sTBName ) )
        {
            pCTB = it->GetCustomizationData();
            break;
        }
    }
    return pCTB;
}

bool CTBWrapper::Read( SvStream& rS )
{
    OSL_TRACE("CTBWrapper::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS >> reserved2 >> reserved3 >> reserved4 >> reserved5;
    rS >> cbTBD >> cCust >> cbDTBC;
    long nExpectedPos =  rS.Tell() + cbDTBC;
    if ( cbDTBC )
    {
        // cbDTBC is the size in bytes of the TBC array
        // but the size of a TBC element is dynamic ( and this relates to TBDelta's
        int nStart = rS.Tell();

        int bytesRead = 0;
        int bytesToRead = cbDTBC - bytesRead;
        // cbDTBC specifies the size ( in bytes ) taken by an array ( of unspecified size )
        // of TBC records ( TBC records have dynamic length, so we need to check our position
        // after each read )
        do
        {
            TBC aTBC;
            if ( !aTBC.Read( rS ) )
                return false;
            rtbdc.push_back( aTBC );
            bytesToRead = cbDTBC - ( rS.Tell() - nStart );
        } while ( bytesToRead > 0 );
    }
    if ( static_cast< long >( rS.Tell() ) != nExpectedPos )
    {
        // Strange error condition, shouldn't happen ( but does in at least
        // one test document ) In the case where it happens the TBC &
        // TBCHeader records seem blank??? ( and incorrect )
        OSL_ENSURE( static_cast< long >(rS.Tell()) == nExpectedPos, "### Error: Expected pos not equal to actual pos after reading rtbdc");
        OSL_TRACE("\tPos now is 0x%x should be 0x%x", rS.Tell(), nExpectedPos );
        // seek to correct position after rtbdc
        rS.Seek( nExpectedPos );
    }
    if ( cCust )
    {
        for ( sal_Int32 index = 0; index < cCust; ++index )
        {
            Customization aCust( this );
            if ( !aCust.Read( rS ) )
                return false;
            rCustomizations.push_back( aCust );
        }
    }
    std::vector< sal_Int16 >::iterator it_end = dropDownMenuIndices.end();
    for ( std::vector< sal_Int16 >::iterator it = dropDownMenuIndices.begin(); it != it_end; ++it )
    {
        rCustomizations[ *it ].bIsDroppedMenuTB = true;
    }
    return true;
}

TBC* CTBWrapper::GetTBCAtOffset( sal_uInt32 nStreamOffset )
{
    for ( std::vector< TBC >::iterator it = rtbdc.begin(); it != rtbdc.end(); ++it )
    {
        if ( (*it).GetOffset() == nStreamOffset )
            return &(*it);
    }
    return NULL;
}

void CTBWrapper::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] CTBWrapper - dump\n", nOffSet );
    bool bRes = ( ch == 0x12 && reserved2 == 0x0 && reserved3 == 0x7 && reserved4 == 0x6 && reserved5 == 0xC );
    if ( bRes )
        indent_printf(fp,"  sanity check ( first 8 bytes conform )\n");
    else
    {
        indent_printf(fp,"    reserved1(0x%x)\n",ch);
        indent_printf(fp,"    reserved2(0x%x)\n",reserved2);
        indent_printf(fp,"    reserved3(0x%x)\n",reserved3);
        indent_printf(fp,"    reserved4(0x%x)\n",reserved4);
        indent_printf(fp,"    reserved5(0x%x)\n",reserved5);
        indent_printf(fp,"Quiting dump");
        return;
    }
    indent_printf(fp,"  size of TBDelta structures 0x%x\n", cbTBD );
    indent_printf(fp,"  cCust: no. of cCust structures 0x%x\n",cCust);
    indent_printf(fp,"  cbDTBC: no. of bytes in rtbdc array 0x%x\n", static_cast< unsigned int >( cbDTBC ));

    sal_Int32 index = 0;

    for ( std::vector< TBC >::iterator it = rtbdc.begin(); it != rtbdc.end(); ++it, ++index )
    {
        indent_printf(fp,"  Dumping rtbdc[%d]\n", static_cast< int >( index ));
        Indent b;
        it->Print( fp );
    }

    index = 0;

    for ( std::vector< Customization >::iterator it = rCustomizations.begin(); it != rCustomizations.end(); ++it, ++index )
    {
        indent_printf(fp,"  Dumping custimization [%d]\n", static_cast< int >( index ));
        Indent c;
        it->Print(fp);
    }
}

bool CTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    for ( std::vector< Customization >::iterator it = rCustomizations.begin(); it != rCustomizations.end(); ++it )
    {
        try
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( ui::ModuleUIConfigurationManagerSupplier::create(xContext) );
            CustomToolBarImportHelper helper( rDocSh, xAppCfgSupp->getUIConfigurationManager( "com.sun.star.text.TextDocument" ) );
            helper.setMSOCommandMap( new MSOWordCommandConvertor() );

            if ( !(*it).ImportCustomToolBar( *this, helper ) )
                return false;
        }
        catch(...)
        {
            continue;
        }
    }
    return false;
}

Customization::Customization( CTBWrapper* wrapper ) : tbidForTBD( 0 )
,reserved1( 0 )
, ctbds( 0 )
, pWrapper( wrapper )
, bIsDroppedMenuTB( false )
{
}

Customization::~Customization()
{
}

bool Customization::Read( SvStream &rS)
{
    OSL_TRACE("Custimization::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> tbidForTBD >> reserved1 >> ctbds;
    if ( tbidForTBD )
    {
        for ( sal_Int32 index = 0; index < ctbds; ++index )
        {
            TBDelta aTBDelta;
            if (!aTBDelta.Read( rS ) )
                return false;
            customizationDataTBDelta.push_back( aTBDelta );
            // Only set the drop down for menu's associated with standard toolbar
            if ( aTBDelta.ControlDropsToolBar() && tbidForTBD == 0x25 )
                pWrapper->InsertDropIndex( aTBDelta.CustomizationIndex() );
        }
    }
    else
    {
        customizationDataCTB.reset( new CTB() );
        if ( !customizationDataCTB->Read( rS ) )
                return false;
    }
    return true;
}

void Customization::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp,"[ 0x%x ] Customization -- dump \n", nOffSet );
    indent_printf( fp,"  tbidForTBD 0x%x ( should be 0 for CTBs )\n", static_cast< unsigned int >( tbidForTBD ));
    indent_printf( fp,"  reserved1 0x%x \n", reserved1);
    indent_printf( fp,"  ctbds - number of customisations %d(0x%x) \n", ctbds, ctbds );
    if ( !tbidForTBD && !ctbds )
        customizationDataCTB->Print( fp );
    else
    {
        const char* pToolBar = NULL;
        switch ( tbidForTBD )
        {
            case 0x9:
                pToolBar = "Standard";
                break;
            case 0x25:
                pToolBar = "Builtin-Menu";
                break;
            default:
                pToolBar = "Unknown toolbar";
                break;
        }

        indent_printf( fp,"  TBDelta(s) are associated with %s toolbar.\n", pToolBar);
        std::vector< TBDelta >::iterator it = customizationDataTBDelta.begin();
        for ( sal_Int32 index = 0; index < ctbds; ++it,++index )
            it->Print( fp );
    }

}

bool Customization::ImportMenu( CTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    if ( tbidForTBD == 0x25 )  // we can handle in a limited way additions the built-in menu bar
    {
        for ( std::vector< TBDelta >::iterator it = customizationDataTBDelta.begin(); it != customizationDataTBDelta.end(); ++it )
        {
            // for each new menu ( control that drops a toolbar )
            // import a toolbar
            if ( it->ControlIsInserted() && it->ControlDropsToolBar() )
            {
                Customization* pCust = pWrapper->GetCustomizaton( it->CustomizationIndex() );
                if ( pCust )
                {
                    // currently only support built-in menu
                    rtl::OUString sMenuBar( "private:resource/menubar/" );

                    sMenuBar = sMenuBar.concat( "menubar" );
                    // Get menu name
                    TBC* pTBC = pWrapper->GetTBCAtOffset( it->TBCStreamOffset() );
                    if ( !pTBC )
                        return false;
                    rtl::OUString sMenuName = pTBC->GetCustomText();
                    sMenuName = sMenuName.replace('&','~');

                    // see if the document has already setting for the menubar

                    uno::Reference< container::XIndexContainer > xIndexContainer;
                    bool bHasSettings = false;
                    if ( helper.getCfgManager()->hasSettings( sMenuBar ) )
                    {
                        xIndexContainer.set( helper.getCfgManager()->getSettings( sMenuBar, sal_True ), uno::UNO_QUERY_THROW );
                        bHasSettings = true;
                    }
                    else
                    {
                        if ( helper.getAppCfgManager()->hasSettings( sMenuBar ) )
                            xIndexContainer.set( helper.getAppCfgManager()->getSettings( sMenuBar, sal_True ), uno::UNO_QUERY_THROW );
                        else
                            xIndexContainer.set( helper.getAppCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
                    }

                    uno::Reference< lang::XSingleComponentFactory > xSCF( xIndexContainer, uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                    uno::Reference< uno::XComponentContext > xContext(  xProps->getPropertyValue( "DefaultContext" ), uno::UNO_QUERY_THROW );
                    // create the popup menu
                    uno::Sequence< beans::PropertyValue > aPopupMenu( 4 );
                    aPopupMenu[0].Name = "CommandURL";
                    aPopupMenu[0].Value = uno::makeAny( rtl::OUString( "vnd.openoffice.org:" ) + sMenuName );
                    aPopupMenu[1].Name = "Label";
                    aPopupMenu[1].Value <<= sMenuName;
                    aPopupMenu[2].Name = "Type";
                    aPopupMenu[2].Value <<= sal_Int32( 0 );
                    aPopupMenu[3].Name = "ItemDescriptorContainer";
                    uno::Reference< container::XIndexContainer > xMenuContainer( xSCF->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                    aPopupMenu[3].Value <<= xMenuContainer;
                    if ( pCust->customizationDataCTB.get() && !pCust->customizationDataCTB->ImportMenuTB( rWrapper, xMenuContainer, helper ) )
                        return false;
                    OSL_TRACE("** there are %d menu items on the bar, inserting after that", xIndexContainer->getCount() );
                    xIndexContainer->insertByIndex( xIndexContainer->getCount(), uno::makeAny( aPopupMenu ) );

                    if ( bHasSettings )
                        helper.getCfgManager()->replaceSettings( sMenuBar, uno::Reference< container::XIndexAccess >( xIndexContainer, uno::UNO_QUERY_THROW ) );
                    else
                        helper.getCfgManager()->insertSettings( sMenuBar, uno::Reference< container::XIndexAccess >( xIndexContainer, uno::UNO_QUERY_THROW ) );
        uno::Reference< ui::XUIConfigurationPersistence > xPersistence( helper.getCfgManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();
                }
            }
        }
        return true;
    }
    return true;
}

bool Customization::ImportCustomToolBar( CTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    if ( GetTBIDForTB() == 0x25 )
        return ImportMenu( rWrapper, helper );
    if ( !customizationDataCTB.get() )
        return false;
    if ( !customizationDataCTB->IsMenuToolbar() )
    {
        if ( !customizationDataCTB->ImportCustomToolBar( rWrapper, helper ) )
            return false;
    }
    return true;
}

TBDelta::TBDelta() : doprfatendFlags(0)
,ibts(0)
,cidNext(0)
,cid(0)
,fc(0)
,cbTBC(0)
{
}

bool TBDelta::ControlIsModified()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x2 );
}

bool TBDelta::ControlIsInserted()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlIsChanged()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlDropsToolBar()
{
    return !( CiTBDE & 0x8000 );
}

sal_Int32 TBDelta::TBCStreamOffset()
{
    return fc;
}

sal_Int16 TBDelta::CustomizationIndex()
{
    sal_Int16 nIndex = CiTBDE;
    nIndex = nIndex >> 1;
    nIndex &= 0x1ff; // only 13 bits are relevant
    return nIndex;
}

bool TBDelta::Read(SvStream &rS)
{
    OSL_TRACE("TBDelta::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> doprfatendFlags >> ibts >> cidNext >> cid >> fc ;
    rS >> CiTBDE >> cbTBC;
    return true;
}

void TBDelta::Print( FILE* fp )
{
    // Like most of the debug output, it's raw and little ( no )
    // interpretation of the data is output ( e.g. flag values etc. )
    indent_printf( fp, "[ 0x%x ] TBDelta -- dump\n", nOffSet );
    indent_printf( fp, " doprfatendFlags 0x%x\n",doprfatendFlags );

    indent_printf( fp, " ibts 0x%x\n",ibts );
    indent_printf( fp, " cidNext 0x%x\n", static_cast< unsigned int >( cidNext ) );
    indent_printf( fp, " cid 0x%x\n", static_cast< unsigned int >( cid ) );
    indent_printf( fp, " fc 0x%x\n", static_cast< unsigned int >( fc ) );
    indent_printf( fp, " CiTBDE 0x%x\n",CiTBDE );
    indent_printf( fp, " cbTBC 0x%x\n", cbTBC );
    if ( ControlDropsToolBar() )
    {
        indent_printf( fp, " this delta is associated with a control that drops a menu toolbar\n", cbTBC );
        indent_printf( fp, " the menu toolbar drops the toolbar defined at index[%d] in the rCustomizations array of the CTBWRAPPER that contains this TBDelta\n", CustomizationIndex() );
    }
}

CTB::CTB() : cbTBData( 0 )
,iWCTBl( 0 )
,reserved( 0 )
,unused( 0 )
,cCtls( 0 )
{
}

CTB::~CTB()
{
}

bool CTB::IsMenuToolbar()
{
    return tb.IsMenuToolbar();
}


bool CTB::Read( SvStream &rS)
{
    OSL_TRACE("CTB::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( !name.Read( rS ) )
        return false;
    rS >> cbTBData;
    if ( !tb.Read( rS ) )
        return false;
    for ( short index = 0; index < nVisualData; ++index )
    {
        TBVisualData aVisData;
        aVisData.Read( rS );
        rVisualData.push_back( aVisData );
    }

    rS >> iWCTBl >> reserved >> unused >> cCtls;

    if ( cCtls )
    {
        for ( sal_Int32 index = 0; index < cCtls; ++index )
        {
            TBC aTBC;
            if ( !aTBC.Read( rS ) )
                return false;
            rTBC.push_back( aTBC );
        }
    }
    return true;
}

void
CTB::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] CTB - dump\n", nOffSet );
    indent_printf(fp, "  name %s\n", rtl::OUStringToOString( name.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf(fp, "  cbTBData size, in bytes, of this structure excluding the name, cCtls, and rTBC fields.  %x\n", static_cast< unsigned int >( cbTBData ) );

    tb.Print(fp);
    for ( short counter = 0; counter < nVisualData; ++counter )
    {
        indent_printf( fp, "  TBVisualData [%d]\n", counter);
        Indent b;
        rVisualData[ counter ].Print( fp );
    }
    indent_printf(fp, "  iWCTBl 0x%x reserved 0x%x unused 0x%x cCtls( toolbar controls ) 0x%x \n", static_cast< unsigned int >( iWCTBl ), reserved, unused, static_cast< unsigned int >( cCtls ) );
    if ( cCtls )
    {
        for ( sal_Int32 index = 0; index < cCtls; ++index )
        {

            indent_printf(fp, "  dumping toolbar control 0x%x\n", static_cast< unsigned int >( index ) );
            rTBC[ index ].Print( fp );
        }
    }
}

bool CTB::ImportCustomToolBar( CTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    static rtl::OUString sToolbarPrefix( "private:resource/toolbar/custom_" );
    bool bRes = false;
    try
    {
        if ( !tb.IsEnabled() )
            return true;  // didn't fail, just ignoring
        // Create default setting
        uno::Reference< container::XIndexContainer > xIndexContainer( helper.getCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xIndexContainer, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xIndexContainer, uno::UNO_QUERY_THROW );

        // set UI name for toolbar
        xProps->setPropertyValue( "UIName", uno::makeAny( name.getString() ) );

        rtl::OUString sToolBarName = sToolbarPrefix.concat( name.getString() );
        for ( std::vector< TBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it )
        {
            // createToolBar item for control
            if ( !it->ImportToolBarControl( rWrapper, xIndexContainer, helper, IsMenuToolbar() ) )
                return false;
        }

        OSL_TRACE("Name of toolbar :-/ %s", rtl::OUStringToOString( sToolBarName, RTL_TEXTENCODING_UTF8 ).getStr() );

        helper.getCfgManager()->insertSettings( sToolBarName, xIndexAccess );
        helper.applyIcons();
#if 1 // don't think this is necessary
        uno::Reference< ui::XUIConfigurationPersistence > xPersistence( helper.getCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();

        xPersistence.set( helper.getCfgManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();
#endif
        bRes = true;
    }
    catch( const uno::Exception& e )
    {
        OSL_TRACE("***** For some reason we have an exception %s", rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        bRes = false;
    }
    return bRes;
}

bool CTB::ImportMenuTB( CTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& xIndexContainer, CustomToolBarImportHelper& rHelper )
{
    for ( std::vector< TBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it )
    {
        // createToolBar item for control
        if ( !it->ImportToolBarControl( rWrapper, xIndexContainer, rHelper, true ) )
            return false;
    }
    return true;
}

TBC::TBC()
{
}

TBC::~TBC()
{
}

bool TBC::Read( SvStream &rS )
{
    OSL_TRACE("TBC::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( !tbch.Read( rS ) )
        return false;
    if ( tbch.getTcID() != 0x1 && tbch.getTcID() != 0x1051 )
    {
        cid.reset( new sal_uInt32 );
        rS >> *cid;
    }
    // MUST exist if tbch.tct is not equal to 0x16
    if ( tbch.getTct() != 0x16 )
    {
        tbcd.reset(  new TBCData( tbch ) );
        if ( !tbcd->Read( rS ) )
            return false;
    }
    return true;
}

void TBC::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBC -- dump\n", nOffSet );
    indent_printf(fp,"  dumping header ( TBCHeader )\n");
    tbch.Print( fp );
    if ( cid.get() )
        indent_printf(fp,"  cid = 0x%x\n", static_cast< unsigned int >( *cid ) );
    if ( tbcd.get() )
    {
        indent_printf(fp,"  dumping toolbar data TBCData \n");
        tbcd->Print(fp);
    }
}

bool
TBC::ImportToolBarControl( CTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuBar )
{
    // cmtFci       0x1 Command based on a built-in command. See CidFci.
    // cmtMacro     0x2 Macro command. See CidMacro.
    // cmtAllocated 0x3 Allocated command. See CidAllocated.
    // cmtNil       0x7 No command. See Cid.
    bool bBuiltin = false;
    sal_uInt16 cmdId = 0;
    if  ( cid.get() )
    {
        sal_uInt16 arg2 = ( *( cid.get() ) & 0xFFFF );

        sal_uInt8 cmt = ( arg2 & 0x7 );
        arg2 = ( arg2 >> 3 );

        switch ( cmt )
        {
            case 1:
                OSL_TRACE("cmt is cmtFci builtin command 0x%x", arg2);
                bBuiltin = true;
                cmdId = arg2;
                break;
            case 2:
                OSL_TRACE("cmt is cmtMacro macro 0x%x", arg2);
                break;
            case 3:
                OSL_TRACE("cmt is cmtAllocated [???] 0x%x", arg2);
                break;
            case 7:
                OSL_TRACE("cmt is cmNill no-phing 0x%x", arg2);
                break;
            default:
                OSL_TRACE("illegal 0x%x", cmt);
                break;
        }
    }

    if ( tbcd.get() )
    {
        std::vector< css::beans::PropertyValue > props;
        if ( bBuiltin )
        {
            rtl::OUString sCommand = helper.MSOCommandToOOCommand( cmdId );
            if ( !sCommand.isEmpty() )
            {
                beans::PropertyValue aProp;

                aProp.Name = "CommandURL";
                aProp.Value <<= sCommand;
                props.push_back( aProp );
            }
        }
        bool bBeginGroup = false;
        if ( ! tbcd->ImportToolBarControl( helper, props, bBeginGroup, bIsMenuBar ) )
            return false;

        TBCMenuSpecific* pMenu = tbcd->getMenuSpecific();
        if ( pMenu )
        {
            OSL_TRACE("** control has a menu, name of toolbar with menu items is %s", rtl::OUStringToOString( pMenu->Name(), RTL_TEXTENCODING_UTF8 ).getStr() );
            // search for CTB with the appropriate name ( it contains the
            // menu items, although we cannot import ( or create ) a menu on
            // a custom toolbar we can import the menu items in a separate
            // toolbar ( better than nothing )
            CTB* pCustTB = rWrapper.GetCustomizationData( pMenu->Name() );
            if ( pCustTB )
            {
                 uno::Reference< container::XIndexContainer > xMenuDesc;
                 uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                 xMenuDesc.set( xMSF->createInstance( "com.sun.star.document.IndexedPropertyValues" ), uno::UNO_QUERY_THROW );
                if ( !pCustTB->ImportMenuTB( rWrapper,xMenuDesc, helper ) )
                    return false;
                if ( !bIsMenuBar )
                {
                    if ( !helper.createMenu( pMenu->Name(), uno::Reference< container::XIndexAccess >( xMenuDesc, uno::UNO_QUERY ), true ) )
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

rtl::OUString
TBC::GetCustomText()
{
    rtl::OUString sCustomText;
    if ( tbcd.get() )
        sCustomText = tbcd->getGeneralInfo().CustomText();
    return sCustomText;


}

bool
Xst::Read( SvStream& rS )
{
    OSL_TRACE("Xst::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    sString = read_uInt16_PascalString(rS);
    return true;
}

void
Xst::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] Xst -- dump\n", nOffSet );
    indent_printf( fp, " %s",  rtl::OUStringToOString( sString, RTL_TEXTENCODING_UTF8 ).getStr() );
}

Tcg::Tcg() : nTcgVer( -1 )
{
}

bool Tcg::Read(SvStream &rS)
{
    OSL_TRACE("Tcg::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> nTcgVer;
    if ( nTcgVer != -1 )
        return false;
    tcg.reset( new Tcg255() );
    return tcg->Read( rS );
}

void Tcg::Print( FILE* fp )
{
    Indent a(true);
    indent_printf(fp, "[ 0x%x ] Tcg - dump %d\n", nOffSet, nTcgVer);
    indent_printf(fp,"  nTcgVer %d\n", nTcgVer);
    if ( tcg.get() )
        tcg->Print( fp );
}

bool Tcg::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    if ( tcg.get() )
        return tcg->ImportCustomToolBar( rDocSh );
    return false;
}

Tcg255::Tcg255()
{
}

Tcg255::~Tcg255()
{
    std::vector< Tcg255SubStruct* >::iterator it = rgtcgData.begin();
    for ( ; it != rgtcgData.end(); ++it )
        delete *it;
}

bool Tcg255::processSubStruct( sal_uInt8 nId, SvStream &rS )
{
     Tcg255SubStruct* pSubStruct = NULL;
     switch ( nId )
     {
         case 0x1:
         {
             pSubStruct = new PlfMcd( false ); // don't read the id
             break;
         }
         case 0x2:
         {
             pSubStruct = new PlfAcd( false );
             break;
         }
         case 0x3:
         case 0x4:
         {
             pSubStruct = new PlfKme( false );
             break;
         }
         case 0x10:
         {
             pSubStruct = new TcgSttbf( false );
             break;
         }
         case 0x11:
         {
             pSubStruct = new MacroNames( false );
             break;
         }
         case 0x12:
         {
             pSubStruct = new CTBWrapper( false );
             break;
         }
         default:
             OSL_TRACE("Unknown id 0x%x",nId);
             return false;
    }
    pSubStruct->ch = nId;
    if ( !pSubStruct->Read( rS ) )
        return false;
    rgtcgData.push_back( pSubStruct );
    return true;
}

bool Tcg255::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    // Find the CTBWrapper
    for ( std::vector< Tcg255SubStruct* >::const_iterator it = rgtcgData.begin(); it != rgtcgData.end(); ++it )
    {
        if ( (*it)->id() == 0x12 )
        {
            // not so great, shouldn't really have to do a horror casting
            CTBWrapper* pCTBWrapper =  dynamic_cast< CTBWrapper* > ( *it );
            if ( pCTBWrapper )
            {
                if ( !pCTBWrapper->ImportCustomToolBar( rDocSh ) )
                    return false;
            }
        }
    }
    return true;
}


bool Tcg255::Read(SvStream &rS)
{
    OSL_TRACE("Tcg255::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    sal_uInt8 nId = 0x40;
    rS >> nId;
    while (  nId != 0x40  )
    {
        if ( !processSubStruct( nId, rS ) )
            return false;
        nId = 0x40;
        rS >> nId;
    }
    return true;
    // Peek at
}

void Tcg255::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] Tcg255 - dump\n", nOffSet );
    indent_printf(fp, "  contains %d sub records\n", rgtcgData.size() );
    std::vector< Tcg255SubStruct* >::iterator it = rgtcgData.begin();
    std::vector< Tcg255SubStruct* >::iterator it_end = rgtcgData.end();

    for( sal_Int32 count = 1; it != it_end ; ++it, ++count )
    {
        Indent b;
        indent_printf(fp, "  [%d] Tcg255SubStruct \n", static_cast< unsigned int >( count ) );
        (*it)->Print(fp);
    }
}


Tcg255SubStruct::Tcg255SubStruct( bool bReadId ) : mbReadId( bReadId ), ch(0)
{
}

bool Tcg255SubStruct::Read(SvStream &rS)
{
    OSL_TRACE("Tcg255SubStruct::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( mbReadId )
        rS >> ch;
    return true;
}

PlfMcd::PlfMcd( bool bReadId ): Tcg255SubStruct( bReadId ), iMac(0), rgmcd( NULL )
{
}

PlfMcd::~PlfMcd()
{
    if ( rgmcd )
        delete[] rgmcd;
}

bool PlfMcd::Read(SvStream &rS)
{
    OSL_TRACE("PffMcd::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS >> iMac;
    if ( iMac )
    {
        rgmcd = new MCD[ iMac ];
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgmcd[ index ].Read( rS ) )
                return false;
        }
    }
    return true;
}

void PlfMcd::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfMcd ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d MCD records\n", static_cast<int>( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] MCD\n", static_cast< int >( count ) );
        rgmcd[ count ].Print( fp );
    }

}

PlfAcd::PlfAcd( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac(0)
,rgacd(NULL)
{
}


PlfAcd::~PlfAcd()
{
    if ( rgacd )
        delete[] rgacd;
}

bool PlfAcd::Read( SvStream &rS)
{
    OSL_TRACE("PffAcd::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS >> iMac;
    if ( iMac )
    {
        rgacd = new Acd[ iMac ];
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgacd[ index ].Read( rS ) )
                return false;
        }
    }
    return true;
}
void PlfAcd::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfAcd ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d ACD records\n", static_cast< int >( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] ACD\n", static_cast< int >( count ) );
        rgacd[ count ].Print( fp );
    }

}

PlfKme::PlfKme( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac( 0 )
,rgkme( NULL )
{
}

PlfKme::~PlfKme()
{
    if ( rgkme )
        delete[] rgkme;
}

bool PlfKme::Read(SvStream &rS)
{
    OSL_TRACE("PlfKme::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS >> iMac;
    if ( iMac )
    {
        rgkme = new Kme[ iMac ];
        for( sal_Int32 index=0; index<iMac; ++index )
        {
            if ( !rgkme[ index ].Read( rS ) )
                return false;
        }
    }
    return true;
}

void PlfKme::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfKme ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d Kme records\n", static_cast< int >( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] Kme\n", static_cast< int >( count ) );
        rgkme[ count ].Print( fp );
    }

}

TcgSttbf::TcgSttbf( bool bReadId ) : Tcg255SubStruct( bReadId )
{
}

bool TcgSttbf::Read( SvStream &rS)
{
    OSL_TRACE("TcgSttbf::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    return sttbf.Read( rS );
}

void TcgSttbf::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TcgSttbf - dump\n", nOffSet );
    sttbf.Print( fp );
}

TcgSttbfCore::TcgSttbfCore() : fExtend( 0 )
,cData( 0 )
,cbExtra( 0 )
,dataItems( NULL )
{
}

TcgSttbfCore::~TcgSttbfCore()
{
    if ( dataItems )
        delete[] dataItems;
}

bool TcgSttbfCore::Read( SvStream& rS )
{
    OSL_TRACE("TcgSttbfCore::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> fExtend >> cData >> cbExtra;
    if ( cData )
    {
        dataItems = new SBBItem[ cData ];
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            rS >> dataItems[ index ].cchData;
            dataItems[ index ].data = read_uInt16s_ToOUString(rS, dataItems[index].cchData);
            rS >> dataItems[ index ].extraData;
        }
    }
    return true;
}

void TcgSttbfCore::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TcgSttbfCore - dump\n");
    indent_printf( fp, " fExtend 0x%x [expected 0xFFFF ]\n", fExtend );
    indent_printf( fp, " cbExtra 0x%x [expected 0x02 ]\n", cbExtra );
    indent_printf( fp, " cData no. or string data items %d (0x%x)\n", cData, cData );

    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
            indent_printf(fp,"   string dataItem[ %d(0x%x) ] has name %s and if referenced %d times.\n", static_cast< int >( index ), static_cast< unsigned int >( index ), rtl::OUStringToOString( dataItems[ index ].data, RTL_TEXTENCODING_UTF8 ).getStr(), dataItems[ index ].extraData );
    }

}
MacroNames::MacroNames( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac( 0 )
,rgNames( NULL )
{
}

MacroNames::~MacroNames()
{
    if ( rgNames )
        delete[] rgNames;
}

bool MacroNames::Read( SvStream &rS)
{
    OSL_TRACE("MacroNames::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS >> iMac;
    if ( iMac )
    {
        rgNames = new MacroName[ iMac ];
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgNames[ index ].Read( rS ) )
                return false;
        }
    }
    return true;
}

void MacroNames::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] MacroNames ( Tcg255SubStruct ) - dump\n");
    indent_printf(fp, " contains %d MacroName records\n", iMac );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] MacroName\n", static_cast<int>( count ) );
        rgNames[ count ].Print( fp );
    }

}

MacroName::MacroName():ibst(0)
{
}


bool MacroName::Read(SvStream &rS)
{
    OSL_TRACE("MacroName::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> ibst;
    return xstz.Read( rS );
}

void MacroName::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] MacroName - dump");
    indent_printf( fp,"  index - 0x%x has associated following record\n", ibst );
    xstz.Print( fp );
}



Xstz::Xstz():chTerm(0)
{
}

bool
Xstz::Read(SvStream &rS)
{
    OSL_TRACE("Xstz::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    if ( !xst.Read( rS ) )
        return false;
    rS >> chTerm;
    if ( chTerm != 0 ) // should be an assert
        return false;
    return true;
}

void Xstz::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] Xstz -- dump\n", nOffSet );
    indent_printf(fp,"  Xst\n");
    xst.Print( fp );
    indent_printf(fp,"  chterm 0x%x ( should be zero )\n", chTerm);
}

Kme::Kme() : reserved1(0)
,reserved2(0)
,kcm1(0)
,kcm2(0)
,kt(0)
,param(0)
{
}

Kme::~Kme()
{
}

bool
Kme::Read(SvStream &rS)
{
    OSL_TRACE("Kme::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> reserved1 >> reserved2 >> kcm1 >> kcm2 >> kt >> param;
    return true;
}

void Kme::Print( FILE* fp )
{
    Indent a;

   indent_printf( fp, "[ 0x%x ] Kme - dump\n", nOffSet );
   indent_printf( fp, " reserved1 0x%x [expected 0x0 ]\n", reserved1 );
   indent_printf( fp, " reserved2 0x%x [expected 0x0 ]\n", reserved2 );
   indent_printf( fp, " kcm1 0x%x [shortcut key]\n", kcm1 );
   indent_printf( fp, " kcm2 0x%x [shortcut key]\n", kcm2 );
   indent_printf( fp, " kt 0x%x \n", kt );
   indent_printf( fp, " param 0x%x \n", static_cast< unsigned int >( param ) );
}

Acd::Acd() : ibst( 0 )
, fciBasedOnABC( 0 )
{
}

bool Acd::Read(SvStream &rS)
{
    OSL_TRACE("Acd::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> ibst >> fciBasedOnABC;
    return true;
}

void Acd::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp,"[ 0x%x ] ACD - dump\n", nOffSet );
    // #TODO flesh out interpretation of these values
    indent_printf( fp,"  ibst 0x%x\n", ibst);
    indent_printf( fp,"  fciBaseObABC 0x%x\n", fciBasedOnABC);
}

MCD::MCD() :  reserved1(0x56)
,reserved2( 0 )
,ibst( 0 )
,ibstName( 0 )
,reserved3( 0xFFFF )
,reserved4( 0 )
,reserved5( 0 )
,reserved6( 0 )
,reserved7( 0 )
{
}

bool  MCD::Read(SvStream &rS)
{
    OSL_TRACE("MCD::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> reserved1 >> reserved2 >> ibst >> ibstName >> reserved3;
    rS >> reserved4 >> reserved5 >> reserved6 >> reserved7;
    return true;
}

void MCD::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] MCD - dump\n", nOffSet );
    indent_printf( fp, " reserved1 0x%x [expected 0x56 ]\n", reserved1 );
    indent_printf( fp, " reserved2 0x%x [expected 0x0 ]\n", reserved2 );
    indent_printf( fp, " ibst 0x%x specifies macro with MacroName.xstz = 0x%x\n", ibst, ibst );
    indent_printf( fp, " ibstName 0x%x index into command string table ( TcgSttbf.sttbf )\n", ibstName );

    indent_printf( fp, " reserved3 0x%x [expected 0xFFFF ]\n", reserved3 );
    indent_printf( fp, " reserved4 0x%x\n", static_cast< unsigned int >( reserved4 ) );
    indent_printf( fp, " reserved5 0x%x [expected 0x0 ]\n", static_cast< unsigned int >( reserved5 ) );
    indent_printf( fp, " reserved6 0x%x\n", static_cast< unsigned int >( reserved6 ) );
    indent_printf( fp, " reserved7 0x%x\n", static_cast< unsigned int >( reserved7 ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
