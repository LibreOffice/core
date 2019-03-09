/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>

#include <sal/log.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <officecfg/Office/Impress.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflclit.hxx>
#include <tools/diagnose_ex.h>
#include <xmloff/autolayout.hxx>

#include "PhotoAlbumDialog.hxx"
#include <strings.hrc>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

namespace sd
{

SdPhotoAlbumDialog::SdPhotoAlbumDialog(weld::Window* pWindow, SdDrawDocument* pActDoc)
    : GenericDialogController(pWindow, "modules/simpress/ui/photoalbum.ui", "PhotoAlbumCreatorDialog")
    , m_pDoc(pActDoc)
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
    , m_xCreateBtn(m_xBuilder->weld_button("ok"))
    , m_xAddBtn(m_xBuilder->weld_button("add_btn"))
    , m_xUpBtn(m_xBuilder->weld_button("up_btn"))
    , m_xDownBtn(m_xBuilder->weld_button("down_btn"))
    , m_xRemoveBtn(m_xBuilder->weld_button("rem_btn"))
    , m_xImagesLst(m_xBuilder->weld_tree_view("images_tree"))
    , m_xImg(new weld::CustomWeld(*m_xBuilder, "preview_img", m_aImg))
    , m_xInsTypeCombo(m_xBuilder->weld_combo_box("opt_combo"))
    , m_xASRCheck(m_xBuilder->weld_check_button("asr_check"))
    , m_xASRCheckCrop(m_xBuilder->weld_check_button("asr_check_crop"))
    , m_xCapCheck(m_xBuilder->weld_check_button("cap_check"))
    , m_xInsertAsLinkCheck(m_xBuilder->weld_check_button("insert_as_link_check"))
{
    m_xCancelBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, CancelHdl));
    m_xCreateBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, CreateHdl));

    m_xAddBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, FileHdl));
    m_xUpBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, UpHdl));
    m_xUpBtn->set_sensitive(false);
    m_xDownBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, DownHdl));
    m_xDownBtn->set_sensitive(false);
    m_xRemoveBtn->connect_clicked(LINK(this, SdPhotoAlbumDialog, RemoveHdl));
    m_xRemoveBtn->set_sensitive(false);
    m_xImagesLst->connect_changed(LINK(this, SdPhotoAlbumDialog, SelectHdl));
    m_xInsTypeCombo->connect_changed(LINK(this, SdPhotoAlbumDialog, TypeSelectHdl));

    m_pGraphicFilter = new GraphicFilter;
    m_xAddBtn->grab_focus();
}

SdPhotoAlbumDialog::~SdPhotoAlbumDialog()
{
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CreateHdl, weld::Button&, void)
{
    if (m_xImagesLst->n_children() == 0)
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xDialog.get(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   SdResId(STR_PHOTO_ALBUM_EMPTY_WARNING)));
        xWarn->run();
    }
    else
    {
        Reference< drawing::XDrawPagesSupplier > xDPS( m_pDoc->getUnoModel(), uno::UNO_QUERY );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
        Reference< lang::XMultiServiceFactory > xShapeFactory( m_pDoc->getUnoModel(), uno::UNO_QUERY );

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // determine if to use Captions (use TitleObject) and choose the correct AutoLayout
        // from the beginning
        const bool bCreateCaptions(m_xCapCheck->get_active());
        const bool bInsertAsLink(m_xInsertAsLinkCheck->get_active());
        const AutoLayout aAutoLayout(bCreateCaptions ? AUTOLAYOUT_TITLE_ONLY : AUTOLAYOUT_NONE);

        // get the option
        const int nOpt = m_xInsTypeCombo->get_active();
        if (nOpt == ONE_IMAGE)
        {
            for( sal_Int32 i = 0; i < m_xImagesLst->n_children(); ++i )
            {
                OUString sUrl = m_xImagesLst->get_id(i);

                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl, xProvider);

                Graphic aGraphic(xGraphic);
                if (bInsertAsLink)
                    aGraphic.setOriginURL(sUrl);

                // Save the original size, multiplied with 100
                ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                Reference< drawing::XShape > xShape(
                    xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                    uno::UNO_QUERY);

                Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                ::awt::Point aPicPos;

                if (m_xASRCheck->get_active() && !m_xASRCheckCrop->get_active())
                {
                    // Resize the image, with keeping ASR
                    aPicSize = createASRSize(aPicSize, aPageSize);
                }
                else if (m_xASRCheckCrop->get_active())
                {
                    aPicSize = createASRSizeCrop(aPicSize, aPageSize);
                }

                xShape->setSize(aPicSize);
                aPicPos.X = (aPageSize.Width - aPicSize.Width)/2;
                aPicPos.Y = (aPageSize.Height - aPicSize.Height)/2;

                xShape->setPosition(aPicPos);
                try
                {
                    xSlide->add(xShape);
                    if (bCreateCaptions)
                        createCaption( aPageSize );
                }
                catch (const css::uno::Exception&)
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN( "sd", exceptionToString(ex) );
                }
            }
        }
        else if( nOpt == TWO_IMAGES )
        {
            for( sal_Int32 i = 0; i < m_xImagesLst->n_children(); i+=2 )
            {
                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                // grab the left one
                OUString sUrl1 = m_xImagesLst->get_id(i);
                // grab the right one
                OUString sUrl2 = m_xImagesLst->get_id(i+1);

                if( !sUrl1.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl1);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/2 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }

                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl2);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/2 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);

                    try
                    {
                        xSlide->add(xShape);
                        if(bCreateCaptions)
                            createCaption( aPageSize );
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }
            }
        }
        else if( nOpt == FOUR_IMAGES )
        {
            for( sal_Int32 i = 0; i < m_xImagesLst->n_children(); i+=4 )
            {
                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                // grab the upper left one
                OUString sUrl1 = m_xImagesLst->get_id(i);

                // grab the upper right one
                OUString sUrl2 = m_xImagesLst->get_id(i+1);

                // grab the lower left one
                OUString sUrl3 = m_xImagesLst->get_id(i+2);

                // grab the lower right one
                OUString sUrl4 = m_xImagesLst->get_id(i+3);

                if( !sUrl1.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl1);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }
                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl2);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }
                if( !sUrl3.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl3, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl3);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2 + aPageSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }
                if( !sUrl4.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl4, xProvider);

                    Graphic aGraphic(xGraphic);
                    if (bInsertAsLink)
                        aGraphic.setOriginURL(sUrl4);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aGraphic.GetSizePixel().Width()*100, aGraphic.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if (m_xASRCheck->get_active())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2 + aPageSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                        if(bCreateCaptions)
                            createCaption( aPageSize );
                    }
                    catch (const css::uno::Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN( "sd", exceptionToString(ex) );
                    }
                }
            }
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          "Function is not implemented!"));
            xInfoBox->run();
        }
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, FileHdl, weld::Button&, void)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_PREVIEW,
        FileDialogFlags::Graphic | FileDialogFlags::MultiSelection, m_xDialog.get());
    // Read configuration
    OUString sUrl(officecfg::Office::Impress::Pictures::Path::get());

    INetURLObject aFile( SvtPathOptions().GetUserConfigPath() );
    if (!sUrl.isEmpty())
        aDlg.SetDisplayDirectory(sUrl);
    else
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        Sequence< OUString > aFilesArr = aDlg.GetSelectedFiles();
        if( aFilesArr.getLength() )
        {
            sUrl = aDlg.GetDisplayDirectory();
            // Write out configuration
            {
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Impress::Pictures::Path::set(sUrl, batch);
                batch->commit();
            }

            for ( sal_Int32 i = 0; i < aFilesArr.getLength(); i++ )
            {
                // Store full path, show filename only. Use INetURLObject to display spaces in filename correctly
                INetURLObject aUrl = INetURLObject(aFilesArr[i]);
                m_xImagesLst->append(aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE), aUrl.GetLastName(INetURLObject::DecodeMechanism::WithCharset), "");
            }
        }
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl, weld::Button&, void)
{
    const int nActPos = m_xImagesLst->get_selected_index();
    if (nActPos != -1 && nActPos != 0)
    {
        OUString sActEntry(m_xImagesLst->get_text(nActPos));
        // actual data
        OUString sAct(m_xImagesLst->get_id(nActPos));

        OUString sUpperEntry(m_xImagesLst->get_text(nActPos - 1));
        // upper data
        OUString sUpper(m_xImagesLst->get_id(nActPos - 1));

        m_xImagesLst->remove_text(sActEntry);
        m_xImagesLst->remove_text(sUpperEntry);

        m_xImagesLst->insert(nActPos - 1, sActEntry, &sAct, nullptr, nullptr);
        m_xImagesLst->insert(nActPos, sUpperEntry, &sUpper, nullptr, nullptr);

        m_xImagesLst->select(nActPos - 1);
    }

    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, DownHdl, weld::Button&, void)
{
    const int nActPos = m_xImagesLst->get_selected_index();
    if (!m_xImagesLst->get_text(nActPos + 1).isEmpty())
    {
        OUString sActEntry(m_xImagesLst->get_selected_text());
        OUString sAct(m_xImagesLst->get_selected_id());

        OUString sDownEntry(m_xImagesLst->get_text(nActPos + 1));
        OUString sDown(m_xImagesLst->get_id(nActPos + 1));

        m_xImagesLst->remove_text(sActEntry);
        m_xImagesLst->remove_text(sDownEntry);

        m_xImagesLst->insert(nActPos, sDownEntry, &sDown, nullptr, nullptr);
        m_xImagesLst->insert(nActPos + 1, sActEntry, &sAct, nullptr, nullptr);

        m_xImagesLst->select(nActPos + 1);
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl, weld::Button&, void)
{
    m_xImagesLst->remove(m_xImagesLst->get_selected_index());
    m_aImg.SetGraphic(Graphic());

    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl, weld::TreeView&, void)
{
    OUString sImgUrl = m_xImagesLst->get_selected_id();

    if (sImgUrl != SdResId(STR_PHOTO_ALBUM_TEXTBOX))
    {
        GraphicFilter aCurFilter;
        Graphic aGraphic;
        INetURLObject aURLObj( sImgUrl );

        sal_uInt16 nFilter = GRFILTER_FORMAT_DONTKNOW;

        if ( aURLObj.HasError() || INetProtocol::NotValid == aURLObj.GetProtocol() )
        {
            aURLObj.SetSmartProtocol( INetProtocol::File );
            aURLObj.SetSmartURL( sImgUrl );
        }

        GraphicFilterImportFlags nFilterImportFlags = GraphicFilterImportFlags::SetLogsizeForJpeg;
        // remote?
        if ( INetProtocol::File != aURLObj.GetProtocol() )
        {
            std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( sImgUrl, StreamMode::READ );

            if( pStream )
                m_pGraphicFilter->ImportGraphic( aGraphic, sImgUrl, *pStream, nFilter, nullptr, nFilterImportFlags );
            else
                m_pGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
        }
        else
        {
            m_pGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
        }

        BitmapEx aBmp = aGraphic.GetBitmapEx();
        sal_Int32 nBmpWidth  = aBmp.GetSizePixel().Width();
        sal_Int32 nBmpHeight = aBmp.GetSizePixel().Height();

        double nXRatio = double(200) / nBmpWidth;
        double nYRatio = double(150) / nBmpHeight;
        if ( nXRatio < nYRatio )
            aBmp.Scale( nXRatio, nXRatio );
        else
            aBmp.Scale( nYRatio, nYRatio );

        aBmp.Convert( BmpConversion::N24Bit );
        m_aImg.SetGraphic(Graphic(aBmp));
    }
    else
    {
        m_aImg.SetGraphic(Graphic());
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, TypeSelectHdl, weld::ComboBox&, void)
{
    // Enable "Fill Slide" only for one image
    // If we want to have it for other images too, we need to implement the actual cropping.
    bool const bEnable = m_xInsTypeCombo->get_active() == ONE_IMAGE;
    m_xASRCheckCrop->set_sensitive(bEnable);
    if (!bEnable)
        m_xASRCheckCrop->set_active(false);
}

Reference< drawing::XDrawPage > SdPhotoAlbumDialog::appendNewSlide(AutoLayout aLayout,
    const Reference< drawing::XDrawPages >& xDrawPages
)
{
    Reference< drawing::XDrawPage > xSlide; // Create the slide
    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
    SdPage* pSlide = m_pDoc->GetSdPage( m_pDoc->GetSdPageCount(PageKind::Standard)-1, PageKind::Standard);
    pSlide->SetAutoLayout(aLayout, true); // Set the layout here
    return xSlide;
}

awt::Size SdPhotoAlbumDialog::createASRSize(const awt::Size& aPicSize, const awt::Size& aMaxSize)
{
    double resizeWidth = aPicSize.Width;
    double resizeHeight = aPicSize.Height;
    double aspect = resizeWidth/resizeHeight;

    if( resizeWidth > aMaxSize.Width )
    {
        resizeWidth = aMaxSize.Width;
        resizeHeight = resizeWidth / aspect;
    }

    if( resizeHeight > aMaxSize.Height )
    {
        aspect = resizeWidth/resizeHeight;
        resizeHeight = aMaxSize.Height;
        resizeWidth = resizeHeight * aspect;
    }
    return awt::Size(resizeWidth, resizeHeight);
}

awt::Size SdPhotoAlbumDialog::createASRSizeCrop(const awt::Size& aPicSize, const awt::Size& aMaxSize)
{
    double resizeWidth = aPicSize.Width;
    double resizeHeight = aPicSize.Height;
    double imgAspect = resizeWidth / resizeHeight;
    double windowAspectRatio = static_cast<double>(aMaxSize.Width) / aMaxSize.Height ;


    //When both sides of an image are bigger than canvas size, image would be downscaled.
    if( resizeWidth > aMaxSize.Width && resizeHeight > aMaxSize.Height )
    {
        if( imgAspect > windowAspectRatio )
        {
            resizeHeight = aMaxSize.Height;
            resizeWidth = aMaxSize.Height * imgAspect;
        }
        else
        {
            resizeHeight = aMaxSize.Width / imgAspect;
            resizeWidth = aMaxSize.Width;
        }

    }
    //In all other cases image is upscaled
    else
    {
        if( imgAspect > windowAspectRatio )
        {
            resizeHeight = aMaxSize.Height;
            resizeWidth = aMaxSize.Height * imgAspect;
        }
        else
        {
            resizeWidth = aMaxSize.Width;
            resizeHeight = aMaxSize.Width / imgAspect;
        }
    }
    return awt::Size(resizeWidth, resizeHeight);
}

void SdPhotoAlbumDialog::createCaption(const awt::Size& aPageSize )
{
    Point CapPos;
    Size CapSize;

    CapSize.setWidth( aPageSize.Width );
    CapSize.setHeight( aPageSize.Height/6 );
    CapPos.setX( 0 );
    CapPos.setY( aPageSize.Height - CapSize.Height() );
    SdPage* pSlide = m_pDoc->GetSdPage( m_pDoc->GetSdPageCount(PageKind::Standard)-1, PageKind::Standard );

    // try to get existing PresObj
    const ::tools::Rectangle rRect(CapPos,CapSize);
    SdrObject* pSdrObj = pSlide->GetPresObj(PRESOBJ_TITLE);

    if(!pSdrObj)
    {
        // if not exists, create. Beware: It is already inserted to the SdPage
        pSdrObj = pSlide->CreatePresObj(PRESOBJ_TITLE,false,rRect);
    }
    else
    {
        // if exists, bring to front and position it
        const size_t nObjNum(pSlide->GetObjCount());

        if(nObjNum)
        {
            pSlide->SetObjectOrdNum(pSdrObj->GetOrdNum(), nObjNum - 1);
        }

        pSdrObj->SetSnapRect(rRect);
    }

    if(pSdrObj)
    {
        // set color, style and some transparency
        SfxItemSet aSet(m_pDoc->GetItemPool() );

        aSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
        aSet.Put( XFillColorItem( "", COL_BLACK ) );
        aSet.Put( XFillTransparenceItem( 20 ) );
        pSdrObj->SetMergedItemSetAndBroadcast(aSet);
    }
}

Reference< graphic::XGraphic> SdPhotoAlbumDialog::createXGraphicFromUrl(const OUString& sUrl,
    const Reference< graphic::XGraphicProvider>& xProvider
)
{
    // The same as above, except this returns an XGraphic from the image URL
    ::comphelper::NamedValueCollection aMediaProperties;
    aMediaProperties.put( "URL", sUrl );
    Reference< graphic::XGraphic> xGraphic =
        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
    return xGraphic;
}

void SdPhotoAlbumDialog::EnableDisableButtons()
{
    m_xRemoveBtn->set_sensitive(m_xImagesLst->count_selected_rows() > 0);
    m_xUpBtn->set_sensitive(m_xImagesLst->count_selected_rows() > 0 &&
                            m_xImagesLst->get_selected_index() != 0);
    m_xDownBtn->set_sensitive(m_xImagesLst->count_selected_rows() > 0 &&
                              m_xImagesLst->get_selected_index() < m_xImagesLst->n_children() - 1);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
