/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuhhconv.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:11:46 $
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

#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONOPTION_HPP_
#include <com/sun/star/i18n/TextConversionOption.hpp>
#endif

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <vcl/msgbox.hxx>

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#define ITEMID_LANGUAGE EE_CHAR_LANGUAGE_CJK
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#define ITEMID_FONT EE_CHAR_FONTINFO_CJK
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif

#include <fuhhconv.hxx>
#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "Window.hxx"

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

#include "sdresid.hxx"
#include "strings.hrc"

class SfxRequest;

#define C2U(cChar)  rtl::OUString::createFromAscii(cChar)

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

class ViewShell;

TYPEINIT1( FuHangulHanjaConversion, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuHangulHanjaConversion::FuHangulHanjaConversion (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDocument,
    SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDocument, rReq),
    pSdOutliner(NULL),
    bOwnOutliner(FALSE)
{
    if ( pViewShell->ISA(DrawViewShell) )
    {
        bOwnOutliner = TRUE;
        pSdOutliner = new Outliner( pDoc, OUTLINERMODE_TEXTOBJECT );
    }
    else if ( pViewShell->ISA(OutlineViewShell) )
    {
        bOwnOutliner = FALSE;
        pSdOutliner = pDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}



/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuHangulHanjaConversion::~FuHangulHanjaConversion()
{
    if (pSdOutliner)
        pSdOutliner->EndConversion();

    if (bOwnOutliner)
        delete pSdOutliner;
}


/*************************************************************************
|*
|* Suchen&Ersetzen
|*
\************************************************************************/

void FuHangulHanjaConversion::StartConversion( INT16 nSourceLanguage, INT16 nTargetLanguage,
        const Font *pTargetFont, INT32 nOptions, BOOL bIsInteractive )
{

    String aString( SdResId(STR_UNDO_HANGULHANJACONVERSION) );
    pView->BegUndo( aString );

    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    if (pBase != NULL)
        pViewShell = pBase->GetMainViewShell();

    if( pViewShell )
    {
        if ( pSdOutliner && pViewShell->ISA(DrawViewShell) && !bOwnOutliner )
        {
            pSdOutliner->EndConversion();

            bOwnOutliner = TRUE;
            pSdOutliner = new Outliner( pDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->BeginConversion();
        }
        else if ( pSdOutliner && pViewShell->ISA(OutlineViewShell) && bOwnOutliner )
        {
            pSdOutliner->EndConversion();
            delete pSdOutliner;

            bOwnOutliner = FALSE;
            pSdOutliner = pDoc->GetOutliner();
            pSdOutliner->BeginConversion();
        }

        if (pSdOutliner)
            pSdOutliner->StartConversion(nSourceLanguage, nTargetLanguage, pTargetFont, nOptions, bIsInteractive );
    }

    // Due to changing between edit mode, notes mode, and handout mode the
    // view has most likely changed.  Get the new one.
    pViewShell = pBase->GetMainViewShell();
    if (pViewShell != NULL)
    {
        pView = pViewShell->GetView();
        pWindow = pViewShell->GetActiveWindow();
    }
    else
    {
        pView;
        pWindow = NULL;
    }

    if (pView != NULL)
        pView->EndUndo();
}


void FuHangulHanjaConversion::ConvertStyles( INT16 nTargetLanguage, const Font *pTargetFont )
{
    if( !pDoc )
        return;

    SfxStyleSheetBasePool* pStyleSheetPool = pDoc->GetStyleSheetPool();
    if( !pStyleSheetPool )
        return;

    SfxStyleSheetBase* pStyle = pStyleSheetPool->First();
    while( pStyle )
    {
        SfxItemSet& rSet = pStyle->GetItemSet();

        const bool bHasParent = pStyle->GetParent().Len() != 0;

        if( !bHasParent || rSet.GetItemState( EE_CHAR_LANGUAGE_CJK, FALSE ) == SFX_ITEM_SET )
            rSet.Put( SvxLanguageItem( nTargetLanguage, EE_CHAR_LANGUAGE_CJK ) );

        if( pTargetFont &&
            ( !bHasParent || rSet.GetItemState( EE_CHAR_FONTINFO_CJK, FALSE ) == SFX_ITEM_SET ) )
        {
            // set new font attribute
            SvxFontItem aFontItem( (SvxFontItem&) rSet.Get( EE_CHAR_FONTINFO_CJK ) );
            aFontItem.GetFamilyName()   = pTargetFont->GetName();
            aFontItem.GetFamily()       = pTargetFont->GetFamily();
            aFontItem.GetStyleName()    = pTargetFont->GetStyleName();
            aFontItem.GetPitch()        = pTargetFont->GetPitch();
            aFontItem.GetCharSet()      = pTargetFont->GetCharSet();
            rSet.Put( aFontItem );
        }

        pStyle = pStyleSheetPool->Next();
    }

    pDoc->SetLanguage( EE_CHAR_LANGUAGE_CJK, nTargetLanguage );
}

void FuHangulHanjaConversion::StartChineseConversion()
{
    //open ChineseTranslationDialog
    Reference< XComponentContext > xContext(
        ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
    if(xContext.is())
    {
        Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
        if(xMCF.is())
        {
            Reference< ui::dialogs::XExecutableDialog > xDialog(
                    xMCF->createInstanceWithContext(
                        rtl::OUString::createFromAscii("com.sun.star.linguistic2.ChineseTranslationDialog")
                        , xContext), UNO_QUERY);
            Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
            if( xInit.is() )
            {
                //  initialize dialog
                Reference< awt::XWindow > xDialogParentWindow(0);
                Sequence<Any> aSeq(1);
                Any* pArray = aSeq.getArray();
                PropertyValue aParam;
                aParam.Name = rtl::OUString::createFromAscii("ParentWindow");
                aParam.Value <<= makeAny(xDialogParentWindow);
                pArray[0] <<= makeAny(aParam);
                xInit->initialize( aSeq );

                //execute dialog
                sal_Int16 nDialogRet = xDialog->execute();
                if( RET_OK == nDialogRet )
                {
                    //get some parameters from the dialog
                    sal_Bool bToSimplified = sal_True;
                    sal_Bool bUseVariants = sal_True;
                    sal_Bool bCommonTerms = sal_True;
                    Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                    if( xProp.is() )
                    {
                        try
                        {
                            xProp->getPropertyValue( C2U("IsDirectionToSimplified") ) >>= bToSimplified;
                            xProp->getPropertyValue( C2U("IsUseCharacterVariants") ) >>= bUseVariants;
                            xProp->getPropertyValue( C2U("IsTranslateCommonTerms") ) >>= bCommonTerms;
                        }
                        catch( Exception& )
                        {
                        }
                    }

                    //execute translation
                    sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                    sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                    sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                    if( !bCommonTerms )
                        nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                    Font aTargetFont = pWindow->GetDefaultFont(
                                        DEFAULTFONT_CJK_PRESENTATION,
                                        nTargetLang, DEFAULTFONT_FLAGS_ONLYONE );

                    StartConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, sal_False );
                    ConvertStyles( nTargetLang, &aTargetFont );
                }
            }
            Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
            if( xComponent.is() )
                xComponent->dispose();
        }
    }
}
} // end of namespace
