/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#ifndef _SFXOBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#include <sfx2/request.hxx>

#include <svx/dialmgr.hxx>
#include <svx/grfflt.hxx>
//#include "grfflt.hrc"
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx> //CHINA001

// --------------------
// - SvxGraphicFilter -
// --------------------

sal_uIntPtr SvxGraphicFilter::ExecuteGrfFilterSlot( SfxRequest& rReq, GraphicObject& rFilterObject )
{
    const Graphic&  rGraphic = rFilterObject.GetGraphic();
    sal_uIntPtr         nRet = SVX_GRAPHICFILTER_UNSUPPORTED_GRAPHICTYPE;

    if( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        SfxViewFrame*   pViewFrame = SfxViewFrame::Current();
        SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : NULL;
        Window*         pWindow = ( pViewFrame && pViewFrame->GetViewShell() ) ? pViewFrame->GetViewShell()->GetWindow() : NULL;
        Graphic         aGraphic;

        switch( rReq.GetSlot() )
        {
            case( SID_GRFFILTER_INVERT ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Invert() )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Invert() )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_SMOOTH ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SMOOTH ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SMOOTH ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_SHARPEN ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SHARPEN ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SHARPEN ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_REMOVENOISE ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_REMOVENOISE ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_REMOVENOISE ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_SOBEL ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SOBEL_GREY ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SOBEL_GREY ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_MOSAIC ):
            {
                //CHINA001 GraphicFilterMosaic aDlg( pWindow, rGraphic, 4, 4, sal_False );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterMosaic( pWindow, rGraphic, 4, 4, sal_False, RID_SVX_GRFFILTER_DLG_MOSAIC);
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                    if( aDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 ); //CHINA001 aGraphic = aDlg.GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg; //add by CHINA001
                }
            }
            break;

            case( SID_GRFFILTER_EMBOSS  ):
            {
                //CHINA001 GraphicFilterEmboss aDlg( pWindow, rGraphic, RP_MM );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterEmboss( pWindow, rGraphic, RP_MM, RID_SVX_GRFFILTER_DLG_EMBOSS );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                    if( aDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 ); //CHINA001 aGraphic = aDlg.GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg; //add by CHINA001
                }
            }
            break;

            case( SID_GRFFILTER_POSTER  ):
            {
                //CHINA001 GraphicFilterPoster aDlg( pWindow, rGraphic, 16 );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterPosterSepia( pWindow, rGraphic, 16, RID_SVX_GRFFILTER_DLG_POSTER );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                    if( aDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 ); //CHINA001 aGraphic = aDlg.GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg; //add by CHINA001
                }
            }
            break;

            case( SID_GRFFILTER_POPART  ):
            {
                if( pShell )
                    pShell->SetWaitCursor( sal_True );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_POPART ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_POPART ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( sal_False );
            }
            break;

            case( SID_GRFFILTER_SEPIA ):
            {
                //CHINA001 GraphicFilterSepia aDlg( pWindow, rGraphic, 10 );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterPosterSepia( pWindow, rGraphic, 10, RID_SVX_GRFFILTER_DLG_SEPIA );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                    if( aDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 ); //CHINA001 aGraphic = aDlg.GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg; //add by CHINA001
                }
            }
            break;

            case( SID_GRFFILTER_SOLARIZE ):
            {
                //CHINA001 GraphicFilterSolarize aDlg( pWindow, rGraphic, 128, sal_False );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterSolarize( pWindow, rGraphic, 128, sal_False, RID_SVX_GRFFILTER_DLG_SOLARIZE );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                    if( aDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 ); //CHINA001 aGraphic = aDlg.GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg; //add by CHINA001
                }
            }
            break;

            case SID_GRFFILTER :
            {
                // do nothing; no error
                nRet = SVX_GRAPHICFILTER_ERRCODE_NONE;
                break;
            }

            default:
            {
                DBG_ERROR( "SvxGraphicFilter: selected filter slot not yet implemented" );
                nRet = SVX_GRAPHICFILTER_UNSUPPORTED_SLOT;
            }
            break;
        }

        if( aGraphic.GetType() != GRAPHIC_NONE )
        {
            rFilterObject.SetGraphic( aGraphic );
            nRet = SVX_GRAPHICFILTER_ERRCODE_NONE;
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

void SvxGraphicFilter::DisableGraphicFilterSlots( SfxItemSet& rSet )
{
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER ) )
        rSet.DisableItem( SID_GRFFILTER );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_INVERT ) )
        rSet.DisableItem( SID_GRFFILTER_INVERT );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SMOOTH ) )
        rSet.DisableItem( SID_GRFFILTER_SMOOTH );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SHARPEN ) )
        rSet.DisableItem( SID_GRFFILTER_SHARPEN );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_REMOVENOISE ) )
        rSet.DisableItem( SID_GRFFILTER_REMOVENOISE );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SOBEL ) )
        rSet.DisableItem( SID_GRFFILTER_SOBEL );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_MOSAIC ) )
        rSet.DisableItem( SID_GRFFILTER_MOSAIC );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_EMBOSS ) )
        rSet.DisableItem( SID_GRFFILTER_EMBOSS );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_POSTER ) )
        rSet.DisableItem( SID_GRFFILTER_POSTER );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_POPART ) )
        rSet.DisableItem( SID_GRFFILTER_POPART );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SEPIA ) )
        rSet.DisableItem( SID_GRFFILTER_SEPIA );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SOLARIZE ) )
        rSet.DisableItem( SID_GRFFILTER_SOLARIZE );
};

// eof
