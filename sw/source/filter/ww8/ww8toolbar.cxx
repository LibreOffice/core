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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <fstream>
#include <comphelper/documentinfo.hxx>
#include <comphelper/indexedpropertyvalues.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/safeint.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/configuration.hxx>
#include <rtl/ref.hxx>
#include <map>
#include <sal/log.hxx>

using namespace com::sun::star;

// no. of visual data elements in a SwCTB ( fixed )
const short nVisualData = 5;

typedef std::map< sal_Int16, OUString > IdToString;

namespace {

class MSOWordCommandConvertor : public MSOCommandConvertor
{
   IdToString m_MSOToOOcmd;
   IdToString m_TCIDToOOcmd;

public:
    MSOWordCommandConvertor();
    virtual OUString MSOCommandToOOCommand( sal_Int16 msoCmd ) override;
    virtual OUString MSOTCIDToOOCommand( sal_Int16 key ) override;
};

}

MSOWordCommandConvertor::MSOWordCommandConvertor()
{
    // mso command id to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    m_MSOToOOcmd[ 0x20b ] = ".uno:CloseDoc";
    m_MSOToOOcmd[ 0x50 ] = ".uno:Open";

    // mso tcid to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    m_TCIDToOOcmd[ 0x9d9 ] = ".uno:Print";
}

OUString MSOWordCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    IdToString::iterator it = m_MSOToOOcmd.find( key );
    if ( it != m_MSOToOOcmd.end() )
        return it->second;
    return OUString();
}

OUString MSOWordCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    IdToString::iterator it = m_TCIDToOOcmd.find( key );
    if ( it != m_TCIDToOOcmd.end() )
        return it->second;
    return OUString();
}

SwCTBWrapper::SwCTBWrapper() :
m_reserved2(0)
,m_reserved3(0)
,m_reserved4(0)
,m_reserved5(0)
,m_cbTBD(0)
,m_cCust(0)
,m_cbDTBC(0)
,m_rtbdc(0)
{
}

SwCTBWrapper::~SwCTBWrapper()
{
}

Customization* SwCTBWrapper::GetCustomizaton( sal_Int16 index )
{
    if ( index < 0 || o3tl::make_unsigned(index) >= m_rCustomizations.size() )
        return nullptr;
    return &m_rCustomizations[ index ];
}

SwCTB* SwCTBWrapper::GetCustomizationData( const OUString& sTBName )
{
    auto it = std::find_if(m_rCustomizations.begin(), m_rCustomizations.end(),
        [&sTBName](Customization& rCustomization) {
            SwCTB* pCTB = rCustomization.GetCustomizationData();
            return pCTB && pCTB->GetName() == sTBName;
        });
    if (it != m_rCustomizations.end())
        return it->GetCustomizationData();
    return nullptr;
}

bool SwCTBWrapper::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","SwCTBWrapper::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadUInt16( m_reserved2 ).ReadUChar( m_reserved3 ).ReadUInt16( m_reserved4 ).ReadUInt16( m_reserved5 );
    rS.ReadInt16( m_cbTBD ).ReadUInt16( m_cCust ).ReadInt32( m_cbDTBC );
    sal_uInt64 nExpectedPos =  rS.Tell() + m_cbDTBC;
    if ( m_cbDTBC )
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
            m_rtbdc.push_back( aTBC );
            bytesToRead = m_cbDTBC - ( rS.Tell() - nStart );
        } while ( bytesToRead > 0 );
    }
    if ( rS.Tell() != nExpectedPos )
    {
        // Strange error condition, shouldn't happen ( but does in at least
        // one test document ) In the case where it happens the SwTBC &
        // TBCHeader records seem blank??? ( and incorrect )
        SAL_WARN_IF( rS.Tell() != nExpectedPos, "sw.ww8","### Error: Expected pos not equal to actual pos after reading rtbdc");
        SAL_INFO("sw.ww8","\tPos now is 0x" << std::hex << rS.Tell() << " should be 0x" << std::hex << nExpectedPos );
        // seek to correct position after rtbdc
        rS.Seek( nExpectedPos );
    }
    if (m_cCust)
    {
        //Each customization takes a min of 8 bytes
        size_t nMaxPossibleRecords = rS.remainingSize() / 8;
        if (m_cCust > nMaxPossibleRecords)
        {
            return false;
        }
        for (sal_uInt16 index = 0; index < m_cCust; ++index)
        {
            Customization aCust( this );
            if ( !aCust.Read( rS ) )
                return false;
            m_rCustomizations.push_back( aCust );
        }
    }
    for ( const auto& rIndex : m_dropDownMenuIndices )
    {
        if (rIndex < 0 || o3tl::make_unsigned(rIndex) >= m_rCustomizations.size())
            continue;
        m_rCustomizations[rIndex].m_bIsDroppedMenuTB = true;
    }
    return rS.good();
}

SwTBC* SwCTBWrapper::GetTBCAtOffset( sal_uInt32 nStreamOffset )
{
    auto it = std::find_if(m_rtbdc.begin(), m_rtbdc.end(),
        [&nStreamOffset](SwTBC& rItem) { return rItem.GetOffset() == nStreamOffset; });
    if ( it != m_rtbdc.end() )
        return &(*it);
    return nullptr;
}

bool SwCTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    for ( auto& rCustomization : m_rCustomizations )
    {
        try
        {
            css::uno::Reference<css::ui::XUIConfigurationManager> xCfgMgr;
            if (!comphelper::IsFuzzing())
            {
                uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( ui::theModuleUIConfigurationManagerSupplier::get(xContext) );
                xCfgMgr = xAppCfgSupp->getUIConfigurationManager(u"com.sun.star.text.TextDocument"_ustr);
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
    : m_tbidForTBD( 0 )
    , m_reserved1( 0 )
    , m_ctbds( 0 )
    , m_pWrapper( wrapper )
    , m_bIsDroppedMenuTB( false )
{
}

bool Customization::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","Customization::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadInt32( m_tbidForTBD ).ReadUInt16( m_reserved1 ).ReadUInt16( m_ctbds );
    if ( m_tbidForTBD )
    {
        //each TBDelta is at least 18 bytes in size
        size_t nMaxAvailableRecords = rS.remainingSize() / 18;
        if (m_ctbds > nMaxAvailableRecords)
            return false;
        for (sal_uInt16 index = 0; index < m_ctbds; ++index)
        {
            TBDelta aTBDelta;
            if (!aTBDelta.Read( rS ) )
                return false;
            m_customizationDataTBDelta.push_back( aTBDelta );
            // Only set the drop down for menus associated with standard toolbar
            if ( aTBDelta.ControlDropsToolBar() && m_tbidForTBD == 0x25 )
                m_pWrapper->InsertDropIndex( aTBDelta.CustomizationIndex() );
        }
    }
    else
    {
        m_customizationDataCTB = std::make_shared<SwCTB>();
        if ( !m_customizationDataCTB->Read( rS ) )
                return false;
    }
    return rS.good();
}

bool Customization::ImportMenu( SwCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    if ( m_tbidForTBD == 0x25 )  // we can handle in a limited way additions the built-in menu bar
    {
        for ( auto& rTBDelta : m_customizationDataTBDelta )
        {
            // for each new menu ( control that drops a toolbar )
            // import a toolbar
            if ( rTBDelta.ControlIsInserted() && rTBDelta.ControlDropsToolBar() )
            {
                Customization* pCust = m_pWrapper->GetCustomizaton( rTBDelta.CustomizationIndex() );
                if ( pCust )
                {
                    // currently only support built-in menu
                    static constexpr OUString sMenuBar( u"private:resource/menubar/menubar"_ustr );

                    // Get menu name
                    SwTBC* pTBC = m_pWrapper->GetTBCAtOffset( rTBDelta.TBCStreamOffset() );
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
                            xIndexContainer.set( helper.getAppCfgManager()->createSettings(), uno::UNO_SET_THROW );
                    }

                    uno::Reference< lang::XSingleComponentFactory > xSCF( xIndexContainer, uno::UNO_QUERY_THROW );
                    uno::Reference< uno::XComponentContext > xContext(
                        comphelper::getProcessComponentContext() );
                    uno::Reference< container::XIndexContainer > xMenuContainer( xSCF->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                    // create the popup menu
                    uno::Sequence< beans::PropertyValue > aPopupMenu{
                        comphelper::makePropertyValue(u"CommandURL"_ustr, "vnd.openoffice.org:" + sMenuName),
                        comphelper::makePropertyValue(u"Label"_ustr, sMenuName),
                        comphelper::makePropertyValue(u"Type"_ustr, sal_Int32( 0 )),
                        comphelper::makePropertyValue(u"ItemDescriptorContainer"_ustr, xMenuContainer)
                    };
                    if ( pCust->m_customizationDataCTB && !pCust->m_customizationDataCTB->ImportMenuTB( rWrapper, xMenuContainer, helper ) )
                        return false;
                    SAL_INFO("sw.ww8","** there are " << xIndexContainer->getCount() << " menu items on the bar, inserting after that");
                    xIndexContainer->insertByIndex( xIndexContainer->getCount(), uno::Any( aPopupMenu ) );

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
    if ( m_tbidForTBD == 0x25 )
        return ImportMenu( rWrapper, helper );
    if ( !m_customizationDataCTB )
        return false;
    if ( !m_customizationDataCTB->IsMenuToolbar() )
    {
        if ( !m_customizationDataCTB->ImportCustomToolBar( rWrapper, helper ) )
            return false;
    }
    return true;
}

TBDelta::TBDelta()
    : m_doprfatendFlags(0)
    , m_ibts(0)
    , m_cidNext(0)
    , m_cid(0)
    , m_fc(0)
    , m_CiTBDE(0)
    , m_cbTBC(0)
{
}

bool TBDelta::ControlIsInserted()
{
    return ( ( m_doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlDropsToolBar()
{
    return !( m_CiTBDE & 0x8000 );
}


sal_Int16 TBDelta::CustomizationIndex()
{
    sal_Int16 nIndex = m_CiTBDE;
    nIndex = nIndex >> 1;
    nIndex &= 0x1ff; // only 13 bits are relevant
    return nIndex;
}

bool TBDelta::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","TBDelta::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUChar( m_doprfatendFlags ).ReadUChar( m_ibts ).ReadInt32( m_cidNext ).ReadInt32( m_cid ).ReadInt32( m_fc ) ;
    rS.ReadUInt16( m_CiTBDE ).ReadUInt16( m_cbTBC );
    return rS.good();
}

SwCTB::SwCTB() : m_cbTBData( 0 )
,m_iWCTBl( 0 )
,m_reserved( 0 )
,m_unused( 0 )
,m_cCtls( 0 )
{
}

SwCTB::~SwCTB()
{
}

bool SwCTB::IsMenuToolbar() const
{
    return m_tb.IsMenuToolbar();
}

bool SwCTB::Read( SvStream &rS)
{
    SAL_INFO("sw.ww8","SwCTB::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !m_name.Read( rS ) )
        return false;
    rS.ReadInt32( m_cbTBData );
    if ( !m_tb.Read( rS ) )
        return false;
    for ( short index = 0; index < nVisualData; ++index )
    {
        TBVisualData aVisData;
        aVisData.Read( rS );
        m_rVisualData.push_back( aVisData );
    }

    rS.ReadInt32( m_iWCTBl ).ReadUInt16( m_reserved ).ReadUInt16( m_unused ).ReadInt32( m_cCtls );

    if ( m_cCtls )
    {
        for ( sal_Int32 index = 0; index < m_cCtls; ++index )
        {
            SwTBC aTBC;
            if ( !aTBC.Read( rS ) )
                return false;
            m_rTBC.push_back( aTBC );
        }
    }
    return rS.good();
}

bool SwCTB::ImportCustomToolBar( SwCTBWrapper& rWrapper, CustomToolBarImportHelper& helper )
{
    bool bRes = false;
    try
    {
        if ( !m_tb.IsEnabled() )
            return true;  // didn't fail, just ignoring
        // Create default setting
        uno::Reference< container::XIndexContainer > xIndexContainer( helper.getCfgManager()->createSettings(), uno::UNO_SET_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xIndexContainer, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xIndexContainer, uno::UNO_QUERY_THROW );

        // set UI name for toolbar
        xProps->setPropertyValue( u"UIName"_ustr, uno::Any( m_name.getString() ) );

        const OUString sToolBarName = "private:resource/toolbar/custom_" + m_name.getString();
        for ( auto& rItem : m_rTBC )
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
    catch( const uno::Exception& )
    {
        TOOLS_INFO_EXCEPTION("sw.ww8","***** For some reason we have an" );
        bRes = false;
    }
    return bRes;
}

bool SwCTB::ImportMenuTB( SwCTBWrapper& rWrapper, const css::uno::Reference< css::container::XIndexContainer >& xIndexContainer, CustomToolBarImportHelper& rHelper )
{
    for ( auto& rItem : m_rTBC )
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

bool SwTBC::Read( SvStream &rS )
{
    SAL_INFO("sw.ww8","SwTBC::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !m_tbch.Read( rS ) )
        return false;
    if ( m_tbch.getTcID() != 0x1 && m_tbch.getTcID() != 0x1051 )
    {
        m_cid = std::make_shared<sal_uInt32>();
        rS.ReadUInt32( *m_cid );
    }
    // MUST exist if tbch.tct is not equal to 0x16
    if ( m_tbch.getTct() != 0x16 )
    {
        m_tbcd = std::make_shared<TBCData>( m_tbch );
        if ( !m_tbcd->Read( rS ) )
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
    if  ( m_cid )
    {
        const sal_uInt32 nCid = ( *m_cid & 0xFFFF );

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

    if ( m_tbcd )
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
        m_tbcd->ImportToolBarControl( helper, props, bBeginGroup, bIsMenuBar );

        TBCMenuSpecific* pMenu = m_tbcd->getMenuSpecific();
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
                rtl::Reference< comphelper::IndexedPropertyValuesContainer > xMenuDesc = new comphelper::IndexedPropertyValuesContainer();
                if ( !pCustTB->ImportMenuTB( rWrapper,xMenuDesc, helper ) )
                    return false;
                if ( !bIsMenuBar )
                {
                    if ( !helper.createMenu( pMenu->Name(), xMenuDesc ) )
                        return false;
                }
                else
                {
                    beans::PropertyValue aProp;
                    aProp.Name = "ItemDescriptorContainer";
                    aProp.Value <<= uno::Reference< container::XIndexContainer >(xMenuDesc);
                    props.push_back( aProp );
                }
            }
        }

        if ( bBeginGroup )
        {
            // insert spacer
            uno::Sequence< beans::PropertyValue > sProps{ comphelper::makePropertyValue(
                u"Type"_ustr, ui::ItemType::SEPARATOR_LINE) };
            toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::Any( sProps ) );
        }

        toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::Any( comphelper::containerToSequence(props) ) );
    }
    return true;
}

OUString
SwTBC::GetCustomText()
{
    if ( m_tbcd )
        return m_tbcd->getGeneralInfo().CustomText();
    return OUString();
}

bool
Xst::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","Xst::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    m_sString = read_uInt16_PascalString(rS);
    return rS.good();
}

Tcg::Tcg() : m_nTcgVer( -1 )
{
}

bool Tcg::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Tcg::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadSChar( m_nTcgVer );
    if ( m_nTcgVer != -1 )
        return false;
    m_tcg.reset( new Tcg255() );
    return m_tcg->Read( rS );
}

bool Tcg::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    if (m_tcg)
        return m_tcg->ImportCustomToolBar( rDocSh );
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
    xSubStruct->m_ch = nId;
    if (!xSubStruct->Read(rS))
        return false;
    m_rgtcgData.push_back(std::move(xSubStruct));
    return true;
}

bool Tcg255::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    // Find the SwCTBWrapper
    for ( const auto & rSubStruct : m_rgtcgData )
    {
        if ( rSubStruct->id() == 0x12 )
        {
            // not so great, shouldn't really have to do a horror casting
            SwCTBWrapper* pCTBWrapper =  dynamic_cast< SwCTBWrapper* > ( rSubStruct.get() );
            if ( pCTBWrapper )
            {
                // tdf#127048 set this flag if we might import something
                uno::Reference<frame::XModel> const xModel(rDocSh.GetBaseModel());
                comphelper::DocumentInfo::notifyMacroEventRead(xModel);

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

Tcg255SubStruct::Tcg255SubStruct( ) : m_ch(0)
{
}

bool Tcg255SubStruct::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Tcg255SubStruct::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    return rS.good();
}

PlfMcd::PlfMcd()
    : m_iMac(0)
{
}

bool PlfMcd::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","PffMcd::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    Tcg255SubStruct::Read( rS );
    rS.ReadInt32( m_iMac );
    if (m_iMac < 0)
        return false;
    auto nMaxPossibleRecords = rS.remainingSize() / 24 /*sizeof MCD*/;
    if (o3tl::make_unsigned(m_iMac) > nMaxPossibleRecords)
    {
        SAL_WARN("sw.ww8", m_iMac << " records claimed, but max possible is " << nMaxPossibleRecords);
        m_iMac = nMaxPossibleRecords;
    }
    if (m_iMac)
    {
        m_rgmcd.resize(m_iMac);
        for ( sal_Int32 index = 0; index < m_iMac; ++index )
        {
            if ( !m_rgmcd[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

PlfAcd::PlfAcd() :
 m_iMac(0)
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
    rS.ReadInt32( m_iMac );
    if (m_iMac < 0)
        return false;
    auto nMaxPossibleRecords = rS.remainingSize() / (sizeof(sal_uInt16)*2);
    if (o3tl::make_unsigned(m_iMac) > nMaxPossibleRecords)
    {
        SAL_WARN("sw.ww8", m_iMac << " records claimed, but max possible is " << nMaxPossibleRecords);
        m_iMac = nMaxPossibleRecords;
    }
    if (m_iMac)
    {
        m_rgacd.reset( new Acd[ m_iMac ] );
        for ( sal_Int32 index = 0; index < m_iMac; ++index )
        {
            if ( !m_rgacd[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

PlfKme::PlfKme() :
 m_iMac( 0 )
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
    rS.ReadInt32( m_iMac );
    if (m_iMac > 0)
    {
        //each Kme is 14 bytes in size
        size_t nMaxAvailableRecords = rS.remainingSize() / 14;
        if (o3tl::make_unsigned(m_iMac) > nMaxAvailableRecords)
            return false;

        m_rgkme.reset( new Kme[ m_iMac ] );
        for( sal_Int32 index=0; index<m_iMac; ++index )
        {
            if ( !m_rgkme[ index ].Read( rS ) )
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
    return m_sttbf.Read( rS );
}

TcgSttbfCore::TcgSttbfCore() : m_fExtend( 0 )
,m_cData( 0 )
,m_cbExtra( 0 )
{
}

TcgSttbfCore::~TcgSttbfCore()
{
}

bool TcgSttbfCore::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8","TcgSttbfCore::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUInt16( m_fExtend ).ReadUInt16( m_cData ).ReadUInt16( m_cbExtra );
    if ( m_cData )
    {
        if (m_cData > rS.remainingSize() / 4) //definitely an invalid record
            return false;
        m_dataItems.reset( new SBBItem[ m_cData ] );
        for ( sal_Int32 index = 0; index < m_cData; ++index )
        {
            rS.ReadUInt16( m_dataItems[ index ].cchData );
            m_dataItems[ index ].data = read_uInt16s_ToOUString(rS, m_dataItems[index].cchData);
            rS.ReadUInt16( m_dataItems[ index ].extraData );
        }
    }
    return rS.good();
}

MacroNames::MacroNames() :
 m_iMac( 0 )
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
    rS.ReadUInt16( m_iMac );
    if ( m_iMac )
    {
        //even an empty MacroName will take 2 bytes
        size_t nMaxAvailableRecords = rS.remainingSize()/sizeof(sal_uInt16);
        if (m_iMac > nMaxAvailableRecords)
            return false;
        m_rgNames.reset( new MacroName[ m_iMac ] );
        for ( sal_Int32 index = 0; index < m_iMac; ++index )
        {
            if ( !m_rgNames[ index ].Read( rS ) )
                return false;
        }
    }
    return rS.good();
}

MacroName::MacroName():m_ibst(0)
{
}

bool MacroName::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","MacroName::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUInt16( m_ibst );
    return m_xstz.Read( rS );
}

Xstz::Xstz():m_chTerm(0)
{
}

bool
Xstz::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Xstz::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    if ( !m_xst.Read( rS ) )
        return false;
    rS.ReadUInt16( m_chTerm );
    if ( m_chTerm != 0 ) // should be an assert
        return false;
    return rS.good();
}

Kme::Kme() : m_reserved1(0)
,m_reserved2(0)
,m_kcm1(0)
,m_kcm2(0)
,m_kt(0)
,m_param(0)
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
    rS.ReadInt16( m_reserved1 ).ReadInt16( m_reserved2 ).ReadUInt16( m_kcm1 ).ReadUInt16( m_kcm2 ).ReadUInt16( m_kt ).ReadUInt32( m_param );
    return rS.good();
}

Acd::Acd() : m_ibst( 0 )
, m_fciBasedOnABC( 0 )
{
}

bool Acd::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","Acd::Read() stream pos 0x" << std::hex << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadInt16( m_ibst ).ReadUInt16( m_fciBasedOnABC );
    return rS.good();
}

MCD::MCD() :  m_reserved1(0x56)
,m_reserved2( 0 )
,m_ibst( 0 )
,m_ibstName( 0 )
,m_reserved3( 0xFFFF )
,m_reserved4( 0 )
,m_reserved5( 0 )
,m_reserved6( 0 )
,m_reserved7( 0 )
{
}

bool MCD::Read(SvStream &rS)
{
    SAL_INFO("sw.ww8","MCD::Read() stream pos 0x" << rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadSChar( m_reserved1 ).ReadUChar( m_reserved2 ).ReadUInt16( m_ibst ).ReadUInt16( m_ibstName ).ReadUInt16( m_reserved3 );
    rS.ReadUInt32( m_reserved4 ).ReadUInt32( m_reserved5 ).ReadUInt32( m_reserved6 ).ReadUInt32( m_reserved7 );
    return rS.good();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
