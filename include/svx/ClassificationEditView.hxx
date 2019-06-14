/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX
#define INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX

#include <sal/config.h>
#include <svx/svxdllapi.h>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/customweld.hxx>

namespace svx {

class ClassificationEditEngine : public EditEngine
{
public:
    ClassificationEditEngine(SfxItemPool* pItemPool);

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rTxtColor, boost::optional<Color>& rFldColor) override;
};

class SVX_DLLPUBLIC ClassificationEditView : public weld::CustomWidgetController
                                           , public EditViewCallbacks
{
public:
    ClassificationEditView();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~ClassificationEditView() override;

    void SetCharAttributes();

    void InsertField(const SvxFieldItem& rField);

    void InvertSelectionWeight();

    void SetNumType(SvxNumType eNumType);

    std::unique_ptr<ClassificationEditEngine> pEdEngine;
    std::unique_ptr<EditView> pEdView;

    const ClassificationEditEngine& getEditEngine()
    {
        return *pEdEngine;
    }

    void SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
    {
        pEdEngine->SetModifyHdl(rLink);
    }

protected:
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void Resize() override;


    virtual void EditViewInvalidate(const tools::Rectangle& rRect) const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(), rRect.GetHeight());
    }

    virtual void EditViewSelectionChange() const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw();
    }

    virtual OutputDevice& EditViewOutputDevice() const override
    {
        return GetDrawingArea()->get_ref_device();
    }
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
