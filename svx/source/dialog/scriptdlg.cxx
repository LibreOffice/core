/**********************************************************************
 *
 *  $RCSfile: scriptdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:15:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <memory>

#include <sfx2/ipfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#pragma hdrstop

#include "dialogs.hrc"
#include "scriptdlg.hrc"
#include "fmresids.hrc"
#include "scriptdlg.hxx"
#include "dialmgr.hxx"

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <drafts/com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <drafts/com/sun/star/script/provider/XScriptProvider.hpp>
#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <drafts/com/sun/star/script/browse/XBrowseNodeFactory.hpp>
#include <drafts/com/sun/star/script/browse/BrowseNodeFactoryViewType.hpp>
#include <drafts/com/sun/star/script/provider/ScriptErrorRaisedException.hpp>
#include <drafts/com/sun/star/script/provider/ScriptExceptionRaisedException.hpp>
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#include <drafts/com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <com/sun/star/script/XInvocation.hpp>

#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <svtools/sbx.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;

void ShowErrorDialog( const Any& aException )
{
    SvxScriptErrorDialog* pDlg = new SvxScriptErrorDialog( NULL, aException );
    pDlg->Execute();
    delete pDlg;
}

SFTreeListBox::SFTreeListBox( Window* pParent, const ResId& rRes, ResMgr* pBasResMgr ) :
    SvTreeListBox( pParent, ResId( rRes.GetId() ) ),
    m_aImagesNormal(ResId(RID_IMGLST_OBJECTS, pBasResMgr )),
    m_aImagesHighContrast(ResId(RID_IMGLST_OBJECTS_HC, pBasResMgr )),
    m_hdImage(ResId(IMG_HARDDISK)),
    m_hdImage_hc(ResId(IMG_HARDDISK_HC)),
    m_sMyMacros(ResId(STR_MYMACROS)),
    m_sProdMacros(ResId(STR_PRODMACROS))
{
    SetSelectionMode( SINGLE_SELECTION );
    OSL_TRACE("setting default node images");

    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL |
                   WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HIDESELECTION |
                   WB_HASLINES | WB_HASLINESATROOT );
    SetNodeDefaultImages();

    FreeResource();
    nMode = 0xFF;   // Alles
}

SFTreeListBox::~SFTreeListBox()
{
    OSL_TRACE("Entering SFTreeListBox::~SFTreeListBox()");
    OSL_TRACE("Leaving SFTreeListBox::~SFTreeListBox()");
    deleteAllTree();
}

void SFTreeListBox::delUserData( SvLBoxEntry* pEntry )
{
    if ( pEntry )
    {

        String text = GetEntryText( pEntry );
        OSL_TRACE("delete userdata  on node named %s",
            ::rtl::OUStringToOString( text , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        SFEntry* pUserData = (SFEntry*)pEntry->GetUserData();
        if ( pUserData )
        {
            delete pUserData;
            // TBD seem to get a Select event on node that is remove ( below )
            // so need to be able to detect that this node is not to be
            // processed in order to do this, setting userData to NULL ( must
            // be a better way to do this )
            pUserData = 0;
            pEntry->SetUserData( pUserData );
        }
    }
}

void SFTreeListBox::deleteTree( SvLBoxEntry* pEntry )
{

    OSL_TRACE("delete tree  on node named %s",
        ::rtl::OUStringToOString( GetEntryText(pEntry) , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    SvLBoxEntry* treeToRemove = pEntry;

    delUserData( pEntry );
    pEntry = FirstChild( pEntry );
    while ( pEntry )
    {
        OSL_TRACE("while: delete tree  on node named %s",
            ::rtl::OUStringToOString( GetEntryText(pEntry) , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        SvLBoxEntry* pNextEntry = NextSibling( pEntry );
        deleteTree( pEntry );
        GetModel()->Remove( pEntry );
        pEntry = pNextEntry;
    }
}

void SFTreeListBox::deleteAllTree()
{
    SvLBoxEntry* pEntry =  GetEntry( 0 );

    // TBD - below is a candidate for a destroyAllTrees method
    if ( pEntry )
    {
        while ( pEntry )
        {
            String text = GetEntryText( pEntry );
            OSL_TRACE("** ** ** call deleteTree on node named %s",
                ::rtl::OUStringToOString( text , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            SvLBoxEntry* pNextEntry = NextSibling( pEntry ) ;
            deleteTree( pEntry );
            GetModel()->Remove( pEntry );
            pEntry = pNextEntry;
        }
    }
}

void SFTreeListBox::Init( const ::rtl::OUString& language  )
{
    OSL_TRACE("Entering Init()");
    SetUpdateMode( FALSE );

    deleteAllTree();

    ::rtl::OUString userStr = ::rtl::OUString::createFromAscii("user");
    ::rtl::OUString shareStr = ::rtl::OUString::createFromAscii("share");
    Reference< browse::XBrowseNode > rootNode;
    Reference< XComponentContext > xCtx;
        try
        {
            Reference < beans::XPropertySet > xProps(
                ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
            xCtx.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), UNO_QUERY_THROW );
            Reference< browse::XBrowseNodeFactory > xFac( xCtx->getValueByName(
                ::rtl::OUString::createFromAscii( "/singletons/drafts.com.sun.star.script.browse.theBrowseNodeFactory") ), UNO_QUERY_THROW );
            rootNode.set( xFac->getView( browse::BrowseNodeFactoryViewType::SCRIPTORGANIZER ) );
        }
        catch( Exception& e )
        {
            OSL_TRACE(" Caught some exception whilst retrieving browse nodes from factory... Exception: %s",
            ::rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            // TODO exception handling
        }
    if (  rootNode.is() )
    {
        if ( rootNode->hasChildNodes() == sal_True )
        {
            Sequence< Reference< browse::XBrowseNode > > children
                = rootNode->getChildNodes();
            // sort the children
            // this may be fixed at the XBrowseNode impl at some stage
            ::std::vector< Reference< browse::XBrowseNode > > childList;
            sal_Int32 n = 0;
            for ( n = 0; n < children.getLength(); n++ )
            {
                childList.push_back( children[ n ] );
            }
            ::std::sort( childList.begin(), childList.end(), dialogSort1 );
            for ( n = 0; n < childList.size(); n++ )
            {
                BOOL app = false;
                ::rtl::OUString uiName = childList[ n ]->getName();
                ::rtl::OUString factoryURL;
                if ( uiName.equals( userStr ) ||
                    uiName.equals( shareStr ) )
                {
                    app = true;
                    if ( uiName.equals( userStr ) )
                    {
                        uiName = m_sMyMacros;
                    }
                    else
                    {
                        uiName = m_sProdMacros;
                    }
                }
                else
                {
                    Reference<XInterface> xDocumentModel = getDocumentModel(xCtx, uiName );
                    if ( xDocumentModel.is() )
                    {
                        Reference< ::drafts::com::sun::star::frame::XModuleManager >
                            xModuleManager(
                                xCtx->getServiceManager()
                                    ->createInstanceWithContext(
                                        ::rtl::OUString::createFromAscii("drafts." // xxx todo
                                              "com.sun.star.frame.ModuleManager"),
                                        xCtx ),
                                    UNO_QUERY_THROW );
                        Reference<container::XNameAccess> xModuleConfig(
                            xModuleManager, UNO_QUERY_THROW );
                        // get the long name of the document:
                        ::rtl::OUString appModule( xModuleManager->identify(
                                            xDocumentModel ) );
                        Sequence<beans::PropertyValue> moduleDescr;
                        Any aAny = xModuleConfig->getByName(appModule);
                        if( sal_True != ( aAny >>= moduleDescr ) )
                        {
                            throw RuntimeException(::rtl::OUString::createFromAscii("SFTreeListBox::Init: failed to get PropertyValue"), Reference< XInterface >());
                        }
                        beans::PropertyValue const * pmoduleDescr =
                            moduleDescr.getConstArray();
                        for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                        {
                            if (pmoduleDescr[ pos ].Name.equalsAsciiL(
                                    RTL_CONSTASCII_STRINGPARAM(
                                        "ooSetupFactoryEmptyDocumentURL") ))
                            {
                                pmoduleDescr[ pos ].Value >>= factoryURL;
                                OSL_TRACE("factory url for doc images is %s",
                                ::rtl::OUStringToOString( factoryURL , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                                break;
                            }
                        }
                    }
                }

                ::rtl::OUString lang( language );
                Reference< browse::XBrowseNode > langEntries =
                    getLangNodeFromRootNode( childList[ n ], lang );
                SvLBoxEntry* pBasicManagerRootEntry = insertEntry(
                                       uiName,
                                        app == true ? IMGID_APPICON : IMGID_DOCUMENT,
                                        0, true,
                                    std::auto_ptr< SFEntry >(new SFEntry( OBJTYPE_SFROOT, langEntries )), factoryURL );

            }
        }

    }
    SetUpdateMode( TRUE );
    OSL_TRACE("Leaving Init()");
}

Reference< XInterface  >
SFTreeListBox::getDocumentModel( Reference< XComponentContext >& xCtx, ::rtl::OUString& docName )
{
    Reference< XInterface > xModel;
    Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();
    Reference< frame::XDesktop > desktop (
        mcf->createInstanceWithContext(
            ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"),                 xCtx ),
            UNO_QUERY );

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    sal_Int32 docIndex = 0;
    while (components->hasMoreElements())
    {
        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            ::rtl::OUString sTdocUrl = xModelToDocTitle( model );
            if( sTdocUrl.equals( docName ) )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

::rtl::OUString SFTreeListBox::xModelToDocTitle( const Reference< frame::XModel >& xModel )
{
    // Set a default name, this should never be seen.
    ::rtl::OUString docNameOrURL =
        ::rtl::OUString::createFromAscii("Unknown");
    if ( xModel.is() )
    {
        ::rtl::OUString tempName;
        try
        {
            Reference< beans::XPropertySet > propSet( xModel->getCurrentController()->getFrame(), UNO_QUERY );
            if ( propSet.is() )
            {
                if ( sal_True == ( propSet->getPropertyValue(::rtl::OUString::createFromAscii( "Title" ) ) >>= tempName ) )
                {
                    docNameOrURL = tempName;
                    if ( xModel->getURL().getLength() == 0 )
                    {
                        // process "UntitledX - YYYYYYYY"
                        // to get UntitledX
                        sal_Int32 pos = 0;
                        docNameOrURL = tempName.getToken(0,' ',pos);
                        OSL_TRACE("xModelToDocTitle() Title for document is %s.",
                            ::rtl::OUStringToOString( docNameOrURL,
                                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                    else
                    {
                        Reference< document::XDocumentInfoSupplier >  xDIS( xModel, UNO_QUERY_THROW );
                        Reference< beans::XPropertySet > xProp (xDIS->getDocumentInfo(),  UNO_QUERY_THROW );
                        Any aTitle = xProp->getPropertyValue(::rtl::OUString::createFromAscii( "Title" ) );

                        aTitle >>= docNameOrURL;
                        if ( docNameOrURL.getLength() == 0 )
                        {
                            docNameOrURL =  parseLocationName( xModel->getURL() );
                        }
                    }
                }
            }
        }
        catch ( Exception& e )
        {
            OSL_TRACE("MiscUtils::xModelToDocTitle() exception thrown: !!! %s",
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }

    }
    else
    {
        OSL_TRACE("MiscUtils::xModelToDocTitle() doc model is null" );
    }
    return docNameOrURL;
}

::rtl::OUString SFTreeListBox::parseLocationName( const ::rtl::OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString::createFromAscii( "/" ) );

    if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
    {
        temp = temp.copy( lastSlashIndex + 1 );
    }
    // maybe we should throw here!!!
    else
    {
        OSL_TRACE("Something wrong with name, perhaps we should throw an exception");
    }
    return temp;
}

Reference< browse::XBrowseNode >
SFTreeListBox::getLangNodeFromRootNode( Reference< browse::XBrowseNode >& rootNode, ::rtl::OUString& language )
{
    OSL_TRACE("Entering getLangNodeFromRootNode");
    Sequence < Reference< browse::XBrowseNode > > children = rootNode->getChildNodes();
    Reference< browse::XBrowseNode > langNode;
    for ( sal_Int32 n = 0; n < children.getLength(); n++ )
    {
        if ( children[ n ]->getName().equals( language ) )
        {
            langNode = children[ n ];
            break;
        }
    }
    OSL_TRACE("Leaving getLangNodeFromRootNode");
    return langNode;
}

void SFTreeListBox:: RequestSubEntries( SvLBoxEntry* pRootEntry, Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >& node )
{
    OSL_TRACE("RequestSubEntries ");
    if (! node.is() )
    {
        OSL_TRACE("root node not not available");
        return;
    }
    OSL_TRACE("Processing node %s",
        ::rtl::OUStringToOString( node->getName() , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    Sequence< Reference< browse::XBrowseNode > > children = node->getChildNodes();
    ::std::vector< Reference< browse::XBrowseNode > > childList;
    sal_Int32 n = 0;
    for ( n = 0; n < children.getLength(); n++ )
    {
        childList.push_back( children[ n ] );
    }
    // sort the children
    // this may be fixed at the XBrowseNode impl at some stage
    ::std::sort( childList.begin(), childList.end(), dialogSort2 );
    for ( n = 0; n < childList.size(); n++ )
    {
        if (  childList[ n ]->getType() !=  browse::BrowseNodeTypes::SCRIPT)
        {
            OSL_TRACE("******   Creating container entry for %s",
            ::rtl::OUStringToOString( childList[ n ]->getName() , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                SvLBoxEntry* container = insertEntry( childList[ n ]->getName(), IMGID_LIB, pRootEntry, true, std::auto_ptr< SFEntry >(new SFEntry( OBJTYPE_SCRIPTCONTAINER, childList[ n ] )));
        }
        else
        {
            OSL_TRACE("RequestSubEntries no children");
            if ( childList[ n ]->getType() == browse::BrowseNodeTypes::SCRIPT )
            {
                OSL_TRACE("creating node for script %s",
                    ::rtl::OUStringToOString( childList[ n ]->getName() , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                insertEntry( childList[ n ]->getName(), IMGID_MODULE, pRootEntry, false, std::auto_ptr< SFEntry >(new SFEntry( OBJTYPE_METHOD, childList[ n ] )));

            }
        }
    }
}

void SFTreeListBox::UpdateEntries()
{
    OSL_TRACE("Update Entries()");
}


SvLBoxEntry* SFTreeListBox::FindEntry( SvLBoxEntry* pParent, const String& rText, BYTE nType )
{
    OSL_TRACE("FindEntry");
    return 0;
}

long SFTreeListBox::ExpandingHdl()
{
    OSL_TRACE("ExpandingHdl");
    OSL_TRACE("expanding depth: %d",GetModel()->GetDepth( GetHdlEntry() ));
    return TRUE;
}

void SFTreeListBox::ExpandAllTrees()
{
    OSL_TRACE("Entering ExpandAllTrees");
    OSL_TRACE("Leaving ExpandAllTrees");
}

SvLBoxEntry * SFTreeListBox::insertEntry(
    String const & rText, USHORT nBitmap, SvLBoxEntry * pParent,
    bool bChildrenOnDemand, std::auto_ptr< SFEntry > aUserData, ::rtl::OUString factoryURL )
{
    SvLBoxEntry * p;
    if( nBitmap == IMGID_DOCUMENT && factoryURL.getLength() > 0 )
    {
        OSL_TRACE("=================================> adding icons for document");
        Image aImage = SvFileInformationManager::GetFileImage(
            INetURLObject(factoryURL), false,
            BMP_COLOR_NORMAL );
        Image aHCImage = SvFileInformationManager::GetFileImage(
            INetURLObject(factoryURL), false,
            BMP_COLOR_HIGHCONTRAST );
        p = InsertEntry(
            rText, aImage, aImage, pParent, bChildrenOnDemand, LIST_APPEND,
            aUserData.release()); // XXX possible leak
        SetExpandedEntryBmp(p, aHCImage, BMP_COLOR_HIGHCONTRAST);
        SetCollapsedEntryBmp(p, aHCImage, BMP_COLOR_HIGHCONTRAST);
    }
    else
    {
        p = insertEntry( rText, nBitmap, pParent, bChildrenOnDemand, aUserData );
    }
    return p;
}

SvLBoxEntry * SFTreeListBox::insertEntry(
    String const & rText, USHORT nBitmap, SvLBoxEntry * pParent,
    bool bChildrenOnDemand, std::auto_ptr< SFEntry > aUserData )
{
    Image aHCImage, aImage;
    if( nBitmap == IMGID_APPICON )
    {
        aImage = m_hdImage;
        aHCImage = m_hdImage_hc;
    }
    else
    {
        aImage = m_aImagesNormal.GetImage(nBitmap);
        aHCImage = m_aImagesHighContrast.GetImage(nBitmap);
    }
    SvLBoxEntry * p = InsertEntry(
        rText, aImage, aImage, pParent, bChildrenOnDemand, LIST_APPEND,
        aUserData.release()); // XXX possible leak
    SetExpandedEntryBmp(p, aHCImage, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(p, aHCImage, BMP_COLOR_HIGHCONTRAST);
    return p;
}

void SFTreeListBox::setEntryBitmap(SvLBoxEntry * pEntry, USHORT nBitmap)
{
    Image aImage(m_aImagesNormal.GetImage(nBitmap));
    SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
    SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
    aImage = m_aImagesHighContrast.GetImage(nBitmap);
    SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_HIGHCONTRAST);
}

void __EXPORT SFTreeListBox::RequestingChilds( SvLBoxEntry* pEntry )
{
    OSL_TRACE("Entering Requesting Childs ");

    SFEntry* userData = 0;
    if ( !pEntry )
    {
        return;
    }
    userData = (SFEntry*)pEntry->GetUserData();

    Reference< browse::XBrowseNode > node;
    if ( userData && !userData->isLoaded() )
    {
        node = userData->GetNode();
        RequestSubEntries( pEntry, node );
        userData->setLoaded();
    }
}

void __EXPORT SFTreeListBox::ExpandedHdl()
{
        OSL_TRACE("BasicTreeListBox::ExpandedHdl()");
/*        SvLBoxEntry* pEntry = GetHdlEntry();
        DBG_ASSERT( pEntry, "Was wurde zugeklappt?" );

        if ( !IsExpanded( pEntry ) && pEntry->HasChildsOnDemand() )
        {
                SvLBoxEntry* pChild = FirstChild( pEntry );
                while ( pChild )
                {
                        GetModel()->Remove( pChild );   // Ruft auch den DTOR
                        pChild = FirstChild( pEntry );
                }
        }*/
}

// ----------------------------------------------------------------------------
// InputDialog ------------------------------------------------------------
// ----------------------------------------------------------------------------
InputDialog::InputDialog(Window * pParent, USHORT nMode )
    : ModalDialog( pParent, SVX_RES( RID_DLG_NEWLIB ) ),
        aText( this, ResId( FT_NEWLIB ) ),
        aEdit( this, ResId( ED_LIBNAME ) ),
        aOKButton( this, ResId( PB_OK ) ),
        aCancelButton( this, ResId( PB_CANCEL ) )
{
    aEdit.GrabFocus();
    if ( nMode == INPUTMODE_NEWLIB )
    {
        SetText( String( ResId( STR_NEWLIB ) ) );
    }
    else if ( nMode == INPUTMODE_NEWMACRO )
    {
        SetText( String( ResId( STR_NEWMACRO ) ) );
        aText.SetText( String( ResId( STR_FT_NEWMACRO ) ) );
    }
    else if ( nMode == INPUTMODE_RENAME )
    {
        SetText( String( ResId( STR_RENAME ) ) );
        aText.SetText( String( ResId( STR_FT_RENAME ) ) );
    }
    FreeResource();

    // some resizing so that the text fits
    Point point, newPoint;
    Size siz, newSiz;
    long gap;

    USHORT style = TEXT_DRAW_MULTILINE | TEXT_DRAW_TOP |
                   TEXT_DRAW_LEFT | TEXT_DRAW_WORDBREAK;

    // get dimensions of dialog instructions control
    point = aText.GetPosPixel();
    siz = aText.GetSizePixel();

    // get dimensions occupied by text in the control
    Rectangle rect =
        GetTextRect( Rectangle( point, siz ), aText.GetText(), style );
    newSiz = rect.GetSize();

    // the gap is the difference between the text width and its control width
    gap = siz.Height() - newSiz.Height();

    //resize the text field
    newSiz = Size( siz.Width(), siz.Height() - gap );
    aText.SetSizePixel( newSiz );

    //move the OK & cancel buttons
    point = aEdit.GetPosPixel();
    newPoint = Point( point.X(), point.Y() - gap );
    aEdit.SetPosPixel( newPoint );

}

InputDialog::~InputDialog()
{
}
// ----------------------------------------------------------------------------
// ScriptOrgDialog ------------------------------------------------------------
// ----------------------------------------------------------------------------
SvxScriptOrgDialog::SvxScriptOrgDialog( Window* pParent, ResMgr* pBasResMgr, ::rtl::OUString language )
    :   SfxModalDialog( pParent, SVX_RES( RID_DLG_SCRIPTORGANIZER ) ),
        aScriptsTxt( this, ResId( SF_TXT_SCRIPTS ) ),
        aScriptsBox( this, ResId( SF_CTRL_SCRIPTSBOX ), pBasResMgr ),
        aRunButton( this, ResId( SF_PB_RUN ) ),
        aCloseButton( this, ResId( SF_PB_CLOSE ) ),
        aCreateButton( this, ResId( SF_PB_CREATE ) ),
        aEditButton( this, ResId( SF_PB_EDIT ) ),
        aRenameButton(this, ResId( SF_PB_RENAME ) ),
        aDelButton( this, ResId( SF_PB_DEL ) ),
        aHelpButton( this, ResId( SF_PB_HELP ) ),
        m_sLanguage( language ),
        m_delErrStr( ResId( RID_SVXSTR_DELFAILED ) ),
        m_delErrTitleStr( ResId( RID_SVXSTR_DELFAILED_TITLE ) ),
        m_delQueryStr( ResId( RID_SVXSTR_DELQUERY ) ),
        m_delQueryTitleStr( ResId( RID_SVXSTR_DELQUERY_TITLE ) ) ,
        m_createErrStr( ResId ( RID_SVXSTR_CREATEFAILED ) ),
        m_createDupStr( ResId ( RID_SVXSTR_CREATEFAILEDDUP ) ),
        m_createErrTitleStr( ResId( RID_SVXSTR_CREATEFAILED_TITLE ) ),
        m_renameErrStr( ResId ( RID_SVXSTR_RENAMEFAILED ) ),
        m_renameErrTitleStr( ResId( RID_SVXSTR_RENAMEFAILED_TITLE ) )
{
    // must be a neater way to deal with the strings than as above
    // append the language to the dialog title
    String winTitle( GetText() );
    winTitle.SearchAndReplace( String::CreateFromAscii( "%MACROLANG" ), language.pData->buffer );
    SetText( winTitle );

    FreeResource();

    aScriptsBox.SetSelectHdl( LINK( this, SvxScriptOrgDialog, ScriptSelectHdl ) );
    aRunButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    aRenameButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    aEditButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    aCreateButton.SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );

    aRunButton.Disable();
    aRenameButton.Disable();
    aEditButton.Disable();
    aDelButton.Disable();
    aCreateButton.Disable();

    aScriptsBox.Init( m_sLanguage );
    RestorePreviousSelection();
}

__EXPORT SvxScriptOrgDialog::~SvxScriptOrgDialog()
{
    // clear the SelectHdl so that it isn't called during the dtor
    aScriptsBox.SetSelectHdl( Link() );
};

short SvxScriptOrgDialog::Execute()
{

    SfxObjectShell *pDoc = SfxObjectShell::GetFirst();

    // force load of MSPs for all documents
    while ( pDoc )
    {
        Reference< provider::XScriptProviderSupplier > xSPS =
            Reference< provider::XScriptProviderSupplier >
                                        ( pDoc->GetModel(), UNO_QUERY );
        if ( xSPS.is() )
        {
            Reference< provider::XScriptProvider > ScriptProvider =
            xSPS->getScriptProvider();
        }

        pDoc = SfxObjectShell::GetNext(*pDoc);
    }
    aScriptsBox.ExpandAllTrees();

    Window* pPrevDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    short nRet = ModalDialog::Execute();
    Application::SetDefDialogParent( pPrevDlgParent );
    return nRet;
}

void SvxScriptOrgDialog::EnableButton( Button& rButton, BOOL bEnable )
{
    OSL_TRACE("Entering SvxScriptOrgDialog::EnableButton()");
    OSL_TRACE("Leaving SvxScriptOrgDialog::EnableButton()");
}

void SvxScriptOrgDialog::CheckButtons( Reference< browse::XBrowseNode >& node )
{
    OSL_TRACE("Entering SvxScriptOrgDialog::CheckButtons()");
    if ( node.is() )
    {
        OSL_TRACE("got ok node %d",node->getType());
        OSL_TRACE("node name: %s", ::rtl::OUStringToOString( node->getName(), RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        if ( node->getType() == browse::BrowseNodeTypes::SCRIPT)
        {
            aRunButton.Enable();
        }
        else
        {
            aRunButton.Disable();
        }
        Reference< beans::XPropertySet > xProps( node, UNO_QUERY );

        if ( !xProps.is() )
        {
            OSL_TRACE("no props");
            aEditButton.Disable();
            aDelButton.Disable();
            aCreateButton.Disable();
            aRunButton.Disable();
            return;
        }

        ::rtl::OUString sName;
        sName = String::CreateFromAscii("Editable")  ;

        if ( getBoolProperty( xProps, sName ) )
        {
            aEditButton.Enable();
        }
        else
        {
            aEditButton.Disable();
        }

        sName = String::CreateFromAscii("Deletable")  ;

        if ( getBoolProperty( xProps, sName ) )
        {
            aDelButton.Enable();
        }
        else
        {
            aDelButton.Disable();
        }

        sName = String::CreateFromAscii("Creatable")  ;

        if ( getBoolProperty( xProps, sName ) )
        {
            aCreateButton.Enable();
        }
        else
        {
            aCreateButton.Disable();
        }

        sName = String::CreateFromAscii("Renamable")  ;

        if ( getBoolProperty( xProps, sName ) )
        {
            aRenameButton.Enable();
        }
        else
        {
            aRenameButton.Disable();
        }
    }
    else
    {
        OSL_TRACE("No node info available for selected");
        // no node info available, disable all configurable actions
        aDelButton.Disable();
        aCreateButton.Disable();
        aEditButton.Disable();
        aRunButton.Disable();
        aRenameButton.Disable();
    }
    OSL_TRACE("Leaving SvxScriptOrgDialog::CheckButtons()");
}

IMPL_LINK_INLINE_START( SvxScriptOrgDialog, MacroDoubleClickHdl, SvTreeListBox *, EMPTYARG )
{
    OSL_TRACE("Entering SvxScriptOrgDialog::MacroDoubleClickHdl()");
    OSL_TRACE("Leaving SvxScriptOrgDialog::MacroDoubleClickHdl()");
    return 0;
}

IMPL_LINK_INLINE_END( SvxScriptOrgDialog, MacroDoubleClickHdl, SvTreeListBox *, EMPTYARG )

IMPL_LINK( SvxScriptOrgDialog, ScriptSelectHdl, SvTreeListBox *, pBox )
{
    OSL_TRACE("Entering SvxScriptOrgDialog::ScriptSelectHdl()");
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
    {
        OSL_TRACE("ScriptSelectHdl entry not selected");
        return 0;
    }

    SvLBoxEntry* pEntry = pBox->GetHdlEntry();

    SFEntry* userData = 0;
    if ( !pEntry )
    {
        OSL_TRACE("No entry for selected position");
        return 0;
    }
    userData = (SFEntry*)pEntry->GetUserData();

    Reference< browse::XBrowseNode > node;
    if ( userData )
    {
        OSL_TRACE("Got userdata");
              node = userData->GetNode();
        CheckButtons( node );
    }

    OSL_TRACE("Leaving SvxScriptOrgDialog::ScriptSelectHdl()");
    return 0;
}

IMPL_LINK( SvxScriptOrgDialog, ButtonHdl, Button *, pButton )
{
    OSL_TRACE("Entering SvxScriptOrgDialog::ButtonHdl()");

    if ( pButton == &aCloseButton )
    {
        StoreCurrentSelection();
        EndDialog( 0 );
    }
    if ( pButton == &aEditButton ||
            pButton == &aCreateButton ||
            pButton == &aDelButton ||
            pButton == &aRunButton ||
            pButton == &aRenameButton )

    {
        if ( aScriptsBox.IsSelected( aScriptsBox.GetHdlEntry() ) )
        {
            SvLBoxEntry* pEntry = aScriptsBox.GetHdlEntry();
            SFEntry* userData = 0;
            if ( !pEntry )
            {
                OSL_TRACE("No entry for selected position");
                return 0;
            }
            userData = (SFEntry*)pEntry->GetUserData();
            if ( userData )
            {
                Reference< browse::XBrowseNode > node;
                node = userData->GetNode();
                if ( !node.is() )
                {
                    OSL_TRACE("No valid node ");
                    return 0;
                }
                if ( pButton == &aRunButton )
                {
                    OSL_TRACE("run button pressed ");
                    ::rtl::OUString tmpString;
                    Reference< beans::XPropertySet > xProp( node, UNO_QUERY );
                    Reference< provider::XScriptProvider > mspNode;
                    if( !xProp.is() )
                    {
                        OSL_TRACE("no xprop ");
                        return 0;
                    }
                    if ( pEntry )
                    {
                        SvLBoxEntry* pLibEntry = aScriptsBox.GetParent( pEntry );
                        if ( pLibEntry )
                        {
                            SvLBoxEntry* pMSPEntry = aScriptsBox.GetParent( pLibEntry );
                            if ( pMSPEntry )
                            {
                    SFEntry* mspUserData = (SFEntry*)pMSPEntry->GetUserData();
                                mspNode.set( mspUserData->GetNode() , UNO_QUERY );

                            }
                        }

                    }
                    xProp->getPropertyValue( String::CreateFromAscii("URI" ) ) >>= tmpString;
                    const String scriptURL( tmpString );

                    if ( mspNode.is() )
                    {
                        Reference< provider::XScript > xScript(
                            mspNode->getScript( scriptURL ), UNO_QUERY );
                        if ( xScript.is() )
                        {
                            const Sequence< Any > args(0);
                            Any aRet;
                            Sequence< sal_Int16 > outIndex;
                            Sequence< Any > outArgs( 0 );
                            try
                            {
                                aRet = xScript->invoke( args, outIndex, outArgs );
                            }
                            catch ( reflection::InvocationTargetException& ite )
                            {
                                ::com::sun::star::uno::Any a = makeAny(ite);
                                ShowErrorDialog(a);
                            }
                            catch ( provider::ScriptFrameworkErrorException& ite )
                            {
                                ::com::sun::star::uno::Any a = makeAny(ite);
                                ShowErrorDialog(a);
                            }
                            catch ( RuntimeException& re )
                            {
                                ::com::sun::star::uno::Any a = makeAny(re);
                                ShowErrorDialog(a);
                            }
                            catch ( Exception& e )
                            {
                                ::com::sun::star::uno::Any a = makeAny(e);
                                ShowErrorDialog(a);
                            }
                        }
                    }
                    StoreCurrentSelection();
                    EndDialog( 0 );
                }
                else if ( pButton == &aEditButton )
                {
                    Reference< script::XInvocation > xInv( node, UNO_QUERY );
                    if ( xInv.is() )
                    {
                        StoreCurrentSelection();
                        EndDialog( 0 );
                        Sequence< Any > args(0);
                        Sequence< Any > outArgs( 0 );
                        Sequence< sal_Int16 > outIndex;
                        try
                        {
                            // ISSUE need code to run script here
                            xInv->invoke( ::rtl::OUString::createFromAscii( "Editable" ), args, outIndex, outArgs );
                        }
                        catch( Exception& e )
                        {
                            OSL_TRACE("Caught exception trying to invoke %s", ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                        }
                    }
                }
                else if ( pButton == &aCreateButton )
                {
                    OSL_TRACE("New button pushed");
                    createEntry( pEntry );
                }
                else if ( pButton == &aDelButton )
                {
                    OSL_TRACE("Delete button pushed");
                    deleteEntry( pEntry );
                }
                else if ( pButton == &aRenameButton )
                {
                    OSL_TRACE("Rename button pushed");
                    renameEntry( pEntry );
                }
            }
        }
    }
    OSL_TRACE("Leaving SvxScriptOrgDialog::ButtonHdl()");
    return 0;
}

Reference< browse::XBrowseNode > SvxScriptOrgDialog::getBrowseNode( SvLBoxEntry* pEntry )
{
    Reference< browse::XBrowseNode > node;
    if ( pEntry )
    {
        SFEntry* userData = (SFEntry*)pEntry->GetUserData();
        if ( userData )
        {
            node = userData->GetNode();
        }
    }

    return node;
}

void SvxScriptOrgDialog::createEntry( SvLBoxEntry* pEntry )
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        ::rtl::OUString aNewName;
        ::rtl::OUString aNewStdName;
        USHORT nMode = INPUTMODE_NEWLIB;
        if( aScriptsBox.GetModel()->GetDepth( pEntry ) == 0 )
        {
            aNewStdName = ::rtl::OUString::createFromAscii( "Library" ) ;
        }
        else
        {
            aNewStdName = ::rtl::OUString::createFromAscii( "Macro" ) ;
            nMode = INPUTMODE_NEWMACRO;
        }
        //do we need L10N for this? ie somethng like:
        //String aNewStdName( ResId( STR_STDMODULENAME ) );
        BOOL bValid = FALSE;
        USHORT i = 1;

        Sequence< Reference< browse::XBrowseNode > > childNodes;
        // no children => ok to create Parcel1 or Script1 without checking
        if( node->hasChildNodes() == sal_False )
        {
            OSL_TRACE("has no childnodes");
            aNewName = aNewStdName;
            aNewName += String::CreateFromInt32( i );
            bValid = TRUE;
        }
        else
        {
            childNodes = node->getChildNodes();
        }

        ::rtl::OUString extn;
        while ( !bValid )
        {
            aNewName = aNewStdName;
            aNewName += String::CreateFromInt32( i );
            OSL_TRACE("trying %s, %d",
                ::rtl::OUStringToOString(
                    aNewName, RTL_TEXTENCODING_ASCII_US ).pData->buffer, childNodes.getLength() );
            BOOL bFound = FALSE;
            if(childNodes.getLength() > 0 )
            {
                ::rtl::OUString nodeName = childNodes[0]->getName();
                sal_Int32 extnPos = nodeName.lastIndexOf( '.' );
                if(extnPos>0)
                    extn = nodeName.copy(extnPos);
                OSL_TRACE("extn is %s",
                    ::rtl::OUStringToOString(
                        extn, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            }
            for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
            {
                if ( (aNewName+extn).equals( childNodes[index]->getName() ) )
                {
                    OSL_TRACE("has childnode %d",i);
                    bFound = TRUE;
                    break;
                }
            }
            if( bFound )
            {
                i++;
            }
            else
            {
                OSL_TRACE("valid for %d",i);
                bValid = TRUE;
            }
        }

        OSL_TRACE("about to popup dialog");
        std::auto_ptr< InputDialog > xNewDlg( new InputDialog( static_cast<Window*>(this), nMode ) );
        xNewDlg->SetObjectName( aNewName );

        do
        {
            OSL_TRACE("about to popup dialog (really)");
            if ( xNewDlg->Execute() && xNewDlg->GetObjectName().Len() )
            {
                ::rtl::OUString aUserSuppliedName = xNewDlg->GetObjectName();
                bValid = TRUE;
                for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
                {
                    if ( (aUserSuppliedName+extn).equals( childNodes[index]->getName() ) )
                    {
                        bValid = FALSE;
                        String aError( m_createErrStr );
                        aError.Append( m_createDupStr );
                        ErrorBox aErrorBox( static_cast<Window*>(this), WB_OK | RET_OK, aError );
                        aErrorBox.SetText( m_createErrTitleStr );
                        aErrorBox.Execute();
                        xNewDlg->SetObjectName( aNewName );
                        break;
                    }
                }
                if( bValid )
                    aNewName = aUserSuppliedName;
            }
            else
            {
                // user hit cancel or hit OK with nothing in the editbox

                return;
            }
        }
        while ( !bValid );

        // open up parent node (which ensures it's loaded)
        aScriptsBox.RequestingChilds( pEntry );

        OSL_TRACE("create for other language using XInvocation....");
        OSL_TRACE("creating with default name= %s",::rtl::OUStringToOString( aNewName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        Sequence< Any > args( 1 );
        args[ 0 ] <<= aNewName;
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( ::rtl::OUString::createFromAscii( "Creatable" ), args, outIndex, outArgs );
            Reference< browse::XBrowseNode > newNode( aResult, UNO_QUERY );
            aChildNode = newNode;

        }
        catch( Exception& e )
        {
            OSL_TRACE("Caught exception trying to Create %s",
                ::rtl::OUStringToOString(
                    e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }
    if ( aChildNode.is() )
    {
        OSL_TRACE("Create seemed to succeed ");
        String aChildName = aChildNode->getName();
        SvLBoxEntry* pNewEntry = NULL;

        // ISSUE do we need to remove all entries for parent
        // to achieve sort? Just need to determine position
        // SvTreeListBox::InsertEntry can take position arg
        // -- Basic doesn't do this on create.
        // Suppose we could avoid this too. -> created nodes are
        // not in alphabetical order
        if ( aChildNode->getType() == browse::BrowseNodeTypes::SCRIPT )
        {
            pNewEntry = aScriptsBox.insertEntry( aChildName,
                    IMGID_MODULE, pEntry, false, std::auto_ptr< SFEntry >(new SFEntry( OBJTYPE_METHOD, aChildNode ) ) );

        }
        else
        {
            pNewEntry = aScriptsBox.insertEntry( aChildName,
                IMGID_LIB, pEntry, false, std::auto_ptr< SFEntry >(new SFEntry( OBJTYPE_SCRIPTCONTAINER, aChildNode ) ) );
                        // If the Parent is not loaded then set to
                        // loaded, this will prevent RequestingChilds ( called
                        // from vcl via RequestingChilds ) from
                        // creating new ( duplicate ) children
                        SFEntry* userData = (SFEntry*)pEntry->GetUserData();
                        if ( userData &&  !userData->isLoaded() )
                        {
                            userData->setLoaded();
                        }
        }
        aScriptsBox.SetCurEntry( pNewEntry );
        aScriptsBox.Select( aScriptsBox.GetCurEntry() );

    }
    else
    {
        OSL_TRACE("Create seemed to fail");
        //ISSUE L10N & message from exception?
        String aError( m_createErrStr );
        ErrorBox aErrorBox( static_cast<Window*>(this), WB_OK | RET_OK, aError );
        aErrorBox.SetText( m_createErrTitleStr );
        aErrorBox.Execute();
    }
}

void SvxScriptOrgDialog::renameEntry( SvLBoxEntry* pEntry )
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        ::rtl::OUString aNewName = node->getName();
        sal_Int32 extnPos = aNewName.lastIndexOf( '.' );
        ::rtl::OUString extn;
        if(extnPos>0)
        {
            extn = aNewName.copy(extnPos);
            aNewName = aNewName.copy(0,extnPos);
        }
        USHORT nMode = INPUTMODE_RENAME;

        OSL_TRACE("about to popup dialog");
        std::auto_ptr< InputDialog > xNewDlg( new InputDialog( static_cast<Window*>(this), nMode ) );
        xNewDlg->SetObjectName( aNewName );

        BOOL bValid;
        do
        {
            OSL_TRACE("about to popup dialog (really)");
            if ( xNewDlg->Execute() && xNewDlg->GetObjectName().Len() )
            {
                ::rtl::OUString aUserSuppliedName = xNewDlg->GetObjectName();
                bValid = TRUE;
                /*
                for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
                {
                    if ( (aUserSuppliedName+extn).equals( childNodes[index]->getName() ) )
                    {
                        bValid = FALSE;
                        String aError( m_createErrStr );
                        aError.Append( m_createDupStr );
                        ErrorBox aErrorBox( static_cast<Window*>(this), WB_OK | RET_OK, aError );
                        aErrorBox.SetText( m_createErrTitleStr );
                        aErrorBox.Execute();
                        xNewDlg->SetObjectName( aNewName );
                        break;
                    }
                } */
                if( bValid )
                    aNewName = aUserSuppliedName;
            }
            else
            {
                // user hit cancel or hit OK with nothing in the editbox
                return;
            }
        }
        while ( !bValid );

        OSL_TRACE("create for other language using XInvocation....");
        OSL_TRACE("creating with default name= %s",::rtl::OUStringToOString( aNewName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        Sequence< Any > args( 1 );
        args[ 0 ] <<= aNewName;
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( ::rtl::OUString::createFromAscii( "Renamable" ), args, outIndex, outArgs );
            Reference< browse::XBrowseNode > newNode( aResult, UNO_QUERY );
            aChildNode = newNode;

        }
        catch( Exception& e )
        {
            OSL_TRACE("Caught exception trying to Rename %s",
                ::rtl::OUStringToOString(
                    e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }
    if ( aChildNode.is() )
    {
        aScriptsBox.SetEntryText( pEntry, aChildNode->getName() );
        aScriptsBox.SetCurEntry( pEntry );
        aScriptsBox.Select( aScriptsBox.GetCurEntry() );

    }
    else
    {
        OSL_TRACE("Rename seemed to fail");
        //ISSUE L10N & message from exception?
        String aError( m_renameErrStr );
        ErrorBox aErrorBox( static_cast<Window*>(this), WB_OK | RET_OK, aError );
        aErrorBox.SetText( m_renameErrTitleStr );
        aErrorBox.Execute();
    }
}
void SvxScriptOrgDialog::deleteEntry( SvLBoxEntry* pEntry )
{
    sal_Bool result = sal_False;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    // ISSUE L10N string & can we centre list?
    String aQuery( m_delQueryStr );
    aQuery.Append( getListOfChildren( node, 0 ) );
    QueryBox aQueryBox( static_cast<Window*>(this), WB_YES_NO | WB_DEF_YES, aQuery );
    aQueryBox.SetText( m_delQueryTitleStr );
    if ( aQueryBox.Execute() == RET_NO )
    {
        return;
    }

    Reference< script::XInvocation > xInv( node, UNO_QUERY );
    if ( xInv.is() )
    {
        OSL_TRACE("delete for other language using XInvocation....");
        Sequence< Any > args( 0 );
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( ::rtl::OUString::createFromAscii( "Deletable" ), args, outIndex, outArgs );
            aResult >>= result; // or do we just assume true if no exception ?
        }
        catch( Exception& e )
        {
            OSL_TRACE("Caught exception trying to delete %s",
                ::rtl::OUStringToOString(
                    e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }

    if ( result == sal_True )
    {
        OSL_TRACE("Delete worked, remove nodes from ui");
        aScriptsBox.deleteTree( pEntry );
        aScriptsBox.GetModel()->Remove( pEntry );
    }
    else
    {
        OSL_TRACE("Delete failed");
        //ISSUE L10N & message from exception?
        ErrorBox aErrorBox( static_cast<Window*>(this), WB_OK | RET_OK, m_delErrStr );
        aErrorBox.SetText( m_delErrTitleStr );
        aErrorBox.Execute();
    }

}

BOOL SvxScriptOrgDialog::getBoolProperty( Reference< beans::XPropertySet >& xProps,
                ::rtl::OUString& propName )
{
    OSL_TRACE("Entering getBoolProperty");
    BOOL result = false;
    try
    {
        sal_Bool bTemp = sal_False;
        xProps->getPropertyValue( propName ) >>= bTemp;
        result = ( bTemp == sal_True );
    }
    catch ( Exception& e )
    {
        OSL_TRACE("caught exception getBoolProperty");
        return result;
    }
    OSL_TRACE("Leaving getBoolProperty");
    return result;
}

String SvxScriptOrgDialog::getListOfChildren( Reference< browse::XBrowseNode > node, int depth )
{
    String result;
    result.Append( String::CreateFromAscii( "\n" ) );
    for( int i=0;i<=depth;i++ )
    {
        result.Append( String::CreateFromAscii( "\t" ) );
    }
    result.Append( String( node->getName() ) );
    if ( node->hasChildNodes() == sal_True )
    {
        Sequence< Reference< browse::XBrowseNode > > children
            = node->getChildNodes();
        for ( sal_Int32 n = 0; n < children.getLength(); n++ )
        {
            result.Append( getListOfChildren( children[ n ] , depth+1 ) );
        }
    }
    return result;
}

Selection_hash SvxScriptOrgDialog::m_lastSelection;

void SvxScriptOrgDialog::StoreCurrentSelection()
{
    String aDescription;
    if ( aScriptsBox.IsSelected( aScriptsBox.GetHdlEntry() ) )
    {
        SvLBoxEntry* pEntry = aScriptsBox.GetHdlEntry();
        while( pEntry )
        {
            aDescription.Insert( aScriptsBox.GetEntryText( pEntry ), 0 );
            pEntry = aScriptsBox.GetParent( pEntry );
            if ( pEntry )
                aDescription.Insert( ';', 0 );
        }
        ::rtl::OUString sDesc( aDescription );
        OSL_TRACE( "entry to store is %s, for lang %s", ::rtl::OUStringToOString( sDesc , RTL_TEXTENCODING_ASCII_US ).pData->buffer,
        ::rtl::OUStringToOString( m_sLanguage , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        m_lastSelection[ m_sLanguage ] = sDesc;
    }
}

void SvxScriptOrgDialog::RestorePreviousSelection()
{
    OSL_TRACE( "entry to restore is %s, for lang %s", ::rtl::OUStringToOString(  m_lastSelection[ m_sLanguage ], RTL_TEXTENCODING_ASCII_US ).pData->buffer,
    ::rtl::OUStringToOString( m_sLanguage , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
    String aStoredEntry = String( m_lastSelection[ m_sLanguage ] );
    if( aStoredEntry.Len() <= 0 )
        return;
    SvLBoxEntry* pEntry = 0;
    USHORT nIndex = 0;
    while ( nIndex != STRING_NOTFOUND )
    {
        String aTmp( aStoredEntry.GetToken( 0, ';', nIndex ) );
        SvLBoxEntry* pTmpEntry = aScriptsBox.FirstChild( pEntry );
        ::rtl::OUString debugStr(aTmp);
        OSL_TRACE( "looking for token %s", ::rtl::OUStringToOString( debugStr , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        while ( pTmpEntry )
        {
            debugStr = ::rtl::OUString(aScriptsBox.GetEntryText( pTmpEntry ));
            OSL_TRACE( "checking %s", ::rtl::OUStringToOString( debugStr , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
            if ( aScriptsBox.GetEntryText( pTmpEntry ) == aTmp )
            {
                pEntry = pTmpEntry;
                break;
            }
            pTmpEntry = aScriptsBox.NextSibling( pTmpEntry );
        }
        if ( !pTmpEntry )
            break;
        aScriptsBox.RequestingChilds( pEntry );
    }
    aScriptsBox.SetCurEntry( pEntry );
}

BOOL SFTreeListBox::dialogSort1( Reference< browse::XBrowseNode > node1,
    Reference< browse::XBrowseNode > node2 )
{
    ::rtl::OUString userStr = ::rtl::OUString::createFromAscii("user");
    ::rtl::OUString shareStr = ::rtl::OUString::createFromAscii("share");
    if( node1->getName().equals( userStr ) )
        return true;
    if( node2->getName().equals( userStr ) )
        return false;
    if( node1->getName().equals( shareStr ) )
        return true;
    if( node2->getName().equals( shareStr ) )
        return false;
    return dialogSort2( node1, node2 );
}

BOOL SFTreeListBox::dialogSort2( Reference< browse::XBrowseNode > node1,
    Reference< browse::XBrowseNode > node2 )
{
    return ( node1->getName().compareTo( node2->getName() ) < 0 );
}

::rtl::OUString ReplaceString(
    const ::rtl::OUString& source,
    const ::rtl::OUString& token,
    const ::rtl::OUString& value )
{
    sal_Int32 pos = source.indexOf( token );

    if ( pos != -1 && value.getLength() != 0 )
    {
        return source.replaceAt( pos, token.getLength(), value );
    }
    else
    {
        return source;
    }
}

::rtl::OUString FormatErrorString(
    const ::rtl::OUString& unformatted,
    const ::rtl::OUString& language,
    const ::rtl::OUString& script,
    const ::rtl::OUString& line,
    const ::rtl::OUString& type,
    const ::rtl::OUString& message )
{
    ::rtl::OUString result = unformatted.copy( 0 );

    result = ReplaceString(
        result, ::rtl::OUString::createFromAscii( "%LANGUAGENAME" ), language );
    result = ReplaceString(
        result, ::rtl::OUString::createFromAscii( "%SCRIPTNAME" ), script );
    result = ReplaceString(
        result, ::rtl::OUString::createFromAscii( "%LINENUMBER" ), line );

    if ( type.getLength() != 0 )
    {
        result += ::rtl::OUString::createFromAscii( "\n\n" );
        result += ::rtl::OUString(ResId(RID_SVXSTR_ERROR_TYPE_LABEL, DIALOG_MGR()));
        result += ::rtl::OUString::createFromAscii( " " );
        result += type;
    }

    if ( message.getLength() != 0 )
    {
        result += ::rtl::OUString::createFromAscii( "\n\n" );
        result += ::rtl::OUString(ResId(RID_SVXSTR_ERROR_MESSAGE_LABEL, DIALOG_MGR()));
        result += ::rtl::OUString::createFromAscii( " " );
        result += message;
    }

    return result;
}

::rtl::OUString GetErrorMessage(
    const provider::ScriptErrorRaisedException& eScriptError )
{
    ::rtl::OUString unformatted =
        ::rtl::OUString( ResId( RID_SVXSTR_ERROR_AT_LINE, DIALOG_MGR() ) );

    ::rtl::OUString unknown = ::rtl::OUString::createFromAscii( "UNKNOWN" );
    ::rtl::OUString language = unknown;
    ::rtl::OUString script = unknown;
    ::rtl::OUString line = unknown;
    ::rtl::OUString type = ::rtl::OUString();
    ::rtl::OUString message = eScriptError.Message;

        if ( eScriptError.language.getLength() != 0 )
        {
            language = eScriptError.language;
        }

        if ( eScriptError.scriptName.getLength() != 0 )
        {
            script = eScriptError.scriptName;
        }

        if ( eScriptError.Message.getLength() != 0 )
        {
            message = eScriptError.Message;
        }
        if ( eScriptError.lineNum != -1 )
        {
            line = ::rtl::OUString::valueOf( eScriptError.lineNum );
            unformatted = ::rtl::OUString(
                ResId( RID_SVXSTR_ERROR_AT_LINE, DIALOG_MGR() ) );
        }
        else
        {
            unformatted = ::rtl::OUString(
            ResId( RID_SVXSTR_ERROR_RUNNING, DIALOG_MGR() ) );
        }

    return FormatErrorString(
        unformatted, language, script, line, type, message );
}

::rtl::OUString GetErrorMessage(
    const provider::ScriptExceptionRaisedException& eScriptException )
{
    ::rtl::OUString unformatted =
    ::rtl::OUString( ResId( RID_SVXSTR_EXCEPTION_AT_LINE, DIALOG_MGR() ) );

    ::rtl::OUString unknown = ::rtl::OUString::createFromAscii( "UNKNOWN" );
    ::rtl::OUString language = unknown;
    ::rtl::OUString script = unknown;
    ::rtl::OUString line = unknown;
    ::rtl::OUString type = unknown;
    ::rtl::OUString message = eScriptException.Message;

    if ( eScriptException.language.getLength() != 0 )
    {
        language = eScriptException.language;
    }
    if ( eScriptException.scriptName.getLength() != 0 )
    {
        script = eScriptException.scriptName;
    }

    if ( eScriptException.Message.getLength() != 0 )
    {
        message = eScriptException.Message;
    }

    if ( eScriptException.lineNum != -1 )
    {
        line = ::rtl::OUString::valueOf( eScriptException.lineNum );
        unformatted = ::rtl::OUString(
            ResId( RID_SVXSTR_EXCEPTION_AT_LINE, DIALOG_MGR() ) );
    }
    else
    {
        unformatted = ::rtl::OUString(
            ResId( RID_SVXSTR_EXCEPTION_RUNNING, DIALOG_MGR() ) );
    }

    if ( eScriptException.exceptionType.getLength() != 0 )
    {
        type = eScriptException.exceptionType;
    }

    return FormatErrorString(
        unformatted, language, script, line, type, message );

}
::rtl::OUString GetErrorMessage(
    const provider::ScriptFrameworkErrorException& sError )
{
    ::rtl::OUString unformatted = ::rtl::OUString(
        ResId( RID_SVXSTR_FRAMEWORK_ERROR_RUNNING, DIALOG_MGR() ) );

    ::rtl::OUString language =
        ::rtl::OUString::createFromAscii( "UNKNOWN" );

    ::rtl::OUString script =
        ::rtl::OUString::createFromAscii( "UNKNOWN" );

    ::rtl::OUString message;

    if ( sError.scriptName.getLength() > 0 )
    {
        script = sError.scriptName;
    }
    if ( sError.language.getLength() > 0 )
    {
        language = sError.language;
    }
    if ( sError.errorType == provider::ScriptFrameworkErrorType::NOTSUPPORTED )
    {
        message = ::rtl::OUString(
            ResId(  RID_SVXSTR_ERROR_LANG_NOT_SUPPORTED, DIALOG_MGR() ) );
        message =  ReplaceString(
        message, ::rtl::OUString::createFromAscii( "%LANGUAGENAME" ), language );

    }
    else
    {
        message = sError.Message;
    }
    return FormatErrorString(
        unformatted, language, script, ::rtl::OUString(), ::rtl::OUString(), message );
}

::rtl::OUString GetErrorMessage( const RuntimeException& re )
{
    Type t = ::getCppuType( &re );
    ::rtl::OUString message = t.getTypeName();
    message += re.Message;

    return message;
}

::rtl::OUString GetErrorMessage( const Exception& e )
{
    Type t = ::getCppuType( &e );
    ::rtl::OUString message = t.getTypeName();
    message += e.Message;

    return message;
}

::rtl::OUString GetErrorMessage( const com::sun::star::uno::Any& aException )
{
    ::rtl::OUString exType;
    if ( aException.getValueType() ==
         ::getCppuType( (const reflection::InvocationTargetException* ) NULL ) )
    {
        OSL_TRACE("Detected InvocationTarget");
        reflection::InvocationTargetException ite;
        aException >>= ite;
        if ( ite.TargetException.getValueType() == ::getCppuType( ( const provider::ScriptErrorRaisedException* ) NULL ) )
        {
            OSL_TRACE("Detected ScriptErrorRaisedException in InvocationTarget");
            // Error raised by script
            provider::ScriptErrorRaisedException scriptError;
            ite.TargetException >>= scriptError;
            return GetErrorMessage( scriptError );
        }
        else if ( ite.TargetException.getValueType() == ::getCppuType( ( const provider::ScriptExceptionRaisedException* ) NULL ) )
        {
            OSL_TRACE("Detected ScriptExceptionRaisedException in InvocationTarget");
            // Exception raised by script
            provider::ScriptExceptionRaisedException scriptException;
            ite.TargetException >>= scriptException;
            return GetErrorMessage( scriptException );
        }
        else
        {
            // Unknown error, shouldn't happen
            // OSL_ASSERT(...)
            OSL_TRACE("Unknown error");
        }

    }
    else if ( aException.getValueType() == ::getCppuType( ( const provider::ScriptFrameworkErrorException* ) NULL ) )
    {
        OSL_TRACE("Detected ScriptFrameworkErrorException ");
        // A Script Framework error has occured
        provider::ScriptFrameworkErrorException sfe;
        aException >>= sfe;
        return GetErrorMessage( sfe );

    }
    // unknown exception
    Exception e;
    RuntimeException rte;
    if ( aException >>= rte )
    {
        return GetErrorMessage( rte );
    }

    aException >>= e;
    return GetErrorMessage( e );

}

SvxScriptErrorDialog::SvxScriptErrorDialog(
    Window* parent, ::com::sun::star::uno::Any aException )
    : m_sMessage()
{
    m_sMessage = GetErrorMessage( aException );
}

SvxScriptErrorDialog::~SvxScriptErrorDialog()
{
}

USHORT SvxScriptErrorDialog::Execute()
{
    // Show Error dialog asynchronously
    //
    // Pass a copy of the message to the ShowDialog method as the
    // SvxScriptErrorDialog may be deleted before ShowDialog is called
    Application::PostUserEvent(
        LINK( this, SvxScriptErrorDialog, ShowDialog ),
        new rtl::OUString( m_sMessage ) );

    return 0;
}

IMPL_LINK( SvxScriptErrorDialog, ShowDialog, ::rtl::OUString*, pMessage )
{
    ::rtl::OUString message;

    if ( pMessage && pMessage->getLength() != 0 )
    {
        message = *pMessage;
    }
    else
    {
        message = ::rtl::OUString( ResId( RID_SVXSTR_ERROR_TITLE, DIALOG_MGR() ) );
    }

    MessBox* pBox = new WarningBox( NULL, WB_OK, message );
    pBox->SetText( ResId( RID_SVXSTR_ERROR_TITLE, DIALOG_MGR() ) );
    pBox->Execute();

    if ( pBox ) delete pBox;
    if ( pMessage ) delete pMessage;

    return 0;
}
