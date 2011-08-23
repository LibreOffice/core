/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <filter/msfilter/mstoolbar.hxx>
#include <rtl/ustrbuf.hxx>
#include <stdarg.h>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <fstream>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <map>
#include <sfx2/objsh.hxx>
#include <basic/basmgr.hxx>
#include <svtools/filterutils.hxx>
#include <boost/scoped_array.hpp>
#include <filter/msfilter/msvbahelper.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

using namespace com::sun::star;

int TBBase::nIndent = 0;

void CustomToolBarImportHelper::showToolbar( const rtl::OUString& rName )
{
    try
    {
        uno::Reference< frame::XController > xCntrller( mrDocSh.GetModel()->getCurrentController(), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xCntrller->getFrame(), uno::UNO_QUERY_THROW );
        uno::Reference< frame::XLayoutManager > xLayoutMgr( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LayoutManager") ) ), uno::UNO_QUERY_THROW );
        xLayoutMgr->showElement( rName );
    }
    catch( uno::Exception& ) {}
}

void CustomToolBarImportHelper::ScaleImage( uno::Reference< graphic::XGraphic >& xGraphic, long nNewSize )
{
    Graphic aGraphic( xGraphic );
    Size aSize = aGraphic.GetSizePixel();
    if ( aSize.Height() && ( aSize.Height() == aSize.Width() ) )
    {
        Image aImage( xGraphic );
        if ( aSize.Height() != nNewSize )
        {
            BitmapEx aBitmap = aImage.GetBitmapEx();
            BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, nNewSize );
            aImage = Image( aBitmapex);
            xGraphic = aImage.GetXGraphic();
        }
    } 
}

void CustomToolBarImportHelper::applyIcons()
{
    for ( std::vector< iconcontrolitem >::iterator it = iconcommands.begin(); it != iconcommands.end(); ++it )
    {
        uno::Sequence< rtl::OUString > commands(1);
        commands[ 0 ] = it->sCommand;
        uno::Sequence< uno::Reference< graphic::XGraphic > > images(1);
        images[ 0 ] = it->image;
       
        OSL_TRACE("About to applyIcons for command %s, have image ? %s", rtl::OUStringToOString( commands[ 0 ], RTL_TEXTENCODING_UTF8 ).getStr(), images[ 0 ].is() ? "yes" : "no" );
        uno::Reference< ui::XImageManager > xImageManager( getCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
        sal_uInt16 nColor = ui::ImageType::COLOR_NORMAL; 

        Window* topwin = Application::GetActiveTopWindow();
	if ( topwin != NULL && topwin->GetDisplayBackground().GetColor().IsDark() )
            nColor = css::ui::ImageType::COLOR_HIGHCONTRAST;

        ScaleImage( images[ 0 ], 16 );
        xImageManager->replaceImages( ui::ImageType::SIZE_DEFAULT | nColor,  commands, images );
        ScaleImage( images[ 0 ], 26 );
        xImageManager->replaceImages( ui::ImageType::SIZE_LARGE | nColor,  commands, images );
    }
}

void CustomToolBarImportHelper::addIcon( const uno::Reference< graphic::XGraphic >& xImage, const rtl::OUString& sString )
{
    iconcontrolitem item;
    item.sCommand = sString;
    item.image = xImage;
    iconcommands.push_back( item );
}

CustomToolBarImportHelper::CustomToolBarImportHelper( SfxObjectShell& rDocShell,  const css::uno::Reference< css::ui::XUIConfigurationManager>& rxAppCfgMgr ) : mrDocSh( rDocShell ) 
{
    m_xCfgSupp.set( mrDocSh.GetModel(), uno::UNO_QUERY_THROW );
    m_xAppCfgMgr.set( rxAppCfgMgr, uno::UNO_QUERY_THROW );
}

uno::Reference< ui::XUIConfigurationManager > 
CustomToolBarImportHelper::getCfgManager()
{
    return m_xCfgSupp->getUIConfigurationManager();
}

uno::Reference< ui::XUIConfigurationManager > 
CustomToolBarImportHelper::getAppCfgManager()
{
    return m_xAppCfgMgr;
}

uno::Any 
CustomToolBarImportHelper::createCommandFromMacro( const rtl::OUString& sCmd )
{
//"vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document"    
    static rtl::OUString scheme = rtl::OUString::createFromAscii( "vnd.sun.star.script:");
    static rtl::OUString part2 = rtl::OUString::createFromAscii("?language=Basic&location=document");
    // create script url
    rtl::OUString scriptURL = scheme + sCmd + part2;
    return uno::makeAny( scriptURL );
}

rtl::OUString CustomToolBarImportHelper::MSOCommandToOOCommand( sal_Int16 msoCmd )
{
    rtl::OUString result;
    if ( pMSOCmdConvertor.get() )
        result = pMSOCmdConvertor->MSOCommandToOOCommand( msoCmd );
    return result;
}

rtl::OUString CustomToolBarImportHelper::MSOTCIDToOOCommand( sal_Int16 msoTCID )
{
    rtl::OUString result;
    if ( pMSOCmdConvertor.get() )
        result = pMSOCmdConvertor->MSOTCIDToOOCommand( msoTCID );
    return result;
}

bool
CustomToolBarImportHelper::createMenu( const rtl::OUString& rName, const uno::Reference< container::XIndexAccess >& xMenuDesc, bool bPersist )
{
    bool bRes = true;
    try
    {
        uno::Reference< ui::XUIConfigurationManager > xCfgManager( getCfgManager() );
        rtl::OUString sMenuBar( RTL_CONSTASCII_USTRINGPARAM("private:resource/menubar/") );
        sMenuBar += rName;
        uno::Reference< container::XIndexContainer > xPopup( xCfgManager->createSettings(), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xPopup, uno::UNO_QUERY_THROW ); 
        // set name for menubar
        xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UIName") ), uno::makeAny( rName ) );
        if ( xPopup.is() )
        {
            uno::Sequence< beans::PropertyValue > aPopupMenu( 4 );
            aPopupMenu[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") );
            aPopupMenu[0].Value = uno::makeAny( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("vnd.openoffice.org:") ) + rName );
            aPopupMenu[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Label") );
            aPopupMenu[1].Value <<= rName;
            aPopupMenu[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ItemDescriptorContainer") );
            aPopupMenu[2].Value = uno::makeAny( xMenuDesc );
            aPopupMenu[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Type" ) );
            aPopupMenu[3].Value <<= sal_Int32( 0 );
            
            xPopup->insertByIndex( xPopup->getCount(), uno::makeAny( aPopupMenu ) );
            if ( bPersist )
            {
                xCfgManager->insertSettings( sMenuBar, uno::Reference< container::XIndexAccess >( xPopup, uno::UNO_QUERY ) );
                uno::Reference< ui::XUIConfigurationPersistence > xPersistence( xCfgManager, uno::UNO_QUERY_THROW );
                xPersistence->store();
            }
        }
    }
    catch( uno::Exception& )
    {
        bRes = false;
    }
    return bRes;
}

void
TBBase::indent_printf( FILE* fp, const char* format, ... )
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

rtl::OUString TBBase::readUnicodeString( SvStream* pS, sal_Int32 nChars )
{
    sal_Int32 nBufSize = nChars * 2;
    boost::scoped_array< sal_uInt8 > pArray( new sal_uInt8[ nBufSize ] );
    pS->Read( pArray.get(), nBufSize );
    return svt::BinFilterUtils::CreateOUStringFromUniStringArray(  reinterpret_cast< const char* >( pArray.get() ), nBufSize );
}

TBCHeader::TBCHeader() : bSignature( 0x3 )
,bVersion( 0x01 )
,bFlagsTCR( 0 )
,tct(0x1) // default to Button
,tcid(0)
,tbct(0)
{
}


TBCHeader::~TBCHeader()
{
}

bool TBCHeader::Read( SvStream* pS )
{
    OSL_TRACE("TBCHeader::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> bSignature >> bVersion >> bFlagsTCR >> tct >> tcid >> tbct >> bPriority;
    //  bit 4 ( from lsb ) 
    if ( bFlagsTCR & 0x10 )
    {
        width.reset( new sal_uInt16 );
        height.reset( new sal_uInt16 );
        *pS >> *width >> *height;
    }
    return true;
}

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
        indent_printf(fp,"  width 0x%d(0x%x)\n", *width, *width);
    if ( height.get() )
        indent_printf(fp,"  height 0x%d(0x%x)\n", *height, *height);
}

TBCData::TBCData( const TBCHeader& Header ) : rHeader( Header )
{
}

bool TBCData::Read(SvStream *pS)
{
    OSL_TRACE("TBCData::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if ( !controlGeneralInfo.Read(pS) /*|| !controlSpecificInfo.Read(pS)*/ )
        return false;
    switch ( rHeader.getTct() )
    {
        case 0x01: // (Button control)
        case 0x10: // (ExpandingGrid control)
            controlSpecificInfo.reset( new TBCBSpecific() );
            break;
        case 0x0A: // (Popup control)
        case 0x0C: // (ButtonPopup control)
        case 0x0D: // (SplitButtonPopup control)
        case 0x0E: // (SplitButtonMRUPopup control)
            controlSpecificInfo.reset( new TBCMenuSpecific() );
            break;
        case 0x02: // (Edit control)
        case 0x04: // (ComboBox control)
        case 0x14: // (GraphicCombo control)
        case 0x03: // (DropDown control)
        case 0x06: // (SplitDropDown control)
        case 0x09: // (GraphicDropDown control)
            controlSpecificInfo.reset( new TBCComboDropdownSpecific( rHeader ) );
            break;
        default:
            break;
    }
    if ( controlSpecificInfo.get() )
        return controlSpecificInfo->Read( pS );
    //#FIXME I need to be able to handle different controlSpecificInfo types.
    return true;
}

TBCMenuSpecific* TBCData::getMenuSpecific()
{
    TBCMenuSpecific* pMenu = dynamic_cast< TBCMenuSpecific* >( controlSpecificInfo.get() );
    return pMenu;
}
bool TBCData::ImportToolBarControl( CustomToolBarImportHelper& helper, std::vector< css::beans::PropertyValue >& props, bool& bBeginGroup, bool bIsMenuBar )
{
    sal_uInt16  nStyle = 0;
    bBeginGroup = rHeader.isBeginGroup();
    controlGeneralInfo.ImportToolBarControlData( helper, props );
    beans::PropertyValue aProp;
    aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Visible") ) ;
    aProp.Value = uno::makeAny( rHeader.isVisible() ); // where is the visible attribute stored
    props.push_back( aProp );
    if ( rHeader.getTct() == 0x01
    || rHeader.getTct() == 0x10 )
    {
        TBCBSpecific* pSpecificInfo = dynamic_cast< TBCBSpecific* >( controlSpecificInfo.get() );
        if ( pSpecificInfo )
        {
            // if we have a icon then lets  set it for the command 
            rtl::OUString sCommand;
            for ( std::vector< css::beans::PropertyValue >::iterator it = props.begin(); it != props.end(); ++it )
            {
                if ( it->Name.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") ) ) )
                    it->Value >>= sCommand;
            }
            if ( TBCBitMap* pIcon = pSpecificInfo->getIcon() )
            {
                // Without a command openoffice won't display the icon
                if ( sCommand.getLength() )
                {    
                    BitmapEx aBitEx( pIcon->getBitMap() );
                    if ( pSpecificInfo->getIconMask() )
                         // according to the spec:
                         // "the iconMask is white in all the areas in which the icon is
                         // displayed as transparent and is black in all other areas."
                         aBitEx = BitmapEx( aBitEx.GetBitmap(), pSpecificInfo->getIconMask()->getBitMap().CreateMask( Color( COL_WHITE ) ) );
    
                    Graphic aGraphic( aBitEx );
                    helper.addIcon( aGraphic.GetXGraphic(), sCommand );
                }
            }
            else if ( pSpecificInfo->getBtnFace() )
            {
               
                rtl::OUString sBuiltInCmd = helper.MSOTCIDToOOCommand(  *pSpecificInfo->getBtnFace() );
                if ( sBuiltInCmd.getLength() )
                {
                    uno::Sequence< rtl::OUString> sCmds(1);
                    sCmds[ 0 ] = sBuiltInCmd;
                    uno::Reference< ui::XImageManager > xImageManager( helper.getAppCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
                    // 0 = default image size
                    uno::Sequence< uno::Reference< graphic::XGraphic > > sImages = xImageManager->getImages( 0, sCmds );
                    if ( sImages.getLength() && sImages[0].is() )
                        helper.addIcon( sImages[0], sCommand );
                }
            }
        }
    }
    else if ( rHeader.getTct() == 0x0a )
    {
        aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") ) ;
        rtl::OUString sMenuBar( RTL_CONSTASCII_USTRINGPARAM("private:resource/menubar/") );
        
        TBCMenuSpecific* pMenu = getMenuSpecific();
        if ( pMenu )
            aProp.Value = uno::makeAny( sMenuBar += pMenu->Name() ); // name of popup
        nStyle |= ui::ItemStyle::DROP_DOWN;
        props.push_back( aProp );
    }

    short icontext =  ( rHeader.getTbct() & 0x03 );
    aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Style") ) ;
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
    return true; // just ignore
}

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

bool
WString::Read( SvStream *pS )
{
    OSL_TRACE("WString::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    sal_Int8 nChars = 0;
    *pS >> nChars;
    sString = readUnicodeString( pS, static_cast< sal_Int32 >( nChars  ) );
    return true;
}

TBCExtraInfo::TBCExtraInfo() : idHelpContext( 0 )
{
}

bool
TBCExtraInfo::Read( SvStream *pS )
{
    OSL_TRACE("TBCExtraInfo::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if( !wstrHelpFile.Read( pS )  )
        return false;

    *pS >> idHelpContext;

    if ( !wstrTag.Read( pS ) || !wstrOnAction.Read( pS ) || !wstrParam.Read( pS ) )
        return false;

    *pS >> tbcu >> tbmg;    
    return true;
}

void
TBCExtraInfo::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCExtraInfo -- dump\n", nOffSet );
    indent_printf( fp, "  wstrHelpFile %s\n", 
        rtl::OUStringToOString( wstrHelpFile.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  idHelpContext 0x%x\n", static_cast< unsigned int >( idHelpContext ) );
    indent_printf( fp, "  wstrTag %s\n", 
        rtl::OUStringToOString( wstrTag.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  wstrOnAction %s\n", 
        rtl::OUStringToOString( wstrOnAction.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  wstrParam %s\n", 
        rtl::OUStringToOString( wstrParam.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  tbcu 0x%x\n", tbcu );
    indent_printf( fp, "  tbmg 0x%x\n", tbmg );
    
}

rtl::OUString
TBCExtraInfo::getOnAction()
{
    return wstrOnAction.getString();
}

TBCGeneralInfo::TBCGeneralInfo() : bFlags( 0 )
{
}

bool TBCGeneralInfo::Read( SvStream *pS )
{
    OSL_TRACE("TBCGeneralInfo::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> bFlags;

    if ( ( bFlags & 0x1 ) && !customText.Read( pS ) )
        return false;
    if ( ( bFlags & 0x2 ) && ( !descriptionText.Read( pS ) ||  !tooltip.Read( pS ) ) )
        return false;
    if ( ( bFlags & 0x4 ) && !extraInfo.Read( pS ) )
        return false;
    return true;
}

void 
TBCGeneralInfo::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCGeneralInfo -- dump\n", nOffSet );
    indent_printf( fp, "  bFlags 0x%x\n", bFlags );
    indent_printf( fp, "  customText %s\n", 
        rtl::OUStringToOString( customText.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  description %s\n", 
        rtl::OUStringToOString( descriptionText.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf( fp, "  tooltip %s\n", 
        rtl::OUStringToOString( tooltip.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    if ( bFlags & 0x4 ) 
        extraInfo.Print( fp );
}

bool 
TBCGeneralInfo::ImportToolBarControlData( CustomToolBarImportHelper& helper, std::vector< beans::PropertyValue >& sControlData )
{
    if ( ( bFlags & 0x5 ) )
    {
        beans::PropertyValue aProp;
        // probably access to the header would be a better test than seeing if there is an action, e.g.
        // if ( rHeader.getTct() == 0x01 && rHeader.getTcID() == 0x01 ) // not defined, probably this is a command
        if ( extraInfo.getOnAction().getLength() )
        {
            aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") );
            ooo::vba::VBAMacroResolvedInfo aMacroInf = ooo::vba::resolveVBAMacro( &helper.GetDocShell(), extraInfo.getOnAction(), true );
            if ( aMacroInf.IsResolved() )
                aProp.Value = helper.createCommandFromMacro( aMacroInf.ResolvedMacro() );
            else
                aProp.Value <<= rtl::OUString::createFromAscii("UnResolvedMacro[").concat( extraInfo.getOnAction() ).concat( rtl::OUString::createFromAscii("]") );
            sControlData.push_back( aProp );
        }

        aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Label") );
        aProp.Value = uno::makeAny( customText.getString().replace('&','~') );
        sControlData.push_back( aProp );

        aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Type") );
        aProp.Value = uno::makeAny( ui::ItemType::DEFAULT ); 
        sControlData.push_back( aProp );

        aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Tooltip") );
        aProp.Value = uno::makeAny( tooltip.getString() ); 
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
    return true;
}

TBCMenuSpecific::TBCMenuSpecific() : tbid( 0 )
{
}

bool
TBCMenuSpecific::Read( SvStream *pS)
{
    OSL_TRACE("TBCMenuSpecific::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> tbid;
    if ( tbid == 1 ) 
    {
        name.reset( new WString() );
        return name->Read( pS );
    }
    return true;
}

void 
TBCMenuSpecific::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TBCMenuSpecific -- dump\n", nOffSet );
    indent_printf( fp, "  tbid 0x%x\n", static_cast< unsigned int >( tbid ) );
    if ( tbid == 1 )
        indent_printf( fp, "  name %s\n", rtl::OUStringToOString( name->getString(), RTL_TEXTENCODING_UTF8 ).getStr() );

}

rtl::OUString TBCMenuSpecific::Name()
{
    rtl::OUString aName;
    if ( name.get() )
        aName = name->getString();
    return aName;
}
TBCBSpecific::TBCBSpecific() : bFlags( 0 )
{
}

bool TBCBSpecific::Read( SvStream *pS)
{
    OSL_TRACE("TBCBSpecific::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> bFlags;

    // bFlags determines what we read next

    // bFlags.fCustomBitmap = 1 ( 0x8 ) set 
    if ( bFlags & 0x8 )
    {
        icon.reset( new TBCBitMap() );
        iconMask.reset( new TBCBitMap() );
        if ( !icon->Read( pS ) || !iconMask->Read( pS ) )
            return false;
    }
    // if bFlags.fCustomBtnFace = 1 ( 0x10 )
    if ( bFlags & 0x10 )
    {
        iBtnFace.reset( new sal_uInt16 );
        *pS >> *iBtnFace.get();
    }
    // if bFlags.fAccelerator equals 1 ( 0x04 )
    if ( bFlags & 0x04 )
    {
        wstrAcc.reset( new WString() );
        return wstrAcc->Read( pS );
    }
    return true;
}


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
        indent_printf( fp, "  iBtnFace 0x%x\n", *(iBtnFace.get()) );
    }
    bResult = ( wstrAcc.get() != NULL );
    indent_printf( fp, "  option string present? %s ->%s<-\n", bResult ? "true" : "false", bResult ? rtl::OUStringToOString( wstrAcc->getString(), RTL_TEXTENCODING_UTF8 ).getStr() : "N/A" );
}

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
        data.reset( new TBCCDData() );
}

bool TBCComboDropdownSpecific::Read( SvStream *pS)
{
    nOffSet = pS->Tell();
    if ( data.get() )
        return data->Read( pS );
    return true;
}

void TBCComboDropdownSpecific::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCComboDropdownSpecific -- dump\n", nOffSet );
    if ( data.get() )
        data->Print( fp );
    else
        indent_printf(fp," no data " );
}

TBCCDData::TBCCDData() : cwstrItems( 0 )
,iSel( 0 )
,cLines( 0 )
,dxWidth( 0 )
{
}

TBCCDData::~TBCCDData()
{
}

bool TBCCDData::Read( SvStream *pS)
{
    nOffSet = pS->Tell();
    *pS >> cwstrItems;
    if ( cwstrItems )
    {
        for( sal_Int32 index=0; index < cwstrItems; ++index )
        {
            WString aString;
            if ( !aString.Read( pS ) )
                return false;
            wstrList.push_back( aString );
        } 
    }
    *pS >> cwstrMRU >> iSel >> cLines >> dxWidth;

    return wstrEdit.Read( pS );
}

void TBCCDData::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBCCDData -- dump\n", nOffSet );
    indent_printf(fp,"  cwstrItems items in wstrList 0x%d\n", cwstrItems);
    for ( sal_Int32 index=0; index < cwstrItems; ++index )
    {
        Indent b;
        indent_printf(fp, "  wstrList[%d] %s", static_cast< int >( index ), rtl::OUStringToOString( wstrList[index].getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    indent_printf(fp,"  cwstrMRU num most recently used string 0x%d item\n", cwstrMRU);
    indent_printf(fp,"  iSel index of selected item 0x%d item\n", iSel);
    indent_printf(fp,"  cLines num of suggested lines to display 0x%d", cLines);
    indent_printf(fp,"  dxWidth width in pixels 0x%d", dxWidth);
    indent_printf(fp,"  wstrEdit %s", rtl::OUStringToOString( wstrEdit.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
}

TBCBitMap::TBCBitMap() : cbDIB( 0 ), size( 0 )
{
}

TBCBitMap::~TBCBitMap()
{
}

// #FIXME Const-ness
Bitmap& 
TBCBitMap::getBitMap()
{
    return mBitMap;
}

bool TBCBitMap::Read( SvStream* pS)
{
    OSL_TRACE("TBCBitMap::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> cbDIB;
    // cbDIB = sizeOf(biHeader) + sizeOf(colors) + sizeOf(bitmapData) + 10
    return mBitMap.Read( *pS, FALSE, TRUE );
}

void TBCBitMap::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] TBCBitMap -- dump\n", nOffSet );
    indent_printf(fp, "  TBCBitMap size of bitmap data 0x%x\n", static_cast< unsigned int > ( cbDIB ) );
}

TB::TB() : bSignature(0x2),
bVersion(0x1),
cCL(0),
ltbid( 0x1 ),
ltbtr(0),
cRowsDefault( 0 ),
bFlags( 0 )
{
}

bool TB::Read(SvStream *pS)
{
    OSL_TRACE("TB::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> bSignature >> bVersion >> cCL >> ltbid >> ltbtr >> cRowsDefault >> bFlags;
    name.Read( pS );
    return true;
 
}

bool TB::IsEnabled()
{
    return ( bFlags & 0x01 ) != 0x01;
}

bool TB::NeedsPositioning()
{
    return ( bFlags & 0x10 ) == 0x10;
}

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
    indent_printf(fp, "  name %s\n", rtl::OUStringToOString( name.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
}

TBVisualData::TBVisualData() : tbds(0), tbv(0), tbdsDock(0), iRow(0)
{
}

bool TBVisualData::Read( SvStream* pS )
{
    OSL_TRACE("TBVisualData::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> tbds >> tbv >> tbdsDock >> iRow;
    rcDock.Read( pS );
    rcFloat.Read( pS );
    return true;
}

void SRECT::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "  left 0x%x\n", left);
    indent_printf( fp, "  top 0x%x\n", top);
    indent_printf( fp, "  right 0x%x\n", right);
    indent_printf( fp, "  bottom 0x%x\n", bottom);
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
