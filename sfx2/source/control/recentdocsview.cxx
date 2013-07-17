/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/recentdocsview.hxx>

#include <sfx2/sfxresid.hxx> //only for testing
#include "../doc/doc.hrc"  //only for testing

#include <sfx2/templateabstractview.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
//using namespace com::sun::star::util;

static const char SFX_REFERER_USER[] = "private:user";

RecentDocsView::RecentDocsView( Window* pParent )
    : RecentDocsViewType(m_aMutex),
      ThumbnailView(pParent)
{
    SetStyle(GetStyle() | WB_HSCROLL);
    //setItemDimensions(20,20,20,0);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRecentDocsView(Window *pParent, VclBuilder::stringmap &)
{
    return new RecentDocsView(pParent);
}

RecentDocsView::~RecentDocsView()
{
}

void RecentDocsView::insertItem(const OUString &rURL, const OUString &rTitle)
{
    RecentDocViewItem *pChild = new RecentDocViewItem(*this);

    // THINK ABOUT: putting all this in the ctor of RecentDocViewItem
    OUString aTitle = rTitle;
    if( !aTitle.getLength() )
    {
        // If we have no title, get filename from the URL
        INetURLObject aURLObj(rURL);
        aTitle = aURLObj.GetName(INetURLObject::DECODE_WITH_CHARSET);
    }

    BitmapEx aThumbnail = ThumbnailView::readThumbnail(rURL);
    if( aThumbnail.IsEmpty() )
    {
        // Use the default thumbnail if we have nothing else
        //aThumbnail = TemplateAbstractView::getDefaultThumbnail(rURL);
        aThumbnail = BitmapEx ( SfxResId( SFX_THUMBNAIL_TEXT ) );
    }

    pChild->aURL = rURL;
    pChild->aTitle = aTitle;
    pChild->maPreview1 = aThumbnail;

    //pChild->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    AppendItem(pChild);
}

// XInitialization
void SAL_CALL RecentDocsView::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if(aArguments[i] >>= aPropValue)
            {
                if ( aPropValue.Name == "Frame" )
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name == "ModuleName" )
                    aPropValue.Value >>= m_aModuleName;
            }
        }

        if ( xFrame.is() )
        {
            m_xFrame        = xFrame;
            m_bInitialized  = true;
        }
    }
}

void RecentDocsView::loadRecentDocs()
{
    int nMaxThumbnailItems = 10;

    Clear();

    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    int nRecentThumbnailItems = nMaxThumbnailItems < aHistoryList.getLength() ? nMaxThumbnailItems : aHistoryList.getLength();
    for ( int i = 0; i < nRecentThumbnailItems; i++ )
    {
        Sequence< PropertyValue >& rRecentEntry = aHistoryList[i];

        OUString aURL;
        OUString aTitle;

        for ( int j = 0; j < rRecentEntry.getLength(); j++ )
        {
            Any a = rRecentEntry[j].Value;

            if (rRecentEntry[j].Name == "URL")
                a >>= aURL;
            else if (rRecentEntry[j].Name == "Title")
                a >>= aTitle;
        }

        insertItem(aURL, aTitle);
    }

    CalculateItemPositions();
    Invalidate();
}

void RecentDocsView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    if(pItem == NULL) return;

    RecentDocViewItem* pRecentItem = dynamic_cast<RecentDocViewItem*>(pItem);

    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    css::util::URL                    aTargetURL;
    Sequence< PropertyValue >         aArgsList;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xDispatchProvider = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    aLock.clear();

    aTargetURL.Complete = pRecentItem->aURL;
    m_xURLTransformer->parseStrict( aTargetURL );

    sal_Int32 nSize = 2;
    aArgsList.realloc( nSize );
    aArgsList[0].Name = "Referer";
    aArgsList[0].Value = makeAny( OUString( SFX_REFERER_USER ) );

    // documents in the picklist will never be opened as templates
    aArgsList[1].Name = "AsTemplate";
    aArgsList[1].Value = makeAny( (sal_Bool) sal_False );

    if (!m_aModuleName.isEmpty())
    {
        // Type detection needs to know which app we are opening it from.
        aArgsList.realloc(++nSize);
        aArgsList[nSize-1].Name = "DocumentService";
        aArgsList[nSize-1].Value <<= m_aModuleName;
    }

    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, "_default", 0 );

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch  = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq    = aArgsList;

        Application::PostUserEvent( STATIC_LINK(0, RecentDocsView, ExecuteHdl_Impl), pLoadRecentFile );
    }
}

IMPL_STATIC_LINK_NOINSTANCE( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( const Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
