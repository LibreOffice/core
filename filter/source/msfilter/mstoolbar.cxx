/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <filter/msfilter/mstoolbar.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <vcl/dibtools.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <sfx2/objsh.hxx>
#include <filter/msfilter/msvbahelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

using namespace com::sun::star;

int TBBase::nIndent = 0;

void CustomToolBarImportHelper::ScaleImage( uno::Reference< graphic::XGraphic >& xGraphic, tools::Long nNewSize )
{
    Graphic aGraphic( xGraphic );
    Size aSize = aGraphic.GetSizePixel();
    if ( aSize.Height() && ( aSize.Height() == aSize.Width() ) )
    {
        Graphic aImage(xGraphic);
        if ( aSize.Height() != nNewSize )
        {
            BitmapEx aBitmap = aImage.GetBitmapEx();
            BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, nNewSize );
            aImage = Graphic(aBitmapex);
            xGraphic = aImage.GetXGraphic();
        }
    }
}

void CustomToolBarImportHelper::applyIcons()
{
    for (auto const& concommand : iconcommands)
    {
        uno::Sequence<OUString> commands { concommand.sCommand };
        uno::Sequence< uno::Reference< graphic::XGraphic > > images(1);
        images[ 0 ] = concommand.image;

        uno::Reference< ui::XImageManager > xImageManager( getCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
        sal_uInt16 nColor = ui::ImageType::COLOR_NORMAL;

        vcl::Window* topwin = Application::GetActiveTopWindow();
        if ( topwin != nullptr && topwin->GetBackgroundColor().IsDark() )
                nColor = css::ui::ImageType::COLOR_HIGHCONTRAST;

        ScaleImage( images[ 0 ], 16 );
        xImageManager->replaceImages( ui::ImageType::SIZE_DEFAULT | nColor,  commands, images );
        ScaleImage( images[ 0 ], 26 );
        xImageManager->replaceImages( ui::ImageType::SIZE_LARGE | nColor,  commands, images );
    }
}

void CustomToolBarImportHelper::addIcon( const uno::Reference< graphic::XGraphic >& xImage, const OUString& sString )
{
    iconcontrolitem item;
    item.sCommand = sString;
    item.image = xImage;
    iconcommands.push_back( item );
}

CustomToolBarImportHelper::CustomToolBarImportHelper( SfxObjectShell& rDocShell,  const css::uno::Reference< css::ui::XUIConfigurationManager>& rxAppCfgMgr ) : mrDocSh( rDocShell )
{
    m_xCfgSupp.set( mrDocSh.GetModel(), uno::UNO_QUERY_THROW );
    m_xAppCfgMgr.set( rxAppCfgMgr, uno::UNO_SET_THROW );
}

uno::Reference< ui::XUIConfigurationManager >
CustomToolBarImportHelper::getCfgManager()
{
    return m_xCfgSupp->getUIConfigurationManager();
}


uno::Any
CustomToolBarImportHelper::createCommandFromMacro( const OUString& sCmd )
{
    //"vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document"
    // create script url
    OUString scriptURL = "vnd.sun.star.script:" + sCmd + "?language=Basic&location=document";
    return uno::makeAny( scriptURL );
}

OUString CustomToolBarImportHelper::MSOCommandToOOCommand( sal_Int16 msoCmd )
{
    OUString result;
    if (pMSOCmdConvertor)
        result = pMSOCmdConvertor->MSOCommandToOOCommand( msoCmd );
    return result;
}

OUString CustomToolBarImportHelper::MSOTCIDToOOCommand( sal_Int16 msoTCID )
{
    OUString result;
    if (pMSOCmdConvertor)
        result = pMSOCmdConvertor->MSOTCIDToOOCommand( msoTCID );
    return result;
}

bool
CustomToolBarImportHelper::createMenu( const OUString& rName, const uno::Reference< container::XIndexAccess >& xMenuDesc )
{
    bool bRes = true;
    try
    {
        uno::Reference< ui::XUIConfigurationManager > xCfgManager( getCfgManager() );
        OUString sMenuBar = "private:resource/menubar/" + rName;
        uno::Reference< container::XIndexContainer > xPopup( xCfgManager->createSettings(), uno::UNO_SET_THROW );
        uno::Reference< beans::XPropertySet > xProps( xPopup, uno::UNO_QUERY_THROW );
        // set name for menubar
        xProps->setPropertyValue("UIName", uno::makeAny( rName ) );
        if ( xPopup.is() )
        {
            uno::Sequence< beans::PropertyValue > aPopupMenu( 4 );
            aPopupMenu[0].Name = "CommandURL";
            aPopupMenu[0].Value <<= "vnd.openoffice.org:" + rName;
            aPopupMenu[1].Name = "Label";
            aPopupMenu[1].Value <<= rName;
            aPopupMenu[2].Name = "ItemDescriptorContainer";
            aPopupMenu[2].Value <<= xMenuDesc;
            aPopupMenu[3].Name = "Type";
            aPopupMenu[3].Value <<= sal_Int32( 0 );

            xPopup->insertByIndex( xPopup->getCount(), uno::makeAny( aPopupMenu ) );
            xCfgManager->insertSettings( sMenuBar, xPopup );
            uno::Reference< ui::XUIConfigurationPersistence > xPersistence( xCfgManager, uno::UNO_QUERY_THROW );
            xPersistence->store();
        }
    }
    catch( const uno::Exception& )
    {
        bRes = false;
    }
    return bRes;
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void TBBase::indent_printf( FILE* fp, const char* format, ... )
{
   va_list ap;
   va_start ( ap, format );

   // indent nIndent spaces
   for ( int i=0; i<nIndent; ++i)
      fprintf(fp," ");
   // append the rest of the message
   vfprintf( fp, format, ap );
   va_end( ap );
}
#endif

TBCHeader::TBCHeader()
    : bSignature(0x3)
    , bVersion(0x01)
    , bFlagsTCR(0)
    , tct(0x1) // default to Button
    , tcid(0)
    , tbct(0)
    , bPriority(0)
{
}

TBCHeader::~TBCHeader()
{
}

bool TBCHeader::Read( SvStream &rS )
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadSChar( bSignature ).ReadSChar( bVersion ).ReadUChar( bFlagsTCR ).ReadUChar( tct ).ReadUInt16( tcid ).ReadUInt32( tbct ).ReadUChar( bPriority );
    //  bit 4 ( from lsb )
    if ( bFlagsTCR & 0x10 )
    {
        width = std::make_shared<sal_uInt16>();
        height = std::make_shared<sal_uInt16>();
        rS.ReadUInt16( *width ).ReadUInt16( *height );
    }
    return true;
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void TBCHeader::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCHeader -- dump\n", nOffSet );
    indent_printf(fp,"  bSignature 0x%x\n", bSignature );
    indent_printf(fp,"  bVersion 0x%x\n", bVersion );
    indent_printf(fp,"  bFlagsTCR 0x%x\n", bFlagsTCR );
    indent_printf(fp,"  tct 0x%x\n", tct );
    indent_printf(fp,"  tcid 0x%x\n", tcid );
    indent_printf(fp,"  tbct 0x%x\n", static_cast< unsigned int >( tbct ));
    indent_printf(fp,"  bPriority 0x%x\n", bPriority );
    if ( width.get() )
        indent_printf(fp,"  width %d(0x%x)\n", *width, *width);
    if ( height.get() )
        indent_printf(fp,"  height %d(0x%x)\n", *height, *height);
}
#endif

TBCData::TBCData( const TBCHeader& Header ) : rHeader( Header )
{
}

bool TBCData::Read(SvStream &rS)
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    if ( !controlGeneralInfo.Read(rS) /*|| !controlSpecificInfo.Read(rS)*/ )
        return false;
    switch ( rHeader.getTct() )
    {
        case 0x01: // (Button control)
        case 0x10: // (ExpandingGrid control)
            controlSpecificInfo = std::make_shared<TBCBSpecific>();
            break;
        case 0x0A: // (Popup control)
        case 0x0C: // (ButtonPopup control)
        case 0x0D: // (SplitButtonPopup control)
        case 0x0E: // (SplitButtonMRUPopup control)
            controlSpecificInfo = std::make_shared<TBCMenuSpecific>();
            break;
        case 0x02: // (Edit control)
        case 0x04: // (ComboBox control)
        case 0x14: // (GraphicCombo control)
        case 0x03: // (DropDown control)
        case 0x06: // (SplitDropDown control)
        case 0x09: // (GraphicDropDown control)
            controlSpecificInfo = std::make_shared<TBCComboDropdownSpecific>( rHeader );
            break;
        default:
            break;
    }
    if ( controlSpecificInfo )
        return controlSpecificInfo->Read( rS );
    //#FIXME I need to be able to handle different controlSpecificInfo types.
    return true;
}

TBCMenuSpecific* TBCData::getMenuSpecific()
{
    TBCMenuSpecific* pMenu = dynamic_cast< TBCMenuSpecific* >( controlSpecificInfo.get() );
    return pMenu;
}
void TBCData::ImportToolBarControl( CustomToolBarImportHelper& helper, std::vector< css::beans::PropertyValue >& props, bool& bBeginGroup, bool bIsMenuBar )
{
    sal_uInt16  nStyle = 0;
    bBeginGroup = rHeader.isBeginGroup();
    controlGeneralInfo.ImportToolBarControlData( helper, props );
    beans::PropertyValue aProp;
    aProp.Name = "Visible";
    aProp.Value <<= rHeader.isVisible(); // where is the visible attribute stored
    props.push_back( aProp );
    if ( rHeader.getTct() == 0x01
    || rHeader.getTct() == 0x10 )
    {
        TBCBSpecific* pSpecificInfo = dynamic_cast< TBCBSpecific* >( controlSpecificInfo.get() );
        if ( pSpecificInfo )
        {
            // if we have an icon then lets set it for the command
            OUString sCommand;
            for (auto const& property : props)
            {
                // TODO JNA : couldn't we break if we find CommandURL to avoid keeping on the loop?
                if ( property.Name == "CommandURL" )
                    property.Value >>= sCommand;
            }
            if ( TBCBitMap* pIcon = pSpecificInfo->getIcon() )
            {
                // Without a command openoffice won't display the icon
                if ( !sCommand.isEmpty() )
                {
                    BitmapEx aBitEx( pIcon->getBitMap() );
                    TBCBitMap* pIconMask = pSpecificInfo->getIconMask();
                    if (pIconMask)
                    {
                        const Bitmap& rMaskBase(pIconMask->getBitMap().GetBitmap());
                        Size aMaskSize = rMaskBase.GetSizePixel();
                        if (aMaskSize.Width() && aMaskSize.Height())
                        {
                            // according to the spec:
                            // "the iconMask is white in all the areas in which the icon is
                            // displayed as transparent and is black in all other areas."
                            aBitEx = BitmapEx(aBitEx.GetBitmap(), rMaskBase.CreateMask(COL_WHITE));
                        }
                    }

                    Graphic aGraphic( aBitEx );
                    helper.addIcon( aGraphic.GetXGraphic(), sCommand );
                }
            }
            else if ( pSpecificInfo->getBtnFace() )
            {

                OUString sBuiltInCmd = helper.MSOTCIDToOOCommand(  *pSpecificInfo->getBtnFace() );
                if ( !sBuiltInCmd.isEmpty() )
                {
                    uno::Sequence<OUString> sCmds { sBuiltInCmd };
                    uno::Reference< ui::XImageManager > xImageManager( helper.getAppCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
                    // 0 = default image size
                    uno::Sequence< uno::Reference< graphic::XGraphic > > sImages = xImageManager->getImages( 0, sCmds );
                    if ( sImages.hasElements() && sImages[0].is() )
                        helper.addIcon( sImages[0], sCommand );
                }
            }
        }
    }
    else if ( rHeader.getTct() == 0x0a )
    {
        aProp.Name = "CommandURL";

        TBCMenuSpecific* pMenu = getMenuSpecific();
        if ( pMenu )
        {
            OUString sMenuBar = "private:resource/menubar/" + pMenu->Name();
            aProp.Value <<= sMenuBar; // name of popup
        }
        nStyle |= ui::ItemStyle::DROP_DOWN;
        props.push_back( aProp );
    }

    short icontext =  ( rHeader.getTbct() & 0x03 );
    aProp.Name = "Style";
    if ( bIsMenuBar )
    {
        nStyle |= ui::ItemStyle::TEXT;
        if ( !icontext || icontext == 0x3 )
            // Text And image
            nStyle |= ui::ItemStyle::ICON;
    }
    else
    {
        if ( ( icontext & 0x02 ) == 0x02 )
            nStyle |= ui::ItemStyle::TEXT;
        if ( !icontext || ( icontext & 0x03 ) == 0x03 )
            nStyle |= ui::ItemStyle::ICON;
    }
    aProp.Value <<= nStyle;
    props.push_back( aProp );
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void TBCData::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCData -- dump\n", nOffSet );
    indent_printf(fp,"  dumping controlGeneralInfo( TBCGeneralInfo )\n");
    controlGeneralInfo.Print( fp );
    //if ( rHeader.getTct() == 1 )
    if ( controlSpecificInfo.get() )
    {
        indent_printf(fp,"  dumping controlSpecificInfo( TBCBSpecificInfo )\n");
        controlSpecificInfo->Print( fp );
    }
}
#endif

bool
WString::Read( SvStream &rS )
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    sal_uInt8 nChars = 0;
    rS.ReadUChar( nChars );
    sString = read_uInt16s_ToOUString(rS, nChars);
    return true;
}

TBCExtraInfo::TBCExtraInfo()
    : idHelpContext(0)
    , tbcu(0)
    , tbmg(0)
{
}

bool
TBCExtraInfo::Read( SvStream &rS )
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    if( !wstrHelpFile.Read( rS )  )
        return false;

    rS.ReadInt32( idHelpContext );

    if ( !wstrTag.Read( rS ) || !wstrOnAction.Read( rS ) || !wstrParam.Read( rS ) )
        return false;

    rS.ReadSChar( tbcu ).ReadSChar( tbmg );
    return true;
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void
TBCExtraInfo::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCExtraInfo -- dump\n", nOffSet );
    indent_printf( fp, "  wstrHelpFile %s\n",
        OUStringToOString( wstrHelpFile.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  idHelpContext 0x%x\n", static_cast< unsigned int >( idHelpContext ) );
    indent_printf( fp, "  wstrTag %s\n",
        OUStringToOString( wstrTag.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  wstrOnAction %s\n",
        OUStringToOString( wstrOnAction.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  wstrParam %s\n",
        OUStringToOString( wstrParam.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  tbcu 0x%x\n", tbcu );
    indent_printf( fp, "  tbmg 0x%x\n", tbmg );
}
#endif

OUString const &
TBCExtraInfo::getOnAction() const
{
    return wstrOnAction.getString();
}

TBCGeneralInfo::TBCGeneralInfo() : bFlags( 0 )
{
}

bool TBCGeneralInfo::Read( SvStream &rS )
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUChar( bFlags );

    if ( ( bFlags & 0x1 ) && !customText.Read( rS ) )
        return false;
    if ( ( bFlags & 0x2 ) && ( !descriptionText.Read( rS ) ||  !tooltip.Read( rS ) ) )
        return false;
    if ( ( bFlags & 0x4 ) && !extraInfo.Read( rS ) )
        return false;
    return true;
}

#ifdef DEBUG_FILTER_MSFILTER
void
TBCGeneralInfo::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCGeneralInfo -- dump\n", nOffSet );
    indent_printf( fp, "  bFlags 0x%x\n", bFlags );
    indent_printf( fp, "  customText %s\n",
        OUStringToOString( customText.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  description %s\n",
        OUStringToOString( descriptionText.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  tooltip %s\n",
        OUStringToOString( tooltip.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    if ( bFlags & 0x4 )
        extraInfo.Print( fp );
}
#endif

void
TBCGeneralInfo::ImportToolBarControlData( CustomToolBarImportHelper& helper, std::vector< beans::PropertyValue >& sControlData )
{
    if ( !(bFlags & 0x5) )
        return;

    beans::PropertyValue aProp;
    // probably access to the header would be a better test than seeing if there is an action, e.g.
    // if ( rHeader.getTct() == 0x01 && rHeader.getTcID() == 0x01 ) // not defined, probably this is a command
    if ( !extraInfo.getOnAction().isEmpty() )
    {
        aProp.Name = "CommandURL";
        ooo::vba::MacroResolvedInfo aMacroInf = ooo::vba::resolveVBAMacro( &helper.GetDocShell(), extraInfo.getOnAction(), true );
        if ( aMacroInf.mbFound )
            aProp.Value = CustomToolBarImportHelper::createCommandFromMacro( aMacroInf.msResolvedMacro );
        else
            aProp.Value <<= "UnResolvedMacro[" + extraInfo.getOnAction() + "]";
        sControlData.push_back( aProp );
    }

    aProp.Name = "Label";
    aProp.Value <<= customText.getString().replace('&','~');
    sControlData.push_back( aProp );

    aProp.Name = "Type";
    aProp.Value <<= ui::ItemType::DEFAULT;
    sControlData.push_back( aProp );

    aProp.Name = "Tooltip";
    aProp.Value <<= tooltip.getString();
    sControlData.push_back( aProp );
/*
aToolbarItem(0).Name = "CommandURL" wstrOnAction
aToolbarItem(0).Value = Command
aToolbarItem(1).Name = "Label"      customText
aToolbarItem(1).Value = Label
aToolbarItem(2).Name = "Type"
aToolbarItem(2).Value = 0
aToolbarItem(3).Name = "Visible"
aToolbarItem(3).Value = true
*/
}

TBCMenuSpecific::TBCMenuSpecific() : tbid( 0 )
{
}

bool
TBCMenuSpecific::Read( SvStream &rS)
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadInt32( tbid );
    if ( tbid == 1 )
    {
        name = std::make_shared<WString>();
        return name->Read( rS );
    }
    return true;
}

#ifdef DEBUG_FILTER_MSFILTER
void
TBCMenuSpecific::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCMenuSpecific -- dump\n", nOffSet );
    indent_printf( fp, "  tbid 0x%x\n", static_cast< unsigned int >( tbid ) );
    if ( tbid == 1 )
        indent_printf( fp, "  name %s\n", OUStringToOString( name->getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
}
#endif

OUString TBCMenuSpecific::Name()
{
    OUString aName;
    if ( name )
        aName = name->getString();
    return aName;
}
TBCBSpecific::TBCBSpecific() : bFlags( 0 )
{
}

bool TBCBSpecific::Read( SvStream &rS)
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUChar( bFlags );

    // bFlags determines what we read next

    // bFlags.fCustomBitmap = 1 ( 0x8 ) set
    if ( bFlags & 0x8 )
    {
        icon = std::make_shared<TBCBitMap>();
        iconMask = std::make_shared<TBCBitMap>();
        if ( !icon->Read( rS ) || !iconMask->Read( rS ) )
            return false;
    }
    // if bFlags.fCustomBtnFace = 1 ( 0x10 )
    if ( bFlags & 0x10 )
    {
        iBtnFace = std::make_shared<sal_uInt16>();
        rS.ReadUInt16( *iBtnFace );
    }
    // if bFlags.fAccelerator equals 1 ( 0x04 )
    if ( bFlags & 0x04 )
    {
        wstrAcc = std::make_shared<WString>();
        return wstrAcc->Read( rS );
    }
    return true;
}


#ifdef DEBUG_FILTER_MSFILTER
void TBCBSpecific::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCBSpecific -- dump\n", nOffSet );
    indent_printf( fp, "  bFlags 0x%x\n", bFlags );
    bool bResult = ( icon.get() != NULL );
    indent_printf( fp, "  icon present? %s\n", bResult ? "true" : "false" );
    if ( bResult )
    {
        Indent b;
        indent_printf( fp, "  icon: \n");
        icon->Print( fp ); // will dump size
    }
    bResult = ( iconMask.get() != NULL );
    indent_printf( fp, "  icon mask present? %s\n", bResult ? "true" : "false" );
    if ( bResult )
    {
        Indent c;
        indent_printf( fp, "  icon mask: \n");
        iconMask->Print( fp ); // will dump size
    }
    if ( iBtnFace.get() )
    {
        indent_printf( fp, "  iBtnFace 0x%x\n", *iBtnFace );
    }
    bResult = ( wstrAcc.get() != NULL );
    indent_printf( fp, "  option string present? %s ->%s<-\n", bResult ? "true" : "false", bResult ? OUStringToOString( wstrAcc->getString(), RTL_TEXTENCODING_UTF8 ).getStr() : "N/A" );
}
#endif

TBCBitMap*
TBCBSpecific::getIcon()
{
    return icon.get();
}

TBCBitMap*
TBCBSpecific::getIconMask()
{
    return iconMask.get();
}

TBCComboDropdownSpecific::TBCComboDropdownSpecific(const TBCHeader& header )
{
    if ( header.getTcID() == 0x01 )
        data = std::make_shared<TBCCDData>();
}

bool TBCComboDropdownSpecific::Read( SvStream &rS)
{
    nOffSet = rS.Tell();
    if ( data )
        return data->Read( rS );
    return true;
}

#ifdef DEBUG_FILTER_MSFILTER
void TBCComboDropdownSpecific::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCComboDropdownSpecific -- dump\n", nOffSet );
    if ( data.get() )
        data->Print( fp );
    else
        indent_printf(fp," no data " );
}
#endif

TBCCDData::TBCCDData()
    : cwstrItems(0)
    , cwstrMRU(0)
    , iSel(0)
    , cLines(0)
    , dxWidth(0)
{
}

TBCCDData::~TBCCDData()
{
}

bool TBCCDData::Read( SvStream &rS)
{
    nOffSet = rS.Tell();
    rS.ReadInt16( cwstrItems );
    if (cwstrItems > 0)
    {
        auto nItems = o3tl::make_unsigned(cwstrItems);
        //each WString is at least one byte
        if (rS.remainingSize() < nItems)
            return false;
        for (decltype(nItems) index = 0; index < nItems; ++index)
        {
            WString aString;
            if ( !aString.Read( rS ) )
                return false;
            wstrList.push_back( aString );
        }
    }
    rS.ReadInt16( cwstrMRU ).ReadInt16( iSel ).ReadInt16( cLines ).ReadInt16( dxWidth );

    return wstrEdit.Read( rS );
}

#ifdef DEBUG_FILTER_MSFILTER
void TBCCDData::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCCDData -- dump\n", nOffSet );
    indent_printf(fp,"  cwstrItems items in wstrList %d\n", cwstrItems);
    for ( sal_Int32 index=0; index < cwstrItems; ++index )
    {
        Indent b;
        indent_printf(fp, "  wstrList[%d] %s", static_cast< int >( index ), OUStringToOString( wstrList[index].getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    indent_printf(fp,"  cwstrMRU num most recently used string %d item\n", cwstrMRU);
    indent_printf(fp,"  iSel index of selected item %d item\n", iSel);
    indent_printf(fp,"  cLines num of suggested lines to display %d", cLines);
    indent_printf(fp,"  dxWidth width in pixels %d", dxWidth);
    indent_printf(fp,"  wstrEdit %s", OUStringToOString( wstrEdit.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
}
#endif

TBCBitMap::TBCBitMap() : cbDIB( 0 )
{
}

TBCBitMap::~TBCBitMap()
{
}

bool TBCBitMap::Read( SvStream& rS)
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadInt32( cbDIB );
    // cbDIB = sizeOf(biHeader) + sizeOf(colors) + sizeOf(bitmapData) + 10
    return ReadDIBBitmapEx(mBitMap, rS, false, true);
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void TBCBitMap::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] TBCBitMap -- dump\n", nOffSet );
    indent_printf(fp, "  TBCBitMap size of bitmap data 0x%x\n", static_cast< unsigned int > ( cbDIB ) );
}
#endif

TB::TB() : bSignature(0x2),
bVersion(0x1),
cCL(0),
ltbid( 0x1 ),
ltbtr(0),
cRowsDefault( 0 ),
bFlags( 0 )
{
}

bool TB::Read(SvStream &rS)
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUChar( bSignature ).ReadUChar( bVersion ).ReadInt16( cCL ).ReadInt32( ltbid ).ReadUInt32( ltbtr ).ReadUInt16( cRowsDefault ).ReadUInt16( bFlags );
    name.Read( rS );
    return true;

}

bool TB::IsEnabled() const
{
    return ( bFlags & 0x01 ) != 0x01;
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void TB::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TB -- dump\n", nOffSet );
    indent_printf(fp,"  bSignature 0x%x\n", bSignature );
    indent_printf(fp,"  bVersion 0x%x\n", bVersion );
    indent_printf(fp,"  cCL 0x%x\n", cCL );
    indent_printf(fp,"  ltbid 0x%x\n", ltbid );
    indent_printf(fp,"  ltbtr 0x%x\n", ltbtr );
    indent_printf(fp,"  cRowsDefault 0x%x\n", cRowsDefault );
    indent_printf(fp,"  bFlags 0x%x\n", bFlags );
    indent_printf(fp, "  name %s\n", OUStringToOString( name.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
}
#endif

TBVisualData::TBVisualData() : tbds(0), tbv(0), tbdsDock(0), iRow(0)
{
}

bool TBVisualData::Read( SvStream& rS )
{
    SAL_INFO("filter.ms", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadSChar( tbds ).ReadSChar( tbv ).ReadSChar( tbdsDock ).ReadSChar( iRow );
    rcDock.Read( rS );
    rcFloat.Read( rS );
    return true;
}

#ifdef DEBUG_FILTER_MSTOOLBAR
void SRECT::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "  left 0x%x\n", left);
    indent_printf( fp, "  top 0x%x\n", top);
    indent_printf( fp, "  right 0x%x\n", right);
    indent_printf( fp, "  bottom 0x%x\n", bottom);
}
#endif

#ifdef DEBUG_FILTER_MSTOOLBAR
void TBVisualData::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBVisualData -- dump\n", nOffSet );
    indent_printf( fp, "  tbds 0x%x\n", tbds);
    indent_printf( fp, "  tbv  0x%x\n", tbv);
    indent_printf( fp, "  tbdsDoc  0x%x\n", tbdsDock);
    indent_printf( fp, "  iRow  0x%x\n", iRow);
    rcDock.Print( fp );
    rcFloat.Print( fp );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
