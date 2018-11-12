/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ww8toolbar.hxx"
#include "ww8scan.hxx"
#include <rtl/ustrbuf.hxx>
#include <stdarg.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <fstream>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/graph.hxx>
#include <map>
#include <sal/log.hxx>

using namespace com::sun::star;

// no. of visual data elements in a SwCTB ( fixed )
const short nVisualData = 5;

typedef std::map< sal_Int16, OUString > IdToString;

class MSOWordCommandConvertor : public MSOCommandConvertor
{
   IdToString msoToOOcmd;
   IdToString tcidToOOcmd;

public:
    MSOWordCommandConvertor();
    virtual OUString MSOCommandToOOCommand( sal_Int16 msoCmd ) override;
    virtual OUString MSOTCIDToOOCommand( sal_Int16 key ) override;
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

OUString MSOWordCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    IdToString::iterator it = msoToOOcmd.find( key );
    if ( it != msoToOOcmd.end() )
        return it->second;
    return OUString();
}

OUString MSOWordCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    IdToString::iterator it = tcidToOOcmd.find( key );
    if ( it != tcidToOOcmd.end() )
        return it->second;
    return OUString();
}

SwCTBWrapper::SwCTBWrapper() :
reserved2(0)
,reserved3(0)
,reserved4(0)
,reserved5(0)
,cbTBD(0)
,cCust(0)
,cbDTBC(0)
,rtbdc(0)
{
}

SwCTBWrapper::~SwCTBWrapper()
{
}

Customization* SwCTBWrapper::GetCustomizaton( sal_Int16 index )
{
    if ( index < 0 || index >= static_cast<sal_Int16>( rCustomizations.size() ) )
        return nullptr;
    return &rCustomizations[ index ];
}

SwCTB* SwCTBWrapper::GetCustomizationData( const OUString& sTBName )
{
    auto it = std::find_if(rCustomizations.begin(), rCustomizations.end(),
        [&sTBName](Customization& rCustomization) {
            SwCTB* pCTB = rCustomization.GetCustomizationData();
            return pCTB && pCTB->GetName() == sTBName;
        });
    if (it != rCustomizations.end())
        return it->GetCustomizationData();
    return nullptr;
}

bool SwCTBWrapper::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","SwCTBWrapper::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadUInt16( reserved2 ).ReadUChar( reserved3 ).ReadUInt16( reserved4 ).ReadUInt16( reserved5 );
    rS.ReadInt16( cbTBD ).ReadUInt16( cCust ).ReadInt32( cbDTBC );
    long nExpectedPos =  rS.Tell() + cbDTBC;
    if ( cbDTBC )
    {
        // cbDTBC is the size in bytes of the SwTBC array
        // but the size of a SwTBC element is dynamic ( and this relates to TBDelta's
        int nStart = rS.Tell();

        int bytesToRead;
        // cbDTBC specifies the size ( in bytes ) taken by an array ( of unspecified size )
        // of SwTBC records ( SwTBC records have dynamic length, so we need to check our position
        // after each read )
        do
        {
            SwTBC aTBC;
            if ( !aTBC.Read( rS ) )
                return false;
            rtbdc.push_back( aTBC );
            bytesToRead = cbDTBC - ( rS.Tell() - nStart );
        } while ( bytesToRead > 0 );
    }
    if ( static_cast< long >( rS.Tell() ) != nExpectedPos )
    {
        // Strange error condition, shouldn't happen ( but does in at least
        // one test document ) In the case where it happens the SwTBC &
        // TBCHeader records seem blank??? ( and incorrect )
        SAL_WARN_IF( static_cast< long >(rS.Tell()) != nExpectedPos, "sw.ww8","### Error: Expected pos not equal to actual pos after reading rtbdc");
        SAL_INFO("sw.ww8","\tPos now is 0x" << std::hex << rS.Tell() << " should be 0x" << std::hex << nExpectedPos );
        // seek to correct position after rtbdc
        rS.Seek( nExpectedPos );
    }
    if (cCust)
    {
        //Each customization takes a min of 8 bytes
        size_t nMaxPossibleRecords = rS.remainingSize() / 8;
        if (cCust > nMaxPossibleRecords)
        {
            return false;
        }
        for (sal_uInt16 index = 0; index < cCust; ++index)
        {
            Customization aCust( this );
            if ( !aCust.Read( rS ) )
                return false;
            rCustomizations.push_back( aCust );
        }
    }
    for ( const auto& rIndex : dropDownMenuIndices )
    {
        if (rIndex < 0 || static_cast<size_t>(rIndex) >= rCustomizations.size())
            continue;
        rCustomizations[rIndex].bIsDroppedMenuTB = true;
    }
    return rS.good();
}

SwTBC* SwCTBWrapper::GetTBCAtOffset( sal_uInt32 nStreamOffset )
{
    auto it = std::find_if(rtbdc.begin(), rtbdc.end(),
        [&nStreamOffset](SwTBC& rItem) { return rItem.GetOffset() == nStreamOffset; });
    if ( it != rtbdc.end() )
        return &(*it);
    return nullptr;
}

bool SwCTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    for ( auto& rCustomization : rCustomizations )
    {
        try
        {
            css::uno::Reference<css::ui::XUIConfigurationManager> xCfgMgr;
            if (!utl::ConfigManager::IsFuzzing())
            {
                uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( ui::theModuleUIConfigurationManagerSupplier::get(xContext) );
                xCfgMgr = xAppCfgSupp->getUIConfigurationManager("com.sun.star.text.TextDocument");
            }
            CustomToolBarImportHelper helper(rDocSh, xCfgMgr);
            helper.setMSOCommandMap( new MSOWordCommandConvertor() );

            if ( !rCustomization.ImportCustomToolBar( *this, helper ) )
                return false;
        }
        catch (...)
        {
            continue;
        }
    }
    return true;
}

Customization::Customization( SwCTBWrapper* wrapper )
    : tbidForTBD( 0 )
    , reserved1( 0 )
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
    SAL_INFO("sw.ww8","Customization::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadInt32( tbidForTBD ).ReadUInt16( reserved1 ).ReadUInt16( ctbds );
    if ( tbidForTBD )
    {
        //each TBDelta is at least 18 bytes in size
        size_t nMaxAvailableRecords = rS.remainingSize() / 18;
        if (ctbds > nMaxAvailableRecords)
            return false;
        for (sal_uInt16 index = 0; index < ctbds; ++index)
        {
            TBDelta aTBDelta;
            if (!aTBDelta.Read( rS ) )
                return false;
            customizationDataTBDelta.push_back( aTBDelta );
            // Only set the drop down for menus associated with standard toolbar
            if ( aTBDelta.ControlDropsToolBar() && tbidForTBD == 0x25 )
                pWrapper->InsertDropIndex( aTBDelta.CustomizationIndex() );
        }
    }
    else
    {
        customizationDataCTB.reset( new SwCTB() );
        if ( !customizationDataCTB->Read( rS ) )
                return false;
    }
    return rS.good();
}

bool Customization::ImportMenu( SwCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    if ( tbidForTBD == 0x25 )  // we can handle in a limited way additions the built-in menu bar
    {
        for ( auto& rTBDelta : customizationDataTBDelta )
        {
            // for each new menu ( control that drops a toolbar )
            // import a toolbar
            if ( rTBDelta.ControlIsInserted() && rTBDelta.ControlDropsToolBar() )
            {
                Customization* pCust = pWrapper->GetCustomizaton( rTBDelta.CustomizationIndex() );
                if ( pCust )
                {
                    // currently only support built-in menu
                    const OUString sMenuBar( "private:resource/menubar/menubar" );

                    // Get menu name
                    SwTBC* pTBC = pWrapper->GetTBCAtOffset( rTBDelta.TBCStreamOffset() );
                    if ( !pTBC )
                        return false;
                    const OUString sMenuName = pTBC->GetCustomText().replace('&','~');

                    // see if the document has already setting for the menubar

                    uno::Reference< container::XIndexContainer > xIndexContainer;
                    bool bHasSettings = false;
                    if ( helper.getCfgManager()->hasSettings( sMenuBar ) )
                    {
                        xIndexContainer.set( helper.getCfgManager()->getSettings( sMenuBar, true ), uno::UNO_QUERY_THROW );
                        bHasSettings = true;
                    }
                    else
                    {
                        if ( helper.getAppCfgManager()->hasSettings( sMenuBar ) )
                            xIndexContainer.set( helper.getAppCfgManager()->getSettings( sMenuBar, true ), uno::UNO_QUERY_THROW );
                        else
                            xIndexContainer.set( helper.getAppCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
                    }

                    uno::Reference< lang::XSingleComponentFactory > xSCF( xIndexContainer, uno::UNO_QUERY_THROW );
                    uno::Reference< uno::XComponentContext > xContext(
                        comphelper::getProcessComponentContext() );
                    // create the popup menu
                    uno::Sequence< beans::PropertyValue > aPopupMenu( 4 );
                    aPopupMenu[0].Name = "CommandURL";
                    aPopupMenu[0].Value <<= "vnd.openoffice.org:" + sMenuName;
                    aPopupMenu[1].Name = "Label";
                    aPopupMenu[1].Value <<= sMenuName;
                    aPopupMenu[2].Name = "Type";
                    aPopupMenu[2].Value <<= sal_Int32( 0 );
                    aPopupMenu[3].Name = "ItemDescriptorContainer";
                    uno::Reference< container::XIndexContainer > xMenuContainer( xSCF->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                    aPopupMenu[3].Value <<= xMenuContainer;
                    if ( pCust->customizationDataCTB.get() && !pCust->customizationDataCTB->ImportMenuTB( rWrapper, xMenuContainer, helper ) )
                        return false;
                    SAL_INFO("sw.ww8","** there are " << xIndexContainer->getCount() << " menu items on the bar, inserting after that");
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

bool Customization::ImportCustomToolBar( SwCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    if ( tbidForTBD == 0x25 )
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

TBDelta::TBDelta()
    : doprfatendFlags(0)
    , ibts(0)
    , cidNext(0)
    , cid(0)
    , fc(0)
    , CiTBDE(0)
    , cbTBC(0)
{
}

bool TBDelta::ControlIsInserted()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlDropsToolBar()
{
    return !( CiTBDE & 0x8000 );
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
    SAL_INFO("sw.ww8","TBDelta::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUChar( doprfatendFlags ).ReadUChar( ibts ).ReadInt32( cidNext ).ReadInt32( cid ).ReadInt32( fc ) ;
    rS.ReadUInt16( CiTBDE ).ReadUInt16( cbTBC );
    return rS.good();
}

SwCTB::SwCTB() : cbTBData( 0 )
,iWCTBl( 0 )
,reserved( 0 )
,unused( 0 )
,cCtls( 0 )
{
}

SwCTB::~SwCTB()
{
}

bool SwCTB::IsMenuToolbar()
{
    return tb.IsMenuToolbar();
}

bool SwCTB::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","SwCTB::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !name.Read( rS ) )
        return false;
    rS.ReadInt32( cbTBData );
    if ( !tb.Read( rS ) )
        return false;
    for ( short index = 0; index < nVisualData; ++index )
    {
        TBVisualData aVisData;
        aVisData.Read( rS );
        rVisualData.push_back( aVisData );
    }

    rS.ReadInt32( iWCTBl ).ReadUInt16( reserved ).ReadUInt16( unused ).ReadInt32( cCtls );

    if ( cCtls )
    {
        for ( sal_Int32 index = 0; index < cCtls; ++index )
        {
            SwTBC aTBC;
            if ( !aTBC.Read( rS ) )
                return false;
            rTBC.push_back( aTBC );
        }
    }
    return rS.good();
}

bool SwCTB::ImportCustomToolBar( SwCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
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

        // set UI name for toolbar
        xProps->setPropertyValue( "UIName", uno::makeAny( name.getString() ) );

        const OUString sToolBarName = "private:resource/toolbar/custom_" + name.getString();
        for ( auto& rItem : rTBC )
        {
            // createToolBar item for control
            if ( !rItem.ImportToolBarControl( rWrapper, xIndexContainer, helper, IsMenuToolbar() ) )
                return false;
        }

        SAL_INFO("sw.ww8","Name of toolbar :-/ " << sToolBarName );

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
        SAL_INFO("sw.ww8","***** For some reason we have an " << e );
        bRes = false;
    }
    return bRes;
}

bool SwCTB::ImportMenuTB( SwCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& xIndexContainer, CustomToolBarImportHelper& rHelper )
{
    for ( auto& rItem : rTBC )
    {
        // createToolBar item for control
        if ( !rItem.ImportToolBarControl( rWrapper, xIndexContainer, rHelper, true ) )
            return false;
    }
    return true;
}

SwTBC::SwTBC()
{
}

SwTBC::~SwTBC()
{
}

bool SwTBC::Read( SvStream &rS )
{
    SAL_INFO("sw.ww8","SwTBC::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !tbch.Read( rS ) )
        return false;
    if ( tbch.getTcID() != 0x1 && tbch.getTcID() != 0x1051 )
    {
        cid.reset( new sal_uInt32 );
        rS.ReadUInt32( *cid );
    }
    // MUST exist if tbch.tct is not equal to 0x16
    if ( tbch.getTct() != 0x16 )
    {
        tbcd.reset(  new TBCData( tbch ) );
        if ( !tbcd->Read( rS ) )
            return false;
    }
    return rS.good();
}

bool
SwTBC::ImportToolBarControl( SwCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuBar )
{
    // cmtFci       0x1 Command based on a built-in command. See CidFci.
    // cmtMacro     0x2 Macro command. See CidMacro.
    // cmtAllocated 0x3 Allocated command. See CidAllocated.
    // cmtNil       0x7 No command. See Cid.
    bool bBuiltin = false;
    sal_Int16 cmdId = 0;
    if  ( cid.get() )
    {
        const sal_uInt32 nCid = ( *cid & 0xFFFF );

        const sal_uInt8 cmt = static_cast<sal_uInt8>( nCid & 0x7 );
        const sal_Int16 arg2 = static_cast<sal_Int16>( nCid >> 3 );

        switch ( cmt )
        {
            case 1:
                SAL_INFO("sw.ww8","cmt is cmtFci builtin command 0x" << std::hex << arg2);
                bBuiltin = true;
                cmdId = arg2;
                break;
            case 2:
                SAL_INFO("sw.ww8","cmt is cmtMacro macro 0x" << std::hex << arg2);
                break;
            case 3:
                SAL_INFO("sw.ww8","cmt is cmtAllocated [???] 0x" << std::hex << arg2);
                break;
            case 7:
                SAL_INFO("sw.ww8","cmt is cmNill no-thing 0x" << std::hex << arg2);
                break;
            default:
                SAL_INFO("sw.ww8","illegal 0x" << std::hex << cmt);
                break;
        }
    }

    if ( tbcd.get() )
    {
        std::vector< css::beans::PropertyValue > props;
        if ( bBuiltin )
        {
            const OUString sCommand = helper.MSOCommandToOOCommand( cmdId );
            if ( !sCommand.isEmpty() )
            {
                beans::PropertyValue aProp;

                aProp.Name = "CommandURL";
                aProp.Value <<= sCommand;
                props.push_back( aProp );
            }
        }
        bool bBeginGroup = false;
        tbcd->ImportToolBarControl( helper, props, bBeginGroup, bIsMenuBar );

        TBCMenuSpecific* pMenu = tbcd->getMenuSpecific();
        if ( pMenu )
        {
            SAL_INFO("sw.ww8","** control has a menu, name of toolbar with menu items is " << pMenu->Name() );
            // search for SwCTB with the appropriate name ( it contains the
            // menu items, although we cannot import ( or create ) a menu on
            // a custom toolbar we can import the menu items in a separate
            // toolbar ( better than nothing )
            SwCTB* pCustTB = rWrapper.GetCustomizationData( pMenu->Name() );
            if ( pCustTB )
            {
                uno::Reference< container::XIndexContainer > xMenuDesc = document::IndexedPropertyValues::create( comphelper::getProcessComponentContext() );
                if ( !pCustTB->ImportMenuTB( rWrapper,xMenuDesc, helper ) )
                    return false;
                if ( !bIsMenuBar )
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

OUString
SwTBC::GetCustomText()
{
    if ( tbcd.get() )
        return tbcd->getGeneralInfo().CustomText();
    return OUString();
}

bool
Xst::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","Xst::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    sString = read_uInt16_PascalString(rS);
    return rS.good();
}

Tcg::Tcg() : nTcgVer( -1 )
{
}

bool Tcg::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Tcg::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadSChar( nTcgVer );
    if ( nTcgVer != -1 )
        return false;
    tcg.reset( new Tcg255() );
    return tcg->Read( rS );
}

bool Tcg::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    if (tcg)
        return tcg->ImportCustomToolBar( rDocSh );
    return false;
}

Tcg255::Tcg255()
{
}

Tcg255::~Tcg255()
{
}

bool Tcg255::processSubStruct( sal_uInt8 nId, SvStream &rS )
{
    std::unique_ptr<Tcg255SubStruct> xSubStruct;
    switch ( nId )
    {
        case 0x1:
        {
            xSubStruct.reset(new PlfMcd);
            break;
        }
        case 0x2:
        {
            xSubStruct.reset(new PlfAcd);
            break;
        }
        case 0x3:
        case 0x4:
        {
            xSubStruct.reset(new PlfKme);
            break;
        }
        case 0x10:
        {
            xSubStruct.reset(new TcgSttbf);
            break;
        }
        case 0x11:
        {
            xSubStruct.reset(new MacroNames);
            break;
        }
        case 0x12:
        {
            xSubStruct.reset(new SwCTBWrapper);
            break;
        }
        default:
            SAL_INFO("sw.ww8","Unknown id 0x" << std::hex << nId);
            return false;
    }
    xSubStruct->ch = nId;
    if (!xSubStruct->Read(rS))
        return false;
    rgtcgData.push_back(std::move(xSubStruct));
    return true;
}

bool Tcg255::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    // Find the SwCTBWrapper
    for ( auto & rSubStruct : rgtcgData )
    {
        if ( rSubStruct->id() == 0x12 )
        {
            // not so great, shouldn't really have to do a horror casting
            SwCTBWrapper* pCTBWrapper =  dynamic_cast< SwCTBWrapper* > ( rSubStruct.get() );
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
    SAL_INFO("sw.ww8","Tcg255::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    sal_uInt8 nId = 0x40;
    rS.ReadUChar( nId );
    while (  nId != 0x40  )
    {
        if ( !processSubStruct( nId, rS ) )
            return false;
        nId = 0x40;
        rS.ReadUChar( nId );
    }
    return rS.good();
    // Peek at
}

Tcg255SubStruct::Tcg255SubStruct( ) : ch(0)
{
}

bool Tcg255SubStruct::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Tcg255SubStruct::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    return rS.good();
}

PlfMcd::PlfMcd()
    : iMac(0)
{
}

bool PlfMcd::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","PffMcd::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadInt32( iMac );
    if (iMac < 0)
        return false;
    auto nMaxPossibleRecords = rS.remainingSize() / 24 /*sizeof MCD*/;
    if (static_cast<sal_uInt32>(iMac) > nMaxPossibleRecords)
    {
        SAL_WARN("sw.ww8", iMac << " records claimed, but max possible is " << nMaxPossibleRecords);
        iMac = nMaxPossibleRecords;
    }
    if (iMac)
    {
        rgmcd.resize(iMac);
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgmcd[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

PlfAcd::PlfAcd() :
 iMac(0)
{
}

PlfAcd::~PlfAcd()
{
}

bool PlfAcd::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","PffAcd::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadInt32( iMac );
    if (iMac < 0)
        return false;
    auto nMaxPossibleRecords = rS.remainingSize() / (sizeof(sal_uInt16)*2);
    if (static_cast<sal_uInt32>(iMac) > nMaxPossibleRecords)
    {
        SAL_WARN("sw.ww8", iMac << " records claimed, but max possible is " << nMaxPossibleRecords);
        iMac = nMaxPossibleRecords;
    }
    if (iMac)
    {
        rgacd.reset( new Acd[ iMac ] );
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgacd[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

PlfKme::PlfKme() :
 iMac( 0 )
{
}

PlfKme::~PlfKme()
{
}

bool PlfKme::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","PlfKme::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadInt32( iMac );
    if (iMac > 0)
    {
        //each Kme is 14 bytes in size
        size_t nMaxAvailableRecords = rS.remainingSize() / 14;
        if (static_cast<sal_uInt32>(iMac) > nMaxAvailableRecords)
            return false;

        rgkme.reset( new Kme[ iMac ] );
        for( sal_Int32 index=0; index<iMac; ++index )
        {
            if ( !rgkme[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

TcgSttbf::TcgSttbf()
{
}

bool TcgSttbf::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","TcgSttbf::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    return sttbf.Read( rS );
}

TcgSttbfCore::TcgSttbfCore() : fExtend( 0 )
,cData( 0 )
,cbExtra( 0 )
{
}

TcgSttbfCore::~TcgSttbfCore()
{
}

bool TcgSttbfCore::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","TcgSttbfCore::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUInt16( fExtend ).ReadUInt16( cData ).ReadUInt16( cbExtra );
    if ( cData )
    {
        if (cData > rS.remainingSize() / 4) //definitely an invalid record
            return false;
        dataItems.reset( new SBBItem[ cData ] );
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            rS.ReadUInt16( dataItems[ index ].cchData );
            dataItems[ index ].data = read_uInt16s_ToOUString(rS, dataItems[index].cchData);
            rS.ReadUInt16( dataItems[ index ].extraData );
        }
    }
    return rS.good();
}

MacroNames::MacroNames() :
 iMac( 0 )
{
}

MacroNames::~MacroNames()
{
}

bool MacroNames::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","MacroNames::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadUInt16( iMac );
    if ( iMac )
    {
        //even an empty MacroName will take 2 bytes
        size_t nMaxAvailableRecords = rS.remainingSize()/sizeof(sal_uInt16);
        if (iMac > nMaxAvailableRecords)
            return false;
        rgNames.reset( new MacroName[ iMac ] );
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgNames[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

MacroName::MacroName():ibst(0)
{
}

bool MacroName::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","MacroName::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUInt16( ibst );
    return xstz.Read( rS );
}

Xstz::Xstz():chTerm(0)
{
}

bool
Xstz::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Xstz::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !xst.Read( rS ) )
        return false;
    rS.ReadUInt16( chTerm );
    if ( chTerm != 0 ) // should be an assert
        return false;
    return rS.good();
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
    SAL_INFO("sw.ww8","Kme::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadInt16( reserved1 ).ReadInt16( reserved2 ).ReadUInt16( kcm1 ).ReadUInt16( kcm2 ).ReadUInt16( kt ).ReadUInt32( param );
    return rS.good();
}

Acd::Acd() : ibst( 0 )
, fciBasedOnABC( 0 )
{
}

bool Acd::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Acd::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadInt16( ibst ).ReadUInt16( fciBasedOnABC );
    return rS.good();
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

bool MCD::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","MCD::Read() stream pos 0x" << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadSChar( reserved1 ).ReadUChar( reserved2 ).ReadUInt16( ibst ).ReadUInt16( ibstName ).ReadUInt16( reserved3 );
    rS.ReadUInt32( reserved4 ).ReadUInt32( reserved5 ).ReadUInt32( reserved6 ).ReadUInt32( reserved7 );
    return rS.good();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
